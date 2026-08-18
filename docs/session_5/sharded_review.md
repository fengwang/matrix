# Session 5 — Sharded Review

Per `docs/prompts/sharded_review.md` (risk medium → 4 shards × 6 axes). Contract
review axes: correctness, security, tests, architecture, performance, readability.
Each shard re-read the actual on-disk code (not the design docs) and scored every
axis. Subagent note (S1/S4 record): on this host subagents exhaust their output
budget — shards ran as separate in-process review passes over the diff
`git diff c40b04b..HEAD` (6 matrix.hpp hunks: 1149, 3183–3191, 4119, 5318, 5352–5377).

**Overall verdict: PASS — no Critical or High findings; 5 Low/informational notes,
none blocking; none require fixes with regression evidence.**

## Shard 1 — `rand` region (5327–5345) + alias declarations (5352–5377) + `tests/cases/rand.hpp` + `test.cc` registration

| Axis | Score | Notes |
|------|-------|-------|
| correctness | PASS | Seed-0 expression bit-identical to pre-fix; per-call locals (`engine`, `distribution`) — no shared state; `[&]` lambda captures live for the `std::generate` call; `static_cast<T>` on an already-`T` distribution result is a no-op (documented as the [uniform.real] enforcer); alias **bodies** untouched (S6 territory), only `noexcept` specifiers dropped where the new `rand` exception behavior makes them `std::terminate` traps. |
| security | PASS | Global-state race eliminated by construction; no new UB; the `reinterpret_cast(&ans)` was pre-existing and is unchanged. |
| tests | PASS | (a) determinism 64×64 + 7≠8; (b)/(c) [0,1) double + float (4096 samples each); (e) type pins; (f) `!noexcept` pin — load-bearing: reverting the engine to `srand` breaks the build. Gap (Low): seed 1 (the examples' seed) is pinned by the AV probe + cross-process P0 check, not the suite — acceptable (the suite pins the property, not every seed). |
| architecture | PASS | No new includes (`<random>` already at line 29); header-only preserved; per-call engine cost is the contract-prescribed design. |
| performance | PASS (note) | `mt19937`+distribution per call replaces 2 libc calls per element with a stateful engine — throughput for large matrices is comparable-or-better than `rand()` (which was also stateful + lock-free-per-thread *but* shared across threads); no hot-path regression measurable in the suite. |
| readability | PASS | Updated `[0, 1)` comment; seed-0 comment explains the residual correlation to future readers; `effective_seed` names the ternary. |

Findings: **L-1** (informational) — a future reader "fixing" the int-T compile error by
swapping the distribution would break the contract-prescribed engine; the suite's (d)
comment explicitly warns. No action.

## Shard 2 — C12 sites (1152–1155, 4125–4131)

| Axis | Score | Notes |
|------|-------|-------|
| correctness | PASS | `total_cores` clamp (0→1) precedes `cache.resize` and the `total_elements / total_cores` division — the division-by-zero / zero-size classes are closed; empty-range `accumulate` path (0 elements) returns `init` as before. 4121 clamp is behavior-neutral: `0 → 1` falls into the pre-existing `parallel_size <= 1` short-circuit exactly as an unguarded 0 did. |
| security | PASS | No new input surface; clamp fires only at 0 (`< 1` on unsigned). |
| tests | PASS (structural) | `hardware_concurrency()==0` is not forceable on this host (verified: `taskset -c 0` floors it at 1); acceptance is code presence (contract) — greps = 1/1; the parallel reduce paths are exercised heavily by the 74-case suite (green). |
| architecture | PASS | Mirrors the existing guarded pattern's intent at line 279 (left verbatim — no double-fix). |
| performance | PASS | Zero-cost on 1-N-core machines (one extra comparison). |
| readability | PASS | Two-line clamp at each site, named consistently with the contract. |

Findings: **L-2** (informational) — at 4134 `std::vector<T> result_cache( parallel_size )`
sits inside a `noexcept` lambda; a `bad_alloc` there would `std::terminate`. Pre-existing
condition, outside S5 scope (S6 owns the noexcept audit). Logged for the risk register.

## Shard 3 — `save_png` (3183–3192, 3436 `fclose`) + `save_as_png` caller (3444–3448)

| Axis | Score | Notes |
|------|-------|-------|
| correctness | PASS | Guard `if ( ! fp ) return;` immediately after `fopen`, before **all** I/O; every later `fputc`/`fclose` (3436) is post-guard, so the open-failure path is null-free. Stray `;;` removed (3192). `noexcept` correctly kept (the failure mode is a `nullptr` return, not an exception). Positive control: 135-byte PNG written on a writable path. |
| security | PASS | UB (SIGSEGV, exit 139) on open failure → silent no-op, exit 0. No new path-handling surface (caller-supplied paths as before). |
| tests | PASS | E15 red→green with exit codes recorded (139 → 0) + positive control + the AV probe's `save_as_png` return check. |
| architecture | PASS | Signature unchanged; failure policy = documented silent no-op (matches the S2 `load_npy` precedent); member `save_as_png` still returns `true` (documented, out of scope). |
| performance | PASS | One branch; no cost on the success path. |
| readability | PASS | Matches house style (`if ( ! x )` spacing). |

Findings: **L-3** (informational) — `fopen` succeeds but the disk fills mid-write:
`fputc`/`fclose` failures remain unchecked (pre-existing). Documented known limitation,
no fake handling (contract adversarial case 5). **L-4** (informational) —
`save_as_png` returns `true` even when the underlying write was a no-op; surfaced in
handoff for S6's I/O-policy pass.

## Shard 4 — session docs (`docs/session_5/**`), evidence (`.work/evidence/s5_*`), probes

| Axis | Score | Notes |
|------|-------|-------|
| correctness | PASS | Stale-claim scan clean: the superseded "int unchanged / all zeros" and the `FILE* const fp` / `"wb+"` plan citation appear **only** in explicitly-labeled correction/discrepancy contexts (AV report, design §3 note, interview Q4). Line-number citations in execution_contract updated to post-edit reality (3187–3189). |
| security | PASS | Evidence logs carry exact commands + outputs (deterministic); no secret material; probes build with `-O1`, no unsafe flags. |
| tests | PASS | Every done-condition claim cites a log (`s5_prefix`, `s5_t1_red`, `s5_t2_green`, `s5_t3_green`, `s5_t4_green`, `s5_av_*`, `s5_example_delta.txt`). |
| architecture | PASS | Doc set follows the S1–S4 template (interview → brainstorming → proposal → design → specs → tasks → plan → execution contract); the five spec files each carry `### Requirement` + `#### Scenario` per the four-hash rule. |
| performance | PASS | n/a (docs). |
| readability | PASS | Discrepancies (4121 already short-circuited; literal-grep unsatisfiability ×2; plan citation drift) are logged in-place with resolutions, not silently absorbed. |

Findings: **L-5** (informational) — `docs/eval_seed_cases.md` + `docs/risk_register.md`
deliveries are T8 (this review precedes closeout); both are in the allowed blast radius.

## Triage summary

| ID | Severity | Disposition |
|----|----------|-------------|
| L-1 | Low | Documented (suite comment (d)) — no action |
| L-2 | Low | Pre-existing; risk register (S6 territory) |
| L-3 | Low | Documented known limitation — no action (contract) |
| L-4 | Low | Handoff note for S6 |
| L-5 | Low | T8 deliverable — no action |

No Critical/High → no re-run of full checks beyond those already recorded (final full
check after this report is part of T8 closeout).
