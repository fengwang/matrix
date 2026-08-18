# Spec: `pseudoinverse` (MODIFIED capability)

Delta: **MODIFIED Requirements** — `pinverse`/`pinv` actually compute the Moore–Penrose
pseudoinverse (C4, PRD §5 row 4) through a single correct SVD-inversion core; `svd_inverse`
calls the SVD in its signature's argument order (R1). Public names and signatures unchanged
(retirement is S6's job). The SVD core's own behavior is out of scope; its empirically
demonstrated wide-matrix (m < n) limitation is documented, not fixed (D4, user-confirmed
narrowing).

## MODIFIED Requirements

### Requirement: R-P1 svd_inverse uses the SVD's argument order

`svd_inverse(a)` SHALL call `singular_value_decomposition(a, u, w, v)` with `u, w, v` in the
signature's order (`u` = left vectors, `w` = singular-value matrix, `v` = right vectors),
invert `w` elementwise under the threshold rule (R-P4), and return `v * w * u.transpose()`.

#### Scenario: diagonal matrix (E06 acceptance)

- WHEN `pinv` (equivalently `svd_inverse`) is applied to `diag(1, 2)`
- THEN the result is `diag(1, 0.5)` within 1e-8 (off-diagonals < 1e-8)

#### Scenario: names match the computation

- WHEN `svd_inverse` is applied to `diag(1, 2)`
- THEN it returns the same value as `pinverse(diag(1, 2))` (bitwise equality — same core)

### Requirement: R-P2 pinverse delegates to the single core

`pinverse(m)` SHALL be `return svd_inverse( m );` — one SVD-inversion core shared by
`pinverse`, `pinv`, and `svd_inverse`; `pinv(m)` remains `return pinverse( m );`.

#### Scenario: same core

- WHEN `pinv(m)` and `pinverse(m)` are applied to the same matrix `m`
- THEN both results are equal (all elements)

### Requirement: R-P3 Moore–Penrose conditions (valid SVD domain)

For every matrix `m` in the SVD core's valid domain (tall or square, per the pre-fix probe
evidence), `p = pinv(m)` SHALL satisfy, within a 1e-8 tolerance:
`m·p·m ≈ m`, `p·m·p ≈ p`, `(m·p)ᵀ ≈ m·p`, `(p·m)ᵀ ≈ p·m`.

#### Scenario: full-column-rank tall matrix

- WHEN `pinv` is applied to a 4×2 full-column-rank matrix
- THEN all four Moore–Penrose residuals are < 1e-8

#### Scenario: rank-deficient tall matrix (E06 adversarial case, D4-narrowed)

- WHEN `pinv` is applied to the rank-1 4×2 matrix `[[1,2],[2,4],[3,6],[4,8]]`
- THEN all four Moore–Penrose residuals are < 1e-8 and `pinv` shape is 2×4

#### Scenario: rank-deficient square matrix (E06 adversarial case, D4-narrowed)

- WHEN `pinv` is applied to `diag(1, 1, 0)` (3×3, rank 2)
- THEN the result equals `diag(1, 1, 0)` within 1e-12 (σ=0 stays 0 under R-P4; MP uniqueness)

#### Scenario: all-zero matrix

- WHEN `pinv` is applied to the 2×2 zero matrix
- THEN the result is the 2×2 zero matrix

### Requirement: R-P4 threshold semantics (inherited, no new epsilon)

The core SHALL invert a singular value `σ` iff `|σ| > 1.0e-10` (strict; the inherited
`svd_inverse` rule — P7 forbids new thresholds); values with `|σ| ≤ 1.0e-10` stay as-is.

#### Scenario: boundary value

- WHEN `pinv` is applied to the 1×1 matrix `[1.0e-10]`
- THEN the result is `1.0e-10` (not inverted — not `1.0e10`)
- WHEN `pinv` is applied to the 1×1 matrix `[2.0e-10]`
- THEN the result is `5.0e9` within 1e-3 relative (inverted)

### Requirement: R-P5 wide-matrix gap is documented, not asserted

No test or probe in this session SHALL assert Moore–Penrose conditions on `pinv` of an
m < n matrix (the SVD core's reconstruction is empirically invalid there — pre-fix evidence
`prefix_probes.log`: 2×4 reconstruction error 6.0). The gap is recorded in
`docs/risk_register.md`, the E06 seed footnote, and the handoff warning as an **S6
candidate**.

#### Scenario: no wide-MP assertion creeps in

- WHEN the session's tests and probes are audited
- THEN no case asserts MP conditions on an m < n `pinv` result
