TEST_CASE( "Matrix View Bracket", "[matrixviewbracket]" )
{
    {
        auto row = 11UL;
        auto col = 13UL;
        auto mat = feng::rand<float>( row, col );

        for ( auto r = 0UL; r != mat.row(); ++r )
            for ( auto c = 0UL; c != mat.col(); ++c )
                for ( auto rx = r+1; rx <= mat.row(); ++rx )
                    for ( auto cx = c+1; cx <= mat.row(); ++cx )
                    {
                        //auto view = feng::matrix_view{ mat, std::make_pair(r, rx), std::make_pair(c, cx) };
                        auto view = feng::make_view( mat, {r, rx}, {c, cx} );
                        auto const& [vr, vc] = view.shape();
                        REQUIRE( vr == rx-r );
                        REQUIRE( vc == cx-c );
                        REQUIRE( vr == view.row() );
                        REQUIRE( vc == view.col() );
                        REQUIRE( vc*vr == view.size() );
                        for ( auto cr = 0UL; cr != view.row(); ++cr )
                            for (auto cc = 0UL; cc != view.col(); ++cc )
                                REQUIRE( std::abs(view[cr][cc] - mat[cr+r][cc+c]) < 1.0e-10 );

                    }
    }

}

