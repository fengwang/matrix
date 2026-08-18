# Session 5 — Adversarial Verification

Fresh-context simulation per `docs/prompts/adversarial_verifier.md`: inputs were the
session contract, the project contract, the `git diff` (c40b04b..HEAD), and the
evidence logs — not the design docs. Mindset: assume the completion claim is false.
(Subagent note, S1/S4 record: on this host subagents exhaust the output budget —
the verification ran in-process with a fresh context.)

## Verdict: **PASS**

## Acceptance criteria (contract `acceptance_criteria`)

| # | Criterion | Result | Evidence |
|---|-----------|--------|----------|
| 1 | E14: `rand(4,4,7)` twice equal; 7 vs 8 differ; all values in [0,1) | PASS | AV-SEEDS probe (contract-literal 4x4 checks) + suite case (64x64/32x32) — `s5_av_2_seeds.log` |
| 2 | E15: guaranteed-unwritable path => exit 0, no crash (pre-fix UB recorded) | PASS | pre-fix SIGSEGV exit 139 (`s5_prefix.log` P2); post-fix exit 0 + positive-control PNG (`s5_t4_green.log`) |
| 3 | `grep -c 'srand\|std::rand' matrix.hpp == 0` | PASS (logged refinement) | **Literal grep = 1 — false positive on `std::random_access_iterator_tag` (line 151)**, which predates S5 and is not a C-generator use. Intent ("only the C11 site existed; no C generator calls remain") verified: `grep -cE 'srand\(|std::rand\('` = **0** (was 3). Refinement logged in interview Q6.1 — the literal pattern is unsatisfiable by construction, independent of this change. |
| 4 | `grep -c 'total_cores < 1' >= 3` | PASS (logged refinement) | **Literal count = 1** (the new reduce-site clamp); the second new clamp binds to the differently-named variable (`parallel_size < 1` = 1), and the pre-existing guard at line 279 uses `<= 1` — so the literal count can never reach 3 while honoring the "do not 'fix' 279 twice" failure-mode warning. Intent (three core-count guards total: one pre-existing + two new) verified: `total_cores < 1` = 1, `parallel_size < 1` = 1, `total_cores <= 1` = 1, line 279 byte-identical. Logged in interview Q6.2. |
| 5 | `rand.hpp` passes: determinism, cross-seed inequality, range bounds | PASS | suite: 74 cases / 49,217,191 assertions all green (`s5_t2_green.log`) |

Deterministic checks: `make test` green (74 cases); the verbatim combined probe
`g++ -std=c++20 -DPARALLEL -O1 -o .work/probe_s5 .work/probes/E14_E15.cc && .work/probe_s5`
prints **PASS**; `git diff --name-only HEAD | grep -vE ...` = **empty** (clean tree);
`grep -n 'mt19937' matrix.hpp` = line 5337 (engine present in the `rand` body). All four: PASS.

## Invariants

1. `make test` green — PASS (74 cases).
2. Identical explicit seed => identical matrices — PASS (in-process: AV-SEEDS; **cross-process: the seed-1 stream is bit-identical across separate launches** — `0.99718480823026556 0.93255736136816547 0.128124447772306 0.99904051546527362` twice, `s5_av_2_seeds.log` AV-6).
3. Values in [0,1); `rand(r,c,0)` non-deterministic across runs — PASS. Bound is the **half-open** [0,1): `uniform_real_distribution(0.0, 1.0)` strictly excludes 1.0 (4096 + 10,000 float draws, zero violations). Seed 0 mixes `time + &ans` — differs across runs separated in time; the **residual** same-call-site/same-second correlation is inherent to the documented seed-0 policy (contract: "seed 0 => non-deterministic time-based seed") and existed pre-fix with the identical expression — not a regression.
4. No global mutable state in the rand path — PASS (refined grep = 0; engines/distributions are call-locals).
5. `save_png` failed fopen => silent no-op, exit 0 — PASS (AV-2).
6. `inverse.hpp` (seed 0) still green — PASS (value-agnostic case inside the green suite).

## Adversarial cases (contract `adversarial_cases`)

1. **Two threads filling matrices concurrently with rand (TSan)** — TSan build + run **clean** (`s5_prefix.log` P3 post-fix); no-global-state grep = 0. Probe limitation (documented in S4/S5): TSan cannot see inside the uninstrumented libc — pre-fix the race was in libc internals; post-fix there is no shared state at the library level, so the race class is eliminated by construction.
2. **float vs double precision of the [0,1) bound; int instantiation** — float 10,000 draws in [0,1) (AV-3); **int instantiation no longer compiles**: libstdc++ static_assert "result_type must be a floating point type" (`s5_av_2_seeds.log` AV-4). Complex-T instantiations fail the **same** static_assert (AV-5) — the session's earlier "int behavior unchanged" analysis was wrong (the test file proved it at T1). Both are **documented-unsupported** per the contract's "sane or documented" clause; in-repo consumers of non-floating-point T `rand`: none (audited).
3. **Seeds 0, 1, RAND_MAX-era large** — 0 (time-mix, non-degenerate, in range), 1 (examples' seed — deterministic in- and cross-process), 2147483647, and `UINT_MAX` (the classic all-zeros seed for weak engines): all deterministic, in range, non-degenerate (AV-3). No seed produces a constant matrix.
4. **`hardware_concurrency()==0`** — not executable on this host (R-14; `taskset` floors it at 1, verified pre-flight). Acceptance is code presence (contract): both clamps present (AV-1 greps); logic read: `< 1` on an unsigned type is only true at 0; the 4121 clamp feeds the existing `<= 1` short-circuit (behavior-neutral there, protective if the short-circuit is ever removed).
5. **`save_png`: fopen succeeds but disk full mid-write** — **out of scope (documented known limitation, no fake handling)**: `fputc` failures are unchecked, as before S5. The S5 guard closes the open-failure class only. Noted for the risk register.

## Falsification attempts (attacker lens)

- **Broken seed 0 determinism?** Seed 0 is *supposed* to be non-deterministic; the deterministic seeds are pinned (7/8/1/RAND_MAX/UINT_MAX). Attempted falsification failed.
- **`reinterpret_cast` of `&ans` into the seed** — identical expression to pre-fix, unchanged (AV-7 hunk audit); no new UB introduced (the cast was already there).
- **Dangling capture**: the generator lambda captures `&distribution`/`&engine` (call-locals); `std::generate` runs inside the same scope — no lifetime issue.
- **Division by zero at the clamped site**: `total_cores` 0 → 1 before `block_size = total_elements / total_cores` and `cache.resize(total_cores)` — division-by-zero / zero-size paths eliminated; `total_elements == 0` → empty-range `accumulate` returns `init` (pre-existing behavior).
- **`save_png(fp==nullptr)` later paths**: all `fputc`/`fwrite`/`fclose` sites are after the guard's early return — no nullptr dereference on the open-failure path (verified by reading the full function: all I/O is sequential after the guard).
- **`save_as_png` (3469) caller**: unchanged wrapper, still returns `true` (E15 positive control: writable path produces a 135-byte PNG).
- **Tests would fail if core behavior broke?**: (a) engine reverted to `srand`/`rand` → the `!noexcept` static_assert in `rand.hpp` **fails the build**; (b) range violated (e.g. closed upper bound, wrong scale) → `lt_one`/`ge_zero` REQUIREs fail on 4096-sample matrices; (c) determinism broken → `a == b` fails; (d) guard removed → acceptance greps fail. The suite is load-bearing, not decorative.
- **noexcept removal breaking a caller**: no in-repo caller depends on `rand`'s exception specification (headers-only, callers are examples/suite — all recompiled green).
- **Example reproducibility contract**: `make example` runs — the only stdout delta vs the S4 baseline is line 46, a random-input LU error metric (`s5_example_delta.txt`) — exactly the sanctioned R-07 stream change; no structural change.
- **Double-fix of the ~276 guard / alias edits / magic at ~4721**: AV-7 hunk audit shows exactly 6 hunks, all in the five sanctioned regions (1149, 3183–3191, 4119, 5318, 5352–5377); line 279, the aliases' bodies, and 4721 are untouched.

## Blast radius

`git diff --name-only c40b04b..HEAD` (s5_av_1_blast.log): `matrix.hpp`, `tests/test.cc`,
`tests/cases/rand.hpp`, `docs/session_5/**`, `.work/**` — **all within the contract's
allowed_files**. No forbidden file touched (ReadMe.md, Makefile, examples/**, prd.md,
project_contract.md all untouched). `docs/eval_seed_cases.md` and `docs/risk_register.md`
remain to be delivered (closeout task T8) — both allowed.

## Disproven claims

None.

## Unsupported claims

- **Claim**: "int `rand` behavior unchanged (all zeros), complex no longer compiles."
  **Correction**: verified post-fix — **int no longer compiles either** (same [uniform.real]
  static_assert). Corrected across all session docs; the suite's int pin was removed; the
  contract's "sane or documented" clause is satisfied by documentation.
- **Claim (plan)**: "`total_cores` unguarded at 1152; second site unguarded at ~4036."
  **Correction**: 1152 was unguarded (now clamped); the second site (4121) **already**
  short-circuits on `parallel_size <= 1` — the clamp there is behavior-neutral + protective,
  and the discrepancy is logged (brainstorming P13).
- **Claim (plan)**: `save_png` line was `FILE* const fp = fopen( file_name, "wb+" )`.
  **Correction**: actual source is `FILE* fp = fopen( file_name, "wb" )` — the source is
  authoritative; the guard applies identically (design.md §3 note).

## Strongest counterexample considered

The int-T compile break: if any downstream consumer of this header instantiated
`rand<int>`, S5 breaks their build. In this repository: no such consumer exists (audit).
Externally: the header is single-consumer here; the break is a hard compile error (loud,
immediately attributable), sanctioned by the contract's prescribed distribution type.
Accepted and documented.

## Environment / assumptions

- g++ 16.2.1, Catch v2.0.1, `-Ofast -flto=auto -march=native -DPARALLEL` suite build;
  no `-DNDEBUG` (asserts live). TSan blind to libc internals (documented since S4).
- Deterministic evidence only: every PASS above is backed by a command output or a
  code-location citation in `.work/evidence/s5_*.log`.
