# Session 4 — Semantics Batch: integer stats, `conv`/`rref` preconditions, Cholesky guard (C8, C9, C10, P2-Cholesky)

> Story outline v1. Refine at start (narrow/clarify only), policy P9.
> Machine-readable authority: `docs/session_4_contract.yaml`. Law: `docs/project_contract.md`.
> Independent of the S1→S3→S6 chain.

## Objective

Fix the four medium semantics findings: promote `mean`/`variance`/`standard_deviation` so integer matrices stop truncating (C8), correct the `conv` "same"-mode asserts so a valid 1×1 kernel is accepted (C9), relax the `rref`/`gauss_jordan_elimination` precondition so square systems work (C10), and give `cholesky_decomposition` a positive-definiteness guard with an honest `bool` return (P2). Each ships a content-asserting test, including the debug-build abort paths.

## Story

The review's stage-4 "numerics half" (the robustness half is S5). Four self-contained findings in four different regions — deliberately one session because each is a one-function change with a small test, and the session stays far under budget. **C8** is the only return-type change here: `mean`/`variance`/`standard_deviation` return `double` uniformly (sanctioned row 8); the `n−1` sample-variance formula in `standard_deviation` is **kept** (we promote types, we don't redefine statistics — E10 pins √0.5, not 0.5). **C9** is the copy-pasted assert: both asserts check `rb`, the message says "at least 1" but the condition `> 1` rejects the well-defined 1×1 kernel; fix the condition to `rb >= 1 && cb >= 1` (the slicing below already handles it: `(rb-1)>>1 == 0`). **C10** is an over-restrictive precondition (`row < col`) on an algorithm that is fully defined for square systems — debug builds abort, release builds work; relax to `row > 0 && col > 0`, keeping the existing 1e-10 pivot early-exit as the singularity signal. **P2-Cholesky**: `cholesky_decomposition` is `void` and silently `sqrt`s negatives; it becomes `bool` (false when a diagonal candidate is negative or a divisor is zero). Verified fact: it has **zero in-repo callers** (evidence map), so the signature change is blast-radius-free.

## In scope

- C8: `mean`/`variance`/`standard_deviation` (anchors 7640/7646/7652) — divisor/promotion to `double`; the `size<=1` branch of `standard_deviation` returns `double{}`; `n−1` formula untouched.
- C9: `conv` "same" asserts (anchor ~6618–6621) — second assert checks `cb`; condition `rb >= 1 && cb >= 1`.
- C10: `gauss_jordan_elimination` precondition (anchor 6396) — `row > 0 && col > 0`; `rref` (anchor 6427) inherits (it delegates).
- P2b: `cholesky_decomposition` (anchor 5676) — `void → bool`; guard: diagonal candidate `sum < 0` ⇒ `false`; divisor `a[i][i] == 0` (non-diagonal step) ⇒ `false`; `true` on success.
- Tests: update `tests/cases/mean.hpp` (integer cases; if it asserts truncated ints, update **as part of** the C8 fix — watch item R-15); new `tests/cases/conv_same.hpp`, `rref.hpp`, `cholesky.hpp`.
- Eval probes E10–E13.

## Out of scope

- `sum`'s own accumulator (int overflow in `sum` for large int matrices is pre-existing and **not** a C8 row — stays `value_type`-accumulated; noted in handoff watch items).
- Population-vs-sample statistics (formula kept); `conv` "full"/"valid" paths (verified correct by the review — do not touch); `lu_decomposition` (S3's); `ReadMe.md` (delta emitted, P4); `examples/`.
- Changing `rref`'s return type (`std::optional<Mat>` stays).

## Deliveries

1. Four fixes in `matrix.hpp` (four small regions).
2. Test updates/new cases: `mean.hpp` (int promotion), `conv_same.hpp` (1×1 kernel + content), `rref.hpp` (square system, debug build), `cholesky.hpp` (PD true / non-PD false).
3. E10–E13 probes live.
4. Handoff with doc deltas for S6 (ReadMe: statistics return `double`, `conv` "same" kernel rules, `rref` domain, `cholesky_decomposition` signature + failure semantics).

## Context budget map (~50K of 128K — do not exceed)

| Read | How much | Why |
|---|---|---|
| `AGENTS.md`, `docs/project_contract.md` §3–§5 | ~2.5K | law |
| `docs/prd.md` §5 rows 8–11, §7 P8 | ~1.5K | authorization |
| `docs/opencode_sharded_review.md` §C8 §C9 §C10 §P2 | ~3K | findings + fixes |
| `matrix.hpp` regions: 5676–5700 (cholesky), 6393–6430 (gauss_jordan/rref), 6573–6645 (conv both overloads + "same"/"valid" slicing), 7630–7660 (sum/mean/variance/std) | ~10K | the code |
| `tests/cases/mean.hpp` full + `tests/test.cc` + one case for style | ~4K | the int-assertion question (R-15) + patterns |
| `docs/eval_seed_cases.md` E10–E13 | ~1K | probe specs |
| **Do NOT read:** rest of `matrix.hpp`, ReadMe, research docs in full | — | budget |

## Deep-research references

- Report 6 §"Proposed semantic model and API blueprint" (line 152): the semantics model this batch aligns with (statistics/convolution conventions). Section only.

## Pre-flight (mandatory)

1. `git status` clean; baseline commit.
2. Re-anchor the four regions by grep.
3. Probe-first (P5), in a **debug** build (asserts live): `conv(A, kernel{1,1}, "same")` must currently **abort** (C9); `rref(square)` must currently **abort** (C10). Record both. (C8/C10… i.e. C8 and the cholesky NaN are probeable without debug: `mean({1,2;1,2})==1`, `cholesky` on non-PD prints NaN.)
4. Read `tests/cases/mean.hpp` and resolve the R-15 question (does it assert integer results?) **before** writing the fix.

## Exit criteria

1. `make test` green — including the updated `mean.hpp` int cases and the 3 new cases; the new `rref`/`conv_same` cases are compiled with asserts live (debug configuration of the suite).
2. E10–E13 pass (E10: `1.5 / 0.25 / 0.70711…`; E12: square `rref` accepted; E13: non-PD ⇒ `false`).
3. `git diff --name-only HEAD` ⊆ {`matrix.hpp`, `tests/test.cc`, `tests/cases/{mean,conv_same,rref,cholesky}.hpp`, `.work/**`, `docs/eval_seed_cases.md`, `docs/risk_register.md`}.
4. Sharded review + adversarial verifier PASS (verifier focus: C8 on `matrix<float>` and `matrix<double>` (no regressions), C9 boundary `rb==1`/`cb==1` both, C10 on over-determined systems (row>col — must still work as before), cholesky on a PSD-but-singular matrix (zero diagonal candidate ⇒ `false`, no NaN, no div-by-zero)).

## Risk and routing

- Risk level **medium** (sanctioned behavior changes; one signature change with zero in-repo callers). Routing: **worker_plus_reviewers** — worker + sharded review + adversarial verifier.
- Failure modes to watch: E10's std expectation (√0.5 sample formula — **not** 0.5); `conv` "valid" path touched by mistake (verified correct — leave it); `rref` on over-determined (row>col) systems regressing when the precondition is relaxed; `cholesky` guard using `<= 0` on the *last* diagonal of an exact PD input with a tiny positive (would false-reject — guard is `< 0` on diagonal candidates, `== 0` only on divisors).

## Handoff requirements

State snapshot (compiler, checks run/not run); decision log (debug-abort reproductions; R-15 resolution; contract refinements); eval seeds E10–E13 live; doc deltas for S6 (exact ReadMe wording per finding); watch item for S6: `sum`'s int-overflow note.

## Contract

`docs/session_4_contract.yaml` — read before pre-flight; authority for scope, invariants, checks.
