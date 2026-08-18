# Session 1 — Plan (micro-task, TDD-style)

Input: `tasks.md`. Context: `design.md`, `specs/`. Baseline commit: `83ea78d` (branch
`phase-1/session-1`). Every commit happens at a clean checkpoint (all checks green).

### Task 1 — Pre-flight

1. `git status --short` → empty. `git log --oneline -1` → `83ea78d`.
2. `make test` (background) → capture to `.work/evidence/baseline_make_test.log`; expect
   `All tests passed (57 test cases, 49.2M assertions)` (pre-existing green baseline; count
   taken from the log, not assumed).
3. Re-anchor:
   `grep -n "the_cols_to_copy\|the_rows_to_copy" matrix.hpp` → `the_rows_to_copy` at 3531–3532
   (bug); `grep -n "swap_ranges" matrix.hpp` → 4479 (bug, C2).
4. Write `.work/probes/E01_E02.cc` (cases e01a/e01b/e01c/e02a/e02b/e02c; `all` default).
5. Compile:
   `g++ -std=c++20 -DNDEBUG -DPARALLEL -fsanitize=address -O1 -o .work/probe_s1 .work/probes/E01_E02.cc`
6. Pre-fix runs (per case, isolated — ASan aborts kill the process):
   `for c in e01a e01b e02a e02b e02c; do .work/probe_s1 $c; done` → expected pre-fix:
   - `e01a`: ASan `heap-buffer-overflow` WRITE (5×5→5×3)
   - `e01b`: `FAIL e01b` content corruption (row 3 = `23,0` instead of `0,0`)
   - `e02a`: ASan `heap-buffer-overflow` READ (3×5 flipdim 2)
   - `e02b`: `FAIL e02b` (4×4 scramble, no ASan)
   - `e02c`: `PASS e02c` (dim==1 correct)
   Save stdout/stderr + exit codes to `.work/evidence/prefix_<case>.out/.err`.
7. **If any expected reproduction does NOT appear** → classify per `failure_arbiter.md` (likely
   TEST_BUG in the probe or ENVIRONMENT) and stop before fixing.
8. Write phase docs (`brainstorming/proposal/design/specs/tasks/plan/execution_contract`).
9. Commit checkpoint: `S1 pre-flight: phase docs, E01/E02 probes, pre-fix evidence`.

### Task 2 — C1 fix + shrink test

1. Failing test first (TDD): write `tests/cases/shrink_to_size.hpp` NOW (bug still present);
   register in `tests/test.cc`; compile — the 3×10→5×2 and 10×10→1×1 content assertions should
   FAIL pre-fix (compile errors/OOB aside; run under the ASan build if needed to demonstrate the
   failure mode). Record output to `.work/evidence/prefix_test_shrink.log`.
2. Apply the fix (one token, `matrix.hpp:3532`):
   ```diff
   -                std::copy( zen.row_begin( r ), zen.row_begin( r ) + the_rows_to_copy, other.row_begin( r ) );
   +                std::copy( zen.row_begin( r ), zen.row_begin( r ) + the_cols_to_copy, other.row_begin( r ) );
   ```
3. `make test` → green incl. new `shrink_to_size` case.
4. ASan: `.work/probe_s1 e01` → `PASS e01a/e01b/e01c` + `PASS E01`, exit 0, no ASan report.
5. Commit checkpoint: `S1 C1: shrink_to_size copies the_cols_to_size per row + regression case`.

### Task 3 — C2 fix + flip test

1. Failing test first (TDD): write `tests/cases/flip.hpp` (C2 bug still present); register;
   the 3×5 dim2 case must fail pre-fix (ASan abort or content FAIL — record).
2. Apply the fix (one identifier, `matrix.hpp:4479`):
   ```diff
   -                std::swap_ranges( ans.col_begin( index_left ), ans.col_end( index_left ), ans.row_begin( index_right ) );
   +                std::swap_ranges( ans.col_begin( index_left ), ans.col_end( index_left ), ans.col_begin( index_right ) );
   ```
3. `make test` → green incl. new `flip` case.
4. ASan: `.work/probe_s1 e02` → `PASS e02a/e02b/e02c` + `PASS E02`, exit 0, no ASan report.
5. Scope check: `git diff 83ea78d -- matrix.hpp` shows exactly the two sanctioned lines;
   4491–4499 (`fliplr`/`flipud`) and the dim==1 branch unchanged.
6. Commit checkpoint: `S1 C2: flipdim dim==2 swaps col against col + regression case`.

### Task 4 — branch_and_compare: independent test-writer + bug-restoration

1. Dispatch independent test-writer (fresh context; input = contract clauses + in-code documented
   comment + public API signatures only; forbidden: the fixed diff, my tests). It re-derives
   expected contents for E01/E02 and writes `.work/independent/probe_s1_independent.cc`.
2. Compile its probe with the ASan flags; run → must pass against the fixed tree.
   Record output to `.work/evidence/independent_probe.log`. Disagreement → failure arbiter.
3. Bug-restoration (C1): `sed` the fix line back to the buggy line → `make test` → the
   `shrink_to_size` case must FAIL (or ASan abort under the probe build; record which) →
   restore fix → green. Output → `.work/evidence/bug_restore_c1.log`.
4. Bug-restoration (C2): same for the flip line → `flip` case must FAIL → restore → green.
   Output → `.work/evidence/bug_restore_c2.log`.
5. Commit checkpoint (if any evidence files): `S1 evidence: independent test-writer + bug-restoration`.

### Task 5 — Full checks + diff audit

1. `make test` (full; log to `.work/evidence/final_make_test.log`).
2. `g++ -std=c++20 -DNDEBUG -DPARALLEL -fsanitize=address -O1 -o .work/probe_s1 .work/probes/E01_E02.cc && .work/probe_s1`
   → `PASS E01` + `PASS E02`, exit 0.
3. `git diff --name-only 83ea78d | grep -vE '^(matrix.hpp|tests/test.cc|tests/cases/(shrink_to_size|flip)\.hpp|\.work/|docs/(eval_seed_cases|risk_register)\.md|docs/session_1/)$'`
   → empty (note: `docs/session_1/**` is in `allowed_files`).
4. `grep -n 'the_cols_to_copy' matrix.hpp` → fix line present (~3532).

### Task 6 — Sharded review (6 axes)

1. Read-only agents (one per axis) over `git diff 83ea78d..HEAD` + contract + evidence; structured
   findings (severity, file:line, clause, smallest safe fix, confidence).
2. Dedup; disposition: fix High/Critical (re-run Task 5 checks after any fix); Medium only with
   2+ reviewers or strong evidence; Nits optional.
3. Record → `docs/session_1/sharded_review.md` (copy in `.work/sharded_review.md`).
4. Commit checkpoint: `S1 review: sharded review results` (+ fixes if any).

### Task 7 — Adversarial verifier

1. Fresh-context verifier agents; inputs ONLY: session + project contracts, `git diff
   83ea78d..HEAD`, check/evidence outputs. Not the implementation conversation.
2. Verifier must specifically falsify: "would the new tests fail if either bug were present?"
   (bug-restoration logs are the cited evidence).
3. Any FAIL → failure-arbiter classification before any further fix; re-run checks; re-verify.
4. Record → `docs/session_1/adversarial_verification.md`.

### Task 8 — Close-out

1. `docs/eval_seed_cases.md`: E01/E02 `seeded` → `promoted`.
2. Handoff `.work/handoff_session_1.md` (template; compiler `g++ 16.2.1`; checks run/not run;
   decision log; doc deltas: **none**; S3 warning re dim==1 / 4491–4499 adjacency).
3. Final commit: `S1 close-out: eval seeds promoted, handoff`.
4. Present diff + evidence to the user (human decision gate); merge only on sign-off.
