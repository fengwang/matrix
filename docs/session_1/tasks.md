# Session 1 — Tasks

Ordered by dependency. Each task is verifiable (done = its check passes; check defined in
`plan.md`). Specs: `specs/`; approach: `design.md`.

## 1. Pre-flight (evidence base — done before any edit)

- [x] 1.1 Baseline: `git status` clean on `phase-1/session-1` @ `83ea78d`; run `make test` on the
      clean tree → green (evidence `.work/evidence/baseline_make_test.log`).
- [x] 1.2 Re-anchor by grep (`the_cols_to_copy` / `swap_ranges` in the shrink/flip regions) —
      confirmed bug lines at `matrix.hpp:3532` and `matrix.hpp:4479` (names authoritative, R-02).
- [x] 1.3 Write `.work/probes/E01_E02.cc` (E01/E02 acceptance scenarios + review repro shapes,
      ragged values, case selection).
- [x] 1.4 Pre-fix ASan probe runs (`e01a e01b e02a e02b e02c`) reproduce the review: C1 OOB write
      (5×5→5×3) + silent corruption (3×10→5×2); C2 OOB read (3×5) + silent corruption (4×4);
      dim==1 pin passes. Outputs in `.work/evidence/prefix_*`. A non-reproducing finding would
      stop the session (failure arbiter) — all reproduced, so proceed.

## 2. C1 fix: `shrink_to_size`

- [ ] 2.1 Apply the 1-token fix at `matrix.hpp:3532` (`the_rows_to_copy` → `the_cols_to_copy`).
- [ ] 2.2 Write `tests/cases/shrink_to_size.hpp` (content assertions per spec scenarios: 5×5→5×3;
      3×10→5×2 ragged 1..30; 1×1→4×4 grow; 10×3→5×2; 3×10→5×2; 10×10→1×1) and register it in
      `tests/test.cc`.
- [ ] 2.3 Targeted check: `make test` green (new case included); ASan probe `e01` group clean.

## 3. C2 fix: `flipdim` dim==2

- [ ] 3.1 Apply the 1-identifier fix at `matrix.hpp:4479` (third arg → `ans.col_begin( index_right )`).
- [ ] 3.2 Write `tests/cases/flip.hpp` (content assertions per spec scenarios: 3×5 dim2 (E02),
      4×4 dim2, 2×7 and 7×2 dim2, 1×5 and 5×1 dim2, 3×5 dim1 pin) and register it in
      `tests/test.cc`.
- [ ] 3.3 Targeted check: `make test` green (both new cases); ASan probe `e02` group clean;
      `fliplr`/`flipud` and dim==1 regions byte-identical to baseline (diff scope check).

## 4. Independent verification (branch_and_compare)

- [ ] 4.1 Independent test-writer (fresh-context subagent, given only the documented contract +
      API, not the implementation diff) re-derives expected contents and writes its own probe to
      `.work/independent/`; probe passes against the fixed tree.
- [ ] 4.2 Bug-restoration check: restore each original bug line in turn → the corresponding new
      test case FAILS (compile + run + record output) → restore fixes → green. Answers
      acceptance criterion 4 / verifier question with recorded evidence.

## 5. Full checks + evidence

- [ ] 5.1 `make test` full suite green (both new cases + all pre-existing).
- [ ] 5.2 ASan probe full run (no args): `PASS E01`, `PASS E02`, exit 0, no ASan report.
- [ ] 5.3 Diff audit: `git diff --name-only <baseline>` ⊆ allowed files (contract
      `deterministic_checks` regex, empty remainder); the 2 changed lines match the sanctioned
      fixes exactly.
- [ ] 5.4 Grep audit: `grep -n 'the_cols_to_copy' matrix.hpp` shows the fix line (~3532).

## 6. Review and verification (risk = high)

- [ ] 6.1 Sharded review, 6 axes (correctness, readability, security, tests, architecture,
      performance), read-only agents over baseline→HEAD diff; dedup findings; record in
      `.work/sharded_review.md` and `docs/session_1/sharded_review.md`.
- [ ] 6.2 Fix High/Critical findings only (Medium: 2+ reviewers or strong evidence), then re-run
      §5 checks.
- [ ] 6.3 Adversarial verifier (fresh context; sees contract + diff + evidence only) returns
      PASS; record in `docs/session_1/adversarial_verification.md`.

## 7. Close-out

- [ ] 7.1 `docs/eval_seed_cases.md`: E01/E02 → `promoted`.
- [ ] 7.2 Handoff `.work/handoff_session_1.md` (template; state snapshot incl. compiler version;
      decision log incl. pre-fix probe evidence + P9 refinements; checks run/not run; doc deltas:
      none; S3 warning).
- [ ] 7.3 Commit session work; present diff + evidence for the **human decision gate** (no merge
      before sign-off).
