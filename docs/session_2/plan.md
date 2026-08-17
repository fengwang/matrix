# Session 2 — Plan

Micro-task TDD plan for `tasks.md`. Context: `design.md` (decisions D1–D11); specs in `specs/`.
Baseline: `ad6fa79` on `phase-1/session-2`. All commands run from the repo root.

## Task 1 — Pre-flight (already executed; commit point: pre-flight checkpoint)

Done. Evidence: `.work/evidence/prefix_*` (18 per-case logs), `.work/evidence/prefix_compile.log`,
fixture header hexdumps (session transcript), phase docs `docs/session_2/**`, probe
`.work/probes/E03_E04.cc`.

**Commit:** `S2 pre-flight: phase docs, E03/E04 probe, pre-fix reproduction evidence (4 ASan OOB,
4 terminate paths, 4 silent misloads, 4 pins); third hazard confirmed (shape/payload overflow)`.

## Task 2 — Negative tests (TDD red)

**2.1** Append to `tests/cases/load_npy.hpp` (after the existing `TEST_CASE`; do not touch it):

- Helpers (file-local, in the append block): `write_bytes( path, vector<uint8_t> )` via binary
  `std::ofstream`; `make_v1( header, payload )` = magic + `01 00` + LE16 length + header +
  payload; `make_v2( header, payload )` = magic + `02 00` + LE32 length + header + payload;
  `dict_header( descr, shape, fortran )` builds `{"descr": '<descr>', 'fortran_order': False,
  'shape': <shape>, }` with numpy's spacing. `std::filesystem::create_directories("tmp")` at the
  top of the block.
- Five `TEST_CASE`s, tag `"[load_npy]"`; each: fresh matrix, capture `r0/c0`, write
  `tmp/s2_neg_<id>.npy`, `REQUIRE( !m.load_npy( path.c_str() ) );`,
  `REQUIRE( m.row() == r0 && m.col() == c0 );`, `std::filesystem::remove( path )`.
  1. `load_npy rejects an unopenable or truncated (3-byte) file` — missing path + `{0x93,'N','U'}`.
  2. `load_npy rejects a truncated header` — 11-byte (ver 1, len 0xFFFF, 3 tail bytes) +
     21-byte (len 80, only 11 header bytes; the E03 seed's second file).
  3. `load_npy rejects a missing or malformed shape token` — no-shape header; `(2,)`; `(-1, 2)`;
     `(a, b)`; 30-digit row.
  4. `load_npy rejects an overflowing header_length (0xFFFFFFFF)` — v2 layout, len bytes
     `FF FF FF FF`, 4 header bytes + 2 payload bytes.
  5. `load_npy rejects a foreign dtype` — well-formed 1×2 `<f4` file (8-byte payload) into
     `matrix<double>` (E04 acceptance) + `>f8` file into `matrix<double>`.
- New includes for the append block: `<cstdint>`, `<filesystem>`, `<fstream>`, `<string>`,
  `<vector>`.
- Each new case first loads `./images/64.npy` (valid 2×3 baseline) and re-checks row/col and
  sampled values after the rejected loads — pinning "no resize before rejection" on a
  non-trivial state.

**2.2** Red run:
```sh
make test 2>&1 | tail -2
./test_test "[load_npy]" 2>&1 | tee .work/evidence/tdd_red_run.log | tail -30
```
Expect: the existing happy case passes; the 5 new cases fail or crash (record which mode:
terminate / garbage-`true` / clean-false-that-doesn't-exist-yet). Verify
`git diff tests/cases/load_npy.hpp` is append-only (existing block byte-identical).

**Commit after 2.2:** `S2 task 2: 5 negative load_npy cases (TDD red pre-fix; happy path untouched)`.

## Task 3 — Fix `crtp_load_npy`

**3.1** Single edit to the body of `load_npy( char const* const file_name ) noexcept`
(`matrix.hpp` ~2508–2566; keep the `std::string` overload and both signatures; no other hunk).
Validate-then-act sequence per specs R-V1…R-V9 and design D1–D11, in order:

1. keep the open attempt; hard `if ( !ifs ) return false;` **only** — `better_assert( ifs, … )`
   is removed (D12: it prints + `abort()` in debug builds; contract requires clean `false` in
   every mode)
2. read whole buffer (existing pattern)
3. `buffer.size() < 12 → false`; magic compare via `std::uint8_t` (6 bytes)
4. `version = buffer[6]`; `version != 1 && version != 2 → false`;
   `data_prefix = version == 1 ? 10 : 12`
5. read `header_length` from version-appropriate bytes (LE16 @8 / LE32 @8 via a small
   body-local lambda over `std::uint8_t`)
6. **`if ( header_length > buffer.size() - data_prefix ) return false;`** (non-wrapping; P3)
7. `header = string(buffer.data() + data_prefix, header_length)`;
   `if ( header.empty() || header[0] != '{' ) return false;`
8. dtype: positional parse of `'descr': '` field (both `find`s npos-guarded, non-empty value);
   compare against the expected canonical descriptor for `value_type` (body-local constexpr
   if-chain; unknown `value_type` → always false)
9. shape: `s = header.find("'shape': (")`; npos → false; `comma = header.find(',', s)`,
   `close = header.find(')', comma)`; npos → false; tokens `header.substr(s+10, comma-…)`,
   `header.substr(comma+1, close-…)`; body-local `parse_dim( string_view ) -> optional-ish
   (bool out + size_t)` digit-bounded parser (skip leading space/tab; digits only;
   `v > SIZE_MAX/10` or `v*10 + d > SIZE_MAX` → fail; empty → fail)
10. `if ( row == 0 || col == 0 ) return false;`
11. overflow-checked `elements`/`payload` (D4: `row > SIZE_MAX/col`, `elements >
    SIZE_MAX/sizeof(value_type)` → false); `data_offset = data_prefix + header_length`;
    **`if ( payload > buffer.size() - data_offset ) return false;`**
12. `row_major = header.find("T") == npos` (preserved verbatim); `zen.resize( row, col );`
    then `if ( !row_major ) zen.reshape( col, row );` (pre-change order)
13. byte copy: `std::copy_n( reinterpret_cast< std::uint8_t* >( buffer.data() + data_offset ),
    payload, reinterpret_cast< std::uint8_t* >( zen.data() ) );` (payload = row·col·sizeof(T))
14. whole region 2–13 inside `try { … } catch ( … ) { return false; }`; `return true;`

No new `#include` (all needed headers already included: cstring, cstdint, limits, vector,
string, fstream).

**3.2** Green:
```sh
make test 2>&1 | tail -2
./test_test "[load_npy]" 2>&1 | tail -10        # expect: 9 cases, 0 failed
make test >/dev/null && ./test_test 2>&1 | tail -4   # full suite: 64 cases
```

**3.3** ASan probe:
```sh
make .work/probe_s2
.work/probe_s2 2>&1 | tail -3                   # expect: PASS E03 + PASS E04, exit 0
```

**Commit after 3.3:** `S2 task 3: load_npy validated input boundary (S1 finding + 3rd hazard:
no deref before size checks; non-wrapping header bound; dtype match; digit-bounded shape;
overflow-checked payload; resize after validation; throw-free noexcept)`.

## Task 4 — Full checks + audit

```sh
g++ --version | head -1
make test 2>&1 | tail -2
./test_test 2>&1 | tee .work/evidence/final_suite_run.log | tail -4
git diff --name-only ad6fa79 -- matrix.hpp tests docs .work | tee .work/evidence/diff_audit.log
sed -n '2499,2590p' matrix.hpp | grep -c 'return false'
git diff ad6fa79 -- tests/cases/load_npy.hpp | head -8     # first lines must be context/append only
```
Pass criteria: suite 64/64; name-only ⊆ allowed set (`matrix.hpp`, `tests/cases/load_npy.hpp`,
`.work/**`, `docs/session_2/**`, `docs/eval_seed_cases.md`, `docs/risk_register.md`);
`matrix.hpp` hunk = the `load_npy` body only (`git diff ad6fa79 -- matrix.hpp` shows one hunk
starting inside the function); grep count > 6.

## Task 5 — Independent derivation + bug restoration

**5.1** Independent writer pass (fresh framing): from `session_2_contract.yaml` + P3 checklist
+ npy wire facts only (do NOT read the Task 3 diff), list expected accept/reject per crafted
input; compare with the suite's expectations; record concurrences/discrepancies in
`.work/independent/derivation.md`. (No subagent tool in this environment — run in-session with a
disciplined fresh framing; deviation from the subagent protocol is recorded in the handoff.)
**5.2** Bug restoration: temporarily remove the `header_length` bound (edit → test → revert);
the 0xFFFFFFFF case must go red/crash; re-verify green after restoring. Never commit the temp
state.

## Task 6 — Sharded review (6 axes)

Per `docs/prompts/sharded_review.md`; run all 6 axes over `git diff ad6fa79 -- matrix.hpp
tests/`; findings (axis, file:line, severity, verdict) → `docs/session_2/sharded_review.md`;
fix High/Critical only; re-run Task 4 checks after any fix. Same no-subagent note as 5.1.

## Task 7 — Adversarial verification

Per `docs/prompts/adversarial_verifier.md`; the verifier sees only: contract + PRD row 18, the
diff, the evidence (suite log, probe output, pre-fix prefix logs) — not the implementation
rationale docs. Focus list: attacker-chosen 3B/11B/12B files, 0xFFFFFFFF length, exact
boundaries (header-length = remainder; payload = tail), missing shape token, big-endian dtype,
real-spec v2 file, zero/negative/overflow shapes. Verdict →
`docs/session_2/adversarial_verification.md`. FAIL → `docs/prompts/failure_arbiter.md` first
(root cause + evidence before any fix).

## Task 8 — Closeout

- `docs/eval_seed_cases.md`: E03/E04 rows → `promoted` (probe `.work/probes/E03_E04.cc`;
  permanent home `tests/cases/load_npy.hpp`).
- `docs/risk_register.md`: S2 watch items — (a) 3rd hazard class now covered, keep in rotation;
  (b) **adjacent finding, out of scope:** `load_binary` (matrix.hpp ~2477–2493) has the same
  overflow-unguarded `sizeof(r)+sizeof(c)+sizeof(Type)*zen.size()` arithmetic with
  file-controlled `r/c` and no dtype check for `Type` — document only (any other finding).
- `.work/handoff_session_2.md` per `docs/templates/handoff.md`: snapshot (compiler g++ 16.2.1),
  done/undone, checks run vs not run, decision log (pre-fix evidence map; 3rd hazard; v2
  convention kept; zero-dim rejected; `docs/handoff.md` outside blast radius → `.work/` path,
  per project contract §1.4; no-subagent deviation), S6 doc deltas (exact ReadMe §"load npy"
  ~1055 replacement line incl. dtype-match + real-spec-v2 rejection note), S5 warning
  (`save_png` is the only other I/O boundary; apply the same validate-then-act pattern).
- Final: re-run Task 4 + probe; verify done condition (contract `done_condition` verbatim);
  final commit; present diff + evidence for the human decision gate.
