# Session 5 — Brainstorming (refinement record)

## Pre-flight evidence (pre-fix, 2026-08-18; `.work/evidence/s5_prefix.log`, `s5_baseline_test.log`)

| # | Observation | Evidence |
|---|-------------|----------|
| P0 | Baseline green: 73 cases / 49,217,182 assertions, all pass; live seeds E01–E13 all PASS | `s5_baseline_test.log` |
| P1 | Exactly **3** C-generator lines: 5327 `srand(time+&ans)`, 5329 `srand(seed)`, 5333 `std::rand()`; all inside `rand` | `grep -cE 'srand\(|std::rand\('` = 3 |
| P2 | `std::random_access_iterator_tag` at line 151 makes the literal acceptance grep `srand\|std::rand` a **false positive** → refined grep `srand\(|std::rand\(` (interview Q6.1) | grep |
| P3 | Guard counts pre-fix: `total_cores < 1` = 0, `total_cores <= 1` = 1 (line 279, existing), `parallel_size < 1` = 0 | grep |
| P4 | Line 1152 `reduce`: `unsigned int const total_cores = hardware_concurrency();` unguarded, divides at 1163 → SIGFPE if 0 | source read |
| P5 | Line 4121 `reduce_impl_private`: `parallel_size` **already short-circuit-guarded** (`<= 1 \|\| size < 32`) — plan's "unguarded" claim unsupported → discrepancy logged (interview Q5) | source read |
| P6 | `save_png` (3181): `fopen` result unchecked (3182); stray `;;` at 3190 on the PNG-signature `fputc` | source read |
| P7 | Pre-fix E15 executable red: `save_as_png` to unwritable path → **SIGSEGV, exit 139** | `s5_prefix.log` P2 probe |
| P8 | ltrace seed trace: stable per call site (e.g. `3067854750`/`3067854782`, 32 B apart); same-site 100 seed-0 calls → **1 distinct value** (per-call-site correlation); cross-site differs | `s5_prefix.log` |
| P9 | Explicit-seed determinism holds pre-fix (7==7, 7≠8) — the E14 invariant side; value streams recorded (`rand(1,4,1)` = `0.84018771683788496 …`) | P0 probe |
| P10 | TSan pre-fix = clean: race is inside uninstrumented libc `rand()` state → contract fallback clause applies (reasoning + grep for absence of global state) | TSan probe |
| P11 | `<random>` already included (line 29); `<cstdint>` (line 12) | header read |
| P12 | `debug_mode` constexpr (57–61) = 0 under `NDEBUG`; `print_assertion` aborts debug-only → C7 is a doc delta, no code | source read |
| P13 | Suite case hosts: `tests/test.cc` include block; insertion point after `proj.hpp` (line 58), before commented `remquo.hpp` (line 59) | source read |
| P14 | Matrix `operator==` exists (line 4235) → suite case can use whole-matrix `==` | grep |

## Decision table

| # | Decision | Alternatives considered | Chosen + why (evidence) |
|---|----------|------------------------|------------------------|
| D1 | Engine = local `std::mt19937{seed}` + `std::uniform_real_distribution<T>(0.0, 1.0)` | `minstd_rand` (31-bit period — too short); `rand_r` (non-portable); global `mt19937` + mutex (still global state, violates C11); thread_local engine (shared state, overkill) | Contract prescribes `uniform_real_distribution<T>`; local engine = thread-safe by construction; `<random>` already included (P11); period 2^19937−1 |
| D2 | Seed-0 keeps the **exact** pre-fix expression `time + reinterpret_cast<uint64_t>(&ans)` (truncated to `unsigned int`) | Pure `time()` (loses the address salt/thread differentiation the pre-fix code intentionally mixed in); `random_device` (breaks "time-based" + makes seed 0 non-reproducible in the documented sense) | Contract: "keep time-based (R-05 note)"; minimal semantic delta; residual per-call-site correlation documented (P8 shows it is inherent to the seed, not the engine) |
| D3 | Drop `noexcept` from 5323/5355/5361/5366 (whole rand chain) | Drop only from `rand`+`rand_like` (contract's literal two) — leaves `random_like`/`randn_like` as `terminate` traps once `rand` can throw | Interview Q3: same defect, one change class; S6 (alias rationalization) inherits the correct state; boundary documented |
| D4 | int/complex-T `rand` no longer compiles → **document, don't fix** | Special-case non-float T (engine + manual cast) | Contract prescribes `uniform_real_distribution<T>`; [uniform.real] requires floating-point `result_type` (libstdc++ enforces, verified); zero in-repo int/complex consumers (audited); a special case would be a behavior invention beyond the contract |
| D5 | C12: clamp at **1152** (`total_cores < 1 → 1`, drop `const`) **and** at **4121** (`parallel_size < 1 → 1`, drop `const`); 279 untouched | Guard only 1152 (plan's real finding) | Contract mandates both clamps + grep acceptance; 4121's clamp is behavior-neutral (P5) and makes the acceptance satisfiable; 279 left per contract |
| D6 | `save_png`: `if ( ! fp ) return;` after `fopen`; drop the stray `;;` (3190); keep `noexcept` on the free helper | Return error code (changes free-helper signature — out of scope); print to stderr (I/O side effect from a noexcept write helper; S2's `load_npy` precedent is silent no-throw) | Contract: "no-throw `save_png` must not crash"; silent no-op matches the in-repo I/O-boundary precedent (load_npy S2); member `save_as_png` return value unchanged (still `true`) |
| D7 | E14 dual role: (a) invariant pin green pre- AND post-fix; (b) the C11 "red" is structural (grep 3→0) + TSan post-fix + P8 correlation demo | Fabricate a value-level red (impossible — determinism/range hold pre-fix, P9) | Contract itself frames E14 as invariant pin + grep acceptance; interview Q2 |
| D8 | C7 deliverable = authored policy text (`specs/ndebug_policy.md` + `design.md §4` + handoff); **no** ReadMe edit | Edit ReadMe now | ReadMe.md is out of the contract blast radius and owned by S6; C7 says "document the delta" |
| D9 | Suite case = new `tests/cases/rand.hpp`, registered in `test.cc` between `proj.hpp` and the commented `remquo.hpp` (alphabetical) | Extend an existing case file | No existing rand case; house style = one file per topic (P13) |
| D10 | Post-fix `make example`: stdout **differs** (R-07) → record delta, `git checkout -- images/` | Require byte-identical examples | Explicit seeds 1/2 now drive an `mt19937` stream (P9 recorded pre-fix values); the contract sanctions the change and requires the delta to be recorded |

## Example impact (expected, print-only — no edits)

| Example | Call | Pre-fix | Post-fix |
|---------|------|---------|----------|
| 0006/0008/0009/0011/0013 | seedless `rand`/`random` | changes every run (time+addr seed) | changes every run (same seed expression, different engine) |
| 0012 | `rand(…, 1)` | fixed stream (RAND, seed 1) | fixed stream (mt19937, seed 1) — **values differ** |
| 0019 | `rand(…, 1)` | as 0012 | as 0012 |
| 0020 | `rand` seed 1 / seedless mix | mixed | mixed; random lines differ |
| 0021 | `rand(…, 2)` | fixed stream (RAND, seed 2) | fixed stream (mt19937, seed 2) — **values differ** |

`images/` outputs: PNGs from random-valued fields differ → checkout policy applies.
