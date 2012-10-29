/* 
 * File:   matrix_impl.hpp
 * Author: feng
 *
 * Created on October 2, 2009, 4:05 PM
 */
#ifndef _DYNAMIC_MATRIX_IMPL_HPP_INCLUDED
#define _DYNAMIC_MATRIX_IMPL_HPP_INCLUDED

#include <matrix/impl/matrix_allocator.hpp>
#include <matrix/impl/matrix_buffer.hpp>
#include <matrix/impl/matrix_range_iterator.hpp>
#include <matrix/impl/matrix_stride_iterator.hpp>
#include <matrix/impl/misc.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef> 
#include <functional>
#include <iosfwd>
#include <iterator>
#include <memory>
#include <numeric>
#include <utility>
#include <valarray>
#include <vector>

namespace feng
{

template<   typename Type, std::size_t Default = 256,
            class Allocator = std::allocator<typename remove_const<typename remove_reference<Type>::result_type>::result_type>
        >
class matrix
{

public:
    typedef typename remove_reference<Type>::result_type                value_type1;
    typedef typename remove_const<value_type1>::result_type             value_type;
    typedef matrix                                                      self_type;
    typedef value_type*                                                 iterator;
    typedef const value_type*                                           const_iterator;
    
    typedef matrix_buffer<value_type, Default, Allocator>               storage_type;
    typedef std::size_t                                                 size_type;
    typedef std::ptrdiff_t                                              difference_type;
    typedef range                                                       range_type;

    //stride iterators
    typedef matrix_stride_iterator<value_type*>                         stride_iterator;
    typedef matrix_stride_iterator<value_type*>                         row_type;
    typedef matrix_stride_iterator<const value_type*>                   const_row_type;
    
    typedef matrix_stride_iterator<value_type*>                         col_type;
    typedef matrix_stride_iterator<const value_type*>                   const_col_type;
    
    typedef matrix_stride_iterator<value_type*>                         diag_type;
    typedef matrix_stride_iterator<const value_type*>                   const_diag_type;

    //for anti diagonal iterator
    typedef matrix_stride_iterator<value_type*>                         anti_diag_type;
    typedef matrix_stride_iterator<const value_type*>                   const_anti_diag_type;

    typedef std::reverse_iterator<iterator>                             reverse_iterator;
    typedef std::reverse_iterator<const_iterator>                       const_reverse_iterator;

    typedef std::reverse_iterator<stride_iterator>                      reverse_stride_iterator;

    typedef std::reverse_iterator<row_type>                             reverse_row_type;
    typedef std::reverse_iterator<const_row_type>                       const_reverse_row_type;
    
    typedef std::reverse_iterator<col_type>                             reverse_col_type;
    typedef std::reverse_iterator<const_col_type>                       const_reverse_col_type;
    
    typedef std::reverse_iterator<diag_type>                            reverse_upper_diag_type;
    typedef std::reverse_iterator<const_diag_type>                      const_reverse_upper_diag_type;
    
    typedef std::reverse_iterator<diag_type>                            reverse_lower_diag_type;
    typedef std::reverse_iterator<const_diag_type>                      const_reverse_lower_diag_type;
    
    typedef std::reverse_iterator<diag_type>                            reverse_diag_type;
    typedef std::reverse_iterator<const_diag_type>                      const_reverse_diag_type;

    typedef std::reverse_iterator<anti_diag_type>                       reverse_anti_diag_type;
    typedef std::reverse_iterator<const_anti_diag_type>                 const_reverse_anti_diag_type;

    //range iterators
    typedef matrix_range_iterator<row_type>                             row_range_type;
    typedef matrix_range_iterator<const_row_type>                       const_row_range_type;
    
    typedef matrix_range_iterator<col_type>                             col_range_type;
    typedef matrix_range_iterator<const_col_type>                       const_col_range_type;
    
    typedef std::reverse_iterator<row_range_type>                       reverse_row_range_type;
    typedef std::reverse_iterator<const_row_range_type>                 const_reverse_row_range_type;
    
    typedef std::reverse_iterator<col_range_type>                       reverse_col_range_type;
    typedef std::reverse_iterator<const_col_range_type>                 const_reverse_col_range_type;

public:
    explicit matrix( const size_type r = 0, const size_type c = 0, const value_type v = value_type() ) 
    : row_(r), col_(c), data_(storage_type(r*c)) 
    { 
        std::fill( begin(), end(), v ); 
    }

    explicit matrix( const size_type n ) 
    : row_(n), col_(n), data_(storage_type(n*n)) 
    {}

    ~matrix() { }
    
    matrix(const self_type& rhs)
    {
        operator=(rhs);
    }

    self_type & operator=(const self_type& rhs)
    {
        do_copy(rhs);
        return *this;
    }

    template<typename T, size_type D, typename A>
    matrix(const matrix<T,D,A>& rhs)
    {
        operator=<T,D,A>(rhs);
    }

    template<typename T, size_type D, typename A>
    self_type & operator=(const matrix<T,D,A>& rhs)
    {
        do_copy<T,D,A> (rhs);
        return *this;
    }

public:
    matrix( self_type&& ) = default;
    self_type& operator = ( self_type&& ) = default;

public:
    self_type& operator = ( const value_type & v )
    {
        std::fill( begin(), end(), v );
        return *this;
    }

public:
    matrix (    const self_type& other, const range_type& rr, const range_type& rc )
        :   row_( rr.second - rr.first ), col_( rc.second - rc.first ), data_(storage_type((rr.second-rr.first)*(rc.second-rc.first))) 
    {
        assert( rr.second > rr.first ); 
        assert( rc.second > rc.first );
        assert( rr.second <= other.row() );
        assert( rc.second <= other.col() );
    
        for ( size_type i = rr.first; i < rr.second; ++i )
            std::copy(  other.row_begin(i)+rc.first, other.row_begin(i)+rc.second, row_begin(i-rr.first));
    }

    template< typename T, size_type D, typename A >
    matrix ( const matrix<T,D,A>& other, const range_type& rr, const range_type& rc )
        :   row_( rr.second - rr.first ), col_( rc.second - rc.first ), data_(storage_type((rr.second-rr.first)*(rc.second-rc.first))) 
    {
        assert( rr.second > rr.first ); 
        assert( rc.second > rc.first );
        assert( rr.second < other.row() );
        assert( rc.second < other.col() );
    
        for ( size_type i = rr.first; i < rr.second; ++i )
            std::copy(  other.row_begin(i)+rc.first, other.row_begin(i)+rc.second, row_begin(i-rr.first));
    }

public:
    template< typename Type1, typename... Types >
    self_type& import( iterator it, const Type1& value1, const Types&... values ) 
    {
        import( it++, value1 );
        return import( it, values... );
    }
    
    self_type& import( iterator )
    {
        return *this;
    }

    template< typename Type1 >
    self_type& import( iterator it, const Type1& value1 )
    {
        *it = value1;
        return *this;
    }

    template< typename Type1, typename... Types >
    self_type& import( reverse_iterator it, const Type1& value1, const Types&... values ) 
    {
        import( it++, value1 );
        return import( it, values... );
    }
    
    self_type& import( reverse_iterator )
    {
        return *this;
    }

    template< typename Type1 >
    self_type& import( reverse_iterator it, const Type1& value1 )
    {
        *it = value1;
        return *this;
    }

    template< typename Type1, typename... Types >
    self_type& import( stride_iterator it, const Type1& value1, const Types&... values ) 
    {
        import( it++, value1 );
        return import( it, values... );
    }
    
    self_type& import( stride_iterator )
    {
        return *this;
    }

    template< typename Type1 >
    self_type& import( stride_iterator it, const Type1& value1 )
    {
        *it = value1;
        return *this;
    }

    template< typename Type1, typename... Types >
    self_type& import( reverse_stride_iterator it, const Type1& value1, const Types&... values ) 
    {
        import( it++, value1 );
        return import( it, values... );
    }
    
    self_type& import( reverse_stride_iterator )
    {
        return *this;
    }

    template< typename Type1 >
    self_type& import( reverse_stride_iterator it, const Type1& value1 )
    {
        *it = value1;
        return *this;
    }

    template< typename... Types >
    self_type& import( const Types&... values ) 
    {
        return import( begin(), values... );
    }

public:
    template< typename Iterator, typename Type1, typename... Types >
    self_type& p_import( Iterator it, const Type1& value1, const Types&... values ) 
    {
        p_import( it++, value1 );
        return p_import( it, values... );
    }
    
    template< typename Iterator>
    self_type& p_import( Iterator )
    {
        return *this;
    }

    template< typename Iterator, typename Type1 >
    self_type& p_import( Iterator it, const Type1& value1 )
    {
        *it = value1;
        return *this;
    }

public:
    template< typename Itor >
    matrix( const size_type r, const size_type c, Itor first, Itor last )
    :   row_(r), col_(c), data_(storage_type(r*c)) 
    {
        std::copy( first, last, begin() ); 
    }

private:
    template<typename T, size_type D, typename A>
    void do_copy(const matrix<T,D,A>& rhs)
    {
        row_ = rhs.row();
        col_ = rhs.col();
        data_.assign(rhs.begin(), rhs.end());
    }
#if 0
    void do_copy( const self_type& rhs )
    {
        row_ = rhs.row();
        col_ = rhs.col();
        data_.assign( rhs.begin(), rhs.end() );
    }
#endif

public:
    size_type 
    row() const
    {
        return row_;
    }

    size_type 
    rows() const
    {
        return row();
    }

    size_type 
    col() const
    {
        return col_;
    }

    size_type 
    cols() const
    {
        return col();
    }

    size_type 
    size() const
    {
        return data_.size();
    }

public:
    self_type&
    resize( const size_type new_row, const size_type new_col )
    {
        if ( ( row_ == new_row ) && ( col_ == new_col ) )
            return *this;

        self_type ans(new_row, new_col);
        const size_type the_row_to_copy = std::min(row_, new_row);
        const size_type the_col_to_copy = std::min(col_, new_col);

        for ( size_type i = 0; i < the_row_to_copy; ++i )
            std::copy( row_begin(i), row_begin(i)+the_col_to_copy, ans.row_begin(i) );  

        *this = ans;
        return *this;
    }

public:
    iterator 
    begin()
    {
        return data_.begin();
    }

    iterator 
    end()
    {
        return data_.end();
    }

    const_iterator 
    begin() const
    {
        return data_.begin();
    }

    const_iterator 
    end() const
    {
        return data_.end();
    }

    const_iterator 
    cbegin() const
    {
        return data_.begin();
    }

    const_iterator 
    cend() const
    {
        return data_.end();
    }

public:
    reverse_iterator 
    rbegin()
    {
        return reverse_iterator( end() );
    }

    reverse_iterator 
    rend()
    {
        return reverse_iterator( begin() );
    }

    const_reverse_iterator 
    rbegin() const
    {
        return const_reverse_iterator( end() );
    }

    const_reverse_iterator 
    rend() const
    {
        return const_reverse_iterator( begin() );
    }

    const_reverse_iterator 
    crbegin() const
    {
        return const_reverse_iterator( end() );
    }

    const_reverse_iterator 
    crend() const
    {
        return const_reverse_iterator( begin() );
    }

public:
    row_type 
    row_begin(const size_type index = 0)
    {
        return row_type(begin() + index * col(), 1);
    }

    row_type 
    row_end(const size_type index = 0)
    {
        return row_begin(index) + col();
    }

    const_row_type 
    row_begin(const size_type index = 0) const
    {
        return const_row_type(begin() + index * col(), 1);
    }

    const_row_type 
    row_end(const size_type index = 0) const
    {
        return row_begin(index) + col();
    }

    const_row_type 
    row_cbegin(const size_type index = 0) const
    {
        return const_row_type(begin() + index * col(), 1);
    }

    const_row_type 
    row_cend(const size_type index = 0) const
    {
        return row_begin(index) + col();
    }

public:
    reverse_row_type 
    row_rbegin( const size_type index = 0 )
    {
        return reverse_row_type( row_end( index ) );
    }

    reverse_row_type 
    row_rend( const size_type index = 0 )
    {
        return reverse_row_type( row_begin( index ) );
    }

    const_reverse_row_type 
    row_rbegin( const size_type index = 0 ) const
    {
        return const_reverse_row_type( row_end( index ) );
    }

    const_reverse_row_type 
    row_rend( const size_type index = 0 ) const
    {
        return const_reverse_row_type( row_begin( index ) );
    }

    const_reverse_row_type 
    row_crbegin( const size_type index = 0 ) const
    {
        return const_reverse_row_type( row_end( index ) );
    }

    const_reverse_row_type 
    row_crend( const size_type index = 0 ) const
    {
        return const_reverse_row_type( row_begin( index ) );
    }

public:
    col_type 
    col_begin(const size_type index)
    {
        return col_type(begin() + index, col());
    }

    col_type 
    col_end(const size_type index)
    {
        return col_begin(index) + row();
    }

    const_col_type 
    col_begin(const size_type index) const
    {
        return const_col_type(begin() + index, col());
    }

    const_col_type 
    col_end(const size_type index) const
    {
        return col_begin(index) + row();
    }

    const_col_type 
    col_cbegin(const size_type index) const
    {
        return const_col_type(begin() + index, col());
    }

    const_col_type 
    col_cend(const size_type index) const
    {
        return col_begin(index) + row();
    }

public:
    reverse_col_type 
    col_rbegin( const size_type index = 0 )
    {
        return reverse_col_type( col_end( index ) );
    }

    reverse_col_type 
    col_rend( const size_type index = 0 )
    {
        return reverse_col_type( col_begin( index ) );
    }

    const_reverse_col_type 
    col_rbegin( const size_type index = 0 ) const
    {
        return const_reverse_col_type( col_end( index ) );
    }

    const_reverse_col_type 
    col_rend( const size_type index = 0 ) const
    {
        return const_reverse_col_type( col_begin( index ) );
    }

    const_reverse_col_type 
    col_crbegin( const size_type index = 0 ) const
    {
        return const_reverse_col_type( col_end( index ) );
    }

    const_reverse_col_type 
    col_crend( const size_type index = 0 ) const
    {
        return const_reverse_col_type( col_begin( index ) );
    }

public:
    diag_type 
    upper_diag_begin(const size_type index)
    {
        return diag_type(begin() + index, col() + 1);
    }

    diag_type 
    upper_diag_end(const size_type index)
    {
        size_type depth = col() - index;
        if (row() < depth)
            depth = row();
        return upper_diag_begin(index) + depth;
    }

    const_diag_type 
    upper_diag_begin(const size_type index) const
    {
        return const_diag_type(begin() + index, col() + 1);
    }

    const_diag_type 
    upper_diag_end(const size_type index) const 
    {
        size_type depth = col() - index;
        if (row() < depth)
            depth = row();
        return upper_diag_begin(index) + depth;
    }

    const_diag_type 
    upper_diag_cbegin(const size_type index) const
    {
        return const_diag_type(cbegin() + index, col() + 1);
    }

    const_diag_type 
    upper_diag_cend(const size_type index) const 
    {
        size_type depth = col() - index;
        if (row() < depth)
            depth = row();
        return upper_diag_cbegin(index) + depth;
    }

public:
    reverse_upper_diag_type 
    upper_diag_rbegin( const size_type index = 0 )
    {
        return reverse_upper_diag_type( upper_diag_end( index ) );
    }

    reverse_upper_diag_type 
    upper_diag_rend( const size_type index = 0 )
    {
        return reverse_upper_diag_type( upper_diag_begin( index ) );
    }

    const_reverse_upper_diag_type 
    upper_diag_rbegin( const size_type index = 0 ) const
    {
        return const_reverse_upper_diag_type( upper_diag_end( index ) );
    }

    const_reverse_upper_diag_type 
    upper_diag_rend( const size_type index = 0 ) const
    {
        return const_reverse_upper_diag_type( upper_diag_begin( index ) );
    }

    const_reverse_upper_diag_type 
    upper_diag_crbegin( const size_type index = 0 ) const
    {
        return const_reverse_upper_diag_type( upper_diag_end( index ) );
    }

    const_reverse_upper_diag_type 
    upper_diag_crend( const size_type index = 0 ) const
    {
        return const_reverse_upper_diag_type( upper_diag_begin( index ) );
    }

public:
    diag_type 
    lower_diag_begin(const size_type index)
    {
        return diag_type(begin() + index * col(), col() + 1);
    }

    diag_type 
    lower_diag_end(const size_type index)
    {
        size_type depth = row() - index;
        if (col() < depth)
            depth = col();
        return lower_diag_begin(index) + depth;
    }

    const_diag_type 
    lower_diag_begin(const size_type index) const
    {
        return const_diag_type(begin() + index * col(), col() + 1);
    }

    const_diag_type 
    lower_diag_end(const size_type index) const
    {
        size_type depth = row() - index;
        if (col() < depth)
            depth = col();
        return lower_diag_begin(index) + depth;
    }

    const_diag_type 
    lower_diag_cbegin(const size_type index) const
    {
        return const_diag_type(begin() + index * col(), col() + 1);
    }

    const_diag_type 
    lower_diag_cend(const size_type index) const
    {
        size_type depth = row() - index;
        if (col() < depth)
            depth = col();
        return lower_diag_begin(index) + depth;
    }

public:
    reverse_lower_diag_type 
    lower_diag_rbegin( const size_type index = 0 )
    {
        return reverse_lower_diag_type( lower_diag_end( index ) );
    }

    reverse_lower_diag_type 
    lower_diag_rend( const size_type index = 0 )
    {
        return reverse_lower_diag_type( lower_diag_begin( index ) );
    }

    const_reverse_lower_diag_type 
    lower_diag_rbegin( const size_type index = 0 ) const
    {
        return const_reverse_lower_diag_type( lower_diag_end( index ) );
    }

    const_reverse_lower_diag_type 
    lower_diag_rend( const size_type index = 0 ) const
    {
        return const_reverse_lower_diag_type( lower_diag_begin( index ) );
    }

    const_reverse_lower_diag_type 
    lower_diag_crbegin( const size_type index = 0 ) const
    {
        return const_reverse_lower_diag_type( lower_diag_end( index ) );
    }

    const_reverse_lower_diag_type 
    lower_diag_crend( const size_type index = 0 ) const
    {
        return const_reverse_lower_diag_type( lower_diag_begin( index ) );
    }

#if 0

public:
    diag_type 
    diag_begin()
    {
        return lower_diag_begin(0);
    }

    diag_type 
    diag_end()
    {
        return lower_diag_end(0);
    }

    const_diag_type 
    diag_begin() const
    {
        return lower_diag_begin(0);
    }

    const_diag_type 
    diag_end() const
    {
        return lower_diag_end(0);
    }

    const_diag_type 
    diag_cbegin() const
    {
        return lower_diag_begin(0);
    }

    const_diag_type 
    diag_cend() const
    {
        return lower_diag_end(0);
    }

public:
    reverse_diag_type 
    diag_rbegin()
    {
        return reverse_diag_type( diag_end() );
    }

    reverse_diag_type 
    diag_rend()
    {
        return reverse_diag_type( diag_begin() );
    }

    const_reverse_diag_type 
    diag_rbegin() const
    {
        return const_reverse_diag_type( diag_end() );
    }

    const_reverse_diag_type 
    diag_rend() const
    {
        return const_reverse_diag_type( diag_begin() );
    }

    const_reverse_diag_type 
    diag_crbegin() const
    {
        return const_reverse_diag_type( diag_end() );
    }

    const_reverse_diag_type 
    diag_crend() const
    {
        return const_reverse_diag_type( diag_begin() );
    }

#endif

#if 1

public:
    diag_type 
    diag_begin( const difference_type index = 0 )
    {
        if ( index > 0 ) return upper_diag_begin( index );
        return lower_diag_begin( -index );
    }
    
    diag_type 
    diag_end( const difference_type index = 0 )
    {
        if ( index > 0 ) return upper_diag_end( index );
        return lower_diag_end( -index );
    }

    const_diag_type 
    diag_begin( const difference_type index = 0 ) const
    {
        if ( index > 0 ) return upper_diag_begin( index );
        return lower_diag_begin( -index );
    }
    
    const_diag_type 
    diag_end( const difference_type index = 0 ) const
    {
        if ( index > 0 ) return upper_diag_end( index );
        return lower_diag_end( -index );
    }

    const_diag_type 
    diag_cbegin( const difference_type index = 0 ) const
    {
        if ( index > 0 ) return upper_diag_cbegin( index );
        return lower_diag_cbegin( -index );
    }
    
    const_diag_type 
    diag_cend( const difference_type index = 0 ) const
    {
        if ( index > 0 ) return upper_diag_cend( index );
        return lower_diag_cend( -index );
    }

public:
    reverse_diag_type 
    diag_rbegin( const difference_type index = 0 )
    {
        if ( index > 0 ) return upper_diag_rbegin( index );
        return lower_diag_rbegin( -index );
    }
    
    reverse_diag_type 
    diag_rend( const difference_type index = 0 )
    {
        if ( index > 0 ) return upper_diag_rend( index );
        return lower_diag_rend( -index );
    }

    const_reverse_diag_type 
    diag_rbegin( const difference_type index = 0 ) const
    {
        if ( index > 0 ) return upper_diag_rbegin( index );
        return lower_diag_rbegin( -index );
    }
    
    const_reverse_diag_type 
    diag_rend( const difference_type index = 0 ) const
    {
        if ( index > 0 ) return upper_diag_rend( index );
        return lower_diag_rend( -index );
    }

    const_reverse_diag_type 
    diag_crbegin( const difference_type index = 0 ) const
    {
        if ( index > 0 ) return upper_diag_crbegin( index );
        return lower_diag_crbegin( -index );
    }
    
    const_reverse_diag_type 
    diag_crend( const difference_type index = 0 ) const
    {
        if ( index > 0 ) return upper_diag_crend( index );
        return lower_diag_crend( -index );
    }

#endif

public:
    anti_diag_type
    upper_anti_diag_begin( const size_type index = 0 ) 
    {
        return anti_diag_type( begin() + col() - index - 1, col() - 1);
    }

    anti_diag_type
    upper_anti_diag_end( const size_type index = 0 ) 
    {
        size_type depth = col() - index;
        if ( row() < depth )
            depth = row();
        return upper_anti_diag_begin(index) + depth;
    }

    const_anti_diag_type
    upper_anti_diag_begin( const size_type index = 0 )  const
    {
        return const_anti_diag_type( begin() + col() - index - 1, col() - 1);
    }

    const_anti_diag_type
    upper_anti_diag_end( const size_type index = 0 )  const
    {
        size_type depth = col() - index;
        if ( row() < depth )
            depth = row();
        return upper_anti_diag_begin(index) + depth;
    }

    const_anti_diag_type
    upper_anti_diag_cbegin( const size_type index = 0 )  const
    {
        return upper_anti_diag_begin(index);
    }

    const_anti_diag_type
    upper_anti_diag_cend( const size_type index = 0 )  const
    {
        return upper_anti_diag_end(index);
    }

public:
    reverse_anti_diag_type
    upper_anti_diag_rbegin( const size_type index = 0 ) 
    {
        return reverse_anti_diag_type(upper_anti_diag_end(index));
    }

    reverse_anti_diag_type
    upper_anti_diag_rend( const size_type index = 0 ) 
    {
        return reverse_anti_diag_type(upper_anti_diag_begin(index));
    }

    const_reverse_anti_diag_type
    upper_anti_diag_rbegin( const size_type index = 0 )  const
    {
        return const_reverse_anti_diag_type(upper_anti_diag_end(index));
    }

    const_reverse_anti_diag_type
    upper_anti_diag_rend( const size_type index = 0 )  const
    {
        return const_reverse_anti_diag_type(upper_anti_diag_begin(index));
    }

    const_reverse_anti_diag_type
    upper_anti_diag_crbegin( const size_type index = 0 )  const
    {
        return upper_anti_diag_rbegin(index);
    }

    const_reverse_anti_diag_type
    upper_anti_diag_crend( const size_type index = 0 )  const
    {
        return upper_anti_diag_rend(index);
    }

#if 0

public:
    anti_diag_type
    anti_diag_begin()
    {
        return upper_anti_diag_begin(0);
    }

    anti_diag_type
    anti_diag_end()
    {
        return upper_anti_diag_end(0);
    }

    const_anti_diag_type
    anti_diag_begin() const
    {
        return upper_anti_diag_begin(0);
    }

    const_anti_diag_type
    anti_diag_end() const
    {
        return upper_anti_diag_end(0);
    }

    const_anti_diag_type
    anti_diag_cbegin() const
    {
        return upper_anti_diag_cbegin(0);
    }

    const_anti_diag_type
    anti_diag_cend() const
    {
        return upper_anti_diag_cend(0);
    }

public:
    reverse_anti_diag_type
    anti_diag_rbegin()
    {
        return upper_anti_diag_rbegin(0);
    }

    reverse_anti_diag_type
    anti_diag_rend()
    {
        return upper_anti_diag_rend(0);
    }

    const_reverse_anti_diag_type
    anti_diag_rbegin() const
    {
        return upper_anti_diag_rbegin(0);
    }

    const_reverse_anti_diag_type
    anti_diag_rend() const
    {
        return upper_anti_diag_rend(0);
    }

    const_reverse_anti_diag_type
    anti_diag_crbegin() const
    {
        return upper_anti_diag_rbegin(0);
    }

    const_reverse_anti_diag_type
    anti_diag_crend() const
    {
        return upper_anti_diag_rend(0);
    }

#endif

public:
    anti_diag_type
    lower_anti_diag_begin( const size_type index = 0 ) 
    {
        return anti_diag_type( begin()+(col()*(index+1))-1, col() - 1 );
    }

    anti_diag_type
    lower_anti_diag_end( const size_type index = 0 ) 
    {
        size_type depth = row() - index;
        if ( col() < depth )
            depth = col();
        return lower_anti_diag_begin(index) + depth;
    }

    const_anti_diag_type
    lower_anti_diag_begin( const size_type index = 0 )  const
    {
        return const_anti_diag_type( begin()+(col()*(index+1))-1, col() - 1 );
    }

    const_anti_diag_type
    lower_anti_diag_end( const size_type index = 0 )  const
    {
        size_type depth = row() - index;
        if ( col() < depth )
            depth = col();
        return lower_anti_diag_begin(index) + depth;
    }

    const_anti_diag_type
    lower_anti_diag_cbegin( const size_type index = 0 )  const
    {
        return lower_anti_diag_begin(index);
    }

    const_anti_diag_type
    lower_anti_diag_cend( const size_type index = 0 )  const
    {
        return lower_anti_diag_end(index);
    }

public:
    reverse_anti_diag_type
    lower_anti_diag_rbegin( const size_type index = 0 ) 
    {
        return reverse_anti_diag_type(lower_anti_diag_end(index));
    }

    reverse_anti_diag_type
    lower_anti_diag_rend( const size_type index = 0 ) 
    {
        return reverse_anti_diag_type(lower_anti_diag_begin(index));
    }

    const_reverse_anti_diag_type
    lower_anti_diag_rbegin( const size_type index = 0 )  const
    {
        return const_reverse_anti_diag_type(lower_anti_diag_end(index));
    }

    const_reverse_anti_diag_type
    lower_anti_diag_rend( const size_type index = 0 )  const
    {
        return const_reverse_anti_diag_type(lower_anti_diag_begin(index));
    }

    const_reverse_anti_diag_type
    lower_anti_diag_crbegin( const size_type index = 0 )  const
    {
        return lower_anti_diag_rbegin(index);
    }

    const_reverse_anti_diag_type
    lower_anti_diag_crend( const size_type index = 0 )  const
    {
        return lower_anti_diag_rend(index);
    }

#if 1

public:
    anti_diag_type 
    anti_diag_begin( const difference_type index = 0 )
    {
        if ( index > 0 ) return upper_anti_diag_begin( index );
        return lower_anti_diag_begin( -index );
    }
    
    anti_diag_type 
    anti_diag_end( const difference_type index = 0 )
    {
        if ( index > 0 ) return upper_anti_diag_end( index );
        return lower_anti_diag_end( -index );
    }

    const_anti_diag_type 
    anti_diag_begin( const difference_type index = 0 ) const
    {
        if ( index > 0 ) return upper_anti_diag_begin( index );
        return lower_anti_diag_begin( -index );
    }
    
    const_anti_diag_type 
    anti_diag_end( const difference_type index = 0 ) const
    {
        if ( index > 0 ) return upper_anti_diag_end( index );
        return lower_anti_diag_end( -index );
    }

    const_anti_diag_type 
    anti_diag_cbegin( const difference_type index = 0 ) const
    {
        if ( index > 0 ) return upper_anti_diag_cbegin( index );
        return lower_anti_diag_cbegin( -index );
    }
    
    const_anti_diag_type 
    anti_diag_cend( const difference_type index = 0 ) const
    {
        if ( index > 0 ) return upper_anti_diag_cend( index );
        return lower_anti_diag_cend( -index );
    }

public:
    reverse_anti_diag_type 
    anti_diag_rbegin( const difference_type index = 0 )
    {
        if ( index > 0 ) return upper_anti_diag_rbegin( index );
        return lower_anti_diag_rbegin( -index );
    }
    
    reverse_anti_diag_type 
    anti_diag_rend( const difference_type index = 0 )
    {
        if ( index > 0 ) return upper_anti_diag_rend( index );
        return lower_anti_diag_rend( -index );
    }

    const_reverse_anti_diag_type 
    anti_diag_rbegin( const difference_type index = 0 ) const
    {
        if ( index > 0 ) return upper_anti_diag_rbegin( index );
        return lower_anti_diag_rbegin( -index );
    }
    
    const_reverse_anti_diag_type 
    anti_diag_rend( const difference_type index = 0 ) const
    {
        if ( index > 0 ) return upper_anti_diag_rend( index );
        return lower_anti_diag_rend( -index );
    }

    const_reverse_anti_diag_type 
    anti_diag_crbegin( const difference_type index = 0 ) const
    {
        if ( index > 0 ) return upper_anti_diag_crbegin( index );
        return lower_anti_diag_crbegin( -index );
    }
    
    const_reverse_anti_diag_type 
    anti_diag_crend( const difference_type index = 0 ) const
    {
        if ( index > 0 ) return upper_anti_diag_crend( index );
        return lower_anti_diag_crend( -index );
    }
#endif

public:
    row_range_type
    row_range( const_row_type begin, const_row_type end )
    {
        return row_range_type ( begin, end - 1, col());
    }

    const_row_range_type
    row_range( const_row_type begin, const_row_type end ) const
    {
        return const_row_range_type ( begin, end - 1, col());
    }

public:
    row_range_type
    row_range_begin( const size_type first, const size_type last )
    {
        return row_range_type ( row_begin(first), row_begin(last), col());
    }

    row_range_type
    row_range_end( const size_type first, const size_type last )
    {
        return row_range_type ( row_end(first), row_end(last), col());
    }

    const_row_range_type
    row_range_begin( const size_type first, const size_type last ) const
    {
        return const_row_range_type ( row_begin(first), row_begin(last), col());
    }

    const_row_range_type
    row_range_end( const size_type first, const size_type last ) const
    {
        return const_row_range_type ( row_end(first), row_end(last), col());
    }

public:
    col_range_type
    col_range( const_col_type begin, const_col_type end )
    {
        return col_range_type ( begin, end - 1, 1);
    }

    const_col_range_type
    col_range( const_col_type begin, const_col_type end ) const
    {
        return const_col_range_type ( begin, end - 1, 1);
    }

public:
    col_range_type
    col_range_begin( const size_type first, const size_type last )
    {
        return col_range_type ( col_begin(first), col_begin(last), 1);
    }

    col_range_type
    col_range_end( const size_type first, const size_type last )
    {
        return col_range_type ( col_end(first), col_end(last), 1);
    }

    const_col_range_type
    col_range_begin( const size_type first, const size_type last ) const
    {
        return const_col_range_type ( col_begin(first), col_begin(last), 1);
    }

    const_col_range_type
    col_range_end( const size_type first, const size_type last ) const
    {
        return const_col_range_type ( col_end(first), col_end(last), 1);
    }

public:
    reverse_row_range_type
    row_range_rbegin( const size_type first, const size_type last )
    {
        return reverse_row_range_type( row_range_end(first,last) );
    }

    reverse_row_range_type
    row_range_rend( const size_type first, const size_type last )
    {
        return reverse_row_range_type( row_range_begin(first,last) );
    }
    
    const_reverse_row_range_type
    row_range_rbegin( const size_type first, const size_type last ) const
    {
        return reverse_row_range_type( row_range_end(first,last) );
    }

    const_reverse_row_range_type
    row_range_rend( const size_type first, const size_type last ) const
    {
        return reverse_row_range_type( row_range_begin(first,last) );
    }

public:
    reverse_row_range_type
    row_range( const_reverse_row_type begin, const_reverse_row_type end )
    {
        return reverse_row_range_type( row_range_end(begin,end) );
    }

    const_reverse_row_range_type
    row_range( const_reverse_row_type  begin, const_reverse_row_type  end ) const
    {
        return reverse_row_range_type( row_range_end(begin,end) );
    }

public:
    reverse_col_range_type
    col_range_rbegin( const size_type first, const size_type last )
    {
        return reverse_col_range_type( col_range_end(first,last) );
    }
    
    const_reverse_col_range_type
    col_range_rbegin( const size_type first, const size_type last ) const
    {
        return reverse_col_range_type( col_range_end(first,last) );
    }
    
    reverse_col_range_type
    col_range_rend( const size_type first, const size_type last )
    {
        return reverse_col_range_type( col_range_begin(first,last) );
    }
    
    const_reverse_col_range_type
    col_range_rend( const size_type first, const size_type last ) const
    {
        return reverse_col_range_type( col_range_begin(first,last) );
    }

public:
    reverse_col_range_type
    col_range( const_reverse_col_type begin, const_reverse_col_type end )
    {
        return reverse_col_range_type( col_range_end(begin,end) );
    }
    
    const_reverse_col_range_type
    col_range( const_reverse_col_type  begin, const_reverse_col_type  end ) const
    {
        return reverse_col_range_type( col_range_end(begin,end) );
    }
    
public:
    row_type 
    operator[](const size_type index)
    {
        return row_begin(index);
    }

    const_row_type 
    operator[](const size_type index) const
    {
        return row_begin(index);
    }

public:
    value_type&
    operator()( const size_type r = 0, const size_type c = 0 )
    {
        return *(row_begin(r)+c);
    }

    value_type
    operator()( const size_type r = 0, const size_type c = 0 ) const 
    {
        return *(row_cbegin(r)+c);
    }

public:
    const self_type 
    operator-() const
    {
        self_type ans(*this);
        std::transform(ans.begin(), ans.end(), ans.begin(), std::negate<value_type>());
        return ans;
    }

    const self_type 
    operator+() const
    {
        self_type ans(*this);
        return ans;
    }

    const self_type 
    operator~() const
    {
        return transpose();
    }

    const self_type 
    operator!() const
    {
        return inverse();
    }

public:
    self_type & 
    operator +=(const value_type& rhs)
    {
        std::transform(begin(), end(), begin(), std::bind1st(std::plus<value_type > (), rhs));
        return *this;
    }

    self_type & 
    operator +=(const self_type& rhs)
    {
        std::transform(begin(), end(), rhs.begin(), begin(), std::plus<value_type > ());
        return *this;
    }

    self_type & 
    operator -=(const value_type& rhs)
    {
        std::transform(begin(), end(), begin(), std::bind1st(std::minus<value_type > (), rhs));
        return *this;
    }

    self_type & 
    operator -=(const self_type& rhs)
    {
        std::transform(begin(), end(), rhs.begin(), begin(), std::minus<value_type > ());
        return *this;
    }

    self_type & 
    operator *=(const value_type& rhs)
    {
        std::transform(begin(), end(), begin(), std::bind1st(std::multiplies<value_type > (), rhs));
        return *this;
    }

#if 0
    
    ALGORITHM FOR:
    
    self_type &
    operator *= ( const self_type& other );
    with this[R]{C], other[C][OC]

    //ALGORITHM DETAILS:
        //threshold should be determined from experiments
    0)  if ( ( max(all dims) < threshold ) || min(all dims) == 1 ) ) 
        do direct multiplicaiton

        //case row is odd
    1)  else if (row() & 1)
        {
            //<1>
            if (row() & 2)
            {
                append one empty row to the matrix from downside 
                    [ new_this <- this ]
                do the multiplication 
                    [new_ans <- new_this * other ]
                remove the last row to generate ans
                    [ ans <- new_ans ]  
            }
            //<2>
            else
            {
                cut last row from the matrix, get two new matrices  
                    [ new_this <- this ]
                    [ last_row <- this ]
                do the multiplications  
                    [ new_ans <- new_this * other ]
                    [ last_row_ans <- last_row * other ]
                 merge the two matrices to generate the ans
                    [ ans <- new_ans | last_row_ans ]
                    [ i.e. last_row_ans appended to new_ans as the last row ]
            }
        }
    
        //case col is odd
    2)  else if (col() & 1)
        {
            //<1>
            if ( col() & 2 )
            {
                append one emtpy col to this from right side
                    [new_this <- this]
                append one empty row to other from downside
                    [new_other <- other]
                do the multiplication
                    [ans <- new_this * new_other]
            }
            //<2>
            else
            {
                cut last col of this from right side
                    [new_this <- this]
                    [last_col <- this]
                cut last row of other from downside
                    [new_other <- other]
                    [last_row <- other]
                do the multiplicaitons
                    [new_ans <- new_this * new_other]
                    [res_col_row <- last_col * last_row]
                do the addition to generate ans
                    [ans <- new_ans + res_col_row]
            }
        }

        //case other.col is odd
    3)  else if ( other.col() & 1 )
        {
            //<1>
            if ( other.col() & 2 )
            {
                append one empty col to other from right side
                    [new_other <- other]
                do the multiplicaiton
                    [new_ans = this * other]
                remove the last col to generate the ans
                    [ans <- new_ans]
            }
            //<2>
            else
            {
                cut the last col from other get two new matrices
                    [new_other <- other]
                    [last_col <- other]
                do the multications
                    [new_ans <- this * new_other]
                    [last_col_ans <- this * last_col]
                merge the two matrices
                    [ans <- new_ans|last_col_ans]
                    [i.e. last_col_ans as the last col of]
            }
        }

        //all dims even, using strassen algorithm
    4) else
        {
            //strassen algorithm 

            ( a_00 a_01 )   ( b_00 b_01 )    ( c_00 c_01 )
            (           ) * (           )  = (           )
            ( a_10 a_11 )   ( b_10 b_11 )    ( c_10 c_11 )

            Q_0 = ( a_00 + a_11 ) * ( b_00 + b_11 )
            Q_1 = ( a_10 + a_11 ) * b_00
            Q_2 = a_00 * ( b_01 - b_11 )
            Q_3 = a_11 * ( -b_00 + b_10 )
            Q_4 = ( a_00 + a_01 ) * b_11
            Q_5 = ( -a_00 + a_10 ) * ( b_00 + b_10 )
            Q_6 = ( a_01 - a_11 ) * ( b_10 + b_11 )
            
            c_00 = Q_0 + Q_3 - Q_4 +Q_6
            c_10 = Q_1 + Q_3
            c_01 = Q_2 + Q_4
            c_11 = Q_0 - Q_1 +Q_2 + Q_5
        }

#endif

private:

    size_type
    max_dim() const
    {
        return std::max( row(), col() );
    }

    size_type 
    min_dim() const
    {
        return std::min( row(), col() );
    }

    self_type&
    direct_multiply( const self_type& other )
    {
        self_type tmp(row(), other.col());
        for (size_type i = 0; i < row(); ++i)
            for (size_type j = 0; j < other.col(); ++j)
                tmp[i][j] = std::inner_product( row_begin(i), row_end(i), other.col_begin(j), value_type(0)); 

        *this = tmp;
        return *this;   
    }

#if 0
    1)  else if (row() & 1)
        {
            //<1>
            if (row() & 2)
            {
                append one empty row to the matrix from downside 
                    [ new_this <- this ] [R+1,C]
                do the multiplication 
                    [new_ans <- new_this * other ] [R+1, OC]
                remove the last row to generate ans
                    [ ans <- new_ans ]  
            }
#endif 
    self_type&
    rr1( const self_type& other )
    {
        const self_type new_this = *this && value_type(0);

        const self_type 
        new_ans = new_this * other;

        const self_type ans(    new_ans, range_type( 0, row() ), range_type( 0, other.col() ));

        *this = ans;
        return *this;
    }

#if 0
    1)  else if (row() & 1)
        {
            //<1>
            if (row() & 2) {...}
            //<2>
            else
            {
                cut last row from the matrix, get two new matrices  
                    [ new_this <- this ] [R-1,C]
                    [ last_row <- this ] [1,C]
                do the multiplications  
                    [ new_ans <- new_this * other ] [R-1,OC]
                    [ last_row_ans <- last_row * other ] [1,OC]
                 merge the two matrices to generate the ans
                    [ ans <- new_ans | last_row_ans ] [R,OC]
                    [ i.e. last_row_ans appended to new_ans as the last row ]
            }
        }
#endif

    self_type&
    rr2( const self_type& other )
    {
        const self_type new_this(   *this, range_type( 0, row()-1 ), range_type( 0, col() ));

        const self_type last_row(   *this, range_type( row()-1, row() ), range_type( 0, col() ));
        
        const self_type new_ans = new_this * other;
        
        const self_type last_row_ans = last_row * other;
        
        const self_type ans = new_ans && last_row_ans;

        *this = ans;
        return *this;
    }

#if 0
        //case col is odd
    2)  else if (col() & 1)
        {
            //<1>
            if ( col() & 2 )
            {
                append one emtpy col to this from right side
                    [new_this <- this] [R,C+1]
                append one empty row to other from downside
                    [new_other <- other] [C+1,OC]
                do the multiplication
                    [ans <- new_this * new_other] [R,OC]
            }
            //<2>
            else {...}
#endif
    self_type&
    cc1( const self_type& other )
    {
        const self_type 
        new_this = *this || value_type(0);

        self_type new_other( col()+1, other.col() );
        std::copy( other.begin(), other.end(), new_other.begin() );

        const self_type ans = new_this * new_other;

        *this = ans;
        return *this;
    }

#if 0
        //case col is odd
    2)  else if (col() & 1)
        {
            //<1>
            if ( col() & 2 ) {...}
            //<2>
            else
            {
                cut last col of this from right side
                    [new_this <- this] [R,C-1]
                    [last_col <- this] [R,1]
                cut last row of other from downside
                    [new_other <- other] [C-1,OC]
                    [last_row <- other]  [1, OC]
                do the multiplicaitons
                    [new_ans <- new_this * new_other]    [R,OC]
                    [res_col_row <- last_col * last_row] [R,OC]
                do the addition to generate ans
                    [ans <- new_ans + res_col_row] [R,OC]
            }
#endif
    self_type&
    cc2( const self_type& other )
    {   
        //[new_this <- this] [R,C-1]
        const self_type new_this(   *this, range_type( 0, row() ), range_type( 0, col()-1 ));
        //[last_col <- this] [R,1]
        const self_type last_col(   *this, range_type( 0, row() ), range_type( col()-1, col() ));

        //[new_other <- other] [C-1,OC]
        const self_type new_other(  other, range_type( 0, other.row()-1 ), range_type( 0, other.col() ));
                    
        //[last_row <- other]  [1, OC]
        const self_type last_row(   other, range_type( other.row()-1, other.row() ), range_type( 0, other.col() ));

        self_type new_ans = new_this * new_other;
        self_type res_col_row = last_col * last_row;

        const self_type ans = new_ans + res_col_row;

        *this = ans;
        return *this;
    }
#if 0
        //case other.col is odd
    3)  else if ( other.col() & 1 )
        {
            //<1>
            if ( other.col() & 2 )
            {
                append one empty col to other from right side
                    [new_other <- other] [C,OC+1]
                do the multiplicaiton
                    [new_ans = this * new_other] [R,OC+1]
                remove the last col to generate the ans
                    [ans <- new_ans] [R,OC]
            }
            //<2>
            else {...}
        }
#endif
    self_type&
    oc1( const self_type& other )
    {
        //[new_other <- other] [C,OC+1]
        const self_type new_other = other || value_type(0);

        const self_type new_ans = *this * new_other;

        const self_type ans( new_ans, range_type( 0, row() ), range_type( 0, other.col() ) );

        *this = ans;
        return *this;
    }

#if 0
        //case other.col is odd
    3)  else if ( other.col() & 1 )
        {
            //<1>
            if ( other.col() & 2 ) {...}
            //<2>
            else
            {
                cut the last col from other get two new matrices
                    [new_other <- other] [C,OC-1]
                    [last_col <- other]  [C,1]
                do the multications
                    [new_ans <- this * new_other]     [R,OC-1]
                    [last_col_ans <- this * last_col] [R,1]
                merge the two matrices
                    [ans <- new_ans|last_col_ans] [R,1]
                    [i.e. last_col_ans as the last col of ans]
            }
        }
#endif
    self_type&
    oc2( const self_type& other )
    {
        const self_type new_other(  other, range_type(0,other.row()), range_type(0, other.col()-1));

        const self_type last_col(   other, range_type(0, other.row()), range_type(other.col()-1, other.col()) );

        const self_type new_ans = (*this) * new_other;
        
        const self_type last_col_ans = (*this) * last_col;

        const self_type ans = new_ans || last_col_ans;

        *this = ans;
        return *this;
    }

#if 0
    4) else
        {
            //strassen algorithm 
            ( a_00 a_01 )   ( b_00 b_01 )    ( c_00 c_01 )
            (           ) * (           )  = (           )
            ( a_10 a_11 )   ( b_10 b_11 )    ( c_10 c_11 )

            Q_0 = ( a_00 + a_11 ) * ( b_00 + b_11 )
            Q_1 = ( a_10 + a_11 ) * b_00
            Q_2 = a_00 * ( b_01 - b_11 )
            Q_3 = a_11 * ( -b_00 + b_10 )
            Q_4 = ( a_00 + a_01 ) * b_11
            Q_5 = ( -a_00 + a_10 ) * ( b_00 + b_10 )
            Q_6 = ( a_01 - a_11 ) * ( b_10 + b_11 )
            
            c_00 = Q_0 + Q_3 - Q_4 +Q_6
            c_10 = Q_1 + Q_3
            c_01 = Q_2 + Q_4
            c_11 = Q_0 - Q_1 +Q_2 + Q_5
        }
#endif

    self_type&
    strassen_multiply( const self_type& other )
    {
        const size_type R_2 = row() >> 1;          
        const size_type C_2 = col() >> 1;
        const size_type OR_2 = C_2;
        const size_type OC_2 = other.col() >> 1;

        const self_type a_00(   *this, 
                                range_type( 0, R_2 ),
                                range_type( 0, C_2 )
                            );
        const self_type a_01(   *this,
                                range_type( 0, R_2 ),
                                range_type( C_2, col() ) 
                            );
        const self_type a_10(   *this,
                                range_type( R_2, row() ),
                                range_type( 0, C_2 )
                            );
        const self_type a_11(   *this,
                                range_type( R_2, row() ),
                                range_type( C_2, col() )
                            );

        const self_type b_00(   other, 
                                range_type( 0, OR_2 ),
                                range_type( 0, OC_2 )
                            );
        const self_type b_01(   other,
                                range_type( 0, OR_2 ),
                                range_type( OC_2, other.col() ) 
                            );
        const self_type b_10(   other,
                                range_type( OR_2, other.row() ),
                                range_type( 0, OC_2 )
                            );
        const self_type b_11(   other,
                                range_type( OR_2, other.row() ),
                                range_type( OC_2, other.col() )
                            );


        const self_type Q_0 = ( a_00 + a_11 ) * ( b_00 + b_11 );
        const self_type Q_1 = ( a_10 + a_11 ) * b_00;
        const self_type Q_2 = a_00 * ( b_01 - b_11 ); 
        const self_type Q_3 = a_11 * ( -b_00 + b_10 );
        const self_type Q_4 = ( a_00 + a_01 ) * b_11;
        const self_type Q_5 = ( -a_00 + a_10 ) * ( b_00 + b_01 );
        const self_type Q_6 = ( a_01 - a_11 ) * ( b_10 + b_11 );

        const self_type c_00 = Q_0 + Q_3 - Q_4 +Q_6;
        const self_type c_10 = Q_1 + Q_3;
        const self_type c_01 = Q_2 + Q_4;
        const self_type c_11 = Q_0 - Q_1 +Q_2 + Q_5;

        const self_type ans = ( c_00 || c_01 ) && ( c_10 || c_11 );

        *this = ans;
        return *this;
    }


public:
    self_type&
    operator *= ( const self_type& other )
    {
        assert( col() == other.row() );
        
        static const size_type threshold = 17;

        const size_type max_dims = std::max( max_dim(), other.max_dim() );
        const size_type min_dims = std::min( min_dim(), other.min_dim() );

        //0
        if ( (max_dims < threshold)  || (min_dims == 1) )
            return direct_multiply( other ); 

        const size_type R = row();
        const size_type C = col();
        const size_type OC = other.col();

        //1
        if ( R & 1 )
        {
            //1 <1>
            if ( R & 2 )
                return rr1(other);  
            //1 <2>
            return rr2(other);
        }
        //2
        if ( C & 1 )
        {   //2 <1>
            if ( C & 2 )
                return cc1(other);
            //2 <2>
            return cc2(other);
        }
        //3
        if ( OC & 1 )
        {   //3 <1>
            if ( OC & 2 )
                return oc1(other);
            //3 <2>
            return oc2(other);
        }
        //4
        return strassen_multiply( other );
    }

    self_type & 
    operator /=(const value_type& rhs)
    {
        std::transform(begin(), end(), begin(), std::bind2nd(std::divides<value_type > (), rhs));
        return *this;
    }

    self_type & 
    operator /=(const self_type& rhs)
    {
        return operator*=(!rhs);
    }

    template<typename T_>
    self_type &
    operator /= ( const T_& rhs )
    {
        std::for_each( begin(), end(), [&rhs](value_type& v) { v /= rhs; } );
        return *this;
    }

    public:
    const self_type transpose() const
    {
        self_type ans(col(), row());

        for (size_type i = 0; i < col(); ++i)
            std::copy(col_begin(i), col_end(i), ans.row_begin(i));

        return ans;
    }

    public:
    const self_type
    inverse() const
    {
        assert( row() == col() );

        if (1 == size())
        {
            self_type ans(*this);

            *ans.begin() = value_type(1) / (*ans.begin());
            return ans;
        }

        if (4 == size())
        {
            self_type ans(*this);
            std::swap(ans[0][0], ans[1][1]);
            ans[0][1] = -ans[0][1]; 
            ans[1][0] = -ans[1][0];
            auto const v = ans[0][0]*ans[1][1] - ans[1][0]*ans[0][1];
            return ans / v;
        }
        
        if (9 == size())
        {
            self_type ans(*this);
            auto const a = ans[0][0]; auto const b = ans[0][1]; auto const c = ans[0][2];        
            auto const d = ans[1][0]; auto const e = ans[1][1]; auto const f = ans[1][2];        
            auto const g = ans[2][0]; auto const h = ans[2][1]; auto const i = ans[2][2];        
            auto const A = e*i - f*h; auto const B = f*g - d*i; auto const C = d*h - e*g;
            auto const D = c*h - b*i; auto const E = a*i - c*g; auto const F = g*b - a*h;
            auto const G = b*f - c*e; auto const H = c*d - a*f; auto const I = a*e - b*d;
            auto const v = a*A + b*B + c*C;
            ans[0][0] = A; ans[0][1] = D; ans[0][2] = G;
            ans[1][0] = B; ans[1][1] = E; ans[1][2] = H;
            ans[2][0] = C; ans[2][1] = F; ans[2][2] = I;
            return ans / v;
        }

        if ( row() & 1 )
            return direct_inverse();

        return strassen_inverse();
    }

    private:
    //
    //A=[P Q, R S], where P is m*m, Q is m*n, R is n*m and S is n*n
    //suppose the inverse is A'=[P' Q', R' S'],
    //then we could get :
    //  P' = (P-Q*S^{-1}*R)^{-1}
    //  Q' = -(P-Q*S^{-1}*R)^{-1} * (Q*S^{-1})
    //  R' = -(S^{-1}*R) * (P-Q*S^{-1}*R)^{-1}
    //  S' = S^{-1} + (S^{-1}*R) * (P-Q*S^{-1}*R)^{-1} * (Q*S^{-1})
    //or short terms: 
    // a) s[n,n] = S^{-1}
    // b) p[m,m] = P^{-1}
    // c) Qs[m,n]= Q*s
    // d) sR[n,m]= s*R
    // e) QsR[m,m]= Q*sR
    // f) L[m,m] = P-QsR
    // g) P'[m,m] = L^{-1}
    // h) Q'[m,n] = -P'*Qs
    // i) R'[n,m] = -sR*P'
    // j) S'[n,n] = s + sR * P' * Qs
    //
    const self_type 
    direct_inverse() const
    {

        self_type ans(row(), col());

        const size_type m = row() >> 1;
        const size_type n = row() - m;
        self_type P(m, m), Q(m, n), R(n, m), S(n, n);

        for (size_type i = 0; i < m; ++i)
        {
            std::copy(row_begin(i), row_begin(i) + m, P.row_begin(i));
            std::copy(row_begin(i) + m, row_end(i), Q.row_begin(i));
        }
        for (size_type i = m; i < row(); ++i)
        {
            std::copy(row_begin(i), row_begin(i) + m, R.row_begin(i - m));
            std::copy(row_begin(i) + m, row_end(i), S.row_begin(i - m));
        }

        // a)
        const self_type s = S.inverse();
        // b)
        const self_type p = P.inverse();
        // c)
        const self_type Qs = Q * s;
        // d)
        const self_type sR = s * R;
        // e)
        const self_type QsR = Q * sR;
        // f)
        const self_type L = P - QsR;
        // g)
        const self_type P_ = L.inverse();
        // h)
        const self_type Q_ = -P_ * Qs;
        // i)
        const self_type R_ = -sR * P_;
        // j)
        const self_type S_ = s - R_ * Qs;

        for (size_type i = 0; i < m; ++i)
        {
            std::copy(P_.row_begin(i), P_.row_end(i), ans.row_begin(i));
            std::copy(Q_.row_begin(i), Q_.row_end(i), ans.row_begin(i) + m);
        }
        for (size_type i = 0; i < n; ++i)
        {
            std::copy(R_.row_begin(i), R_.row_end(i), ans.row_begin(i + m));
            std::copy(S_.row_begin(i), S_.row_end(i), ans.row_begin(i + m) + m);
        }

        return ans;
    }

    // A:   suppose the matrices (a_11 a_12, a_21 a_22) and (c_11 c_12, c_21 c_22) are inverses of each other.
    //      the c's can be obtained by following operations:
    //  1       R1 = inverse(a_11)
    //  2       R2 = a_21 * R1
    //  3       R3 = R1 * a_12
    //  4       R4 = a_21 * R3
    //  5       R5 = R4 - a_22
    //  6       R6 = inverse(R5)
    //  7       c_12 = R3*R6
    //  8       c_21 = R6*R2
    //  9       R7 = R3*c_21
    //  10      c_11 = R1-R7
    //  11      c_22 = -R6
    // B:   merge   c_11 c_12
    //              c_21 c_22
    //      to generate the reverse matrix.
    const self_type 
    strassen_inverse() const
    {
        const size_type n = row();
        const size_type n_2 = n >> 1;

        //A
        const self_type a_11( *this, range_type(0, n_2), range_type(0, n_2) );
        const self_type a_12( *this, range_type(0, n_2), range_type(n_2, n) );
        const self_type a_21( *this, range_type(n_2, n), range_type(0, n_2) );
        const self_type a_22( *this, range_type(n_2, n), range_type(n_2, n) );
        
        //1
        const self_type R1 = a_11.inverse();
        //2
        const self_type R2 = a_21 * R1;
        //3
        const self_type R3 = R1 * a_12;
        //4
        const self_type R4 = a_21 * R3;
        //5
        const self_type R5 = R4 - a_22;
        //6
        const self_type R6 = R5.inverse();
        //7
        const self_type c_12 = R3 * R6;
        //8
        const self_type c_21 = R6 * R2;
        //9
        const self_type R7 = R3 * c_21;
        //10
        const self_type c_11 = R1 - R7;
        //11
        const self_type c_22 = -R6;
        //B
        const self_type ans = (c_11 || c_12) && (c_21 || c_22);

        return ans;
    }

public:
    //A=
    //  [P[m,m] Q[m,n]]
    //  [R[m,n] S[n,n]]
    //\det A =
    // \det P * \det (S - R*P^{-1}*Q)
    //
    const value_type 
    det() const
    {
        assert( row() == col() );

        if (1 == size())
            return *begin();

        if (4 == size())
            return (*this)[0][0] * (*this)[1][1] - (*this)[1][0] * (*this)[0][1];

        auto const N = row();
        auto const m = N >> 1;
        
        self_type const P(*this, range_type(0, m), range_type(0, m));
        self_type const Q(*this, range_type(0, m), range_type(m, N));
        self_type const R(*this, range_type(m, N), range_type(0, m));
        self_type const S(*this, range_type(m, N), range_type(m, N));
        auto const tmp = S - ( R * ( P.inverse() ) * Q );

        return P.det() * tmp.det();
    }

public:
    const value_type 
    tr() const 
    {
        return std::accumulate( diag_begin(), diag_end(), value_type() );
    }

public:
    const std::vector<value_type, Allocator>
    to_vector() const 
    {
        std::vector<value_type, Allocator> ans( row() * col() );
        std::copy( begin(), end(), ans.begin() );
        return ans;
    }

    const std::valarray<value_type>
    to_valarray() const 
    {
        std::valarray<value_type> ans( row() * col() );
        std::copy( begin(), end(), &ans[0] );
        return ans;
    }

private:

    size_type row_;
    size_type col_;
    storage_type data_;
};


template<typename T, std::size_t D, typename A>
std::ostream & 
operator <<(std::ostream& lhs, const matrix<T, D, A>& rhs)
{
    typedef typename matrix<T>::size_type size_type;
    typedef typename matrix<T>::value_type value_type;

    for (size_type i = 0; i < rhs.row(); ++i)
    {
        std::copy(rhs.row_begin(i), rhs.row_end(i),
                  std::ostream_iterator<value_type > (lhs, " \t "));
        lhs << "\n";
    }

    return lhs;
} 

}//namespace feng

#endif  /* _DYNAMIC_MATRIX_IMPL_HPP_INCLUDED*/

