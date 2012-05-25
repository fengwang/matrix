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
            return ( key_type(r) << 32 ) | key_type(c);
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
            assert( 0 == ( key_type(row) >> 32 ) ); 
            assert( 0 == ( key_type(col) >> 32 ) ); 
        }

        sparse_matrix( const self_type& other ) : data_( other.data_ ), row_( other.row_ ), col_( other.col_ ) {}
        sparse_matrix( self_type&& other ) : data_( other.data_ ), row_( other.row_ ), col_( other.col_ ) {}

        const value_type& 
        operator() ( const size_type r, const size_type c ) const 
        {
            assert( row_ > r );
            assert( col_ > c );

            if ( auto it = data_.find(sparse_matrix_private::make_key()(r, c)) != data_.end() ) return *it;

            return value_type();
        }

        value_type& 
        operator() ( const size_type r, const size_type c )
        {
            assert( row_ > r );
            assert( col_ > c );

            return data_[sparse_matrix_private::make_key()(r, c)];
        }
        
    private:
        void remove_invalid()
        {
            data_.erase( std::remove_if( data_.begin(), data_.end(),[](wrapped_value_type p){ value_type() == p.second; } ), data_.end() );
        }

    public:
        self_type& 
        operator += ( const self_type& other )
        {
            std::for_each( other.data_.begin(), other.data_.end(), [this](wrapped_value_type p)
                            { 
                                ((*this).data_)[p.first]+= p.second; 
                                if ( value_type() == ((*this).data_)[p.first] ) ((*this).data_).erase(p.first);
                            } 
                         );
            return *this;
        }

        self_type& 
        operator -= ( const self_type& other )
        {
            std::for_each( other.data_.begin(), other.data_.end(), [this](wrapped_value_type p)
                            { 
                                ((*this).data_)[p.first]-= p.second; 
                                if ( value_type() == ((*this).data_)[p.first] ) ((*this).data_).erase(p.first);
                            } 
                         );
            return *this;
        }

        self_type& 
        operator *= ( const value_type v ) 
        {
            std::for_each( data_.begin(), data_.end(), [v](wrapped_value_type& p) { p.second *= v; } );
            return *this;
        }

        self_type& 
        operator /= ( const value_type v ) 
        {
            std::for_each( data_.begin(), data_.end(), [v](wrapped_value_type& p) { p.second /= v; } );
            return *this;
        }

    public:
        self_type&
        operator *= ( const self_type& other )
        {
            assert( col_ == other.row_ );

            self_type ans( row_, other.col_ );
            
            // !!!! TODO: parallel here !!!!
            std::for_each( data_.begin(), data_.end(), [&](wrapped_value_type& p)
                           {
                            std::for_each( other.data_.begin(), other.data_.end(), [&](wrapped_value_type pp)
                                           {
                                            if ( sparse_matrix_private::isolate_col()(p.first) == sparse_matrix_private::isolate_row()(pp.first) )
                                                ans.data_[sparse_matrix_private::make_key()(sparse_matrix_private::isolate_row()(p.first), sparse_matrix_private::isolate_col()(pp.first))] += p.second * pp.second;
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
            std::for_each( ans.begin(), ans.end(), []( wrapped_value_type& p ) { p.second = - p.second; } );
            return std::move(ans);
        }

        const self_type 
        operator + () 
        {
            self_type ans( *this );
            return std::move(ans);
        }
    
    public:
        size_type row() const { return row_; }
        size_type col() const { return col_; }

        bool empty() const { return data_.empty(); }
        bool size() const { return data_.size(); }
        void clear() { data_.clear(); }

        const self_type
        transpose() const 
        {
            self_type ans(col(), row());
            for( auto i = data_.begin(); i != data_.end(); ++i )
                ans.insert( ((((*i).first)<<32)>>32), ((*i).first)>>32, (*i).second ); 
            return std::move( ans );
        }

        void insert( const size_type r, const size_type c, const value_type v )
        {
            assert( row_ > r );
            assert( col_ > c );

            const key_type key = sparse_matrix_private::make_key()(r, c);
            data_[key] = v;
            if ( value_type() == v ) data_.erase( key );
        }

        void resize( const size_type r, const size_type c )
        {
            if ( r < row_ )
                data_.erase( std::remove_if( data_.begin(), data_.end(),[r](wrapped_value_type& p){ ((p.first)>>32) > r; } ), data_.end() );
            if ( c < col_ )
                data_.erase( std::remove_if( data_.begin(), data_.end(),[c](wrapped_value_type& p){ ((p.first)&0xFFFFFFFFUL) > c; } ), data_.end() );

            row_ = r;
            col_ = c;
        }

        void swap( self_type& other )
        {
            data_.swap(other.data_);
            row_ ^= other.row_; other.row_ ^= row_; row_ ^= other.row_;
            col_ ^= other.col_; other.col_ ^= col_; col_ ^= other.col_;
        }

    friend const self_type // A+B
        operator + ( const self_type& lhs, const self_type& rhs )
        {
            self_type ans( lhs );
            ans += rhs;
            return std::move( ans );
        }

    friend const self_type // A-B
        operator - ( const self_type& lhs, const self_type& rhs )
        {
            self_type ans( lhs );
            ans -= rhs;
            return std::move( ans );
        }

    friend const self_type // A*b
        operator * ( const self_type& lhs, const value_type rhs )
        {
            self_type ans( lhs );
            ans *= rhs;
            return std::move( ans );
        }

    friend const self_type // A/b
        operator / ( const self_type& lhs, const value_type rhs )
        {
            self_type ans( lhs );
            ans /= rhs;
            return std::move( ans );
        }

    friend const self_type // AB
        operator * ( const self_type& lhs, const self_type& rhs )
        {
            self_type ans( lhs );
            ans *= rhs;
            return std::move( ans );
        }

    friend const std::vector<value_type> // Ax
        operator * ( const self_type& lhs, const std::vector<value_type>& rhs )
        {
            assert( rhs.size() == lhs.col_ );
            std::vector<value_type> ans( lhs.row_ ); 
            lhs.multiply_array( rhs.begin(), rhs.end(), ans.begin() );
            return std::move( ans );
        }

    friend const std::valarray<value_type> 
        operator * ( const self_type& lhs, const std::valarray<value_type>& rhs )
        {
            assert( rhs.size() == lhs.col_ );
            std::valarray<value_type> ans( lhs.row_ ); 

            lhs.multiply_array( &rhs[0], &rhs[lhs.col_], &ans[0] );

            return std::move( ans );
        }
    friend std::ostream&
        operator << ( std::ostream& lhs, const self_type& rhs )
        {
            std::for_each( rhs.data_.begin(), rhs.data_.end(), [&](wrapped_value_type p)
                           { 
                            lhs << "(" << sparse_matrix_private::isolate_row()(p.first) << ", " << sparse_matrix_private::isolate_col()(p.first) << ") = " << p.second << "\n"; 
                           } 
                         );
            return lhs;
        }

        //dense matrix multiply sparse matrix
        template< std::size_t D, typename A >
    friend const matrix<T,D,A>
        operator * ( const matrix<T,D,A>& lhs, const sparse_matrix<T>& rhs )
        {
            assert( lhs.col() == rhs.row() );
            //TODO: this method only works for square matrices, rewrite when have time
            return (rhs.transpose() * lhs.transpose()).transpose();        
        }

        //sparse matrix multiply dense matrix
        template< std::size_t D, typename A >
    friend const matrix<T,D,A>
        operator * ( const sparse_matrix<T>& lhs, const matrix<T,D,A>& rhs )
        {
            assert( lhs.col() == rhs.row() );
            matrix<T,D,A> ans( lhs.row(), rhs.col() );
            
            for ( std::size_t i = 0; i < rhs.col(); ++i )
                lhs.multiply_array( rhs.col_begin(i), rhs.col_end(i), ans.col_begin(i), ans.col_end(i) );
            
            return ans;
        }

    private:

        template< typename Itor, typename Otor >
        Otor 
        multiply_array( Itor in_first, Itor in_last, Otor out_first, Otor out_last ) const 
        {
            size_type index = 0;
            auto last_head = data_.begin();
            while ( out_first != out_last )
            {   //find the first one whose row is index, from the start, and save the result to head
                auto head = std::find_if( last_head, data_.end(), [index](wrapped_value_type p) { return index == sparse_matrix_private::isolate_row()(p.first); } );
                //find the first one whose row is not index, from the head, and save the result to tail
                auto tail = std::find_if( head, data_.end(), [index](wrapped_value_type p) { return index != sparse_matrix_private::isolate_row()(p.first); } );
                //if no one whose row is index found, then set the output to 0, and continue the loop
                if ( data_.end() == head ) { *out_first++ = value_type(); last_head = data_.begin(); continue; }
                //a simple inner product to generate the output value
                auto sum = value_type();//                                                  the n th  element in the array                                  sparse value
                std::for_each( head, tail, [&sum, in_first](wrapped_value_type p ) { sum += (*( in_first + sparse_matrix_private::isolate_col()(p.first) )) * p.second; } );
                *out_first++ = sum;
                ++index;
            }

            return out_first;
        }

        // multiply array 
        template< typename Itor, typename Otor >
        Otor multiply_array( Itor in_first, Itor in_last, Otor out ) const 
        {
            static const auto block_multiply = [&,this](size_type index_first, size_type index_last) 
            {
                auto head = std::find_if( (*this).data_.begin(), (*this).data_.end(), [index_first](wrapped_value_type p) { return index_first == sparse_matrix_private::isolate_row()(p.first); } );
                auto tail = head;

                for ( auto i = index_first; i != index_last; ++i )
                {
                    head = tail;
                    tail = std::find_if( head, (*this).data_.end(), [i](wrapped_value_type p) { return i+1 == sparse_matrix_private::isolate_row()(p.first); } );
                    if ( std::distance( head, tail ) == 0 ) { *out++ = 0; continue; }
                    auto tmp = value_type();
                    std::for_each( head, tail, [&tmp, in_first](wrapped_value_type p ) { tmp += (*( in_first + sparse_matrix_private::isolate_col()(p.first) )) * p.second; } );
                    *(out+i) = tmp;                            
                }
            };

            const size_type length = std::distance( in_first, in_last );
            assert( col_ == length );
            
#if 1
            //!!non-parallel impl
            block_multiply( size_type(), length );
#endif 

#if 0
            //!!parallel impl
            parallel_organizer()( block_multiply, size_type(), size_type(std::distance( in_first, in_last )) );            
#endif

#if 0
            //!!concret parallel impl
            size_type cpu_number = std::thread::hardware_concurrency() > 4 ? std::thread::hardware_concurrency() : 4;
            const size_type thread_number = cpu_number - 1;
            std::thread concurrency_thread[ thread_number ];
            
            const size_type block_size = length / thread_number;

            size_type block_first = 0;
            size_type block_last = 0;

            for ( size_type index = 0; index != thread_number; ++index )
            {
                block_last += block_size;
                concurrency_thread[index] = std::thread( block_multiply, block_first, block_last );
                block_first = block_last;
            }
            block_multiply( block_last, length );

            std::for_each( concurrency_thread, concurrency_thread+thread_number, [](std::thread& t){t.join();});
#endif       

            return out; 

        }
        // multiply array 

    };//sparse_matrix 
}//namespace feng

#endif//_SPARSE_MATRIX_HPP_INCLUDED_SDLKJ4OIJSDFEKLJSDOUJ4LKJSDFLKJSDFLKMNCLKJDFLKJSOIJULKJDFOIJ4LKJSFDOIJFJ

