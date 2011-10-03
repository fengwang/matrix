//this file is to be removed

#ifndef _MATRIX_NUMERIC_ALGORITHM_HPP_INCLUDED
#define _MATRIX_NUMERIC_ALGORITHM_HPP_INCLUDED

#include <matrix/matrix.hpp>

#include <cassert>
#include <algorithm>
#include <functional>
#include <numeric>
#include <cstring>

#include <iostream>

namespace dynamic
{

//
// Solve Equation Ax=b for A[n:n], x[n,m], b[n,m]
// Example:
//    matrix<double> A(n,n);
//    matrix<double> b(n,m);
//    
//    auto x = gaussian_elimination()(A,b);    
//  

struct gaussian_elimination
{
    template<typename Matrix_Type>
    const Matrix_Type
    operator()( const Matrix_Type& A, const Matrix_Type& b) const 
    {
        typedef typename Matrix_Type::size_type     size_type;
        typedef typename Matrix_Type::value_type    value_type;
        typedef typename Matrix_Type::range_type    range_type;
        assert( A.row() == A.col() );
        assert( A.row() == b.row() );
        Matrix_Type a( A || b ); // a -- [A:b]
        const size_type n = b.row();
        const size_type m = b.col();

        //std::cerr << "\na = " << a << "\n";

        for ( size_type i = 0; i<n; ++i )
        {
            //std::cerr << "\nstep " << i;
            //find max element
            const size_type 
            p = std::distance( a.col_begin(i), 
                               std::max_element( a.col_begin(i)+i, a.col_end(i), 
                                                 [](value_type x, value_type y ){ return std::abs(x) < std::abs(y);} ));
            //std::cerr << "\nmax element pos is " << p;
            //swap row i and row p
            if ( p != i )
                std::swap_ranges( a.row_begin(i)+i, a.row_end(i), a.row_begin(p)+i ); 
            //std::cerr << "\nafter swapping a = \n" << a;
            const value_type factor = a[i][i];
            assert( factor != value_type() );
            //eliminate
            std::for_each( a.row_rbegin(i), a.row_rend(i)-i, [factor](value_type& x){ x/=factor; } );
            for ( size_type j = 0; j < n; ++j )
            {
                if ( i == j ) continue;
                const value_type ratio = a[j][i];
                std::transform( a.row_rbegin(j), a.row_rend(j)-i, a.row_rbegin(i), a.row_rbegin(j), 
                                [ratio](value_type x, value_type y){ return x - y * ratio; } );
            }
            //std::cerr << "\nafter elimination a = \n" << a;
        }
        
        return Matrix_Type( a, range_type( 0, n ), range_type( n, m+n ) );
    }

};

}//namespace dynamic

#endif

