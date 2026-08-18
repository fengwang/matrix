# Independent test-writer derivation (S2 task 5)

Process note: no subagent tool is available in this environment (the protocol's independent
derivation assumes a fresh-context subagent). Deviation: the derivation below was produced in
this session under a disciplined fresh framing — inputs limited to the contract YAML, PRD §5
row 18, the NPY wire-format facts, and the P3 untrusted-input checklist; the Task 3 diff and the
implementation rationale docs were not consulted while writing the table. The comparison
section (after the table) was written with full context.

## Inputs (contract-only)

- `docs/session_2_contract.yaml`: P3 validation checklist (size before deref; `header_length`
  within buffer — `> buffer.size() - prefix`, not wrapping; dtype match; npos-guarded shape parse;
  row/col ≥ 1; payload bound `> buffer.size() - data_offset` → false; resize only after all
  checks; wrap `stoul` → no throw escapes `noexcept`); invariants (3B/11B/12B clean false;
  0xFFFFFFFF clean false; missing shape clean false; foreign dtype clean false); failure modes
  to watch (10 vs 12 prefix mix-up; wrapping bound form; silent misinterpretation).
- PRD §5 row 18: malformed/truncated/foreign-dtype → `false`; valid files load exactly as before.
- NPY wire facts: 6-byte magic `\x93NUMPY`; version byte @6; v1 = 2-byte LE header length @8,
  data @10; v2 (library convention, per contract's pinned 10/12) = 4-byte LE length @8, data @12;
  header is a dict literal with `descr`, `fortran_order`, `shape` fields.

## Expected outcomes (derived, pre-implementation reading)

| # | Input | Expected `load_npy` | Reason (contract clause) |
|---|---|---|---|
| 1 | 3-byte file `{0x93,'N','U'}` | false, ASan-clean, no abort | size < 12 must be checked before any deref (P3 size-before-deref); contract invariant |
| 2 | 11-byte file (magic, ver 1, len 0xFFFF, 3 tail) | false, ASan-clean | header_length bound: 0xFFFF > 11−10 (P3 non-wrapping form) |
| 3 | 12-byte file, no shape token | false, no terminate | shape parse npos-guarded (P3; contract invariant "missing shape → clean false") |
| 4 | magic wrong, ≥12 bytes | false | magic check (implied by "size and shape sanity"; P3 untrusted input) |
| 5 | version byte 0 (or 3) | false | "version in {1,2}" (P3 checklist) |
| 6 | v2, header_length = 0xFFFFFFFF | false, ASan-clean | wrapping form `size < 12 + len` overflows → must use `len > size − prefix` (contract failure mode, named) |
| 7 | header_length == size − prefix exactly | bound passes; content checks decide (no shape → false) | bound is the reject condition `>`, not `≥` (contract: "header_length within buffer (… > buffer.size() − prefix … → false)") |
| 8 | header not starting with `{` | false | header dict-literal sanity (NPY fact; guards real-spec-v2 4-byte-shift misread) |
| 9 | descr `<f4` into `matrix<double>` | false (E04 acceptance) | dtype must match target (P3 "dtype matches target"); pre-fix silent misload |
| 10 | descr `>f8` (big-endian) into `matrix<double>` | false | only canonical little-endian descriptor matches; byte-swap is not in scope (row 18: foreign dtype → false) |
| 11 | descr `Vf8` (native) into `matrix<double>` | false | as 10 |
| 12 | descr `\|u1` into `matrix<uint8_t>` | true (fixture u8.npy) | valid file loads exactly as before (row 18) |
| 13 | shape `(2,)` | false | 1-D not a 2-D row/col pair; parse must not throw (no `stoul` escape) |
| 14 | shape `(2, 3, 4)` | false | 3-D; col token would contain a comma → non-digit |
| 15 | shape `(-1, 2)` | false, no terminate | sign not a digit; pre-fix `stoul("-1")` → resize throw → terminate (empirically observed) |
| 16 | 30-digit row | false, no terminate | overflow-safe parse; pre-fix `stoul` → `out_of_range` → terminate |
| 17 | shape `(0, 2)` | false | row/col ≥ 1 (P3 checklist, verbatim) |
| 18 | payload ends exactly at file tail | true | payload bound is reject condition `>` (contract: "payload size within buffer (… > … → false)") — equality is within |
| 19 | payload 1 byte short | false, ASan-clean | contract invariant "truncated payload → clean false" |
| 20 | `row*col` wraps `size_t` (e.g. 2^40 × 2^24) | false before resize | resize only after validation + no overflow (P3 checklist "row/col ≥ 1" + overflow-checked arithmetic implied by "payload size within buffer") |
| 21 | missing file | false, no abort, **in every build mode** | contract invariant "unopenable path → clean `false` return" (no mode qualifier); assert-abort is process death, not clean false |
| 22 | v2-convention valid file (4B len, prefix 12) | true, correct values | contract failure mode pins both 10/12 offsets as staying (v2 must keep working under the library convention) |
| 23 | fortran `True` 2×3 file | true, with the pre-change transpose semantics | valid files load exactly as before (row 18) — the `"T"` detection expression's observable behavior is pinned |
| 24 | real-spec v2 file (8-byte length) | false (clean) | read under the library convention it begins with the high length bytes, not `{` → dict sanity rejects (row 18: malformed-under-convention → false) |
| 25 | matrix state on any rejection | unchanged (row/col pre == post) | failure mode "reject before zen.resize" (contract, verbatim) |

## Comparison with implementation results (written after, with full context)

- Suite (post-fix, assert-enabled build): 6/6 `load_npy` cases pass — covers rows 1, 2, 3 (5
  shape variants incl. 13–17), 6, 9, 10, 21, 25.
- ASan probe (post-fix, `-DNDEBUG`): 18/18 `ok` lines as expected — rows 1–3, 5, 6, 9–12, 13–17,
  18, 19, 21–24 (probe cases: `e03_trunc3b/11b/12b`, `e03_v0/v3`, `e03_ffff`, `e04_f4/u1/be/vf8`,
  `e03_noshape/1d/3d/negshape/16digit`, `e03_exact/short`, `e03_missing`, `e03_v2`,
  `e03_fortran`).
- Rows 7 (exact header boundary) and 20 (wrapping product) are reasoned-through rather than
  probed: row 7's content path is exercised by the shape-missing cases (header consumes the
  remainder in `e03_12b` — 12-byte file, header_length = 2 = size − prefix exactly → bound
  passes, shape missing → false: the inclusive-bound behavior is pinned); row 20's guard is a
  two-line overflow check whose inputs (2^40/2^24) would allocate ~2^64 bytes without it — the
  bug-restoration check (task 5.2) exercises the same guard class (bound removed → red).
- Discrepancies: none. No row of the table required an implementation accommodation.
