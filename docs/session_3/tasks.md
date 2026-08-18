# Session 3 — Tasks

Ordered by dependency. Each task is verifiable (done = its check passes; checks defined in
`plan.md`). Specs: `specs/`; approach: `design.md`; contract: `docs/session_3_contract.yaml`.
Baseline: `5c8fad9` on the session branch.

## 1. Pre-flight (evidence base — done before any edit)

- [x] 1.1 Baseline: `git status` clean; `make test` + suite run green (64 cases,
      49,216,811 assertions) — `.work/evidence/baseline_*`; `make example` exit 0
      (`.work/evidence/baseline_example.log`); compiler g++ (GCC) 16.2.1.
- [x] 1.2 Re-anchor by grep (uniform +86 line drift from S2's `load_npy` hunk, `crtp_det`
      unchanged at 2048): `crtp_det` 2048; `flipdim` 4539 / `fliplr` 4577 (bug:
      `flipdim(m,1)`) / `flipud` 4582 (bug: `flipdim(m,2)`); SVD signature 5007
      (`(A, u, w, v, max_its=1000)`); `svd_inverse` 5302 (bug: calls `(a, u, v, w)`);
      `pinverse` 5312 (bug: `v * w * u.transpose()` without inverting `w`); `pinv` 5319;
      `operator^` 5641 (bug at 5653); `lu_decomposition` 6585 (no pivoting); 1-arg tuple
      overload 6618; `lu_solver` 6627. All five bug mechanisms confirmed by direct read.
- [x] 1.3 Pre-fix probes (probe-first, P5): `.work/probes/E05_E09.cc` (E05/E06/E07/E09 + SVD
      supplement) and `.work/probes/E08.cc` (separate TU). Recorded: E05 FAIL (aliases
      swapped); E06 FAIL (`pinv(diag(1,2)) = diag(1,2)`); E07 singular FAIL (`−nan`),
      nonsingular 4×4 PASS pre-fix (51 == Bareiss 51); E09 PASS pre-fix (legacy oracle
      validates itself: ‖x−x_exact‖∞ = 4.4e-16); **E08 hard compile error** at
      `matrix.hpp:5653` (`uint_least64_t * matrix`, no matching `operator*`) —
      `.work/evidence/prefix_probes.log`, `.work/evidence/prefix_E08_compile_error.log`.
- [x] 1.4 **New finding**: SVD core invalid for wide (m<n) matrices (2×4 reconstruction
      error 6.0, `u` ~1e306; tall/square valid at 2.2e-16) → contract-vs-reality divergence
      on the 2×4 pinv adversarial case. Asked to the user live (interview-me Q1); answered
      **(a) narrow**: 4×2 + 3×3 rank-deficient pinv cases; wide gap logged (S6 candidate).
- [x] 1.5 Callers audited: `lu_decomposition`/`lu_solver` — only
      `examples/cases/0019_lu_decomposition.hpp` (1-arg overloads) + in-file `lu_solver`;
      no test callers. `tests/test.cc` include list positions for the five new case files
      verified. Examples touching changed numerics identified: 0005 (det), 0019 (LU),
      0021 (SVD — tuple order `(u,w,v)` kept).
- [x] 1.6 Phase docs written (brainstorming, proposal, design, 5 specs, tasks, plan,
      execution contract); pre-flight checkpoint committed.

## 2. Task 1 — C3: flip aliases (TDD)

- [ ] 2.1 Add `tests/cases/flip_aliases.hpp` (5 cases, tag `[flip]`: E05 2×3; 3×3 ragged;
      1×3 row; 3×1 column; 1×1) + register in `tests/test.cc` (after `flip.hpp`).
- [ ] 2.2 TDD red: `make test` → `flip_aliases` cases FAIL pre-fix (aliases return the other
      flip); record output.
- [ ] 2.3 Fix: swap the dimension arguments in `fliplr`/`flipud` bodies (2 lines, ~4580/4585).
- [ ] 2.4 Targeted green: `./test_test "[flip]"` (flip + flip_aliases) → all pass; E05 probe
      green; commit.

## 3. Task 2 — C4 + R1: single pinv core (TDD)

- [ ] 3.1 Add `tests/cases/pinv.hpp` (tag `[pinv]`: diag(1,2)→diag(1,0.5); pinv==pinverse
      exact; 4×2 full-column-rank MP; 4×2 rank-1 MP (D4 case); 3×3 diag(1,1,0)→itself;
      zeros→zeros; 1×1 boundary 1e-10 / 2e-10) + register in `tests/test.cc`.
- [ ] 3.2 TDD red: `make test` → pinv cases FAIL pre-fix (no inversion); record output.
- [ ] 3.3 Fix `svd_inverse` body (correct arg order; invert `w`; return `v * w * u.transpose()`)
      and `pinverse` body (one-line delegation).
- [ ] 3.4 Targeted green: `./test_test "[pinv]"` → all pass; E06 probe green (incl. boundary
      and supplement); commit.

## 4. Task 3 — P2: LU partial pivoting (TDD)

- [ ] 4.1 Add `tests/cases/lu_pivoting.hpp` **stage A** (tag `[lu_pivoting]`, 3-arg-only
      cases so the file compiles pre-fix: E09 6×6 invariance vs in-file legacy oracle +
      exact x; 3×3 zero-first-pivot rescue; singular 2×2 nullopt pin) + register in
      `tests/test.cc`.
- [ ] 4.2 TDD red: `make test` → the rescue case FAILs pre-fix (nullopt — zero first pivot,
      no swap); E09 passes pre-fix trivially (library == legacy algorithm); record output.
- [ ] 4.3 Fix: new 5-arg `lu_decomposition` primary (pivoting, working copy, L-row swap,
      perm, sign) + 3-arg delegate + `lu_solver` P-to-b. Then **stage B**: append the 5-arg
      API-surface cases (3×3 perm/sign/PA==LU; `[[0,1],[1,0]]` perm/sign; identity no-op) —
      green-on-first-run by construction (the API did not exist pre-fix; no red state
      possible — documented).
- [ ] 4.4 Targeted green: `./test_test "[lu_pivoting]"` → all pass; E09 probe green
      (invariance < 1e-9 vs legacy oracle); `make example` exit 0 (0019 solution unchanged);
      commit.

## 5. Task 4 — C5: det via pivoted LU (TDD)

- [ ] 5.1 Add `tests/cases/det.hpp` (tag `[det]`: 1×1 {0}/{5}; 2×2 odd swap −1; 2×2 → 3;
      singular block == 0 exactly; 4×4 vs in-file Bareiss (51); diag(1,1e-14) ≈ 1e-14 ≠ 0;
      3×3 two-swap → 16) + register.
- [ ] 5.2 TDD red: `make test` → singular-block case FAILs pre-fix (`−nan != 0`); record
      output.
- [ ] 5.3 Fix `crtp_det::det` body (LU product; exact-zero → 0; message typo; `noexcept`
      dropped).
- [ ] 5.4 Targeted green: `./test_test "[det]"` → all pass; E07 probe green; `make example`
      exit 0 (0005 unchanged); commit.

## 6. Task 5 — C6: operator^ (TDD, documented variant)

- [ ] 6.1 Add `tests/cases/matrix_power.hpp` (tag `[matrix_power]`: 2×2 closed form
      n∈{0,1,2,3,4,5}; 3×3 n∈{3,5} vs loop oracle; 1×1 {2}^5 = 32) + register in
      `tests/test.cc`.
- [ ] 6.2 TDD red (documented variant): the new TU is a **compile failure** pre-fix
      (instantiating `m ^ 3` pulls the ill-formed odd branch — same class as the E08
      pre-fix log); record the compile log as the red.
- [ ] 6.3 Fix the odd branch (`half = lhs ^ (n >> 1); return half * half * lhs;`).
- [ ] 6.4 Targeted green: `make test` (whole suite compiles + `matrix_power` passes); E08
      probe compiles + `PASS E08`; commit.

## 7. Full checks + audit

- [ ] 7.1 Full suite: `make test` + `./test_test` → 69 cases green (64 + 5 new), log to
      `.work/evidence/final_suite_run.log`.
- [ ] 7.2 `make example` exit 0; stdout delta audited vs
      `.work/evidence/baseline_example.log` (expected: 0005 identical, 0019 MAE digits
      possibly shifted ~1e-15, 0021 identical, rest identical).
- [ ] 7.3 Diff audit vs `5c8fad9`: `git diff --name-only` ⊆ allowed set; `matrix.hpp` hunks
      confined to the five sanctioned regions; test file = `test.cc` include lines only.
- [ ] 7.4 Post-fix probe re-runs: `.work/probe_s3` (E05/E06/E07/E09 all green) +
      `.work/probe_e08` (PASS E08) → `.work/evidence/final_probes.log`.

## 8. Sharded review + fixes

- [ ] 8.1 Sharded review per `docs/prompts/sharded_review.md` (fresh context; contract +
      diff + evidence only) → `docs/session_3/sharded_review.md`.
- [ ] 8.2 Fix High/Critical findings only (Low recorded); re-run affected checks.

## 9. Adversarial verification

- [ ] 9.1 Adversarial verifier per `docs/prompts/adversarial_verifier.md` (fresh context;
      contract + diff + evidence only; attack the five fixes + the D4 narrowing) →
      `docs/session_3/adversarial_verification.md`.
- [ ] 9.2 Resolve/record verdict; re-run full checks if any fix landed.

## 10. Closeout

- [ ] 10.1 Done-condition check (contract acceptance criteria, one by one, with evidence).
- [ ] 10.2 `docs/eval_seed_cases.md`: E05–E09 → `promoted`; E06 footnote (D4 narrowing).
- [ ] 10.3 `docs/risk_register.md`: S3 watch items (wide-SVD limitation; pinv/svd_inverse
      name retirement → S6; det `noexcept` drop; LU sign/perm API surface; 0019 stdout
      delta).
- [ ] 10.4 `.work/handoff_session_3.md` (template `docs/templates/handoff.md`): decision
      log (incl. Q1 user answer), S6 doc deltas (ReadMe `det` section ~765 describes the old
      Schur behavior; pinv semantics), warnings (wide SVD; S6 rewire of `pinv` now that the
      core is correct).
- [ ] 10.5 Closeout commit.
