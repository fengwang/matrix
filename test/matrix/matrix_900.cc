#include <f/matrix/matrix.hpp>
#include <f/parser/parser.hpp>

#include <iostream>

int main( int argc, char** argv )
{
    unsigned long age = 35;
    unsigned long month = 7;
    int exit = 0;
    unsigned long grid_len = 16;
    unsigned long ban_len = 2;

    auto help_triggered_show = [](){ std::cout << "--help \t show help\n--age \t set age\n--month \t set month\n"; };
    auto help_triggered_exit = [&exit](){ exit = 1; };

    auto ph = f::make_option<void>("--help", [&](){help_triggered_show(); help_triggered_exit(); } );
    auto pa = f::make_option<unsigned long>( "--age", [&age]( unsigned long age_ ){ age = age_; } );
    auto pm = f::make_option<unsigned long>( "--month", [&month]( unsigned long month_ ){ month = month_; } );

    f::parse( argc, argv, ph, pa, pm );

    if ( exit )
        return 0;

    unsigned long dim_size = grid_len + ban_len + ban_len;
    unsigned long dim = dim_size * 30;
    f::matrix<double> ma{dim,dim};
    std::fill( ma.begin(), ma.end(), 1.0 );
    //edge -- 1.0
    //pased -- 0.0
    //rem -- 0.5
    
    auto set_matrix = [&]( unsigned long r, unsigned long c, double x )
    {
        unsigned long off_r = (r-1)*dim_size;
        unsigned long off_c = (c-1)*dim_size;
        for ( unsigned long r_ = off_r+1; r_ != off_r+grid_len+1; ++r_ )
            for ( unsigned long c_ = off_c+1; c_ != off_c+grid_len+1; ++c_ )
            {
                ma[r_][c_] = x;
            }
    };


    unsigned long rows = (age*12+month) / 30;
    unsigned long rem = age*12+month - rows*30;

    for ( unsigned long r = 1; r <= rows; ++r )
        for ( unsigned long c = 1;  c <= 30; ++c )
            set_matrix( r, c, 0.0 );
    for ( unsigned long c = 1; c <= rem; ++c )
        set_matrix( rows+1, c, 0.0 );

    for ( unsigned long c = rem+1; c <= 30; ++c )
        set_matrix( rows+1, c, 0.5 );

    for ( unsigned long r = rows+2; r <= 30; ++r )
        for ( unsigned long c = 1;  c <= 30; ++c )
            set_matrix( r, c, 0.5 );

    ma = ma.transpose();

    ma.save_as_bmp("output.bmp");

    return 0;
}

