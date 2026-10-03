#!/usr/bin/env bash
# tools/check.sh <lane> [args] [--smoke]  (D-013)
# Builds only under build/<lane>/, logs under build/logs/<lane>/, TMPDIR=build/tmp, runs from the repo root.
# Each suite run gets its own fresh TMPDIR=build/tmp/run.XXXXXX, removed after the run (concurrent runs never share one).
# Lanes: gcc clang sanitize tagged '<spec>' warnings api examples make ci docs <ID> compile-fail <ID> fuzz tsan
#        oracle <ID> bench <workload> bench compare config all.
# `fuzz` (S5-R5), `tsan` (S6-R2), `oracle` (S7-R6), `bench` (S8-R4; `bench compare` S10-R1) and `config` (S9-R3)
# are not part of `all` (cost, optional Eigen, timing, link-failure pairs).
#              BENCH_CPU (default 2) is the CPU `bench compare` pins to; BENCH_ONLY=<regex> restricts its workloads
#              (experiments only: a restricted run cannot pass while a kept workload is left out).
# Last line: `LANE <lane> PASS[: detail]` (exit 0) or `LANE <lane> FAIL: <reason>` (exit non-zero).
# Environment: CHECK_GCCS (default "g++ g++-15"), CHECK_CLANG (default clang++), CHECK_JOBS (default 8),
#              CHECK_REBUILD=1 forces rebuilding binaries that are otherwise reused when up to date,
#              CHECK_API_FAMILIES="a b ..." replaces the api lane's family list (default: the backticked first
#              column of the `## API families (S1-R4)` table in docs/stages/S1/spec.md).
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"
SELF="$ROOT/tools/check.sh"

export TMPDIR="$ROOT/build/tmp"
mkdir -p "$TMPDIR"

CHECK_GCCS="${CHECK_GCCS:-g++ g++-15}"
CHECK_CLANG="${CHECK_CLANG:-clang++}"
CHECK_JOBS="${CHECK_JOBS:-8}"
SAN_FLAGS="-fsanitize=address,undefined -fno-sanitize-recover=all -O1 -g -DFENG_MATRIX_CHECKED_ITERATORS"
export ASAN_OPTIONS="${ASAN_OPTIONS:-detect_leaks=1}"
export UBSAN_OPTIONS="${UBSAN_OPTIONS:-print_stacktrace=1}"

die_lane() { # lane reason
    echo "LANE $1 FAIL: $2"
    exit 1
}

# True when $1 exists, its flag stamp equals $2 and it is newer than every input.
up_to_date() {
    local bin="$1" flags="$2" f
    [[ "${CHECK_REBUILD:-0}" == 1 ]] && return 1
    [[ -x "$bin" && -f "$bin.flags" ]] || return 1
    [[ "$(cat "$bin.flags")" == "$flags" ]] || return 1
    for f in matrix.hpp tests/test.cc tests/catch.hpp tests/cases/* tests/fuzz/*.hpp; do
        [[ "$f" -nt "$bin" ]] && return 1
    done
    return 0
}

# build <cxx> <flags> <bin> <buildlog>; returns the compiler's status.
build_suite() {
    local cxx="$1" flags="$2" bin="$3" log="$4"
    mkdir -p "$(dirname "$bin")" "$(dirname "$log")"
    if up_to_date "$bin" "$cxx $flags"; then
        echo "reused $bin (up to date)" > "$log"
        return 0
    fi
    rm -f "$bin" "$bin.flags"
    # shellcheck disable=SC2086
    if "$cxx" $flags -pthread -isystem tests tests/test.cc -o "$bin" > "$log" 2>&1; then
        printf '%s' "$cxx $flags" > "$bin.flags"
        return 0
    fi
    return 1
}

# run_suite <bin> <runlog> [catch spec]; succeeds only if it exits 0, at least one test case ran and nothing failed.
run_suite() {
    local bin="$1" log="$2" tmp rc=0; shift 2
    tmp="$(mktemp -d "$TMPDIR/run.XXXXXX")" || return 1
    TMPDIR="$tmp" "$bin" "$@" > "$log" 2>&1 || rc=$?
    rm -rf "$tmp"
    [[ $rc -eq 0 ]] || return 1
    grep -q '^No tests ran' "$log" && return 2
    grep -Eq '^All tests passed \([0-9]+ assertions? in [1-9][0-9]* test cases?\)' "$log" || return 2
    return 0
}

# Internal: one config of a build matrix. __config <lane> <cxx> <std> <debug|ndebug> <serial|parallel>
config_job() {
    local lane="$1" cxx="$2" std="$3" mode="$4" par="$5"
    local flags="-std=c++$std"
    if [[ "$mode" == debug ]]; then flags+=" -O0 -g"; else flags+=" -O2 -DNDEBUG"; fi
    [[ "$par" == parallel ]] && flags+=" -DFENG_MATRIX_PARALLEL"
    local id="${cxx}-c++${std}-${mode}-${par}"
    local bin="build/$lane/$id/test" logd="build/logs/$lane"
    local status=PASS why=""
    if ! build_suite "$cxx" "$flags" "$bin" "$logd/$id.build.log"; then
        status=FAIL; why=" (build, see $logd/$id.build.log)"
    else
        local rc=0
        run_suite "$bin" "$logd/$id.run.log" || rc=$?
        if [[ $rc == 2 ]]; then status=FAIL; why=" (no test ran, see $logd/$id.run.log)"
        elif [[ $rc != 0 ]]; then status=FAIL; why=" (tests, see $logd/$id.run.log)"; fi
    fi
    echo "CONFIG $cxx c++$std $mode $par $status$why"
    echo "$status" > "build/$lane/$id.result"
}

# matrix_lane <lane> <jobfile lines on stdin>
matrix_lane() {
    local lane="$1"
    mkdir -p "build/$lane" "build/logs/$lane"
    rm -f "build/$lane"/*.result
    local jobs; jobs="$(cat)"
    local n; n="$(grep -c . <<< "$jobs" || true)"
    [[ "$n" -gt 0 ]] || die_lane "$lane" "no configurations"
    # xargs exits non-zero if a job fails; results are judged from the result files.
    xargs -P "$CHECK_JOBS" -L 1 "$SELF" __config "$lane" <<< "$jobs" || true
    local pass fail
    pass="$(cat "build/$lane"/*.result 2>/dev/null | grep -c '^PASS$' || true)"
    fail=$(( n - pass ))
    if [[ "$fail" -ne 0 ]]; then
        die_lane "$lane" "$fail of $n configurations failed"
    fi
    echo "LANE $lane PASS: $n configurations"
}

lane_gcc() {
    local cxx std mode par
    {
        if [[ "$SMOKE" == 1 ]]; then
            echo "g++ 20 debug parallel"
            echo "g++-15 26 ndebug serial"
        else
            for cxx in $CHECK_GCCS; do for std in 20 23 26; do for mode in debug ndebug; do for par in serial parallel; do
                echo "$cxx $std $mode $par"
            done; done; done; done
        fi
    } | matrix_lane gcc
}

lane_clang() {
    local cxx std mode par
    {
        if [[ "$SMOKE" == 1 ]]; then
            echo "$CHECK_CLANG 23 ndebug parallel"
        else
            for cxx in $CHECK_CLANG; do for std in 20 23 26; do for mode in debug ndebug; do for par in serial parallel; do
                echo "$cxx $std $mode $par"
            done; done; done; done
        fi
    } | matrix_lane clang
}

first_gcc() { set -- $CHECK_GCCS; echo "$1"; }

# Internal: one sanitizer build. __sanbuild <cxx> <debug|ndebug>; builds build/sanitize/<cxx>-<mode>/test.
san_bin() { echo "build/sanitize/$1-$2/test"; }
san_flags() {
    local f="-std=c++20 $SAN_FLAGS -DFENG_MATRIX_PARALLEL"
    [[ "$1" == ndebug ]] && f+=" -DNDEBUG"
    echo "$f"
}
san_build_job() {
    local cxx="$1" mode="$2"
    mkdir -p build/logs/sanitize
    if build_suite "$cxx" "$(san_flags "$mode")" "$(san_bin "$cxx" "$mode")" "build/logs/sanitize/$cxx-$mode.build.log"; then
        echo "BUILD $cxx $mode asan+ubsan PASS"
    else
        echo "BUILD $cxx $mode asan+ubsan FAIL (see build/logs/sanitize/$cxx-$mode.build.log)"
    fi
}

san_variants() { # prints "<cxx> <mode>" lines
    local g; g="$(first_gcc)"
    if [[ "$SMOKE" == 1 && "$1" == sanitize ]]; then
        echo "$g debug"; echo "$CHECK_CLANG ndebug"
    else
        echo "$g debug"; echo "$g ndebug"; echo "$CHECK_CLANG debug"; echo "$CHECK_CLANG ndebug"
    fi
}

# san_runs <lane> [spec]; builds every variant, runs each, checks logs.
san_runs() {
    local lane="$1"; shift
    mkdir -p build/sanitize "build/logs/$lane"
    local variants; variants="$(san_variants "$lane")"
    xargs -P "$CHECK_JOBS" -L 1 "$SELF" __sanbuild <<< "$variants" || true
    local n=0 fail=0 nomatch=0 cxx mode bin log rc
    while read -r cxx mode; do
        n=$((n + 1))
        bin="$(san_bin "$cxx" "$mode")"
        log="build/logs/$lane/$cxx-$mode.run.log"
        if [[ ! -x "$bin" ]]; then
            echo "CONFIG $cxx c++20 $mode asan+ubsan FAIL (build)"; fail=$((fail + 1)); continue
        fi
        rc=0
        run_suite "$bin" "$log" "$@" || rc=$?
        if grep -Eq 'runtime error:|AddressSanitizer' "$log"; then rc=3; fi
        case $rc in
            0) echo "CONFIG $cxx c++20 $mode asan+ubsan PASS" ;;
            2) echo "CONFIG $cxx c++20 $mode asan+ubsan FAIL (no test matched, see $log)"; nomatch=$((nomatch + 1)) ;;
            3) echo "CONFIG $cxx c++20 $mode asan+ubsan FAIL (sanitizer diagnostic, see $log)"; fail=$((fail + 1)) ;;
            *) echo "CONFIG $cxx c++20 $mode asan+ubsan FAIL (tests, see $log)"; fail=$((fail + 1)) ;;
        esac
    done <<< "$variants"
    if [[ $nomatch -ne 0 ]]; then die_lane "$lane" "$nomatch of $n runs matched no test"; fi
    if [[ $fail -ne 0 ]]; then die_lane "$lane" "$fail of $n runs failed"; fi
    echo "LANE $lane PASS: $n runs"
}

lane_sanitize() { san_runs sanitize; }

lane_tagged() {
    [[ ${#ARGS[@]} -ge 1 && -n "${ARGS[0]}" ]] || die_lane tagged "usage: tools/check.sh tagged '<catch spec>' [--smoke]"
    san_runs tagged "${ARGS[@]}"
}

# ---- warnings lane (S1-R3) ----
WARN_FLAGS="-Wall -Wextra -Werror -DFENG_MATRIX_PARALLEL -pthread -isystem tests -fsyntax-only"

# Internal: one warnings config. __warn <cxx> <std>; any diagnostic, or any literal-operator deprecation, fails it.
warn_job() {
    local cxx="$1" std="$2" id="$1-c++$2" log="build/logs/warnings/$1-c++$2.log" status=PASS why=""
    mkdir -p build/warnings build/logs/warnings
    # shellcheck disable=SC2086
    if ! "$cxx" -std=c++$std $WARN_FLAGS tests/test.cc > "$log" 2>&1; then
        status=FAIL; why=" (compile, see $log)"
    elif [[ -s "$log" ]]; then
        status=FAIL; why=" (diagnostics, see $log)"
    fi
    if grep -q 'deprecated-literal-operator' "$log"; then status=FAIL; why=" (deprecated-literal-operator, see $log)"; fi
    echo "CONFIG $cxx c++$std warnings $status$why"
    echo "$status" > "build/warnings/$id.result"
}

lane_warnings() {
    local cxx std jobs n pass
    if [[ "$SMOKE" == 1 ]]; then
        jobs="$(first_gcc) 26"$'\n'"$CHECK_CLANG 26"
    else
        jobs="$(for cxx in $CHECK_GCCS $CHECK_CLANG; do for std in 20 26; do echo "$cxx $std"; done; done)"
    fi
    mkdir -p build/warnings build/logs/warnings
    rm -f build/warnings/*.result
    n="$(grep -c . <<< "$jobs")"
    xargs -P "$CHECK_JOBS" -L 1 "$SELF" __warn <<< "$jobs" || true
    pass="$(cat build/warnings/*.result 2>/dev/null | grep -c '^PASS$' || true)"
    [[ $(( n - pass )) -eq 0 ]] || die_lane warnings "$(( n - pass )) of $n configurations failed"
    echo "LANE warnings PASS: $n configurations"
}

# ---- api lane (S1-R4) ----
API_FLAGS="-std=c++20 -Wall -Wextra -Werror -DFENG_MATRIX_PARALLEL"

# The families: CHECK_API_FAMILIES if set, else the backticked first column of the spec's API families table.
api_families() {
    if [[ -n "${CHECK_API_FAMILIES:-}" ]]; then
        tr ' ' '\n' <<< "$CHECK_API_FAMILIES" | grep .
        return 0
    fi
    awk '/^## API families \(S1-R4\)/ { on = 1; next }
         on && /^## / { exit }
         on && /^\| *`[^`]+` *\|/ { s = $0; sub(/^\| *`/, "", s); sub(/`.*/, "", s); print s }' docs/stages/S1/spec.md
}

api_exflag() { [[ "$1" == noexc ]] && echo "-fno-exceptions" || true; }

# Internal: compile one family TU alone. __api <cxx> <exc|noexc> <family>
api_job() {
    local cxx="$1" mode="$2" fam="$3" id="$1-$2-$3"
    local obj="build/api/$1-$2/$3.o" log="build/logs/api/$id.log" status=PASS why=""
    mkdir -p "build/api/$1-$2" build/logs/api
    rm -f "$obj"
    if [[ ! -f "tests/api/$fam.cc" ]]; then
        status=FAIL; why=" (missing tests/api/$fam.cc)"; : > "$log"
    else
        # shellcheck disable=SC2046,SC2086
        "$cxx" $API_FLAGS $(api_exflag "$mode") -c "tests/api/$fam.cc" -o "$obj" > "$log" 2>&1 \
            || { status=FAIL; why=" (compile, see $log)"; }
    fi
    echo "API $fam $cxx $mode $status$why"
    echo "$status" > "build/api/$id.result"
}

# Internal: build link_a.cc + link_b.cc with one configuration, link and run. __apilink <cxx>
api_link_job() {
    local cxx="$1" d="build/api/link-$1" log="build/logs/api/link-$1.log" status=PASS why=""
    mkdir -p "$d" build/logs/api
    rm -f "$d"/*.o "$d/link"
    {
        # shellcheck disable=SC2086
        "$cxx" $API_FLAGS -O2 -pthread -c tests/api/link_a.cc -o "$d/link_a.o" &&
        "$cxx" $API_FLAGS -O2 -pthread -c tests/api/link_b.cc -o "$d/link_b.o" &&
        "$cxx" -pthread "$d/link_a.o" "$d/link_b.o" -o "$d/link"
    } > "$log" 2>&1 || { status=FAIL; why=" (build or link, see $log)"; }
    if [[ "$status" == PASS ]]; then
        local rc=0
        "$d/link" >> "$log" 2>&1 || rc=$?
        [[ $rc -eq 0 ]] || { status=FAIL; why=" (program exited $rc, see $log)"; }
    fi
    echo "LINK $cxx link_a+link_b $status$why"
    echo "$status" > "build/api/link-$cxx.result"
}

lane_api() {
    local fams cxx mode f n jobs links compilers
    fams="$(api_families)"
    n="$(grep -c . <<< "$fams" || true)"
    [[ "$n" -gt 0 ]] || die_lane api "no API families found"
    if [[ "$SMOKE" == 1 ]]; then compilers="$(first_gcc)"; else compilers="$CHECK_GCCS $CHECK_CLANG"; fi
    jobs="$(for cxx in $compilers; do for mode in exc noexc; do for f in $fams; do echo "$cxx $mode $f"; done; done; done)"
    links="$(for cxx in $compilers; do echo "$cxx"; done)"
    mkdir -p build/api build/logs/api
    rm -f build/api/*.result
    xargs -P "$CHECK_JOBS" -L 1 "$SELF" __api <<< "$jobs" || true
    xargs -P "$CHECK_JOBS" -L 1 "$SELF" __apilink <<< "$links" || true
    local total pass fail
    total=$(( $(grep -c . <<< "$jobs") + $(grep -c . <<< "$links") ))
    pass="$(cat build/api/*.result 2>/dev/null | grep -c '^PASS$' || true)"
    fail=$(( total - pass ))
    [[ "$fail" -eq 0 ]] || die_lane api "$n families, $fail failures"
    echo "LANE api PASS: $n families, 0 failures"
}

# ---- examples lane (S1-R5) ----
# Builds examples/example.cc under build/examples/ and runs it in a fresh scratch copy of images/ (the program
# rewrites ./images/*); the repo's images/ must stay untouched. --smoke is the same run (one binary).
EX_FLAGS="-std=c++20 -O2 -DFENG_MATRIX_PARALLEL"
lane_examples() {
    local cxx bin=build/examples/example log=build/logs/examples run=build/examples/run stamp
    cxx="$(first_gcc)"
    mkdir -p build/examples "$log"
    stamp="build/examples/images.stamp"
    : > "$stamp"
    local stale=1 f
    if [[ "${CHECK_REBUILD:-0}" != 1 && -x "$bin" && -f "$bin.flags" && "$(cat "$bin.flags")" == "$cxx $EX_FLAGS" ]]; then
        stale=0
        for f in matrix.hpp examples/example.cc examples/cases/*; do [[ "$f" -nt "$bin" ]] && stale=1; done
    fi
    if [[ $stale == 1 ]]; then
        rm -f "$bin" "$bin.flags"
        # shellcheck disable=SC2086
        "$cxx" $EX_FLAGS -pthread examples/example.cc -o "$bin" > "$log/build.log" 2>&1 \
            || die_lane examples "build failed, see $log/build.log"
        printf '%s' "$cxx $EX_FLAGS" > "$bin.flags"
    else
        echo "reused $bin (up to date)" > "$log/build.log"
    fi
    rm -rf "$run"
    mkdir -p "$run"
    cp -r images "$run/images"
    local rc=0
    ( cd "$run" && "$ROOT/$bin" ) > "$log/run.log" 2>&1 || rc=$?
    if [[ -n "$(find images -newer "$stamp" -print -quit)" ]]; then
        die_lane examples "the repo's images/ was modified"
    fi
    [[ $rc -eq 0 ]] || die_lane examples "example exited $rc, see $log/run.log"
    echo "LANE examples PASS: example ran in $run"
}

# ---- make lane (S1-R6) ----
# Asserts the default and FAST=1 flag sets from `make -n`, then runs one real default build into build/make/.
lane_make() {
    local log=build/logs/make dry fast before after
    mkdir -p "$log"
    dry="$(env -u BUILD_DIR -u FAST make -n -B test example 2>&1)" || die_lane make "make -n failed"
    printf '%s\n' "$dry" > "$log/dry-default.log"
    grep -q -- '-O2' <<< "$dry" || die_lane make "default flags lack -O2"
    if grep -Eq -- '-Ofast|-march=native|-ffast-math|-flto' <<< "$dry"; then
        die_lane make "default flags contain -Ofast, -march=native, -ffast-math or -flto"
    fi
    grep -q -- '-o build/test_test' <<< "$dry" || die_lane make "default output is not build/test_test"
    grep -q -- '-o build/test_example' <<< "$dry" || die_lane make "default output is not build/test_example"
    if grep -oE -- '-o [^ ]+' <<< "$dry" | grep -qv -- '^-o build/'; then
        die_lane make "an output path is outside build/"
    fi
    fast="$(env -u BUILD_DIR make -n -B FAST=1 test example 2>&1)" || die_lane make "make -n FAST=1 failed"
    printf '%s\n' "$fast" > "$log/dry-fast.log"
    grep -q -- '-Ofast' <<< "$fast" || die_lane make "FAST=1 lacks -Ofast"
    grep -q -- '-march=native' <<< "$fast" || die_lane make "FAST=1 lacks -march=native"
    before="$(ls -A)"
    local bflag=""
    [[ "${CHECK_REBUILD:-0}" == 1 ]] && bflag="-B"
    # shellcheck disable=SC2086
    env -u FAST make $bflag -j2 BUILD_DIR=build/make test example > "$log/build.log" 2>&1 \
        || die_lane make "make test example failed, see $log/build.log"
    after="$(ls -A)"
    [[ "$before" == "$after" ]] || die_lane make "make wrote at the repo root"
    [[ -x build/make/test_test && -x build/make/test_example ]] || die_lane make "binaries missing in build/make/"
    echo "LANE make PASS: -O2 default, FAST=1 -Ofast -march=native, build/make/test_test and test_example"
}

# ---- ci lane (S1-R6) ----
lane_ci() {
    local log=build/logs/ci out rc=0
    mkdir -p "$log"
    out="$(python3 - .github/workflows/ci.yml <<'EOF' 2>&1
import re, sys
try:
    import yaml
except ImportError:
    print("FAIL: PyYAML missing"); sys.exit(1)
path = sys.argv[1]
text = open(path).read()
doc = yaml.safe_load(text)
jobs = (doc or {}).get("jobs") or {}
if not jobs:
    print("FAIL: no jobs"); sys.exit(1)
if re.search(r"(gcc|g\+\+)[-: ]?12\b|version:\s*['\"]?12\b", text, re.I):
    print("FAIL: GCC 12 appears"); sys.exit(1)
gcc_jobs, clang_jobs = [], []
for name, job in jobs.items():
    blob = yaml.safe_dump(job)
    runs = " ".join(str(s.get("run", "")) for s in (job.get("steps") or []))
    if "tools/check.sh" not in runs:
        continue
    gv = [int(v) for v in re.findall(r"(?:gcc|g\+\+)[-: ](\d+)", blob, re.I)]
    cv = [int(v) for v in re.findall(r"clang(?:\+\+)?[-: ](\d+)", blob, re.I)]
    if gv and min(gv) >= 15 and not cv:
        gcc_jobs.append(f"{name} (GCC {min(gv)})")
    if cv and min(cv) >= 22:
        clang_jobs.append(f"{name} (Clang {min(cv)})")
if not gcc_jobs:
    print("FAIL: no GCC >= 15 job running tools/check.sh"); sys.exit(1)
if not clang_jobs:
    print("FAIL: no Clang >= 22 job running tools/check.sh"); sys.exit(1)
print("OK: gcc " + ", ".join(gcc_jobs) + "; clang " + ", ".join(clang_jobs))
EOF
)" || rc=$?
    printf '%s\n' "$out" > "$log/ci.log"
    [[ $rc -eq 0 ]] || die_lane ci "$(tail -n 1 <<< "$out" | sed 's/^FAIL: //')"
    echo "LANE ci PASS: ${out#OK: }"
}

# ---- docs lane (S1-R6, D-013) ----
lane_docs() {
    [[ ${#ARGS[@]} -ge 1 && -n "${ARGS[0]}" ]] || die_lane docs "usage: tools/check.sh docs <ID>"
    local id="${ARGS[0]}" f=docs/migration.md section bad n
    if [[ "$id" == S9 ]]; then
        # S9-R1: tools/const_returns.py (clang AST, D-034) counts top-level const value returns in matrix.hpp.
        # S9-R6: matrix.hpp line counts at the stage start (the parent of the commit whose subject is exactly
        # `S9: spec and tasks`; the oldest one if there are several) and at head.
        local scan cr start_commit lines_start lines_head
        scan="$(python3 tools/const_returns.py)" || die_lane docs "tools/const_returns.py failed: $(tail -n 1 <<< "$scan")"
        [[ -n "$scan" ]] && grep -v '^CONST_RETURNS ' <<< "$scan" || true
        cr="$(sed -n 's/^CONST_RETURNS \([0-9][0-9]*\)$/\1/p' <<< "$scan")"
        [[ -n "$cr" ]] || die_lane docs "tools/const_returns.py printed no CONST_RETURNS count"
        echo "CONST_RETURNS $cr"
        start_commit="$(git log --format='%H %s' --grep='^S9: spec and tasks$' \
            | awk '{ h = $1; $1 = ""; if ( substr( $0, 2 ) == "S9: spec and tasks" ) last = h } END { print last }')"
        [[ -n "$start_commit" ]] || die_lane docs "no commit with subject 'S9: spec and tasks'"
        lines_start="$(git show "$start_commit^:matrix.hpp" | wc -l)"
        lines_head="$(wc -l < matrix.hpp)"
        echo "LINES start $lines_start head $lines_head"
        [[ "$cr" -eq 0 ]] || die_lane docs "$cr top-level const value returns in matrix.hpp"
        [[ "$lines_head" -lt "$lines_start" ]] \
            || die_lane docs "matrix.hpp has $lines_head lines at head, not fewer than $lines_start at the stage start"
    fi
    [[ -f "$f" ]] || die_lane docs "$f missing"
    grep -qx "## $id" "$f" || die_lane docs "no '## $id' section in $f"
    section="$(awk -v h="## $id" '$0 == h { on = 1; next } on && /^## / { exit } on' "$f")"
    n="$(grep -c '^- ' <<< "$section" || true)"
    [[ "$n" -gt 0 ]] || die_lane docs "'## $id' has no entries"
    bad="$(grep '^- ' <<< "$section" \
        | grep -Ev '^- none$' \
        | grep -Ev '^- [^ ].*: old: .+; new: .+; finding: (F[0-9][0-9]|none)$' || true)"
    [[ -z "$bad" ]] || die_lane docs "malformed entry: $(head -n 1 <<< "$bad")"
    if [[ "$id" == S1 ]]; then
        grep -q 'tools/check.sh' ReadMe.md || die_lane docs "ReadMe.md does not name tools/check.sh"
        grep -q 'FAST=1' ReadMe.md || die_lane docs "ReadMe.md does not name FAST=1"
        grep -q 'make test example' ReadMe.md || die_lane docs "ReadMe.md does not name make test example"
    fi
    if [[ "$id" == S2 ]]; then
        grep -q 'std::abort' ReadMe.md || die_lane docs "ReadMe.md does not name std::abort"
        grep -q 'fliplr' ReadMe.md || die_lane docs "ReadMe.md does not name fliplr"
        grep -qF 'at(' ReadMe.md || die_lane docs "ReadMe.md does not name at("
    fi
    if [[ "$id" == S3 ]]; then
        grep -q 'matrix_element' ReadMe.md || die_lane docs "ReadMe.md does not name matrix_element"
        grep -q 'must not throw' ReadMe.md || die_lane docs "ReadMe.md does not say callbacks must not throw"
    fi
    if [[ "$id" == S4 ]]; then
        grep -q 'invalidat' ReadMe.md || die_lane docs "ReadMe.md does not describe iterator invalidation"
        grep -q 'make_mutable_view' ReadMe.md || die_lane docs "ReadMe.md does not name make_mutable_view"
        grep -q 'temporary' ReadMe.md || die_lane docs "ReadMe.md does not say a temporary owner is rejected"
    fi
    if [[ "$id" == S5 ]]; then
        grep -q 'save_as_npy' ReadMe.md || die_lane docs "ReadMe.md does not name save_as_npy"
        grep -q 'fortran_order' ReadMe.md || die_lane docs "ReadMe.md does not name fortran_order"
    fi
    if [[ "$id" == S6 ]]; then
        grep -q 'uniform_random_bit_generator' ReadMe.md || die_lane docs "ReadMe.md does not name uniform_random_bit_generator"
        grep -q 'ddof' ReadMe.md || die_lane docs "ReadMe.md does not name ddof"
        grep -q 'common_element_t' ReadMe.md || die_lane docs "ReadMe.md does not name common_element_t"
        grep -q 'overflow' ReadMe.md || die_lane docs "ReadMe.md does not state the overflow precondition"
        grep -q 'must not throw' ReadMe.md || die_lane docs "ReadMe.md does not say callbacks must not throw"
    fi
    if [[ "$id" == S7 ]]; then
        grep -q 'lu_factor' ReadMe.md || die_lane docs "ReadMe.md does not name lu_factor"
        grep -q 'linalg_status' ReadMe.md || die_lane docs "ReadMe.md does not name linalg_status"
        grep -q 'experimental' ReadMe.md || die_lane docs "ReadMe.md does not name the experimental routines"
    fi
    if [[ "$id" == S9 ]]; then
        grep -q 'FENG_MATRIX_PARALLEL' ReadMe.md || die_lane docs "ReadMe.md does not name FENG_MATRIX_PARALLEL"
        grep -q 'nodiscard' ReadMe.md || die_lane docs "ReadMe.md does not name nodiscard"
        grep -q 'to_mdspan' ReadMe.md || die_lane docs "ReadMe.md does not name to_mdspan"
        grep -q 'to_expected' ReadMe.md || die_lane docs "ReadMe.md does not name to_expected"
    fi
    echo "LANE docs PASS: $id, $n entries"
}

# ---- compile-fail lane (S3-R3) ----
# For each tests/compile_fail/<ID>/*.cc, compiles with the first GCC and $CHECK_CLANG (-std=c++20 -fsyntax-only
# -isystem tests, under LC_ALL=C so GCC quotes with ASCII '); a case passes when every compile fails and, per
# compiler, the diagnostic contains the case's `// expect:` text (required: a case without it fails) and, when the
# case has one, that compiler's `// expect-gcc:` (GCC compile) or `// expect-clang:` (Clang compile) text. Lines
# quoting a `// expect` comment itself do not count. A case's optional `// flags:` line appends its text to the
# compile flags of both compilers (S9-R1: `-Werror=unused-result`); a case without it compiles with $CF_FLAGS only.
# Logs under build/logs/compile-fail/<ID>/; --smoke is the same run; zero cases is a failure.
CF_FLAGS="-std=c++20 -fsyntax-only -isystem tests"
# cf_expect <file> <key>: the text after the first `// <key>:` in <file>, trailing blanks trimmed.
cf_expect() { sed -n "s|^.*// $2: *||p" "$1" | head -n 1 | sed 's/[[:space:]]*$//'; }
lane_compile_fail() {
    [[ ${#ARGS[@]} -ge 1 && -n "${ARGS[0]}" ]] || die_lane compile-fail "usage: tools/check.sh compile-fail <ID>"
    local id="${ARGS[0]}" dir="tests/compile_fail/${ARGS[0]}" logd="build/logs/compile-fail/${ARGS[0]}"
    local f name expect tight cxx kind log n=0 bad=0 ok miss extra
    [[ -d "$dir" ]] || die_lane compile-fail "no directory $dir"
    mkdir -p "$logd"
    local cases=()
    for f in "$dir"/*.cc; do [[ -f "$f" ]] && cases+=("$f"); done
    [[ ${#cases[@]} -gt 0 ]] || die_lane compile-fail "no cases in $dir"
    for f in "${cases[@]}"; do
        n=$((n + 1))
        name="$(basename "$f" .cc)"
        expect="$(cf_expect "$f" expect)"
        if [[ -z "$expect" ]]; then
            echo "CASE $id/$name FAIL (no // expect: line)"; bad=$((bad + 1)); continue
        fi
        extra="$(cf_expect "$f" flags)"
        ok=1
        for kind in gcc clang; do
            if [[ $kind == gcc ]]; then cxx="$(first_gcc)"; else cxx="$CHECK_CLANG"; fi
            tight="$(cf_expect "$f" "expect-$kind")"
            log="$logd/$name.$cxx.log"
            # shellcheck disable=SC2086
            if LC_ALL=C "$cxx" $CF_FLAGS $extra "$f" > "$log" 2>&1; then
                echo "CASE $id/$name $cxx FAIL (compiled, see $log)"; ok=0
            else
                miss=""
                grep -vF -- '// expect' "$log" | grep -qF -- "$expect" || miss="'$expect'"
                if [[ -n "$tight" ]] && ! grep -vF -- '// expect' "$log" | grep -qF -- "$tight"; then
                    miss="${miss:+$miss and }'$tight'"
                fi
                if [[ -z "$miss" ]]; then
                    echo "CASE $id/$name $cxx PASS (expect: $expect${tight:+; expect-$kind: $tight}${extra:+; flags: $extra})"
                else
                    echo "CASE $id/$name $cxx FAIL (diagnostic lacks $miss, see $log)"; ok=0
                fi
            fi
        done
        [[ $ok == 1 ]] || bad=$((bad + 1))
    done
    [[ $bad -eq 0 ]] || die_lane compile-fail "$bad of $n cases failed"
    echo "LANE compile-fail PASS: $n cases"
}

# ---- fuzz lane (S5-R5) ----
# Builds tests/fuzz/fuzz_<t>.cc for t in npy bmp binary text with $CHECK_CLANG and libFuzzer+ASan+UBSan into
# build/fuzz/<t>/ (logs build/logs/fuzz/), copies tests/fuzz/corpus/<t>/ into a fresh build/fuzz/<t>/corpus, replays
# every file in tests/fuzz/regressions/<t>/ (if it exists), then runs the four targets in parallel for 300 s each
# (--smoke: 20 s) with a fixed seed per target and input, time, memory and leak limits. Passes only when every run
# exited 0, no crash-/leak-/timeout-/oom-/slow-unit- artifact exists and `git status --short` is unchanged.
FUZZ_FLAGS="-std=c++20 -g -O1 -fsanitize=fuzzer,address,undefined -fno-sanitize-recover=all -DFENG_MATRIX_CHECKED_ITERATORS"
FUZZ_TARGETS="npy bmp binary text"
fuzz_seed() { case "$1" in npy) echo 1001 ;; bmp) echo 1002 ;; binary) echo 1003 ;; text) echo 1004 ;; esac; }
fuzz_max_len() { [[ "$1" == text ]] && echo 16384 || echo 65536; }

# Internal: one target. __fuzz <t> <secs>; prints `TARGET <t> PASS|FAIL (...)` and writes build/fuzz/<t>.result.
fuzz_job() {
    local t="$1" secs="$2" d="build/fuzz/$1" logd="build/logs/fuzz" status=PASS why=""
    local bin="build/fuzz/$1/fuzz_$1" art="build/fuzz/$1/artifacts/"
    mkdir -p "$logd"
    rm -rf "$d"
    mkdir -p "$d/corpus" "$art"
    local opts=( -seed="$(fuzz_seed "$t")" -max_len="$(fuzz_max_len "$t")" -timeout=10 -rss_limit_mb=2048
                 -malloc_limit_mb=512 -detect_leaks=1 -artifact_prefix="$art" )
    # shellcheck disable=SC2086
    if ! "$CHECK_CLANG" $FUZZ_FLAGS -pthread "tests/fuzz/fuzz_$t.cc" -o "$bin" > "$logd/$t.build.log" 2>&1; then
        echo "TARGET $t FAIL (build, see $logd/$t.build.log)"; echo FAIL > "build/fuzz/$t.result"; return 0
    fi
    [[ -d "tests/fuzz/corpus/$t" ]] && cp -r "tests/fuzz/corpus/$t/." "$d/corpus/"
    local regs=() f nreg=0
    if [[ -d "tests/fuzz/regressions/$t" ]]; then
        for f in "tests/fuzz/regressions/$t"/*; do [[ -f "$f" ]] && regs+=("$f"); done
    fi
    nreg=${#regs[@]}
    : > "$logd/$t.replay.log"
    if [[ $nreg -gt 0 ]] && ! "$bin" "${opts[@]}" "${regs[@]}" > "$logd/$t.replay.log" 2>&1; then
        status=FAIL; why="regression replay failed, see $logd/$t.replay.log"
    fi
    local rc=0 start=$SECONDS
    if [[ $status == PASS ]]; then
        "$bin" "${opts[@]}" -max_total_time="$secs" -print_final_stats=1 "$d/corpus" > "$logd/$t.run.log" 2>&1 || rc=$?
        [[ $rc -eq 0 ]] || { status=FAIL; why="exit $rc, see $logd/$t.run.log"; }
    fi
    local found
    found="$(find "$art" -maxdepth 1 -type f \( -name 'crash-*' -o -name 'leak-*' -o -name 'timeout-*' -o -name 'oom-*' -o -name 'slow-unit-*' \) | head -n 1)"
    if [[ -n "$found" ]]; then status=FAIL; why="${why:+$why; }artifact $found"; fi
    local runs; runs="$(sed -n 's/^stat::number_of_executed_units: *//p' "$logd/$t.run.log" 2>/dev/null | tail -n 1)"
    if [[ $status == PASS ]]; then
        why="$((SECONDS - start)) s, ${runs:-?} runs, $nreg regressions, seed $(fuzz_seed "$t")"
    fi
    echo "TARGET $t $status ($why)"
    echo "$status" > "build/fuzz/$t.result"
}

lane_fuzz() {
    local secs=300 t gs_before="" gs_after="" git_ok=0 pass n
    [[ "$SMOKE" == 1 ]] && secs=20
    unset DEBUGINFOD_URLS # the symbolizer must not reach the network
    command -v "$CHECK_CLANG" > /dev/null || die_lane fuzz "$CHECK_CLANG not found"
    if gs_before="$(git status --short 2>/dev/null)"; then git_ok=1; fi
    mkdir -p build/fuzz build/logs/fuzz
    rm -f build/fuzz/*.result
    n="$(wc -w <<< "$FUZZ_TARGETS")"
    tr ' ' '\n' <<< "$FUZZ_TARGETS" | sed "s/\$/ $secs/" | xargs -P 4 -L 1 "$SELF" __fuzz || true
    pass="$(cat build/fuzz/*.result 2>/dev/null | grep -c '^PASS$' || true)"
    [[ $(( n - pass )) -eq 0 ]] || die_lane fuzz "$(( n - pass )) of $n targets failed"
    if [[ $git_ok == 1 ]]; then
        gs_after="$(git status --short 2>/dev/null || true)"
        [[ "$gs_before" == "$gs_after" ]] || die_lane fuzz "git status changed"
    fi
    echo "LANE fuzz PASS: $n targets, $secs s each"
}

# ---- tsan lane (S6-R2) ----
# Builds the suite with -fsanitize=thread -O1 -g -DFENG_MATRIX_PARALLEL into build/tsan/<cxx>/test with the first GCC and
# $CHECK_CLANG (--smoke: $CHECK_CLANG only) and runs TSAN_SPEC in each; logs in build/logs/tsan/. Fails if a build
# fails, a run matches no test, a test fails, or any log holds 'WARNING: ThreadSanitizer'.
TSAN_FLAGS="-std=c++20 -fsanitize=thread -O1 -g -DFENG_MATRIX_PARALLEL"
TSAN_SPEC="[S6-R1],[S6-R3]"
lane_tsan() {
    export TSAN_OPTIONS="${TSAN_OPTIONS:-halt_on_error=1}"
    unset DEBUGINFOD_URLS # the symbolizer must not reach the network
    local compilers cxx bin log n=0 fail=0 nomatch=0 rc
    if [[ "$SMOKE" == 1 ]]; then compilers="$CHECK_CLANG"; else compilers="$(first_gcc) $CHECK_CLANG"; fi
    mkdir -p build/tsan build/logs/tsan
    rm -f build/logs/tsan/*.log
    for cxx in $compilers; do
        n=$((n + 1))
        bin="build/tsan/$cxx/test"
        log="build/logs/tsan/$cxx.run.log"
        if ! build_suite "$cxx" "$TSAN_FLAGS" "$bin" "build/logs/tsan/$cxx.build.log"; then
            echo "CONFIG $cxx c++20 tsan FAIL (build, see build/logs/tsan/$cxx.build.log)"; fail=$((fail + 1)); continue
        fi
        rc=0
        run_suite "$bin" "$log" "$TSAN_SPEC" || rc=$?
        if grep -q 'WARNING: ThreadSanitizer' "$log"; then rc=3; fi
        case $rc in
            0) echo "CONFIG $cxx c++20 tsan PASS" ;;
            2) echo "CONFIG $cxx c++20 tsan FAIL (no test matched, see $log)"; nomatch=$((nomatch + 1)) ;;
            3) echo "CONFIG $cxx c++20 tsan FAIL (ThreadSanitizer warning, see $log)"; fail=$((fail + 1)) ;;
            *) echo "CONFIG $cxx c++20 tsan FAIL (tests, see $log)"; fail=$((fail + 1)) ;;
        esac
    done
    if grep -lq 'WARNING: ThreadSanitizer' build/logs/tsan/*.log 2>/dev/null; then
        die_lane tsan "ThreadSanitizer warning in build/logs/tsan/"
    fi
    if [[ $nomatch -ne 0 ]]; then die_lane tsan "$nomatch of $n runs matched no test"; fi
    if [[ $fail -ne 0 ]]; then die_lane tsan "$fail of $n runs failed"; fi
    echo "LANE tsan PASS: $n runs"
}

# ---- oracle lane (S7-R6) ----
# Builds the suite with the first GCC at -std=c++20 -O2 -DFENG_MATRIX_PARALLEL into build/oracle/test, runs "[oracle][<ID>]"
# (the committed fixtures under tests/fixtures/<id>/, each test printing `ORACLE <ID> FIXTURES <n>`), then, when
# /usr/include/eigen3/Eigen/Core and tests/oracle/<id>_eigen.cc both exist, builds that program under build/oracle/
# with -isystem /usr/include/eigen3 and runs it (it must exit 0 and print `EIGEN PASS`); otherwise prints
# `oracle: Eigen absent`. Logs under build/logs/oracle/; --smoke is the same run; `git status --short` must not change.
ORACLE_FLAGS="-std=c++20 -O2 -DFENG_MATRIX_PARALLEL"
ORACLE_EIGEN_FLAGS="-std=c++20 -O2 -isystem /usr/include/eigen3"
lane_oracle() {
    [[ ${#ARGS[@]} -ge 1 && -n "${ARGS[0]}" ]] || die_lane oracle "usage: tools/check.sh oracle <ID>"
    local id="${ARGS[0]}" lid logd=build/logs/oracle bin=build/oracle/test rc=0 n eigen=absent
    local gs_before="" gs_after="" git_ok=0
    lid="$(tr '[:upper:]' '[:lower:]' <<< "$id")"
    mkdir -p build/oracle "$logd"
    if gs_before="$(git status --short 2>/dev/null)"; then git_ok=1; fi
    build_suite "$(first_gcc)" "$ORACLE_FLAGS" "$bin" "$logd/build.log" || die_lane oracle "build failed, see $logd/build.log"
    run_suite "$bin" "$logd/run.log" "[oracle][$id]" || rc=$?
    [[ $rc != 2 ]] || die_lane oracle "no [oracle][$id] test ran, see $logd/run.log"
    [[ $rc == 0 ]] || die_lane oracle "fixture tests failed, see $logd/run.log"
    n="$(sed -n "s/^ORACLE $id FIXTURES \([0-9][0-9]*\)\$/\1/p" "$logd/run.log" | awk '{ s += $1 } END { print s + 0 }')"
    grep -q "^ORACLE $id FIXTURES " "$logd/run.log" || die_lane oracle "no 'ORACLE $id FIXTURES' line, see $logd/run.log"
    echo "fixtures: $n checked ($logd/run.log)"
    local src="tests/oracle/${lid}_eigen.cc" ebin="build/oracle/${lid}_eigen"
    if [[ -f /usr/include/eigen3/Eigen/Core && -f "$src" ]]; then
        rm -f "$ebin"
        # shellcheck disable=SC2086
        "$(first_gcc)" $ORACLE_EIGEN_FLAGS -pthread "$src" -o "$ebin" > "$logd/eigen.build.log" 2>&1 \
            || die_lane oracle "Eigen program build failed, see $logd/eigen.build.log"
        rc=0
        "$ebin" > "$logd/eigen.run.log" 2>&1 || rc=$?
        [[ $rc == 0 ]] || die_lane oracle "Eigen comparison exited $rc, see $logd/eigen.run.log"
        grep -q '^EIGEN PASS ' "$logd/eigen.run.log" || die_lane oracle "no EIGEN PASS line, see $logd/eigen.run.log"
        head -n 1 "$logd/eigen.run.log"
        grep '^EIGEN PASS ' "$logd/eigen.run.log"
        eigen=ran
    else
        echo "oracle: Eigen absent"
    fi
    if [[ $git_ok == 1 ]]; then
        gs_after="$(git status --short 2>/dev/null || true)"
        [[ "$gs_before" == "$gs_after" ]] || die_lane oracle "git status changed"
    fi
    echo "LANE oracle PASS: $n fixtures, eigen $eigen"
}

# ---- bench lane (S8-R4, D-031) ----
# Builds tools/bench/<workload>.cc with the first GCC at -std=c++20 -O2 -pthread (no -march=native, -Ofast or
# fast-math, D-006) into build/bench/<workload>/, logs under build/logs/bench/, runs it (passing --smoke through) and
# judges its output. Workload `fft`: passes when the `BENCH fft 256x256` median is under 1 s and both `RATIO` values
# are below 6.75 (the medians are per transform; fft.cc batches repeats so each sample lasts at least ~20 ms); the
# BENCH, RATIO and INFO lines are echoed. `git status --short` must not change.
BENCH_FLAGS="-std=c++20 -O2 -pthread"

# `bench compare` (S10-R1, S10-R3, S10-R4, D-008): the baseline is matrix.hpp at the parent of the oldest commit whose
# subject is exactly `S10: spec and tasks`, extracted with git show to build/bench/compare/base/matrix.hpp. bench/bench.cc
# is built by the first GCC with exactly $BENCH_FLAGS -DBENCH_FLAGS="..." twice: base/bench (-I the extracted header)
# and head/bench (-I the repo root); bench/ holds no matrix.hpp, so each build sees only its own header. The parallel
# pair (S10-T6) base-par/bench and head-par/bench is the same two builds with $BENCH_FLAGS -DFENG_MATRIX_PARALLEL (the
# define is part of their recorded INFO flags) against the same two headers; its workloads are named par/<w>. Logs go
# to build/logs/bench/. tools/bench_compare.py runs per workload the processes ABBA BAAB ABBA BAAB (A base, B head;
# smoke A B B A), each with 5 samples (smoke 3), pools 40 (smoke 6) samples per build (serial pinned, par/ unpinned),
# prints the INFO lines and the one table and judges bench/kept.txt; its LANE line is printed last, after the
# `git status --short` check.
lane_bench_compare() {
    local d=build/bench/compare logd=build/logs/bench gs_before="" gs_after="" git_ok=0 rc=0 start cxx line
    [[ ! -e bench/matrix.hpp ]] || die_lane bench "bench/matrix.hpp exists; the base build would include it"
    if gs_before="$(git status --short 2>/dev/null)"; then git_ok=1; fi
    start="$(git log --format='%H %s' --grep='^S10: spec and tasks$' \
        | awk '{ h = $1; $1 = ""; if ( substr( $0, 2 ) == "S10: spec and tasks" ) last = h } END { print last }')"
    [[ -n "$start" ]] || die_lane bench "no commit with subject 'S10: spec and tasks'"
    mkdir -p "$d/base" "$d/head" "$d/base-par" "$d/head-par" "$logd"
    rm -f "$d/base/bench" "$d/head/bench" "$d/base-par/bench" "$d/head-par/bench"
    git show "$start^:matrix.hpp" > "$d/base/matrix.hpp" || die_lane bench "git show $start^:matrix.hpp failed"
    echo "INFO baseline $(git rev-parse --short "$start^") (parent of $(git rev-parse --short "$start"))"
    cxx="$(first_gcc)"
    # shellcheck disable=SC2086
    "$cxx" $BENCH_FLAGS -DBENCH_FLAGS="\"$BENCH_FLAGS\"" -I "$d/base" bench/bench.cc -o "$d/base/bench" \
        > "$logd/compare.base.build.log" 2>&1 &
    local pb=$!
    # shellcheck disable=SC2086
    "$cxx" $BENCH_FLAGS -DBENCH_FLAGS="\"$BENCH_FLAGS\"" -I . bench/bench.cc -o "$d/head/bench" \
        > "$logd/compare.head.build.log" 2>&1 &
    local ph=$!
    local par_flags="$BENCH_FLAGS -DFENG_MATRIX_PARALLEL"
    # shellcheck disable=SC2086
    "$cxx" $par_flags -DBENCH_FLAGS="\"$par_flags\"" -I "$d/base" bench/bench.cc -o "$d/base-par/bench" \
        > "$logd/compare.base-par.build.log" 2>&1 &
    local pbp=$!
    # shellcheck disable=SC2086
    "$cxx" $par_flags -DBENCH_FLAGS="\"$par_flags\"" -I . bench/bench.cc -o "$d/head-par/bench" \
        > "$logd/compare.head-par.build.log" 2>&1 &
    local php=$!
    wait "$pb" || die_lane bench "base build failed, see $logd/compare.base.build.log"
    wait "$ph" || die_lane bench "head build failed, see $logd/compare.head.build.log"
    wait "$pbp" || die_lane bench "base-par build failed, see $logd/compare.base-par.build.log"
    wait "$php" || die_lane bench "head-par build failed, see $logd/compare.head-par.build.log"
    echo "INFO build $cxx $BENCH_FLAGS"
    echo "INFO build-par $cxx $par_flags"
    local smoke=()
    [[ "$SMOKE" == 1 ]] && smoke=(--smoke)
    rm -f "$d/rc"
    { python3 tools/bench_compare.py --base "$d/base/bench" --head "$d/head/bench" \
        --base-par "$d/base-par/bench" --head-par "$d/head-par/bench" --kept bench/kept.txt \
        --log "$logd/compare.run.log" "${smoke[@]}" 2>&1 || echo "$?" > "$d/rc"; } \
        | tee "$logd/compare.log" | { grep --line-buffered -v '^LANE ' || true; }
    [[ -f "$d/rc" ]] && rc="$(cat "$d/rc")"
    line="$(grep '^LANE bench ' "$logd/compare.log" | tail -n 1 || true)"
    if [[ $git_ok == 1 ]]; then
        gs_after="$(git status --short 2>/dev/null || true)"
        [[ "$gs_before" == "$gs_after" ]] || die_lane bench "git status changed"
    fi
    [[ -n "$line" ]] || die_lane bench "tools/bench_compare.py exited $rc without a LANE line, see $logd/compare.log"
    echo "$line"
    [[ $rc -eq 0 && "$line" == "LANE bench PASS"* ]] || exit 1
}

lane_bench() {
    [[ ${#ARGS[@]} -ge 1 && -n "${ARGS[0]}" ]] || die_lane bench "usage: tools/check.sh bench <workload>|compare [--smoke]"
    if [[ "${ARGS[0]}" == compare ]]; then lane_bench_compare; return; fi
    local w="${ARGS[0]}" logd=build/logs/bench gs_before="" gs_after="" git_ok=0 rc=0 out
    local src="tools/bench/${ARGS[0]}.cc" bin="build/bench/${ARGS[0]}/${ARGS[0]}"
    [[ "$w" =~ ^[A-Za-z0-9_-]+$ && -f "$src" ]] || die_lane bench "unknown workload '$w' (no tools/bench/$w.cc)"
    case "$w" in
        fft) ;;
        *) die_lane bench "unknown workload '$w' (no pass rule)" ;;
    esac
    mkdir -p "build/bench/$w" "$logd"
    if gs_before="$(git status --short 2>/dev/null)"; then git_ok=1; fi
    rm -f "$bin"
    # shellcheck disable=SC2086
    "$(first_gcc)" $BENCH_FLAGS -I . "$src" -o "$bin" > "$logd/$w.build.log" 2>&1 \
        || die_lane bench "build failed, see $logd/$w.build.log"
    local smoke=()
    [[ "$SMOKE" == 1 ]] && smoke=(--smoke)
    "$bin" "${smoke[@]}" > "$logd/$w.run.log" 2>&1 || rc=$?
    [[ $rc -eq 0 ]] || die_lane bench "$w exited $rc, see $logd/$w.run.log"
    grep -E '^(BENCH|RATIO|INFO) ' "$logd/$w.run.log" || true
    if [[ $git_ok == 1 ]]; then
        gs_after="$(git status --short 2>/dev/null || true)"
        [[ "$gs_before" == "$gs_after" ]] || die_lane bench "git status changed"
    fi
    local med r1 r2
    med="$(sed -n 's/^BENCH fft 256x256 median \([0-9.eE+-]*\) s$/\1/p' "$logd/$w.run.log" | tail -n 1)"
    r1="$(sed -n 's|^RATIO 256/128 \([0-9.eE+-]*\)$|\1|p' "$logd/$w.run.log" | tail -n 1)"
    r2="$(sed -n 's|^RATIO 512/256 \([0-9.eE+-]*\)$|\1|p' "$logd/$w.run.log" | tail -n 1)"
    [[ -n "$med" && -n "$r1" && -n "$r2" ]] || die_lane bench "missing BENCH 256x256 or RATIO line, see $logd/$w.run.log"
    awk -v m="$med" 'BEGIN { exit !(m + 0 < 1) }' || die_lane bench "fft 256x256 median $med s is not under 1 s"
    awk -v a="$r1" 'BEGIN { exit !(a + 0 < 6.75) }' || die_lane bench "RATIO 256/128 $r1 is not below 6.75"
    awk -v a="$r2" 'BEGIN { exit !(a + 0 < 6.75) }' || die_lane bench "RATIO 512/256 $r2 is not below 6.75"
    echo "LANE bench PASS: fft 256x256 median $med s, ratios $r1 $r2"
}

# ---- config lane (S9-R3, D-033) ----
# Builds tests/config/tu_a.cc and tu_b.cc into build/config/<cxx>[-lto]/<pair>/ (logs build/logs/config/) with the
# first GCC and $CHECK_CLANG at -std=c++20 -O2, in four pairs per compiler: same-new (both -DFENG_MATRIX_PARALLEL)
# and old-vs-new (-DPARALLEL vs -DFENG_MATRIX_PARALLEL) must link and run with exit 0; mixed-parallel and
# mixed-checked-iterators must fail to link with a log naming feng_matrix_configuration_mismatch. The first GCC with
# -flto adds same-new and mixed-parallel. One line per pair; --smoke is the same run. Not part of `all`.
CONFIG_FLAGS="-std=c++20 -O2 -pthread"
# config_pair <cxx> <lto|plain> <pair> <link|nolink> <flags a> <flags b>; prints one PAIR line, returns 1 on failure.
config_pair() {
    local cxx="$1" lto="$2" pair="$3" want="$4" fa="$5" fb="$6" extra="" tag="$1"
    [[ "$lto" == lto ]] && { extra="-flto"; tag="$1-lto"; }
    local d="build/config/$tag/$pair" log="build/logs/config/$tag-$pair.log" rc=0 got
    mkdir -p "$d" build/logs/config
    rm -f "$d"/*.o "$d/prog"
    # shellcheck disable=SC2086
    "$cxx" $CONFIG_FLAGS $extra $fa -c tests/config/tu_a.cc -o "$d/tu_a.o" > "$log" 2>&1 || rc=1
    # shellcheck disable=SC2086
    [[ $rc == 0 ]] && { "$cxx" $CONFIG_FLAGS $extra $fb -c tests/config/tu_b.cc -o "$d/tu_b.o" >> "$log" 2>&1 || rc=1; }
    if [[ $rc != 0 ]]; then echo "PAIR $tag $pair FAIL (compile, see $log)"; return 1; fi
    # shellcheck disable=SC2086
    if "$cxx" $CONFIG_FLAGS $extra "$d/tu_a.o" "$d/tu_b.o" -o "$d/prog" >> "$log" 2>&1; then got=link; else got=nolink; fi
    if [[ "$want" == link ]]; then
        [[ $got == link ]] || { echo "PAIR $tag $pair FAIL (link failed, see $log)"; return 1; }
        "$d/prog" >> "$log" 2>&1 || rc=$?
        [[ $rc == 0 ]] || { echo "PAIR $tag $pair FAIL (program exited $rc, see $log)"; return 1; }
        echo "PAIR $tag $pair PASS (links, runs, exit 0)"
    else
        [[ $got == nolink ]] || { echo "PAIR $tag $pair FAIL (mixed configuration linked, see $log)"; return 1; }
        grep -q feng_matrix_configuration_mismatch "$log" \
            || { echo "PAIR $tag $pair FAIL (link failed without feng_matrix_configuration_mismatch, see $log)"; return 1; }
        echo "PAIR $tag $pair PASS (link fails: feng_matrix_configuration_mismatch)"
    fi
}
lane_config() {
    local cxx g n=0 bad=0
    g="$(first_gcc)"
    command -v "$g" > /dev/null || die_lane config "$g not found"
    command -v "$CHECK_CLANG" > /dev/null || die_lane config "$CHECK_CLANG not found"
    local p1="-DFENG_MATRIX_PARALLEL -DCONFIG_EXPECT_PARALLEL=1" p0="-DCONFIG_EXPECT_PARALLEL=0"
    for cxx in "$g" "$CHECK_CLANG"; do
        n=$((n + 4))
        config_pair "$cxx" plain same-new link "$p1" "-DFENG_MATRIX_PARALLEL" || bad=$((bad + 1))
        config_pair "$cxx" plain old-vs-new link "-DPARALLEL -DCONFIG_EXPECT_PARALLEL=1" "-DFENG_MATRIX_PARALLEL" || bad=$((bad + 1))
        config_pair "$cxx" plain mixed-parallel nolink "$p1" "" || bad=$((bad + 1))
        config_pair "$cxx" plain mixed-checked-iterators nolink "$p0 -DFENG_MATRIX_CHECKED_ITERATORS" "" || bad=$((bad + 1))
    done
    n=$((n + 2))
    config_pair "$g" lto same-new link "$p1" "-DFENG_MATRIX_PARALLEL" || bad=$((bad + 1))
    config_pair "$g" lto mixed-parallel nolink "$p1" "" || bad=$((bad + 1))
    [[ $bad -eq 0 ]] || die_lane config "$bad of $n pairs failed"
    echo "LANE config PASS: $n pairs"
}

# ---- all lane (S1-R5) ----
# Runs every regression lane in sequence even after a failure; full output in build/logs/all/<lane>.log.
lane_all() {
    local log=build/logs/all lane line failed=() smoke=() gs_before="" gs_after="" git_ok=0
    mkdir -p "$log"
    [[ "$SMOKE" == 1 ]] && smoke=(--smoke)
    if gs_before="$(git status --short 2>/dev/null)"; then git_ok=1; fi
    for lane in gcc clang sanitize warnings api examples make ci "docs S1"; do
        local name="${lane%% *}" rc=0
        # shellcheck disable=SC2086
        case "$name" in
            make|ci|docs) "$SELF" $lane > "$log/$name.log" 2>&1 || rc=$? ;;
            *) "$SELF" $lane "${smoke[@]}" > "$log/$name.log" 2>&1 || rc=$? ;;
        esac
        line="$(grep '^LANE ' "$log/$name.log" | tail -n 1)"
        [[ -n "$line" ]] || line="LANE $name FAIL: no LANE line (exit $rc), see $log/$name.log"
        echo "$line"
        if [[ $rc -ne 0 || "$line" != "LANE $name PASS"* ]]; then failed+=("$name"); fi
    done
    if [[ $git_ok == 1 ]]; then
        gs_after="$(git status --short 2>/dev/null || true)"
        [[ "$gs_before" == "$gs_after" ]] || failed+=("git-status")
    fi
    [[ ${#failed[@]} -eq 0 ]] || die_lane all "${failed[*]}"
    echo "LANE all PASS"
}

# ---- dispatch ----
if [[ "${1:-}" == __config ]]; then shift; config_job "$@"; exit 0; fi
if [[ "${1:-}" == __sanbuild ]]; then shift; san_build_job "$@"; exit 0; fi
if [[ "${1:-}" == __warn ]]; then shift; warn_job "$@"; exit 0; fi
if [[ "${1:-}" == __api ]]; then shift; api_job "$@"; exit 0; fi
if [[ "${1:-}" == __apilink ]]; then shift; api_link_job "$@"; exit 0; fi
if [[ "${1:-}" == __fuzz ]]; then shift; fuzz_job "$@"; exit 0; fi

LANE="${1:-}"
[[ -n "$LANE" ]] || { echo "usage: tools/check.sh <lane> [args] [--smoke]"; echo "LANE none FAIL: no lane given"; exit 2; }
shift
SMOKE=0
ARGS=()
for a in "$@"; do
    if [[ "$a" == --smoke ]]; then SMOKE=1; else ARGS+=("$a"); fi
done

case "$LANE" in
    gcc)      lane_gcc ;;
    clang)    lane_clang ;;
    sanitize) lane_sanitize ;;
    tagged)   lane_tagged ;;
    warnings) lane_warnings ;;
    api)      lane_api ;;
    examples) lane_examples ;;
    make)     lane_make ;;
    ci)       lane_ci ;;
    docs)     lane_docs ;;
    compile-fail) lane_compile_fail ;;
    fuzz)     lane_fuzz ;;
    tsan)     lane_tsan ;;
    oracle)   lane_oracle ;;
    bench)    lane_bench ;;
    config)   lane_config ;;
    all)      lane_all ;;
    *) die_lane "$LANE" "unknown lane" ;;
esac
