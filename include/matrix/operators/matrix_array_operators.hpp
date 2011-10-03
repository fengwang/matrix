#ifndef _MATRIX_ARRAY_OPERATORS_HPP_INCLUDED_DSFOIU4989RW8OISFDOIFSLKSFDJLKJVCXLKSLKJFSDLFSKOIEWEURSIDFUSLKJFEOIUFSDLIJDD
#define _MATRIX_ARRAY_OPERATORS_HPP_INCLUDED_DSFOIU4989RW8OISFDOIFSLKSFDJLKJVCXLKSLKJFSDLFSKOIEWEURSIDFUSLKJFEOIUFSDLIJDD

#include <matrix/matrix.hpp>

#include <cassert>
#include <cstddef>
#include <vector>
#include <valarray>
#include <numeric>

namespace dynamic
{

    //matrix -- vector
    //matrix -- valarray
    //matrix -- ptr

    // matrix<double>   m;
    // double*          p;
    // valarray<double> v;
    // vector<double>   a;
    // auto b = m * p;
    // auto b = m * v;
    // auto b = m * a;
    
    template<typename T, std::size_t D, typename A, typename T_>
    const std::valarray<T>
    operator * ( const matrix<T,D,A>& lhs,  const T_* const rhs )
    {   //TODO: in case of optimization, parallel here
        std::valarray<T> ans(lhs.row());
        for ( std::size_t i = 0; i < lhs.row(); ++i )
            ans[i] = std::inner_product( lhs.row_begin(i), lhs.row_end(i), rhs, T() );
        return ans;
    }
    
    template<typename T, std::size_t D, typename A, typename T_>
    const std::valarray<T>
    operator * ( const matrix<T,D,A>& lhs,  const std::valarray<T_>& rhs )
    {
        assert( lhs.col() == rhs.size() );
        return lhs * (&rhs[0]); 
    }
    
    template<typename T, std::size_t D, typename A, typename T_>
    const std::valarray<T>
    operator * ( const matrix<T,D,A>& lhs,  const std::vector<T_>& rhs )
    {
        assert( lhs.col() == rhs.size() );
        return lhs * (&rhs[0]); 
    }

}//namespace dynamic

#endif//_MATRIX_ARRAY_OPERATORS_HPP_INCLUDED_DSFOIU4989RW8OISFDOIFSLKSFDJLKJVCXLKSLKJFSDLFSKOIEWEURSIDFUSLKJFEOIUFSDLIJDD

