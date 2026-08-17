# Session 2 — Sharded review (6 axes)

Scope: `git diff ad6fa79 -- matrix.hpp tests/cases/load_npy.hpp` (single hunk at
`matrix.hpp @@ -2509 +2509` = the `load_npy( char const* )` body; test file append-only).
Contract: `docs/session_2_contract.yaml`; specs in `specs/`. Process note: no subagent tool in
this environment — the six axes were run in-session as six separate passes, each re-deriving its
verdict from the diff + contract only (deviation recorded in the handoff).

Method: each axis was run against the prompt's question list
(`docs/prompts/sharded_review.md`). Findings below are the deduplicated actionable set;
non-findings per axis are summarized after.

## Findings

### F1 — Low (fixed during this review)
- **Axis:** Tests
- **Severity:** Low
- **Location:** `tests/cases/load_npy.hpp` (case 1) / spec `specs/load_npy_validation.md`
  R-V1 scenario "non-NPY magic of sufficient size rejected"
- **Evidence:** neither the suite nor the 18-case probe pinned a ≥12-byte bad-magic file (e.g.
  16×0xAA) — the magic-compare branch was untested; only the size gate (3B/11B) and the
  version/dtype gates had content.
- **Violated clause:** contract `evidence` (T2: negative-path cases assert content); spec R-V1
  scenario without a test.
- **Impact:** a regression that swapped the magic bytes or skipped the compare would not fail
  any test (bad-magic files would then fall through to version/length checks and be rejected
  anyway in most cases — hence Low, not Medium).
- **Smallest safe fix (applied):** added a 16-byte `0xAA` file to case 1
  (`REQUIRE( !m.load_npy( path_badmag.c_str() ) )`) + cleanup.
- **Confidence:** high.
- **Post-fix re-run:** full suite green (64 cases, 49,216,811 assertions).

### F2 — Low (fixed before this review)
- **Axis:** Tests
- **Severity:** Low
- **Location:** spec `specs/load_npy_validation.md` R-V7 scenario "zero dimension rejected"
- **Evidence:** `(0, 2)` had no suite or probe case; the `row == 0 || col == 0` gate (D8) was
  untested.
- **Impact:** a regression removing the zero-dim gate would silently allow `resize(0, ·)`
  territory (unverified library behavior).
- **Smallest safe fix (applied):** added `(0, 2)` as the 6th variant of case 3.
- **Confidence:** high.

### F3 — Info (no change)
- **Axis:** Readability
- **Severity:** Info
- **Location:** `matrix.hpp` R-V3 block — two separate `if ( version == 2 )` lines building the
  4-byte length.
- **Evidence:** could be one `if` with two `|=` lines; current form is explicit and each line
  independently reviewable.
- **Impact:** none (no behavior difference; no maintainability blocker).
- **Confidence:** high. Not fixed: the extra one line is clearer for a security review than a
  merged branch.

### F4 — Info (no change; documented design decision)
- **Axis:** Security / Correctness
- **Severity:** Info
- **Location:** `matrix.hpp` — `try { … } catch ( … ) { return false; }` around the whole
  validated region.
- **Evidence:** `catch(…)` could mask a genuine bug inside the region.
- **Impact:** by design (D5): at an I/O shell boundary the contractually required outcome for
  *any* exception is `false` (noescape-from-noexcept, P3). Bug-restoration check (task 5.2) and
  the 18-case probe prove the *checks* — not the catch — do the work; a latent parse bug would
  surface as a spurious `false`, detectable via the probe's content pins.
- **Confidence:** high.

### F5 — Info (no change; scope note)
- **Axis:** Architecture / Security
- **Severity:** Info
- **Location:** `matrix.hpp` `crtp_load_binary` (~2469) — adjacent, **not** in this diff.
- **Evidence:** `load_binary` has the same hazard class (overflow-unguarded
  `sizeof(r)+sizeof(c)+sizeof(Type)*zen.size()` with file-controlled `r/c`; no dtype check for
  `Type`).
- **Impact:** none for this session (out of blast radius; "any other finding"). Recorded in
  `docs/risk_register.md` S2 watch items for a future session.
- **Confidence:** high.

## Non-findings summary (per axis)

- **Correctness:** all R-V1…R-V9 verified against the diff line by line: size≥12 precedes every
  deref (indices 6, 8–11); non-wrapping bound form (`> size − prefix`, both occurrences);
  `descr_end − descr_pos − 10` cannot underflow (find start ≤ result); `col_pos_end >
  row_pos_end` (the comma is not `)`) so the col-token substr cannot underflow; digit parser
  overflow check `(max − d)/10` correct for d ≤ 9; `row > max/col` precedes the multiply with
  `col ≥ 1` already proven; `data_offset ≤ size` by the header bound; copy range
  `[data_offset, data_offset+payload)` ⊆ buffer and exactly `elements` values ⊆ `zen` (resize
  precedes it); `row_major` expression preserved verbatim (D10); resize/reshape/copy order
  preserved. All tests pass (64/64) and assert content + state, not just return.
- **Readability:** names match the pre-change code (`header_length`, `data_offset`,
  `row_pos`/`col_pos_end`) and the spec IDs in comments point to `specs/`; control flow is one
  flat validate-then-act sequence (no nesting beyond the try/lambda); the `parse_dim` lambda
  is self-contained; no dead code, no back-compat shims. The dtype if-constexpr chain is long
  but is the whole dtype policy — a table lookup would be a second indirection for 10 entries
  (abstraction not earning its keep at this count).
- **Security/safety:** every file-controlled quantity (version, header_length, header bytes,
  dtype, shape digits) is gated before use; no OOB reachable (indices provably in-bounds at
  each deref); no `stoul`/throws from file input; allocation bounded by file size (header
  string ≤ size; resize ≤ size/sizeof(T) elements); `noexcept` honest in all build modes
  (debug-mode assert-abort removed per D12, proven by the missing-file case in the assert-enabled
  suite build).
- **Tests:** 5 cases + 6 shape variants + bad-magic + missing file; each asserts `ok==false`
  **and** unchanged state (row/col/sampled values after a prior valid load — non-tautological);
  cleanup per case; happy block byte-identical; probe (ASan, NDEBUG) + suite (debug, asserts)
  together cover the contract's evidence list; zero-dim and bad-magic gaps closed (F1, F2).
- **Architecture:** single hunk confined to the contracted body (Deliveries 1); no new public
  symbols/includes; inline validation matches the in-repo models (`load_binary`, `load_bmp`);
  no feature logic leaked into shared code; the CRTP `value_type` is the explicit type boundary
  (if-constexpr map, no silent fallback — unknown value_type rejects all).
- **Performance:** single buffer read (unchanged); header parse is linear `find`/`substr` on a
  ≤file-size string (same as pre-fix); the magic loop is 6 iterations; the byte-level `copy_n`
  compiles to one memcpy (the pre-fix per-element `copy_n<value_type>` was the same memory
  traffic); no new allocations beyond the pre-existing buffer + header string + resize.

**Verdict:** 0 Critical, 0 High, 2 Low (both fixed and re-verified), 3 Info (documented, no
change). Review passes.
