# Session 3 — Plan

Micro-task TDD plan for `tasks.md`. Context: `design.md` (decisions D1–D12); specs in
`specs/`. Baseline: `5c8fad9` on the session branch. All commands run from the repo root.

## Task 1 — Pre-flight (already executed; commit point: pre-flight checkpoint)

Done. Evidence: `.work/evidence/prefix_probes.log` (E05/E06/E07 red, E07-nonsingular pass,
E09 oracle self-check, SVD supplement incl. wide-matrix gap),
`.work/evidence/prefix_E08_compile_error.log`, `.work/evidence/baseline_*`, probe sources
`.work/probes/E05_E09.cc` + `.work/probes/E08.cc`, phase docs `docs/session_3/**`.

**Commit:** `S3 pre-flight: phase docs, E05–E09 probes, pre-fix evidence (E05/E06/E07 red, E08
compile error, E09 oracle self-check, wide-SVD gap finding; Q1 user answer: narrow 2x4)`

## Task 2 — C3 flip aliases

**2.1** Create `tests/cases/flip_aliases.hpp` (header comment cites spec
`docs/session_3/specs/flip_aliases.md` + "C3 regression"):

```cpp
#include <cmath>
TEST_CASE( "Matrix fliplr/flipud", "[flip]" )
{
    // C3 regression (spec: docs/session_3/specs/flip_aliases.md).
    // MATLAB/NumPy convention: fliplr = flipdim(m, 2), flipud = flipdim(m, 1).

    // Scenario: E05 2x3 content.
    {
        feng::matrix<double> const m{ 2, 3, { 1.0, 2.0, 3.0, 4.0, 5.0, 6.0 } };
        feng::matrix<double> const lr = feng::fliplr( m );
        feng::matrix<double> const ud = feng::flipud( m );
        double const want_lr[2][3] = { { 3.0, 2.0, 1.0 }, { 6.0, 5.0, 4.0 } };
        double const want_ud[2][3] = { { 4.0, 5.0, 6.0 }, { 1.0, 2.0, 3.0 } };
        REQUIRE( lr.row() == 2 ); REQUIRE( lr.col() == 3 );
        REQUIRE( ud.row() == 2 ); REQUIRE( ud.col() == 3 );
        for ( unsigned long r = 0; r != 2; ++r )
            for ( unsigned long c = 0; c != 3; ++c )
            {
                REQUIRE( std::abs( lr[r][c] - want_lr[r][c] ) < 1.0e-12 );
                REQUIRE( std::abs( ud[r][c] - want_ud[r][c] ) < 1.0e-12 );
            }
    }
    // Scenario: 3x3 ragged content (both aliases).  [9 distinct values; mirrored]
    // Scenario: 1x3 fliplr == elementwise reverse; 3x1 flipud == row reverse.
    // Scenario: 1x1 fixed point (both aliases).
}
```

Register in `tests/test.cc` between the `flip.hpp` and `floor.hpp` lines:
`#include "./cases/flip_aliases.hpp"`.

**2.2** Red: `make test 2>&1 | tail -3 && ./test_test "[flip]" 2>&1 | tail -15`
→ the new case fails on the `want_lr` assertions (pre-fix `fliplr` returns the up-down
flip). Log to `.work/evidence/t1_red.log`. Existing `Matrix flipdim` case still green
(R-F3).

**2.3** Fix `matrix.hpp` (~4577–4587): `fliplr` body → `return flipdim( m, 2 );`, `flipud`
body → `return flipdim( m, 1 );`.

**2.4** Green: `./test_test "[flip]" 2>&1 | tail -5` → all pass; `g++ -std=c++20 -DPARALLEL
-O1 -o .work/probe_s3 .work/probes/E05_E09.cc && .work/probe_s3 2>&1 | grep E05` → both
E05 checks PASS. Commit.

## Task 3 — C4/R1 pinv core

**3.1** Create `tests/cases/pinv.hpp` (tag `[pinv]`; spec `pseudoinverse.md`). Cases:

1. `pinv(diag(1,2))` ≈ `diag(1, 0.5)` (1e-8, off-diag < 1e-8) — E06.
2. `pinv(m) == pinverse(m)` elementwise (same core) on a fixed 3×3 nonsingular matrix.
3. 4×2 full-column-rank `[[1,0],[0,1],[2,0],[0,3]]`: four MP residuals < 1e-8; result 2×4.
4. 4×2 rank-1 `[[1,2],[2,4],[3,6],[4,8]]` (D4 case): four MP residuals < 1e-8; result 2×4.
5. 3×3 `diag(1,1,0)`: result == input within 1e-12.
6. 2×2 zeros → zeros.
7. 1×1 boundary: `pinv([1e-10]) ≈ 1e-10` (NOT 1e10 — assert `< 1e-9` distance from 1e-10);
   `pinv([2e-10]) ≈ 5e9` (relative 1e-3).

Helper (file-local): `mp_residuals(a, p)` returning the max of the four MP norms.

Register in `tests/test.cc` between `operator_equal.hpp` and `pooling.hpp`:
`#include "./cases/pinv.hpp"`.

**3.2** Red: `make test 2>&1 | tail -3 && ./test_test "[pinv]" 2>&1 | tail -20`
→ case 1/3/4/5/7 fail (no inversion pre-fix). Log to `.work/evidence/t2_red.log`.

**3.3** Fix `matrix.hpp` (~5302–5320) per design.md: `svd_inverse` (arg order `u, w, v`;
invert `w`; return `v * w * u.transpose()`); `pinverse` → `return svd_inverse( m );`.
`pinv` untouched.

**3.4** Green: `./test_test "[pinv]"` all pass; E06 probe section green (boundary +
supplement printed; note: the supplement's wide-2×4 MP values remain large — that is the
documented D4 gap, NOT a failure of this task). Commit.

## Task 4 — P2 LU pivoting

**4.1** Create `tests/cases/lu_pivoting.hpp` stage A (tag `[lu_pivoting]`):

- In-file legacy oracle: byte-verbatim copy of the pre-fix `lu_decomposition` +
  `forward_substitution` + `backward_substitution` (from baseline `5c8fad9`; static,
  file-local; comment cites "pre-pivoting oracle — E09").
- Case 1 (E09): 6×6 SPD system (A, b from the probe; exact x = (1..6)):
  `x_lib = feng::lu_solver(A, b)` (optional overload); REQUIRE has_value;
  `‖x_lib − x_legacy‖∞ < 1e-9`; `‖x_lib − x_exact‖∞ < 1e-9`.
- Case 2 (rescue): `A = [[0,1,2],[1,0,3],[4,5,6]]`, `b = (3,4,15)`:
  REQUIRE `sol.has_value()` (RED pre-fix); `‖x − (1,1,1)‖∞ < 1e-9`.
- Case 3 (singular pin): `[[1,2],[2,4]]`, `b = (3,4)`: REQUIRE `!sol.has_value()`.

Register in `tests/test.cc` after `lround.hpp` (before `mean.hpp`… exact line: after the
`lround.hpp` include): `#include "./cases/lu_pivoting.hpp"`.

**4.2** Red: `make test 2>&1 | tail -3 && ./test_test "[lu_pivoting]" 2>&1 | tail -10`
→ case 2 FAILs (nullopt pre-fix); cases 1, 3 pass (E09 trivially; singular already fails).
Log to `.work/evidence/t3_red.log`.

**4.3** Fix `matrix.hpp` (~6585–6655) per design.md:
- 5-arg primary (pivoting on working copy `M`; `argmax` with first-max tie; `swap_ranges`
  on `M` rows; `L[j][k] ↔ L[p][k]` for `k < j`; `perm`; `sign`); keep `better_assert`,
  keep the `isinf/isnan → return 1` guard.
- 3-arg overload delegates: `int sign{}; std::vector<std::uint_least64_t> perm;
  return lu_decomposition( A, L, U, sign, perm );`
- `lu_solver(A, x, b)`: take `sign`/`perm` from the 5-arg call; build `Pb[i][0] =
  b[perm[i]][0]`; forward-substitute `L Y = Pb`; backward `U x = Y`. Signatures unchanged.
- Stage B: append the 5-arg API cases to the same file (3×3 perm/sign/PA==LU; `[[0,1],[1,0]]`;
  identity no-op) — green on first run (documented: API did not exist pre-fix).

**4.4** Green: `./test_test "[lu_pivoting]"` all pass; E09 probe green; `make example
2>&1 | tail -5` exit 0 (0019 MAE unchanged or ~1e-15-shifted — record). Commit.

## Task 5 — C5 det

**5.1** Create `tests/cases/det.hpp` (tag `[det]`; spec `determinant.md`):

1. 1×1 `{0}` → `== 0.0`; 1×1 `{5}` → `== 5`.
2. 2×2 `[[0,1],[1,0]]` → `−1` (1e-12).
3. 2×2 `[[2,1],[1,2]]` → `3` (1e-12).
4. Singular block 4×4 (review matrix) → `== 0.0` exactly (E07; RED pre-fix: `−nan`).
5. 4×4 integer matrix (det 51) vs in-file Bareiss (same fraction-free 3×3… 4×4 loop as in
   the probe; comment cites "independent reference — E07"): within 1e-9 relative.
6. `diag(1, 1e-14)` → `!= 0` and `|det − 1e-14| < 1e-28` (P7).
7. 3×3 `[[0,1,2],[1,0,3],[4,5,6]]` → `16` (1e-9).

Register in `tests/test.cc` after `cos.hpp` (before `erfc.hpp`): `#include "./cases/det.hpp"`.

**5.2** Red: `make test 2>&1 | tail -3 && ./test_test "[det]" 2>&1 | tail -10`
→ case 4 FAILs pre-fix (`−nan == 0.0` false). Log to `.work/evidence/t4_red.log`.

**5.3** Fix `crtp_det::det` (~2055–2085) per design.md: LU product; `if ( lu_decomposition(
zen, L, U, sign, perm ) ) return value_type{};`; loop `U[i][i] == 0 → return value_type{}`;
`product` initialized to `sign`; message typo "row and col"; drop `noexcept`.

**5.4** Green: `./test_test "[det]"` all pass; E07 probe green (both checks, exact 0 + 51);
`make example 2>&1 | tail -3` exit 0 (0005 stdout identical — record). Commit.

## Task 6 — C6 operator^

**6.1** Create `tests/cases/matrix_power.hpp` (tag `[matrix_power]`; spec
`matrix_power.md`): 2×2 `{1,1,0,1}` closed form for n = 0,1,2,3,4,5 (exact values); 3×3
fixed `{1,2,3,4,5,6,7,8,10}`: `m^3`, `m^5` vs in-file loop-product oracle (1e-9); 1×1
`{2}`^5 = 32. Register in `tests/test.cc` after `lu_pivoting.hpp` (before `mean.hpp`):
`#include "./cases/matrix_power.hpp"`.

**6.2** Red (documented variant): `make test 2>&1 | tee .work/evidence/t5_red.log | head -30`
→ **compile failure**: instantiating `m ^ 3` from the new case pulls the ill-formed odd
branch (`uint_least64_t * matrix`, `matrix.hpp:5653`) — same class as
`.work/evidence/prefix_E08_compile_error.log`. The red is the compile log; no runtime
state exists pre-fix.

**6.3** Fix `matrix.hpp` (~5653): odd branch →
```cpp
if ( n & 1 )
{
    auto const half = lhs ^ ( n >> 1 );
    return half * half * lhs;
}
```

**6.4** Green: `make test 2>&1 | tail -3 && ./test_test "[matrix_power]" 2>&1 | tail -8`
→ compiles, all pass; `g++ -std=c++20 -DPARALLEL -O1 -o .work/probe_e08
.work/probes/E08.cc && .work/probe_e08` → `PASS E08`. Commit.

## Task 7 — Full checks + audit

```sh
make test 2>&1 | tail -3 | tee .work/evidence/final_suite_run.log   # expect 69 cases, exit 0
./test_test 2>&1 | tail -3
make example 2>&1 | tail -5; echo "example exit: $?"
diff <(grep -v '^running' .work/evidence/baseline_example.log) <(grep -v '^running' <(make example 2>&1)) | head -30
git diff --name-only 5c8fad9 | tee .work/evidence/final_diff_names.log
g++ -std=c++20 -DPARALLEL -O1 -o .work/probe_s3 .work/probes/E05_E09.cc && .work/probe_s3 > .work/evidence/final_probes.log 2>&1; echo "probe exit: $?"
g++ -std=c++20 -DPARALLEL -O1 -o .work/probe_e08 .work/probes/E08.cc && .work/probe_e08 | tee -a .work/evidence/final_probes.log
```

Audit gates: suite 69/69; example exit 0 with audited stdout delta; `git diff --name-only
5c8fad9` ⊆ {`matrix.hpp`, `tests/test.cc`, the five `tests/cases/*` files, `.work/**`,
`docs/session_3/**`, `docs/eval_seed_cases.md`, `docs/risk_register.md`}; `matrix.hpp`
hunks confined to the five regions (verify with `git diff 5c8fad9 -- matrix.hpp | grep
'^@@'`).

## Task 8 — Sharded review

Per `docs/prompts/sharded_review.md`. Fresh context; inputs: contract + `git diff 5c8fad9`
+ evidence logs only. Shards: (1) C3+flip tests, (2) pinv core+tests, (3) LU pivoting
algorithm (L-row-swap propagation, perm direction, sign) + lu_solver, (4) det rewrite +
tests, (5) operator^ + tests + test.cc/suite-safety. Output:
`docs/session_3/sharded_review.md`. Fix High/Critical only; re-run affected checks after
each fix.

## Task 9 — Adversarial verification

Per `docs/prompts/adversarial_verifier.md`. Fresh context; inputs: contract + diff +
evidence only. Attack list: the five fixes (bug-restoration reasoning), the D4 narrowing
(no wide-MP assertion; boundary 1e-10 semantics), suite-safety (no assert-abort case),
example invariance, blast-radius containment. Output:
`docs/session_3/adversarial_verification.md`.

## Task 10 — Closeout

Done-condition table (contract acceptance criteria × evidence), eval-seed promotion
(E05–E09 → `promoted`, E06 footnote), risk-register entries,
`.work/handoff_session_3.md`, closeout commit:

**Commit:** `S3 closeout: sharded review + adversarial verification + full re-checks
(69 cases) + E05–E09 promoted + risk register + handoff (wide-SVD gap → S6 candidate)`
