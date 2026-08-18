#include <type_traits>

// S5 C11/E14: rand's per-call engine pins — explicit-seed determinism, seed
// inequality, [0,1) range (double + float), documented int all-zeros, type and
// noexcept pins. Invariant case: green pre- AND post-fix (the C11 red is
// structural: grep for global state + TSan + the (f) noexcept pin).

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
