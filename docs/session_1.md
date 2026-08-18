# Session 1 — Memory Corruption: `shrink_to_size` and `flipdim` (C1, C2)

> Story outline v1. This session refines its own details at start (narrow/clarify only), per policy P9.
> Machine-readable authority: `docs/session_1_contract.yaml`. Law: `docs/project_contract.md`.

## Objective

Eliminate the two Critical heap-corruption bugs — C1 (`shrink_to_size` copies the wrong column count) and C2 (`flipdim(m,2)` swaps a column against a row) — with the review's smallest-safe-fixes, and pin both with content-asserting regression tests and ASan probes.

## Story

The review's first fix stage. Two one-line fixes close the only findings that can corrupt the heap under ordinary use: a wrong length in a `std::copy` and a wrong third argument to `std::swap_ranges`. Both are confirmed present in the current header (anchors below). The fixes are trivial *because the surrounding invariants are already correct* — the session's real work is proving it: re-verify each finding with an ASan probe (the review's method: `-DNDEBUG` so `better_assert` is silent and the real OOB is observable), apply the minimal fix, and add tests that assert **content**, not just shape, so T1's "green suite over untested paths" pattern cannot recur on these functions. Nothing else moves: no aliases, no tests beyond the two functions, no docs (doc deltas: none expected — both fixes restore *documented* behavior).

## In scope

- C1: `crtp_shrink_to_size` (anchor ~3508; buggy `std::copy` at ~3531–3532) — copy `the_cols_to_copy` per row.
- C2: `flipdim` dim==2 branch (anchor ~4453; buggy `swap_ranges` at ~4479) — third argument `ans.col_begin( index_right )`.
- Regression tests: new `tests/cases/shrink_to_size.hpp`, new `tests/cases/flip.hpp`, registered in `tests/test.cc`.
- Eval seeds E01, E02 written as probes and marked live.

## Out of scope

- C3 (`fliplr`/`flipud` alias swap) — depends on C2 landing first; it is S3's. Do **not** touch 4491–4499.
- All other findings; any refactor (A1 CRTP stays as-is); any `ReadMe.md` edit (P4); `examples/`; `Makefile`.
- Changing `shrink_to_size`/`flipdim` signatures or semantics beyond the documented contract.

## Deliveries

1. Two fixes in `matrix.hpp` (diff ≈ 2 lines, plus nothing else).
2. Two new test cases + registration (content assertions, shapes incl. non-square both directions).
3. E01/E02 probes in `.work/probes/`, both passing, ASan-clean.
4. Handoff `.work/handoff_session_1.md`; eval seeds registered; `git` audit clean.

## Context budget map (~45K of 128K — do not exceed; the rest is work)

| Read | How much | Why |
|---|---|---|
| `AGENTS.md`, `docs/project_contract.md` | full (~2K) | law |
| `docs/prd.md` §5 rows 1–2, §7 P8 | ~1K | authorization + test policy |
| `docs/opencode_sharded_review.md` §C1, §C2 only | ~2K | the findings' evidence + smallest fixes |
| `matrix.hpp` regions: 3500–3560 (shrink), 4440–4500 (flip) | ~5K | the code (named regions, never the whole file) |
| `tests/test.cc` + one existing case (e.g. `inverse.hpp`) | ~2K | registration pattern + assertion style |
| `docs/eval_seed_cases.md` E01–E02 rows | ~0.5K | probe specs |
| **Do NOT read:** rest of `matrix.hpp`, ReadMe, deep-research docs in full, examples | — | budget |

## Deep-research references

- Report 6 `docs/deep_research/deep-research-report (6).md` §"Layout, performance, execution, safety and correctness" (line 511): the safety/correctness framing this session implements (boundary + invariant discipline). Read ~the section only.
- P3500R0 `docs/deep_research/C++ Standard Tensor Proposal Blueprint.md` §"Storage Architecture…" (line 136): context for why buffer-length validation at resize boundaries is standard practice. Read ~the section only.

## Pre-flight (mandatory, before any edit)

1. `git status` clean; baseline commit noted in handoff.
2. Re-anchor: `grep -n "the_cols_to_copy\|swap_ranges" matrix.hpp` — confirm the review's bug lines are still the bug lines (else re-anchor by function name and note it).
3. Write E01/E02 probes against the **current** code; compile with the ASan variant; confirm the review's reproductions (OOB report for 5×5→5×3; corruption/OOB for 3×10→5×2 and 3×5 flip). If a probe does *not* reproduce the finding: classify (failure_arbiter) and stop — do not fix an unverified finding.
4. Record pre-fix probe output in the handoff (evidence base for the diff).

## Exit criteria

1. `make test` green (full suite + the two new cases).
2. E01/E02 pass; ASan variant clean (no report, exit 0).
3. `git diff --name-only HEAD` ⊆ {`matrix.hpp`, `tests/cases/shrink_to_size.hpp`, `tests/cases/flip.hpp`, `tests/test.cc`, `.work/**`}.
4. Sharded review (6 axes) + adversarial verifier PASS (verifier specifically: "would the new tests fail if either bug were present?").
5. **Human decision gate (high risk):** user reviews diff + evidence before merge.

## Risk and routing

- Risk level **high** (memory corruption domain). Routing: **branch_and_compare** — worker implements on a branch; an independent test-writer pass re-derives the expected contents from the *documented* contract (not from the implementation); sharded review; adversarial verifier; human gate.
- Failure modes to watch: fixing the copy count but leaving a row-offset error; tests asserting shape-only; ASan probe accidentally compiled without `-DNDEBUG` (then `better_assert` aborts and masks the real behavior); accidental edit of the `flipdim` dim==1 branch.

## Handoff requirements

State snapshot (compiler version, commits, checks run/not run); decision log (contract refinements, probe results vs review); eval seeds E01/E02 status; doc deltas (expected: **none** — behavior now matches the documented contract); warnings for S3 (the dim==1 branch and `fliplr`/`flipud` are adjacent — S3 must not assume they were touched).

## Contract

`docs/session_1_contract.yaml` — read it before pre-flight; it is the authority for scope, invariants, and checks.
