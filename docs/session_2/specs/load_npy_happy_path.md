# Spec: `load_npy_happy_path_loading` (MODIFIED capability)

Delta: **MODIFIED Requirements** — the full updated content of the requirement is given below.
Behavior of valid files is **unchanged** (PRD §5 row 18: "valid files load exactly as before");
what changes is that the requirement is now explicit, and that it coexists with the new
validation capability. The 4 existing `TEST_CASE` blocks in `tests/cases/load_npy.hpp` and their
registration at `tests/test.cc:35` MUST remain byte-for-byte unchanged.

## MODIFIED Requirements

### Requirement: R-H1 Valid files load to identical values (updated)

`load_npy` SHALL load every well-formed NPY file accepted by the `load_npy_boundary_validation`
capability to exactly the values the pre-change implementation produced, for all four in-repo
fixture types and the probe-pinned variants:

1. `./images/u8.npy` (descr `|u1`, shape `(2, 3)`) into `matrix<std::uint8_t>` → values
   `{4,1,8 / 9,1,5}`.
2. `./images/8.npy` (descr `|i1`, shape `(2, 3)`) into `matrix<std::int8_t>` → values
   `{4,1,8 / 9,1,5}`.
3. `./images/32.npy` (descr `<f4`, shape `(2, 3)`) into `matrix<float>` → values
   `{4.815519, 1.0601262, 8.989337 / 9.510697, 1.8137231, 5.7381544}` within 1e-5.
4. `./images/64.npy` (descr `<f8`, shape `(2, 3)`) into `matrix<double>` → same values within
   1e-5.
5. A well-formed version-2-convention file (4-byte LE length, prefix 12, descr `<f8`, shape
   `(1, 2)`) loads with correct values (probe `e03_v2`).
6. A well-formed `fortran_order: True` file loads with the transpose semantics the pre-change
   code applied (`resize(row, col)` + `reshape(col, row)`) (probe `e03_fortran`: 2×3 logical
   array loads as its 3×2 transpose).
7. A file whose payload ends exactly at the file tail loads successfully (probe `e03_exact`).

#### Scenario: fixture u8 unchanged

- WHEN `./images/u8.npy` is loaded into `matrix<std::uint8_t>` after the change
- THEN the 6 element assertions of the existing case pass unmodified (byte-identical TEST_CASE
  block, green in the full suite)

#### Scenario: fixture 8 / 32 / 64 unchanged

- WHEN `./images/8.npy`, `./images/32.npy`, `./images/64.npy` are loaded into `matrix<std::int8_t>`,
  `matrix<float>`, `matrix<double>` respectively
- THEN all existing assertions pass unmodified (full-suite run, same case count +20 assertions
  as baseline suite minus the new cases)

#### Scenario: v2-convention file loads (no 10-vs-12 prefix mix-up)

- WHEN the probe's v2-convention file (prefix 12 layout) is loaded into `matrix<double>`
- THEN `ok == true` and both values are exact (a swapped 10/12 prefix would read garbage and fail
  the content check)

#### Scenario: fortran-order file preserves pre-change transpose semantics

- WHEN a 2×3 `fortran_order: True` file (payload `[1,4,2,5,3,6]`) is loaded into
  `matrix<double>`
- THEN the result is 3×2 equal to `[[1,4],[2,5],[3,6]]` (the logical 2×3 array transposed —
  identical to pre-change behavior, pinned against the `"T"`-detection expression being
  accidentally rewritten)

#### Scenario: payload-to-tail file loads

- WHEN a 1×1 `<f8` file with payload exactly 8 bytes (file ends at payload end) is loaded
- THEN `ok == true` and the value equals the stored double (inclusive payload bound, spec R-V8)
