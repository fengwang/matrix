# Session 4 — Brainstorming (refinement record)

Status: refinement only (policy P9 — narrow/clarify, no scope widening). The problem space was
explored in the 2026-08-17 blueprint interview (PRD §2) and the 2026-07-13 review
(`docs/review-report-2026-07-13.md`, findings C8/C9/C10 + P2-cholesky; the `conv` full/valid
paths were already verified correct in that review). This document records the session-start
interview-me pass (see `interview.md`), the pre-flight evidence, and the design decisions. No
new exploration.

**Process note:** the interview-me pass (Q1–Q6, `interview.md`) resolved two contract-internal
conflicts (C-11 zero-diagonal boundary; C8 invariant shape) purely from the dominant documents.
Pre-flight probes (P5) were run before any edit and are recorded in
`.work/evidence/prefix_p0.log` + the probe TUs in `.work/probes/`.

## Pre-flight evidence (pre-fix, 2026-08-18)

| # | Probe (`.work/probes/`) | Pre-fix result | Evidence |
|---|---|---|---|
| p0 | `S4_p0_values.cc` | `mean(matrix<int>)` → `unsigned long` (value **1**, truncated; unsigned — negative int means would wrap); `mean/variance/std(matrix<float>)` → `float` (1.5/0.25/0.707107); same for `double`; `cholesky([[1,2],[2,1]])` → `a[1][1] = -nan` (void); `cholesky([[4,2],[2,3]])` → valid `[[2,0],[1,1.41421]]` | `prefix_p0.log` |
| p0 run 1 | (same TU, int variance) | **compile error** at `matrix.hpp:7783`: `no match for 'operator-' (matrix<int>, unsigned long)` | `prefix_p0.log` |
| p0b | `S4_p0b_stddev_int.cc` | **compile error** at `matrix.hpp:7791`, same root cause | `prefix_p0.log` |
| p0c | `S4_p0c_variance_int.cc` | **compile error** (isolated variance red) | kept for the TDD red record |
| p1 | `S4_p1_conv_abort.cc` | SIGABRT(134), `[Assertion Failure]: 'rb > 1' … Line: 6755` — the copy-pasted second assert fires first on a 1×1 kernel | `prefix_p0.log` |
| p2 | `S4_p2_rref_abort.cc` | SIGABRT(134), `[Assertion Failure]: 'row < col && …' … Line: 6486` | `prefix_p0.log` |
| p3 | `S4_p3_wide_asan.cc` | **heap-buffer-overflow READ** in `gauss_jordan_elimination` for `rref(3×2)` under NDEBUG+ASan — pre-existing release-reachable OOB (assert compiled out) | `prefix_p0.log` |

Discrepancies vs the 2026-07-13 review (recorded, same class as S1–S3 findings): the review
claimed pre-fix int `variance` "returns 0.25 → 0" — it does not compile; the review's C8
invariant notation `{1,2;1,2}` + std 0.70711 is only consistent with the 1×2 matrix (E10).

## Decision table (stress-test of residual points)

| # | Question | Resolution (source) |
|---|---|---|
| D-C8-1 | Where does the int promotion happen? | **Inside each statistic, via `m.astype<double>()`** (member template, `matrix.hpp:3976`). Required because `m - mean(m)` on an `int` matrix binds `operator-(matrix<T>, const T&)` which requires the scalar to be **exactly** `T` — a `double` mean would be implicitly truncated to `int`, silently corrupting variance/std even if `mean` itself returned `double`. Promotion first makes the unchanged formula correct. Contract's own wording: "no integer division (divisor/promotion to double)". |
| D-C8-2 | Which value types change? | `int` (and any integral): `unsigned long`/int → `double`; `float`: `float` → `double` (E10: "each is double" for integer, float, double); `double`: `double` → `double`, **copy-free fast path** (no `astype` no-op copy); `complex`: legacy expression preserved verbatim (no in-repo complex callers; `sum<=0` is ill-formed for complex, so the guard cannot be universal). |
| D-C8-3 | Formula? | Unchanged: `mean = Σ/n`; `variance = mean((m−μ)²)` (population, `n`); `std = sqrt(Σ(m−μ)²/(n−1))` (sample, **`n−1` kept** per PRD C-10 — E10's `0.70711 = √0.5`, not `0.5`). `size ≤ 1 → double{}` branch kept, now typed `double`. |
| D-C8-4 | Return-type mechanism | Uniform type-class dispatcher per function: `if constexpr ( complex ) → legacy expression verbatim` (complex `mean` stays `complex`-typed; complex `variance`/`stddev` were already compile errors pre-fix and stay so — `operator-(matrix<complex<T>>, T)` takes the real type, so `m - mean(m)` is ill-formed); `else { if constexpr ( double ) → copy-free fast path; else → astype<double>() promotion path }`. All real paths return `double`. `constexpr` specifier kept (decorative today — the body already calls non-constexpr `reduce`; the template compiles). |
| D-C8-5 | Tests | Extend `tests/cases/mean.hpp`: int 1×2 `{1,2}` (mean 1.5, var 0.25, std `√0.5`), int 2×2 `{1,2;1,2}` (mean 1.5, var 0.25, std `√(1/3)` — kills the shape ambiguity, Q2), int 1×1 `{7}` (std 0.0), float 1×2, double 1×2 (regression), plus `static_assert` of `double` return types. Existing random-double case untouched. |
| D-C9-1 | The exact fix | Second assert's condition `rb > 1` → `cb >= 1`; first assert's condition `rb > 1` → `rb >= 1`. Both messages kept (they are already correct per-axis). A 1×1 kernel now passes: slice offsets `(rb−1)>>1 = 0` and `ra + 0` — the full conv with a 1×1 kernel is scaling, already verified correct by the 2026-07-13 review's "same/valid/full conv verified" scope. |
| D-C9-2 | Tests | New `tests/cases/conv_same.hpp`: (a) 1×1 kernel 2×2 → scaled (E11 content); (b) `rb==1, cb==2`: `A={1,2,3;4,5,6}`, `B={1,1}` → `{{1,3,5},{4,9,11}}` (hand-derived from the full-conv path + NumPy `convolve(...,'same')` cross-check); (c) `rb==2, cb==1`: `A={1,2;3,4;5,6}`, `B={1;1}` → `{{1,2},{4,6},{8,10}}` (same method); (d) "valid" regression: `A` 4×5 (1..20), `B` 2×3 with `B[0][0]=0.5` rest 0 → 3×3 `0.5·A[0:3,0:3]` (pins the valid path untouched). All expected values hand-derived in design.md. |
| D-C10-1 | The exact fix | `better_assert( row < col && "matrix row must be less than colum to execut a Gauss-Jordan Elimination", row, col )` → `better_assert( row > 0 && col > 0 && "matrix must have at least one row and one column to execute a Gauss-Jordan Elimination", row, col )`. Same house style (`cond && "msg"`); the pre-existing message typos ("colum", "execut") are fixed only because the message text is rewritten (S3 D7 precedent). |
| D-C10-2 | `row > col` OOB | **Pre-existing** (p3: ASan heap OOB READ pre-fix, release build). The relaxation does not cause it; it only removes a debug-only guard that happened to mask it in debug builds. The contract sanctions the precondition line only and requires row>col to "behave identically to pre-relaxation" — i.e. **the OOB stays, documented**. Before/after ASan logs compared at closeout. Watch item for a future session (S5/S6 or I/O hardening pass — recommend a dedicated precondition-UB audit). |
| D-C10-3 | Tests | New `tests/cases/rref.hpp`: (a) square 2×2 `diag{2,3}` → `≈ I` (E12 content, no abort); (b) singular square `{{1,2},{2,4}}` → `nullopt` via the 1e-10 pivot exit (finite values only — R-19 safe); (c) wide 2×3 regression (the originally-supported case still works): `{{1,0,2},{0,1,3}}` → itself. row>col is **not** in the suite (UB) — it is pinned by the ASan probe pair only. |
| D-P2b-1 | Guard placement | One guard at the diagonal step: `if ( sum <= value_type(0) ) return false;` **before** `a[i][i] = sqrt(sum)`, inside the `i == j` branch, wrapped in `if constexpr ( ! std::is_complex_v< value_type > )` (complex has no ordering; legacy path preserved, returns `true` on completion — no in-repo complex callers). Q5: the PRD's "keep the `a[i][i]==0` check" refers to a check that does not exist in the current source; the strict-positivity guard subsumes it (a zero `a[i][i]` can only result from a zero diagonal step, which already returns false, so no later divide-by-zero is reachable). |
| D-P2b-2 | Boundary semantics | Strict: diagonal step `sum` must be `> 0`. `1×1 {0}` → false (adversarial case); PSD-singular (`[[1,1],[1,1]]`, second step = 0) → false; non-PD (`[[1,2],[2,1]]`) → false at the second diagonal, **before** the `sqrt` — `a` receives no NaN; PD (`[[4,2],[2,3]]`) → true, `a = [[2,0],[1,√2]]`. Tiny positive steps stay `true` (documented strict-PD semantics; false-rejection only at the mathematically singular boundary). |
| D-P2b-3 | Tests | New `tests/cases/cholesky.hpp`: the four adversarial cases as REQUIREs: non-PD → `false`; PD → `true` + `a·aᵀ ≈ m` (finite tolerance); `1×1 {0}` → `false`; `1×1 {4}` → `true` with `a[0][0] == 2`. No NaN assertions (R-19: `-Ofast` folds NaN comparisons — the non-PD path provably never produces NaN post-fix, so none are needed). |
| D-01 | E10–E13 probe | Single TU `.work/probes/E10_E13.cc`, built exactly per the contract's deterministic check: `g++ -std=c++20 -DPARALLEL -O1 -o .work/probe_s4 .work/probes/E10_E13.cc && .work/probe_s4` (asserts live, no fast-math) → prints `PASS`. Row>col excluded (pre-existing UB; ASan pair covers it). |
| D-02 | Eval seeds | E10–E13: `seeded` → `promoted` (probe + permanent home in `tests/cases/`), mirroring S1–S3. |
| D-03 | Commits / docs / handoff | Per-task commits (`S4 pre-flight: …`, `S4 task N: …`, `S4 closeout: …`); phase docs in `docs/session_4/` (this set); handoff at `.work/handoff_session_4.md` per `docs/templates/handoff.md`; `docs/eval_seed_cases.md` + `docs/risk_register.md` updated at closeout. |

## Example impact (expected, print-only — no edits)

`make example` stdout is expected **identical** to the S3 baseline (grep-verified callers):
- `gauss_jordan_elimination`: example 0020 uses `rand<double>(64, 128, 1)` — 64 rows × 128 cols, i.e. `row < col`, the originally-supported case; the relaxed precondition still passes and the algorithm body is unchanged → bit-identical output.
- `mean/variance/standard_deviation`: the only example use is a **commented-out** `variance` (0026) on a `double` matrix — dead code.
- `conv(…, "same")` with a 1×1 kernel and `cholesky_decomposition`: zero example uses.

The `make example` + `images/` `git checkout` policy (S3 watch item) still applies: verify stdout delta, `git checkout -- images/` before commits.
