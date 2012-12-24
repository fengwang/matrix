#ifndef _LU_SOLVER_HPP_INCLUDED_SDOJI43POUIHAFLKJHADLKJH4IJASFODIJ4980UAFLHSLFDKJ498ULFKJALFKJSFOIJDF
#define _LU_SOLVER_HPP_INCLUDED_SDOJI43POUIHAFLKJHADLKJH4IJASFODIJ4980UAFLHSLFDKJ498ULFKJALFKJSFOIJDF

#include <matrix/matrix.hpp>
#include <matrix/functional.hpp>
#include <matrix/numeric/lu_decomposition.hpp>
#include <matrix/numeric/forward_substitution.hpp>
#include <matrix/numeric/backward_substitution.hpp>

#include <algorithm>
#include <cstddef>
#include <cassert>
#include <cmath>

namespace feng
{
    //solving   Ax = b      -- A is a upper triangular matrix
    //
    //return
    //          0    --     success
    //          1    --     failed
    template<typename T1, std::size_t D1, typename A1, typename T2, std::size_t D2, typename A2, typename T3, std::size_t D3, typename A3>
    int
    lu_solver( const matrix<T1,D1,A1>&           A, 
               matrix<T2,D2,A2>&                 x, 
               const matrix<T3,D3,A3>&           b )
    {
        typedef matrix<T1,D1,A1>                 matrix_type;
        //typedef typename matrix_type::value_type value_type;
        typedef typename matrix_type::size_type  size_type;

        assert( A.row() == A.col() );
        assert( A.row() == b.row() );
        assert( b.col() == 1 );
        size_type const n = A.row();

        matrix_type L, U;
        // if lu decomposition failed, return 
        if ( lu_decomposition( A, L, U ) ) return 1;

        matrix_type Y;
        if( forward_substitution( L, Y, b ) ) return 1; // solve LY=b
        if( backward_substitution( U, x, Y )) return 1; //solve Ux=Y

        return 0;
    }

}//namespace feng

#endif//_LU_SOLVER_HPP_INCLUDED_SDOJI43POUIHAFLKJHADLKJH4IJASFODIJ4980UAFLHSLFDKJ498ULFKJALFKJSFOIJDF

