# Session 3 — Brainstorming (refinement record)

Status: refinement only (policy P9 — narrow/clarify, no scope widening). The problem space was
explored in the 2026-08-17 blueprint interview (PRD §2) and the 2026-07-13 sharded review
(`docs/opencode_sharded_review.md`, findings C3/C4/C5/C6/R1/P2). This document records the
session-start interview-me pass and the design decisions. No new exploration.

**Process note (interview-me skill):** the project-level intent interview (PRD §2, explicit user
yes) already fixed the *what*; the session contract fixes the *how*. The pass below stress-tests
the contract set against every residual decision point. Exactly **one** point could not be
resolved from the contract set (Q1 — the 2×4 wide-matrix pinv adversarial case vs. the
empirically demonstrated wide-SVD limitation) and was asked to the user live this session;
the user answered **(a): narrow** (4×2 tall + 3×3 rank-deficient pinv cases; the 2×4 wide gap
is logged, not fixed). Nothing else remains that only a user answer could resolve.

## Interview-me pass

HYPOTHESIS: the user wants the S3 contract executed end to end — C3/C4+R1/C5+P2/C6 fixed with
TDD, content-asserting tests, deterministic probes E05–E09, sharded review, adversarial
verification, and a handoff — with zero scope creep beyond `docs/session_3_contract.yaml`.
CONFIDENCE at session start: ~90% (one contract-vs-reality divergence unresolved); **96%** after
Q1 answered.

### Q1 — the 2×4 rank-deficient pinv adversarial case (asked live; answered "a")

The contract lists "pinv of a 2×4 rank-deficient matrix" among `adversarial_cases`. The pre-fix
SVD supplement probe (`.work/evidence/prefix_probes.log`) proved the SVD core is numerically
**invalid for wide matrices (m < n)**: 2×4 SVD reconstruction error 6.0, `u` entries ~1e306;
tall (4×2) and square SVDs are valid (reconstruction 2.2e-16). SVD's public behavior is
explicitly out of S3 scope ("the SVD's public behavior beyond the inversion core is not in S3
scope"). Any Moore–Penrose assertion on a 2×4 `pinv` would therefore fail even after a correct
C4 fix — a spec gap, not a fixable bug.

**Resolution (user-confirmed, option a):** the adversarial pinv set is **narrowed, not dropped**:
4×2 tall rank-deficient + 3×3 rank-deficient cases (both have valid SVDs and the same
rank-deficiency property the case tests). The wide-2×4 gap is recorded in the decision log
(this file), `docs/risk_register.md`, the E06 seed footnote in `docs/eval_seed_cases.md`, and
the handoff warning as a pre-existing SVD limitation and **S6 candidate**. Not a scope
widening (option c rejected: fixing wide SVD would violate the contract's scope clause).

## Decision table (stress-test of residual points)

| # | Question | Resolution (source) |
|---|---|---|
| D1 | Which exact C3 line changes? | `fliplr → flipdim(m, 2)`, `flipud → flipdim(m, 1)` (PRD §5 row 3: "MATLAB/NumPy convention"; contract in-scope line 1). Pre-fix probe: both aliases return the *other* flip (E05 FAIL). |
| D2 | Where does the single pinv core live? | **`svd_inverse`, fixed in place.** The R1 line mandates fixing `svd_inverse`'s swapped argument order anyway; the fixed body calls `singular_value_decomposition(a, u, w, v)` (names now match the `(A, u, w, v)` signature), inverts `w` in place (inherited 1e-10 threshold), returns `v * w * u.transpose()`. `pinverse` becomes `return svd_inverse( m );` (contract: "pinverse delegates to the single correct SVD inversion core"). `pinv` untouched (already delegates). Both public names keep compiling (retirement is S6's job). Pre-fix `svd_inverse` is numerically correct *only* because the swapped names happen to compute `V·Σ⁺·Uᵀ`; post-fix it computes the same value with honest names (E06 pre-fix evidence: `svd_inverse(diag(1,2)) = diag(1,0.5)` already, `pinverse(diag(1,2)) = diag(1,2)`). |
| D3 | Threshold semantics at the boundary? | Inherited rule, strict `> 1e-10` (P7 forbids new epsilons; inherited threshold is the current `svd_inverse` behavior): σ exactly 1e-10 → **not** inverted (stays 1e-10); σ = 2e-10 → 5e9. Pre-fix probe pins both sides (`prefix_probes.log`). Pinned by a test + the E06 seed. |
| D4 | 2×4 wide pinv adversarial case? | **Narrowed to 4×2 + 3×3** — Q1 above (user-confirmed). |
| D5 | How does pivoting expose permutation + sign? | New 5-arg primary `int lu_decomposition( A, L, U, int& sign, std::vector<size_type>& perm )`: `P·A = L·U`, `perm[i]` = original row index of permuted row `i`, `sign = det(P) ∈ {−1,+1}`. The existing 3-arg overload **delegates** (creates dummy sign/perm) — source-compatible; no existing call breaks (project contract §3: no public signature changes; overloading is additive). The 1-arg tuple overload and `lu_solver` overloads keep their signatures. `sign` alone is *not* sufficient for the solver (it needs the full permutation) — the contract's own wording "permutation and sign are exposed so lu_solver applies P to b" confirms both channels. |
| D6 | Pivoting algorithm? | Partial pivoting on a **working copy** `M` of `A` (PA = LU form; `A` stays untouched, matching the 3-arg function's `A const&` contract). Per column `j`: `p = argmax_{i≥j} |M[i][j]|`; if `p ≠ j`: swap `M` rows `j↔p`, swap already-computed `L[j][k] ↔ L[p][k]` for `k < j` (verified necessary by hand-derived 3×3 invariant check), swap `perm[j] ↔ perm[p]`, flip `sign`. Then the existing Doolittle column accumulation, reading `M` instead of `A`. The existing `isinf/isnan → return 1` guard stays (failure signaling unchanged). **No** explicit zero-pivot `return 1` added to `lu_decomposition` itself: a zero pivot in the *last* column currently yields rc=0 (no L-division happens) and `lu_solver` still fails correctly via the `backward_substitution` inf/nan guard — adding the check would be an unsanctioned behavior change on the singular edge. `det` gets its own exact-zero check (D7). Hand-derived verification (3×3 example, two swaps) in `design.md`. |
| D7 | `det` rewrite details? | Single pivoted-LU path (functional-thinking: one Calculation; the 1×1/2×2 fast paths are removed — they are micro-optimizations whose special cases are exactly the bug class C5 reports, and LU handles them in one or two columns). `P·A = L·U` ⇒ `det(A) = det(P)·det(L)·det(U) = sign · ∏ U[i][i]`. Failure → `return 0`: (a) decomposition rc≠0 (zero pivot before the last column, or inf/nan), (b) **explicit** `U[i][i] == 0` → `return 0` (exact zero, **no epsilon** — P7; this also covers the rc==0 last-pivot gap D6 notes). `size == 0 → 0` **kept** (current behavior; changing to the mathematical empty product 1 is unsanctioned). Non-square stays `better_assert`-guarded (undefined, unchanged). Message typo fixed ("the row and matrix are supposed to be same" → "…row and col…"). `noexcept` **dropped** from `det()` — the body now allocates (L, U, perm, M); project contract §3 sanctions dropping `noexcept` where the body can throw. |
| D8 | 0×0 det? | Returns `0` (current `0 == size → value_type{}` branch kept verbatim — behavior preservation; D7). |
| D9 | `operator^` odd branch? | `auto const half = lhs ^ ( n >> 1 ); return half * half * lhs;` — log₂ recursion, no precedence trap (the fixed line contains no `^` and no implicit `*`-vs-`^` ambiguity), correct for every odd `n ≥ 1` (n=1: half = m^0 = I → I·I·m = m ✓; n=3: m·m·m ✓). n=0 and n=1 fast paths unchanged; even branch unchanged. Note (empirical, E08 probe): pre-fix the bug is a **hard compile error for all n** (n is a runtime value, so the ill-formed `uint_least64_t * matrix` expression defeats the whole function instantiation) — "even powers compile" is not actually true pre-fix; post-fix every n compiles. |
| D10 | Test placement and suite-safety? | Five new files in `tests/cases/` (`flip_aliases.hpp`, `pinv.hpp`, `det.hpp`, `matrix_power.hpp`, `lu_pivoting.hpp`) + five include lines in `tests/test.cc` (alphabetical positions verified against the existing list: `cos < det < erfc`, `flip.hpp < flip_aliases.hpp < floor`, `lround < lu_pivoting < matrix_power < mean`, `operator_equal < pinv < pooling`; exact lines in plan.md). The suite build has asserts enabled (no `-DNDEBUG` in the Makefile; `better_assert` aborts in debug — S2's D12 evidence): **no** non-square `det()` or `operator^` call in any suite case; the non-square-undefined paths are pinned by the release-mode adversarial verifier instead. E09's oracle is an **in-TU copy of the legacy no-pivot LU** inside `lu_pivoting.hpp` (deterministic, no external data; the copy is byte-verbatim from the pre-fix tree so it is a true independent implementation). |
| D11 | Eval seeds E05–E09? | All five → `promoted` (probe in `.work/probes/` + permanent home in `tests/cases/`), mirroring S1 (E01/E02) and S2 (E03/E04). E06's row gains the D4 footnote. |
| D12 | Commits / docs / handoff? | Per-task commits following S1/S2 convention (`S3 pre-flight: …`, `S3 task N: …`, `S3 closeout: …`); phase docs in `docs/session_3/` (this set); handoff at `.work/handoff_session_3.md` (project contract §1.4, not `docs/handoff.md`); `docs/risk_register.md` + `docs/eval_seed_cases.md` updated at closeout. |

## Example impact (expected, print-only — no edits)

- `0005_det.hpp` (127×127 diagonal): LU on a diagonal matrix performs **no swaps** (each
  diagonal element is the column max; off-diagonals are 0) → same product, same stdout.
- `0019_lu_decomposition.hpp` (Lenna LU + `lu_solver` MAE): solution **invariant** under
  pivoting (D6 solves the same system); `L`/`U` factors differ (printed to bmp only —
  images/ churn is expected artifact, not a contract item); MAE stdout may shift by
  ~1e-15-level digits.
- `0021_singular_value_decomposition.hpp`: the 1-arg `singular_value_decomposition` return
  tuple order `(u, w, v)` is **unchanged** (contract: "keep the return type (u, w, v) —
  example 0021 depends on it"); SVD's public behavior untouched → identical stdout.
- All other examples: numerics on unchanged paths → identical stdout.

## Failure-mode watchlist (carried into review + verifier briefs)

- **Sign bookkeeping**: an even number of swaps on a matrix the no-pivot code could solve
  (sign must come back +1); the `[[0,1],[1,0]]` odd-swap case pins −1.
- **L-row swap propagation**: omitting the `L[j][k] ↔ L[p][k]` swap makes `P·A ≠ L·U` on
  second-and-later columns (hand-derived 3×3 counterexample in `design.md`; pinned by the
  PA==LU residual test).
- **perm direction**: `perm[i] = original row of permuted row i` (P·A row i = A row perm[i]).
  Inverting the convention makes `lu_solver` apply P to the wrong rows on ≥3-row systems.
- **det zero pivot**: `−0.0` vs `0.0` — C++ `==` treats them equal, but the explicit
  `U[i][i] == 0 → return 0` returns positive zero; the E07 exact-zero check plus a print
  confirms no `−0` in stdout.
- **pinv threshold edge**: σ exactly 1e-10 must stay 1e-10 (strict `>`), not 1e10.
- **wide-SVD gap** (D4): do not let any new test accidentally assert MP conditions on an
  m<n `pinv` — it would encode the pre-existing SVD limitation as a contract.
- **Suite assert-abort** (D10): any non-square `det`/`operator^` in a suite case would
  `better_assert`-abort the whole suite in the debug build.
