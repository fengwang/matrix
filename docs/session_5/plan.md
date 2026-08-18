# Session 5 — Plan

## Command plan (all from the repository root)

| Step | Command | Expected |
|------|---------|----------|
| T1 build+run | `make test && ./test_test` | 74 cases, all pass (pre-fix engine) |
| T2 build+run | `make test && ./test_test` | 74 cases, all pass (new engine) |
| T2 grep | `grep -cE 'srand\(|std::rand\(' matrix.hpp` | 0 |
| T2 TSan | `g++ -std=c++20 -DPARALLEL -fsanitize=thread -O1 -o .work/evidence/tsan_post .work/probes/S5_p1_tsan.cc && .work/evidence/tsan_post` | `T SAN CLEAN`, no report |
| T2 probe re-run | `.work/evidence/seed_S5_p0` (rebuilt) | determinism 7==7/7≠8; value streams differ from P9 (R-07) |
| T3 grep | `grep -c 'total_cores < 1' matrix.hpp && grep -c 'parallel_size < 1' matrix.hpp` | 1 and 1 |
| T4 probe | `g++ -std=c++20 -DPARALLEL -O1 -o .work/evidence/seed_S5_p2 .work/probes/S5_p2_save_png.cc && .work/evidence/seed_S5_p2` | `PASS E15`, exit 0 |
| T6 verbatim | `g++ -std=c++20 -DPARALLEL -O1 -o .work/probe_s5 .work/probes/E14_E15.cc && .work/probe_s5` | `PASS` |
| T6 examples | `make example` + record stdout delta + `git checkout -- images/` | delta recorded (R-07) |

## TDD red states (pre-fix verified — `s5_prefix.log`)

| Task | Red | Evidence |
|------|-----|----------|
| T1 | none by design (invariant pin; determinism/range hold pre-fix) | P9 |
| T2 (C11) | **structural**: grep = 3 global-state lines; TSan blind to the libc-internal race (documented); per-call-site seed-0 correlation | P1/P8/P10 |
| T3 (C12) | structural: both grep counts = 0 (0-core not forceable in-env) | P3 |
| T4 (png) | **executable**: E15 probe SIGSEGV, exit 139 | P7 |
| T5 (C7) | none (doc capability) | — |

## Failure classification policy (project contract §1.3)

- **BUG** (change defect) → fix, re-run targeted check, commit.
- **SPEC_GAP/AMBIGUITY** (contract unsatisfiable as written) → stop, surface; the
  logged grep refinements (interview Q6) are intent-preserving, already resolved.
- **ENVIRONMENT** (host/toolchain blocks a check) → record evidence first (e.g. the
  TSan libc limitation, P10), classify, then proceed via the contract's fallback
  clause if it names one.
- **PRE-EXISTING** (e.g. the `load_binary` adjacent warning) → risk register only.

## Self-critique checkpoints (per task, before commit)

- T1: Catch quirk respected (local bools); no NaN asserts; int pin = documented
  consequence; include alphabetical.
- T2: seed-0 expression bit-identical to pre-fix; engine/distribution
  contract-prescribed; noexcept chain complete (4 lines); no new includes; aliases'
  *bodies* untouched (S6).
- T3: clamps fire only on 0; 279 verbatim; diff = 4 lines.
- T4: happy path byte-identical; `save_as_png` untouched; `load_binary` untouched.
- T5: mechanism line-checked; precedents named; no code touched.

## Sharded review plan (risk = medium)

Per `docs/prompts/sharded_review.md`, shards by region (6 axes each: correctness,
readability/simplicity, security/safety, tests, architecture, maintainability):
- **S1** — `matrix.hpp` rand region (5320–5370) + `tests/cases/rand.hpp` + `tests/test.cc` registration.
- **S2** — `matrix.hpp` C12 sites (1152/4121) + surrounding `reduce`/`reduce_impl` code.
- **S3** — `matrix.hpp` `save_png` region (3181–3190) + `save_as_png` call site (3469).
- **S4** — docs (`docs/session_5/**`, `eval_seed_cases.md`, `risk_register.md`) + evidence (`.work/**`).

Findings triaged: Critical/High → fix + regression evidence; Medium/Low → record or
fix with justification. Dedupe across axes. Subagent note (S1/S4 record): on this
host subagents exhaust the output budget — shards run in-process with fresh context
per shard; the report states this limitation.

## Adversarial verification plan

Fresh-context simulation (read only: contract `adversarial_cases`, the diff, the
evidence logs — not the design docs), per `docs/prompts/adversarial_verifier.md`:
attempt to falsify each adversarial case with an actual build+run (seeds 0/1/large;
float vs double; int; two threads; complex-T instantiation attempt; noexcept
impact; the 3469 call site; R-07 examples), then check the done condition line by
line → `docs/session_5/adversarial_verification.md`.
