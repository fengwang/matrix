#ifndef _DOT_HPP_INCLUDED_SFIOJ43089ADFOIJALSFKDJVCLKJSAFJSU4JOIJFDSAOIFFFDSAIALKASIJKLFSDAOIWSFKLJF
#define _DOT_HPP_INCLUDED_SFIOJ43089ADFOIJALSFKDJVCLKJSAFJSU4JOIJFDSAOIFFFDSAIALKASIJKLFSDAOIWSFKLJF

#include <matrix/matrix.hpp>

#include <numeric>
#include <cassert>

namespace feng
{
    template< typename Matrix1, typename Matrix2 >
    typename Matrix1::value_type
    dot( const Matrix1& m1, const Matrix2& m2 )
    {
        assert( m1.row() == m2.row() );
        assert( m1.col() == m2.col() );

        return std::inner_product( m1.begin(), m1.end(), m2.begin(), typename Matrix1::value_type(0) );
    }

}//namespace feng

#endif//_DOT_HPP_INCLUDED_SFIOJ43089ADFOIJALSFKDJVCLKJSAFJSU4JOIJFDSAOIFFFDSAIALKASIJKLFSDAOIWSFKLJF

