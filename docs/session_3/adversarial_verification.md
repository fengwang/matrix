# Session 3 — Adversarial Verification

- Verified state: commit `e2ac38d` (post sharded review; no High/Critical fixes required).
- Method: a fresh probe (`.work/probes/adversarial.cc`, built `-O1`, IEEE math) constructed
  **independent of the test files** — new matrices and references that do not appear in
  `tests/cases/`. Evidence: `.work/evidence/adversarial.log` (12/12 PASS, exit 0).
- The probe intentionally re-derives each contract claim with different data than the suite.

## AV1 — flip aliases (C3)

Fresh 3×4 content matrix (1..12 row-major):
- `fliplr` content: row0 → 4 3 2 1, row2 → 12 11 10 9. PASS.
- `flipud` content: row0 ← 9 10 11 12, row2 ← 1 2 3 4. PASS.
- `fliplr(B) == flipdim(B, 2)` and `flipud(B) == flipdim(B, 1)` bitwise. PASS.

## AV2 — pinv (C4/R1)

Fresh 5×3 tall full-column-rank matrix. Reference: the normal-equations formula
`(AᵀA)⁻¹Aᵀ` computed through `inverse()` (a different code path than SVD):
- `||pinv(A) − (AᵀA)⁻¹Aᵀ||∞ = 1.59e-12` (< 1e-6). PASS.
- Moore–Penrose residuals: `||Apa − A||∞ = 3.55e-14`, `||pap − p||∞ = 8.88e-16`,
  symmetry of `A·p` = 4.89e-15. PASS.

## AV3 — det (C5/P7)

Fresh 5×5 matrices:
- Lower triangular with diagonal (2,3,4,5,6): `det = 720` exactly (720 = 2·3·4·5·6,
  printed as `720`). PASS.
- Random-ish integer 5×5: library `det = −9183.9999999999982` vs independent in-probe
  Bareiss (exact fraction-free) reference `−9184` (relative diff ~2e-13 < 1e-9). PASS.

## AV4 — operator^ (C6)

Fresh 4×4 matrix:
- `E^7` vs a 7-step multiplication loop: `d = 3.55e-15`. PASS (odd exponent — the branch
  fixed in this session).
- `E^0` bitwise-equal to the 4×4 identity. PASS.

## AV5 — LU pivoting (P2)

Fresh 7×7 matrix (non-diagonally-dominant in places, forces swaps):
- `lu_decomposition` rc = 0; `perm` is a valid permutation; `sign ∈ {±1}`. PASS.
- `||P·A − L·U||∞ < 1e-9` with `P` built explicitly from `perm` (PA = LU). PASS.
- `lu_solver` on `b := A·x0`, `x0 = (1..7)`: `||x − x0||∞ = 8.88e-16`,
  residual `||A·x − b||∞ = 1.42e-14`. PASS.

## Notes

- One probe self-bug during this pass (not a library issue): AV1's hand-derived expectation
  mis-computed one flipped entry (index 1 of the flipped last row is 11, not 10); the
  alias-equivalence check had already confirmed the library behavior. Fixed in the probe;
  library untouched.
- `inverse()` returns `matrix` directly (not `optional`) — confirmed during this pass;
  the adversarial reference used it as such.

## Conclusion

All five contract claim areas (C3, C4/R1, C5/P7, C6, P2) hold on fresh independent inputs.
No new defects found. Proceed to closeout.
