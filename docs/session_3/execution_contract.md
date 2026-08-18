# Session 3 — Execution Contract

Companion to `docs/session_3_contract.yaml` (the YAML is the authoritative artifact; this
file carries the operational detail the session protocol requires). Baseline: `5c8fad9`
(S2 closeout review fixes) on the session branch.

## Planned file changes (exact paths)

| File | Change | Task |
|---|---|---|
| `matrix.hpp` | `fliplr`/`flipud` bodies (2 lines, ~4577–4587) | 2 |
| `matrix.hpp` | `svd_inverse` body (~5302–5311) + `pinverse` body (~5312–5318); `pinv` untouched | 3 |
| `matrix.hpp` | new 5-arg `lu_decomposition` primary + 3-arg delegate (~6585–6615) + `lu_solver` P-to-b (~6627–6650) | 4 |
| `matrix.hpp` | `crtp_det::det` body rewrite (~2055–2085); `noexcept` dropped; message typo fixed | 5 |
| `matrix.hpp` | `operator^` odd branch (~5653, 1 line → 4 lines) | 6 |
| `tests/test.cc` | 5 include lines (alphabetical: after `cos.hpp`, after `flip.hpp`, after `lround.hpp`, after `lu_pivoting.hpp`, after `operator_equal.hpp`) | 2–6 |
| `tests/cases/flip_aliases.hpp` | new, 1 `TEST_CASE` (5 scenarios), tag `[flip]` | 2 |
| `tests/cases/pinv.hpp` | new, 1 `TEST_CASE` (7 scenarios), tag `[pinv]` | 3 |
| `tests/cases/lu_pivoting.hpp` | new, 1 `TEST_CASE` (stage A: 3 cases + legacy oracle; stage B: 3 API cases), tag `[lu_pivoting]` | 4 |
| `tests/cases/det.hpp` | new, 1 `TEST_CASE` (7 scenarios), tag `[det]` | 5 |
| `tests/cases/matrix_power.hpp` | new, 1 `TEST_CASE` (3 scenario groups), tag `[matrix_power]` | 6 |
| `.work/probes/E05_E09.cc`, `.work/probes/E08.cc` | pre-fix + post-fix runs | 1, 7 |
| `.work/evidence/**` | baseline / prefix / per-task red-green / final logs, diff audit | 1–7 |
| `.work/handoff_session_3.md` | closeout handoff | 10 |
| `docs/session_3/**` | phase docs (this set), sharded review, adversarial verification | 1, 8, 9 |
| `docs/eval_seed_cases.md` | E05–E09 rows → `promoted`; E06 footnote (D4) | 10 |
| `docs/risk_register.md` | S3 watch items | 10 |

## Allowed blast radius (from the YAML; anything else requires a stop)

**Allowed:** `matrix.hpp` (only the five sanctioned regions above — every other hunk is a
stop), `tests/test.cc` (include lines only), the five new `tests/cases/*` files,
`.work/**`, `docs/eval_seed_cases.md`, `docs/risk_register.md`, `docs/session_3/**`.

**Forbidden:** `ReadMe.md` (S6 owns; P4 — delta emitted in handoff: the `det` section
~line 765 describes the old Schur-complement behavior; `pinv` section semantics),
`Makefile` (no new dependencies; probes built ad hoc with the contract flags
`g++ -std=c++20 -DPARALLEL -O1`), `examples/**` (print-only; stdout delta audited, not
edited), `docs/prd.md` (frozen), `docs/project_contract.md` (frozen), the 1-arg
`singular_value_decomposition` return-tuple order `(u, w, v)` (example 0021 depends on it),
the `flipdim` bodies (S1 territory — verified via E02, not re-edited).

## First test to write (TDD)

File: `tests/cases/flip_aliases.hpp` (Task 2). Case:
`TEST_CASE( "Matrix fliplr/flipud", "[flip]" )` — the E05 2×3 content scenario first:
`fliplr([[1,2,3],[4,5,6]])` must equal `[[3,2,1],[6,5,4]]`. Red on the pre-fix tree
(pre-fix: `fliplr` returns the up-down flip `[[4,5,6],[1,2,3]]` → assertion fails).

## Checks per task (commands from repo root)

- Task 2 (C3): `make test 2>&1 | tail -2` · `./test_test "[flip]"` (flip + flip_aliases,
  all pass) · probe E05 section PASS.
- Task 3 (C4/R1): `make test 2>&1 | tail -2` · `./test_test "[pinv]"` (all pass) ·
  probe E06 section PASS (boundary + supplement).
- Task 4 (P2): `make test 2>&1 | tail -2` · `./test_test "[lu_pivoting]"` (all pass) ·
  probe E09 section PASS (invariance < 1e-9) · `make example 2>&1 | tail -3` exit 0.
- Task 5 (C5): `make test 2>&1 | tail -2` · `./test_test "[det]"` (all pass) ·
  probe E07 section PASS (exact 0 + 51) · `make example` exit 0 (0005 unchanged).
- Task 6 (C6): `make test 2>&1 | tail -2` (whole suite compiles) ·
  `./test_test "[matrix_power]"` (all pass) · probe E08 compiles + `PASS E08`.
- Task 7 (full): `make test && ./test_test 2>&1 | tail -3` (69 cases) ·
  `make example` exit 0 + stdout delta vs `baseline_example.log` ·
  `git diff --name-only 5c8fad9` ⊆ allowed set ·
  `git diff 5c8fad9 -- matrix.hpp | grep -c '^@@'` (hunk count ≤ 7: five regions + at most
  two adjacent-line merges) · full probe re-run (`PASS E05_E09` + `PASS E08`).
- Task 8/9 (after any review/verifier fix): re-run the Task 7 set.

## Review axes (sharded review, 5)

1. **Correctness** — specs vs diff: flip alias dimensions (2/1); pinv core = single
   `V·Σ⁺·Uᵀ` with strict `> 1e-10`; LU `P·A = L·U` with the L-row-swap propagation
   (hand-derived 3×3 check in design.md); det = `sign · ∏U_ii` with exact-zero → 0;
   `operator^` odd branch.
2. **Readability** — house style (braces, `better_assert`, `( x )` spacing), no
   unexplained magic numbers (1e-10 threshold is inherited — must carry a comment),
   comment density appropriate.
3. **Numerics** — no new epsilon (P7: only the inherited 1e-10 pinv threshold); tie-breaking
   determinism (first max wins); `−0.0` normalization in det; no overflow surface added
   (int det truncation is pre-existing class, unchanged).
4. **Tests** — content/state-asserting (not return-value-only); suite-safety (no
   assert-abort case: all `det`/`operator^`/`lu_*` cases square); legacy oracle
   byte-verbatim; MP residuals bounded; boundary 1e-10 pinned.
5. **Architecture** — diff confined to the five sanctioned regions; additive API only
   (5-arg overload); no signature changes; `pinv`/`pinverse`/`svd_inverse` names kept
   (S6 retires); `forward_substitution`/`backward_substitution`/SVD core untouched.

## Adversarial verifier brief (what the verifier sees; focus list)

Sees: `docs/session_3_contract.yaml` (+ PRD §5 rows 3/4/7); `git diff 5c8fad9 -- matrix.hpp
tests/test.cc tests/cases/`; evidence (`.work/evidence/final_*`, `prefix_*`). Does NOT see:
brainstorming/design rationale, task notes.

Focus list:
- Bug-restoration reasoning per finding: re-introducing each pre-fix line must fail a named
  case (C3 swap → flip_aliases; C4 no-inversion → pinv case 1; R1 arg order → pinv case 2;
  P2 no-pivoting → lu_pivoting rescue case; C5 Schur → det case 4 `−nan`; C6 bad branch →
  compile failure).
- Sign/perm bookkeeping: odd-swap 2×2 (−1), two-swap 3×3 (+1), perm direction
  (`perm[i]` = original row of permuted row i), PA==LU residual.
- det zero pivot: the review's singular block (exact 0, not NaN), 1×1 {0}, and
  near-singular `diag(1, 1e-14)` (nonzero, NOT thresholded).
- pinv boundary: 1×1 `[1e-10]` stays 1e-10 (strict `>`), `[2e-10]` → 5e9.
- D4 narrowing: confirm NO test/probe asserts MP conditions on an m<n pinv; the 4×2/3×3
  rank-deficient cases are in the SVD-valid domain.
- Suite safety: confirm no non-square `det`/`operator^` call in any suite case
  (assert-abort in the debug suite build).
- Example invariance: 0005 identical, 0019 MAE unchanged (~1e-15 tolerance), 0021 identical.
- Blast radius: `git diff --name-only` ⊆ allowed set; `matrix.hpp` hunks ⊆ five regions.

## Commit points

Per `tasks.md` §"Commit points" (7 commits: pre-flight, 5 tasks, closeout).
