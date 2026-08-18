# Spec — `cholesky-guard` (P2b): `cholesky_decomposition` reports failure

## Requirement
`feng::cholesky_decomposition` (matrix.hpp:5766) returns `bool` (was `void`) and guards the
diagonal step: if the diagonal-step residual `sum` is `<= 0` (real value types), the function
returns `false` **before** the `sqrt`; a completed factorization returns `true`.

## Constraints
- One guard at the diagonal step, inside the `i == j` branch, before
  `a[i][i] = std::sqrt(sum)`: `if ( sum <= value_type(0) ) return false;`, wrapped in
  `if constexpr ( ! std::is_complex_v< value_type > )` (complex has no ordering; legacy
  path preserved, returns `true` on completion; no in-repo complex callers).
- Strict boundary (C-11 resolution): `<= 0`, not `< 0` — forced by the contract's own
  adversarial cases `1×1 {0} → false` and "PSD-singular → false"; PRD line 11: "false when
  the diagonal step is not positive-definite" (strict positivity IS the PD diagonal step).
- Factorization arithmetic, the `better_assert(m.row() == m.col())` precondition, and the
  upper-triangular zero-fill unchanged. No new epsilon (the guard is exact-zero inclusive,
  no `tiny_eps`).
- The off-diagonal divide-by-`a[i][i]` needs no separate guard: after the diagonal guard
  every `a[i][i] = sqrt(sum)` has `sum > 0` ⇒ strictly positive (the PRD's "keep the
  `a[i][i]==0` check" refers to a check absent from the current source; the guard subsumes
  it — recorded in brainstorming D-P2b-1).
- `a` must be left in a defined state on `false` (it is: the guard fires before the bad
  `sqrt`; earlier entries are finite; **no NaN is written** post-fix).

## Acceptance (from contract)
1. E13: non-PD `[[1,2],[2,1]]` → `false`, `a` defined, no NaN.
2. PD `[[4,2],[2,3]]` → `true`, `a = [[2,0],[1,√2]]`, `a·aᵀ ≈ m` (finite tolerance).
3. PSD-singular `[[1,1],[1,1]]` → `false` (boundary).
4. `1×1 {0}` → `false`; `1×1 {4}` → `true` with `a[0][0] == 2`.
5. `make test` (new `tests/cases/cholesky.hpp`) passes with no NaN-dependent assertions
   (R-19: `-Ofast` folds NaN comparisons; none needed — the non-PD path produces no NaN
   post-fix).

## Out of scope
The Cholesky arithmetic, convergence behavior, complex PD semantics, other decompositions.
