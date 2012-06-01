#ifndef _MATRIX_VALUE_OPERATORS_HPP_INCLUDED
#define _MATRIX_VALUE_OPERATORS_HPP_INCLUDED

#include <matrix/matrix.hpp>
#include <matrix/misc/eye.hpp>

#include <cassert>
#include <cstddef>
#include <algorithm>

/* operators overloading between Matrix and Value 
 *  
 * Examples:
 *  
 *    double d;
 *    matrix<double> m;
 *    std::size_t n;
 *
 *    matrix<double> m1 = d+m;
 *    matrix<double> m2 = m+d;
 *    matrix<double> m3 = d-m;
 *    matrix<double> m4 = m-d;
 *    matrix<double> m5 = d*m;
 *    matrix<double> m6 = m*d;
 *    m.resize( n, n ); //need square matrix
 *    matrix<double> m7 = d/m;
 *    matrix<double> m8 = m/d;
 *    matrix<double> m9 = d&&m;
 *    matrix<double> m10= m&&d;
 *    matrix<double> m11= d||m;
 *    matrix<double> m12= m||d;
 *    matrix<double> m13= m^2;
 *
 */

namespace feng{

template< typename T, std::size_t D, typename A, typename T_>
const matrix<T,D,A>
operator + ( const matrix<T,D,A>& lhs, const T_& rhs )
{
	matrix<T,D,A> ans( lhs );
	ans += rhs;
	return ans;
}

template< typename T, std::size_t D, typename A, typename T_>
const matrix<T,D,A>
operator + ( const T_& lhs, const matrix<T,D,A>& rhs )
{
	return rhs + lhs;
}

template< typename T, std::size_t D, typename A, typename T_>
const matrix<T,D,A>
operator - ( const matrix<T,D,A>& lhs, const T_& rhs )
{
	matrix<T,D,A> ans( lhs );
	ans -= rhs;
	return ans;
}

template< typename T, std::size_t D, typename A, typename T_>
const matrix<T,D,A>
operator - ( const T_& lhs, const matrix<T,D,A>& rhs )
{
	return - rhs + lhs;
}

template< typename T, std::size_t D, typename A, typename T_>
const matrix<T,D,A>
operator * ( const matrix<T,D,A>& lhs, const T_& rhs )
{
	matrix<T,D,A> ans( lhs );
	ans *= rhs;
	return ans;
}

template< typename T, std::size_t D, typename A, typename T_>
const matrix<T,D,A>
operator * ( const T_& lhs, const matrix<T,D,A>& rhs )
{
	return rhs * lhs;
}

template< typename T, std::size_t D, typename A, typename T_>
const matrix<T,D,A>
operator / ( const matrix<T,D,A>& lhs, const T_& rhs )
{
	matrix<T,D,A> ans( lhs );
	ans /= rhs;
	return ans;
}

template< typename T, std::size_t D, typename A, typename T_>
const matrix<T,D,A>
operator / ( const T_& lhs, const matrix<T,D,A>& rhs )
{
	return lhs * rhs.inverse();
}

template< typename T, std::size_t D, typename A, typename T_ >
const matrix<T,D,A>
operator || ( const matrix<T,D,A>& lhs, const T_& rhs )
{
	matrix<T,D,A> ans( lhs.row(), lhs.col()+1 );

	for ( std::size_t i = 0; i < lhs.row(); ++i )
		std::copy( lhs.row_begin(i), lhs.row_end(i), ans.row_begin(i) );	
	
	std::fill( ans.col_begin(lhs.col()), ans.col_end(lhs.col()), rhs );

	return ans;
}

template< typename T, std::size_t D, typename A, typename T_ >
const matrix<T,D,A>
operator || ( const T_& lhs, const matrix<T,D,A>& rhs )
{
	matrix<T,D,A> ans( rhs.row(), rhs.col()+1 );

	for ( std::size_t i = 0; i < lhs.row(); ++i )
		std::copy( lhs.row_begin(i), lhs.row_end(i), ans.row_begin(i)+1 );	

	std::fill( ans.col_begin(0), ans.col_end(0), rhs );
	
	return ans;
}

template< typename T, std::size_t D, typename A, typename T_ >
const matrix<T,D,A>
operator && ( const matrix<T,D,A>& lhs, const T_& rhs )
{
	matrix<T,D,A> ans( lhs.row()+1, lhs.col() );

	for ( std::size_t i = 0; i < lhs.row(); ++i )
		std::copy( lhs.row_begin(i), lhs.row_end(i), ans.row_begin(i) );	
	
	std::fill( ans.row_begin(lhs.row()), ans.row_end(lhs.row()), rhs );

	return ans;
}

template< typename T, std::size_t D, typename A, typename T_ >
const matrix<T,D,A>
operator && ( const T_& lhs, const matrix<T,D,A>& rhs )
{
	matrix<T,D,A> ans( rhs.row()+1, rhs.col() );

	for ( std::size_t i = 0; i < lhs.row(); ++i )
		std::copy( lhs.row_begin(i), lhs.row_end(i), ans.row_begin(i+1) );	

	std::fill( ans.row_begin(0), ans.row_end(0), rhs );
	
	return ans;
}

template< typename T, std::size_t D, typename A >
const matrix<T,D,A>
operator ^ ( const matrix<T,D,A>& lhs, std::size_t n )
{
    assert( lhs.row() == lhs.col() );

    auto const r = lhs.row();

    if ( 0 == n )
        return eye<T>( r, r );

    if ( 1 == n )
        return lhs;
    
    if ( n & 1 )
        return lhs^(n-1) * lhs;

    auto const lhs_2 = lhs^(n>>1);
    return lhs_2 * lhs_2;
}

}//namespace feng

#endif//_MATRIX_VALUE_OPERATORS_HPP_INCLUDED

