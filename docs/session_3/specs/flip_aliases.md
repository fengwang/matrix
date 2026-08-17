# Spec: `flip_aliases` (MODIFIED capability)

Delta: **MODIFIED Requirements** — `fliplr`/`flipud` are corrected to the MATLAB/NumPy
convention (PRD §5 row 3). The `flipdim` bodies are S1's and MUST remain byte-for-byte
unchanged (verified by E02, not re-edited).

## MODIFIED Requirements

### Requirement: R-F1 fliplr flips left-right (dim 2)

`fliplr(m)` SHALL return the left-right reversal of `m` for any matrix `m` (square or
rectangular), i.e. the same result as `flipdim(m, 2)`: element `(r, c)` of the result equals
`m[r][col() - 1 - c]`; shape is unchanged.

#### Scenario: 2x3 content (E05 acceptance)

- WHEN `fliplr` is applied to `[[1,2,3],[4,5,6]]`
- THEN the result is exactly `[[3,2,1],[6,5,4]]` (all six elements asserted)

#### Scenario: 3x3 ragged content

- WHEN `fliplr` is applied to a 3×3 matrix with 9 distinct values
- THEN every element equals the left-right mirrored input value

### Requirement: R-F2 flipud flips up-down (dim 1)

`flipud(m)` SHALL return the up-down reversal of `m` for any matrix `m`, i.e. the same result
as `flipdim(m, 1)`: element `(r, c)` of the result equals `m[row() - 1 - r][c]`; shape is
unchanged.

#### Scenario: 2x3 content (E05 acceptance)

- WHEN `flipud` is applied to `[[1,2,3],[4,5,6]]`
- THEN the result is exactly `[[4,5,6],[1,2,3]]` (all six elements asserted)

#### Scenario: degenerate shapes

- WHEN `fliplr` is applied to a 1×3 row vector and `flipud` to a 3×1 column vector
- THEN the results are the elementwise row reversal and the row reversal respectively
- WHEN either alias is applied to a 1×1 matrix
- THEN the result equals the input (fixed point)

### Requirement: R-F3 flipdim bodies untouched

The `flipdim` free function (both dimension branches) SHALL remain byte-for-byte unchanged by
this session; the S1 regression case (`tests/cases/flip.hpp`) stays green.

#### Scenario: S1 regression

- WHEN the full suite runs after the alias fix
- THEN the `Matrix flipdim` case passes unmodified (E02 probe remains green)
