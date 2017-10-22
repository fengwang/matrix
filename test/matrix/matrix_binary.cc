#include <f/matrix/matrix.hpp>

#include <complex>
#include <cassert>

using namespace f;

std::string test_file_name = "/tmp/test.bin";
unsigned long r = 16;
unsigned long c = 16;

template< typename T >
void check()
{
    matrix<T> mat{ r, c };
    T ini{0};
    T step{1};
    std::for_each( mat.begin(), mat.end(), [&]( T& val ) { val = ini; ini += step; } );

    mat.save_as_binary( test_file_name );

    matrix<T> nat;
    nat.load_binary( test_file_name );

    assert( mat == nat && "Not match" );

    std::cout << "Test success\n" << std::endl;

    mat.save_as_txt( "/tmp/test.txt" );
}

int main()
{
    check<char>();
    check<short>();
    check<int>();
    check<long>();
    check<long long>();
    check<float>();
    check<double>();
    check<long double>();
    check<std::complex<float>>();
    check<std::complex<double>>();
    check<std::complex<long double>>();
    check<char>();


    matrix<double> stone;
    stone.load( "./matrix/stone_calibrate.txt" );
    stone.save_as_binary( "./matrix/stone_calibrate.bin" );

    return 0;
}

