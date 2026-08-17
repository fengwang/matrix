# Session 2 — Proposal

Concise extraction from the brainstorming (no re-exploration). Authority: `docs/session_2_contract.yaml`.

## Motivation

`load_npy` is the library's only binary matrix import path and currently treats file contents as
trusted (finding S1, the review's only High *security* finding). Pre-flight reproduction this turn
(`.work/evidence/prefix_*`) confirmed every review claim and added a third hazard:

- 3B / 11B / truncated-header / `0xFFFFFFFF` files → **ASan heap-buffer-overflow reads**
  (fixed-offset derefs at `buffer.data()+6/8/9` and the `header` string construction, no bound
  checks, `header_length` overflow in the v2 path's offset arithmetic);
- missing/1-D/3-D/30-digit shape tokens → **`std::terminate`** (`stoul` throws
  `invalid_argument` / `out_of_range` out of the `noexcept` member);
- shape `(-1, 2)` → `stoul("-1")` = 2⁶⁴−1 → `resize` throws `bad_array_new_length` → terminate
  (**third hazard: overflow-unguarded shape/payload arithmetic** — new this turn, confirmed by probe);
- foreign dtype (`<f4`, `>f8`, `Vf8`, `|u1` into `matrix<double>`) and 1-byte-short payload →
  **silent misload, `ok=1`** (no dtype check; `row*col` payload copy of file-controlled size).

Sanctioned behavior change: PRD §5 row 18 — malformed/truncated/foreign-dtype input yields `false`
instead of OOB/terminate/misinterpretation; valid files load exactly as before.

## Specific changes agreed

1. **`matrix.hpp`** — rewrite the body of `crtp_load_npy::load_npy(char const*)` (~2508) as a
   validate-then-act boundary (P3): magic/size/version checks before any byte deref; non-wrapping
   `header_length` bound; header dict sanity (`'{'`); dtype-match vs `value_type`; npos-guarded,
   digit-bounded shape parse with `row, col ≥ 1`; overflow-checked payload bound; `resize` only
   after all checks; byte-level copy (`int8_t*` pattern); `try/catch(…) → false` so the `noexcept`
   member is genuinely throw-free. Signatures unchanged.
2. **`tests/cases/load_npy.hpp`** — append exactly 5 negative `TEST_CASE`s (contract list), each
   crafting bytes at runtime into `tmp/`, asserting `ok==false` **and** matrix state unchanged;
   existing 4 happy cases byte-for-byte untouched; no `tests/test.cc` change (already registered
   at line 35).
3. **`.work/probes/E03_E04.cc`** — 18-case ASan probe (reject/dtype/boundary-pin classes),
   compiled by the contract's deterministic check.
4. **`docs/eval_seed_cases.md`** — E03/E04 status `seeded` → `promoted` (probe + permanent home in
   `tests/cases/load_npy.hpp`).
5. **`docs/risk_register.md`** — S2 closeout watch items (incl. the adjacent `load_binary`
   overflow-unguarded `r*c*sizeof(Type)` arithmetic observed while reading the sibling pattern —
   documentation only, not a fix; out of scope).
6. **`.work/handoff_session_2.md`** — decision log, doc deltas for S6 (ReadMe §"load npy" ~1055,
   incl. real-spec-v2 rejection note), warning for S5 (`save_png` is the other I/O boundary).

## Capabilities

### New capabilities

- **`load_npy_boundary_validation`** — hard P3 validation of all file content before any
  dereference, parse, or resize (magic, min size 12, version ∈ {1,2}, non-wrapping header-length
  bound, dict sanity, dtype match, npos-guarded digit-bounded shape, overflow-checked payload
  bound). Spec: `specs/load_npy_validation.md` (ADDED).
- **`load_npy_rejection_semantics`** — complete reject table: every named malformed/foreign/
  truncated input returns `false`, never throws/aborts/UBs, leaves the matrix unchanged;
  `noexcept` honest. Spec: `specs/load_npy_rejection_semantics.md` (ADDED).

### Modified capabilities

- **`load_npy_happy_path_loading`** — behavior preserved (4 fixtures, v1 + library v2 convention,
  fortran order, payload-to-tail), now explicitly specified and pinned; the requirement previously
  existed only implicitly ("loads work"). Spec: `specs/load_npy_happy_path.md` (MODIFIED — full
  updated content).

## Impact

- **Code:** `matrix.hpp` — one function body (~lines 2508–2567 → ~2508–2600). Nothing else.
- **API:** no signature changes; both `load_npy` overloads keep `bool … noexcept`. The observable
  contract of the happy path is identical (sanctioned change is rejection-only, row 18).
- **Dependencies:** none added (policy: no production dependencies).
- **Docs:** `ReadMe.md` untouched (P4); delta wording emitted in the handoff for S6.
- **Tests:** `tests/cases/load_npy.hpp` append-only.
