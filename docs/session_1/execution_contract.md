# Session 1 — Execution Contract

Produced before implementation per the session lifecycle. Authority chain: this document
operationalizes `docs/session_1_contract.yaml`; it may narrow, never widen.

## Planned file changes

| File | Change | Why |
|---|---|---|
| `matrix.hpp` | 2 lines only: `:3532` (`the_rows_to_copy`→`the_cols_to_copy`), `:4479` (third arg → `ans.col_begin( index_right )`) | C1, C2 sanctioned fixes (PRD §5 rows 1–2) |
| `tests/cases/shrink_to_size.hpp` | new | C1 regression, content-asserting (P8) |
| `tests/cases/flip.hpp` | new | C2 regression + dim==1 pin (P8) |
| `tests/test.cc` | +2 include lines | registration |
| `.work/probes/E01_E02.cc` | new | E01/E02 ASan probes (already written pre-flight) |
| `.work/evidence/*` | new (logs) | deterministic evidence (pre/post fix, bug-restoration, independent probe) |
| `.work/handoff_session_1.md` | new | handoff requirement |
| `.work/independent/*` | new | branch_and_compare independent test-writer output |
| `docs/session_1/**` | new (phase docs incl. this file, review + verifier records) | session plan refining phases + exit criteria |
| `docs/eval_seed_cases.md` | E01/E02 status `seeded`→`promoted` | eval-seed registration |
| `docs/risk_register.md` | **no change expected** (rows owned jointly with S2/S5 stay open) | minimize diff; nothing S1-only closes |

## Allowed blast radius (per contract `blast_radius.allowed_files`)

`matrix.hpp`, `tests/test.cc`, `tests/cases/shrink_to_size.hpp`, `tests/cases/flip.hpp`,
`.work/`, `docs/eval_seed_cases.md`, `docs/risk_register.md`, `docs/session_1/**`.
Forbidden: `ReadMe.md`, `Makefile`, `examples/**`, `docs/prd.md`, `docs/project_contract.md`.
Diff audit vs baseline `83ea78d` must leave no file outside the allowed set.

## First test to write

`.work/probes/E01_E02.cc` (pre-flight, **before** any code edit) — probe-first rule (P5).
Pre-fix it fails/reproduces (e01a ASan OOB, e01b corruption, e02a ASan OOB, e02b corruption;
e02c passes). Then the first spec-derived failing suite test is
`tests/cases/shrink_to_size.hpp` (written while the C1 bug is still present).

## Checks after each task

| Task | Checks |
|---|---|
| 1 pre-flight | baseline `make test` green (log); pre-fix probe runs reproduce all findings (logs) |
| 2 C1 | `make test` green; `.work/probe_s1 e01` → `PASS E01` exit 0, no ASan report |
| 3 C2 | `make test` green; `.work/probe_s1 e02` → `PASS E02` exit 0; `git diff 83ea78d -- matrix.hpp` = exactly the 2 sanctioned lines |
| 4 compare | independent probe compiles + passes (ASan flags); bug-restoration logs show each new case FAIL with its bug line restored, PASS after restore |
| 5 full | full `make test` (log); `.work/probe_s1` (no args) → `PASS E01` + `PASS E02` exit 0; diff-audit grep empty; `grep -n the_cols_to_copy matrix.hpp` fix line present |
| 6 review | 6-axis sharded review; High/Critical fixes re-run Task 5 checks |
| 7 verify | adversarial verifier PASS (fresh context) |
| 8 close | re-run contract `deterministic_checks`; handoff complete; final commit |

## Review axes (end of session)

correctness, readability, security, tests, architecture, performance (contract `review_axes`),
all read-only, structured findings (severity / evidence / clause / smallest safe fix /
confidence). Fix High/Critical only; Medium requires 2+ reviewers or strong evidence.

## Adversarial verifier brief

Fresh-context verifier(s) receive ONLY: `docs/session_1_contract.yaml`,
`docs/project_contract.md` (relevant §), `git diff 83ea78d..HEAD`, and the check/evidence
outputs (pre-fix probe logs, final `make test` log, ASan probe output, bug-restoration logs,
diff-audit output). NOT the implementation conversation. Mission: falsify the done condition.
Specific targets:
1. Do the new tests actually fail if either original bug line is restored? (evidence:
   bug-restoration logs — verifier may re-derive this reasoning from the asserts' expected
   values vs the buggy output.)
2. Are all 4 acceptance criteria met with cited output?
3. Blast radius: diff ⊆ allowed files; `fliplr`/`flipud` (4491–4499) and dim==1 branch untouched.
4. Invariants: full suite green; grow zero-pads / shrink truncates; no signature change.
5. Edge cases: non-square both directions, 1×N / N×1, grow-only / shrink-only extremes.

## Done condition

All contract `acceptance_criteria` pass with cited output:
1. E01 semantics hold (5×5→5×3 preserves first 3 cols; 1×1→4×4 zero-pads) — probe + test evidence.
2. E02 semantics hold (3×5 dim2 = hand-written left-right flip; dim1 = up-down flip) — probe + test evidence.
3. ASan probes (5×5→5×3, 3×10→5×2, 3×5 flipdim) run with `-DNDEBUG -fsanitize=address`: no report, exit 0.
4. New tests fail if either original bug line is restored — bug-restoration logs.
Plus: `make test` full green; git audit clean; sharded review + adversarial verifier PASS;
branch_and_compare independent test-writer concurs; handoff complete; **human decision gate**
sign-off before merge.

## Failure policy

Any check failure is classified per `docs/prompts/failure_arbiter.md` (BUG / SPEC_GAP /
AMBIGUITY / ENVIRONMENT / TEST_BUG) **before** any fix; classification + evidence recorded in
`docs/session_1/failure_arbiter.md` (only if invoked).
