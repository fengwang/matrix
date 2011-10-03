#ifndef _LU_DECOMPOSITION_HPP_INCLUDED_AIE98U3OIAFD9812894LKJSFDLKJSOI83OAIP8D81
#define _LU_DECOMPOSITION_HPP_INCLUDED_AIE98U3OIAFD9812894LKJSFDLKJSOI83OAIP8D81

#include <matrix/matrix.hpp>
#include <cstddef>
#include <cassert>
#include <algorithm>

namespace dynamic{

template< typename MATRIX >
struct lu_decomposition
{
	typedef MATRIX matrix_type;

	lu_decomposition( const matrix_type& );

	const matrix_type
		lower() const;
	const matrix_type
		upper() const;
};

template<>
template <
		 typename T,
         std::size_t D,
         typename A
         >
struct  lu_decomposition<matrix<T, D, A> >
{
        typedef matrix<T, D, A> matrix_type;
        typedef typename matrix_type::value_type value_type;

        const matrix_type& a_;
        matrix_type l_;
        matrix_type u_;

        lu_decomposition( const matrix_type& a ) :
            a_( a ),
            l_( matrix_type( a.row(), a.col() ) ),
            u_( matrix_type( a.row(), a.col() ) )
        {
            assert( a.row() == a.col() );
            do_decomposition();
        }

        const matrix_type
        lower() const
        {
            return l_;
        }

        const matrix_type
        upper() const
        {
            return u_;
        }

    private:
        void 
		do_decomposition()
        {
            const std::size_t n = a_.row();
            std::fill( l_.diag_begin(), l_.diag_end(), value_type( 1 ) );
            value_type sum;

            for ( std::size_t j = 0; j < n; ++j )
                {
                    for ( std::size_t i = 0; i < j + 1; ++i )
                        {
                            sum = value_type();

                            for ( std::size_t k = 0; k < i; ++k )
                                {
                                    sum += l_[i][k] * u_[k][j];
                                }

                            u_[i][j] = a_[i][j] - sum;
                        }

                    for ( std::size_t i = j + 1; i < n; ++i )
                        {
                            sum = value_type();

                            for ( std::size_t k = 0; k < j; ++k )
                                {
                                    sum += l_[i][k] * u_[k][j];
                                }

                            l_[i][j] = a_[i][j] - sum;
                            l_[i][j] /= u_[j][j];
                        }
                }
        }

};

}//namespace sm

#endif//_LU_DECOMPOSITION_HPP_INCLUDED_AIE98U3OIAFD9812894LKJSFDLKJSOI83OAIP8D81

