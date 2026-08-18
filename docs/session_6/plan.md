# Session 6 — Plan

## Command plan (all from the repository root)

| Step | Command | Expectation |
|---|---|---|
| Baseline (done) | `make test && ./test_test && make example && ./test_example` | 74 cases / 49,217,191 assertions, all pass (matches S5 closeout); example exit 0; `.work/evidence/s6_baseline.log` |
| Preflight (done) | probe `S6_prefix_c13_p1.cc` (`-O1`) | `.work/evidence/s6_prefix_probe.log`: pre-fix `fft` = `R·C·x[0][0]·E₀₀` (F1), swap-vs-roll n=4 equal / n=5 mismatch (C13) |
| T1 red | `make test && ./test_test` | new `fft` case FAILS (differential fast+fallback, E16, E17 odd-n, normalization-once); even-dim pin + 0×0 guard PASS; others green; red logged |
| T2 | `make test && ./test_test` + probe + benchmark | 75 cases green; `g++ -std=c++20 -DPARALLEL -O1 -o .work/probe_s6 .work/probes/E16_E17.cc && .work/probe_s6` → `E16_E17 PASS`; 512×512×100 benchmark pre-fix (scratch copy of pre-fix header) vs post-fix, 10–100× gate |
| T3 | `make test && ./test_test` | 75 cases green (E17/odd-n scenarios turn green; even-dim pin stays); probe still PASS |
| T4 | E18 probes + `make test` + `make example` + greps | E18 negative compile FAILS naming a retired identifier; E18 positive compiles+runs; suite green; example renders (0013 via `feng::rand`); `pinverse\|svd_inverse` count 0; `random\b` match list = exactly the include line (F3) |
| T5 | `make test` + `git diff tests/cases/pinv.hpp` | green; diff = alias scenario + naming only |
| T6 | grep checklist (design §4) | all 11 sweep items present; S2/S5 texts verbatim vs source; code-fence parity even |
| T7 full | `make test && ./test_test && make example && ./test_example` + probe + greps | everything green at closeout; `.work/evidence/s6_final.log` |
| T7 review | in-process sharded review (6 shards × 6 axes) | `docs/session_6/sharded_review.md`; Critical/High fixed with regression evidence |
| T7 adversarial | in-process fresh-context verification | `docs/session_6/adversarial_verification.md`; done condition verified line by line |

## TDD red states (pre-fix verified where applicable)

| Task | Red | Mechanism |
|---|---|---|
| T1 | 8×8/6×8 differential FAIL; E16 `max‖X−1‖ = 63` (not < 1e-9); E17 odd-3/odd-5 orders wrong (n=5: `(3,4,2,0,1)` vs `(2,3,4,0,1)`); normalization-once FAIL (pre-fix `ifft∘fft = (RC)²·x[0][0]·E₀₀`) | REQUIRE failures in the suite TU (pre-fix `fft` returns the rank-1 corner, F1) |
| T1 (green pre-fix, expected) | even-dim swap-of-halves pin, 0×0 guard | they pin pre-existing behavior (degenerate but stable corner value + permutation) — recorded as the intentional split |
| T4 | E18 negative probe compiles pre-fix (the names exist) | the probe's red state is "compiles"; post-fix it must fail to compile |

## Failure classification policy (project contract §1.3)

Any check failure: classify (FIX / TEST_BUG / ENVIRONMENT / SPEC_GAP /
AMBIGUITY) with the Failure Arbiter before acting. Deterministic evidence only.
Pre-flight classifications F1–F3 are already logged in
`docs/session_6/failure_arbiter.md`.

## Self-critique checkpoints (per task, before commit)

1. Does the diff touch only the sanctioned regions? (`git diff` audit against
   the previous commit; T2 = FFT region only, T4 = A2 regions only, T6 =
   ReadMe only, T5 = the logged one file.)
2. Does the test assert content (values), not just compilation/no-abort?
3. Is the test R-19-safe (tolerances, finite values, no NaN assertions) for
   `-Ofast`?
4. Does the change preserve the documented out-of-scope behaviors (S5 `rand`
   seed-0/time behavior + `#include <random>`, `add_complex` promotion,
   `pinv` threshold `|w| > 1e-10`, member `det()`, `randn_like` existence)?
5. Is the oracle still frozen (no post-S6 edits to the embedded naive-DFT
   copy's math — R-18)?

## Sharded review plan (risk = medium)

Shards: S1 `matrix.hpp` FFT region (radix-2 kernel, twiddles, path selection,
`ifft` scaling), S2 `matrix.hpp` shift region (`shift_roll`, both public
functions), S3 `matrix.hpp` A2 regions (`pinv_core` + ADL, deletions,
like-family re-pointing, free `det`), S4 `tests/cases/fft.hpp` +
`tests/test.cc` (oracle fidelity, R-19 safety, registration), S5
`tests/cases/pinv.hpp` + `examples/cases/0013_prefix.hpp` (F2 minimalism,
consumer correctness), S6 `ReadMe.md` (all 11 sweep items verbatim/content
check). Six axes per `docs/prompts/sharded_review.md`. Findings triaged:
Critical/High → fix + regression evidence; Medium/Low → record or fix with
justification. Dedupe across axes.

## Adversarial verification plan

Fresh-context simulation (read only: contract `adversarial_cases`, the diff,
the evidence logs — not the design docs), per
`docs/prompts/adversarial_verifier.md`: attempt to falsify each adversarial
case with an actual build+run (normalization on `fft` by mistake; double
application; odd-even mixed shapes 3×5/5×3/3×4/4×3; fallback selection 6×8 and
126×128; E18 compile probe; no dangling references), then check the done
condition line by line. Subagent note: on this host subagents exhaust the
output budget (S1/S4 record) — the fresh-context simulation is in-process;
the verifier report states this limitation.
