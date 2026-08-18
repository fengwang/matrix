# Session 5 — Proposal (capability breakdown)

One capability per finding, plus the test/evidence capability. Each capability maps
1:1 to a spec file under `docs/session_5/specs/`.

## Capabilities

### 1. `rand-engine` — C11: `rand` uses a per-call local engine (no global state)
- Swap the `srand`/`std::rand` body (matrix.hpp 5325–5335) for
  `std::mt19937{effective_seed}` + `std::uniform_real_distribution<T>(0.0, 1.0)`.
- Seed 0 keeps the exact pre-fix `time + &ans` mix (documented residual
  per-call-site correlation; interview Q1, P8).
- Drop `noexcept` on the whole rand chain (5323/5355/5361/5366; interview Q3).
- Consequences: int-T → all zeros (unchanged from pre-fix, documented); complex-T →
  no longer compiles (no in-repo consumers, documented); thread-safe by construction.
- Acceptance: `grep -cE 'srand\(|std::rand\(' matrix.hpp == 0`; E14 invariant green;
  TSan post-fix clean.

### 2. `core-count-guard` — C12: clamp `hardware_concurrency()` to ≥ 1
- `reduce` (1152): `const` dropped, `if ( total_cores < 1 ) total_cores = 1;`.
- `reduce_impl_private` (4121): `const` dropped, `if ( parallel_size < 1 ) parallel_size = 1;`
  (already short-circuit-safe; contract-mandated clamp, behavior-neutral — P5).
- Line 279 (`total_cores <= 1`) untouched per contract.
- Acceptance: `grep -c 'total_cores < 1' == 1` and `grep -c 'parallel_size < 1' == 1`.

### 3. `save-png-boundary` — S2-finding/R3-slice: `save_png` `fopen` guard + stray `;;`
- Guard `if ( ! fp ) return;` after the `fopen` at 3182; silent no-op (R-05 note),
  `noexcept` kept; happy path unchanged (E15 positive control).
- Remove the stray `;;` at 3190 (PNG-signature `fputc`) — R3-slice.
- Acceptance: `grep 'if (!fp) return' matrix.hpp` (refined: `if ( ! fp ) return`
  matching house spacing) ≥ 1; E15 probe green post-fix (was SIGSEGV pre-fix, P7).

### 4. `ndebug-policy-doc` — C7: authored `NDEBUG` policy delta (no code)
- `better_assert` is debug-only (no-op under `NDEBUG`; `debug_mode` 57–61).
- I/O boundaries are hard, NDEBUG-independent, silent-failure checks
  (precedents: `load_npy` S2, `save_png` S5).
- Deliverable: draft delta text for S6's ReadMe change (spec + design §4 + handoff).

### 5. `rand-regression-tests` — E14: `tests/cases/rand.hpp`
- Explicit-seed determinism (7 == 7), seed inequality (7 ≠ 8), [0,1) range for
  `double` (64×64) and `float` (32×32) instantiation, int-T all-zeros pin,
  `noexcept`-removal pin (`static_assert` on `!noexcept(…)`), return-type pins.
- Registered in `tests/test.cc` after `proj.hpp` (alphabetical, P13).
- Green pre- AND post-fix (invariant pin; the C11 red is structural — interview Q2).

## Risk table (carried from the contract; plan-of-record = TDD + targeted check)

| Risk | Severity | Mitigation |
|------|----------|------------|
| R-07 sanctioned stream change (examples 0012/0019/0020/0021 values change) | low | pre-fix streams recorded (P9); `make example` delta recorded; `git checkout -- images/` |
| Engine quality regression (period/bounds) | low | `mt19937` + `uniform_real_distribution` (contract-prescribed); range pinned in suite |
| `noexcept` removal changes observable semantics | low | only affects the (documented) allocation-throw path; no caller depends on noexcept |
| Complex-T consumers break | low | zero in-repo consumers (audited); documented |
| C12 clamp misfires on valid 1-core machines | low | `< 1` clamps only 0 (hardware_concurrency's "undetermined" sentinel); 1 stays 1 |
| save_png guard changes happy path | low | E15 positive control (writable path → PNG exists); full suite green |

## Files that will change (blast radius check)

`matrix.hpp`, `tests/test.cc`, `tests/cases/rand.hpp` (new),
`docs/eval_seed_cases.md`, `docs/risk_register.md`, `docs/session_5/**`, `.work/**`
(force-added evidence, repo precedent). All within the contract's `allowed_files`.
ReadMe.md, Makefile, examples, `load_binary`, the alias *renames* (S6) untouched.
