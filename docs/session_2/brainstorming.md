# Session 2 — Brainstorming (refinement record)

Status: refinement only (policy P9 — narrow/clarify, no scope widening). The problem space was
explored in the 2026-08-17 blueprint interview (PRD §2) and the 2026-07-13 sharded review (§S1).
This document records the session-start interview-me pass, the design decisions, and the validated
design. No new exploration.

**Process note (interview-me skill, non-interactive context):** this session runs the contracted
autonomous lifecycle from a single user prompt; there is no live grilling channel. The project-level
intent interview (PRD §2, explicit user yes) already fixed the *what*. The pass below therefore
stress-tests the contract set against every residual decision point instead of asking the user;
nothing remains that only a user answer could resolve. Confidence below 95% would have stopped the
session with a blocker instead of guessing.

## Interview-me pass (stress-test of my thinking)

HYPOTHESIS: the user wants the S2 contract executed end to end — `load_npy` becomes a validated
input boundary (P3), the T2 negative-path gap is closed with content/state-asserting tests, E03/E04
are live and ASan-clean, and the whole chain is evidence-backed for the human decision gate — with
zero scope creep beyond `docs/session_2_contract.yaml`.
CONFIDENCE: **96%** — every decision point below is fixed by the contract set; pre-flight probes
reproduced all listed hazards *and one more* (third hazard, empirically confirmed). Nothing blocks.

| # | Question | Resolution (source) |
|---|---|---|
| 1 | Which exact code changes? | Re-anchored by grep: `crtp_load_npy` at `matrix.hpp:2499`, members at 2504/2508. Review structure confirmed: fixed-offset derefs at `buffer.data()+6/8/9/10/11`, unguarded `stoul`, no dtype check, no size checks before deref. The two extra hazards (C-11) hold: `header_length` taken from file bytes feeds `12 + header_length` arithmetic (0xFFFFFFFF wraps); `header.find("'shape': (")` is `npos`-unguarded → `stoul` throws from the `noexcept` member → `std::terminate`. **Third hazard found + confirmed this turn:** shape digits pass through `stoul`, which accepts a leading `-` — `(-1, 2)` → `stoul("-1")` = 2⁶⁴−1 → `resize` throws `bad_array_new_length` → terminate (evidence `prefix_e03_negshape.err`). `row*col`/`payload` arithmetic is therefore overflow-unguarded too. |
| 2 | What does "version in {1,2}" mean for the v2 wire format? | Keep the **existing in-code convention**: v1 = 2-byte LE length @8, prefix 10; v2 = 4-byte LE length @8, prefix 12. Contract `failure_modes_to_watch` ("version-1 vs version-2 offset (10 vs 12) mixed up") pins both prefixes as staying. Adopting the real npy v2 spec (8-byte length, prefix 16) would be an unsanctioned wire-format behavior change; rejecting v2 would violate "version in {1,2}". Consequence: real-spec v2 files are now **rejected** (clean `false`) instead of silently misloaded 4 bytes shifted — an improvement sanctioned by PRD §5 row 18 (malformed → false). The `{` header sanity check (Q11) is what makes this sound. |
| 3 | Which dtype strings are accepted? | The canonical little-endian descriptor per target `value_type`: `uint8_t |u1`, `int8_t |i1`, `int16_t <i2`, `uint16_t <u2`, `int32_t <i4`, `uint32_t <u4`, `int64_t <i8`, `uint64_t <u8`, `float <f4`, `double <f8`. Fixture headers hex-dumped this turn confirm exactly `|u1` / `|i1` / `<f4` / `<f8` (note: `8.npy` is **int8** `|i1` loaded into `matrix<std::int8_t>`). Any other value_type accepts nothing (→ always false). |
| 4 | How is the `stoul` hazard handled? | **Define away** (error tier 1): a digit-bounded parser (non-empty, optional leading whitespace, digits only, no sign, no `size_t` overflow) replaces `stoul`. The contract clause "stoul wrapped (catch) → return false" is intent (no throw escapes the `noexcept` member); defining the hazard away satisfies it more strongly. A residual `try/catch(...) → false` around the parse/resize/copy region still covers allocation throws (`bad_alloc`) — see Q10. |
| 5 | Are zero-dim shapes `(0, k)` / `(k, 0)` accepted? | **Reject.** The library's dimension policy is non-zero (S1 closeout watch item: asserts non-zero dims; constructor policy not verified elsewhere); `resize(0, ·)` is unverified territory; no fixture or eval seed uses a zero dim. Documented interpretation (AMBIGUITY resolved in spec V7). |
| 6 | What happens at `header_length == buffer.size() - data_prefix` (exact boundary)? | The bound check is **inclusive** (passes); subsequent header-content checks (dict sanity, dtype, shape) then decide — in practice a header that consumes the whole file fails shape/dtype and returns `false`. Documented in spec V3 + verifier brief. |
| 7 | What happens at `payload == buffer.size() - data_offset` (exact boundary)? | **Accepted** (the payload ends exactly at the file tail — a well-formed file). One byte short → reject. These are the contract's `adversarial_cases` boundary pair; pinned by probe cases `e03_exact` / `e03_short`. |
| 8 | Does the dtype/shape validation change the `row_major` (`"T"`) detection? | No — the expression `header.find("T") != npos → fortran` is preserved **verbatim**. With dtype restricted to the accepted set and shape restricted to digits, the only `'T'` source in a well-formed header is the `fortran_order: True/False` value; semantics are pinned by the `e03_fortran` probe case (2×3 fortran file loads as the transpose). |
| 9 | Which copy form replaces `std::copy_n<value_type>`? | Byte-level copy via `std::int8_t*` (the in-repo pattern of sibling `load_binary` at ~2496 and the `load_txt`-adjacent helper at ~2494). Identical bytes land in `zen`; removes the formally-undefined unaligned strict-typed load on strict-alignment targets. Same diff containment (function body only). |
| 10 | Does `noexcept` survive, and where does the catch go? | `noexcept` stays (no signature change — API policy §3). The body from buffer construction through the copy is wrapped in `try { … } catch (… ) { return false; }`: with all arithmetic validated, every remaining allocation is bounded by file size, so the member is now genuinely throw-free (contract in_scope: "body stays throw-free so `noexcept` remains honest" — strengthened). `better_assert(ifs, …)` is **removed** (D12 — red-run evidence: it prints + `abort()` in `debug_mode` builds and the pre-fix suite build SIGABRTs on a missing file); the hard `if ( !ifs ) return false;` is the only open-failure behavior, in every build mode. |
| 11 | What closes the real-spec-v2 shift hole? | Header dict sanity: `header[0] == '{'` (npy spec: the header is a Python dict literal). A real-spec v2 file read under the library convention yields a "header" starting with the high bytes of the 8-byte length field (NUL for small lengths) → rejected. Fixtures start with `{'descr'` → pass. |
| 12 | Where do the negative tests live, and how many? | Appended to `tests/cases/load_npy.hpp` — **exactly 5** `TEST_CASE`s (contract list: truncated magic 3B; truncated header; malformed/missing shape token; `header_length=0xFFFFFFFF`; float32-into-double), each additionally asserting matrix state unchanged. The file is already registered at `tests/test.cc:35` → **no `test.cc` change** (which is fortunate: `test.cc` is not in `allowed_files`). Crafted bytes written at runtime into `tmp/` (gitignored) and removed per case (determinism failure-mode). The missing-file invariant is pinned inside case 1 and in the E03 probe. |
| 13 | What probes carry the acceptance? | `.work/probes/E03_E04.cc` — 18 selectable cases (the contract's deterministic check compiles exactly this path): 9 reject-class, 4 dtype-class, 5 boundary/pin-class (missing file, exact payload, short payload, fortran, v2-convention). Pre-fix runs recorded per case before any code edit. |
| 14 | Branch / human gate? | Work on existing branch `phase-1/session-2`; baseline `ad6fa79` (S1 closeout) is the diff-audit reference. Human decision gate (high risk): final message presents diff + evidence; no merge before sign-off (contract exit 5). |
| 15 | Is `docs/handoff.md` (repo root) in scope? | **No.** The session-end protocol names it, but `blast_radius.allowed_files` does not, and the project contract §1.4 (higher authority) specifies `.work/handoff_session_{n}.md`. The contract wins; the handoff goes to `.work/handoff_session_2.md` (same as S1). Recorded so the discrepancy is explicit, not silent. |

## Context exploration (budget-conform)

- `matrix.hpp` regions read: 1–40 (includes: `cstdint`, `type_traits`, `limits`, `cstring`,
  `filesystem` all present), 2416–2585 (`load_txt`/`load_binary`/`crtp_load_npy` — the changed code
  plus both in-repo boundary patterns), 6643–6692 (`load_bmp` model — re-anchored by grep at 6643;
  the review's ~6760 is stale, R-02: names are authoritative).
- `tests/test.cc:35` (registration line), `tests/cases/load_npy.hpp` full (4 happy cases).
- `Makefile` (build recipe; `make test` builds `test_test`, `./test_test` runs it), `.gitignore`
  (`tmp/*`, `.work/` ignored).
- Fixture headers: 4× `images/*.npy` hex-dumped ≤64B each (magic/version/descr/shape offsets from
  bytes, per pre-flight 4).
- `docs/`: prd §5 row 18 + §7 (P2–P4, P8), project contract, `session_2.md` + contract, evidence
  map §S1 + C-11, eval seeds E03/E04 rows, risk register, workflow prompts (arbiter/review/
  verifier/harvest), handoff template, `.work/handoff_session_1.md` + `docs/session_1/*` (format
  precedent only).
- **Not read:** rest of `matrix.hpp`, `ReadMe.md`, deep-research docs, the full review report
  (evidence map + contract carry the S1 finding), `examples/**` (budget map).

## Design decisions (validated)

1. **Validate-then-act boundary** (the `load_bmp` model adapted to the member pattern of
   `load_binary`): every file byte is dereferenced only after the preceding bound is proven;
   every failure path `return false`s before `resize`; `resize` moves after all checks.
2. **Diff confined to the `load_npy(char const*)` body** (Deliveries 1). The dtype map and the
   digit-bounded shape parser are private, body-local (constexpr if-chain + lambda) — no new public
   symbols, no new includes, no helper outside the body.
3. **Hard checks only, never asserts, for behavior** (P2): `better_assert` remains the
   debug-message layer only; each real check is a plain `if (…) return false;`.
4. **Error handling:** define-away for the shape digits (tier 1); one documented aggregate at the
   shell (`try/catch(…) → false`, tier 3 at the boundary) — the I/O boundary is exactly where
   masking belongs (functional-thinking: masking is an Action at the shell, never in the
   Calculation; the parse region is a pure Calculation inside the Action).
5. **Tests assert state, not just return value:** every negative case captures `row()/col()`
   before the call and requires them unchanged after (the "resize already applied" failure mode is
   pinned, not just avoided).
6. **No doc delta to `ReadMe.md`** (P4): exact replacement wording for the ReadMe §"load npy" line
   (~1055) is emitted in the handoff for S6 to consume, including the real-spec-v2 rejection note.

## Approaches considered

- **A1 (chosen): body-only validate-then-act rewrite.** Minimal blast radius (one function body +
  one test file), every check independently testable via the probe case selector, matches both
  in-repo models. The function becomes one deep Action facade whose Calculation core (parse/
  validate) is testable through the public boundary — no new surface.
- **A2: extract a private free helper `npy_header_parse(buffer, …) -> optional<…>`** (deeper ACD
  split; parse = pure Calculation, load = thin Action). *Rejected:* the diff would extend beyond
  the function body (Deliveries 1), the helper has exactly one consumer in this session (YAGNI),
  and the in-repo siblings keep validation inline — consistency wins. Revisit inside the deferred
  A1/CRTP-teardown project if `load_*` boundaries get a shared validator.
- **A3: adopt the real npy v2 wire format (8-byte length) or reject v2 outright.** *Rejected:*
  contract failure mode pins both 10/12 offsets; rejecting v2 violates "version in {1,2}"; the
  real-spec adoption is an unsanctioned behavior change (R-03: stop and report). The `{` sanity
  check delivers the safety goal of A3 without the wire change.
