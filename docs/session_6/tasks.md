# Session 6 — Tasks

Each task: goal, files, TDD step, check command, done criterion. One commit
per task (baseline commit exists, so `git diff` against the previous commit
is the audit). Tasks are ordered so every check runs green before the next
task starts.

## T1 — fft.hpp suite case RED (oracle + scenarios, pre-fix)

- Goal: write `tests/cases/fft.hpp` (embedded corrected-naive oracle + the
  scenarios in spec `fft-tests.md`) and register it in `tests/test.cc`
  (between `fabs` and `flip`). Against the pre-fix header the new case must
  FAIL (fast-path differential, E16, E17, normalization all fail pre-fix).
- Files: `tests/cases/fft.hpp` (new), `tests/test.cc`.
- TDD: red first — the suite fails on the new case; the pre-fix failures are
  recorded as evidence (they are the P1/C13 baseline in the suite's own
  vocabulary).
- Check: `make test` builds; `./test_test -t "fft"` (or the case name) fails
  with the expected failures; all other cases still pass.
- Done: red is recorded in `.work/evidence/s6_t1_red.log` with the exact
  failing assertions (differential, E16, E17, normalization-once).
- Note: the even-dim swap-of-halves regression pin and the 0×0 guard case
  PASS pre-fix (they pin unchanged behavior) — that split (some red, some
  green) is expected and recorded.

## T2 — `fft`/`ifft` rewrite (P1)

- Goal: implement `fft_private` (`is_power_of_two`, `twiddle_table`,
  `radix2_fft_1d`, `naive_dft`), the whole-matrix path selection in `fft`,
  the `ifft` single `1/(R·C)` normalization, and the corrected naive data
  index (F1). Delete the empty `ifft_private` namespace.
- Files: `matrix.hpp` (FFT region only; A2 untouched in this task).
- TDD: T1's red scenarios turn green.
- Check: `./test_test` fully green (75 cases); probe build
  `g++ -std=c++20 -DPARALLEL -O1 -o .work/probe_s6 .work/probes/E16_E17.cc && .work/probe_s6`
  → `E16_E17 PASS`; benchmark: 512×512 ×100 serial pre-fix (recorded from a
  scratch copy of the pre-fix header, `.work/evidence/s6_bench_prefix.log`)
  vs post-fix (`.work/evidence/s6_bench_postfix.log`) shows the 10–100× gate
  (PRD goal 3).
- Done: suite green, probe PASS, benchmark logged.
- Commit: "session 6: P1 fft/ifft — separable radix-2 + corrected naive
  fallback + ifft normalization".

## T3 — `fftshift`/`ifftshift` roll (C13)

- Goal: replace both remaps with the shared `shift_roll` (per-axis roll
  `(n+1)/2`); keep the fused transform+shape.
- Files: `matrix.hpp` (fftshift/ifftshift region only).
- TDD: the E17/odd-n scenarios in `fft.hpp` turn green (they were red in T1).
- Check: `./test_test` fully green; the even-dim pin stays green (bit-identity
  invariant); probe `E16_E17` still PASS.
- Done: all shift scenarios green; pre-fix n=5 remap `(3,4,2,0,1)` → post-fix
  `(2,3,4,0,1)` (the C13 fix, visible in the suite).
- Commit: "session 6: C13 fftshift/ifftshift — NumPy-pinned (n+1)/2 roll".

## T4 — A2 deletions + consumers

- Goal: delete `random` (2 overloads), `random_like`, `pinverse`,
  `svd_inverse`, free `det(m)`; move the SVD core to
  `matrix_details::pinv_core` (end of the second `matrix_details` block,
  before line 4156; ADL comment); `pinv` delegates; `rand_like`/`randn_like`
  re-point to `rand< T, A >( row, col )`. Update
  `examples/cases/0013_prefix.hpp:3` and `ReadMe.md:1277` to
  `feng::rand<double>( 127, 127 )`.
- Files: `matrix.hpp` (A2 regions only), `examples/cases/0013_prefix.hpp`,
  `ReadMe.md` (one line — the rest of the ReadMe is T6).
- TDD: the E18 compile probe (write `.work/probes/E18_a2.cc` +
  `.work/probes/E18_ok.cc` first; the negative probe must fail to compile).
- Check: E18 negative compile fails naming a retired identifier; E18
  positive compiles + runs; `make test` green; `make example` + run green
  (0013 renders); grep gates: `grep -c 'pinverse\|svd_inverse' matrix.hpp`
  = 0; `grep -n 'random\b' matrix.hpp` returns exactly one line — line 29,
  `#include <random>` (F3: the A2 invariant keeps the include, which the
  literal acceptance count also matches; the gate is over identifier use,
  and the match list is recorded, not just the count).
- Done: greps verified by listing the matches (not just the count); suite +
  example green.
- Commit: "session 6: A2 retire legacy aliases, matrix_details::pinv_core".

## T5 — F2 canonicalization of pinv.hpp

- Goal: minimal one-file refinement (reported per §1.2): drop the
  "pinv == pinverse" scenario, rename TEST_CASE to "Matrix pinv", fix the
  header comments to reference `matrix_details::pinv_core`.
- Files: `tests/cases/pinv.hpp` only (the single refinement; logged in
  `failure_arbiter.md` F2).
- TDD: the change is exercised by `make test` (the file compiles against the
  post-T4 header only if the alias reference is gone).
- Check: `make test` green; `git diff` of `tests/cases/pinv.hpp` shows only
  the alias scenario + naming lines.
- Done: refinement is exactly the logged scope.
- Commit: "session 6: F2 pinv.hpp canonical-name update (logged refinement)".

## T6 — ReadMe sweep

- Goal: land the 11-item sweep from `design.md` §4 (S2 verbatim load_npy,
  S4 det/SVD/conv/rref/cholesky/statistics notes, R-20 disclosure, S5 NDEBUG
  verbatim, new fft section + alias-retirement table).
- Files: `ReadMe.md` only (single editor).
- TDD: n/a (docs); verification is the line-by-line checklist in `design.md`
  §4.
- Check: each sweep item grep-verified present (key phrases); the S2 and S5
  texts match their sources verbatim (diff against the quoted source
  passages); the ReadMe still renders (no broken code fences — count ```
  parity).
- Done: all 11 items checked and marked in a sweep checklist
  (`.work/evidence/s6_readme_sweep.md`).
- Commit: "session 6: ReadMe sweep (S2–S5 deltas + FFT + aliases + NDEBUG)".

## T7 — Closeout: evidence, eval seeds, evidence map, handoff

- Goal: full verification + documentation closeout.
- Files: `docs/evidence_map.md` (C-08 correction, A2/A3 notes, E16/E17 rows,
  A3 → "S6 live"), `docs/eval_seed_cases.md` (E16/E17/E18 live),
  `.work/handoff_session_6.md`, `.work/evidence/s6_final.log`.
- Check (full set): `make test` (75 cases, all green); `./test_test`
  assertion count; `make example` + run; E16/E17 probe PASS; E18 probes as
  spec'd; grep gates (A2/A3, listed matches); benchmark logs present;
  `git log` shows one commit per task; `git diff` of the whole session
  audited against the blast radius (+ the logged F2 refinement).
- Done: every `exit_criteria` line of the contract verified with evidence;
  handoff written per the template.
- Commit: "session 6: closeout (evidence map, eval seeds, handoff)".
