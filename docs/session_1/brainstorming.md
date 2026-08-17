# Session 1 — Brainstorming (refinement record)

Status: refinement only (policy P9 — narrow/clarify, no scope widening). The problem space was
already explored in the 2026-08-17 blueprint interview (PRD §2) and the 2026-07-13 sharded review.
This document records the session-start interview-me pass, the design decisions, and the validated
design. No new exploration.

## Interview-me pass (stress-test of my thinking)

Question format: what could still make this session fail or ship the wrong thing? Each question was
resolved against the contract set before any edit; nothing remains that needs a user answer, because
every shared decision is already fixed by `docs/prd.md` §5 rows 1–2, `docs/session_1_contract.yaml`,
and `docs/project_contract.md`.

| # | Question | Resolution (source) |
|---|---|---|
| 1 | Which exact lines change? | The review's smallest-safe-fixes, re-anchored this session: `matrix.hpp:3532` (`the_rows_to_copy` → `the_cols_to_copy`) and `matrix.hpp:4479` (third arg `row_begin` → `col_begin`). Code-verified 2026-08; anchors re-confirmed by grep at pre-flight. |
| 2 | What is the documented contract the fixes must restore? | `shrink_to_size`: in-code comment ~3515–3517 ("padding with zero" on growth, "drop these elements" on shrink) + PRD §5 row 1. `flipdim(m,2)`: left-right flip for all shapes, per the parallel structure of the dim==1 branch and the public `fliplr`/`flipud` API (PRD §5 row 2; review §C2). |
| 3 | Does the eval-seed sketch ("rows = 1 1 1 0 0 pattern") conflict with the contract acceptance? | The sketch conflates shrink (truncation, no padding) with growth (zero-pad). The contract acceptance criteria win (authority chain). Probes use **ragged values** so "first 3 cols preserved" is verifiable by content, not by a 1.0 fill (failure-mode guard from the contract). |
| 4 | Are `fliplr`/`flipud` touched? | No. C3 is S3's; anchors 4491–4499 stay untouched (contract `out_of_scope`). The dim==1 branch of `flipdim` is also untouched; it is pinned by a regression case anyway. |
| 5 | What do the tests assert? | **Content, not shape** (T1 anti-pattern guard): exact expected values on ragged matrices, non-square both directions, grow-only and shrink-only extremes, 1×N / N×1 for flipdim. |
| 6 | How is "tests catch the bug" proven? | Empirically: restore each original buggy line in turn → the new test cases must FAIL → restore the fix → green. Output recorded as evidence (acceptance criterion 4 + verifier question). |
| 7 | ASan probe build flags? | `g++ -std=c++20 -DNDEBUG -DPARALLEL -fsanitize=address -O1` — `-DNDEBUG` deliberate so `better_assert` is silent and the real OOB is observable (project contract §4; R-06). |
| 8 | Where do probes/tests live? | Probes: `.work/probes/E01_E02.cc` (single file; the contract's deterministic check compiles exactly this path). Tests: `tests/cases/shrink_to_size.hpp`, `tests/cases/flip.hpp`, registered in `tests/test.cc`. |
| 9 | What does "registered live" mean for E01/E02? | Per `docs/eval_seed_cases.md` the ladder is seeded → live → promoted (live + permanent home in `tests/cases/`). Both seeds will have a permanent home (same acceptance scenarios in the new test cases) → final status **promoted** (subsumes "live"). Logged as a P9 clarification. |
| 10 | Branching / human gate? | Work on the existing session branch `phase-1/session-1`; baseline commit `83ea78d` is the diff-audit reference. Human decision gate (high risk): final message presents diff + evidence; no merge before sign-off. |

Confidence: **>95%** — all decision points are fixed by the contract set; pre-flight probes already
reproduced both findings (evidence: `.work/evidence/prefix_*.out/err`). No open question blocks
implementation.

## Context exploration (budget-conform)

- `matrix.hpp` regions read: 3500–3560 (`crtp_shrink_to_size`), 4440–4500 (`flipdim`/aliases), 3784–3840
  (constructors, for probe/test authoring), 1925–1943 (`col_begin`/`col_end`/`row_begin` semantics).
- `tests/test.cc` + `tests/cases/ones.hpp`, `inverse.hpp` (registration + assertion style).
- `docs/opencode_sharded_review.md` §C1, §C2 only.
- **Not read:** rest of `matrix.hpp`, ReadMe, deep-research docs, examples (budget map).

## Design decisions (validated)

1. **Fix = the review's smallest-safe-fix, verbatim.** One token each. No surrounding logic moves
   (the surrounding invariants — zero-fill of `other`, `min`-based copy extents, `swap` — are already
   correct; that is why the one-line fixes are safe).
2. **Probes before code.** Pre-flight probes reproduce both findings on the pre-fix tree (done; see
   `.work/evidence/`). A finding that did not reproduce would stop the session (failure arbiter).
3. **Tests are content-exact** (integer-valued doubles, tolerance 1e-12), mirroring the eval-seed
   expectations, plus extra adversarial shapes from the contract's `adversarial_cases`.
4. **`flip.hpp` pins dim==1 too**, so an accidental edit of the untouched branch (named failure mode)
   is caught by the suite, not just by review.
5. **No doc deltas** expected (both fixes restore documented behavior; P4 — ReadMe untouched here).

## Approaches considered

- **A1 (chosen): one-line fixes + content tests + ASan probes.** Minimal blast radius, maximum
  evidence per line changed. Matches the contract's smallest-safe-fix mandate.
- **A2: rewrite `shrink_to_size` with `std::copy_n`/`span` idioms.** Cleaner, but widens the diff
  beyond the sanctioned 2 lines and re-touches invariants that are already correct → rejected
  (contract invariant: "diff limited to the two buggy lines + tests").
- **A3: make `flipdim` a generic axis-permutation (handles N-D).** New feature territory, out of
  scope, and the CRTP/matrix model is 2-D → rejected (PRD §4 "no new features").
