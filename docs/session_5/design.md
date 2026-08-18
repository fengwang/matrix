# Session 5 — Design

Line numbers refer to `matrix.hpp` at HEAD `c40b04b` (7877 lines). No other file has
design content except `tests/cases/rand.hpp` (§5).

## 1. `rand-engine` (C11)

### Current code (verbatim, 5320–5337)

```cpp
//generating a matrix uniformly in (0, 1)
    template < typename T = double, typename A = std::allocator< T > >
    matrix< T, A > const rand( const std::uint_least64_t r, const std::uint_least64_t c, unsigned int seed = 0 ) noexcept
    {
        matrix< T, A > ans{ r, c };
        if ( 0 == seed )
            std::srand( static_cast< unsigned int >( static_cast< std::uint_least64_t >( std::time( nullptr ) ) + reinterpret_cast< std::uint_least64_t >( &ans ) ) );
        else
            std::srand( seed );

        auto const& generator = []() noexcept
        {
            return ( static_cast<T>( std::rand() ) + 1 ) / ( static_cast<T>( RAND_MAX ) + 2 ); // make sure in open bounds range (0, 1)
        };
        std::generate( ans.begin(), ans.end(), generator );
        return ans;
    }
```

### Final code (replaces 5320–5337)

```cpp
//generating a matrix uniformly in [0, 1)
    template < typename T = double, typename A = std::allocator< T > >
    matrix< T, A > const rand( const std::uint_least64_t r, const std::uint_least64_t c, unsigned int seed = 0 )
    {
        matrix< T, A > ans{ r, c };
        // seed 0 keeps the documented time-based mix (time + &ans address salt, low entropy;
        // residual same-call-site/same-second correlation is inherent to this seed — documented, not a violation)
        unsigned int const effective_seed = ( 0 == seed )
            ? static_cast< unsigned int >( static_cast< std::uint_least64_t >( std::time( nullptr ) ) + reinterpret_cast< std::uint_least64_t >( &ans ) )
            : seed;
        std::mt19937 engine{ effective_seed }; // per-call local engine: no global state, thread-safe by construction
        std::uniform_real_distribution< T > distribution{ 0.0, 1.0 }; // non-const: operator() is non-const
        auto const& generator = [ & ]()
        {
            return static_cast< T >( distribution( engine ) ); // in [0, 1)
        };
        std::generate( ans.begin(), ans.end(), generator );
        return ans;
    }
```

Changes, exactly:
1. Body 5325–5335 replaced: no `srand`, no `std::rand`, no `RAND_MAX`; local
   `std::mt19937` + `std::uniform_real_distribution<T>` (contract-prescribed).
2. `noexcept` dropped from the declaration (5323) — allocation inside the matrix
   ctor / `std::generate` can throw.
3. Header comment `(0, 1)` → `[0, 1)` (the distribution's actual bounds).

### `noexcept` removals in the rand chain (interview Q3)

| Line | Declaration | Change |
|------|-------------|--------|
| 5323 | `rand(r, c, seed)` | drop `noexcept` |
| 5355 | `rand_like(mat)` | drop `noexcept` (calls `random` → `rand`) |
| 5361 | `random_like(mat)` | drop `noexcept` (calls `rand_like`) |
| 5366 | `randn_like(mat)` | drop `noexcept` (calls `rand_like`) |

`rand(n)` (5339), `random(r,c)` (5345), `random(n)` (5350) already lack `noexcept`.

### Behavior table (adversarial input → result)

| Input | Result |
|-------|--------|
| `rand<double>(m,n,7)` twice | bitwise-identical matrices (determinism — E14 pin, green pre- and post-fix) |
| seed 7 vs seed 8 | different matrices (E14 pin) |
| seed 0, same call site, same second | same matrix (residual correlation — inherent to the time+&ans seed, D2; engine change does not alter it) |
| seed 0, different call sites | usually different (`&ans` salt, P8) |
| any seed, `T = double`/`float` | all elements in [0,1) |
| `T = int` | does not compile — the contract-prescribed `uniform_real_distribution<T>` requires a floating-point `result_type` ([uniform.real]); libstdc++ enforces it (static_assert, verified post-fix); pre-fix int gave all zeros. No in-repo consumers (audited) — documented |
| `T = complex` | does not compile (same standard requirement as int — non-floating-point `result_type`); zero in-repo consumers; documented (D4) |
| 0×0 / 1×1 shape | shape preserved (generate over empty/single range) |
| two threads calling `rand` concurrently | no data race — no shared mutable state in user code (D1); TSan post-fix clean |
| allocation failure (throw) | propagates (no longer `terminate`) — `noexcept` removed |

### R-07 (sanctioned stream change)

Explicit-seed streams change (pre-fix `rand(1,4,1)` = `0.84018771683788496 …`, P9).
Examples 0012/0019/0020/0021 print different random values post-fix; seedless
examples already varied per run. `make example` stdout delta recorded at closeout
(`.work/evidence/s5_example_delta.txt`); `images/` checked out.

## 2. `core-count-guard` (C12)

### Site 1 — `reduce` (1152, currently unguarded; divides at 1163)

Before:
```cpp
        unsigned int const total_cores = std::thread::hardware_concurrency();
```
After:
```cpp
        unsigned int total_cores = std::thread::hardware_concurrency();
        if ( total_cores < 1 )
            total_cores = 1;
```
(`const` dropped; `hardware_concurrency()` returns 0 = "could not determine" per
[thread.hard_concurrency] — 0 would SIGFPE at the 1163 divide. No test case per
contract; evidence = grep + review.)

### Site 2 — `reduce_impl_private` (4121, already short-circuit-safe; contract-mandated clamp)

Before:
```cpp
            auto parallel_size = std::thread::hardware_concurrency();
            if ( parallel_size <= 1 || mat.size() < 32 )
```
After:
```cpp
            auto parallel_size = std::thread::hardware_concurrency();
            if ( parallel_size < 1 )
                parallel_size = 1;
            if ( parallel_size <= 1 || mat.size() < 32 )
```
Behavior-neutral (P5: `0 <= 1` already took the sequential path); the explicit clamp
satisfies the contract's `grep 'parallel_size < 1'` acceptance and removes reliance on
the incidental `<= 1` short-circuit.

### Site 3 — 279: untouched
`if ( total_cores <= 1 )` (existing `reduce`-family guard) left exactly as is per the
contract.

## 3. `save-png-boundary` (S2-finding / R3-slice)

### The guard (3181–3184)

Before:
```cpp
        inline static void save_png( std::uint8_t* img,  unsigned w, unsigned h, int alpha, char const* const file_name ) noexcept
        {
            FILE* const fp = fopen( file_name, "wb+" );
```
After:
```cpp
        inline static void save_png( std::uint8_t* img,  unsigned w, unsigned h, int alpha, char const* const file_name ) noexcept
        {
            FILE* const fp = fopen( file_name, "wb+" );
            if ( ! fp )
                return;
```
Silent no-op on open failure (R-05 note; matches the `load_npy` S2 precedent — I/O
boundaries fail hard-but-silently, no throw, no stderr). The member `save_as_png`
(call site 3469) still returns `true`; the failed open is documented as silent.
`noexcept` on the free helper is kept (no allocation in the guarded path).

### The stray `;;` (3190, R3-slice)

Before: `fputc( ( ( "\x89PNG\r\n\32\n" )[i] ), fp );;`
After:  `fputc( ( ( "\x89PNG\r\n\32\n" )[i] ), fp );`
(Harmless double semicolon; removal verified by diff + clean rebuild.)

### Evidence
- Pre-fix red: `save_as_png` to `/nonexistent_dir_s5/x.png` → **SIGSEGV, exit 139** (P7).
- Post-fix green: same call → exit 0, no crash; **positive control**: writable path
  → PNG file exists (E15 probe, both halves).

## 4. `ndebug-policy-doc` (C7)

Mechanism (verified, P12): `debug_mode` (57–61) is `constexpr` — `0` when `NDEBUG`
is defined, else `1`; `print_assertion` prints to `std::cerr` and `abort()`s only in
debug mode. Under `NDEBUG`, `better_assert` is a silent no-op.

**Draft delta text for S6's ReadMe change** (authored here; ReadMe.md itself is S6's):

> ### Assertions, `better_assert`, and `NDEBUG`
>
> `better_assert` is debug-only enforcement. Its runtime check is gated by the
> `debug_mode` constant (matrix.hpp), which is `0` when `NDEBUG` is defined and `1`
> otherwise; in debug builds (the Makefile's default: `-Ofast`, no `-DNDEBUG`) a
> failed assertion prints a message to `std::cerr` and aborts (core dump). Release
> builds that define `NDEBUG` skip every `better_assert` check silently.
>
> Hard runtime checks at I/O and external boundaries are **not** subject to
> `NDEBUG`: `load_npy` (S2) and `save_png` (S5) fail silently instead of throwing
> or aborting on unreadable input / unwritable output. Decomposition-domain guards
> added in S4 (`rref`, `rref_2d`, `cholesky_decomposition`) are ordinary control
> flow, not assertions.
>
> Rule of thumb: API preconditions → `better_assert` (debug-only). I/O and
> external-data boundaries → hard, silent, NDEBUG-independent checks.

## 5. `rand-regression-tests` (E14)

### `tests/cases/rand.hpp` (new)

```cpp
#include <catch2/catch.hpp>
#include <type_traits>

#include "../matrix.hpp"

TEST_CASE( "rand: explicit-seed determinism, [0,1) range, and engine pins (C11/E14)", "[rand]" )
{
    // (a) determinism: same seed -> identical matrix (invariant pin; green pre- and post-fix)
    feng::matrix< double > const a = feng::rand< double >( 64, 64, 7 );
    feng::matrix< double > const b = feng::rand< double >( 64, 64, 7 );
    feng::matrix< double > const c = feng::rand< double >( 64, 64, 8 );

    REQUIRE( a.row() == 64 );
    REQUIRE( a.col() == 64 );
    REQUIRE( a == b );
    REQUIRE( !( a == c ) );

    // (b) [0,1) range, double (no NaN involved -> safe under -Ofast fast-math)
    bool ge_zero = true;
    bool lt_one = true;
    for ( unsigned long r = 0; r < a.row(); ++r )
        for ( unsigned long col = 0; col < a.col(); ++col )
        {
            ge_zero = ( ge_zero && ( a[r][col] >= 0.0 ) );
            lt_one = ( lt_one && ( a[r][col] < 1.0 ) );
        }
    REQUIRE( ge_zero );
    REQUIRE( lt_one );

    // (c) float instantiation: compiles, deterministic, in range
    feng::matrix< float > const fa = feng::rand< float >( 32, 32, 7 );
    feng::matrix< float > const fb = feng::rand< float >( 32, 32, 7 );
    REQUIRE( fa == fb );
    bool f_ge_zero = true;
    bool f_lt_one = true;
    for ( unsigned long r = 0; r < fa.row(); ++r )
        for ( unsigned long col = 0; col < fa.col(); ++col )
        {
            f_ge_zero = ( f_ge_zero && ( fa[r][col] >= 0.0f ) );
            f_lt_one = ( f_lt_one && ( fa[r][col] < 1.0f ) );
        }
    REQUIRE( f_ge_zero );
    REQUIRE( f_lt_one );

    // (d) non-floating-point instantiations (int, complex) do NOT compile: the contract-prescribed
    //     std::uniform_real_distribution<T> requires a floating-point result_type ([uniform.real]);
    //     libstdc++ enforces it (static_assert). No in-repo int/complex consumers (audited) —
    //     documented consequence, same class as the complex-T note.

    // (e) type pins (return type is the const value type, house style)
    static_assert( std::is_same_v< decltype( feng::rand< double >( 4, 4, 7 ) ), feng::matrix< double > const > );
    static_assert( std::is_same_v< decltype( feng::rand< float >( 4, 4, 7 ) ), feng::matrix< float > const > );

    // (f) engine pin: the per-call engine is allocation-backed -> rand must NOT be noexcept (C11)
    static_assert( !noexcept( feng::rand< double >( 1, 1, 7 ) ) );
}
```

House-style notes: local bools + separate `REQUIRE`s (Catch v2.0.1 quirk, interview
Q7); `feng::` qualified; `[r][col]` indexing; whole-matrix `==` (operator at
matrix.hpp 4235, brainstorming P14).

### `tests/test.cc` registration

Insert `#include "./cases/rand.hpp"` after `#include "./cases/proj.hpp"` (line 58),
before the commented `//#include "./cases/remquo.hpp"` (line 59) — alphabetical
(`proj < rand < remquo < rint`). Suite: 73 → 74 cases.

## 6. Test and probe policy

- **Fast-math (R-19):** suite builds with `-Ofast` (fast-math); no NaN-dependent
  assertions anywhere in the new case (range checks only; the rand engine produces
  no NaN).
- **Probe builds (verbatim, contract form):**
  - `g++ -std=c++20 -DPARALLEL -O1 -o .work/probe_s5 .work/probes/E14_E15.cc` —
    post-fix combined E14/E15 probe → `PASS`.
  - TSan: `g++ -std=c++20 -DPARALLEL -fsanitize=thread -O1 -o … S5_p1_tsan.cc` —
    post-fix run must print clean (pre-fix clean = documented libc limitation, P10).
- **E14 is green pre- AND post-fix by design** (invariant pin); the C11 "red" is
  structural (grep 3→0, P1) + the pre-fix correlation/UB evidence (P7/P8) + TSan
  post-fix. E15 has a true executable red→green (SIGSEGV 139 → exit 0, P7).
