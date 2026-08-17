# Session 2 — Input Boundary: `load_npy` Validation (finding S1)

> Story outline v1. Refine at start (narrow/clarify only), policy P9.
> Machine-readable authority: `docs/session_2_contract.yaml`. Law: `docs/project_contract.md`.

## Objective

Make `load_npy` a validated input boundary: no dereference of file bytes before a size/shape/dtype check, no throwing out of `noexcept`, no UB on truncated or malformed files. Ship negative-path tests (the T2 gap) and ASan probes that reproduce the review's OOB before the fix and run clean after.

## Story

The review's second stage, and the only High *security* finding: `load_npy` treats file contents as trusted — fixed-offset dereferences at `buffer.data()+6/8/10`, a `header_length` taken from the file with no bound (it can also overflow the offset arithmetic), `std::stoul` that can throw from a `noexcept` member (→ `std::terminate`), a `npos`-unguarded shape parse, a `row*col` payload copy of file-controlled size, and no dtype check (a float32 `.npy` loaded into `matrix<double>` copies misinterpreted bytes). The in-repo model of correct behavior already exists: `load_bmp` (~6760) validates header/size consistency before parsing — the review explicitly calls it out as the pattern to follow. The fix converts `load_npy` to that pattern (policy P3): validate magic/version, `buffer.size() >= 12`, `header_length` within remaining bytes *before* any offset arithmetic, header bounds, shape-token presence (`npos` guarded), dtype string matching the target `value_type`, and `buffer.size() >= data_offset + row*col*sizeof(value_type)`; catch `stoul` → `false`. Behavior changes are all sanctioned (PRD §5): truncated/malformed/mismatched input now yields `false` instead of OOB/terminate/misinterpretation; valid files load exactly as before (the existing happy-path test pins that).

## In scope

- `crtp_load_npy` (anchors ~2499–2570; members at 2504/2508): validation per P3 (list above).
- Negative-path tests: extend `tests/cases/load_npy.hpp` with crafted-byte cases (truncated magic, truncated header, malformed dtype, header_length overflow, float32-into-double) — crafted at runtime into `tmp/`, no new committed binaries.
- Eval probes E03, E04 (`.work/probes/`), ASan-clean.

## Out of scope

- The happy path's semantics (existing 4 dtype cases in `tests/cases/load_npy.hpp` must pass **unchanged**).
- `load_bmp`/`save_as_bmp`/`save_png` (S5 owns `save_png`); any other finding; `ReadMe.md` (doc delta emitted, P4); `examples/`; `Makefile`.
- Adding a dependency for npy parsing (policy: no production dependencies).

## Deliveries

1. Validated `load_npy` in `matrix.hpp` (diff confined to the function body).
2. Negative-path test cases (5 above) passing; happy-path cases untouched and green.
3. E03/E04 probes live and ASan-clean.
4. Handoff with doc deltas for S6 (ReadMe §"load npy" line ~1055: "returns false on malformed/foreign-dtype files; dtype must match the target type").

## Context budget map (~45K of 128K — do not exceed)

| Read | How much | Why |
|---|---|---|
| `AGENTS.md`, `docs/project_contract.md` §3–§5 | ~2.5K | law + API/verification policy |
| `docs/prd.md` §5 row 18 (load_npy), §7 P3 | ~1K | authorization + boundary policy |
| `docs/opencode_sharded_review.md` §S1 (+ "verified non-issues: load_bmp" note) | ~2K | finding + the in-repo model |
| `matrix.hpp` regions: 2499–2575 (load_npy), 6755–6775 (load_bmp model) | ~6K | the code + the pattern |
| `tests/cases/load_npy.hpp` full + `tmp/` note | ~2K | happy-path pin + fixture layout |
| `docs/eval_seed_cases.md` E03–E04 rows | ~0.5K | probe specs |
| **Do NOT read:** rest of `matrix.hpp`, ReadMe, research docs in full, images (except the 4 `.npy` fixture headers, hex-dump ≤ 64B each) | — | budget |

## Deep-research references

- Report 6 §"Layout, performance, execution, safety and correctness" (line 511): safety framing — untrusted external data validated at the boundary. Section only.
- P3500R0 §"Interoperability Engine: Native C ABI Exchange via DLPack" (line 171): *context only* — why a file/ABI boundary that trusts its bytes is a liability as this library's I/O surface grows (npy is the forerunner of any DLPack-style exchange). Do **not** implement anything from it (R-17).

## Pre-flight (mandatory)

1. `git status` clean; baseline commit.
2. Re-anchor `crtp_load_npy` by grep; confirm the review's structure (fixed-offset derefs, unguarded `stoul`, no dtype check) still holds — and log the two extra hazards (header_length overflow; `npos` shape parse) as verified-this-turn findings in the handoff.
3. Reproduce the review's ASan OOB (3-byte file, `-DNDEBUG` build). Record pre-fix output.
4. Hex-dump one fixture (≤64B) to confirm the magic/version offsets the fix will validate (do not trust the offsets from memory — from bytes).

## Exit criteria

1. `make test` green: 4 existing `load_npy` cases **unchanged** + 5 new negative cases.
2. E03/E04 pass; ASan variant (truncated magic, truncated header, overflow-length file) clean, exit 0, no `terminate`.
3. `git diff --name-only HEAD` ⊆ {`matrix.hpp`, `tests/cases/load_npy.hpp`, `.work/**`, `docs/eval_seed_cases.md`, `docs/risk_register.md`}.
4. Sharded review + adversarial verifier PASS (verifier focus: attacker-chosen bytes — 3B, 11B, 12B files; `header_length` = 0xFFFFFFFF; `shape` token missing; wrong-endianness dtype).
5. **Human decision gate (high risk):** user reviews diff + evidence before merge.

## Risk and routing

- Risk level **high** (security: untrusted external input). Routing: **branch_and_compare** — worker implements; independent test-writer derives expected accept/reject from the P3 checklist *without reading the worker's implementation*; sharded review; adversarial verifier (attacker mindset per `docs/prompts/adversarial_verifier.md`); human gate.
- Failure modes to watch: validation added *after* a dereference (too late); `header_length` overflow surviving the checks (use `header_length <= buffer.size() - data_prefix` style bounds, not `buffer.size() < 10 + header_length`); happy-path regression (endianness/version path broken by the new checks); crafted test files not cleaned from `tmp/` (determinism).

## Handoff requirements

State snapshot (compiler, checks run/not run); decision log (pre-fix ASan output, the two extra hazards and their chosen handling, contract refinements); eval seeds E03/E04 live; doc deltas for S6 (exact ReadMe §"load npy" wording); warnings for S5 (`save_png` is the other I/O boundary — same P3 discipline applies there).

## Contract

`docs/session_2_contract.yaml` — read before pre-flight; authority for scope, invariants, checks.
