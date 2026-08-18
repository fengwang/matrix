# PRD — Matrix Library Upgrade (Findings Repair + Bounded Modernization)

- **Status:** v1, two-pass compiled (draft → adversarial spec review → revised; revision record in §10).
- **Reads:** `docs/project_contract.md` (law), `docs/evidence_map.md` (proof), `docs/risk_register.md` (risks), `docs/eval_seed_cases.md` (probes), `docs/session_{n}.md` + `docs/session_{n}_contract.yaml` (per-session authority).
- **Source findings:** `docs/opencode_sharded_review.md` (2026-07-13; 22 findings, all with verified evidence).

## 1. Problem

The single-header C++20 matrix library (`matrix.hpp`, 7,688 lines) has a fully green test suite (57 cases, 49.2M assertions) that coexists with two Critical heap-corruption bugs, a security hole in file input (`load_npy`), a pseudoinverse that doesn't invert, a determinant that returns `NaN` on valid singular input, and a matrix power operator that doesn't compile for half its domain. The suite is green precisely *because* the buggy paths are untested (T1). The library's own conventions (MATLAB/NumPy) are violated by its flip aliases. A sharded review produced 22 findings with evidence, smallest-safe-fixes, and a suggested order. This project converts that report into a budget-safe, contract-driven, evidence-checked repair program.

## 2. Confirmed intent (interview, 2026-08-17 — explicit user yes)

- **Outcome:** a complete planning + contract document set (this PRD, six session story outlines, the project contract, an evidence map, six session contracts, a risk register, eval seeds) that directs the upgrade. No code is written in the blueprint turn.
- **User:** future fresh-context development sessions that pick a session story one at a time, refine it, and execute it; the user as human decision gate on high-risk sessions.
- **Why now:** 2 Critical memory-corruption bugs and a file-input OOB are waiting; `AGENTS.md` already defers session authority to contract documents that did not exist before this blueprint.
- **Success:** any one of the six sessions can start from a fresh ~128K context, load only its named regions, complete **without context compression**, and exit on deterministic evidence (`make test` + new regression cases + ASan probes where mandated) plus a filled handoff doc.
- **Constraint:** ~128K tokens per session, no context compression; `matrix.hpp` alone ≈ 80–85K tokens (estimate), so every session reads surgically by function name.
- **Scope shape (user decision):** findings repair **plus one modernization session** (real FFT + API hygiene + ReadMe/cheatsheet). Findings are the core; the deep-research docs are semantics authority and future context, not work items.

## 3. Goals

1. **Correctness:** eliminate all 13 `C*` findings; every fix ships a content-asserting regression test.
2. **Safety:** `load_npy` validates all external input at the boundary (S1 finding); `save_png` survives open failure; `NDEBUG` policy is explicit; `rand` is thread-safe with deterministic explicit seeds.
3. **Honest performance:** `fft`/`ifft` are genuinely O(N²·log N) for power-of-2 sizes (separable radix-2), with the naive path retained as a documented fallback for other sizes.
4. **API hygiene:** one canonical public name per operation (rule in project contract §3); documented conventions (flip/conv/fftshift) match MATLAB/NumPy.
5. **Docs:** the ReadMe (including its usage/cheatsheet-style sections; no separate cheatsheet file exists — verified this turn) reflects every sanctioned behavior change.
6. **Process:** every session is contract-bounded, evidence-checked (sharded review + adversarial verifier), and handoff-complete — the eval-loop assets (`docs/prompts/*`) are exercised end to end.

## 4. Non-goals (this project)

- **No new features** beyond finding fixes + S6 scope. `openimageio` I/O, `cuda_matrix`, `concatenate`, broadcasting, slicing APIs — all deferred (REVIEW.md TODOs; research-doc topics).
- **A1 CRTP teardown** — deferred to its own future project (whole-header refactor; exceeds one 128K session).
- **A3-full elementwise namespacing** — parked (risk R-08). Verified this turn: the current file contains **no** `feng::elem::` calls and **no** `elem` namespace, so the finding as described is unsupported — S6 does a verification pass + hygiene policy note only (no code change expected).
- **R2 elementwise macro consolidation** — deferred (no correctness impact; would churn every test file).
- **C7 full conversion** of `better_assert` to runtime checks — the policy is documented instead (decision C-04).
- **DLPack / NumPy interop** — parked; no consumer exists in this repo.
- No CI infrastructure, no dependency additions (project contract: no production dependencies without approval), no allocator/layout changes.

## 5. Sanctioned public behavior changes

This table **is** the authorization required by `AGENTS.md` ("do not change public API behavior unless the contract says so"). Any required change outside these rows stops the session and is reported.

| # | Change | Old behavior | New behavior | Finding | Session |
|---|---|---|---|---|---|
| 1 | `shrink_to_size` content | wrong column count copied → heap OOB / silent corruption | documented copy+zero-pad/truncate semantics actually hold | C1 | S1 |
| 2 | `flipdim(m,2)` | column-vs-row `swap_ranges` → OOB / corruption | true left-right flip for all shapes | C2 | S1 |
| 3 | `fliplr` / `flipud` | swapped vs convention | `fliplr` = left-right (dim 2), `flipud` = up-down (dim 1) | C3 | S3 |
| 4 | `pinv` (and interim `pinverse`) output | `V·Σ·Uᵀ` (singular values not inverted) | Moore–Penrose pseudoinverse `V·Σ⁺·Uᵀ` (1e-10 threshold) | C4 | S3 |
| 5 | `det` of singular-matrix inputs | silent `NaN` via Schur `P.inverse()` | `0` on exact zero pivot; LU-based computation (values may shift in low ulps for nonsingular inputs) | C5, C-07 | S3 |
| 6 | `operator^` odd n≥3 | compile error | works (`(lhs^(n>>1))²·lhs`) | C6 | S3 |
| 7 | `lu_decomposition` L/U factors | no partial pivoting | partial pivoting; **solutions unchanged**, factors differ; L/U-printing example output changes | P2 | S3 |
| 8 | `mean`/`variance`/`standard_deviation` on **all** scalar types | integer truncation on int matrices (`mean({1,2;1,2}) == 1`); float matrices return `float` | uniform `double` return (int `== 1.5`; float/double unchanged within rounding); the `n−1` sample-variance formula is **kept** | C8 | S4 |
| 9 | `conv(..., "same")` with 1×1 kernel | debug abort / unguarded column bound | accepted (well-defined); asserts corrected to check `cb` | C9 | S4 |
| 10 | `rref` / `gauss_jordan_elimination` on square systems | debug abort (precondition `row < col`) | accepted; precondition `row>0 && col>0`; pivot early-exit (1e-10) remains the singularity signal | C10 | S4 |
| 11 | `cholesky_decomposition` | `void`; NaN on non-PD input | `bool`; `false` when a diagonal step is not positive-definite (zero in-repo callers — safe) | P2 | S4 |
| 12 | `NDEBUG` policy | implicit (silent no-op) | **documented** in ReadMe/cheatsheet: `better_assert` is debug-only; I/O boundaries use hard runtime checks, never asserts | C7 | S5 (doc delta → S6) |
| 13 | `rand` value stream / thread-safety | global `srand`/`rand`, re-seeded per call, race-unsafe | local `std::mt19937` + `uniform_real_distribution`; **explicit seed ⇒ deterministic stream (invariant)**; seed 0 ⇒ time-based; range `[0,1)` unchanged | C11 | S5 |
| 14 | `save_png` on open failure | null-`FILE*` UB | silent no-op + documented; stray `;;` removed | S2-finding | S5 |
| 15 | Name retirements | `random`, `random_like`, `pinverse`, `svd_inverse`, free `det(m)` exist | deleted; canonicals `rand`/`rand_like`/`pinv`/member `det()` remain; `examples/0013` + ReadMe updated to canonical names | A2, C-05 | S6 |
| 16 | `fft`/`ifft` | correct naive O(N⁴) DFTs (the review's "no-op stub" description does not hold — verified this turn); `ifft` **lacks the `1/(R·C)` normalization**, so `ifft(fft(x)) == R·C·x` | fast path: separable radix-2, O(N²·log N) for power-of-2 sizes, results identical to the naive DFT within rounding (differential-tested against it); `ifft` gains `1/(R·C)` so the round-trip is the identity; non-PoT sizes keep the naive path as a **documented fallback** | P1 | S6 |
| 17 | `fftshift`/`ifftshift` odd dimensions | swap-based remap (equals NumPy for even `n`; wrong for odd `n`) | NumPy circular roll by `(n+1)/2` per axis (probe-verified, even + odd); even-dim behavior must stay bit-identical; the fused transform+shift design is kept and documented (deliberate deviation from NumPy's pure reindex) | C13, C-01 | S6 |
| 18 | `load_npy` on malformed/truncated/foreign-dtype files | UB / heap OOB read / `std::terminate` (no defined behavior) | returns `false` (hard boundary checks per P3); valid files load exactly as before | S1-finding | S2 |

## 6. Scope allocation (22 findings → 6 sessions)

| Session | Findings | One-liner | Risk / routing |
|---|---|---|---|
| S1 | C1, C2 | the two Critical memory-corruption one-line fixes + regression tests | high / branch_and_compare + human gate |
| S2 | S1 (report) | `load_npy` boundary validation (size, header, dtype) + negative-path tests | high / branch_and_compare + human gate |
| S3 | C3, C4, C5, C6, R1, P2(LU), R3-slice(det-typo) | numerical-semantics session: flip aliases, pinv, det-via-LU + partial pivoting, `operator^` (det message typo fixed in the rewrite) | medium / worker_plus_reviewers |
| S4 | C8, C9, C10, P2(cholesky) | integer-stat promotion, conv/rref precondition fixes, cholesky guard | medium / worker_plus_reviewers |
| S5 | C7, C11, C12, S2 (report), R3-slice(`;;`) | robustness: NDEBUG policy doc-delta, `rand`→mt19937, core-count guards, `save_png` (stray `;;` dies with it) | medium / worker_plus_reviewers |
| S6 | P1, C13, A2, A3(verify), ReadMe | modernization: fast FFT + `ifft` normalization, fftshift fix, alias retirements, A3 verification pass, docs sweep | medium / worker_plus_reviewers |
| deferred | A1, A3-full, R2, R3(const-sweep), C7(convert) | see §4 non-goals (the R3 det-typo and save_png `;;` slices are **not** deferred — they ride with S3/S5) | — |

**Execution order and dependencies:** S1 → S3 → S6 is a hard chain (C2 fixed before the C3 alias swap is meaningful; S6 retires names whose implementations S3 fixed and documents S3–S5 deltas). S2, S4, S5 are independent of the chain and of each other. **S6 runs last.** The full suite must stay green after every session — each session is independently mergeable.

## 7. Policy decisions (binding for all sessions)

- **P1 — Canonical names:** the name documented in `ReadMe.md` is canonical; undocumented duplicates are retired in S6 (project contract §3). Pseudoinverse canonical = `pinv`; the SVD-inversion implementation becomes the single private core.
- **P2 — `NDEBUG` policy:** `better_assert` remains debug-only and is *documented* as such; **I/O boundaries** (`load_npy`, `save_png`, file ops) use hard runtime checks returning `false`/no-op — never `better_assert`, never UB. Full assert-conversion is out of scope.
- **P3 — Untrusted input rule:** data from files is untrusted; validate *before* any dereference (model: `load_bmp` at ~6760). `load_npy` rejects (returns `false`) on: file < 12 bytes, inconsistent header length, missing dtype match for the target type, or truncated payload. `noexcept` stays only where the body genuinely cannot throw; `stoul` exceptions are caught → `false`.
- **P4 — Doc deltas:** fix sessions S1–S5 do **not** edit `ReadMe.md`; each lists its doc deltas in the handoff. S6 consumes all deltas and owns the ReadMe/cheatsheet sweep (single writer, no drift races).
- **P5 — Probe-first:** every assigned finding is re-verified with a minimal probe before its fix is written. Mandatory for C13 (derived finding; conflict C-01) and for any finding whose evidence conflicts with a reference.
- **P6 — FFT scope:** radix-2 Cooley–Tukey, separable (row-wise then column-wise), trig precomputed; power-of-2 sizes only for the fast path; all other sizes use the retained naive implementation (moved to a private helper), documented in the cheatsheet with the complexity table. No planar/Bluestein work.
- **P7 — `det` singularity rule:** exact zero pivot ⇒ `det == 0` (documented contract). No epsilon threshold for "near-singular" — tiny nonzero determinants are correct floating-point answers.
- **P8 — Regression tests ship with fixes (T1/T2):** content-asserting cases in `tests/cases/`, registered in `tests/test.cc`; error-path cases for S2 (truncated/dtype-mismatch npy), S3 (singular det), S4 (square rref, 1×1 conv, non-PD cholesky).
- **P9 — Contract lifecycle:** session contracts are v1; refinement at session start is allowed (narrow/clarify only), logged in the handoff decision log; blast-radius expansion is never allowed silently (project contract §1).
- **P10 — Budget discipline:** each session reads its context budget map (its `session_{n}.md` §context budget) and nothing more of the header; research docs are read at named sections only, never in full; `make test` output is the primary evidence artifact.

## 8. Success criteria (project level)

1. All 18 sanctioned changes landed with cited evidence; no behavior change outside the §5 table (audited via `git diff` per session).
2. Full suite green + new regression cases green; `make example` green; ASan probes (E01, E02, E03) clean.
3. Every eval seed E01–E18 passes on the final tree (the standing smoke set).
4. Handoff docs exist for all six sessions; risk register entries closed or explicitly re-owned; no finding left untracked.

## 9. Deep-research document usage (binding)

`docs/deep_research/deep-research-report (6).md` ("Designing a NumPy-Like Tensor and Algebra Library for the C++ Standard Library") and `docs/deep_research/C++ Standard Tensor Proposal Blueprint.md` (P3500R0) are referenced **by section and line** (verified this planning turn). They serve exactly three roles:
1. **Semantics authority** where a fix needs a convention (NumPy semantics: report 6 §152 "Proposed semantic model and API blueprint").
2. **Scope discipline** (report 6 §1314 scope-explosion risk — FFT is a named growth vector; §1335 "which FFT should we provide?"; P3500R0 §144 execution domains — do not over-claim performance).
3. **Future context** for the deferred items (P3500R0 §79 class template architecture, §136 storage, §171 DLPack — the A1/DLPack parking rationale).

They are **not** work-item sources: no session implements standard-library tensor features from them. Sessions never read them in full (budget); the per-session named sections are listed in each `session_{n}.md`.

## 10. Revision record (two-pass contract compilation)

First pass: draft from requirement + repo evidence + review report. Second pass (adversarial spec review) removed/changed:
- **C-01:** caught that the review's C13 worked example contradicts NumPy's `fftshift` for odd sizes; replaced "fix as review states" with **probe-first + NumPy-pinned semantics** (would have shipped a wrong fix from a derived finding).
- **C-02:** the review's R2 line range (6495–6930) is wrong (actual 6840–7189); anchors policy hardened (names authoritative, lines hints).
- **A3 scoped down:** full `feng::elem::` namespacing removed from S6 (would have broken every test file and blown the budget); minimal `norm` cleanup + policy note kept; full namespacing parked in the risk register.
- **C-05:** canonical pseudoinverse name fixed to the *documented* `pinv` (not the review's probe name `pinverse`); `pinverse`/`svd_inverse` names retired in S6.
- **C-04:** C7 settled to "document + I/O-boundary hard checks" (removes an ambiguous dual-path clause from S5).
- **P2(cholesky):** `void→bool` signature change surfaced and added to the §5 table (was missing; would have violated the API policy).
- **C11 invariant added:** explicit-seed determinism (found by grepping examples for seeded `rand` calls) — without it, S5 could silently break the examples' reproducibility comments.
- **A2 blast radius made concrete:** `examples/cases/0013_prefix.hpp:3` uses `feng::random` — added to S6 `allowed_files` (consumer audit performed).
- Removed: a planned "R3 top-level-const sweep" (over-specified cosmetics; deferred), per-finding line-number MUSTs (untestable after drift; replaced by function-name anchors), any requirement to convert `better_assert` globally (untestable at scale / out of scope).
- **C-08 (code re-verification this turn):** the review's P1 description ("no-op stub") is unsupported — `fft`/`ifft` are correct O(n⁴) DFTs; the real `ifft` gap is the missing `1/(R·C)` normalization (now §5 row 16). The review's C13 description (index remap) is also unsupported — the code is a **swap-based** remap that happens to equal NumPy for even `n` and diverges for odd `n` (probe-first retained; even-dim bit-identity now a regression pin). A3 is unsupported in the current code (no `feng::elem::`/`elem` namespace) → S6 does verification + policy note only. Two **extra** `load_npy` hazards found while verifying (beyond the review): `header_length` overflow in offset arithmetic; `npos`-unguarded shape parse — both in the S2 contract.
- **C-09 (finding allocation fix):** the `det` message typo (R3 slice) lives inside the function S3 rewrites → allocated to S3; the `save_png` stray `;;` goes to S5 (its region). §6 updated.
- **C-10 (eval seed fix):** E10's `standard_deviation` expectation corrected to `√0.5 ≈ 0.70711` (the `n−1` sample formula is preserved — the seed had wrongly encoded the population value 0.5).
- **C-11 (§5 completeness):** `load_npy`'s invalid-input behavior was missing from the sanctioned table (S2's exit criteria cite it) → added as row 18; §8 "17" → 18 sanctioned changes.
- **Structure check:** PRD serves the stated goal (repair within budget), not an assumed one (tensor-standardization — explicitly fenced in §4/§9); coupling points (finding→session, canonical names, line numbers, doc ownership) each got a single-owner rule (§5 table, P1, contract §7, P4).
