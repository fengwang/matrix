void _0000_singular_value_decomposition()
{
    // load
    feng::matrix<double> m;
    m.load_txt( "./images/Teacher.txt" );
    // normalize
    auto const mx = *std::max_element( m.begin(), m.end() );
    auto const mn = *std::min_element( m.begin(), m.end() );
    m = ( m - mn ) / ( mx - mn + 1.0e-10 );
    // take a snapshot
    m.save_as_bmp( "./images/0000_singular_value_decomposition.bmp", "gray" );
    // adding noise
    auto const[r, c] = m.shape();
    m += feng::rand<double>( r, c );
    // record noisy matrix
    m.save_as_bmp( "./images/0001_singular_value_decomposition.bmp", "gray" );
    // execute svd
    auto const& svd = feng::singular_value_decomposition( m );
    // check svd result
    if (svd) // case successful
    {
        // extracted svd result matrices, u, v w
        auto const& [u, v, w] = (*svd);
        // try to reconstruct matrix using  u * v * w'
        auto const& m_ = u * v * (w.transpose());
        // record reconstructed matrix
        m_.save_as_bmp( "./images/0002_singular_value_decomposition.bmp", "gray" );

        auto dm = std::min( r, c );
        auto factor = 2UL;
        while ( dm >= factor )
        {
            auto new_dm = dm / factor;

            feng::matrix<double> const new_u{ u, std::make_pair(0UL, r), std::make_pair(0UL, new_dm) };
            feng::matrix<double> const new_v{ v, std::make_pair(0UL, new_dm), std::make_pair(0UL, new_dm) };
            feng::matrix<double> const new_w{ w, std::make_pair(0UL, c), std::make_pair(0UL, new_dm) };

            auto const& new_m = new_u * new_v * new_w.transpose();

            new_m.save_as_bmp( "./images/0003_singular_value_decomposition_"+std::to_string(new_dm)+".bmp", "gray" );

            factor *= 2UL;
        }
    }
    else
    {
        std::cout << "Failed to execute Singular Value Decomposition for this matrix!\n";
    }
}

