# Session 3 — Design

Approach, algorithm decisions, and code sketches. Decisions D1–D12 are defined in
`brainstorming.md`; this file carries the *how*. Baseline: `5c8fad9` (S2 closeout review
fixes) on branch `phase-1/session-3` (the diff-audit reference is unambiguous:
**baseline commit `5c8fad9`**).

## ACD classification (functional-thinking skill)

- **C3 (flip aliases):** two *Actions* (aliases) fixed to call the right *Calculation*
  (`flipdim` — S1 territory, untouched).
- **C4/R1 (pinv):** one *Calculation* (SVD-inversion core = `svd_inverse`); two *Actions*
  (`pinverse`, `pinv`) delegate to it. The core's state is explicit: `u, w, v` out-params of
  the SVD, `w` mutated in place (the only mutation in the core — the Σ⁺ construction), one
  return value.
- **C5 (det):** one *Calculation* (LU product), no special-case branches beyond the
  size-0 preservation branch. The Schur recursion's `P.inverse()` (a second, unguarded
  Calculation feeding a third) is deleted.
- **C6 (power):** one *Calculation* (log₂ recursion); the even/odd split stays (it is the
  recursion), the odd branch becomes an explicit two-multiply composition with no operator
  precedence surface.
- **P2 (LU):** one *Calculation* (`lu_decomposition` 5-arg) with an explicit permutation
  channel (`perm`, `sign`); `lu_solver` *Action* consumes the channel (`Pb = P·b`), so the
  solver's data flow is explicit: `A →(LU+P)→ (L, U, perm) → Pb → Y → x`.

All five changes are body-only inside existing sanctioned regions; no new public types, no
new headers, no new dependencies (project contract §2: no production dependencies).

## Approaches considered (P2 pivoting — the only multi-option decision)

1. **Chosen (D5/D6): 5-arg primary + delegating 3-arg overload, `P·A = L·U` on a working copy.**
   Pros: source-compatible (every existing call keeps compiling — verified: only callers are
   `examples/cases/0019_lu_decomposition.hpp` via the 1-arg overloads and the in-file
   `lu_solver`); permutation + sign are explicit values (testable, no reconstruction);
   `A` stays read-only. Cons: one working copy allocation (already paid by `L`/`U` today).
2. **Rejected: permutation vector only, `sign` derived.** `sign = (−1)^swaps` is not
   recoverable from `perm` without recomputing the swap count (possible, but it turns a
   free byproduct into a hidden Calculation and the contract names both outputs).
3. **Rejected: in-place `A` mutation (classic LAPACK-style LU).** Breaks the `A const&`
   parameter contract and the 1-arg overload's "A stays as given" behavior; examples print `A`
   after the call (0019).

## Pivoted LU algorithm (the 5-arg primary)

```cpp
template< Matrix Mat >
int lu_decomposition( Mat const& A, Mat& L, Mat& U, int& sign, std::vector< std::uint_least64_t >& perm )
{
    typedef typename Mat::value_type value_type;
    better_assert( A.row() == A.col() && "Square Matrix Requred!" );

    const std::uint_least64_t n = A.row();
    sign = 1;
    perm.resize( n );
    for ( std::uint_least64_t i = 0; i < n; ++i )
        perm[i] = i;                      // perm[i] = original row index of permuted row i

    Mat M{ A };                           // working copy (PA = LU form; A untouched)

    L.resize( n, n );
    std::fill( L.begin(), L.end(), value_type{0} );
    std::fill( L.diag_begin(), L.diag_end(), value_type( 1 ) );

    U.resize( n, n );
    std::fill( U.begin(), U.end(), value_type{0} );

    for ( std::uint_least64_t j = 0; j < n; ++j )
    {
        // Partial pivoting: max |M[i][j]| over i >= j (first max wins ties).
        std::uint_least64_t p = j;
        for ( std::uint_least64_t i = j + 1; i < n; ++i )
            if ( std::abs( M[i][j] ) > std::abs( M[p][j] ) )
                p = i;

        if ( p != j )
        {
            std::swap_ranges( M.row_begin( p ), M.row_end( p ), M.row_begin( j ) );
            for ( std::uint_least64_t k = 0; k < j; ++k )
                std::swap( L[j][k], L[p][k] );   // propagate the swap to computed L rows
            std::swap( perm[j], perm[p] );
            sign = -sign;
        }

        for ( std::uint_least64_t i = 0; i < j + 1; ++i )
            U[i][j] = M[i][j] - std::inner_product( L.row_begin( i ), L.row_begin( i ) + i, U.col_begin( j ), value_type() );

        for ( std::uint_least64_t i = j + 1; i < n; ++i )
        {
            L[i][j] = ( M[i][j] - std::inner_product( L.row_begin( i ), L.row_begin( i ) + j, U.col_begin( j ), value_type() ) ) / U[j][j];
            if ( std::isinf( L[i][j] ) || std::isnan( L[i][j] ) )
                return 1;
        }
    }

    return 0;
}
```

**Why the `L[j][k] ↔ L[p][k]` swap is required (hand-derived 3×3 check).** A =
`[[0,1,2],[1,0,3],[4,5,6]]` (true det = 16; zero first pivot → the no-pivot code returns rc=1
via `inf`, but a swap rescues it — the watchlist case). Step 0: pivot row 2 → swap M rows
0↔2, perm = [2,1,0], sign = −1. Step 1: pivot row 2 (|M[2][1]| = 1 > |M[1][1]| = 0) → swap
M rows 1↔2 **and** `L[1][0] ↔ L[2][0]` (0.25 ↔ 0), perm = [2,0,1], sign = +1. Result:
`L = [[1,0,0],[0,1,0],[0.25,−1.25,1]]`, `U = [[4,5,6],[0,1,2],[0,0,4]]`. Check `P·A = L·U`:
P·A = `[[4,5,6],[0,1,2],[1,0,3]]`; L·U rows: `(4,5,6)` ✓, `(0,1,2)` ✓,
`0.25·(4,5,6) − 1.25·(0,1,2) + (0,0,4) = (1,0,3)` ✓. Without the L-swap, row 2 of L·U is
`(1,1.25,−1)` ≠ P·A row 2. (`sign` = (+1) after two swaps; det = +1 · 4·1·4 = 16 ✓.)

**Tie-breaking:** `p` keeps the first (topmost) max — deterministic, keeps `sign` stable
across platforms. **Degenerate:** identity/upper-triangular inputs never swap (p stays j) →
`sign = +1`, `perm = identity` — pins the "no-op path" behavior.

## `lu_solver` (P to b)

```cpp
template< Matrix Mat >
int lu_solver( Mat const& A, Mat& x, Mat const& b )
{
    typedef Mat matrix_type;
    better_assert( A.row() == A.col() );
    better_assert( A.row() == b.row() );
    better_assert( b.col() == 1 );
    matrix_type L, U;
    int sign{};
    std::vector< std::uint_least64_t > perm;

    if ( lu_decomposition( A, L, U, sign, perm ) )
        return 1;

    // PA = LU  =>  L·Y = P·b =: Pb,  U·x = Y.
    matrix_type Pb;
    Pb.resize( b.row(), 1 );
    for ( std::uint_least64_t i = 0; i < b.row(); ++i )
        Pb[i][0] = b[ perm[i] ][ 0 ];

    matrix_type Y;
    if ( forward_substitution( L, Y, Pb ) )
        return 1;

    if ( backward_substitution( U, x, Y ) )
        return 1;

    return 0;
}
```

(`sign` is written by the decomposition and unused by the solver — taken as an out-param
because the 5-arg primary requires it; no warning: it is address-taken.)

## `det` (C5)

```cpp
value_type det() const            // noexcept dropped (D7: the body allocates)
{
    zen_type const& zen = static_cast< zen_type const& >( *this );
    better_assert( zen.row() == zen.col(), " matrix::det(), the row and col are supposed to be same, but now row is ", zen.row(), " and col is ", zen.col() );

    if ( 0 == zen.size() )
        return value_type{};                    // D8: preserved (0x0 -> 0)

    // PA = LU  =>  det(A) = det(P)·det(L)·det(U) = sign · ∏ U[i][i].
    zen_type L, U;
    int sign{};
    std::vector< size_type > perm;
    if ( lu_decomposition( zen, L, U, sign, perm ) )
        return value_type{};                    // zero pivot (pre-last) / inf / nan -> exactly 0

    value_type product{ sign };
    for ( size_type i = 0; i < zen.row(); ++i )
    {
        if ( 0 == U[i][i] )
            return value_type{};                // P7: exact zero pivot -> exactly 0 (no epsilon)
        product *= U[i][i];
    }
    return product;
}
```

- `lu_decomposition` resolves by ADL (all `matrix` arguments are in `feng`; the free function
  is defined later in the TU — same pattern as existing late free functions called from
  CRTP mixins).
- `product` starts at `±1` (the sign), so a single-pivot 1×1 matrix also flows through the
  same path (no fast path — D7).
- Zero-pivot matrices return `value_type{}` — **positive** zero (never `−0.0`), matching
  E07's "exactly 0" expectation.

## `operator^` (C6)

```cpp
if ( n & 1 )
{
    auto const half = lhs ^ ( n >> 1 );
    return half * half * lhs;                   // no '^' on this line: no precedence surface
}
```

`n >> 1` for odd `n ≥ 1` is `≥ 0`, so the recursion terminates at `n = 0`/`n = 1` (existing
fast paths). Complexity `O(log n)` matrix squarings, unchanged from the even branch.

## `svd_inverse` / `pinverse` (C4/R1)

```cpp
template < typename T, Allocator A>
matrix<T,A> const svd_inverse( matrix<T, A> const& a )
{
    matrix<T, A> u;
    matrix<T, A> w;
    matrix<T, A> v;
    singular_value_decomposition( a, u, w, v );                       // R1: names match the signature
    matrix_details::for_each( w.begin(), w.end(), []( auto & val ) { if ( std::abs( val ) > 1.0e-10 ) val = 1.0 / val; });
    return v * w * u.transpose();
}

template < typename Matrix >
Matrix const pinverse( const Matrix& m )
{
    return svd_inverse( m );                        // C4: single correct core
}
```

Pre-fix `pinverse` computed `V·Σ·Uᵀ` (no inversion — the C4 bug); pre-fix `svd_inverse`
computed `V·Σ⁺·Uᵀ` with swapped local names. Post-fix both return the Moore–Penrose
pseudoinverse (for m ≥ n; see D4 for the wide-matrix gap).

## Tests (D10) — one file per capability, content-asserting

| File | Cases (red pre-fix unless noted) |
|---|---|
| `flip_aliases.hpp` `[flip]` | E05 2×3 content; 3×3 ragged content; 1×3 `fliplr` == elementwise reverse; 3×1 `flipud` == row reverse; 1×1 fixed point (both). |
| `pinv.hpp` `[pinv]` | `pinv(diag(1,2)) ≈ diag(1,0.5)` (1e-8); `pinverse == pinv` (same core, exact equality); 4×2 rank-1 matrix: MP conditions (A·P·A≈A, P·A·P≈P, (A·P)′=A·P, (P·A)′=P·A) within 1e-8; 3×3 `diag(1,1,0)` → pinv == A exactly; `pinv(zeros 2×2)` == zeros; boundary: `pinv([1e-10]) ≈ 1e-10` (not inverted), `pinv([2e-10]) ≈ 5e9` (inverted). |
| `det.hpp` `[det]` | 1×1 `{0}` → 0, `{5}` → 5; 2×2 `[[0,1],[1,0]]` → −1 (odd swap); 2×2 `[[2,1],[1,2]]` → 3; singular block 4×4 → `== 0.0` exactly (E07); nonsingular 4×4 (det 51) vs in-file Bareiss; `diag(1, 1e-14)` → `≈ 1e-14` and `!= 0` (P7); 3×3 `[[0,1,2],[1,0,3],[4,5,6]]` → 16 (two swaps). |
| `matrix_power.hpp` `[matrix_power]` | 2×2 `{1,1,0,1}`: n=0→I, 1→m, 2→`{1,2,0,1}`, 3→`{1,3,0,1}`, 4→`{1,4,0,1}` (closed form `[[1,n],[0,1]]`); 3×3 fixed matrix: n=3 and n=5 vs a loop-product oracle (independent); 1×1 `{2}`: n=5 → 32. |
| `lu_pivoting.hpp` `[lu_pivoting]` | E09 6×6 SPD system: `lu_solver` vs in-TU legacy no-pivot oracle (< 1e-9) and vs exact x (1..6) (< 1e-9); zero-first-pivot rescue 3×3 (pre-fix: nullopt; post-fix: x ≈ (1,1,1)); 5-arg on the same 3×3: `perm == {2,0,1}`, `sign == +1`, `‖P·A − L·U‖∞ < 1e-9`; 2×2 `[[0,1],[1,0]]`: `perm == {1,0}`, `sign == −1`; identity 3×3: `perm` identity, `sign == +1`; singular 2×2 `[[1,2],[2,4]]`: `lu_solver` → nullopt (pin). |

Suite-safety: every case uses square matrices for `det`/`operator^`/`lu_*` (D10).

## Commit points (per S1/S2 convention)

1. `S3 pre-flight: phase docs, E05–E09 probes, pre-fix evidence (E05/E06/E07 red, E08 compile error, E09 oracle self-check, wide-SVD gap finding)`
2. `S3 task 1 (C3): flip aliases to MATLAB/NumPy convention + flip_aliases cases (TDD red→green; E05 green)`
3. `S3 task 2 (C4/R1): single pinv SVD-inversion core (svd_inverse fixed in place, pinverse delegates) + pinv cases (TDD; E06 green)`
4. `S3 task 3 (P2): lu_decomposition partial pivoting (5-arg primary, delegating 3-arg) + lu_solver P-to-b + lu_pivoting cases (TDD; E09 invariance green)`
5. `S3 task 4 (C5): det via pivoted LU product (exact-zero -> 0, no epsilon) + det cases (TDD; E07 green)`
6. `S3 task 5 (C6): operator^ odd branch + matrix_power cases (TDD red = compile failure, documented; E08 green)`
7. `S3 closeout: sharded review + adversarial verification + full re-checks + eval-seed promotion (E05–E09) + risk register + handoff`

Ordering rationale: P2 (task 3) before C5 (task 4) — `det` consumes the pivoted LU;
C3/C4 are independent and go first (smallest diffs first). C6 last: its red state is a
compile failure of the whole `operator^` instantiation, so the new test file only compiles
after the fix (documented TDD variant — the *red* is the pre-fix compile log of the test TU,
recorded before the fix).
