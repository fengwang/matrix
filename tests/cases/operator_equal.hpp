TEST_CASE( "Operator equal", "[opeq]" )
{
    {
        unsigned long N = 100;
        for ( unsigned long r = 1; r != N; ++r )
            for ( unsigned long c = 1; c != N; ++c )
            {
                auto const& mat = feng::rand<double>( r, c );
                auto mat_ = mat;
                auto const& nat = feng::rand<double>( r, c );
                auto nat_ = nat;
                mat_ -= nat;
                nat_ -= 1.0;

                for ( unsigned long r_ = 0; r_ != r; ++r_ )
                    for ( unsigned long c_ = 0; c_ != c; ++c_ )
                    {
                        REQUIRE( std::abs( mat_[r_][c_] + nat[r_][c_] - mat[r_][c_] ) <= 1.0e-5 );
                        REQUIRE( std::abs( nat_[r_][c_] + 1.0 - nat[r_][c_] ) <= 1.0e-5 );
                    }

            }
    }
    {
        unsigned long N = 100;
        for ( unsigned long r = 1; r != N; ++r )
            for ( unsigned long c = 1; c != N; ++c )
            {
                auto const& mat = feng::rand<double>( r, c );
                auto mat_ = mat;
                auto const& nat = feng::rand<double>( r, c );
                auto nat_ = nat;
                mat_ += nat;
                nat_ += 1.0;

                for ( unsigned long r_ = 0; r_ != r; ++r_ )
                    for ( unsigned long c_ = 0; c_ != c; ++c_ )
                    {
                        REQUIRE( std::abs( mat_[r_][c_] - nat[r_][c_] - mat[r_][c_] ) <= 1.0e-5 );
                        REQUIRE( std::abs( nat_[r_][c_] - 1.0 - nat[r_][c_] ) <= 1.0e-5 );
                    }

            }
    }

}
