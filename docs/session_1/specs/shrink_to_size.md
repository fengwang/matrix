# Spec — Capability: `shrink-to-size` (C1)

Authority: `docs/session_1_contract.yaml` invariants; PRD §5 row 1; in-code documented contract
(`matrix.hpp` comment ~3515–3517: "if new row or col are larger than the original, padding with
zero; otherwise, drop these elements").

## MODIFIED Requirements

### Requirement: `shrink_to_size` preserves the top-left block and zero-pads growth

`matrix::shrink_to_size(new_row, new_col)` SHALL resize the matrix in place to shape
`(new_row, new_col)` such that, for `rows_keep = min(row, new_row)` and `cols_keep = min(col,
new_col)`:

1. For every `r < rows_keep` and `c < cols_keep`, the value at `(r, c)` in the result SHALL equal
   the value at `(r, c)` in the original matrix (no reordering, no offset shift).
2. Every element of the result outside the top-left `rows_keep × cols_keep` block SHALL be
   `value_type{}` (zero for arithmetic types).
3. The result SHALL have exactly the shape `(new_row, new_col)`.
4. All accesses SHALL be within the allocated buffers of the original and the new matrix (no
   out-of-bounds read or write for any shape, including non-square grow/shrink combinations).

The implementation MUST NOT change the function signature or `noexcept`-ness beyond the existing
one.

#### Scenario: Column-only shrink on a filled matrix (E01 acceptance, review ASan repro)

- WHEN a `5×5` matrix of `1.0` is shrunk to `(5, 3)`
- THEN the result shape is `5×3` and all 15 elements equal `1.0`, and the operation completes
  without an AddressSanitizer report (build flags `-DNDEBUG -fsanitize=address`).

#### Scenario: Mixed shrink-and-grow with ragged values (review silent-corruption repro)

- WHEN a `3×10` matrix holding `1..30` row-major is resized to `(5, 2)`
- THEN the result equals
  `[[1,2],[11,12],[21,22],[0,0],[0,0]]` exactly (first 2 columns of the first 3 rows preserved;
  growth rows zero-padded), with no AddressSanitizer report.

#### Scenario: Grow-only zero-pad (contract acceptance E01)

- WHEN a `1×1` matrix holding `7.0` is resized to `(4, 4)`
- THEN `result[0][0] == 7.0`, all other 15 elements equal `0.0`, and the shape is `4×4`.

#### Scenario: Non-square shrink, both directions (adversarial)

- WHEN a `10×3` matrix holding `r*3+c+1` (ragged) is resized to `(5, 2)`, and a `3×10` matrix
  holding `r*10+c+1` is resized to `(5, 2)`
- THEN each result equals the top-left `5×2` block of its source, exactly.

#### Scenario: Shrink-only extreme

- WHEN a `10×10` matrix holding `r*10+c+1` is resized to `(1, 1)`
- THEN the result is `1×1` holding the original `(0,0)` value `1.0`.
