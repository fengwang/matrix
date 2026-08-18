# Session 4 — Interview (intent extraction)

Status: complete. Per the interview-me method, the session contract set was stress-tested question
by question until every residual decision point was resolved from the dominant documents or
explicitly escalated. **Zero points required a live user answer this session** (S3 needed one;
S4's two conflicts are resolvable from the document set itself, because in both cases the
executable acceptance criteria — the contract's `adversarial_cases` and the PRD revision record —
outrank the descriptive lines in conflict with them).

HYPOTHESIS: the user wants the S4 contract executed end to end — C8/C9/C10/P2(cholesky) fixed
with TDD, content-asserting tests, deterministic probes E10–E13, sharded review, adversarial
verification, and a handoff — with zero scope creep beyond `docs/session_4_contract.yaml`.
CONFIDENCE at session start: ~93% (two contract-internal conflicts unresolved); **~97%** after
the resolutions below (both are forced by the contract's own acceptance criteria).

## Stress-test questions

### Q1 — Cholesky zero-diagonal boundary (contract-internal conflict; resolved from documents)

The contract's `in_scope` line says "return false when the diagonal step is not positive
definite (`sum < 0`…)", but the same contract's `adversarial_cases` require `1×1 {0} → false`
and "PSD-singular → false" (e.g. `[[1,1],[1,1]]` has a second diagonal step of exactly 0).
The `<0` reading fails both adversarial cases; the `<=0` reading passes all of them, and the
dominant PRD (line 11, revision record C-08/C-10 context) states "false when the diagonal step
is not positive-definite" — strict positivity **is** the definition of the PD diagonal step.

**Resolution:** guard is `sum <= 0 → return false` (strict positivity). This is a conflict
record (C-11, mirroring the E10 sample-variance trap), not a scope decision. Sources:
contract `adversarial_cases` (executable) > PRD line 11 > contract `in_scope` wording
(descriptive). A tiny positive diagonal step still returns `true` (no false-rejection beyond
the strict-PD definition).

### Q2 — The C8 invariant matrix shape (contract-internal ambiguity; resolved from documents)

The contract `in_scope` line pins "the `{1,2;1,2}` invariant (mean 1.5, variance 0.25, std
0.70711)". As written, `{1,2;1,2}` reads as a 2×2 matrix — but the std of a 2×2 `{1,2;1,2}`
with the `n−1` formula is `√(1/3) ≈ 0.57735`, **not** 0.70711. The same contract's E10 case
is explicit: `matrix<int> m{1,2,{1,2}}` — a **1×2** matrix — for which the numbers are exactly
right (mean 1.5, variance 0.25, `√(0.5/1) = √0.5 ≈ 0.70711`). The PRD's C-10 revision record
confirms the `n−1` pin.

**Resolution:** the canonical pin is the 1×2 matrix (E10). The 2×2 `{1,2;1,2}` case is added
as a *second* pin with its own hand-computed values (std `√(1/3) ≈ 0.57735`), so the
shape ambiguity can never recur: both shapes assert distinct content.

### Q3 — Which value types get the `double` return (resolved from documents)

E10 says "each is `double`" for **integer, float, and double** value types. The contract's
`in_scope` phrasing emphasizes integer matrices, but the acceptance criterion is uniform.
**Resolution:** all non-complex value types return `double`; the implementation promotes
`int`/`float` via the library's `astype<double>()` (the contract's own "divisor/promotion to
double" wording) and keeps the `double` path copy-free; the complex legacy path is preserved
verbatim (no in-repo complex callers; recorded in the handoff).

### Q4 — C10 `row > col` behavior (resolved from documents; pre-existing UB pinned, not fixed)

Relaxing the precondition to `row > 0 && col > 0` makes over-determined systems (`row > col`)
pass the debug assert and reach the algorithm. The algorithm loops `i` over `range(row)` and
dereferences `col_begin(i)` for `i >= col` — a strided read past the end. ASan evidence
(`.work/evidence/prefix_p0.log`, p3): **heap-buffer-overflow READ in
`gauss_jordan_elimination` pre-fix, in a release (NDEBUG) build** — the UB is already
reachable today (the old assert is compiled out under NDEBUG). The contract sanctions
exactly one line (the precondition) and its adversarial case requires "row>col (must behave
identically to pre-relaxation)". **Resolution:** fix = precondition only (the contract's own
smallest fix); the row>col OOB is documented as a pre-existing watch item with before/after
ASan evidence, not repaired in S4. Repairing it would change the algorithm body — outside
the sanctioned line — and the contract's own out-of-scope clause ("the fix does not change
any other gauss_jordan line").

### Q5 — Where the Cholesky guard goes (resolved from code reading)

The current body is a nested `i/j` loop: diagonal step `a[i][i] = sqrt(a[i][j] − inner_product)`
and off-diagonal `a[j][i] = sum / a[i][i]`. The only IEEE-undefined operation is the diagonal
`sqrt` of a non-positive `sum`; the off-diagonal divide-by-`a[i][i]` can only hit a zero
denominator if an earlier diagonal step was 0 — which the new guard already aborts on.
**Resolution:** one guard at the diagonal step (`sum <= 0 → false` before the `sqrt`). The
PRD's "keep the `a[i][i] == 0` check" refers to a check that **does not exist** in the current
source (a review misreading, same class as the C8 "variance returns 0" claim — pre-fix
`variance(matrix<int>)` does not compile at all); the new strict-positivity guard is strictly
stronger and subsumes it. Recorded as decision D-P2b in brainstorming.md.

### Q6 — Test placement and suite-safety (resolved from code reading)

Suite build is `-Ofast` (fast-math, R-19): new suite assertions use tolerances, finite values
only, no NaN-dependent checks; exact bit-equality pins live in the `-O1` deterministic probe
(E10_E13). Placement: extend `tests/cases/mean.hpp` (existing double case stays as regression
net); new `tests/cases/{conv_same,rref,cholesky}.hpp` registered in `tests/test.cc` at verified
alphabetical positions (`ceil < cholesky < cosh`, `cos < conv_same < det`, `rint < rref < round`).

## Residual unknowns (all closed by pre-flight probes before any edit)

- C8 pre-fix reality: `mean(matrix<int>)` returns `unsigned long` (value 1); `variance`/
  `standard_deviation` for `matrix<int>` are **hard compile errors** pre-fix (the review's
  "variance of {1,2} is 0.25 → 0" is an unsupported claim). Confirmed, `.work/evidence/prefix_p0.log`.
- C9 pre-fix: SIGABRT(134) at `matrix.hpp:6755` (the copy-pasted second `rb > 1` assert fires
  first on a 1×1 kernel).
- C10 pre-fix: SIGABRT(134) at `matrix.hpp:6486` (`row < col`).
- P2b pre-fix: `cholesky_decomposition([[1,2],[2,1]])` silently writes `-nan` into `a[1][1]`
  (void, no failure channel); the PD case computes a valid factor.

**Interview verdict: ready to brainstorm with zero open questions.**
