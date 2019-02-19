TEST_CASE( "Misc Range", "[miscrange]" )
{
    {
        auto start = 0UL;
        auto end = 10UL;

        for ( auto x = start; x != end; ++x )
            for ( auto y = x; y != end; ++y )
            {
                auto ranger = feng::matrix_details::range( x, y );
                auto begin = std::begin(ranger);
                auto end = std::end(ranger);
                REQUIRE( std::distance(begin, end) == y - x );
                auto val = x;
                for (auto v : ranger )
                {
                    REQUIRE( val == v );
                    ++val;
                }
            }

        for ( auto x = start; x != end; ++x )
        {
                auto ranger = feng::matrix_details::range( x );
                auto begin = std::begin(ranger);
                auto end = std::end(ranger);
                REQUIRE( std::distance(begin, end) == x );
                auto val = 0UL;
                for (auto v : ranger)
                {
                    REQUIRE( val == v );
                    ++val;
                }
        }
    }
}

