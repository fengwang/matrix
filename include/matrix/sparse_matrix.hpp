#ifndef _SPARSE_MATRIX_HPP_INCLUDED_SDLKJ5OIJSDFEKLJSDOUJ4LKJSDFLKJSDFLKMNCLKJDFLKJSOIJULKJDFOIJ4LKJSFDOIJFJ
#define _SPARSE_MATRIX_HPP_INCLUDED_SDLKJ5OIJSDFEKLJSDOUJ4LKJSDFLKJSDFLKMNCLKJDFLKJSOIJULKJDFOIJ4LKJSFDOIJFJ

#include <matrix/matrix.hpp>

#include <cstddef>
#include <cassert>
#include <vector>
#include <valarray>
#include <map>
#include <algorithm>
#include <thread>
#include <iostream>

namespace feng
{

    namespace sparse_matrix_private
    {

        struct make_key
        {
            typedef std::size_t size_type;
            typedef unsigned long long key_type;

            key_type operator() ( const size_type r, const size_type c ) const
            {
                return ( key_type( r ) << 32 ) | key_type( c );
            }
        };

        struct isolate_row
        {
            typedef make_key::size_type size_type;
            typedef make_key::key_type key_type;
            size_type operator()( const key_type k ) const
            {
                return k >> 32;
            }
        };

        struct isolate_col
        {
            typedef make_key::size_type size_type;
            typedef make_key::key_type key_type;
            size_type operator()( const key_type k ) const
            {
                return k & 0xFFFFFFFFUL ;
            }
        };
    }//namespace sparse_matrix_private

    template< typename T >
    struct sparse_matrix
    {
            typedef sparse_matrix self_type;

            typedef T value_type;
            typedef sparse_matrix_private::make_key::size_type size_type;
            typedef sparse_matrix_private::make_key::key_type key_type;
            typedef std::map<key_type, value_type> coordinate_value_associate_type;
            typedef typename coordinate_value_associate_type::value_type wrapped_value_type;

        private:
            coordinate_value_associate_type data_;

            size_type row_;
            size_type col_;

        public:
            sparse_matrix( const size_type row = 10, const size_type col = 10 ) : row_( row ), col_( col )
            {
                assert( 0 == ( key_type( row ) >> 32 ) );
                assert( 0 == ( key_type( col ) >> 32 ) );
            }

            sparse_matrix( const self_type& other ) : data_( other.data_ ), row_( other.row_ ), col_( other.col_ ) {}
            sparse_matrix( self_type && other ) : data_( other.data_ ), row_( other.row_ ), col_( other.col_ ) {}

            const value_type&
            operator() ( const size_type r, const size_type c ) const
            {
                assert( row_ > r );
                assert( col_ > c );
                if ( auto it = data_.find( sparse_matrix_private::make_key()( r, c ) ) != data_.end() )
                    return *it;
                return value_type();
            }

            value_type&
            operator() ( const size_type r, const size_type c )
            {
                assert( row_ > r );
                assert( col_ > c );
                return data_[sparse_matrix_private::make_key()( r, c )];
            }

        private:
            void remove_invalid()
            {
                data_.erase( std::remove_if( data_.begin(), data_.end(), []( wrapped_value_type p )
                {
                    value_type() == p.second;
                } ), data_.end() );
            }

        public:
            self_type&
            operator += ( const self_type& other )
            {
                std::for_each( other.data_.begin(), other.data_.end(), [this]( wrapped_value_type p )
                {
                    ( ( *this ).data_ )[p.first] += p.second;
                    if ( value_type() == ( ( *this ).data_ )[p.first] )
                        ( ( *this ).data_ ).erase( p.first );
                }
                             );
                return *this;
            }

            self_type&
            operator -= ( const self_type& other )
            {
                std::for_each( other.data_.begin(), other.data_.end(), [this]( wrapped_value_type p )
                {
                    ( ( *this ).data_ )[p.first] -= p.second;
                    if ( value_type() == ( ( *this ).data_ )[p.first] )
                        ( ( *this ).data_ ).erase( p.first );
                }
                             );
                return *this;
            }

            self_type&
            operator *= ( const value_type v )
            {
                std::for_each( data_.begin(), data_.end(), [v]( wrapped_value_type & p )
                {
                    p.second *= v;
                } );
                return *this;
            }

            self_type&
            operator /= ( const value_type v )
            {
                std::for_each( data_.begin(), data_.end(), [v]( wrapped_value_type & p )
                {
                    p.second /= v;
                } );
                return *this;
            }

        public:
            self_type&
            operator *= ( const self_type& other )
            {
                assert( col_ == other.row_ );
                self_type ans( row_, other.col_ );
                // !!!! TODO: parallel here !!!!
                std::for_each( data_.begin(), data_.end(), [&]( wrapped_value_type & p )
                {
                    std::for_each( other.data_.begin(), other.data_.end(), [&]( wrapped_value_type pp )
                    {
                        if ( sparse_matrix_private::isolate_col()( p.first ) == sparse_matrix_private::isolate_row()( pp.first ) )
                            ans.data_[sparse_matrix_private::make_key()( sparse_matrix_private::isolate_row()( p.first ), sparse_matrix_private::isolate_col()( pp.first ) )] += p.second * pp.second;
                    }
                                 );// foreach other
                }
                             );// foreach *this
                swap( ans );
                //remove_invalid();
                return *this;
            }

        public:
            const self_type
            operator - ()
            {
                self_type ans( *this );
                std::for_each( ans.begin(), ans.end(), []( wrapped_value_type & p )
                {
                    p.second = - p.second;
                } );
                return ans;
            }

            const self_type
            operator + ()
            {
                self_type ans( *this );
                return ans;
            }

        public:
            size_type row() const
            {
                return row_;
            }
            size_type col() const
            {
                return col_;
            }

            bool empty() const
            {
                return data_.empty();
            }
            bool size() const
            {
                return data_.size();
            }
            void clear()
            {
                data_.clear();
            }

            const self_type
            transpose() const
            {
                self_type ans( col(), row() );
                for ( auto i = data_.begin(); i != data_.end(); ++i )
                    ans.insert( ( ( ( ( *i ).first ) << 32 ) >> 32 ), ( ( *i ).first ) >> 32, ( *i ).second );
                return ( ans );
            }

            void insert( const size_type r, const size_type c, const value_type v )
            {
                assert( row_ > r );
                assert( col_ > c );
                if ( value_type() == v )
                    return;
                const key_type key = sparse_matrix_private::make_key()( r, c );
                data_.insert( std::make_pair( key, v ) );
                //data_[key] = v;
            }

            void resize( const size_type r, const size_type c )
            {
                if ( r < row_ )
                    data_.erase( std::remove_if( data_.begin(), data_.end(), [r]( wrapped_value_type & p )
                {
                    ( ( p.first ) >> 32 ) > r;
                } ), data_.end() );
                if ( c < col_ )
                    data_.erase( std::remove_if( data_.begin(), data_.end(), [c]( wrapped_value_type & p )
                {
                    ( ( p.first ) & 0xFFFFFFFFUL ) > c;
                } ), data_.end() );
                row_ = r;
                col_ = c;
            }

            void swap( self_type& other )
            {
                data_.swap( other.data_ );
                row_ ^= other.row_;
                other.row_ ^= row_;
                row_ ^= other.row_;
                col_ ^= other.col_;
                other.col_ ^= col_;
                col_ ^= other.col_;
            }

            friend const self_type // A+B
            operator + ( const self_type& lhs, const self_type& rhs )
            {
                self_type ans( lhs );
                ans += rhs;
                return ( ans );
            }

            friend const self_type // A-B
            operator - ( const self_type& lhs, const self_type& rhs )
            {
                self_type ans( lhs );
                ans -= rhs;
                return ( ans );
            }

            friend const self_type // A*b
            operator * ( const self_type& lhs, const value_type rhs )
            {
                self_type ans( lhs );
                ans *= rhs;
                return ( ans );
            }

            friend const self_type // A/b
            operator / ( const self_type& lhs, const value_type rhs )
            {
                self_type ans( lhs );
                ans /= rhs;
                return ( ans );
            }

            friend const self_type // AB
            operator * ( const self_type& lhs, const self_type& rhs )
            {
                self_type ans( lhs );
                ans *= rhs;
                return ( ans );
            }

            friend const std::vector<value_type> // Ax
            operator * ( const self_type& lhs, const std::vector<value_type>& rhs )
            {
                assert( rhs.size() == lhs.col_ );
                std::vector<value_type> ans( lhs.row_ );
                lhs.multiply_array( rhs.begin(), rhs.end(), ans.begin() );
                return ( ans );
            }

            friend const std::valarray<value_type>
            operator * ( const self_type& lhs, const std::valarray<value_type>& rhs )
            {
                assert( rhs.size() == lhs.col_ );
                std::valarray<value_type> ans( lhs.row_ );
                lhs.multiply_array( &rhs[0], &rhs[lhs.col_], &ans[0] );
                return ( ans );
            }
            friend std::ostream&
            operator << ( std::ostream& lhs, const self_type& rhs )
            {
                std::for_each( rhs.data_.begin(), rhs.data_.end(), [&]( wrapped_value_type p )
                {
                    lhs << "(" << sparse_matrix_private::isolate_row()( p.first ) << ", " << sparse_matrix_private::isolate_col()( p.first ) << ") = " << p.second << "\n";
                }
                             );
                return lhs;
            }

            //dense matrix multiply sparse matrix
            template< std::size_t D, typename A >
            friend const matrix<T, D, A>
            operator * ( const matrix<T, D, A>& lhs, const sparse_matrix<T>& rhs )
            {
                //dimension check
                assert( lhs.col() == rhs.row() );
                matrix<T, D, A> ans( lhs.row(), rhs.col() );
                for ( std::size_t i = 0; i < lhs.row(); ++i )
                    // the ith row of left side dense matrix, say x
                    // compute x multiply every column of the right side sparse matrix,
                    // the generated result is the ith row of the ans
                    rhs.left_multiply_array( lhs.row_begin( i ), lhs.row_end( i ), ans.row_begin( i ) );
                return ans;
            }

            //sparse matrix multiply dense matrix
            template< std::size_t D, typename A >
            friend const matrix<T, D, A>
            operator * ( const sparse_matrix<T>& lhs, const matrix<T, D, A>& rhs )
            {
                //dimension check
                assert( lhs.col() == rhs.row() );
                matrix<T, D, A> ans( lhs.row(), rhs.col() );
                for ( std::size_t i = 0; i < rhs.col(); ++i )
                    lhs.multiply_array( rhs.col_begin( i ), rhs.col_end( i ), ans.col_begin( i ) );
                return ans;
            }

        private:
            // Inputs:
            //          in_first :      the first pos of the left array
            //          in_last  :      the last pos of the left array
            //          out_first:      the first pos of the output array
            // Return:
            //          the last pos of the output array
            // Function:
            //          compute blas level 2
            //                      x * A
            //          where  x is an array whose size is (1 , m)
            //                 A is the current sparse matrix, whose size is (m, n)
            //          then put forward the result to the output iterator
            template< typename Itor, typename Otor >
            Otor // x*A
            left_multiply_array( Itor in_first, Itor in_last, Otor out_first ) const
            {
                //dimension check
                assert( std::distance( in_first, in_last ) == row() );
                //create an empty array
                std::valarray<value_type> b( value_type( 0 ), col() );
                //for each elements in data_
                for ( auto it = data_.begin(); it != data_.end(); ++it )
                {
                    // the 'cth' value of the result array is generated by all the elements in sparse matrix's 'cth' column multiplying array x
                    auto const& wrapped_value = *it;
                    auto const r = sparse_matrix_private::isolate_row()( wrapped_value.first );
                    auto const c = sparse_matrix_private::isolate_col()( wrapped_value.first );
                    b[c] += wrapped_value.second * *( in_first + r );
                }
                std::copy( std::begin( b ), std::end( b ), out_first );
                return out_first;
            }

            // Funciton:
            //          calculate y = A*x
            //          where A is *this, a sparse matrixwith size (M,N),
            //          x is an array with size N
            //          y is the calculation result, an array with size M
            // Input:
            //          in_first        the first pos of the array x
            //          in_last         the last pos of the array x
            //          out_first       the first pos of the result y
            // Return:
            //          the last pos of y
            template< typename Itor, typename Otor >
            Otor // A*x
            multiply_array( Itor in_first, Itor in_last, Otor out_first ) const
            {
                //dimension check
                assert( std::distance( in_first, in_last ) == col() );
                //create an empty array
                std::valarray<value_type> b( value_type( 0 ), row() );
                //for each elements in data_
                for ( auto it = data_.begin(); it != data_.end(); ++it )
                {
                    // the 'cth' value in array x multiplies the sparse matrix element with index [r][c],
                    // then accumulate all the results to the 'rth' value of the target array y
                    // or simplily, the 'rth' value of array y is generated by all the elements in sparse matrix row 'r' multiplying array x
                    auto const& wrapped_value = *it;
                    auto const r = sparse_matrix_private::isolate_row()( wrapped_value.first );
                    auto const c = sparse_matrix_private::isolate_col()( wrapped_value.first );
                    b[r] += wrapped_value.second * *( in_first + c );
                }
                //forward the result to output itertor
                std::copy( std::begin( b ), std::end( b ), out_first );
                return out_first;
            }
    };//sparse_matrix
}//namespace feng

#endif//_SPARSE_MATRIX_HPP_INCLUDED_SDLKJ4OIJSDFEKLJSDOUJ4LKJSDFLKJSDFLKMNCLKJDFLKJSOIJULKJDFOIJ4LKJSFDOIJFJ

