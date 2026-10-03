#!/usr/bin/env python3
"""tools/bench_compare.py: the S10-R1/R3/R4 (D-008) judge behind `tools/check.sh bench compare`.

usage: bench_compare.py --base <bin> --head <bin> [--base-par <bin> --head-par <bin>] --kept <file> --log <file>
                        [--smoke]

For each workload (head `--list`, or its `--list --smoke` subset plus the kept workloads with --smoke; env
BENCH_ONLY=<regex> restricts the set for experiments) it runs the processes in ORDER_FULL = ABBA BAAB ABBA BAAB
(A = base, B = head; ORDER_SMOKE = A B B A with --smoke) so drift and process-to-process noise cancel, each
`--only <w> --samples <s>` with s = SAMPLES_FULL = 5 (SAMPLES_SMOKE = 3 with --smoke), pinned with
`taskset -c ${BENCH_CPU:-2}` when taskset exists. The runs of a build are pooled (8 x 5 = 40, smoke 2 x 3 = 6
samples); median, rel. MAD (median absolute deviation / median) and
change = head median / base median - 1 are printed per workload. Stable: rel. MAD <= 5% in both builds.

Parallel variant (S10-T6): --base-par/--head-par are the same harness built with -DFENG_MATRIX_PARALLEL; their
`--list` names the par/ set (prefixed `par/`). Those workloads run after the serial ones with the same order and
sample counts but are NOT pinned (they need many cores); they share the table, BENCH_ONLY, kept.txt and the rules.

Judgement: bench/kept.txt (`<opt-id> <workload>...`, `#` comments, blank lines) must list at least one optimization,
only known workloads, and each listed workload must have change <= -10% (KEPT-ok, else KEPT-miss). Full runs also
fail on any stable workload with change > +5% (REGRESSION); smoke runs print that flag but do not judge it.
The last line is `LANE bench PASS: <k> kept, <n> workloads, <u> unstable` or `LANE bench FAIL: <reason>`.
Standard library only.
"""
import argparse
import os
import re
import shutil
import statistics
import subprocess
import sys

STABLE_MAD = 0.05
KEPT_GAIN = -0.10
REGRESSION = 0.05
ORDER_FULL = "ABBABAABABBABAAB"  # A = base, B = head; ABBA BAAB ABBA BAAB, 8 processes per build
ORDER_SMOKE = "ABBA"     # 2 processes per build
SAMPLES_FULL = 5         # samples per process
SAMPLES_SMOKE = 3


def out(line=""):
    print(line, flush=True)


def fail(reason):
    out(f"LANE bench FAIL: {reason}")
    sys.exit(1)


def run_bench(cmd, log):
    proc = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    log.write(f"$ {' '.join(cmd)}\n{proc.stdout}")
    log.flush()
    if proc.returncode != 0:
        fail(f"'{' '.join(cmd)}' exited {proc.returncode}, see {log.name}")
    return proc.stdout


def parse_samples(text, name):
    for line in text.splitlines():
        f = line.split()
        if len(f) >= 4 and f[0] == "SAMPLES" and f[1] == name and f[2] == "reps":
            return [float(x) for x in f[4:]]
    return None


def info_lines(text):
    return [l for l in text.splitlines() if l.startswith("INFO ")]


def read_kept(path, known):
    """Returns [(opt_id, [workloads])]; fails on an unknown workload or a line without one."""
    kept = []
    try:
        with open(path, encoding="utf-8") as f:
            lines = f.readlines()
    except OSError as e:
        fail(f"cannot read {path}: {e.strerror}")
    for no, raw in enumerate(lines, 1):
        line = raw.split("#", 1)[0].strip()
        if not line:
            continue
        f = line.split()
        if len(f) < 2:
            fail(f"{path}:{no}: optimization '{f[0]}' names no workload")
        for w in f[1:]:
            if w not in known:
                fail(f"{path}:{no}: unknown workload '{w}' (not in bench --list)")
        kept.append((f[0], f[1:]))
    return kept


def stats(xs):
    med = statistics.median(xs)
    mad = statistics.median(abs(x - med) for x in xs)
    return med, (mad / med if med > 0 else float("inf"))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--base", required=True)
    ap.add_argument("--head", required=True)
    ap.add_argument("--base-par")
    ap.add_argument("--head-par")
    ap.add_argument("--kept", required=True)
    ap.add_argument("--log", required=True)
    ap.add_argument("--smoke", action="store_true")
    a = ap.parse_args()
    if bool(a.base_par) != bool(a.head_par):
        ap.error("--base-par and --head-par go together")

    samples = SAMPLES_SMOKE if a.smoke else SAMPLES_FULL
    order = [{"A": "base", "B": "head"}[c] for c in (ORDER_SMOKE if a.smoke else ORDER_FULL)]
    runs_per_build = order.count("base")
    assert runs_per_build == order.count("head")
    smoke = ["--smoke"] if a.smoke else []
    cpu = os.environ.get("BENCH_CPU", "2")
    if shutil.which("taskset"):
        pin = ["taskset", "-c", cpu]
        pinned = f"cpu {cpu} (taskset -c {cpu})"
    else:
        pin = []
        pinned = "none (taskset not found)"

    # (label, base binary, head binary, pin prefix): the serial pair pinned, the parallel pair unpinned
    pairs = [("", a.base, a.head, pin)]
    if a.base_par:
        pairs.append(("-par", a.base_par, a.head_par, []))

    with open(a.log, "w", encoding="utf-8") as log:
        lists = []
        for label, base_exe, head_exe, _ in pairs:
            names = run_bench([head_exe, "--list"], log).split()
            if names != run_bench([base_exe, "--list"], log).split():
                fail(f"base{label} and head{label} bench --list differ (the harness must be the same source)")
            lists.append(names)
        known = [w for names in lists for w in names]
        if len(set(known)) != len(known):
            fail("a workload name is listed by both the serial and the parallel build")
        kept = read_kept(a.kept, set(known))
        kept_w = {w for _, ws in kept for w in ws}
        only = os.environ.get("BENCH_ONLY")
        rx = re.compile(only) if only else None
        jobs = []  # (workload, base binary, head binary, pin prefix) in harness order, serial pair first
        for (label, base_exe, head_exe, pair_pin), names in zip(pairs, lists):
            chosen = run_bench([head_exe, "--list", "--smoke"], log).split() if a.smoke else list(names)
            chosen += [w for w in names if w in kept_w and w not in chosen]
            if rx:
                chosen = [w for w in chosen if rx.search(w)]
            jobs += [(w, base_exe, head_exe, pair_pin) for w in names if w in chosen]
        if only and not jobs:
            fail(f"BENCH_ONLY='{only}' matches no workload")

        infos = {}
        rows = []
        for w, base_exe, head_exe, pair_pin in jobs:
            pool = {"base": [], "head": []}
            label = "-par" if w.startswith("par/") else ""
            for build in order:
                exe = base_exe if build == "base" else head_exe
                text = run_bench(pair_pin + [exe, "--only", w, "--samples", str(samples)] + smoke, log)
                xs = parse_samples(text, w)
                if not xs or len(xs) != samples:
                    fail(f"{build}{label} printed no SAMPLES line with {samples} samples for {w}, see {a.log}")
                pool[build] += xs
                infos.setdefault(build + label, info_lines(text))
            bm, bd = stats(pool["base"])
            hm, hd = stats(pool["head"])
            change = hm / bm - 1.0
            stable = bd <= STABLE_MAD and hd <= STABLE_MAD
            if w in kept_w:
                flag = "KEPT-ok" if change <= KEPT_GAIN else "KEPT-miss"
            elif stable and change > REGRESSION:
                flag = "REGRESSION"
            else:
                flag = "stable" if stable else "unstable"
            rows.append((w, bm, hm, change, bd, hd, len(pool["base"]), len(pool["head"]), stable, flag))

    for build in ("head", "base", "head-par", "base-par"):
        for l in infos.get(build, []):
            key = l.split()[1] if len(l.split()) > 1 else ""
            if build == "head" or key == "flags" or key == "compiler":
                out(f"INFO {build} {l[5:]}")
    out(f"INFO pinned {pinned}")
    if a.base_par:
        out("INFO pinned par/ none (par/ workloads run unpinned: the parallel build needs many cores)")
    seq = ORDER_SMOKE if a.smoke else ORDER_FULL
    seq = " ".join(seq[i:i + 4] for i in range(0, len(seq), 4))
    out(f"INFO mode {'smoke' if a.smoke else 'full'}, order {seq} "
        f"(A base, B head), {runs_per_build} runs x {samples} samples = {runs_per_build * samples} pooled per build")
    if only:
        out(f"INFO BENCH_ONLY {only}")
    out(f"{'workload':<24} {'base_med_s':>12} {'head_med_s':>12} {'change%':>8} {'base_mad%':>9} "
        f"{'head_mad%':>9} {'n':>7} flag")
    for w, bm, hm, ch, bd, hd, nb, nh, _, flag in rows:
        out(f"{w:<24} {bm:12.4e} {hm:12.4e} {100 * ch:+8.2f} {100 * bd:9.2f} {100 * hd:9.2f} "
            f"{f'{nb}/{nh}':>7} {flag}")

    reasons = []
    ran = {r[0]: r for r in rows}
    if not kept:
        reasons.append(f"no kept optimization ({a.kept} lists none)")
    for opt, ws in kept:
        for w in ws:
            if w not in ran:
                reasons.append(f"{opt}: kept workload {w} not run (BENCH_ONLY)")
            elif ran[w][9] == "KEPT-miss":
                reasons.append(f"{opt}: {w} change {100 * ran[w][3]:+.2f}% is not <= -10%")
    regress = [r for r in rows if r[9] == "REGRESSION"]
    if regress and a.smoke:
        out(f"INFO smoke: {len(regress)} REGRESSION flag(s) printed, not judged")
    elif regress:
        reasons.append("stable regression > +5%: " + ", ".join(f"{r[0]} {100 * r[3]:+.2f}%" for r in regress))
    if reasons:
        fail("; ".join(reasons))
    unstable = sum(1 for r in rows if not r[8])
    out(f"LANE bench PASS: {len(kept)} kept, {len(rows)} workloads, {unstable} unstable")


if __name__ == "__main__":
    main()
