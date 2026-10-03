// S1-R4 API family `allocator`: matrix with a non-default stateless allocator, get_allocator. Compiled alone by
// `tools/check.sh api` and never run.
#include "../../matrix.hpp"

#include <complex>
#include <cstddef>
#include <memory>
#include <new>

namespace
{
    // A small stateless allocator that is not std::allocator: it forwards to the global operator new/delete.
    template < typename T >
    struct api_allocator
    {
        using value_type = T;

        api_allocator() noexcept = default;
        template < typename U >
        api_allocator( api_allocator<U> const& ) noexcept
        {
        }

        T* allocate( std::size_t n ) { return static_cast<T*>( ::operator new( n * sizeof( T ) ) ); }
        void deallocate( T* p, std::size_t ) noexcept { ::operator delete( p ); }

        template < typename U >
        bool operator==( api_allocator<U> const& ) const noexcept
        {
            return true;
        }
    };

    template < typename T >
    T exercise_allocator()
    {
        using alloc_type  = api_allocator<T>;
        using matrix_type = feng::matrix<T, alloc_type>;
        alloc_type const alloc{};
        T acc{};

        matrix_type a{ alloc, 3, 3, T{ 1 } };
        matrix_type b{ 3, 3, T{ 2 } };
        matrix_type c{ a };
        matrix_type d{ alloc, 3, 3 };
        d = b;
        c.swap( d );
        c.resize( 2, 2 );

        alloc_type const got = a.get_allocator();
        acc += ( got == alloc ) ? T{ 1 } : T{};

        matrix_type const s = a + b;
        matrix_type const p = a * b;
        matrix_type const t = a.transpose();
        acc += s[0][0] + p[0][0] + t[0][0] + c[0][0];
        acc += feng::sum( a );
        return acc;
    }
} // namespace

double api_allocator_double() { return exercise_allocator<double>(); }
float api_allocator_float() { return exercise_allocator<float>(); }
std::complex<double> api_allocator_complex() { return exercise_allocator<std::complex<double>>(); }
int api_allocator_int() { return exercise_allocator<int>(); }
