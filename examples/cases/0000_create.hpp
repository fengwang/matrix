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





