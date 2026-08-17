# Session 2 — Design

## Context

- **Current state:** `crtp_load_npy::load_npy(char const*)` (`matrix.hpp:2508`) is a `noexcept`
  member that (a) dereferences `buffer.data()+6/8/9/10/11` with no size check, (b) builds the
  `header` string from file-controlled `header_length` with wrapping arithmetic, (c) parses shape
  with `npos`-unguarded `find` + `stoul` (throws out of `noexcept` → terminate), (d) never checks
  dtype, (e) resizes from file-controlled shape, and (f) copies `row*col` elements of
  file-controlled total size. Pre-fix reproduction evidence: `.work/evidence/prefix_*`.
- **In-repo models:** `load_bmp` (`matrix.hpp:6643`) — validate header/size consistency before
  parsing, return failure (empty optional) early; `load_binary` (`matrix.hpp:2469`) — the member
  pattern: `noexcept` + `better_assert` debug message + hard `if (…) return false;` + `int8_t*`
  byte copy. The review names `load_bmp` as the pattern to follow (evidence map §S1).
- **Constraints:** PRD §5 row 18 (rejection-only behavior change), P2 (hard checks at I/O
  boundaries, never asserts for behavior), P3 (untrusted-input rule; non-wrapping bound style),
  contract `blast_radius` (body-only diff; `tests/test.cc` NOT in allowed files), happy-path
  invariance (4 fixtures byte-identical results), `noexcept` kept (API policy §3: no signature
  change), diff-auditable vs baseline `ad6fa79`.
- **Stakeholders:** fresh-context verifier (attacker mindset), S6 (ReadMe single writer, consumes
  the doc delta), S5 (`save_png` sibling boundary, warned in handoff), future sessions (R-02
  anchor hygiene).

## Goals / Non-Goals

**Goals**
1. No file byte dereferenced before its bound is proven (validate-then-act).
2. `load_npy` returns `false` for every input in the contract invariant list; never throws,
   aborts, or UBs; `noexcept` honest.
3. Happy path behaviorally identical (4 fixtures + v2-convention/fortran/payload-tail pins).
4. 5 content/state-asserting negative tests in the suite; E03/E04 live and ASan-clean.
5. Deterministic, re-runnable evidence chain (suite, ASan probe, diff audit, grep counts).

**Non-Goals**
- Real npy v2 wire format (8-byte length) adoption or v2 rejection (brainstorming Q2).
- Big-endian byte-swapping / any dtype translation (rejection only).
- Zero-dim support, N-D (1-D/3-D) npy support (rejection only; library is 2-D, non-zero dims).
- Fixing `load_binary`/`load_txt` siblings (their overflow-unguarded arithmetic noted as a
  watch item only — out of scope, any other finding).
- `ReadMe.md` edits (P4 — delta emitted), `examples/**`, `Makefile`, new dependencies.

## Decisions

| # | Decision | Chosen | Alternatives | Rationale |
|---|---|---|---|---|
| D1 | Validation location | Inline in the `load_npy(char const*)` body | Private free helper `npy_header_parse` (pure Calculation) | Deliveries 1: "diff confined to the function body"; one consumer (YAGNI); in-repo siblings keep validation inline. Deeper split belongs to the deferred A1 teardown project. |
| D2 | v2 wire convention | Keep in-code convention (4B LE length @8, prefix 12) | Real-spec 8-byte/16; reject v2 | Contract failure mode pins both 10/12 offsets; real-spec = unsanctioned wire change; reject = violates "version in {1,2}". Real-spec v2 files now reject via D6 (improvement, row 18). |
| D3 | Shape parse | Digit-bounded parser (no sign, no `size_t` overflow, optional leading whitespace) replacing `stoul` | Keep `stoul` inside `try/catch` | Define-away (tier 1) beats masking (tier 3); `stoul` accepts `-` (empirically: `bad_array_new_length` terminate) and its `out_of_range` is a second throw source; the parser is ~8 lines. Contract intent (no throw) satisfied more strongly; residual `catch(…)` retained (D5). |
| D4 | Payload/element arithmetic | Overflow-checked multiply (`row > SIZE_MAX/col` → reject; `elems > SIZE_MAX/sizeof(T)` → reject), then `payload > buffer.size() - data_offset` (non-wrapping) | `__builtin_mul_overflow`; `size_t`-wide assume | Portable, matches the contract's mandated non-wrapping bound style; the two extra `size_t` comparisons are the whole defense (3rd hazard, brainstorming Q1). |
| D5 | Throw containment | `try { … } catch (… ) { return false; }` around buffer-build→copy | Catch only `std::exception` around `stoul` sites | With D3/D4, residual throws are allocation-only and bounded by file size; one documented aggregate at the I/O shell (functional-thinking: mask at the shell) makes `noexcept` honest for *all* inputs; `catch(…)` also covers non-std exceptions. |
| D6 | Real-spec-v2 shift hole | Header dict sanity: `header[0] == '{'` | Reject version 2; byte-swap support | Closes the 4-byte-shifted silent misload of real v2 files without a wire change; npy spec says the header is a dict literal; fixtures pass. |
| D7 | dtype check | Exact match of the descr field against the canonical descriptor per `value_type` (brainstorming Q3 map); descr field parsed positionally (`'descr': '` + quoted value, npos-guarded) | Substring search for the expected dtype | Substring would accept an attacker-planted token outside the descr field; positional parse + exact match is strict and fixture-compatible. |
| D8 | Zero-dim shapes | Reject (`row ≥ 1 && col ≥ 1`) | Accept `resize(0,·)` | Library non-zero-dim policy (S1 watch item); avoids unverified `resize(0,·)` territory; no fixture/seed affected. AMBIGUITY resolved, logged. |
| D9 | Copy form | `std::copy_n(reinterpret_cast<std::int8_t*>(…), payload, reinterpret_cast<std::int8_t*>(zen.data()))` | Keep `copy_n<value_type>` | Identical bytes (payload = row·col·sizeof(T)); removes unaligned strict-typed loads; in-repo pattern (`load_binary` ~2496). |
| D10 | `row_major` detection | Preserved verbatim (`header.find("T") != npos`) | Rewrite as proper `'fortran_order': True` search | Happy-path invariance; with D7 (dtype set) + D3 (digit shapes) the `'T'` source is uniquely the fortran value; pinned by the `e03_fortran` probe case. |
| D11 | Failure-time matrix state | `resize` strictly after all checks; negative tests assert row/col unchanged | — | Contract failure mode "stoul exception path returns true by accident (resize already applied) — reject before zen.resize". |
| D12 | Open-failure handling | Remove `better_assert( ifs, … )` from `load_npy`; hard `if ( !ifs ) return false;` only | Keep the assert as the debug-message layer | `better_assert` = `print_assertion` = print + **`abort()`** when `debug_mode` (i.e. without `-DNDEBUG`) — confirmed by the TDD red run: the pre-fix suite build (asserts enabled) **SIGABRTs** on a missing file (evidence `tdd_red_run.log`). The contract invariant "unopenable path → clean `false`" carries no build-mode qualifier; an assert-abort on attacker input is the S1 hazard class (process death), so the I/O boundary keeps the hard check only. Diagnostics for a failed open are not worth a process death. |

## Risks / Trade-offs

- [Happy-path regression via over-strict dtype/shape checks] → fixtures hex-dumped from bytes
  (exact descr strings + `(2, 3)` layout with the space after the comma); the 4 existing TEST_CASE
  blocks are byte-untouched and green in the full suite; probe pins (`e03_exact`, `e03_fortran`,
  `e03_v2`) cover non-fixture valid files. **Leading whitespace in shape tokens is required**
  (numpy writes `(2, 3)`); trailing whitespace is rejected — numpy's writer never emits it.
- [Trailing-whitespace strictness rejects some hand-crafted files] → documented in the spec
  (V7) and the handoff; such files are malformed under the reference writer; rejection is the
  P3-compliant outcome.
- [v2 divergence from the real npy spec] → real-spec v2 files now `false` (pre-fix: shifted
  misload — strictly safer); disclosed via the S6 doc delta wording.
- [`catch(…)` swallows a genuine bug inside the parse region] → masking at the I/O shell is the
  designed behavior (P2/P3); the suite + probes + bug-restoration check (plan §5.3) prove the
  rejection logic, not the catch, does the work.
- [Removal of `better_assert` loses the debug open-failure message] → accepted: the message's
  only channel was an `abort()` (D12); the hard check returns `false` in every mode, which is
  what callers (and the new tests) can rely on.
- [`resize` partial state on `bad_alloc`] → the invariant only requires false/throw-free/UB-free;
  `resize`'s own exception safety is out of scope (pre-existing library property).
- [Crafted test files left in `tmp/` on a crashing run] → each case removes its own file; `tmp/`
  is gitignored so no audit pollution; a crashing pre-fix run is expected (TDD red) and recorded.
- [Anchor drift (R-02)] → `load_bmp` re-anchored at 6643 (review said ~6760); all edits keyed by
  function name, lines as hints; discrepancy logged here + handoff.

## Migration Plan

Single branch `phase-1/session-2` off baseline `ad6fa79`; commits: pre-flight checkpoint (phase
docs + probe + pre-fix evidence) → TDD-red tests → fix → full checks → closeout. Rollback:
`git reset --hard ad6fa79` (no data migration; no persisted state touched). Merge gated on the
human decision (contract exit 5).

## Open Questions

None blocking. Residuals are recorded as decisions (D2 real-spec-v2 disclosure to S6; D8 zero-dim
interpretation; 3rd hazard documentation) and ride in the handoff decision log.
