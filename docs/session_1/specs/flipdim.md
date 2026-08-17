# Spec — Capability: `flipdim` (C2)

Authority: `docs/session_1_contract.yaml` invariants; PRD §5 row 2; review §C2 (violated contract:
"flipdim must flip along dimension 2 (left/right flip)"). The `fliplr`/`flipud` aliases (C3) are
**out of scope** for this session (S3; anchors 4491–4499 untouched) and are pinned here only as a
boundary statement, not as a behavior requirement.

## MODIFIED Requirements

### Requirement: `flipdim(m, 2)` is a true left-right flip for all shapes

For any `matrix<T, A> m` with `row() > 0` and `col() > 0`, `flipdim(m, 2)` SHALL return a matrix
`f` of the same shape as `m` such that for every `r, c`: `f[r][c] == m[r][col()-1-c]`
(left-right / column flip). The operation SHALL complete without out-of-bounds reads or writes
for any shape, including non-square, 1×N, and N×1. `flipdim` SHALL NOT mutate `m`.

#### Scenario: Non-square 3×5 left-right flip (E02 acceptance, review ASan repro)

- WHEN `m` is `3×5` holding `1..15` row-major
- THEN `flipdim(m, 2)` equals `[[5,4,3,2,1],[10,9,8,7,6],[15,14,13,12,11]]` exactly, and the
  operation completes without an AddressSanitizer report (build flags
  `-DNDEBUG -fsanitize=address`).

#### Scenario: Square 4×4 left-right flip (review silent-corruption repro)

- WHEN `m` is `4×4` holding `1..16` row-major
- THEN `flipdim(m, 2)` satisfies `f[r][c] == m[r][3-c]` for all `r, c` (i.e., row `0` is
  `4,3,2,1` and row `3` is `16,15,14,13`).

#### Scenario: Ragged non-square, both orientations (adversarial)

- WHEN `m` is `2×7` holding `r*7+c+1`, and `m'` is `7×2` holding `r*2+c+1`
- THEN `flipdim(m, 2)[r][c] == m[r][6-c]` for all `r,c`, and `flipdim(m', 2)[r][c] ==
  m'[r][1-c]` for all `r,c`.

#### Scenario: Degenerate single-row / single-column shapes (adversarial)

- WHEN `m` is `1×5` holding `1..5`, or `m'` is `5×1` holding `1..5`
- THEN `flipdim(m, 2)` equals `5,4,3,2,1` (1×5), `flipdim(m', 2)` equals `m'` (5×1, a single
  column is unchanged by a left-right flip), and both complete without an AddressSanitizer report.

### Requirement: `flipdim(m, 1)` behavior is unchanged (boundary pin)

This session SHALL NOT modify the `dim == 1` branch. As a regression pin: for any `m`,
`flipdim(m, 1)` SHALL return `f` with `f[r][c] == m[row()-1-r][c]` (up-down flip), the shape
unchanged, and `m` unmodified.

#### Scenario: 3×5 up-down flip (dimension-1 pin)

- WHEN `m` is `3×5` holding `1..15` row-major
- THEN `flipdim(m, 1)` equals `[[11,12,13,14,15],[6,7,8,9,10],[1,2,3,4,5]]` exactly.

## REMOVED Requirements

(none — no requirement is removed; the previous buggy behaviors — OOB read/write and column-vs-row
scramble on dim 2 — are defects, not documented requirements.)
