void _0000_create()
{
    feng::matrix<double> m{ 64, 256 };
    std::generate( m.begin(), m.end(),  [](){ double init = 0.0; return [init]() mutable { init += 0.1; return init; }; }() );
    m.save_as_bmp( "./images/0000_create.bmp" );
}

void _0001_create()
{
    feng::matrix<double> m{ 64, 256 };

    std::generate( m.row_begin(17), m.row_end(17),  [](){ double init = 0.0; return [init]() mutable { init += 0.1; return init; }; }() );
    m.save_as_bmp( "./images/0001_create.bmp" );
}

void _0002_create()
{
    feng::matrix<double> m{ 64, 256 };
    m.save_as_bmp( "./images/0002_create.bmp" );
}

void _0003_create()
{
    feng::matrix<double> m{ 64, 256 };

    std::generate( m.row_rbegin(17), m.row_rend(17),  [](){ double init = 0.0; return [init]() mutable { init += 0.1; return init; }; }() );
    m.save_as_bmp( "./images/0003_create.bmp" );
}

void _0004_create()
{
    feng::matrix<double> m{ 64, 256 };

    std::generate( m.col_begin(17), m.col_end(17),  [](){ double init = 0.0; return [init]() mutable { init += 0.1; return init; }; }() );
    m.save_as_bmp( "./images/0004_create.bmp" );
}

void _0005_create()
{
    feng::matrix<double> m{ 64, 256 };

    std::generate( m.col_rbegin(17), m.col_rend(17),  [](){ double init = 0.0; return [init]() mutable { init += 0.1; return init; }; }() );
    m.save_as_bmp( "./images/0005_create.bmp" );
}

void _0006_create()
{
    feng::matrix<double> m{ 64, 256 };
    std::generate( m.rbegin(), m.rend(),  [](){ double init = 0.0; return [init]() mutable { init += 0.1; return init; }; }() );
    m.save_as_bmp( "./images/0006_create.bmp" );
}

void _0007_create()
{
    feng::matrix<double> m{ 64, 256 };

    std::generate( m.upper_diag_begin(17), m.upper_diag_end(17),  [](){ double init = 0.0; return [init]() mutable { init += 0.1; return init; }; }() );
    m.save_as_bmp( "./images/0007_create.bmp" );
}

void _0008_create()
{
    feng::matrix<double> m{ 64, 256 };

    std::generate( m.upper_diag_rbegin(17), m.upper_diag_rend(17),  [](){ double init = 0.0; return [init]() mutable { init += 0.1; return init; }; }() );
    m.save_as_bmp( "./images/0008_create.bmp" );
}

void _0009_create()
{
    feng::matrix<double> m{ 64, 256 };

    std::generate( m.lower_diag_begin(17), m.lower_diag_end(17),  [](){ double init = 0.0; return [init]() mutable { init += 0.1; return init; }; }() );
    m.save_as_bmp( "./images/0009_create.bmp" );
}

void _0010_create()
{
    feng::matrix<double> m{ 64, 256 };

    std::generate( m.lower_diag_rbegin(17), m.lower_diag_rend(17),  [](){ double init = 0.0; return [init]() mutable { init += 0.1; return init; }; }() );
    m.save_as_bmp( "./images/0010_create.bmp" );
}

void _0011_create()
{
    feng::matrix<double> m{ 64, 256 };

    std::generate( m.diag_begin(), m.diag_end(),  [](){ double init = 0.0; return [init]() mutable { init += 0.1; return init; }; }() );
    m.save_as_bmp( "./images/0011_create.bmp" );
}

void _0012_create()
{
    feng::matrix<double> m{ 64, 256 };

    std::generate( m.diag_rbegin(), m.diag_rend(),  [](){ double init = 0.0; return [init]() mutable { init += 0.1; return init; }; }() );
    m.save_as_bmp( "./images/0012_create.bmp" );
}

void _0013_create()
{
    feng::matrix<double> m{ 64, 256 };

    std::generate( m.upper_anti_diag_begin(17), m.upper_anti_diag_end(17),  [](){ double init = 0.0; return [init]() mutable { init += 0.1; return init; }; }() );
    m.save_as_bmp( "./images/0013_create.bmp" );
}

void _0014_create()
{
    feng::matrix<double> m{ 64, 256 };

    std::generate( m.upper_anti_diag_rbegin(17), m.upper_anti_diag_rend(17),  [](){ double init = 0.0; return [init]() mutable { init += 0.1; return init; }; }() );
    m.save_as_bmp( "./images/0014_create.bmp" );
}

void _0015_create()
{
    feng::matrix<double> m{ 64, 256 };

    std::generate( m.lower_anti_diag_begin(17), m.lower_anti_diag_end(17),  [](){ double init = 0.0; return [init]() mutable { init += 0.1; return init; }; }() );
    m.save_as_bmp( "./images/0015_create.bmp" );
}

void _0016_create()
{
    feng::matrix<double> m{ 64, 256 };

    std::generate( m.lower_anti_diag_rbegin(17), m.lower_anti_diag_rend(17),  [](){ double init = 0.0; return [init]() mutable { init += 0.1; return init; }; }() );
    m.save_as_bmp( "./images/0016_create.bmp" );
}

void _0017_create()
{
    feng::matrix<double> m{ 64, 256 };

    std::generate( m.anti_diag_begin(), m.anti_diag_end(),  [](){ double init = 0.0; return [init]() mutable { init += 0.1; return init; }; }() );
    m.save_as_bmp( "./images/0017_create.bmp" );
}

void _0018_create()
{
    feng::matrix<double> m{ 64, 256 };

    std::generate( m.anti_diag_rbegin(), m.anti_diag_rend(),  [](){ double init = 0.0; return [init]() mutable { init += 0.1; return init; }; }() );
    m.save_as_bmp( "./images/0018_create.bmp" );
}

void _0019_create()
{
    feng::matrix<double> m{ 64, 256 };

    for ( auto r = 12; r != 34; ++r )
        for ( auto c = 34; c != 45; ++c )
            m[r][c] = 1.0;

    for ( auto r = 34; r != 45; ++r )
        for ( auto c = 123; c != 234; ++c )
            m(r, c) = -1.0;

    m.save_as_bmp( "./images/0019_create.bmp" );
}

void _0020_create()
{
    feng::matrix<double> m{ 64, 256 };
    for ( auto r = 12; r != 34; ++r )
        for ( auto c = 12; c != 34; ++c )
            m[r][c] = 1.0;
    m.save_as_bmp( "./images/0020_create.bmp" );

    feng::matrix<double> n = m; //copying
    n.save_as_bmp( "./images/0021_create.bmp" );

    n.resize( 63, 244 );
    n.save_as_bmp( "./images/0022_create.bmp" );

    m.reshape( m.col(), m.row() );
    m.save_as_bmp( "./images/0023_create.bmp" );
}
















