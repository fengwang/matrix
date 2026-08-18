# Session 3 — Sharded Review

- State reviewed: commit `e2ac38d` (branch `phase-1/session-3`), baseline `5c8fad9`.
- Method: four read-only shards, each re-verifying one change cluster against hand-derived
  references, the specs in `docs/session_3/specs/`, and the committed test evidence. All checks
  re-run on the committed state; evidence in `.work/evidence/` (gitignored).
- Verdict convention: PASS / FAIL per shard; findings logged with severity
  (Critical / High / Medium / Low). High/Critical would require fixes before closeout.

## Shard 1 — LU partial pivoting (P2): `matrix.hpp` hunks @@ -6582..-6670, `tests/cases/lu_pivoting.hpp`

Verified:

1. Algorithm: Doolittle with partial pivoting on working copy `M`; on swap of working rows
   `j` and `p`, only rows `j`/`p` of `M` are exchanged (rows `0..j-1` of `M` still feed later U
   columns and must not move), L-subrows `k<j` follow the rows, `perm`/`sign` tracked. Matches the
   hand-derived 3×3 (A = [0 1 2; 1 0 3; 4 5 6]): two swaps, perm = [2,0,1], sign = +1,
   L = [1 0 0; 0 1 0; 1/4 −5/4 1], U = [4 5 6; 0 1 2; 0 0 4], PA = LU, det = 16.
2. Suite assertions (stage B) pin perm, sign, every L/U entry, PA == LU (via `L*U`), and
   det-consistency `sign·∏U_ii = 16` — all green (79 assertions, `[lu_pivoting]`).
3. `lu_solver` builds `Pb[i][j] = b[perm[i]][j]` and solves L·Y = P·b, U·x = Y — E09 6×6
   invariance vs the in-TU legacy no-pivot oracle (`||x_lib − x_legacy||∞ = 0`) and vs the exact
   solution (`4.4e-16`) both in-suite and in the `-O1` probe.
4. 3-arg overload delegates to the 5-arg; the tuple overload and both `lu_solver` overloads keep
   source-compatible signatures (no public signature change outside the sanctioned 5-arg add).
5. TDD red was genuine: pre-fix the zero-first-pivot rescue returns `has_value` with x = NaN
   (suite flags fold the isinf/isnan guards — see finding F1); the bit-level finiteness check
   (`lu_is_finite`) fails pre-fix (`.work/evidence/t3_red.log`) and passes post-fix.
6. Singular-system `nullopt` pinned in the `-O1` probe (E09b: `has_value=0` pre- and post-fix) —
   guard behavior observable only outside the fast-math suite (F1).

Verdict: **PASS**.

## Shard 2 — determinant (C5/P7): `matrix.hpp` hunk @@ -2052, `tests/cases/det.hpp`

Verified:

1. `det()` is now the single pivoted-LU path `sign · ∏U_ii`; Schur-complement recursion and the
   1×1/2×2 special cases removed; 0×0 → `value_type{}` preserved; non-square `better_assert`
   preserved (message typo fixed); `noexcept` dropped (body allocates L, U, perm, working copy —
   the pre-fix body already allocated and inverted, so the old `noexcept` was already false).
2. P7 policy: exact zero pivot returns exactly `0` (no epsilon); `lu_decomposition` failure
   signal (rc ≠ 0) also returns `0`. Pre-fix the singular block 4×4 (E07 matrix) returned `-nan`
   (probe + suite). Post-fix: probe prints `0`; suite case passes via the bit-level exact-zero
   check (`det_is_zero`, bits == 0).
3. The plain `REQUIRE( ds == 0.0 )` was **insufficient** as the red/green assertion: under the
   suite's `-Ofast`, `-nan == 0.0` evaluated to `1` (observed: `.work/probes/det_prefix_check.cc`,
   bits `fff8000000000000` → `== 0.0` → 1). The bit-level check cannot be folded (F2).
4. Nonsingular 4×4 (det = 51, hand-verified) matches the independent Bareiss reference in both
   the suite case and the probe; 3×3 two-swap det = 16; near-singular diag(1, 1e-14) is nonzero
   and ≈ 1e-14 (no threshold zeroes it).
5. One test bug found and fixed during TDD (TEST_BUG, not library): `[[2,1],[3,2]]` det asserted
   with exact `== 1.0`; the LU multiplier 2/3 is not binary-exact → replaced with a 1e-12
   tolerance. Library behavior unchanged.

Verdict: **PASS**.

## Shard 3 — pseudoinverse (C4/R1) + flip aliases (C3): `matrix.hpp` hunks @@ -4575, @@ -5303, `tests/cases/pinv.hpp`, `tests/cases/flip_aliases.hpp`

Verified:

1. `svd_inverse` now calls `singular_value_decomposition( a, u, w, v )` (matching the signature
   `(A, u, w, v)`), inverts `w` where `|σ| > 1e-10` (strict — inherited rule, P7: no new
   epsilon; `pinv([1e-10]) = 1e-10` confirmed not inverted in the probe), returns
   `v · w · uᵀ`. `pinverse` is `return svd_inverse( m );` (single core); `pinv` unchanged.
   All three names still compile (retirement is S6's job — nothing renamed/removed).
2. `pinv == pinverse` bitwise (delegation), `pinv(diag(1,2)) = diag(1, 0.5)` (E06, red pre-fix:
   `1.5 < 1e-8`), 4×2 full-rank and 4×2 rank-1 satisfy all four Moore–Penrose conditions
   (< 1e-8), diag(1,1,0) → itself, zeros → zeros, 1×1 boundary 1e-10/2e-10. All green (38
   assertions, `[pinv]`).
3. MP-residual helper dimensions re-checked: for 4×2 a, `a·p·a` is 4×2, `p·a·p` is 2×4,
   `a·p`/`p·a` are square — the max-diff loops compare equal shapes.
4. Wide-matrix exclusion (D4/R-P5): no m<n MP assertions in the suite; the gap is documented
   in the test header and the spec (SVD core invalid for m<n — F3).
5. `fliplr` → `flipdim(m, 2)`, `flipud` → `flipdim(m, 1)`; `flipdim` bodies untouched (S1's);
   content assertions (2×3, ragged 3×3, 1×3, 3×1, 1×1) green; existing `[flip]` S1 cases still
   green (150 assertions with the alias cases).

Verdict: **PASS**.

## Shard 4 — `operator^` (C6) + test quality + process: `matrix.hpp` hunk @@ -5647, `tests/cases/matrix_power.hpp`

Verified:

1. Odd branch is now `half = lhs^(n>>1); return half * half * lhs;` — n=3 → m³, n=5 → m⁵,
   n=1 handled by the earlier `1 == n` branch; even branch and n=0 unchanged (R-M2). No new
   precedence surface: `half * half * lhs` is a left-associative product (R-M3).
2. TDD red was the compile error itself (`matrix.hpp:5650:36: no match for 'operator*'
   (operand types 'uint_least64_t' and 'const feng::matrix<double>')`, `.work/evidence/t5_red.log`)
   — the whole function failed to instantiate at any call site because n is a runtime value.
3. Post-fix: `[matrix_power]` 84 assertions green (Jordan block A^k = [1 k; 0 1] for k = 0..5,
   3×3 vs naive loop oracle k = 0..5, 1×1 2^k exact for k = 0..5); E08 probe compiles and
   passes (`m^3 = m·m·m`).
4. Test-quality rules applied across the five new case files:
   - degenerate-domain assertions use fast-math-immune bit checks (`lu_is_finite`, `det_is_zero`)
     plus defense-in-depth ranges; finite-domain assertions use plain comparisons;
   - no suite case triggers a `better_assert` abort (suite has asserts on);
   - "would fail if the bug were reintroduced" mapping holds for all five fixes
     (alias swap → flip content; missing w inversion → pinv diag(1,0.5); Schur recursion →
     −nan vs bit-zero; precedence bug → compile error; no pivoting → NaN vs bit-finiteness).
5. Process/scope:
   - `git diff --name-only 5c8fad9` (working tree and HEAD): `docs/session_3/**` (pre-flight),
     `matrix.hpp`, the five new case files, `tests/test.cc` — all within the contract blast
     radius; no `examples/**`, no ReadMe, no `.work/` tracked, images restored.
   - `matrix.hpp` hunks confined to the six sanctioned regions (det 2052; flip 4575; pinv 5303;
     operator^ 5647; LU 6582-6670).
   - Required checks all green on the committed state: `make test` (69 cases / 49,217,068
     assertions), `./test_test`, `make example` exit 0, probes E05–E09 all PASS.
   - Example stdout delta vs baseline: exactly one line — example 0019 LU-solver MAE
     1.30e-11 → 1.57e-10 (expected: different pivot order → different-but-equally-valid
     factorization; F4). Example 0005 (127×127 diag det) stdout identical.

Verdict: **PASS**.

## Findings

| ID | Severity | Finding | Disposition |
|----|----------|---------|-------------|
| F1 | Medium | The suite's `-Ofast` (fast-math) build folds the library's `isinf`/`isnan` guards: pre-fix `lu_solver` on a degenerate 3×3 returned `has_value` with x = NaN instead of `nullopt`; the same folding makes suite-level IEEE guard behavior unobservable. Pre-existing build property (Makefile is out of blast radius), not an S3 code defect — but it shaped the test strategy (bit-level finiteness checks; singular-nullopt pinned in the `-O1` probe). | Documented in `lu_pivoting.hpp` header + this review; carried to `.work/handoff_session_3.md` and `docs/risk_register.md`; future sessions must use fast-math-immune assertions for degenerate-domain checks. |
| F2 | Medium | Specific F1 instance: `std::abs(nan − c) < e`, `nan < c`, and even `nan == 0.0` can compile to `true` in the suite build (observed in-context; a standalone TU with the same flags evaluated some of the same forms to false). No comparison form proved context-stable; bit-pattern checks via `memcpy` are the reliable form. | Same as F1. |
| F3 | Medium | The SVD core is numerically invalid for wide matrices (m < n): reconstruction error 6.0, `u` entries ~1e306 (pre-existing; tall/square valid). Contract's "2×4 rank-deficient pinv" adversarial case narrowed (user decision D4); no m<n MP assertions in S3. | Logged as S6 candidate in handoff + risk register; spec R-P5 documents the exclusion. |
| F4 | Low | Example 0019 LU MAE changed 1.30e-11 → 1.57e-10 after pivoting (pivot-order roundoff difference; both negligible). | Documented as expected delta; evidence in `.work/evidence/final_example.log` diff. |
| F5 | Low | Integer-matrix `det` (n ≥ 3) is numerically meaningless both pre- and post-fix (truncating LU division / truncated Schur inverse) — no compile regression, quality class unchanged. det/pinv tests use double, per P7. | Observation only; out of scope. |
| F6 | Low | Self-inflicted during task 3: first `lu_is_finite` masked the 11-bit exponent with 9 bits (`0x1FF`), making the check a tautology (GCC `-Wtautological-compare` caught it). Fixed to `0x7FF`. | Fixed in-tree; lesson recorded (verify IEEE field widths when writing bit checks). |
| F7 | Low | `svd_inverse` ignores the SVD return count (non-convergence is silent) — pre-existing behavior, preserved by S3's fix. | Observation; handoff warning for S6. |

## High/Critical fixes required by this review

None. No Critical or High findings; all four shards PASS on the committed state.
