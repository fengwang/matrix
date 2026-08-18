# Session Handoff — Session 5 (robustness: rand engine, core-count guards, save_png boundary, NDEBUG policy)

## State Snapshot
- Session: S5 — C11 `rand` → per-call local `mt19937` (+`noexcept` removal, `rand`-family chain), C12 core-count guards (both `hardware_concurrency()` sites), S2-finding `save_png` `fopen` guard + stray `;;` (R3-slice), C7 NDEBUG policy doc delta (no code change)
- Branch: `phase-1/session-5` (baseline `c40b04b` = S4 closeout)
- Last commit: `<closeout>` (this commit) — full chain: `48763f4` pre-flight (phase docs, probes, pre-fix evidence) → `a971944` tasks 1+2 (E14 suite case + C11 engine) → `41ea4aa` task 3 (C12) → `5dca4f6` task 4 (save_png) → closeout (sharded review, adversarial verification, seed/risk-register deltas, this handoff)
- Changed files (vs `c40b04b`): `matrix.hpp` (5 sanctioned hunks: reduce clamp ~1152, `save_png` guard + `;;` 3187–3192, `reduce_impl_private` clamp ~4125, `rand` body 5329–5345, alias `noexcept` removals 5362–5377), `tests/test.cc` (+1 include), new `tests/cases/rand.hpp`, `docs/session_5/**` (interview → plan → execution contract → sharded review → adversarial verification), `docs/eval_seed_cases.md` (E14 promoted, E15 live), `docs/risk_register.md` (S5 watch items), `.work/probes/S5_*` + `.work/probes/E14_E15.cc` + `.work/evidence/s5_*`
- Checks run:
  - `make test` + `./test_test` (fresh, final state): **74 test cases / 49,217,191 assertions, all pass** (baseline 73 / 49,217,182; +1 case = the E14 rand case)
  - deterministic check verbatim (contract): `g++ -std=c++20 -DPARALLEL -O1 -o .work/probe_s5 .work/probes/E14_E15.cc && .work/probe_s5` → **`PASS`** (E14 contract-literal 4×4 seed checks + E15 unwritable-path no-crash + positive-control PNG)
  - adversarial seed probe `S5_av_seeds.cc`: **PASS** (seeds 0/1/2147483647/UINT_MAX deterministic + non-degenerate + in [0,1); float 10,000 draws in [0,1); 0×0 and 1×1 shapes; seed-1 stream **bit-identical across separate process launches** — the examples' reproducibility contract)
  - int / complex-T instantiation compile attempts: **hard static_assert failure** ("result_type must be a floating point type") — documented-unsupported, no in-repo consumers (audited)
  - `make example` exit 0; `./test_example` stdout delta vs S4 baseline = **exactly one line** (example 0019 LU MAE 1.5697e-10 → 1.7746e-10; random-input-derived) — `s5_example_delta.txt`; `git checkout -- images/` after
  - TSan probe post-fix: **clean** (pre-fix: clean *but* the race was in uninstrumented libc internals — documented probe limitation; post-fix there is no library-level shared state, so the race class is eliminated by construction)
  - pre-fix evidence (all in `s5_prefix.log`): `rand` global-state grep = 3 (`srand`×2 + `std::rand`); ltrace seed trace (per-call-site seed = `time + &ans`, adjacent locals 32 B apart; same-site same-second → 1 distinct value over 100 calls — seed-0 correlation mechanism resolved); E15 pre-fix **SIGSEGV exit 139**; pre-fix value streams recorded (R-07)
  - C11 executable red: the suite's `static_assert( !noexcept( feng::rand< double >( 1, 1, 7 ) ) )` **failed to compile pre-fix** (`s5_t1_red.log`)
  - scope audit: `git diff --name-only c40b04b..HEAD` within the allowed set (see AV report); `matrix.hpp` diff = exactly the 5 sanctioned hunks (AV-7 hunk list)
  - sharded review (4 shards × 6 contract axes, in-process fresh-context simulation): **0 Critical/High, 5 Low/informational — none blocking** (`docs/session_5/sharded_review.md`)
  - adversarial verification (contract + diff + evidence only, in-process fresh-context simulation): **PASS** — no disproven claims; 3 unsupported claims corrected in place (`docs/session_5/adversarial_verification.md`)
- Checks not run: forcing `hardware_concurrency()==0` (unreachable on this host — `taskset -c 0` floors it at 1, verified; contract acceptance is code-presence, R-14 classification); 32-bit build (host is x86-64); disk-full-mid-write for `save_png` (contract adversarial case 5 = document as known limitation, no fake handling — `fputc` failures remain unchecked, pre-existing); subagent-dispatched review/verification (host output-budget constraint — in-process fresh-context simulation, re-confirmed in the risk register)
- Current status: **complete, green, committed** — done-condition satisfied (evidence table below); ready for S6

## Done-Condition Evidence (contract `docs/session_5_contract.yaml`)

| Contract item | Evidence |
|---|---|
| C11: `rand` → local `std::mt19937` + `std::uniform_real_distribution<T>(0.0, 1.0)`; seed 0 non-deterministic time-based; explicit seed deterministic (hard invariant); `noexcept` dropped; `rand_like` semantics preserved | `grep -n 'mt19937'` = 5337 (engine in `rand` body); seed-0 expression bit-identical to pre-fix (diff audit); T1 red = `noexcept` static_assert (`s5_t1_red.log`); T2 green: refined global-state grep 3→0 (`s5_t2_green.log`); TSan clean post-fix; `rand_like`/`random_like`/`randn_like` **bodies untouched** (AV-7 hunk list); suite case (a)–(f) in `tests/cases/rand.hpp` |
| C12: guard `total_cores < 1 => 1` at reduce (~1152) + second site (~4036), mirroring ~276 | clamps at 1152–1154 and 4126–4127; `grep -c 'total_cores < 1'` = 1, `grep -c 'parallel_size < 1'` = 1 (contract's literal `>= 3` unsatisfiable — logged refinement Q6.2: the second variable is named `parallel_size`, and the pre-existing guard at 279 uses `<= 1` and is left verbatim); suite green |
| S2-finding: `save_png` → `if ( ! fp ) return;` after `fopen`; stray `;;` removed; failure = documented silent no-op | guard at 3188–3189; `grep -cF 'if ( ! fp )'` = 1; `;;` count 0; E15 red→green (139 → exit 0 + 135-byte positive-control PNG, `s5_t4_green.log`); `noexcept` kept (fopen fails via nullptr, not exception) |
| C7: no code change; author exact policy text as doc delta for S6 | policy text in `specs/ndebug_policy.md` = design.md §4 (8 `NDEBUG` mentions in design, 6 in spec — `s5_t5_check.log`); no `ReadMe.md`/code change (S6 owns ReadMe per R-13) |
| New test `tests/cases/rand.hpp` (E14 determinism/inequality/range) + registration | file created; `#include "./cases/rand.hpp"` in `tests/test.cc` (after `proj.hpp`); suite 73 → 74 cases |
| Eval probes E14/E15 in `.work/probes/` | `E14_E15.cc` (combined, verbatim contract build form) + `S5_p0_preflight.cc`, `S5_p1_tsan.cc`, `S5_p2_save_png.cc`, `S5_av_seeds.cc` |
| Invariants | all six — see adversarial verification report (suite green; determinism in- + cross-process; [0,1) + seed-0 time-based; no global state (grep 0); save_png silent no-op exit 0; `inverse.hpp` seed-0 case green inside the suite) |
| `acceptance_criteria` | all five — AV report table (criteria 3 and 4 via logged, intent-preserving grep refinements: `srand\|std::rand` literal false-positives on `std::random_access_iterator_tag`; `total_cores < 1 >= 3` literal unattainable — both refinements proven unsatisfiable-by-construction, independent of S5's changes) |
| Deterministic checks | `make test` green; verbatim combined probe → `PASS`; `git diff --name-only HEAD | grep -vE …` empty (clean tree); `grep -n 'mt19937'` present |

## Narrative Context

S5 closed the three code robustness findings left by S1–S4's reports plus the C7 documentation delta. The dominant finding was **C11**: `rand` seeded the process-wide C generator (`srand`/`rand`) — a data race under the suite's `-DPARALLEL` build, non-deterministic "deterministic" seeds (the seed mixed in a process-global address), and a `noexcept` lie (the body allocates). The fix is the contract-prescribed per-call local `std::mt19937` + `std::uniform_real_distribution<T>(0.0, 1.0)`: thread-safe by construction, explicitly seeded streams deterministic **across process launches** (the examples' reproducibility contract, now stronger than pre-fix), and honest exception behavior.

Three plan claims did not survive pre-flight (all logged, none silently absorbed): (1) the "second unguarded `hardware_concurrency()` site" (4121) was **already** short-circuit-guarded — the clamp is behavior-neutral + protective; (2) the literal acceptance greps are unsatisfiable as written (false-positive on an unrelated typedef; a differently-named variable) — intent-preserving refinements used; (3) the plan's citation of the `save_png` line (`FILE* const`/`"wb+"`) did not match the actual source (`FILE*`/`"wb"`).

The one substantive mid-session correction: the early analysis claimed `rand<int>` kept "all-zeros, unchanged" behavior. The T1 test file **proved otherwise at compile time** — `uniform_real_distribution<T>` requires a floating-point `result_type` ([uniform.real]), enforced by libstdc++'s static_assert, so **int and complex T no longer instantiate**. No in-repo consumers exist (audited); the contract's "sane or documented" clause is satisfied by documentation.

## Decision Log

| ID | Decision | Rationale |
|---|---|---|
| D1 | Per-call local engine (contract-prescribed); no shared/static engine | thread-safe by construction; deterministic per seed; the contract's wording is the design |
| D2 | Seed 0 keeps the **exact** pre-fix expression `time + reinterpret_cast<uint64_t>(&ans)` | contract: "seed 0 => non-deterministic time-based seed"; residual same-call-site/same-second correlation is inherent to this policy (documented, not a violation) |
| D3 | `noexcept` removed from the whole `rand`-family chain (`rand`, `rand_like`, `random_like`, `randn_like`) | contract named `rand` + `rand_like`; the other two wrappers are the same defect class — once `rand` can throw `bad_alloc`, a `noexcept` wrapper is a `std::terminate` trap |
| D4 | int/complex-T compile impact **documented, not fixed** | contract prescribes the distribution type; no in-repo consumers (audited); the "sane or documented" clause is satisfied; a loud hard error beats a silent semantic trap |
| D5 | C12 clamps at **both** 1152 and 4121 despite 4121's pre-existing short-circuit | behavior-neutral + protective; satisfies the contract's grep acceptance; line 279 left verbatim ("do not fix twice") |
| D6 | `save_png` guard = `if ( ! fp ) return;` silent no-op | matches the S2 `load_npy` precedent (policy P3); `noexcept` kept (failure mode is nullptr, not exception); `save_as_png` return semantics untouched (S6 I/O-policy note) |
| D7 | C7 = authored policy text only (spec + design §4 + this handoff) | contract: "no code change"; S6 owns `ReadMe.md` (R-13 single-writer) |
| D8 | Suite case pins (a)–(f); int pin removed (documented in comment (d)) | (f) `!noexcept` is the load-bearing engine pin (reverting to `srand` breaks the build); (d) cannot exist as a test — it is the absence of compilation |
| D9 | Acceptance grep refinements (Q6.1/Q6.2) logged, intent-preserving | both literal patterns proven unsatisfiable-by-construction (pre-existing typedef false positive; variable naming); refinements are the minimal intent-faithful reading |
| D10 | Sharded review + adversarial verification in-process (fresh-context simulation) | host subagent output-budget exhaustion (S1/S4 record, re-confirmed); documented in both reports |

## Next Priority Queue

1. **S6** (per PRD §6): A2 alias/retirement audit — note the current `noexcept` state of the `rand`-family (D3) so it isn't "restored"; consume the **C7 policy delta** (design.md §4 = `specs/ndebug_policy.md` text) into the ReadMe; read-only `random`/`random_like`/`randn_like` bodies were not touched here.
2. Future I/O-boundary session: `load_binary` hazard class (S2 finding, carried in the risk register) + `save_as_png`-returns-true-on-noop + disk-full-mid-write (both in S5 watch items).
3. If a future session wants seed-0 to be non-correlated across same-second same-site calls, that is a **new seed-policy decision** (out of S5 scope; D2 preserved the documented expression).

## Warnings And Gotchas

- **Explicit-seed streams changed** (sanctioned, PRD row 13) — pre-fix values are recorded in `s5_prefix.log`; do not "restore" them; do not treat the one-line `make example` delta (0019 LU MAE) as a regression.
- **`rand<int>` / `rand<std::complex<…>>` do not compile** post-fix — if you see the static_assert "result_type must be a floating point type", that is the documented consequence (D4), not a bug to fix by swapping distributions.
- **Line 279 `total_cores <= 1` is the pre-existing guard** — do not "unify" it with the new clamps (it guards a different function's early-exit; different variable lifetime).
- **The suite build has asserts live** (no `-DNDEBUG`) — the C7 policy text describes the `debug_mode` mechanism at `matrix.hpp` 57–61 as it exists at this commit.
- **TSan is blind to libc internals** — a "clean" TSan run pre-fix did not prove absence of the race (it was in libc); the proof is the no-global-state grep + the per-call-local design.
- Subagents on this host exhaust their 16K output budget — keep any future subagent units pasted-only with short outputs (re-confirmed S5).

## Eval Seeds

- **E14 promoted** (C11): suite case `tests/cases/rand.hpp` ("rand: explicit-seed determinism, [0,1) range, and engine pins (C11/E14)") + probe `.work/probes/E14_E15.cc` E14 block; PASS post-fix (runId 5dca4f6). Pre-fix red = the `(f)` `!noexcept` static_assert compile failure + structural (grep/TSan/correlation evidence).
- **E15 live** (save_png boundary): probes `.work/probes/S5_p2_save_png.cc` + `E14_E15.cc` E15 block; probe-only by design (no permanent suite home; `save_as_png` return semantics are S6's); pre-fix SIGSEGV 139 → post-fix exit 0 + positive control.
- All other seeds (E01–E13 promoted, E16–E19 seeded) unchanged by S5; the seed-0 user in `tests/cases/inverse.hpp` stays green (value-agnostic — watch item from the standing list, satisfied).
