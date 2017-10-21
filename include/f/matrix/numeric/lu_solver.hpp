#ifndef MLU_SOLVER_HPP_INCLUDED_SDOJI43POUIHAFLKJHADLKJH4IJASFODIJ4980UAFLHSLFDKJ498ULFKJALFKJSFOIJDF
#define MLU_SOLVER_HPP_INCLUDED_SDOJI43POUIHAFLKJHADLKJH4IJASFODIJ4980UAFLHSLFDKJ498ULFKJALFKJSFOIJDF

#include "./lu_decomposition.hpp"
#include "./forward_substitution.hpp"
#include "./backward_substitution.hpp"

namespace f
{
    //solving   Ax = b      -- A is a upper triangular matrix
    //
    //return
    //          0    --     success
    //          1    --     failed
    template<typename T1, typename A1,
             typename T2, typename A2,
             typename T3, typename A3>
    int
    lu_solver( const matrix<T1, A1>&           A,
               matrix<T2, A2>&                 x,
               const matrix<T3, A3>&           b )
    {
        typedef matrix<T1, A1>                 matrix_type;
        assert( A.row() == A.col() );
        assert( A.row() == b.row() );
        assert( b.col() == 1 );
        matrix_type L, U;

        // if lu decomposition failed, return
        if ( lu_decomposition( A, L, U ) ) return 1;

        matrix_type Y;

        if ( forward_substitution( L, Y, b ) ) return 1; // solve LY=b

        if ( backward_substitution( U, x, Y ) ) return 1; //solve Ux=Y

        return 0;
    }

}//namespace f

#endif//_LU_SOLVER_HPP_INCLUDED_SDOJI43POUIHAFLKJHADLKJH4IJASFODIJ4980UAFLHSLFDKJ498ULFKJALFKJSFOIJDF

