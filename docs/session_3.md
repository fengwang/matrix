# Session 3 — Numerical Semantics: flips, `pinv`, `det`, `operator^` (C3, C4, C5, C6, R1, P2-LU)

> Story outline v1. Refine at start (narrow/clarify only), policy P9.
> Machine-readable authority: `docs/session_3_contract.yaml`. Law: `docs/project_contract.md`.
> **Depends on S1** (`flipdim` must be fixed before the alias swap is meaningful). Runs on the chain S1→S3→S6.

## Objective

Fix the library's documented-convention violations in the numerical core: swap the `fliplr`/`flipud` aliases (C3), make the pseudoinverse actually invert singular values (C4), replace the Schur-complement `det` with a pivoted-LU determinant that returns `0` (not `NaN`) on singular input (C5+P7), make `operator^` compile and compute for odd exponents (C6), fix the `svd_inverse` argument-order trap (R1), and give `lu_decomposition` partial pivoting (P2) — each with content-asserting tests.

## Story

The review's third stage. Five findings, one theme: *the public numerics do not honor the library's documented contract* (MATLAB/NumPy conventions, ReadMe §det). The alias swap (C3) is two lines but changes observable behavior for `fliplr`/`flipud` users — sanctioned (PRD §5 row 3). The pseudoinverse (C4) gains its missing `Σ⁺` via the already-correct SVD-inversion path; `svd_inverse`'s swapped argument order (R1) is fixed in the same breath because it is the direct cause of C4 and the comprehension trap. `det` (C5) drops the Schur-complement recursion with its unguarded `P.inverse()` for the library's own LU: `det = ±∏U_ii`, exact zero pivot ⇒ `0` (policy P7); this needs `lu_decomposition` to pivot (P2), whose factor values change (sanctioned row 7) while solutions stay invariant (E09 pins that). `operator^` (C6) is the precedence bug in the odd branch. The session is *medium* risk: new logic + sanctioned API behavior changes, no memory-safety or security surface, and `examples/` (0005/0019/0021) exercises the changed numerics — so `make example` is a required check.

## In scope

- C3: `fliplr`/`flipud` (anchors 4491/4496) → `flipdim(m,2)` / `flipdim(m,1)`.
- C4: `pinverse` body (anchor 5226) → single correct SVD-inversion core (1e-10 threshold, inherited from `svd_inverse`).
- R1: `svd_inverse` (anchor 5216) — call `singular_value_decomposition(a, u, w, v)` matching the signature (anchor 4921); local names follow the signature.
- C5+P2: `crtp_det` (anchor 2048) rewritten as pivoted-LU product; `lu_decomposition` (anchors 6499/6532) gains partial pivoting (max-magnitude row swap; permutation sign returned/accumulated for det).
- C6: `operator^` odd branch (anchor 5566) → `half = lhs^(n>>1); return half*half*lhs;`.
- R3-slice: the `det` precondition message typo (anchor ~2056, "the row and matrix…") fixed **here** (function under rewrite).
- Tests: new `tests/cases/flip_aliases.hpp`, `pinv.hpp`, `det.hpp`, `matrix_power.hpp`, `lu_pivoting.hpp`; registered in `tests/test.cc`.
- Eval probes E05–E09.

## Out of scope

- Retiring `pinverse`/`svd_inverse`/free `det(m)` **names** — that is S6 (A2); S3 fixes behavior only, all names still compile.
- The `flipdim` bodies (S1's; only verify they hold via E02, do not re-edit).
- `cholesky_decomposition` guard (S4); `svd` public behavior beyond the inversion core; `examples/` value expectations (print-only — no edits; S6's docs sweep notes the changed printed outputs); `ReadMe.md` (delta emitted, P4).
- `backward_substitution`/`forward_substitution` logic beyond what pivoting requires.

## Deliveries

1. Five fixes in `matrix.hpp` (flip aliases; pinv core; det rewrite; `operator^`; LU pivoting).
2. Five new test cases + registration.
3. E05–E09 probes live.
4. Handoff with doc deltas for S6 (ReadMe: flip convention, `pinv` semantics + threshold, `det` zero-pivot rule, `lu_decomposition` pivoting note, `^` domain restored; examples' printed L/U/det values noted as changed).

## Context budget map (~55K of 128K — do not exceed)

| Read | How much | Why |
|---|---|---|
| `AGENTS.md`, `docs/project_contract.md` §3–§5 | ~2.5K | law |
| `docs/prd.md` §5 rows 3–7, §7 P1/P5/P7/P8 | ~2K | authorization + policies |
| `docs/opencode_sharded_review.md` §C3 §C4 §C5 §C6 §R1 §P2 | ~4K | findings + smallest fixes |
| `matrix.hpp` regions: 2040–2090 (crtp_det), 4440–4500 (flip block), 4915–4935 (SVD signature), 5195–5240 (svd_inverse/pinverse/pinv), 5550–5580 (operator^), 6360–6395 (forward_substitution context), 6490–6560 (lu_decomposition/lu_solver) | ~15K | the code, named regions only |
| `ReadMe.md` §det (765–780) only | ~1K | the documented det contract (C-07) |
| `tests/test.cc` + 2 existing cases | ~2K | patterns |
| `examples/cases/0005_det.hpp`, `0019_lu_decomposition.hpp` | ~2K | confirm print-only (no assertion updates needed) |
| `docs/eval_seed_cases.md` E05–E09 | ~1K | probe specs |
| **Do NOT read:** rest of `matrix.hpp`, ReadMe in full, research docs in full | — | budget |

## Deep-research references

- Report 6 §"Proposed semantic model and API blueprint" (line 152): the NumPy semantics the flips/pinv follow (section only — it is the convention authority for this session).
- P3500R0 §"Interoperability with std::mdspan and std::linalg" (line 274): context for determinant/inversion semantics in a linalg-adjacent API. Section only.

## Pre-flight (mandatory)

1. `git status` clean; confirm S1 is merged (E02 green) — if not, stop (chain dependency).
2. Re-anchor all six regions by grep; log any drift.
3. Probe-first (P5): run pre-fix probes for E05–E08 — record that C3/C4/C5 misbehave and **C6 fails to compile** (compile the `m^3` probe separately so it cannot mask the others).
4. Read `ReadMe.md` §det — the `0`-on-singular contract (C-07) must be the stated contract, not an assumption.

## Exit criteria

1. `make test` green (suite + 5 new cases); `make example` green (compile check; printed values may differ — expected, rows 5/7).
2. E05–E09 pass with cited output (E07: singular det == 0 exactly; E09: `‖x_before − x_after‖∞ < 1e-9`).
3. `git diff --name-only HEAD` ⊆ {`matrix.hpp`, `tests/test.cc`, `tests/cases/{flip_aliases,pinv,det,matrix_power,lu_pivoting}.hpp`, `.work/**`, `docs/eval_seed_cases.md`, `docs/risk_register.md`}.
4. Sharded review + adversarial verifier PASS (verifier focus: det on singular *and* near-singular inputs; LU pivoting sign bookkeeping for odd permutation counts; `^` for n=0..5; pinv on rank-deficient rectangular input).
5. Doc deltas written for S6 (exact wording per finding).

## Risk and routing

- Risk level **medium** (new logic + sanctioned API behavior changes; no memory-safety/security surface). Routing: **worker_plus_reviewers** — worker + sharded review (6 axes) + adversarial verifier.
- Failure modes to watch: permutation **sign** error in pivoted det (test 3×3 needing an odd number of swaps); pivoting changing `lu_solver`'s solution (E09 must hold); `pinverse` and `pinv` diverging again (they must share the single core); det epsilon creeping in (P7: exact zero only); editing `flipdim` bodies (S1 territory).

## Handoff requirements

State snapshot (compiler, checks run/not run, note on changed example outputs); decision log (pre-fix probe outputs; convention choice C3 documented with the NumPy citation; P7 rule restated; contract refinements); eval seeds E05–E09 live; doc deltas for S6 (complete list with target ReadMe sections); warning for S6: names `pinverse`/`svd_inverse`/free `det` are now behavior-fixed and ready for retirement — do not re-implement anything.

## Contract

`docs/session_3_contract.yaml` — read before pre-flight; authority for scope, invariants, checks.
