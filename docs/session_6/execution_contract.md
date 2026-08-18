# Session 6 — Execution Contract (pre-execution confirmation)

Status: **confirmed** — all refining phases complete, pre-flight done, no
blocking open questions. One escalated item (F2 blast-radius refinement) is
reported in `interview.md` Q1 and the session's final answer.

## Contract alignment (docs/session_6_contract.yaml)

| Contract element | Disposition |
|---|---|
| In-scope: P1 (fft/ifft fast + normalization) | T2; approach A (whole-matrix selection, strided DIT) — `proposal.md` |
| In-scope: C13 (fftshift/ifftshift odd n, NumPy-pinned, probe-first) | T3; approach A (`shift ∘ transform`, shared `shift_roll`) — pre-fix probe done (F1 also captured pre-fix P1 baseline) |
| In-scope: A2 (retire random/random_like/pinverse/svd_inverse/free det; pinv core → `matrix_details::pinv_core`) | T4; core moves verbatim (hard-coded `|w| > 1e-10`; no tolerance parameter exists — verified against code) |
| In-scope: A3 (verification pass + namespace hygiene note) | T4 greps + T7 note; pre-flight recorded in `specs/a3-verification.md` |
| In-scope: ReadMe sweep (S2–S5 deltas + FFT + aliases) | T6; 11-item checklist in `design.md` §4; S2 text verbatim from S2 handoff, S5 NDEBUG verbatim from `docs/session_5/design.md` §4 |
| In-scope: E16/E17 probes (contract-verbatim) + E18 compile probe | T1–T4; `.work/probes/E16_E17.cc`, `.work/probes/E18_a2.cc` + `E18_ok.cc` |
| In-scope: `docs/eval_seed_cases.md` E16/E17 promotion | T7 |
| Out-of-scope (S7–S11 work) | untouched |
| Exit criteria | mapped to T2/T3/T4/T7 checks in `plan.md` |
| Adversarial cases | mapped to `specs/fft-tests.md` + the adversarial verification plan in `plan.md` |
| Risk medium → sharded review (6 axes) + adversarial verifier | `plan.md` (in-process; subagent limitation disclosed, S1/S4 precedent) |

## Refinements logged at session start (policy P9; never silent)

1. **F1 (SPEC_GAP):** the pre-fix `fft`/`ifft` loops are not a DFT (data-index
   bug `x[r][c]` vs `x[r_][c_]`; measured baseline `fft(x) = R·C·x[0][0]·E₀₀`).
   The oracle is the *corrected* naive DFT (one-token fix, frozen per R-18);
   the pre-fix baseline is the measured broken behavior (supersedes the
   contract's "R·C·x" note and PRD note C-08).
2. **F2 (SPEC_GAP + TEST_BUG):** `tests/cases/pinv.hpp` (outside blast radius)
   calls `feng::pinverse` (A2 deletes it). Refinement: that file enters the
   blast radius for the canonical-name update only (alias scenario removed,
   TEST_CASE renamed, comments fixed). **Reported to the user** (project
   contract §1.2).
3. **F3 (AMBIGUITY):** A3's `grep -c 'random\b' == 0` is unsatisfiable while
   A2's own invariant keeps `#include <random>` (line 29 matches the pattern;
   the include is required by S5's `mt19937`). Gate interpreted over
   identifier use: post-fix match list is exactly the include line, recorded
   and verified.

## Boundaries honored

- No production dependencies added (TBB/parallel backends explicitly deferred
  to session 11; the parallelism opportunity is recorded, not implemented).
- No public API behavior change except the sanctioned ones (fft/ifft
  semantics now the true DFT per the contract; alias retirements per A2).
- Blast radius: `matrix.hpp`, `tests/test.cc`, `tests/cases/fft.hpp` (new),
  `ReadMe.md`, `examples/cases/0013_prefix.hpp`, `docs/evidence_map.md`,
  `docs/eval_seed_cases.md` — plus the logged F2 refinement
  (`tests/cases/pinv.hpp`) and the `docs/session_6/` phase docs.
- S1–S5 code regions untouched except the documented A2 deletions and the F2
  refinement; S5's `rand` (5329–5352) and `#include <random>` verbatim-kept.

## Verification commitments (deterministic evidence only)

- `.work/evidence/s6_baseline.log` (done), `s6_prefix_probe.log` (done),
  `s6_t1_red.log`, `s6_e16_e17.log` (pre + post), `s6_bench_prefix.log` +
  `s6_bench_postfix.log`, `s6_e18.log`, `s6_final.log`.
- Suite: 74 → 75 cases, all green, `-Ofast`-safe assertions (R-19).
- Probe: `E16_E17` PASS post-fix (verbatim contract probe, `-O1`).
- Grep gates listed (match lists, not just counts).
- Benchmark: pre-fix O(n⁴) vs post-fix radix-2 on 512×512 ×100 serial (the
  pre-fix measurement uses a scratch copy of the pre-fix header in `.work/`,
  so the post-fix tree is never modified by the benchmark).

## Commits (one per task)

T1 (red test, the intentional red — its done criterion is the recorded red
state, evidence `s6_t1_red.log`) → T2 P1 (suite green again) → T3 C13 → T4
A2 → T5 F2 → T6 ReadMe → T7 closeout. The session-boundary commit (T7) is
fully green; intermediate commits follow TDD (red at T1, green from T2).
