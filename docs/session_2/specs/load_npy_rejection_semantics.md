# Spec: `load_npy_rejection_semantics` (NEW capability)

Delta: **ADDED Requirements**. Defines the complete rejection contract — the T2 error-path gap
this session closes (PRD §7 P8). Normative language: MUST.

## ADDED Requirements

### Requirement: R-R1 Complete reject table

`load_npy` MUST return `false` (and never throw, abort, or exhibit undefined behavior) for every
input in this table, in any build mode (with or without `NDEBUG`):

| Input class | Example |
|---|---|
| Missing / unopenable file | `tmp/nonexistent.npy` |
| File < 12 bytes | 3-byte, 11-byte files |
| Bad magic | ≥12 bytes, wrong first 6 bytes |
| Version ∉ {1, 2} | version byte 0 or 3 |
| `header_length` beyond remaining bytes | `header_length` = 0xFFFFFFFF; `header_length` = file tail + 1 |
| Header not a dict literal | first header byte ≠ `'{'` |
| Missing / unparseable descr field | no `'descr': '` token; unterminated descr quote |
| Foreign dtype | `<f4`/`>f8`/`Vf8`/`\|u1` into `matrix<double>`; `<f8` into `matrix<float>` |
| Missing / malformed shape token | no `'shape': (`; `(2,)`; `(2, 3, 4)`; `(a, b)`; `(-1, 2)`; 30-digit token |
| Zero dimension | `(0, 2)`, `(2, 0)` |
| Shape product / payload overflow | `row*col` or `payload` would wrap `size_t` |
| Truncated payload | `payload > buffer.size() - data_offset` |

#### Scenario: every reject-table input yields false with the matrix untouched

- WHEN each input class of the table is loaded into a default-constructed `matrix<double>` (or
  the target type named)
- THEN `load_npy` returns `false` and the matrix's `row()`/`col()` equal the values captured
  before the call

#### Scenario: reject is clean under ASan in release mode

- WHEN the probe build (`-DNDEBUG -fsanitize=address`) loads the 3-byte, 11-byte, 12-byte,
  0xFFFFFFFF-length, and missing-shape files
- THEN no ASan report is emitted, no `terminate` occurs, and the process exits 0

#### Scenario: missing file rejected without abort in a debug (assert-enabled) build

- WHEN an unopenable path is loaded in the suite build (no `-DNDEBUG`, `debug_mode` = 1)
- THEN `load_npy` returns `false` and the process does not abort (pre-fix: `better_assert` →
  `print_assertion` → `abort()` — SIGABRT, evidence `tdd_red_run.log`)

#### Scenario: reject leaves no partial state (no resize before rejection)

- WHEN a file passes magic/version/bounds but fails the dtype check (E04: `<f4` into
  `matrix<double>`)
- THEN the member was not resized (row/col unchanged) — rejection happens before `zen.resize`

### Requirement: R-R2 noexcept honesty

The `load_npy` members MUST remain `noexcept` (signature unchanged) and MUST be throw-free in
practice: any exception raised within the validated region (including allocation failures) MUST be
converted to a `false` return. `std::terminate` from an escaping exception is a contract
violation.

#### Scenario: no terminate on any crafted input

- WHEN the full probe case set (18 cases) runs under `-DNDEBUG`
- THEN no `terminate called` message appears and every case completes (pre-fix: four distinct
  terminate paths were recorded)

### Requirement: R-R3 Deterministic test hygiene

Negative-path tests MUST craft their input bytes at runtime (no new committed binary fixtures),
write them into `tmp/`, and remove them at the end of each case so repeated runs see an identical
filesystem state.

#### Scenario: test run is idempotent

- WHEN `make test` is run twice in a row
- THEN both runs are green and `git status` shows no new files (tmp/ is gitignored; files removed
  per case)

### Requirement: R-R4 Documented rejection semantics (doc delta)

The handoff MUST emit the exact ReadMe replacement wording for the §"load npy" line (~1055) so
that S6 can publish it: `load_npy` returns `false` on malformed/foreign-dtype files; the dtype
must match the target type; truncated and truncated-payload files are rejected; the version-2
layout follows the library's existing 4-byte-length convention (real-spec 8-byte-length v2 files
are rejected).

#### Scenario: doc delta wording exists in the handoff

- WHEN the handoff (`.work/handoff_session_2.md`) is read
- THEN it contains a verbatim ReadMe line replacement covering the reject semantics and the
  dtype-match requirement
