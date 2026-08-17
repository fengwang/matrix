# Session 5 — Robustness: `rand` reimplementation, core-count guards, `save_png`, NDEBUG policy (C7, C11, C12, S2-finding, R3-slice)

> Story outline v1. Refine at start (narrow/clarify only), policy P9.
> Machine-readable authority: `docs/session_5_contract.yaml`. Law: `docs/project_contract.md`.
> Independent of the S1→S3→S6 chain. Must complete before S6 (doc deltas).

## Objective

Close the robustness findings: replace the global `srand`/`rand` in `rand` with a per-call local engine (thread-safe, deterministic under explicit seed) (C11), guard the two unguarded `hardware_concurrency()` sites against a 0 return (C12), make `save_png` survive a failed `fopen` and drop the stray `;;` (S2-finding + R3 slice), and produce the `NDEBUG` policy doc delta (C7) that S6 lands in the ReadMe.

## Story

The review's stage-4 "robustness half". **C11** is the real work: `rand` currently re-seeds the single global generator from `time + &ans` on every seed-0 call and draws from `std::rand()` — correlated within a second, and a data race if two threads fill matrices concurrently (the library's own algorithms run multi-threaded; this hazard is latent, risk register R-07 notes the value-stream change). The fix is a per-call local `std::mt19937` + `std::uniform_real_distribution<T>(0,1)`: seed 0 ⇒ time-based seed (non-deterministic, as today's intent), **explicit seed ⇒ deterministic stream (hard invariant — examples 0012/0019/0020/0021 rely on it, verified)**. `noexcept` is dropped (the allocation can throw; contract §3 allows this without a table row). **C12** is two one-line guards matching the existing guarded pattern at ~276. **S2-finding** is the I/O-boundary discipline (policy P3): `save_png` checks `fopen` and no-ops on failure — documented, never UB; the stray `;;` dies with it. **C7** ships as a doc delta (the policy decision was made in the PRD: `better_assert` stays debug-only and gets *documented*; I/O boundaries use hard checks — which S5's `save_png` and S2's `load_npy` now exemplify). No code change for C7 itself.

## In scope

- C11: `rand` (anchor 5240) — local `std::mt19937` (seed: explicit ⇒ as given; 0 ⇒ `std::time(nullptr)+address`-equivalent, non-deterministic), `std::uniform_real_distribution<T>(0.0, 1.0)`; `rand_like` (anchor 5272) semantics preserved (delegates, seedless ⇒ non-deterministic); `noexcept` dropped on `rand`.
- C12: `reduce` (anchor ~1152) and the second site (anchor ~4036) — `if ( total_cores < 1 ) total_cores = 1;` mirroring the guard at ~276.
- S2-finding: `save_png` (anchor 3096) — `if ( !fp ) return;` after `fopen`; remove stray `;;` (R3 slice at ~3105).
- C7: **no code change** — author the exact policy text as a doc delta for S6 (NDEBUG = `better_assert` disabled; debug-only checks; I/O boundaries use hard runtime checks, never asserts; cite the `load_npy`/`save_png` exemplars).
- Tests: new `tests/cases/rand.hpp` (E14: determinism, inequality across seeds, range); no Catch case for C12 (acceptance = guard presence, not executable on this host) or the `save_png` no-op (E15 probe).
- Eval probes E14, E15.

## Out of scope

- The `better_assert` macro itself (no conversion — C7 policy is documentation, decision C-04); the `parallel` helper at ~276 (already guarded); `rand`'s value stream beyond the engine swap (documented, row 13); other `srand`/`rand` uses (none exist outside the C11 site — verified by grep at pre-flight); `ReadMe.md` (delta emitted, P4); `examples/`.
- `magic` (the other `n & 1` at ~4721 is `magic`, not `operator^` — do not touch).

## Deliveries

1. Four code changes in `matrix.hpp` (rand body; two guards; save_png guard+`;;`).
2. `tests/cases/rand.hpp` + registration.
3. E14/E15 probes live.
4. Handoff with the C7 policy text (exact wording for S6's ReadMe) + doc deltas for C11 (thread-safety, seed semantics), S2-finding (save_png no-op), C12.

## Context budget map (~40K of 128K — do not exceed)

| Read | How much | Why |
|---|---|---|
| `AGENTS.md`, `docs/project_contract.md` §3–§5 | ~2.5K | law |
| `docs/prd.md` §5 rows 12–14, §7 P2/P5/P8 | ~1.5K | authorization + policy text inputs |
| `docs/opencode_sharded_review.md` §C7 §C11 §C12 §S2 §R3 | ~3.5K | findings + fixes |
| `matrix.hpp` regions: 80–100 (better_assert macro, for the policy text), 1145–1170 (reduce), 270–285 (the guard pattern), 3090–3120 (save_png), 4030–4045 (second site), 5240–5290 (rand/rand_like/random* — read but **do not edit** the random* aliases, S6 territory) | ~9K | the code |
| `tests/test.cc` + one case for style | ~2K | patterns |
| `docs/eval_seed_cases.md` E14–E15 | ~0.5K | probe specs |
| **Do NOT read:** rest of `matrix.hpp`, ReadMe, research docs in full | — | budget |

## Deep-research references

- Report 6 §"Layout, performance, execution, safety and correctness" (line 511) and the host-execution-model paragraph (~line 545): thread-safety under the standard host execution model — the frame for the C11 invariant. Section only.

## Pre-flight (mandatory)

1. `git status` clean; baseline commit.
2. Re-anchor all six regions by grep; **grep the whole header for `srand`/`std::rand` and confirm the C11 site is the only one** (if more exist, report — do not silently fix them).
3. Probe-first (P5): record the current seed-0 correlation (two seed-0 `rand` calls in the same second → identical matrices) and that `rand(r,c,seed)` with explicit seed is deterministic today (must remain so after the swap — E14 pins both sides).
4. Confirm `tests/cases/inverse.hpp` (seed 0) passes before the change; re-run it after and note the value change (expected; value-agnostic test, R-07).

## Exit criteria

1. `make test` green (suite + new `rand.hpp`).
2. E14 passes (`a==b` same explicit seed; `a!=c` different seed; all values in `[0,1)`); E15 passes (unwritable path ⇒ exit 0, no crash).
3. Guard presence check: `grep -c "total_cores < 1" matrix.hpp` ≥ 2 in the C12 sites (deterministic substitute for the unreachable-0 test; R-14).
4. `git diff --name-only HEAD` ⊆ {`matrix.hpp`, `tests/test.cc`, `tests/cases/rand.hpp`, `.work/**`, `docs/eval_seed_cases.md`, `docs/risk_register.md`}.
5. Sharded review + adversarial verifier PASS (verifier focus: any residual global mutable state in the rand path; `uniform_real_distribution<T>` behavior for `T=int`?? — **rand is only instantiated with floating T in-repo; if an integer instantiation is feasible, the distribution's `int` specialization must be sane — check and document**; the `noexcept` drop on `rand` and its effect on `rand_like`).

## Risk and routing

- Risk level **medium** (new logic in a widely-used function + sanctioned behavior change; the concurrency angle is *removing* a shared global, and the user-approved classification stands — see risk register). Routing: **worker_plus_reviewers**.
- Failure modes to watch: explicit-seed determinism broken (E14 regression — the examples' reproducibility depends on it); `T`-specific distribution surprises (`uniform_real_distribution<int>`); the second `hardware_concurrency` site at ~4036 missed (it is **not** the `parallel` helper — that one is already guarded); editing the `random`/`random_like` aliases (S6 territory, anchors 5262/5267/5278 — read-only here).

## Handoff requirements

State snapshot (compiler, checks run/not run); decision log (pre-fix correlation demo; seed-semantics restatement; contract refinements); eval seeds E14/E15 live; **C7 policy text** (final wording for S6's ReadMe); doc deltas for C11/C12/S2-finding; warning for S6: `random`/`random_like` aliases read but untouched — S6 retires them (A2) and their `rand` delegation now sits on the new engine.

## Contract

`docs/session_5_contract.yaml` — read before pre-flight; authority for scope, invariants, checks.
