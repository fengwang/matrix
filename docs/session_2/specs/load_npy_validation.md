# Spec: `load_npy_boundary_validation` (NEW capability)

Delta: **ADDED Requirements**. This capability did not exist — the pre-fix `load_npy` performed
no boundary validation (finding S1). Normative language: MUST/SHALL. All scenarios are
testable: each maps to a probe case (`.work/probes/E03_E04.cc`) and/or a suite case
(`tests/cases/load_npy.hpp`).

## ADDED Requirements

### Requirement: R-V1 Minimum size and NPY magic before any dereference

`load_npy` MUST reject (return `false`) before dereferencing any file byte unless the file is at
least 12 bytes long and its first 6 bytes equal the NPY magic `\x93NUMPY`. The 12-byte minimum
covers the 6-byte magic + 2-byte version + 4-byte maximum header-length field.

#### Scenario: 3-byte file rejected without OOB

- WHEN a 3-byte file `{0x93, 'N', 'U'}` is loaded into `matrix<double>` under the ASan probe build
- THEN `load_npy` returns `false`, and the process is ASan-clean (no report, no abort, exit 0)

#### Scenario: 11-byte file rejected without OOB

- WHEN an 11-byte file (valid magic, version 1, `header_length` = 0xFFFF, 3 trailing bytes) is
  loaded into `matrix<double>` under the ASan probe build
- THEN `load_npy` returns `false` and the process is ASan-clean

#### Scenario: 12-byte file with no shape token rejected cleanly

- WHEN a 12-byte file (valid magic, version 1, `header_length` = 2, header `{}`) is loaded
- THEN `load_npy` returns `false` (pre-fix: `std::terminate` via `std::out_of_range`)

#### Scenario: non-NPY magic of sufficient size rejected

- WHEN a 16-byte file of all `0xAA` bytes is loaded into `matrix<double>`
- THEN `load_npy` returns `false`

### Requirement: R-V2 Version acceptance set and prefix selection

`load_npy` MUST accept only version bytes 1 and 2. Version 1 SHALL use a 2-byte little-endian
header length at offsets 8–9 and data prefix 10. Version 2 SHALL use a 4-byte little-endian header
length at offsets 8–11 and data prefix 12 (the library's existing in-code convention). Any other
version byte MUST be rejected.

#### Scenario: version byte 0 rejected

- WHEN a file with valid magic, version byte 0, and otherwise valid v1 layout is loaded
- THEN `load_npy` returns `false`

#### Scenario: version 2 file under the library convention loads

- WHEN a well-formed file with version byte 2, 4-byte LE `header_length`, descr `<f8`, shape
  `(1, 2)`, and a 16-byte payload is loaded into `matrix<double>`
- THEN `load_npy` returns `true` and the values are correct (convention pin, probe `e03_v2`)

### Requirement: R-V3 Non-wrapping header-length bound

`load_npy` MUST read `header_length` from the version-appropriate bytes and reject with `false`
whenever `header_length > buffer.size() - data_prefix`. The bound MUST be evaluated in the
non-wrapping form (subtraction from `buffer.size()`); the wrapping form
`buffer.size() < data_prefix + header_length` MUST NOT be used because it overflows for
`header_length` = 0xFFFFFFFF. When `header_length` equals the remaining size exactly, the bound
passes and subsequent header-content checks decide the outcome.

#### Scenario: header_length 0xFFFFFFFF rejected without OOB

- WHEN a 16-byte v2-convention file claims `header_length` = 0xFFFFFFFF is loaded under the ASan
  probe build
- THEN `load_npy` returns `false` and the process is ASan-clean (pre-fix: ASan heap-buffer-overflow)

#### Scenario: exact-boundary header_length does not wrap

- WHEN `header_length == buffer.size() - data_prefix` exactly
- THEN the bound check passes (no wrap), and a header lacking the shape token yields `false`

### Requirement: R-V4 Header dict sanity

The header MUST begin with `'{'` (the NPY header is a Python dict literal per the NPY spec). A
header not beginning with `'{'` MUST be rejected.

#### Scenario: header not starting with '{' rejected

- WHEN a valid-magic v1 file's header begins with `x'descr': …` (no leading `{`)
- THEN `load_npy` returns `false`

### Requirement: R-V5 dtype match against the target value_type

`load_npy` MUST parse the descr field positionally (locate `'descr': '`, then the next single
quote, both `npos`-guarded) and reject unless the descr value exactly equals the canonical
little-endian descriptor of the member's `value_type`:

| value_type | accepted descr |
|---|---|
| `std::uint8_t` | `\|u1` |
| `std::int8_t` | `\|i1` |
| `std::int16_t` | `<i2` |
| `std::uint16_t` | `<u2` |
| `std::int32_t` | `<i4` |
| `std::uint32_t` | `<u4` |
| `std::int64_t` | `<i8` |
| `std::uint64_t` | `<u8` |
| `float` | `<f4` |
| `double` | `<f8` |

Any other `value_type` MUST reject all files. Big-endian (`>`) and native (`V`) descriptors MUST
be rejected (byte-swapping is out of scope).

#### Scenario: float32 file into matrix<double> rejected (E04)

- WHEN a well-formed 1×2 file with descr `<f4` and an 8-byte payload is loaded into
  `matrix<double>`
- THEN `load_npy` returns `false` (pre-fix: returned `true` with misinterpreted bytes)

#### Scenario: big-endian descriptor rejected

- WHEN a well-formed 1×2 file with descr `>f8` is loaded into `matrix<double>`
- THEN `load_npy` returns `false` (pre-fix: returned `true` loading garbage)

#### Scenario: native-endian descriptor rejected

- WHEN a well-formed file with descr `Vf8` is loaded into `matrix<double>`
- THEN `load_npy` returns `false`

#### Scenario: matching descriptor accepted (fixture behavior)

- WHEN `./images/u8.npy` (descr `|u1`) is loaded into `matrix<std::uint8_t>`
- THEN `load_npy` succeeds with the fixture values (existing case, unchanged)

### Requirement: R-V6 npos-guarded shape token presence

`load_npy` MUST locate the shape via `header.find("'shape': (")` and reject with `false` if it is
absent. The row token MUST end at the first `','` after the token start and the column token at
the first `')'` after that; each `find` result MUST be checked against `npos` before use.

#### Scenario: missing shape token rejected

- WHEN a well-formed-magic v1 file has a header containing descr and `fortran_order` but no
  `'shape': (` token
- THEN `load_npy` returns `false` (pre-fix: `std::terminate` via `std::invalid_argument`)

#### Scenario: 1-D shape rejected

- WHEN a file's shape token is `(2,)`
- THEN `load_npy` returns `false` (pre-fix: `std::terminate`)

#### Scenario: 3-D shape rejected

- WHEN a file's shape token is `(2, 3, 4)`
- THEN `load_npy` returns `false` (the column token contains a comma → not a digit string)

### Requirement: R-V7 Digit-bounded shape values

Each shape token MUST parse as a non-empty unsigned decimal integer: optional leading whitespace
(space/tab), then one or more ASCII digits, no trailing characters, no sign. Parsing MUST NOT
overflow `size_t` (values exceeding `SIZE_MAX` are rejected). Both parsed dimensions MUST satisfy
`row >= 1` and `col >= 1`; zero dimensions MUST be rejected.

#### Scenario: negative shape rejected

- WHEN the shape token is `(-1, 2)`
- THEN `load_npy` returns `false` (pre-fix: `stoul("-1")` → `resize` throws
  `bad_array_new_length` → `std::terminate`)

#### Scenario: size_t-overflowing shape rejected

- WHEN the row token is 30 digits of `9`
- THEN `load_npy` returns `false` (pre-fix: `stoul` throws `std::out_of_range` → terminate)

#### Scenario: zero dimension rejected

- WHEN the shape token is `(0, 2)`
- THEN `load_npy` returns `false`

#### Scenario: well-formed shape parses

- WHEN the shape token is `(2, 3)` (numpy layout, space after the comma)
- THEN row = 2 and col = 3 (all four fixtures load unchanged)

### Requirement: R-V8 Overflow-checked payload bound

`load_npy` MUST compute the payload byte count with overflow-checked multiplication: reject if
`row > SIZE_MAX / col`, then `elements = row * col`; reject if `elements > SIZE_MAX /
sizeof(value_type)`, then `payload = elements * sizeof(value_type)`. With `data_offset =
data_prefix + header_length` (safe by R-V3), `load_npy` MUST reject unless
`payload <= buffer.size() - data_offset`. The bound is inclusive: a payload ending exactly at the
file tail is accepted.

#### Scenario: payload exactly at file tail accepted

- WHEN `buffer.size() == data_offset + row * col * sizeof(value_type)` exactly
- THEN `load_npy` returns `true` with correct values (probe `e03_exact`)

#### Scenario: payload one byte short rejected

- WHEN `buffer.size() == data_offset + payload - 1`
- THEN `load_npy` returns `false`, ASan-clean (pre-fix: returned `true` reading past the buffer)

#### Scenario: wrapping shape product rejected before resize

- WHEN the shape parses as `row = 2^40`, `col = 2^24` (product wraps `size_t`)
- THEN `load_npy` returns `false` before any `resize` call

### Requirement: R-V9 Resize only after validation; throw-free body

`load_npy` MUST NOT call `zen.resize` (or `reshape`) before R-V1 through R-V8 have all passed.
The body from buffer construction through the data copy MUST be wrapped so that no exception can
escape the `noexcept` member (residual exception → `false`).

#### Scenario: resize never precedes a rejection

- WHEN any rejection scenario (R-V1…R-V8) fires
- THEN no `resize`/`reshape` was called and the member's `row()`/`col()` are unchanged

#### Scenario: allocation failure inside the boundary yields false

- WHEN an allocation within the validated region throws (e.g. `bad_alloc` from `resize` on a
  validated-but-hostile shape under memory pressure)
- THEN `load_npy` returns `false` and the member does not throw or abort
