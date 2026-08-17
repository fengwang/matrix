# Session 2 — Execution Contract

Companion to `docs/session_2_contract.yaml` (the YAML is the authoritative artifact; this file
carries the operational detail the session protocol requires). Baseline: `ad6fa79` on
`phase-1/session-2`.

## Planned file changes (exact paths)

| File | Change | Task |
|---|---|---|
| `tests/cases/load_npy.hpp` | Append 5 negative `TEST_CASE`s + file-local helpers; existing 4 cases byte-untouched | 2 |
| `matrix.hpp` | Body-only rewrite of `crtp_load_npy::load_npy( char const* )` (~2508–2566 → ~2508–2600); single hunk | 3 |
| `.work/probes/E03_E04.cc` | ASan probe, 18 selectable cases (pre-fix + post-fix runs) | 1, 3 |
| `.work/evidence/**` | baseline / pre-fix / red / green / final logs, diff audit | 1–8 |
| `.work/independent/derivation.md` | independent test-writer derivation | 5 |
| `.work/handoff_session_2.md` | closeout handoff | 8 |
| `docs/session_2/**` | phase docs (this set), sharded review, adversarial verification | 1, 6, 7 |
| `docs/eval_seed_cases.md` | E03/E04 rows → `promoted` | 8 |
| `docs/risk_register.md` | S2 watch items (3rd hazard; adjacent `load_binary` note) | 8 |

## Allowed blast radius (from the YAML; anything else requires a stop)

**Allowed:** `matrix.hpp` (the `crtp_load_npy` body only), `tests/cases/load_npy.hpp`
(append-only), `.work/**`, `docs/eval_seed_cases.md`, `docs/risk_register.md`, `tmp/**`,
`docs/session_2/**`.

**Forbidden:** `ReadMe.md` (S6 owns; P4 — delta emitted in handoff), `Makefile` (no new
dependencies; probe built ad hoc with the contract flags), `examples/**`, binary fixtures in
`images/**`, `docs/prd.md` (frozen), `docs/project_contract.md` (frozen), `tests/test.cc`
(not in allowed_files — registration already present at line 35, so no change needed).

## First test to write (TDD)

File: `tests/cases/load_npy.hpp` (append block, first of the five).
Case name: `TEST_CASE( "load_npy rejects an unopenable or truncated (3-byte) file", "[load_npy]" )` —
missing-file `REQUIRE( !m.load_npy( "tmp/s2_neg_missing.npy" ) )` + crafted 3-byte
`{0x93,'N','U'}` file `REQUIRE( !m.load_npy( path ) )`, with row/col-unchanged assertions and
cleanup. Red on the pre-fix tree (pre-fix: ASan OOB read / crash under sanitizer; unclean
exit without).

## Checks per task (commands from repo root)

- Task 2 (red): `make test 2>&1 | tail -2` · `./test_test "[load_npy]"` (expect 4 pass / 5
  fail-crash) · `git diff tests/cases/load_npy.hpp | head` (append-only proof).
- Task 3 (fix): `make test 2>&1 | tail -2` · `./test_test "[load_npy]"` (9/9) ·
  `make .work/probe_s2 && .work/probe_s2` (`PASS E03`, `PASS E04`, exit 0).
- Task 4 (audit): `g++ --version | head -1` · `make test && ./test_test 2>&1 | tail -4`
  (64/64) · `git diff --name-only ad6fa79` (⊆ allowed set) ·
  `sed -n '2499,2590p' matrix.hpp | grep -c 'return false'` (> 6) ·
  `git diff ad6fa79 -- matrix.hpp` (single hunk inside the function).
- Task 6 (after any review fix): re-run all Task 4 checks.
- Task 7: verifier re-runs Task 4 checks + the probe from a fresh framing.

## Review axes (sharded review, 6)

1. **Correctness** — spec R-V1…R-V9 / R-H1 vs the diff; boundary exactness (inclusive payload,
   non-wrapping bound form; the 10/12 prefix pairing).
2. **Readability** — house style (braces, `better_assert`, spacing `( x )`), comment density
   appropriate to the safety-critical path, no unexplained magic numbers (R-05).
3. **Security** — residual attacker inputs: every file-controlled quantity (`header_length`,
   shape digits, dtype, version) flows only through a validated gate; no integer overflow; no
   OOB; no `stoul`; `noexcept` honest.
4. **Tests** — the 5 cases assert content and state (not only return value); cleanup
   determinism; happy-path blocks byte-identical; probe covers the contract's `evidence` list.
5. **Architecture** — diff confined to the body (Deliveries 1); no new public API; in-repo
   pattern consistency (`load_binary`/`load_bmp` models); no layering violation.
6. **Performance** — single buffer read (no double parse), no per-element work added, no
   allocations beyond the pre-existing buffer + resize; `const`/`size_t` hygiene.

## Adversarial verifier brief (what the verifier sees; focus list)

Sees: `docs/session_2_contract.yaml` + PRD §5 row 18; `git diff ad6fa79 -- matrix.hpp
tests/cases/load_npy.hpp`; evidence (`.work/evidence/final_suite_run.log`, probe output,
pre-fix `prefix_*` logs). Does NOT see: brainstorming/design rationale, task notes.

Focus list (attacker-chosen bytes):
- 3-byte / 11-byte / 12-byte files (minimum-size and magic gates).
- `header_length` = 0xFFFFFFFF (v2) and `header_length` = remaining+1 (both versions).
- Exact boundaries: `header_length == buffer.size() - data_prefix` (bound inclusive; content
  checks then decide) and `payload == buffer.size() - data_offset` (must load).
- Missing shape token; 1-D `(2,)`; 3-D `(2, 3, 4)`; negative `(-1, 2)`; 30-digit token.
- Big-endian `>f8` and native `Vf8` descriptors into `matrix<double>` (silent misloads pre-fix).
- Real-spec v2 file (8-byte length field) → must be rejected cleanly (the `{` sanity gate).
- Zero dims `(0, 2)`; version bytes 0 and 3.
- Missing file; happy-path regression (all 4 fixtures + v2-convention + fortran + tail pins).

If any focus item fails: classify via `docs/prompts/failure_arbiter.md` before fixing
(root-cause evidence first).

## Concrete done condition (verbatim contract)

`make test` green AND `.work/probe_s2` (ASan, release) exits 0 on the E03 + E04 cases AND the
full test suite is green AND `grep -c 'return false'` on lines 2499–2590 of `matrix.hpp` > 6.

Operational additions (this file): the existing happy-path `TEST_CASE` passes byte-unchanged;
the 5 negative cases assert `ok == false` and matrix state unchanged; the diff is confined to the
allowed set (audit vs `ad6fa79`); sharded review + adversarial verification recorded with no
open High/Critical; seeds E03/E04 promoted; handoff written with S6 doc delta + S5 warning;
compiler version recorded; checks run and not run both stated; human decision gate presented.
