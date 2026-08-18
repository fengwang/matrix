# Spec — `conv-same-kernel` (C9): `conv` "same" mode accepts 1×1 (and any non-square) kernels

## Requirement
`feng::conv(A, B, "same")` (matrix.hpp:6739-6757) accepts kernels with `rb >= 1` and
`cb >= 1` (independently), fixing the copy-pasted second assert that tested `rb > 1` twice.

## Constraints
- Exactly two lines change: the two `better_assert` conditions
  (`rb > 1` → `rb >= 1`; `rb > 1` → `cb >= 1`). Messages unchanged (already per-axis
  correct). No other `conv` line touched — the full/valid/same slice arithmetic was
  verified correct by the 2026-07-13 review.
- No change to the `A.size() > B.size()` swap, the padding, or the `"valid"` path.
- `noexcept` and signature unchanged.

## Acceptance (from contract)
1. E11: `conv(A{2,2}, kernel{1,1,{0.5}}, "same")` → `[[0.5,1],[1.5,2]]`, no abort in a
   debug (assert-live) build (pre-fix: SIGABRT at matrix.hpp:6755, probe p1).
2. `rb==1, cb>1` separately: `A=[[1,2,3],[4,5,6]]`, `B=[[1,1]]` → `[[1,3,5],[4,9,11]]`.
3. `rb>1, cb==1` separately: `A=[[1,2],[3,4],[5,6]]`, `B=[[1],[1]]` → `[[1,2],[4,6],[8,10]]`.
4. "valid" mode regression (2×3 kernel on 4×5 A): `B[0][0]=0.5` rest 0 →
   `[[0.5,1,1.5],[3,3.5,4],[5.5,6,6.5]]` (pins the untouched valid branch).
5. `make test` (new `tests/cases/conv_same.hpp`) passes; asserts compile under
   `-Wall -Wextra` (suite build).

## Out of scope
Full/valid/same slice arithmetic, the `conv` swap heuristic, other modes, performance.
