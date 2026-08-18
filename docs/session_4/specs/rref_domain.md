# Spec — `rref-domain` (C10): `rref`/`gauss_jordan_elimination` accept square (and any non-empty) systems

## Requirement
`feng::gauss_jordan_elimination` (matrix.hpp:6483, aliased `rref` at 6517) accepts any
non-empty matrix: the precondition relaxes from `row < col` to `row > 0 && col > 0`.

## Constraints
- Exactly one line changes: the `better_assert` at matrix.hpp:6486 (condition + rewritten
  message; house style `cond && "msg"` and the `row, col` variadic payload kept).
- **No algorithm-body change** (contract: "the fix does not change any other gauss_jordan
  line"). In particular the pre-existing `row > col` strided-read OOB (`col_begin(i)` for
  `i ≥ col`) is documented and ASan-pinned, **not repaired** (probe p3: heap-buffer-overflow
  READ pre-fix, NDEBUG+ASan — already reachable in release).
- Return type `std::optional<Mat>` unchanged; the `1e-10` singular-pivot exit unchanged
  (finite comparison — R-19 fast-math safe).

## Acceptance (from contract)
1. E12: `rref(diag{2,3})` → option with value `≈ I`, no abort in a debug build
   (pre-fix: SIGABRT at matrix.hpp:6486, probe p2).
2. Square singular `{{1,2},{2,4}}` → `nullopt` via the 1e-10 pivot exit, no hang, no abort.
3. Wide regression (originally supported `row < col` case): `{{1,0,2},{0,1,3}}` → itself.
4. `row > col` (3×2): release (NDEBUG+ASan) behavior **identical to pre-relaxation** —
   post-fix ASan report matches the pre-fix one byte-for-byte in substance (same OOB read,
   same frame). Not a suite test (UB).
5. `make test` (new `tests/cases/rref.hpp`) passes; example 0020 (64×128 wide) output
   unchanged.

## Out of scope
The `row > col` algorithm OOB (watch item for a future session — an algorithm-body decision),
the optional-return type, the elimination algorithm, the 1e-10 threshold value.
