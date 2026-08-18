# Spec: `matrix_power` (MODIFIED capability)

Delta: **MODIFIED Requirements** — `operator^(m, n)` computes the exact integer power for
every `n ≥ 0` (C6). Pre-fix, the odd branch `return lhs ^ (n - 1) * lhs;` is parsed as
`lhs ^ ((n - 1) * lhs)` (operator precedence): `uint_least64_t * matrix` has no matching
`operator*`, and since `n` is a runtime value the ill-formed expression defeats the whole
function instantiation — **every** call to `m ^ n` is a hard compile error pre-fix (E08
pre-fix log). Post-fix, all `n` compile and compute correctly. The `n == 0` (identity) and
`n == 1` (input) fast paths and the even branch are unchanged. Non-square input remains
undefined and `better_assert`-guarded (unchanged).

## MODIFIED Requirements

### Requirement: R-M1 odd powers compile and compute

For a square matrix `m` and odd `n ≥ 3`, `m ^ n` SHALL compile and equal the left-
associative product `m·m·…·m` (n factors).

#### Scenario: 2x2 closed form (E08 acceptance)

- WHEN `m = [[1,1],[0,1]]` (so `m^k = [[1,k],[0,1]]` for all k)
- THEN `m^3 == [[1,3],[0,1]]` and `m^5 == [[1,5],[0,1]]` (all elements exact)

#### Scenario: 3x3 vs independent oracle

- WHEN `m ^ 3` and `m ^ 5` are applied to a fixed 3×3 matrix with distinct entries
- THEN each equals the loop product `m·m·…·m` (independent in-file oracle) within 1e-9

### Requirement: R-M2 even and small powers unchanged

The `n == 0` → identity, `n == 1` → input, and even-branch (`(m^(n/2))²`) behaviors SHALL be
unchanged.

#### Scenario: n in {0, 1, 2, 4}

- WHEN `m ^ n` is applied with `n ∈ {0, 1, 2, 4}` to the 2×2 matrix above
- THEN the results are `I`, `m`, `[[1,2],[0,1]]`, `[[1,4],[0,1]]` respectively (exact)

#### Scenario: 1x1 scalar power

- WHEN `m ^ 5` is applied to the 1×1 matrix `{2}`
- THEN the result is the 1×1 matrix `{32}`

### Requirement: R-M3 no precedence surface remains

The odd-branch body SHALL contain no `^` operator and no unparenthesized mixed
`*`/`^` expression; the recursion is `half = m ^ (n >> 1); half * half * m`.

#### Scenario: compile (E08 probe)

- WHEN the probe TU `.work/probes/E08.cc` (which instantiates `m ^ 3`) is compiled post-fix
- THEN it compiles with no error and prints `PASS E08` (pre-fix: hard compile error at the
  odd-branch line — recorded as evidence)
