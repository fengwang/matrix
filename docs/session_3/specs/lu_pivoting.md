# Spec: `lu_pivoting` (MODIFIED capability)

Delta: **MODIFIED Requirements** — `lu_decomposition` performs partial pivoting and exposes
the permutation and its sign (P2, PRD §5 row 7: "solution unchanged, factors differ"). A new
5-arg primary is added; the existing 3-arg overload delegates to it (source-compatible —
no existing call breaks); the 1-arg tuple overload and both `lu_solver` overloads keep
their signatures. `lu_solver` applies P to b. `forward_substitution`/`backward_substitution`
are unchanged.

## MODIFIED Requirements

### Requirement: R-L1 partial pivoting with permutation and sign channels

`lu_decomposition(A, L, U, sign, perm)` (the 5-arg primary) SHALL produce `L` (unit
lower-triangular), `U` (upper-triangular), `sign ∈ {−1, +1}`, and `perm` such that
`P·A = L·U`, where `perm[i]` is the original row index of permuted row `i` and
`sign = det(P)`. Pivoting SHALL be partial (max absolute value in the current column over
rows `j … n−1`, first maximum wins ties). `A` SHALL not be modified. The existing
`isinf/isnan → return 1` failure signaling SHALL be preserved.

#### Scenario: PA = LU residual (E09-related)

- WHEN the 5-arg decomposition is applied to the fixed 3×3 matrix
  `[[0,1,2],[1,0,3],[4,5,6]]` (zero first pivot; two swaps required)
- THEN `‖P·A − L·U‖∞ < 1e-9`, `perm == {2, 0, 1}`, and `sign == +1` (two swaps)

#### Scenario: odd swap sign

- WHEN the 5-arg decomposition is applied to `[[0,1],[1,0]]`
- THEN `perm == {1, 0}`, `sign == −1`, and `‖P·A − L·U‖∞ < 1e-9`

#### Scenario: no-op path

- WHEN the 5-arg decomposition is applied to the 3×3 identity
- THEN `perm == {0, 1, 2}`, `sign == +1`, `L == I`, `U == I`

### Requirement: R-L2 the 3-arg overload delegates

`lu_decomposition(A, L, U)` SHALL call the 5-arg primary (discarding `sign`/`perm`) and
keep its signature and return-code semantics; the 1-arg tuple overload SHALL keep its
`optional<tuple<Mat, Mat>>` signature and behavior.

#### Scenario: source compatibility

- WHEN the pre-session call sites (`examples/cases/0019_lu_decomposition.hpp` via the 1-arg
  overloads; in-file `lu_solver` via the 3-arg form) compile against the new header
- THEN they compile unmodified and `make example` exits 0

### Requirement: R-L3 lu_solver applies P to b

`lu_solver(A, x, b)` SHALL solve `A·x = b` via `L·Y = P·b`, `U·x = Y` using the
decomposition's `perm` (i.e. `Pb[i] = b[perm[i]]`); its signatures (3-arg int, 2-arg
`optional`) SHALL be unchanged. Solutions SHALL be invariant under the pivoting change for
nonsingular systems.

#### Scenario: solution invariance vs legacy oracle (E09 acceptance)

- WHEN `lu_solver` is applied to the fixed 6×6 well-conditioned system (exact solution
  (1,2,3,4,5,6))
- THEN `‖x_lib − x_legacy‖∞ < 1e-9` against the in-file legacy (pre-pivoting) LU oracle and
  `‖x_lib − x_exact‖∞ < 1e-9`

#### Scenario: zero-first-pivot rescue

- WHEN `lu_solver` is applied to the 3×3 system `A·x = (3, 4, 15)` with
  `A = [[0,1,2],[1,0,3],[4,5,6]]` (pre-pivoting code fails: `L[i][j] = x/0 = inf/nan` →
  nullopt; a row swap rescues the matrix)
- THEN the result is `has_value()` and `x ≈ (1, 1, 1)` within 1e-9

#### Scenario: singular system still nullopt

- WHEN `lu_solver` is applied to the singular 2×2 `[[1,2],[2,4]]`
- THEN the result is `nullopt` (unchanged failure signaling; zero pivot in the last column
  is caught by the `backward_substitution` inf/nan guard, as pre-fix)
