void _0000_magic()
{
    std::cout << "Magic 3\n " << feng::magic( 3 ) << std::endl;
    std::cout << "Magic 4\n " << feng::magic( 4 ) << std::endl;
    std::cout << "Magic 5\n " << feng::magic( 5 ) << std::endl;
    std::cout << "Magic 6\n " << feng::magic( 6 ) << std::endl;
    std::cout << "Magic 8\n " << feng::magic( 8 ) << std::endl;
    std::cout << "Magic 10\n " << feng::magic( 10 ) << std::endl;
}

void _0001_magic()
{
    unsigned long n = 38;
    unsigned long pixs = 16;

    auto const& mat = feng::magic( n );

    feng::matrix<double> v_mat( n*pixs, n*pixs );

    for ( auto r = 0UL; r != n; ++r )
        for ( auto c = 0UL; c != n; ++c )
            for ( auto rr = 0UL; rr != pixs; ++rr )
                for ( auto cc = 0UL; cc != pixs; ++cc )
                    v_mat[r*pixs+rr][c*pixs+cc] = mat[r][c];

    v_mat.save_as_bmp("./images/0001_magic.bmp");
}

