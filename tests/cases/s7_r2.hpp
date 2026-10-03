// S7-R2 (PR-10, PR-2): det and inverse from the LU factors (F13). Uses the helpers of tests/cases/s7_r1.hpp.
namespace s7
{
    template< typename T >
    void check_det()
    {
        using R = real_t< T >;
        // block exchange: 4 within 8·n·ε·|4|
        auto const bx = from_real< T >( 4, 4, { 0, 0, 1, 2, 0, 0, 3, 4, 5, 6, 0, 0, 7, 8, 0, 0 } );
        REQUIRE( std::abs( bx.det() - T( 4 ) ) <= R( 8 ) * R( 4 ) * eps< T >() * R( 4 ) );
        REQUIRE( std::abs( feng::det( bx ) - T( 4 ) ) <= R( 8 ) * R( 4 ) * eps< T >() * R( 4 ) );
        REQUIRE( std::abs( feng::lu_factor( bx ).det() - T( 4 ) ) <= R( 8 ) * R( 4 ) * eps< T >() * R( 4 ) );
        // exactly 0 when rank < n: [[1..9]], a zero row, a repeated column
        for ( auto const& s : { from_real< T >( 3, 3, { 1, 2, 3, 4, 5, 6, 7, 8, 9 } ),
                                from_real< T >( 3, 3, { 1, 2, 3, 0, 0, 0, 4, 5, 6 } ),
                                from_real< T >( 3, 3, { 0.1, 2, 0.1, 0.3, 4, 0.3, 0.7, 6, 0.7 } ) } )
        {
            REQUIRE( s.det() == T( 0 ) );
            REQUIRE( feng::det( s ) == T( 0 ) );
        }
        // 1 for 0×0 (numpy)
        REQUIRE( mat< T >{}.det() == T( 1 ) );
        REQUIRE( feng::det( mat< T >{} ) == T( 1 ) );
        // identity, diagonal and row swap are exact
        REQUIRE( feng::eye< T >( 6, 6 ).det() == T( 1 ) );
        REQUIRE( from_real< T >( 3, 3, { 2, 0, 0, 0, -3, 0, 0, 0, 0.5 } ).det() == T( -3 ) );
        REQUIRE( from_real< T >( 3, 3, { 0, 1, 0, 1, 0, 0, 0, 0, 1 } ).det() == T( -1 ) );
    }

    template< typename T >
    void check_det_product()
    {
        using R = real_t< T >;
        for ( std::size_t n = 1; n <= 12; ++n )
        {
            INFO( "n = " << n );
            // random entries in (0, 1) plus n·I: the bound has no κ factor, and the det error of plain random
            // (0, 1) fixtures grows with κ (measured up to 272·n·ε at n = 11)
            mat< T > const shift = feng::eye< T >( n, n ) * T( R( n ) );
            mat< T > const a = random_square< T >( n, 0x57D0ULL + n ) + shift;
            mat< T > const b = random_square< T >( n, 0x57E0ULL + n ) + shift;
            T const dab = ( a * b ).det(), da = a.det(), db = b.det();
            REQUIRE( std::abs( dab - da * db ) <= R( 64 ) * R( n ) * eps< T >() * std::abs( da * db ) );
        }
    }

    template< typename T >
    void check_inverse()
    {
        using R = real_t< T >;
        for ( auto const& [name, a] : r09_inputs< T >() )
        {
            INFO( name );
            std::size_t const n = a.row();
            mat< T > const x = a.inverse();
            REQUIRE( x.row() == n ); REQUIRE( x.col() == n );
            R const kappa = norm_inf( a ) * norm_inf( x );
            R const ratio = norm_inf< T >( a * x - feng::eye< T >( n, n ) ) / ( R( n ) * eps< T >() * kappa );
            REQUIRE( ratio <= R( 16 ) );
            REQUIRE( norm_inf< T >( feng::inverse( a ) - x ) == R( 0 ) );
            REQUIRE( norm_inf< T >( feng::inv( a ) - x ) == R( 0 ) );
            auto const t = feng::try_inverse( a );
            REQUIRE( t.ok() );
            REQUIRE( norm_inf< T >( t.value - x ) == R( 0 ) );
            auto const f = feng::lu_factor( a ).inverse();
            REQUIRE( f.ok() );
            REQUIRE( norm_inf< T >( f.value - x ) == R( 0 ) );
            mat< T > out;
            REQUIRE( feng::inverse( a, out ) == feng::linalg_status::ok );
            REQUIRE( norm_inf< T >( out - x ) == R( 0 ) );
        }
        // singular and nonfinite: statuses, out unchanged, value-returning forms give 0×0 (D-026)
        auto nf = feng::eye< T >( 3, 3 );
        nf[0][1] = T( std::numeric_limits< R >::infinity() );
        std::pair< mat< T >, feng::linalg_status > const bad[] = {
            { from_real< T >( 3, 3, { 1, 2, 3, 4, 5, 6, 7, 8, 9 } ), feng::linalg_status::singular },
            { from_real< T >( 2, 2, { 0, 0, 0, 0 } ), feng::linalg_status::singular },
            { nf, feng::linalg_status::nonfinite } };
        for ( auto const& [s, st] : bad )
        {
            REQUIRE( s.inverse().size() == 0 );
            REQUIRE( s.inverse().row() == 0 );
            REQUIRE( feng::inverse( s ).size() == 0 );
            REQUIRE( feng::inv( s ).size() == 0 );
            auto const t = feng::try_inverse( s );
            REQUIRE( t.status == st );
            REQUIRE( t.value.size() == 0 );
            REQUIRE( feng::lu_factor( s ).inverse().status == st );
            mat< T > out{ 1, 2, T( 3 ) };
            REQUIRE( feng::inverse( s, out ) == st );
            REQUIRE( out.row() == 1 ); REQUIRE( out.col() == 2 ); REQUIRE( out[0][1] == T( 3 ) );
        }
        // 0×0 inverts to 0×0 with status ok
        REQUIRE( feng::try_inverse( mat< T >{} ).ok() );
    }
}

TEST_CASE( "S7-R2 det of the block-exchange and singular matrices", "[S7][S7-R2]" )
{
    s7::check_det< double >();
    s7::check_det< std::complex< double > >();
    s7::check_det< float >();
    s7::check_det< std::complex< float > >();
}

TEST_CASE( "S7-R2 det of a product is the product of dets", "[S7][S7-R2]" )
{
    s7::check_det_product< double >();
    s7::check_det_product< std::complex< double > >();
    s7::check_det_product< float >();
    s7::check_det_product< std::complex< float > >();
}

TEST_CASE( "S7-R2 inverse from the factors and its failure statuses", "[S7][S7-R2]" )
{
    s7::check_inverse< double >();
    s7::check_inverse< std::complex< double > >();
    s7::check_inverse< float >();
    s7::check_inverse< std::complex< float > >();
}
