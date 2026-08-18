# Spec: `determinant` (MODIFIED capability)

Delta: **MODIFIED Requirements** — `det()` is rewritten from Schur-complement recursion
(unguarded `P.inverse()`) to the pivoted-LU product `sign · ∏ U[i][i]` (C5, built on the
P2 pivoting capability). Exact-zero pivot ⇒ exactly `0`, never NaN (P7: no epsilon
thresholds). `size == 0 → 0` is preserved. Non-square `det` remains undefined and
`better_assert`-guarded (unchanged). `noexcept` is dropped from `det()` (the body allocates;
project contract §3).

## MODIFIED Requirements

### Requirement: R-D1 det is the pivoted-LU product

For a square matrix `a` of size `n ≥ 1`, `a.det()` SHALL equal `sign · ∏ᵢ U[i][i]` where
`(L, U, sign)` come from the pivoted `lu_decomposition` capability (`P·A = L·U`, `sign = det(P)`),
computed without the Schur-complement recursion and without any `inverse()` call.

#### Scenario: known nonsingular 4x4 (E07 acceptance)

- WHEN `det` is applied to the 4×4 integer matrix with det 51 (probe matrix)
- THEN the result matches the in-file independent Bareiss fraction-free product within
  1e-9 relative

#### Scenario: known 3x3 requiring two swaps

- WHEN `det` is applied to `[[0,1,2],[1,0,3],[4,5,6]]` (det = 16; zero first pivot rescued by
  the first swap, second swap at column 1)
- THEN the result is 16 within 1e-9

#### Scenario: 2x2 odd swap

- WHEN `det` is applied to `[[0,1],[1,0]]`
- THEN the result is −1 within 1e-12 (sign bookkeeping: one swap ⇒ sign = −1)

### Requirement: R-D2 exact-zero pivot gives exactly 0 (no NaN, no epsilon)

If any pivoted diagonal entry `U[i][i]` is exactly `0`, `det` SHALL return exactly `0`
(`value_type{}`, positive zero). No epsilon threshold SHALL be applied to pivots (P7): a
small-but-nonzero pivot yields a small-but-nonzero determinant.

#### Scenario: singular block matrix (E07 acceptance)

- WHEN `det` is applied to the review's singular block matrix
  `[[1,2,0,0],[2,4,0,0],[0,0,1,1],[0,0,1,2]]`
- THEN the result is exactly `0.0` (pre-fix: `−nan`)

#### Scenario: 1x1 zero

- WHEN `det` is applied to the 1×1 matrix `{0}`
- THEN the result is exactly `0.0`

#### Scenario: near-singular (P7 — no thresholding)

- WHEN `det` is applied to `diag(1, 1.0e-14)`
- THEN the result is nonzero and within 1e-28 of `1.0e-14` (a tiny pivot must not be
  thresholded to zero)

### Requirement: R-D3 preserved edge behaviors

`det` SHALL preserve: `size == 0 → 0` (the existing `0 == size` branch); non-square input
stays `better_assert`-guarded (undefined behavior, unchanged); the assert message reads
"the row and col are supposed to be same".

#### Scenario: 0x0 preserved

- WHEN `det` is applied to a 0×0 matrix
- THEN the result is `0` (unchanged pre-fix behavior — the mathematical empty product 1 is
  NOT adopted; unsanctioned to change)

#### Scenario: 1x1 nonzero

- WHEN `det` is applied to the 1×1 matrix `{5}`
- THEN the result is `5`
