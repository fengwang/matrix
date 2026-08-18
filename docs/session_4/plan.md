# Session 4 — Plan

## Command plan (all from the repository root)

| Step | Command | Expectation |
|---|---|---|
| Baseline (done) | `make test && ./test_test` | 69 cases / 49,217,068 assertions, all pass (matches S3 closeout) |
| Preflight (done) | probes p0/p0b/p0c/p1/p2/p3 | recorded in `.work/evidence/prefix_p0.log` |
| T1–T4 per task | `make test && ./test_test` | all pass after green; case count grows 69 → 73 (4 new/extended cases) |
| T5 probe | `g++ -std=c++20 -DPARALLEL -O1 -o .work/probe_s4 .work/probes/E10_E13.cc && .work/probe_s4` | prints `PASS` (contract's deterministic check, verbatim) |
| T5 ASan | `g++ -std=c++20 -DNDEBUG -DPARALLEL -O1 -fsanitize=address -o .work/probe_s4_p3post .work/probes/S4_p3_wide_asan.cc && .work/probe_s4_p3post` | ASan report identical in substance to pre-fix p3 |
| T6 full | `make test && ./test_test` | green |
| T6 example | `make example && ./test_example > .work/evidence/s4_example_stdout.log` | stdout identical to S3 baseline (example 0020 wide case unchanged); `git checkout -- images/` after |
| T6 review | in-process sharded review (4 shards × 6 axes) | `docs/session_4/sharded_review.md` |
| T6 adversarial | in-process fresh-context verification | `docs/session_4/adversarial_verification.md` |

## TDD red states (pre-fix verified)

| Task | Red | Mechanism |
|---|---|---|
| T1 | compile error (int variance/stddev) + mean value `1 != 1.5` | suite TU fails to build / REQUIRE fails |
| T2 | SIGABRT (134) on 1×1 same kernel | assert live in suite build |
| T3 | SIGABRT (134) on square rref | assert live in suite build |
| T4 | compile error (`bool ok = void-return`) | suite TU fails to build |

## Failure classification policy (project contract §1.3)
Any check failure: classify (FIX / TEST_DEFECT / ENVIRONMENT) before acting. Deterministic
evidence only (probe logs, ASan reports, diff). No narrative claims without a captured
artifact.

## Self-critique checkpoints (per task, before commit)
1. Does the diff touch only the sanctioned lines? (`git diff` line-count audit.)
2. Does the test assert content (values), not just compilation/no-abort?
3. Is the test R-19-safe (tolerances, finite values, no NaN assertions) for `-Ofast`?
4. Does the fix preserve the documented out-of-scope behaviors (valid conv, wide rref,
   1e-10 exit, n−1 formula, complex paths)?

## Sharded review plan (risk = medium)
Shards by file: S1 `matrix.hpp` statistics region (C8), S2 `matrix.hpp` conv+rref regions
(C9/C10), S3 `matrix.hpp` cholesky region (P2b) + `tests/test.cc` registrations, S4
`tests/cases/*` (all new/extended test files) + docs. Six axes per
`docs/prompts/sharded_review.md`. Findings triaged: Critical/High → fix + regression
evidence; Medium/Low → record or fix with justification. Dedupe across axes.

## Adversarial verification plan
Fresh-context simulation (read only: contract `adversarial_cases`, the diff, the evidence
logs — not the design docs), per `docs/prompts/adversarial_verifier.md`: attempt to falsify
each adversarial case with an actual build+run, then check the done condition line by line.
Subagent note: on this host subagents exhaust the output budget (S1 record) — the
fresh-context simulation is in-process; the verifier report states this limitation.
