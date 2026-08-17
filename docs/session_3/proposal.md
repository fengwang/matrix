# Session 3 — Proposal

Concise extraction from the brainstorming (no re-exploration). Authority:
`docs/session_3_contract.yaml`.

## Motivation

The 2026-07-13 sharded review found five numerical-contract violations in `matrix.hpp`
(`docs/opencode_sharded_review.md`, C3/C4/C5/C6/R1/P2). Pre-flight reproduction this turn
(`.work/evidence/prefix_probes.log`, `.work/evidence/prefix_E08_compile_error.log`) confirmed
every claim and added one finding the review did not list:

- **C3** — `fliplr`/`flipud` are swapped: `fliplr` calls `flipdim(m, 1)` (up-down) and `flipud`
  calls `flipdim(m, 2)` (left-right). Probe: both aliases return the *other* flip (E05 FAIL).
- **C4 + R1** — `pinverse`/`pinv` do not invert the singular values: `pinverse` computes
  `v * w * u.transpose()` without touching `w` — `pinv(diag(1,2))` returns `diag(1,2)` (E06
  FAIL); `svd_inverse` calls the SVD with `u, v, w` against the `(A, u, w, v)` signature —
  numerically accidentally correct (swapped names compute `V·Σ⁺·Uᵀ`), formally wrong (R1).
- **C5 + P2** — `det()` is Schur-complement recursion (`P.det() * (S − R·P⁻¹·Q).det()`) with an
  unguarded `P.inverse()`: the review's singular block matrix returns **`−nan`** (E07 FAIL), and
  `lu_decomposition` has no pivoting (a zero first pivot yields `L[i][j] = x/0 = inf/nan` →
  failure even when a row swap would rescue the matrix).
- **C6** — `operator^`'s odd branch `return lhs ^ (n - 1) * lhs;` is parsed as
  `lhs ^ ((n-1) * lhs)` (operator precedence): `uint_least64_t * matrix` has no matching
  `operator*` → **hard compile error for every n** (E08 pre-fix: `matrix.hpp:5653`, recorded).
- **P2** — `lu_decomposition` (Doolittle) has no partial pivoting and exposes no permutation
  or sign; `lu_solver` cannot apply P to b.
- **New finding (this turn)** — the SVD core is numerically invalid for wide matrices (m < n):
  2×4 SVD reconstruction error 6.0, `u` entries ~1e306; tall/square are valid (2.2e-16).
  Out of S3 scope (SVD's public behavior) — drives the documented narrowing D4 (user-confirmed).

Sanctioned behavior changes: PRD §5 row 3 (`fliplr = flipdim(m, 2)`, `flipud = flipdim(m, 1)`),
row 4 (`pinv` = Moore–Penrose via SVD, zero singular values excluded), row 7
(`lu_decomposition` gains partial pivoting; solutions unchanged, factors differ).

## Specific changes agreed

1. **`matrix.hpp`** — five targeted edits, all within sanctioned regions:
   - `fliplr`/`flipud` bodies (~4577/4582): swap the `flipdim` dimension arguments (2 lines).
   - `svd_inverse` body (~5302): correct argument order `singular_value_decomposition(a, u, w, v)`,
     invert `w` in place (threshold `> 1e-10` inherited), return `v * w * u.transpose()`;
     `pinverse` body (~5312): one-line delegation `return svd_inverse( m );` (R1 + C4, single core).
   - `crtp_det::det` (~2048–2086): rewrite as pivoted-LU product `sign · ∏ U[i][i]` with
     exact-zero-pivot → `0` (no epsilon, P7); `size == 0 → 0` preserved; assert message typo
     fixed; `noexcept` dropped (allocation now in the body — project contract §3).
   - `operator^` odd branch (~5653): `auto const half = lhs ^ (n >> 1); return half * half * lhs;`
   - `lu_decomposition` (~6585): new 5-arg primary with partial pivoting + `sign`/`perm`
     outputs (`P·A = L·U`); existing 3-arg overload delegates; `lu_solver` (~6627) applies P to b
     via `perm` before forward substitution. Signatures of the 1-arg tuple overloads and both
     `lu_solver` overloads unchanged; `forward_substitution`/`backward_substitution` unchanged.
2. **`tests/test.cc`** — five include lines (alphabetical; exact positions in `plan.md`).
3. **`tests/cases/`** — five new case files, content-asserting, each red pre-fix
   (`flip_aliases.hpp`, `pinv.hpp`, `det.hpp`, `matrix_power.hpp`, `lu_pivoting.hpp`).
4. **`.work/probes/`** — `E05_E09.cc` (E05/E06/E07/E09 + SVD supplement) and `E08.cc` (separate
   TU — pre-fix it is a compile error); pre-fix and post-fix runs recorded in `.work/evidence/`.
5. **`docs/eval_seed_cases.md`** — E05–E09 `seeded` → `promoted`; E06 footnote for the D4
   narrowing (2×4 wide gap → S6 candidate).
6. **`docs/risk_register.md`** — S3 watch items (wide-SVD limitation; `svd_inverse`/`pinverse`
   name retirement for S6; det `noexcept` drop; LU sign/perm API surface).
7. **`.work/handoff_session_3.md`** — decision log, S6 doc deltas (ReadMe `det` section ~line 765
   describes the old Schur behavior — S6 territory, listed here for handoff), warnings.

## Capabilities

### Modified capabilities

- **`flip_aliases`** — `fliplr`/`flipud` return the MATLAB/NumPy flips of any matrix (C3).
- **`pseudoinverse`** — `pinv`/`pinverse`/`svd_inverse` share one correct SVD-inversion core:
  Moore–Penrose pseudoinverse with the inherited 1e-10 singular-value threshold (C4, R1).
- **`determinant`** — `det()` via pivoted LU product; exact-zero pivot → exactly `0`, never
  NaN; no epsilon (C5, P7).
- **`matrix_power`** — `operator^` computes the exact integer power for all `n ≥ 0`, square
  matrices (C6).
- **`lu_pivoting`** — `lu_decomposition` performs partial pivoting and exposes permutation +
  sign; `lu_solver` applies P to b; solutions invariant (P2).
