#ifndef _LU_DECOMPOSITION_HPP_INCLUDED_AIE98U3OIAFD9812894LKJSFDLKJSOI83OAIP8D81
#define _LU_DECOMPOSITION_HPP_INCLUDED_AIE98U3OIAFD9812894LKJSFDLKJSOI83OAIP8D81

#include <matrix/matrix.hpp>

#include <cstddef>
#include <cassert>
#include <algorithm>

namespace dynamic
{

    // Example:
    //          matrix<double> m( 100, 100 );
    //          lu_decompositon ld(m);
    //          auto l = ld.lower();
    //          auto u = ld.upper();
    //
    template < typename Matrix_Type >
    struct  lu_decomposition
    {
        typedef Matrix_Type                         matrix_type;
        typedef typename matrix_type::value_type    value_type;

        const matrix_type& a_;
        matrix_type l_;
        matrix_type u_;

        lu_decomposition( const matrix_type& a ) :
            a_( a ),
            l_( a.row(), a.col() ),
            u_( a.row(), a.col() )
        {
            assert( a.row() == a.col() );
            do_decomposition();
        }

        const matrix_type
        lower() const
        { return l_; }

        const matrix_type
        upper() const
        { return u_; }

    private:
        void
        do_decomposition()
        {
            const std::size_t n = a_.row();
            std::fill( l_.diag_begin(), l_.diag_end(), value_type( 1 ) );

            for ( std::size_t j = 0; j < n; ++j )
            {
                for ( std::size_t i = 0; i < j + 1; ++i )
                    { u_[i][j] = a_[i][j] - std::inner_product( l_.row_begin( i ), l_.row_begin( i ) + i, u_.col_begin( j ), value_type() ); }

                for ( std::size_t i = j + 1; i < n; ++i )
                    { l_[i][j] = ( a_[i][j] - std::inner_product( l_.row_begin( i ), l_.row_begin( i ) + j, u_.col_begin( j ), value_type() ) ) / u_[j][j]; }
            }
        }

    };

}//namespace sm

#endif//_LU_DECOMPOSITION_HPP_INCLUDED_AIE98U3OIAFD9812894LKJSFDLKJSOI83OAIP8D81

