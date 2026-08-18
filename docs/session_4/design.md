# Session 4 — Design

Exact per-capability design: code, hand-derived expected values, edge-case analysis, and the
fast-math (R-19) test policy. All pre-fix facts below are probe-verified (`.work/evidence/prefix_p0.log`).

## 1. `stat-promotion` (C8)

### Type-class analysis (verified)

- `matrix::size_type` = `std::uint_least64_t` (`matrix.hpp:1320`) → `int_sum / size_type`
  undergoes integer conversion (int rank < uint64 rank) → **unsigned integer division**.
  Pre-fix `mean(matrix<int>{1,2})` = `3 / 2` in `unsigned long` = `1` (probe p0). Negative
  int sums would wrap to huge unsigned values — strictly worse than the review's description.
- `operator-( const matrix< T, A >&, const T& )` (`matrix.hpp:5534`) requires the scalar to be
  **exactly** `T`. With `mean` returning `double`, `m - mean(m)` on an `int` matrix would
  truncate the mean to `int` — silently wrong variance/std. Hence promotion **before** the
  formula, not just a cast of the result.
- `astype<T>()` member template (`matrix.hpp:3976`) returns `matrix<T, rebind>` by value.
- `pow( Mat, std::floating_point auto )` (`matrix.hpp:7436`) exists for any `Matrix` →
  `pow(matrix<double>, 2.0)` → `matrix<double>` ✓.

### Final code — uniform type-class dispatcher (all three keep their `constexpr auto … requires Matrix< Mat >` declarations)

One structural rule across all three functions (functional-thinking: the type class is the
input domain; the dispatcher is explicit, and the legacy complex path is visibly isolated):

```cpp
// mean
    if constexpr ( std::is_complex_v< typename Mat::value_type > )
        return sum( m ) / m.size();   // legacy, verbatim: complex / size_t -> complex
    else
    {
        if constexpr ( std::is_same_v< typename Mat::value_type, double > )
            return sum( m ) / m.size();
        else
        {
            // integer/float matrices: promote before dividing. `sum / size` on an integer
            // sum is unsigned integer division (truncating; negative sums wrap), and the
            // variance/std expressions below need a double mean (operator-(matrix<T>, T)
            // would otherwise truncate it). double matrices stay copy-free.
            auto const d = m.astype< double >();
            return sum( d ) / d.size();
        }
    }
```

`variance` and `standard_deviation` follow the identical pattern with the **unchanged**
formula in each branch:

```cpp
// variance
    if constexpr ( std::is_complex_v< typename Mat::value_type > )
        return mean( pow( m-mean( m ), 2.0 ) );   // legacy, verbatim (see complex note below)
    else
    {
        if constexpr ( std::is_same_v< typename Mat::value_type, double > )
            return mean( pow( m-mean( m ), 2.0 ) );
        else
        {
            auto const d = m.astype< double >();
            return mean( pow( d - mean( d ), 2.0 ) );
        }
    }

// standard_deviation
    if constexpr ( std::is_complex_v< typename Mat::value_type > )
    {
        if ( m.size() <= 1 )
            return typename Mat::value_type{};    // legacy, verbatim (complex-consistent deduction)
        return std::sqrt( sum( pow( m-mean( m ), 2.0 ) ) / ( m.size() - 1 ) );
    }
    else
    {
        if ( m.size() <= 1 )
            return double{};                      // was value_type{} — now double for all real types
        if constexpr ( std::is_same_v< typename Mat::value_type, double > )
            return std::sqrt( sum( pow( m-mean( m ), 2.0 ) ) / ( m.size() - 1 ) );
        else
        {
            auto const d = m.astype< double >();
            return std::sqrt( sum( pow( d - mean( d ), 2.0 ) ) / ( d.size() - 1 ) );
        }
    }
```

Notes:
- **`auto` deduction consistency forces the `double{}` in the real `size≤1` branch.**
  `auto` return deduction requires every return statement to deduce one type (compile-time,
  not runtime): with the promoted real path, the `sqrt` branch deduces `double`, so a
  `value_type{}` (e.g. `int{}`) size≤1 return would be ill-formed. The complex branch keeps
  `value_type{}` (there both branches deduce `complex<T>` — the legacy expression is
  verbatim, so complex `standard_deviation` has exactly the pre-fix (non-)compilability;
  see below).
- `n−1` sample formula kept (D-C8-3). Population `n` for `variance` kept.
- **Complex reality (verified by code reading):** `operator-( matrix<complex<T>>, const T& )`
  (`matrix.hpp:5461`) takes the *real* element type `T`, not `complex<T>` — so
  `m - mean(m)` for a complex matrix is ill-formed **pre-fix already**: complex
  `variance`/`standard_deviation` were hard compile errors before this session (same root
  cause as the int case). "Legacy preserved" for them = preserved compile error; complex
  `mean` keeps compiling and keeps returning `complex` (E10 mandates `double` for integer,
  float, and double value types only). No in-repo complex callers of any of the three.
- `constexpr` specifier kept: decorative pre- AND post-fix (the body already calls
  non-constexpr `reduce`; GCC 16 accepts constexpr templates without a qualifying
  instantiation — the pre-fix header proves it). Fallback if the post-fix compile disagreed:
  drop `constexpr` (decorative removal, no behavior change) — will be noted if needed.

### Hand-derived expected values (exact in binary where marked)

| Matrix | mean | variance | stddev (n−1) |
|---|---|---|---|
| int 1×2 `{1,2}` | `3/2 = 1.5` (exact) | `((−0.5)²+0.5²)/2 = 0.25` (exact) | `√(0.5/1) = √0.5 = 0.7071067811865476` |
| int 2×2 `{1,2;1,2}` | `6/4 = 1.5` (exact) | `4×0.25/4 = 0.25` (exact) | `√(1.0/3) = 0.5773502691896257` |
| int 1×1 `{7}` | `7.0` (exact) | `0.0` (exact) | `0.0` (size≤1 branch) |
| float 1×2 `{1,2}` | `1.5` | `0.25` | `√0.5` (via double promotion) |
| double 1×2 `{1,2, ...}` | unchanged from pre-fix (probe p0: 1.5/0.25/0.707107) | | |

## 2. `conv-same-kernel` (C9)

### The fix (exactly two lines, `matrix.hpp:6754-6755`)

```cpp
better_assert( rb >= 1, " For a convolution in 'same' mode, the row of the second matrix is at least 1, but now has ", rb );
better_assert( cb >= 1, " For a convolution in 'same' mode, the column of the second matrix is at least 1, but now has ", cb );
```

(second assert: condition `rb > 1` → `cb >= 1`; first: `rb > 1` → `rb >= 1`; messages
unchanged — they were already per-axis correct.)

### Why 1×1 is well-defined in the existing arithmetic

- full conv with 1×1 kernel: the padding/sum path is the review-verified "full" path; result
  is elementwise `A·k` (each output position has exactly one non-zero overlap).
- "same" slice: rows `{(rb−1)>>1, ra + (rb−1)>>1}` = `{0, ra}` and cols `{0, ca}` for
  `rb = cb = 1` → the entire full conv. No `rb−1` underflow (rb ≥ 1), no `>>1` issue.
- The `A.size() > B.size()` swap inside the full conv still applies and is unaffected.

### Hand-derived expected values (full-conv trace; cross-checked vs NumPy `convolve(...,'same')`)

Kernel swap note: when `A.size() > B.size()` the full path swaps, so both kernels below are
hand-traced through the swapped padding, and the final slices were independently
cross-checked against NumPy's `'same'` centering `(cb−1)//2 .. (cb−1)//2+ra` (identical for
these shapes).

- **(a) E11:** `A = [[1,2],[3,4]]`, `B = [[0.5]]` → `[[0.5,1],[1.5,2]]`.
- **(b) rb==1, cb==2:** `A = [[1,2,3],[4,5,6]]`, `B = [[1,1]]` (sum filter) →
  `[[1,3,5],[4,9,11]]`.
  Trace: swap → A'=1×2 `{1,1}`, B'=2×3 (A). Padded B' 2×5: `{0,1,2,3,0; 0,4,5,6,0}`
  (B' copied at col offset A'.col()−1 = 1). Full 2×4: row0 = `{1,3,5,3}`, row1 =
  `{4,9,11,6}`. same slice: rows `{0,2}`, cols `{(2−1)>>1=0, 3+(2−1)>>1=3}` →
  `[[1,3,5],[4,9,11]]` ✓ (NumPy: `convolve([1,2,3],[1,1],'same') = [1,3,5]` ✓).
- **(c) rb==2, cb==1:** `A = [[1,2],[3,4],[5,6]]`, `B = [[1],[1]]` → `[[1,2],[4,6],[8,10]]`.
  Trace: swap → A'=2×1, B'=3×2 (A). Padded B' 5×2: `{0,0;1,2;3,4;5,6;0,0}` (row offset 1).
  Full 4×2: `{1,2; 4,6; 8,10; 5,6}`. same slice: rows `{(2−1)>>1=0, 3+0=3}`, cols
  `{0, 2+0=2}` → `[[1,2],[4,6],[8,10]]` ✓ (NumPy column-wise: `convolve([1,3,5],[1,1],'same') = [1,4,8]` ✓).
- **(d) valid regression:** `A` = 4×5 row-major `1..20`, `B` = 2×3 with `B[0][0]=0.5`, rest
  0 → valid slice = `0.5·A[0:3, 0:3]` = `[[0.5,1,1.5],[3,3.5,4],[5.5,6,6.5]]`.
  (Pinned so an accidental edit to the valid branch fails loudly; the valid assert lines are
  untouched by this task.)

## 3. `rref-domain` (C10)

### The fix (exactly one line, `matrix.hpp:6486-6488`)

```cpp
better_assert( row > 0 && col > 0 &&
    "matrix must have at least one row and one column to execute a Gauss-Jordan Elimination",
    row, col );
```

House style `cond && "msg"` kept; the `row, col` variadic payload kept; the pre-existing
message typos ("colum", "execut") are corrected because the message is rewritten (S3 D7
precedent: fix typos in the line you touch).

### Algorithm-domain analysis (why the body is untouched)

`gauss_jordan_elimination` (matrix.hpp:6483-6515) loops `i` over `range(row)` and, per
`i`, scans `col_begin(i)` — a **strided** iterator (element `i`, stride `col`). The scan is
valid only for `i < col`:

- `row < col` (original domain): every `i` is a valid column → no OOB. (Example 0020:
  64×128 — this case.)
- `row == col` (new): all `i < col` → no OOB. **Square systems are exactly safe.**
- `row > col` (new, exposed): for `i ≥ col` the strided read runs past the end —
  **pre-existing UB, already reachable in release** (probe p3: ASan heap-buffer-overflow
  READ pre-fix under NDEBUG). The relaxation neither causes nor worsens it; per the
  contract's out-of-scope clause ("does not change any other gauss_jordan line") it is
  documented + ASan-pinned (before/after pair must be identical), not repaired.

### Hand-derived expected values

- **(a) E12:** `diag{2,3}` → RREF `I`. Trace: i=0 pivot col 0: candidates a[0][0]=2, a[1][0]=0
  → p=0, no swap; factor=2; row0 = {1,0}; eliminate row1: a[1][0]−a[0][0]·0 → 0. i=1: pivot
  a[1][1]=3 → row1 = {0,1}. Result `I` ✓.
- **(b) singular square** `{{1,2},{2,4}}`: i=0: pivot col 0: |1| vs |2| → p=1, swap →
  `{2,4;1,2}`, factor=2, row0={1,2}; eliminate: row1 = {1−1·1, 2−1·2} = {0,0}. i=1: pivot
  a[1][1]=0 → `std::abs(factor) < 1e-10` → `return {}` (nullopt) ✓ — the existing finite
  guard fires **before any division**; no hang, no NaN (R-19-safe: the guard compares
  finite values, survives fast-math).
- **(c) wide regression** `{{1,0,2},{0,1,3}}` → already in RREF → unchanged `{1,0,2;0,1,3}`.

### Pre-existing UB watch item (for the risk register + handoff)

`rref`/`gauss_jordan_elimination` on `row > col` reads OOB (strided `col_begin(i)` for
`i ≥ col`). Pre-existing (p3). S4 documents; repair is a future-session decision (needs an
algorithm-body change: clamp the pivot scan to `min(row, col)` or switch the outer loop to
columns — both are behavior decisions outside S4).

## 4. `cholesky-guard` (P2b)

### The fix (`matrix.hpp:5766-5784`)

```cpp
template < typename Matrix1, typename Matrix2 >
bool cholesky_decomposition( const Matrix1& m, Matrix2& a )
{
    typedef typename Matrix1::value_type value_type;
    better_assert( m.row() == m.col() );
    a                   = m;
    const std::uint_least64_t n = m.row();

    for ( std::uint_least64_t i = 0; i < n; ++i )
        for ( std::uint_least64_t j = i; j < n; ++j )
        {
            const value_type sum = a[i][j] - std::inner_product( a.row_begin( i ), a.row_begin( i ) + i, a.row_begin( j ), value_type( 0 ) );
            if ( i == j )
            {
                // positive-definiteness guard: the diagonal step must be strictly
                // positive, else the sqrt below is of a non-positive (real) value and
                // the factor silently contains NaN. complex value_type has no
                // ordering — legacy path preserved (no in-repo complex callers).
                if constexpr ( ! std::is_complex_v< value_type > )
                    if ( sum <= value_type( 0 ) )
                        return false;
                a[i][i] = std::sqrt( sum );
            }
            else
                a[j][i] = sum / a[i][i];
        }

    for ( std::uint_least64_t i = 1; i < n; ++i )
        std::fill( a.upper_diag_begin( i ), a.upper_diag_end( i ), value_type() );
    return true;
}
```

### Edge-case table (verified by the E13 probe post-fix)

| Input | Trace | Result |
|---|---|---|
| `[[1,2],[2,1]]` (non-PD, eig −1,3) | i=0: a[0][0]=1, a[1][0]=2; i=1: sum = 1 − 2² = −3 ≤ 0 | `false`; `a = [[1,0],[2,0]]` defined, **no NaN** (guard fires before the sqrt) |
| `[[4,2],[2,3]]` (PD) | a[0][0]=2; a[1][0]=1; sum = 3−1 = 2 > 0 → a[1][1]=√2 | `true`; `a = [[2,0],[1,√2]]`; `a·aᵀ = [[4,2],[2,3]]` ✓ |
| `[[1,1],[1,1]]` (PSD singular) | i=1: sum = 1 − 1 = 0 ≤ 0 | `false` (C-11 boundary: `<=`, not `<`) |
| `1×1 {0}` | sum = 0 ≤ 0 | `false` (adversarial case) |
| `1×1 {4}` | sum = 4 > 0 → a[0][0]=2 | `true`, `a[0][0] == 2` (adversarial case) |

The off-diagonal divide-by-`a[i][i]` cannot reach a zero denominator: `a[i][i] = sqrt(sum)`
with `sum > 0` is strictly positive (real path); a zero `a[i][i]` is only reachable if the
diagonal step was 0 — which already returned false. The PRD's "keep the `a[i][i] == 0`
check" refers to a check absent from the current source (review misreading, Q5); the guard
subsumes it.

## 5. Test and probe policy (R-19 fast-math)

- **Suite (`-Ofast`, fast-math):** tolerances only (`1e-12` for exact-binary values, `1e-10`
  for RREF/Cholesky products), finite values only, no NaN-dependent assertions (the cholesky
  non-PD path provably produces no NaN post-fix; the singular-rref path exits before any
  division). `static_assert` on return types (compile-time, fast-math-proof).
- **Deterministic probe (E10_E13, `-O1`, asserts live):** exact comparisons
  (`== std::sqrt(0.5)`, `== 2.0`, abort-liveness for E11/E12) — same header + same IEEE ops
  at `-O1` (no fast-math) → deterministic. Built exactly per the contract's deterministic
  check command.
- **row>col ASan pair:** pre-fix log already captured (`prefix_p0.log` p3); the post-fix
  rerun must show the identical ASan report (same read, same frame in
  `gauss_jordan_elimination`) — proves "identical to pre-relaxation" for the release path.
  The row>col case is **not** in the suite (UB — running it in an ASan-less `-Ofast` suite
  is not a test, it is a hazard).
