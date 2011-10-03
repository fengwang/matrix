#ifndef _MATRIX_MATRIX_OPERATORS_HPP_INCLUDED
#define _MATRIX_MATRIX_OPERATORS_HPP_INCLUDED

#include <cassert>
#include <algorithm>
#include <numeric>
#include <functional>

#include <matrix/matrix.hpp>

/*operators overloading between matrix and matrix
 *  
 *Examples:
 * 
 * matrix<double> m1;
 * matrix<double> m2;
 *
 * auto a1 = m1 + m2;
 * auto a2 = m1 - m2;
 * auto a3 = m1 * m2;
 * auto a4 = m1 / m2;
 * auto a5 = m1 > m2;
 * auto a6 = m1 >= m2;
 * auto a7 = m1 < m2;
 * auto a8 = m1 <= m2;
 * auto a9 = m1 == m2;
 * auto a10= m1 && m2;
 * auto a11= m1 || m2;
 *
 */

namespace dynamic{

template< typename T1, std::size_t D1, typename A1, typename T2, std::size_t D2, typename A2 >
const matrix<T1,D1,A1> 
operator + ( const matrix<T1,D1,A1>& lhs, const matrix<T2,D2,A2>& rhs )
{
	matrix<T1,D1,A1> ans(lhs);
	ans += rhs;
	return ans;
}

template< typename T1, std::size_t D1, typename A1, typename T2, std::size_t D2, typename A2 >
const matrix<T1,D1,A1> 
operator - ( const matrix<T1,D1,A1>& lhs, const matrix<T2,D2,A2>& rhs )
{
	matrix<T1,D1,A1> ans(lhs);
	ans -= rhs;
	return ans;
}

template< typename T1, std::size_t D1, typename A1, typename T2, std::size_t D2, typename A2 >
const matrix<T1,D1,A1> 
operator * ( const matrix<T1,D1,A1>& lhs, const matrix<T2,D2,A2>& rhs )
{
	matrix<T1,D1,A1> ans(lhs);
	ans *= rhs;
	return ans;
}

template< typename T1, std::size_t D1, typename A1, typename T2, std::size_t D2, typename A2 >
const matrix<T1,D1,A1> 
operator / ( const matrix<T1,D1,A1>& lhs, const matrix<T2,D2,A2>& rhs )
{
	matrix<T1,D1,A1> ans(lhs);
	ans /= rhs;
	return ans;
}

template< typename T1, std::size_t D1, typename A1, typename T2, std::size_t D2, typename A2 >
bool
operator < ( const matrix<T1,D1,A1>& lhs, const matrix<T2,D2,A2>& rhs )
{
	assert( lhs.row() == rhs.row() );
	assert( lhs.col() == rhs.col() );

	return std::lexicographical_compare( lhs.begin(), lhs.end(), rhs.begin(), rhs.end() );
}

template< typename T1, std::size_t D1, typename A1, typename T2, std::size_t D2, typename A2 >
bool 
operator == ( const matrix<T1,D1,A1>& lhs, const matrix<T2,D2,A2>& rhs )
{
	assert( lhs.row() == rhs.row() );
	assert( lhs.col() == rhs.col() );

	return std::equal( lhs.begin(), lhs.end(), rhs.begin() );
}

template< typename T1, std::size_t D1, typename A1, typename T2, std::size_t D2, typename A2 >
bool 
operator > ( const matrix<T1,D1,A1>& lhs, const matrix<T2,D2,A2>& rhs )
{
	return !( (lhs < rhs) || (lhs == rhs) );
}

template< typename T1, std::size_t D1, typename A1, typename T2, std::size_t D2, typename A2 >
bool 
operator >= ( const matrix<T1,D1,A1>& lhs, const matrix<T2,D2,A2>& rhs )
{
	return !( lhs < rhs );
}

template< typename T1, std::size_t D1, typename A1, typename T2, std::size_t D2, typename A2 >
bool 
operator <= ( const matrix<T1,D1,A1>& lhs, const matrix<T2,D2,A2>& rhs )
{
	return !( lhs > rhs );
}


//x[N][x+y] <- a[N][x] || b[N][y]
template< typename T1, std::size_t D1, typename A1, typename T2, std::size_t D2, typename A2 >
const matrix<T1,D1,A1> 
operator || ( const matrix<T1,D1,A1>& lhs, const matrix<T2,D2,A2>& rhs )
{
	assert( lhs.row() == rhs.row() );

	typedef matrix<T1,D1,A1> matrix_type;
	typedef typename matrix_type :: size_type size_type;

	const size_type row = lhs.row();
	const size_type col = lhs.col() + rhs.col();

	matrix_type ans( row, col );

	for ( size_type i = 0; i < row; ++i )
	{
		std::copy( lhs.row_begin(i), lhs.row_end(i), ans.row_begin(i));
		std::copy( rhs.row_begin(i), rhs.row_end(i), ans.row_begin(i)+lhs.col() );
	}

	return ans;
}

//x[x+y][M] <- a[x][M] && b[y][M]
template< typename T1, std::size_t D1, typename A1, typename T2, std::size_t D2, typename A2 >
const matrix<T1,D1,A1> 
operator && ( const matrix<T1,D1,A1>& lhs, const matrix<T2,D2,A2>& rhs )
{
	assert( lhs.col() == rhs.col() );

	typedef matrix<T1,D1,A1> matrix_type;
	typedef typename matrix_type :: size_type size_type;

	const size_type row = lhs.row() + rhs.row();
	const size_type col = lhs.col();

	matrix_type ans( row, col );

	for ( size_type i = 0; i < col; ++i )
	{
		std::copy( lhs.col_begin(i), lhs.col_end(i), ans.col_begin(i));
		std::copy( rhs.col_begin(i), rhs.col_end(i), ans.col_begin(i)+lhs.row() );
	}

	return ans;
}

}//namespace dynamic

#endif//_MATRIX_MATRIX_OPERATORS_HPP_INCLUDED

