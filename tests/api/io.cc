// S1-R4 API family `io`: save_as_txt, load_txt, save_as_binary, load_binary, save_as_npy, load_npy, stream
// operators, disp/display. Compiled alone by `tools/check.sh api` and never run; file names point under build/tmp.
#include "../../matrix.hpp"

#include <complex>
#include <sstream>
#include <string>

namespace
{
    template < typename T >
    T sink( feng::matrix<T> const& m )
    {
        return m.size() ? m[0][0] : T{};
    }

    template < typename T >
    T exercise_files()
    {
        feng::matrix<T> a{ 2, 3, T{ 1 } };
        feng::matrix<T> b;
        std::string const txt{ "build/tmp/api_io.txt" };
        std::string const bin{ "build/tmp/api_io.bin" };
        bool ok = true;
        ok = a.save_as_txt( txt ) && ok;
        ok = a.save_as_txt( txt.c_str() ) && ok;
        ok = b.load_txt( txt ) && ok;
        ok = b.load_txt( txt.c_str() ) && ok;
        ok = a.save_as_binary( bin ) && ok;
        ok = a.save_as_binary( bin.c_str() ) && ok;
        ok = b.load_binary( bin ) && ok;
        ok = b.load_binary( bin.c_str() ) && ok;
        return ok ? sink( b ) : T{};
    }

    template < typename T >
    T exercise_npy()
    {
        feng::matrix<T> a{ 2, 3, T{ 1 } };
        feng::matrix<T> b;
        std::string const npy{ "build/tmp/api_io.npy" };
        bool ok = a.save_as_npy( npy );
        ok = a.save_as_npy( npy.c_str() ) && ok;
        ok = b.load_npy( npy ) && ok;
        ok = b.load_npy( npy.c_str() ) && ok;
        return ok ? sink( b ) : T{};
    }

    template < typename T >
    T exercise_streams()
    {
        feng::matrix<T> a{ 2, 2, T{ 1 } };
        std::ostringstream os;
        os << a;
        feng::matrix<T> b{ 2, 2 };
        std::istringstream is{ os.str() };
        is >> b;
        feng::display( a );
        feng::disp( a );
        return sink( b );
    }

    template < typename T >
    T exercise_io()
    {
        return exercise_files<T>() + exercise_streams<T>();
    }
} // namespace

double api_io_double() { return exercise_io<double>() + exercise_npy<double>(); }
float api_io_float() { return exercise_io<float>() + exercise_npy<float>(); }
std::complex<double> api_io_complex() {
    return exercise_io<std::complex<double>>() + exercise_npy<std::complex<double>>();
}
int api_io_int() { return exercise_io<int>() + exercise_npy<int>(); }
