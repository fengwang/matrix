# Session Handoff

## State Snapshot
- Session: S1 — C1 (`shrink_to_size` wrong copy extent) + C2 (`flipdim` dim==2 column-vs-row swap)
- Branch: `phase-1/session-1`
- Last commit: `b356840` (tasks 4–5) + closeout commit (this handoff, seed/risk updates, review records)
- Baseline (pre-session): `83ea78d`. Pre-flight checkpoint: `d34ffef`. Fix commits: `8a4323e` (C1), `7b784fb` (C2).
- Changed files (vs `83ea78d`; audit-verified ⊆ allowed set):
  - `matrix.hpp` — exactly 2 lines: 3532 `the_rows_to_copy`→`the_cols_to_copy`; 4479 `row_begin(index_right)`→`col_begin(index_right)`
  - `tests/test.cc` — +2 includes
  - `tests/cases/shrink_to_size.hpp` (new, 9 assertions blocks), `tests/cases/flip.hpp` (new)
  - `docs/eval_seed_cases.md` (E01/E02 `seeded`→`promoted`), `docs/risk_register.md` (S1 watch items)
  - `docs/session_1/**` (13 records: specs, design, plan, tasks, execution contract, sharded_review, adversarial_verification, failure_arbiter)
  - `.work/**` (probe `E01_E02.cc`, independent probe + derivation, 25 evidence logs; committed via `git add -f`; binaries not committed)
- Checks run:
  1. Baseline `make test` + `./test_test` at `83ea78d`: 57 cases green (49,216,592 assertions)
  2. Pre-fix ASan probe (exact flags `-std=c++20 -DNDEBUG -DPARALLEL -fsanitize=address -O1`): e01a heap-OOB WRITE (5×5→5×3), e01b content corruption (3×10→5×2), e02a heap-OOB READ (3×5 flipdim2), e02b scramble (4×4), e02c PASS (dim==1 pin)
  3. Pre-fix suite (TDD red): shrink case segfault, flip case abort
  4. Post-fix `make test` + `./test_test`: **59 cases green, 49,216,776 assertions, exit 0**
  5. Post-fix full ASan probe: e01a–c + e02a–c PASS, `PASS E01`, `PASS E02`, exit 0, no ASan report
  6. Independent fresh-context probe (derivation from contract-only inputs; runId `wf_msxq6npb-4-94d3a0fc21f7`): 8/8 PASS, ASan clean
  7. Bug restoration (empirical, temp states never committed): C1 line restored → shrink case FAILS (segfault); C2 line restored → flip case FAILS (2 asserts + abort); fixes restored → green
  8. Diff audit vs `83ea78d` (all changed paths ⊆ allowed set; matrix.hpp = exactly the 2 sanctioned lines) + grep audit (fixed line at 3532)
  9. Sharded review, 6 axes (runId `wf_msxqbh35-5-b659e2f3c550`): 0 Critical/High; 3 Low + 3 Nit, all dispositioned (S3-route / convention / out-of-scope / rejected-by-contract)
  10. Adversarial verification, 2 fresh-context lenses (runId `wf_msxqiff5-6-2930c7a81316`): both VERDICT PASS, 0 disproven / 0 unsupported claims; verifiers independently re-ran suite, filtered cases, ASan probe rebuild, diff + caller audits
- Checks not run:
  - `make example` — examples out of scope and unchanged; verifier grep confirmed **no in-repo caller** of `shrink_to_size`/`flipdim` outside the new case files, so example behavior cannot shift
  - Seeds E03–E18 — owned by S2–S6; red-expected pre-owner per usage rules (E01/E02 are the S1 subset and are green)
  - Fuzzing / property testing — not in session scope
- Current status: **done condition met; awaiting human decision gate (no merge before sign-off)**

## Narrative Context
Session 1 eliminated the two Critical memory-corruption findings in the matrix library's documented
behaviors. C1: `crtp_shrink_to_size` copied `the_rows_to_copy` columns per row instead of
`the_cols_to_copy`, writing past row ends (heap OOB on shrink) and mis-reading the source layout
(silent corruption when rows≠cols). C2: `flipdim(m,2)` swapped column *left* against *row*
*right* (`row_begin` as the `swap_ranges` third argument) — an out-of-bounds, order-destroying
"flip". Both were fixed with the review-sanctioned one-line changes, then pinned with
content-asserting Catch2 cases (ragged values so no shape-only pass) and ASan probes E01/E02.
Empirical proof of regression power came from restoring each original bug line and watching the
corresponding new case fail; a fresh-context independent re-derivation of all expected contents
concurred; a 6-axis sharded review and two-lens adversarial verification found nothing
Critical/High and no falsifiable claim. The `fliplr`/`flipud` alias inversion (C3) was left
untouched per contract — S3 owns it, and S1's fix is what makes S3's swap correct.

## Decision Log
| Decision | Chosen | Rejected | Reason | Contract Ref |
|---|---|---|---|---|
| Fix shape | Review-sanctioned one-line fixes (copy extent; swap third arg) | Defensive rewrite of `crtp_shrink_to_size` / `flipdim` | Invariant: "diff limited to the two buggy lines + tests"; smallest-safe-fix | `session_1_contract.yaml` invariants |
| Test file name | `tests/cases/flip.hpp` | `flipdim.hpp` (reviewer Nit) | Contract `allowed_files` names `flip.hpp` exactly | `blast_radius.allowed_files` |
| Unused `<cstdlib>` in new tests | Kept | Delete (reviewer Low) | Sibling case files all carry an unused `<cassert>`; convention; not High/Critical | local convention |
| Independent test-writer | Small fresh-context reasoning-only unit (pasted inputs) + orchestrator-encoded probe | Large file-writing subagent (failed ×2) | ENVIRONMENT failure (16K thinking budget) per failure_arbiter record 1; adaptation logged | AGENTS.md subagent rules; `failure_arbiter.md` |
| Eval seed status | `promoted` | `live` | Probe exists + passes + permanent home in `tests/cases/` (subsumes `live`; P9 clarification) | `eval_seed_cases.md` status legend |
| C3 alias swap | Not touched | "Fix while we're here" | Out of scope; S3 owns; S1→S3 hard chain (R-15) | `out_of_scope`, PRD §6 |
| Zero-size `flipdim` pattern | Documented as watch item, not fixed | Add guard at `flipdim` top | Pre-existing, symmetric with untouched dim==1 branch; outside 2-line diff | `blast_radius` |

## Next Priority Queue
1. **S2** — `load_npy`/`save_npy` (seeds E03/E04; also the two extra `load_npy` hazards noted in R-02)
2. **S3** — C3 alias bodies (`fliplr`/`flipud`) swap + pinv/det/pow/LU-pivoting (seeds E05–E09); C3 re-confirmed by S1 review
3. S4, S5, then **S6** (last; ReadMe single-writer, seed E16–E18, zero-size policy if ever adopted)

## Warnings And Gotchas
- Environment issues:
  - Subagents on this host: single model (Qwen3.8-27B, reasoning, xhigh) exhausts the 16K output
    budget on the thinking channel for moderate prompts → empty returns. Keep units small
    (pasted-only inputs, no file writes, short structured outputs); probe trivial capability
    first. (Also: `/tmp` is per-agent sandboxed — exchange via `.work/`, never `/tmp`.)
  - `make test` only **builds** `test_test`; run `./test_test` (optionally a case name) to execute.
- Known failing tests: none (suite 59/59 green).
- Deferred risks: C3 alias inversion (S3); pre-existing 0-size `dim()-1` pattern in `flipdim`
  (watch item, needs 0-size support decision first); `hardware_concurrency()==0` unreachable
  on this host (pre-existing, unrelated).
- Files future sessions must not casually edit: `fliplr`/`flipud` alias region (~`matrix.hpp`
  4491–4499, S3 only); `ReadMe.md` (S6 only); `Makefile` (none); `examples/**` (none);
  `docs/prd.md` / `docs/project_contract.md` (dominant, change only via contract process).
- `.work/` is gitignored: session evidence/probes are committed with `git add -f` (selected
  files; compiled binaries are not committed).

## Eval Seeds
- Missed check: none — no bug discovered beyond E01–E18 coverage.
- New regression test candidate: promoted in place — `tests/cases/shrink_to_size.hpp`,
  `tests/cases/flip.hpp` (E01/E02 marked `promoted` in `docs/eval_seed_cases.md`).
- Instruction update candidate: subagent small-unit discipline recorded in
  `docs/risk_register.md` (S1 closeout watch items) + `docs/session_1/failure_arbiter.md`.
