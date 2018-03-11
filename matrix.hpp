#ifndef FENG_MATRIX_HPP_INCLUDED_
#define FENG_MATRIX_HPP_INCLUDED_

#ifdef __clang__
#define SUPPRESS_WARNINGS \
    _Pragma("clang diagnostic push ") \
    _Pragma("clang diagnostic ignored \"-Wshorten-64-to-32\"" ) \
    _Pragma("clang diagnostic ignored \"-Wcast-align\"" ) \
    _Pragma("clang diagnostic ignored \"-Wdouble-promotion\"" ) \
    _Pragma("clang diagnostic ignored \"-Wreserved-id-macro\"" ) \
    _Pragma("clang diagnostic ignored \"-Wdocumentation-unknown-command\"") \
    _Pragma("clang diagnostic ignored \"-Wundef\"") \
    _Pragma("clang diagnostic ignored \"-Wc++98-compat\"") \
    _Pragma("clang diagnostic ignored \"-Wexit-time-destructors\"") \
    _Pragma("clang diagnostic ignored \"-Wdocumentation-deprecated-sync\"") \
    _Pragma("clang diagnostic ignored \"-Wdocumentation\"") \
    _Pragma("clang diagnostic ignored \"-Wmissing-prototypes\"") \
    _Pragma("clang diagnostic ignored \"-Wold-style-cast\"") \
    _Pragma("clang diagnostic ignored \"-Wpadded\"") \
    _Pragma("clang diagnostic ignored \"-Wc++98-compat-pedantic\"") \
    _Pragma("clang diagnostic ignored \"-Wc++98-compat\"") \
    _Pragma("clang diagnostic ignored \"-Wglobal-constructors\"") \
    _Pragma("clang diagnostic ignored \"-Wshadow-uncaptured-local\"") \
    _Pragma("clang diagnostic ignored \"-Wsign-conversion\"") \
    _Pragma("clang diagnostic ignored \"-Wzero-as-null-pointer-constant\"")
#define RESTORE_WARNINGS \
    _Pragma( "clang diagnostic pop" )
#else
#define SUPPRESS_WARNINGS
#define RESTORE_WARNINGS
#endif

SUPPRESS_WARNINGS
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <complex>
#include <cstring>
#include <deque>
#include <fstream>
#include <functional>
#include <initializer_list>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <new>
#include <numeric>
#include <optional>
#include <random>
#include <set>
#include <sstream>
#include <streambuf>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <valarray>
#include <vector>

namespace feng
{
    constexpr unsigned long matrix_version = 20180101;

    template < typename Iterator_Type >
    struct stride_iterator
    {
        typedef stride_iterator                                                     self_type;
        typedef typename std::iterator_traits< Iterator_Type >::value_type          value_type;
        typedef typename std::iterator_traits< Iterator_Type >::reference           reference;
        typedef typename std::iterator_traits< Iterator_Type >::difference_type     difference_type;
        typedef typename std::iterator_traits< Iterator_Type >::pointer             pointer;
        typedef typename std::iterator_traits< Iterator_Type >::iterator_category   iterator_category;

        Iterator_Type                                                               iterator_;
        difference_type                                                             step_;

        stride_iterator( const Iterator_Type& it, const difference_type& dt ) noexcept : iterator_( it ) , step_( dt ) { }

        stride_iterator() noexcept : iterator_( 0 ) , step_( 1 ) { }
        stride_iterator( const self_type& ) noexcept = default;
        stride_iterator( self_type&& ) noexcept = default;
        self_type& operator=( const self_type& ) noexcept = default;
        self_type& operator=( self_type&& ) noexcept = default;

        self_type& operator++() noexcept
        {
            iterator_ += step_;
            return *this;
        }
        const self_type operator++( int ) noexcept
        {
            self_type ans( *this );
            operator++();
            return ans;
        }
        self_type& operator+=( const difference_type dt ) noexcept
        {
            iterator_ += dt * step_;
            return *this;
        }
        friend const self_type operator+( const self_type& lhs, const difference_type rhs ) noexcept
        {
            self_type ans( lhs );
            ans += rhs;
            return ans;
        }
        friend const self_type operator+( const difference_type lhs, const self_type& rhs ) noexcept
        {
            return rhs + lhs;
        }
        self_type& operator--() noexcept
        {
            iterator_ -= step_;
            return *this;
        }
        const self_type operator--( int ) noexcept
        {
            self_type ans( *this );
            operator--();
            return ans;
        }
        self_type& operator-=( const difference_type dt ) noexcept
        {
            iterator_ -= dt * step_;
            return *this;
        }
        friend const self_type operator-( const self_type& lhs, const difference_type rhs ) noexcept
        {
            self_type ans( lhs );
            ans -= rhs;
            return ans;
        }
        reference operator[]( const difference_type dt ) noexcept
        {
            return iterator_[dt * step_];
        }
        const reference operator[]( const difference_type dt ) const noexcept
        {
            return iterator_[dt * step_];
        }
        reference operator*() noexcept
        {
            return *iterator_;
        }
        const reference operator*() const noexcept
        {
            return *iterator_;
        }
        friend bool operator==( const self_type& lhs, const self_type& rhs ) noexcept
        {
            if ( lhs.step_ != rhs.step_ ) return false;
            return lhs.iterator_ == rhs.iterator_;
        }
        friend bool operator!=( const self_type& lhs, const self_type& rhs ) noexcept
        {
            if ( lhs.step_ != rhs.step_ ) return true;
            return lhs.iterator_ != rhs.iterator_;
        }
        friend bool operator<( const self_type& lhs, const self_type& rhs ) noexcept
        {
            if ( lhs.step_ != rhs.step_ ) return false;
            return lhs.iterator_ < rhs.iterator_;
        }
        friend bool operator<=( const self_type& lhs, const self_type& rhs ) noexcept
        {
            if ( lhs.step_ != rhs.step_ ) return false;
            return lhs.iterator_ <= rhs.iterator_;
        }
        friend bool operator>( const self_type& lhs, const self_type& rhs ) noexcept
        {
            if ( lhs.step_ != rhs.step_ ) return false;
            return lhs.iterator_ > rhs.iterator_;
        }
        friend bool operator>=( const self_type& lhs, const self_type& rhs ) noexcept
        {
            if ( lhs.step_ != rhs.step_ ) return false;
            return lhs.iterator_ >= rhs.iterator_;
        }
        friend difference_type operator-( const self_type& lhs, const self_type& rhs ) noexcept
        {
            assert( lhs.step_ == rhs.step_ && "stride iterators of different steps" );
            return ( lhs.iterator_ - rhs.iterator_ ) / lhs.step_;
        }
    };

    template < typename Type, typename Allocator >
    struct crtp_typedef
    {
        typedef typename std::decay< Type >::type                       value_type;
        typedef value_type*                                             iterator;
        typedef const value_type*                                       const_iterator;
        typedef Allocator                                               allocator_type;
        typedef std::uint64_t                                           size_type;
        typedef std::ptrdiff_t                                          difference_type;
        typedef std::pair<size_type, size_type>                         range_type;
        typedef typename Allocator::pointer                             pointer;
        typedef typename Allocator::const_pointer                       const_pointer;
        typedef stride_iterator< value_type* >                          matrix_stride_iterator;
        typedef value_type*                                             row_type;
        typedef const value_type*                                       const_row_type;
        typedef stride_iterator< value_type* >                          col_type;
        typedef stride_iterator< const value_type* >                    const_col_type;
        typedef stride_iterator< value_type* >                          diag_type;
        typedef stride_iterator< const value_type* >                    const_diag_type;
        typedef stride_iterator< value_type* >                          anti_diag_type;
        typedef stride_iterator< const value_type* >                    const_anti_diag_type;
        typedef std::reverse_iterator< iterator >                       reverse_iterator;
        typedef std::reverse_iterator< const_iterator >                 const_reverse_iterator;
        typedef std::reverse_iterator< matrix_stride_iterator >         reverse_matrix_stride_iterator;
        typedef std::reverse_iterator< row_type >                       reverse_row_type;
        typedef std::reverse_iterator< const_row_type >                 const_reverse_row_type;
        typedef std::reverse_iterator< col_type >                       reverse_col_type;
        typedef std::reverse_iterator< const_col_type >                 const_reverse_col_type;
        typedef std::reverse_iterator< diag_type >                      reverse_upper_diag_type;
        typedef std::reverse_iterator< const_diag_type >                const_reverse_upper_diag_type;
        typedef std::reverse_iterator< diag_type >                      reverse_lower_diag_type;
        typedef std::reverse_iterator< const_diag_type >                const_reverse_lower_diag_type;
        typedef std::reverse_iterator< diag_type >                      reverse_diag_type;
        typedef std::reverse_iterator< const_diag_type >                const_reverse_diag_type;
        typedef std::reverse_iterator< anti_diag_type >                 reverse_anti_diag_type;
        typedef std::reverse_iterator< const_anti_diag_type >           const_reverse_anti_diag_type;
    };

    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_shape
    {
        typedef Matrix zen_type;
        auto shape() const noexcept
        {
            auto const& zen = static_cast<zen_type const&>(*this);
            return std::make_tuple( zen.row(), zen.col() );
        }
    };

    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_get_allocator
    {
        typedef Matrix zen_type;
        auto get_allocator() const noexcept
        {
            auto const& zen = static_cast<zen_type const&>(*this);
            return zen.allocator_;
        }
    };

    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_anti_diag_iterator
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::difference_type difference_type;
        typedef typename type_proxy_type::anti_diag_type anti_diag_type;
        typedef typename type_proxy_type::const_anti_diag_type const_anti_diag_type;
        typedef typename type_proxy_type::reverse_anti_diag_type reverse_anti_diag_type;
        typedef typename type_proxy_type::const_reverse_anti_diag_type const_reverse_anti_diag_type;
        anti_diag_type upper_anti_diag_begin( const size_type index = 0 ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            return anti_diag_type( zen.begin() + zen.col() - index - 1, zen.col() - 1 );
        }
        anti_diag_type upper_anti_diag_end( const size_type index = 0 ) noexcept
        {
            zen_type& zen   = static_cast< zen_type& >( *this );
            size_type depth = zen.col() - index;

            if ( zen.row() < depth )
            {
                depth = zen.row();
            }

            return upper_anti_diag_begin( index ) + depth;
        }
        const_anti_diag_type upper_anti_diag_begin( const size_type index = 0 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return const_anti_diag_type( zen.begin() + zen.col() - index - 1, zen.col() - 1 );
        }
        const_anti_diag_type upper_anti_diag_end( const size_type index = 0 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            size_type depth     = zen.col() - index;

            if ( zen.row() < depth )
            {
                depth = zen.row();
            }

            return upper_anti_diag_begin( index ) + depth;
        }
        const_anti_diag_type upper_anti_diag_cbegin( const size_type index = 0 ) const noexcept
        {
            return upper_anti_diag_begin( index );
        }
        const_anti_diag_type upper_anti_diag_cend( const size_type index = 0 ) const noexcept
        {
            return upper_anti_diag_end( index );
        }
        reverse_anti_diag_type upper_anti_diag_rbegin( const size_type index = 0 ) noexcept
        {
            return reverse_anti_diag_type( upper_anti_diag_end( index ) );
        }
        reverse_anti_diag_type upper_anti_diag_rend( const size_type index = 0 ) noexcept
        {
            return reverse_anti_diag_type( upper_anti_diag_begin( index ) );
        }
        const_reverse_anti_diag_type upper_anti_diag_rbegin( const size_type index = 0 ) const noexcept
        {
            return const_reverse_anti_diag_type( upper_anti_diag_end( index ) );
        }
        const_reverse_anti_diag_type upper_anti_diag_rend( const size_type index = 0 ) const noexcept
        {
            return const_reverse_anti_diag_type( upper_anti_diag_begin( index ) );
        }
        const_reverse_anti_diag_type upper_anti_diag_crbegin( const size_type index = 0 ) const noexcept
        {
            return upper_anti_diag_rbegin( index );
        }
        const_reverse_anti_diag_type upper_anti_diag_crend( const size_type index = 0 ) const noexcept
        {
            return upper_anti_diag_rend( index );
        }
        anti_diag_type lower_anti_diag_begin( const size_type index = 0 ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            return anti_diag_type( zen.begin() + ( zen.col() * ( index + 1 ) ) - 1, zen.col() - 1 );
        }
        anti_diag_type lower_anti_diag_end( const size_type index = 0 ) noexcept
        {
            zen_type& zen   = static_cast< zen_type& >( *this );
            size_type depth = zen.row() - index;

            if ( zen.col() < depth )
            {
                depth = zen.col();
            }

            return lower_anti_diag_begin( index ) + depth;
        }
        const_anti_diag_type lower_anti_diag_begin( const size_type index = 0 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return const_anti_diag_type( zen.begin() + ( zen.col() * ( index + 1 ) ) - 1, zen.col() - 1 );
        }
        const_anti_diag_type lower_anti_diag_end( const size_type index = 0 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            size_type depth     = zen.row() - index;

            if ( zen.col() < depth )
            {
                depth = zen.col();
            }

            return lower_anti_diag_begin( index ) + depth;
        }
        const_anti_diag_type lower_anti_diag_cbegin( const size_type index = 0 ) const noexcept
        {
            return lower_anti_diag_begin( index );
        }
        const_anti_diag_type lower_anti_diag_cend( const size_type index = 0 ) const noexcept
        {
            return lower_anti_diag_end( index );
        }
        reverse_anti_diag_type lower_anti_diag_rbegin( const size_type index = 0 ) noexcept
        {
            return reverse_anti_diag_type( lower_anti_diag_end( index ) );
        }
        reverse_anti_diag_type lower_anti_diag_rend( const size_type index = 0 ) noexcept
        {
            return reverse_anti_diag_type( lower_anti_diag_begin( index ) );
        }
        const_reverse_anti_diag_type lower_anti_diag_rbegin( const size_type index = 0 ) const noexcept
        {
            return const_reverse_anti_diag_type( lower_anti_diag_end( index ) );
        }
        const_reverse_anti_diag_type lower_anti_diag_rend( const size_type index = 0 ) const noexcept
        {
            return const_reverse_anti_diag_type( lower_anti_diag_begin( index ) );
        }
        const_reverse_anti_diag_type lower_anti_diag_crbegin( const size_type index = 0 ) const noexcept
        {
            return lower_anti_diag_rbegin( index );
        }
        const_reverse_anti_diag_type lower_anti_diag_crend( const size_type index = 0 ) const noexcept
        {
            return lower_anti_diag_rend( index );
        }
        anti_diag_type anti_diag_begin( const difference_type index = 0 ) noexcept
        {
            if ( index > 0 )
            {
                return upper_anti_diag_begin( index );
            }

            return lower_anti_diag_begin( -index );
        }
        anti_diag_type anti_diag_end( const difference_type index = 0 ) noexcept
        {
            if ( index > 0 )
            {
                return upper_anti_diag_end( index );
            }

            return lower_anti_diag_end( -index );
        }
        const_anti_diag_type anti_diag_begin( const difference_type index = 0 ) const noexcept
        {
            if ( index > 0 )
            {
                return upper_anti_diag_begin( index );
            }

            return lower_anti_diag_begin( -index );
        }
        const_anti_diag_type anti_diag_end( const difference_type index = 0 ) const noexcept
        {
            if ( index > 0 )
            {
                return upper_anti_diag_end( index );
            }

            return lower_anti_diag_end( -index );
        }
        const_anti_diag_type anti_diag_cbegin( const difference_type index = 0 ) const noexcept
        {
            if ( index > 0 )
            {
                return upper_anti_diag_cbegin( index );
            }

            return lower_anti_diag_cbegin( -index );
        }
        const_anti_diag_type anti_diag_cend( const difference_type index = 0 ) const noexcept
        {
            if ( index > 0 )
            {
                return upper_anti_diag_cend( index );
            }

            return lower_anti_diag_cend( -index );
        }
        reverse_anti_diag_type anti_diag_rbegin( const difference_type index = 0 ) noexcept
        {
            if ( index > 0 )
            {
                return upper_anti_diag_rbegin( index );
            }

            return lower_anti_diag_rbegin( -index );
        }
        reverse_anti_diag_type anti_diag_rend( const difference_type index = 0 ) noexcept
        {
            if ( index > 0 )
            {
                return upper_anti_diag_rend( index );
            }

            return lower_anti_diag_rend( -index );
        }
        const_reverse_anti_diag_type anti_diag_rbegin( const difference_type index = 0 ) const noexcept
        {
            if ( index > 0 )
            {
                return upper_anti_diag_rbegin( index );
            }

            return lower_anti_diag_rbegin( -index );
        }
        const_reverse_anti_diag_type anti_diag_rend( const difference_type index = 0 ) const noexcept
        {
            if ( index > 0 )
            {
                return upper_anti_diag_rend( index );
            }

            return lower_anti_diag_rend( -index );
        }
        const_reverse_anti_diag_type anti_diag_crbegin( const difference_type index = 0 ) const noexcept
        {
            if ( index > 0 )
            {
                return upper_anti_diag_crbegin( index );
            }

            return lower_anti_diag_crbegin( -index );
        }
        const_reverse_anti_diag_type anti_diag_crend( const difference_type index = 0 ) const noexcept
        {
            if ( index > 0 )
            {
                return upper_anti_diag_crend( index );
            }

            return lower_anti_diag_crend( -index );
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_apply
    {
        typedef Matrix zen_type;
        template < typename Function >
        void apply( const Function& func ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            for (auto& x : zen )
                func(x);
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_bracket_operator
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::value_type value_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::row_type row_type;
        typedef typename type_proxy_type::const_row_type const_row_type;
        row_type operator[]( const size_type index ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            assert( index < zen.row() && "Row index outof boundary!" );
            return zen.row_begin( index );
        }
        const_row_type operator[]( const size_type index ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            assert( index < zen.row() && "Row index outof boundary!" );
            return zen.row_begin( index );
        }
        value_type operator()( size_type r, size_type c ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            assert( r < zen.row() && "Row index out of boundary!" );
            assert( c < zen.col() && "Column index out of boundary!" );
            return zen[r][c];
        }
        value_type& operator()( size_type r, size_type c ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            assert( r < zen.row() && "Row index out of boundary!" );
            assert( c < zen.col() && "Column index out of boundary!" );
            return zen[r][c];
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_clear
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        void clear() noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );

            zen.get_allocator().deallocate( zen.data(), zen.size() );
            zen.dat_ = nullptr;
            zen.row_ = 0;
            zen.col_ = 0;
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_clone
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        template < typename Other_Matrix >
        zen_type& clone( const Other_Matrix& other, std::initializer_list<size_type> r, std::initializer_list<size_type> c ) noexcept
        {
            assert( r.size() == 2 && "row size should be 2!" );
            assert( c.size() == 2 && "col size should be 2!" );
            return clone( other, r[0], r[1], c[0], c[1] );
        }
        template < typename Other_Matrix >
        zen_type& clone( const Other_Matrix& other, size_type const r0, size_type const r1, size_type const c0, size_type const c1 ) noexcept
        {
            assert( r1 > r0 && "row range error!" );
            assert( c1 > c0 && "col range error!" );

            zen_type& zen = static_cast< zen_type& >( *this );
            zen_type tmp{ zen.get_allocator(), r1-r0, c1-c0 };

            for ( size_type r = 0; r != tmp.row(); ++r )
                std::copy_n( other.row_begin(r+r0)+c0, tmp.col(), tmp.row_begin(r) );

            zen.swap( tmp );
            return zen;
        }

        zen_type const clone( std::initializer_list<size_type> r, std::initializer_list<size_type> c ) const noexcept
        {
            assert( r.size() == 2 && "row size should be 2!" );
            assert( c.size() == 2 && "col size should be 2!" );
            return clone( r[0], r[1], c[0], c[1] );
        }
        zen_type const clone( size_type const r0, size_type const r1, size_type const c0, size_type const c1 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            zen_type ans{ zen.get_allocator() };
            ans.clone( zen, r0, r1, c0, c1 );
            return ans;
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_col_iterator
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::col_type col_type;
        typedef typename type_proxy_type::const_col_type const_col_type;
        typedef typename type_proxy_type::reverse_col_type reverse_col_type;
        typedef typename type_proxy_type::const_reverse_col_type const_reverse_col_type;
        col_type col_begin( const size_type index ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            return col_type( zen.begin() + index, zen.col() );
        }
        col_type col_end( const size_type index ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            return col_begin( index ) + zen.row();
        }
        const_col_type col_begin( const size_type index ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return const_col_type( zen.begin() + index, zen.col() );
        }
        const_col_type col_end( const size_type index ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return col_begin( index ) + zen.row();
        }
        const_col_type col_cbegin( const size_type index ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return const_col_type( zen.begin() + index, zen.col() );
        }
        const_col_type col_cend( const size_type index ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return col_begin( index ) + zen.row();
        }
        reverse_col_type col_rbegin( const size_type index = 0 ) noexcept
        {
            return reverse_col_type( col_end( index ) );
        }
        reverse_col_type col_rend( const size_type index = 0 ) noexcept
        {
            return reverse_col_type( col_begin( index ) );
        }
        const_reverse_col_type col_rbegin( const size_type index = 0 ) const noexcept
        {
            return const_reverse_col_type( col_end( index ) );
        }
        const_reverse_col_type col_rend( const size_type index = 0 ) const noexcept
        {
            return const_reverse_col_type( col_begin( index ) );
        }
        const_reverse_col_type col_crbegin( const size_type index = 0 ) const noexcept
        {
            return const_reverse_col_type( col_end( index ) );
        }
        const_reverse_col_type col_crend( const size_type index = 0 ) const noexcept
        {
            return const_reverse_col_type( col_begin( index ) );
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_copy
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        template < typename Other_Matrix >
        void copy( const Other_Matrix& rhs ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            zen.allocator_ = rhs.allocator_;
            zen.resize( rhs.row(), rhs.col() );
            std::copy( rhs.begin(), rhs.end(), zen.begin() );//<- should be overloaded when with cuda_allocator
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_data
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::pointer pointer;
        typedef typename type_proxy_type::const_pointer const_pointer;
        pointer data() noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            return zen.dat_;
        }
        const_pointer data() const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return zen.dat_;
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_det
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::value_type value_type;
        typedef typename type_proxy_type::range_type range_type;
        value_type det() const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            assert( zen.row() == zen.col() );

            if ( 0 == zen.size() )
            {
                return value_type();
            }

            if ( 1 == zen.size() )
            {
                return *( zen.begin() );
            }

            if ( 4 == zen.size() )
            {
                return zen[0][0] * zen[1][1] - zen[1][0] * zen[0][1];
            }

            size_type const n = zen.row();
            size_type const m = n >> 1;
            zen_type const P( zen, range_type( 0, m ), range_type( 0, m ) );
            zen_type const Q( zen, range_type( 0, m ), range_type( m, n ) );
            zen_type const R( zen, range_type( m, n ), range_type( 0, m ) );
            zen_type const S( zen, range_type( m, n ), range_type( m, n ) );
            zen_type const& tmp = S - ( R * ( P.inverse() ) * Q );

            return P.det() * tmp.det();
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_diag_iterator
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::difference_type difference_type;
        typedef typename type_proxy_type::diag_type diag_type;
        typedef typename type_proxy_type::const_diag_type const_diag_type;
        typedef typename type_proxy_type::reverse_upper_diag_type reverse_upper_diag_type;
        typedef typename type_proxy_type::const_reverse_upper_diag_type const_reverse_upper_diag_type;
        typedef typename type_proxy_type::reverse_lower_diag_type reverse_lower_diag_type;
        typedef typename type_proxy_type::const_reverse_lower_diag_type const_reverse_lower_diag_type;
        typedef typename type_proxy_type::reverse_diag_type reverse_diag_type;
        typedef typename type_proxy_type::const_reverse_diag_type const_reverse_diag_type;
        diag_type upper_diag_begin( const size_type index ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            return diag_type( zen.begin() + index, zen.col() + 1 );
        }
        diag_type upper_diag_end( const size_type index ) noexcept
        {
            zen_type& zen   = static_cast< zen_type& >( *this );
            size_type depth = std::min( zen.col()-index, zen.row() );
            /*
            size_type depth = zen.col() - index;

            if ( zen.row() < depth )
            {
                depth = zen.row();
            }
            */
            return diag_type( upper_diag_begin( index ) + depth );
        }
        const_diag_type upper_diag_begin( const size_type index ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return const_diag_type( zen.begin() + index, zen.col() + 1 );
        }
        const_diag_type upper_diag_end( const size_type index ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            size_type depth = std::min( zen.col()-index, zen.row() );
            /*
            size_type depth     = zen.col() - index;

            if ( zen.row() < depth )
            {
                depth = zen.row();
            }
            */
            return upper_diag_begin( index ) + depth;
        }
        const_diag_type upper_diag_cbegin( const size_type index ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return const_diag_type( zen.cbegin() + index, zen.col() + 1 );
        }
        const_diag_type upper_diag_cend( const size_type index ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            size_type depth = std::min( zen.col()-index, zen.row() );
            return upper_diag_cbegin( index ) + depth;
        }
        reverse_upper_diag_type upper_diag_rbegin( const size_type index = 0 ) noexcept
        {
            return reverse_upper_diag_type( upper_diag_end( index ) );
        }
        reverse_upper_diag_type upper_diag_rend( const size_type index = 0 ) noexcept
        {
            return reverse_upper_diag_type( upper_diag_begin( index ) );
        }
        const_reverse_upper_diag_type upper_diag_rbegin( const size_type index = 0 ) const noexcept
        {
            return const_reverse_upper_diag_type( upper_diag_end( index ) );
        }
        const_reverse_upper_diag_type upper_diag_rend( const size_type index = 0 ) const noexcept
        {
            return const_reverse_upper_diag_type( upper_diag_begin( index ) );
        }
        const_reverse_upper_diag_type upper_diag_crbegin( const size_type index = 0 ) const noexcept
        {
            return const_reverse_upper_diag_type( upper_diag_end( index ) );
        }
        const_reverse_upper_diag_type upper_diag_crend( const size_type index = 0 ) const noexcept
        {
            return const_reverse_upper_diag_type( upper_diag_begin( index ) );
        }
        diag_type lower_diag_begin( const size_type index ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            return diag_type( zen.begin() + index * zen.col(), zen.col() + 1 );
        }
        diag_type lower_diag_end( const size_type index ) noexcept
        {
            zen_type& zen   = static_cast< zen_type& >( *this );
            size_type depth = std::min( zen.row()-index, zen.col() );
            /*
            size_type depth = zen.row() - index;

            if ( zen.col() < depth )
            {
                depth = zen.col();
            }
            */
            return lower_diag_begin( index ) + depth;
        }
        const_diag_type lower_diag_begin( const size_type index ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return const_diag_type( zen.begin() + index * zen.col(), zen.col() + 1 );
        }
        const_diag_type lower_diag_end( const size_type index ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            size_type depth = std::min( zen.row()-index, zen.col() );
            return lower_diag_begin( index ) + depth;
        }
        const_diag_type lower_diag_cbegin( const size_type index ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return const_diag_type( zen.begin() + index * zen.col(), zen.col() + 1 );
        }
        const_diag_type lower_diag_cend( const size_type index ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            size_type depth = std::min( zen.row()-index, zen.col() );
            return lower_diag_begin( index ) + depth;
        }
        reverse_lower_diag_type lower_diag_rbegin( const size_type index = 0 ) noexcept
        {
            return reverse_lower_diag_type( lower_diag_end( index ) );
        }
        reverse_lower_diag_type lower_diag_rend( const size_type index = 0 ) noexcept
        {
            return reverse_lower_diag_type( lower_diag_begin( index ) );
        }
        const_reverse_lower_diag_type lower_diag_rbegin( const size_type index = 0 ) const noexcept
        {
            return const_reverse_lower_diag_type( lower_diag_end( index ) );
        }
        const_reverse_lower_diag_type lower_diag_rend( const size_type index = 0 ) const noexcept
        {
            return const_reverse_lower_diag_type( lower_diag_begin( index ) );
        }
        const_reverse_lower_diag_type lower_diag_crbegin( const size_type index = 0 ) const noexcept
        {
            return const_reverse_lower_diag_type( lower_diag_end( index ) );
        }
        const_reverse_lower_diag_type lower_diag_crend( const size_type index = 0 ) const noexcept
        {
            return const_reverse_lower_diag_type( lower_diag_begin( index ) );
        }
        diag_type diag_begin( const difference_type index = 0 ) noexcept
        {
            if ( index > 0 ) return upper_diag_begin( index );
            return lower_diag_begin( -index );
        }
        diag_type diag_end( const difference_type index = 0 ) noexcept
        {
            if ( index > 0 ) return upper_diag_end( index );
            return lower_diag_end( -index );
        }
        const_diag_type diag_begin( const difference_type index = 0 ) const noexcept
        {
            if ( index > 0 ) return upper_diag_begin( index );
            return lower_diag_begin( -index );
        }
        const_diag_type diag_end( const difference_type index = 0 ) const noexcept
        {
            if ( index > 0 ) return upper_diag_end( index );
            return lower_diag_end( -index );
        }
        const_diag_type diag_cbegin( const difference_type index = 0 ) const noexcept
        {
            if ( index > 0 ) return upper_diag_cbegin( index );
            return lower_diag_cbegin( -index );
        }
        const_diag_type diag_cend( const difference_type index = 0 ) const noexcept
        {
            if ( index > 0 ) return upper_diag_cend( index );
            return lower_diag_cend( -index );
        }
        reverse_diag_type diag_rbegin( const difference_type index = 0 ) noexcept
        {
            if ( index > 0 ) return upper_diag_rbegin( index );
            return lower_diag_rbegin( -index );
        }
        reverse_diag_type diag_rend( const difference_type index = 0 ) noexcept
        {
            if ( index > 0 ) return upper_diag_rend( index );
            return lower_diag_rend( -index );
        }
        const_reverse_diag_type diag_rbegin( const difference_type index = 0 ) const noexcept
        {
            if ( index > 0 ) return upper_diag_rbegin( index );
            return lower_diag_rbegin( -index );
        }
        const_reverse_diag_type diag_rend( const difference_type index = 0 ) const noexcept
        {
            if ( index > 0 ) return upper_diag_rend( index );
            return lower_diag_rend( -index );
        }
        const_reverse_diag_type diag_crbegin( const difference_type index = 0 ) const noexcept
        {
            if ( index > 0 ) return upper_diag_crbegin( index );
            return lower_diag_crbegin( -index );
        }
        const_reverse_diag_type diag_crend( const difference_type index = 0 ) const noexcept
        {
            if ( index > 0 ) return upper_diag_crend( index );
            return lower_diag_crend( -index );
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_direct_iterator
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::iterator iterator;
        typedef typename type_proxy_type::reverse_iterator reverse_iterator;
        typedef typename type_proxy_type::const_iterator const_iterator;
        typedef typename type_proxy_type::const_reverse_iterator const_reverse_iterator;
        iterator begin() noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            return zen.data();
        }
        iterator end() noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            return zen.begin()+zen.size();
        }
        const_iterator begin() const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return zen.data();
        }
        const_iterator end() const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return zen.begin()+zen.size();
        }
        const_iterator cbegin() const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return zen.data();
        }
        const_iterator cend() const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return zen.begin()+zen.size();
        }
        reverse_iterator rbegin() noexcept
        {
            return reverse_iterator( end() );
        }
        reverse_iterator rend() noexcept
        {
            return reverse_iterator( begin() );
        }
        const_reverse_iterator rbegin() const noexcept
        {
            return const_reverse_iterator( end() );
        }
        const_reverse_iterator rend() const noexcept
        {
            return const_reverse_iterator( begin() );
        }
        const_reverse_iterator crbegin() const noexcept
        {
            return const_reverse_iterator( end() );
        }
        const_reverse_iterator crend() const noexcept
        {
            return const_reverse_iterator( begin() );
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_divide_equal_operator
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::value_type value_type;
        zen_type& operator/=( const value_type& rhs ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );

            for ( auto& v : zen )
                v /= rhs;

            return zen;
        }
        zen_type& operator/=( const zen_type& rhs ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            zen *= rhs.inverse();
            return zen;
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_inverse
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::value_type value_type;
        typedef typename type_proxy_type::range_type range_type;
        const zen_type inverse() const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );

            assert( zen.row() == zen.col() && "matrix is not square!" );

            size_type const n = zen.row();
            zen_type a( zen.get_allocator(),  n, n + n );
            std::fill( a.begin(), a.end(), value_type{0} );

            for ( size_type i = 0; i != n; ++i )
            {
                std::copy( zen.row_begin( i ), zen.row_end( i ), a.row_begin( i ) );
            }

            std::fill( a.upper_diag_begin( n ), a.upper_diag_end( n ), value_type( 1 ) );

            for ( size_type i = 0; i < n; ++i )
            {
                const size_type p = std::distance( a.col_begin( i ), std::max_element( a.col_begin( i ) + i, a.col_end( i ), []( value_type x, value_type y )
                {
                    return std::abs( x ) < std::abs( y );
                } ) );

                if ( p != i )
                {
                    std::swap_ranges( a.row_begin( i ) + i, a.row_end( i ), a.row_begin( p ) + i );
                }

                const value_type factor = a[i][i];

                assert( std::abs(factor) >= std::numeric_limits<value_type>::epsilon() && "Failed inversing matrix, too small factor for Gaussian elimination!" );

                std::for_each( a.row_rbegin( i ), a.row_rend( i ) - i, [factor]( value_type & x ) noexcept
                {
                    x /= factor;
                } );

                for ( size_type j = 0; j < n; ++j )
                {
                    if ( i == j )
                    {
                        continue;
                    }

                    const value_type ratio = a[j][i];
                    std::transform( a.row_rbegin( j ), a.row_rend( j ) - i, a.row_rbegin( i ), a.row_rbegin( j ), [ratio]( value_type x, value_type y ) noexcept
                    {
                        return x - y * ratio;
                    } );
                }
            }

            return zen_type{ a, range_type{ 0, n }, range_type{ n, n + n } };
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_load_txt
    {
        typedef Matrix zen_type;
        typedef typename crtp_typedef< Type, Allocator >::value_type value_type;
        typedef typename crtp_typedef< Type, Allocator >::size_type size_type;
        bool load_txt( std::string const& file_name ) noexcept
        {
            return load_txt(file_name.c_str());
        }
        bool load_txt( const char* const file_name ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            std::ifstream ifs( file_name );
            assert( ifs && "matrix::load_ascii -- failed to open file" );
            std::stringstream iss;
            std::copy( std::istreambuf_iterator< char >( ifs ), std::istreambuf_iterator< char >(), std::ostreambuf_iterator< char >( iss ) );
            std::string cache = iss.str();
            std::for_each( cache.begin(), cache.end(), []( auto & ch )
            {
                if ( ch == ',' || ch == ';' )
                    ch = ' ';
            } );
            iss.str( cache );
            std::vector< value_type > buff;
            std::copy( std::istream_iterator< value_type >( iss ), std::istream_iterator< value_type >(), std::back_inserter( buff ) );
            size_type const total_elements = buff.size();
            const std::string& stream_buff = iss.str();
            size_type const r_             = std::count( stream_buff.begin(), stream_buff.end(), '\n' );
            size_type const r              = *( stream_buff.rbegin() ) == '\n' ? r_ : r_ + 1;
            size_type const c              = total_elements / r;

            if ( r * c != total_elements )
            {
                std::cerr << "Error: Failed to match matrix size.\n \tthe size of matrix stored in file \"" << file_name << "\" is " << buff.size() << ".\n";
                std::cerr << " \tthe size of the destination matrix is " << r << " by " << c << " elements.\n";
                return false;
            }

            zen.resize( r, c );
            std::copy( buff.begin(), buff.end(), zen.begin() );
            ifs.close();
            return true;
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_load_binary
    {
        typedef Matrix zen_type;
        typedef typename crtp_typedef< Type, Allocator >::value_type value_type;
        typedef typename crtp_typedef< Type, Allocator >::size_type size_type;
        bool load_binary( std::string const& file_name ) noexcept
        {
            return load_binary( file_name.c_str() );
        }
        bool load_binary( char const* const file_name ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            std::ifstream ifs( file_name, std::ios::binary );
            assert( ifs && "matrix::load_binary -- failed to open file" );

            if ( !ifs )
                return false;

            std::vector< char > buffer{ ( std::istreambuf_iterator< char >( ifs ) ), ( std::istreambuf_iterator< char >() ) };
            assert( buffer.size() >= sizeof( size_type ) + sizeof( size_type ) && "matrix::load_library -- file too small, maybe be damaged" );

            if ( buffer.size() <= sizeof( size_type ) + sizeof( size_type ) )
                return false;

            size_type r;
            std::copy( buffer.begin(), buffer.begin() + sizeof( r ), reinterpret_cast< char* >( std::addressof( r ) ) );
            size_type c;
            std::copy( buffer.begin() + sizeof( r ), buffer.begin() + sizeof( r ) + sizeof( c ), reinterpret_cast< char* >( std::addressof( c ) ) );
            zen.resize( r, c );
            assert( buffer.size() == sizeof( r ) + sizeof( c ) + sizeof( Type ) * zen.size() && "matrix::load_binary -- data does not match, maybe damaged" );

            if ( buffer.size() != sizeof( r ) + sizeof( c ) + sizeof( Type ) * zen.size() )
                return false;

            std::copy( buffer.begin() + sizeof( r ) + sizeof( c ), buffer.end(), reinterpret_cast< char* >( zen.data() ) );
            return true;
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_minus_equal_operator
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::value_type value_type;
        zen_type& operator-=( const value_type& rhs ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );

            for ( auto& v : zen )
                v -= rhs;

            return zen;
        }
        zen_type& operator-=( const zen_type& rhs ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            std::transform( zen.begin(), zen.end(), rhs.begin(), zen.begin(), std::minus< value_type >() );
            return zen;
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_multiply_equal_operator
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::value_type value_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::range_type range_type;
        zen_type& operator*=( const value_type& rhs ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );

            for ( auto& v : zen )
                v *= rhs;

            return zen;
        }
        zen_type& direct_multiply( const zen_type& other ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            assert( zen.col() == other.row() && "direct_multiply: dimesion not match!" );
            zen_type tmp( zen.row(), other.col() );

            for ( size_type i = 0; i < tmp.row(); ++i )
                for ( size_type j = 0; j < tmp.col(); ++j )
                {
                    tmp[i][j] = std::inner_product( zen.row_begin( i ), zen.row_end( i ), other.col_begin( j ), value_type( 0 ) );
                }

            zen.clone( tmp, 0, zen.row(), 0, other.col() );
            return zen;
        }
        zen_type& rr1( const zen_type& other ) noexcept
        {
            zen_type& zen            = static_cast< zen_type& >( *this );
            const zen_type& new_this = zen && value_type( 0 );
            const zen_type& new_ans  = new_this * other;
            zen.clone( new_ans, 0, zen.row(), 0, other.col() );
            return zen;
        }
        zen_type& rr2( const zen_type& other ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            const zen_type new_this( zen, range_type( 0, zen.row() - 1 ), range_type( 0, zen.col() ) );
            const zen_type last_row( zen, range_type( zen.row() - 1, zen.row() ), range_type( 0, zen.col() ) );
            const zen_type& new_ans      = new_this * other;
            const zen_type& last_row_ans = last_row * other;
            const zen_type& ans          = new_ans && last_row_ans;
            zen.clone( ans, 0, zen.row(), 0, other.col() );
            return zen;
        }
        zen_type& cc1( const zen_type& other ) noexcept
        {
            zen_type& zen             = static_cast< zen_type& >( *this );
            const zen_type& new_this  = zen || value_type( 0 );
            const zen_type& new_other = other && value_type( 0 );
            const zen_type& ans       = new_this * new_other;
            zen.clone( ans, 0, zen.row(), 0, other.col() );
            return zen;
        }
        zen_type& cc2( const zen_type& other ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            const zen_type new_this( zen, range_type( 0, zen.row() ), range_type( 0, zen.col() - 1 ) );
            const zen_type last_col( zen, range_type( 0, zen.row() ), range_type( zen.col() - 1, zen.col() ) );
            const zen_type new_other( other, range_type( 0, other.row() - 1 ), range_type( 0, other.col() ) );
            const zen_type last_row( other, range_type( other.row() - 1, other.row() ), range_type( 0, other.col() ) );
            const zen_type& new_ans     = new_this * new_other;
            const zen_type& res_col_row = last_col * last_row;
            const zen_type& ans         = new_ans + res_col_row;
            zen.clone( ans, 0, zen.row(), 0, other.col() );
            return zen;
        }
        zen_type& oc1( const zen_type& other ) noexcept
        {
            zen_type& zen             = static_cast< zen_type& >( *this );
            const zen_type& new_other = other || value_type( 0 );
            const zen_type& new_ans   = zen * new_other;
            zen.clone( new_ans, 0, zen.row(), 0, other.col() );
            return zen;
        }
        zen_type& oc2( const zen_type& other ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            const zen_type new_other( other, range_type( 0, other.row() ), range_type( 0, other.col() - 1 ) );
            const zen_type last_col( other, range_type( 0, other.row() ), range_type( other.col() - 1, other.col() ) );
            const zen_type& new_ans      = zen * new_other;
            const zen_type& last_col_ans = zen * last_col;
            const zen_type& ans          = new_ans || last_col_ans;
            zen.clone( ans, 0, zen.row(), 0, other.col() );
            return zen;
        }
        zen_type& strassen_multiply( const zen_type& other ) noexcept
        {
            zen_type& zen        = static_cast< zen_type& >( *this );
            const size_type R_2  = zen.row() >> 1;
            const size_type C_2  = zen.col() >> 1;
            const size_type OR_2 = C_2;
            const size_type OC_2 = other.col() >> 1;
            const zen_type a_00( zen, range_type( 0, R_2 ), range_type( 0, C_2 ) );
            const zen_type a_01( zen, range_type( 0, R_2 ), range_type( C_2, zen.col() ) );
            const zen_type a_10( zen, range_type( R_2, zen.row() ), range_type( 0, C_2 ) );
            const zen_type a_11( zen, range_type( R_2, zen.row() ), range_type( C_2, zen.col() ) );
            const zen_type b_00( other, range_type( 0, OR_2 ), range_type( 0, OC_2 ) );
            const zen_type b_01( other, range_type( 0, OR_2 ), range_type( OC_2, other.col() ) );
            const zen_type b_10( other, range_type( OR_2, other.row() ), range_type( 0, OC_2 ) );
            const zen_type b_11( other, range_type( OR_2, other.row() ), range_type( OC_2, other.col() ) );
            const zen_type& Q_0  = ( a_00 + a_11 ) * ( b_00 + b_11 );
            const zen_type& Q_1  = ( a_10 + a_11 ) * b_00;
            const zen_type& Q_2  = a_00 * ( b_01 - b_11 );
            const zen_type& Q_3  = a_11 * ( -b_00 + b_10 );
            const zen_type& Q_4  = ( a_00 + a_01 ) * b_11;
            const zen_type& Q_5  = ( -a_00 + a_10 ) * ( b_00 + b_01 );
            const zen_type& Q_6  = ( a_01 - a_11 ) * ( b_10 + b_11 );
            const zen_type& c_00 = Q_0 + Q_3 - Q_4 + Q_6;
            const zen_type& c_10 = Q_1 + Q_3;
            const zen_type& c_01 = Q_2 + Q_4;
            const zen_type& c_11 = Q_0 - Q_1 + Q_2 + Q_5;
            const zen_type& ans  = ( c_00 || c_01 ) && ( c_10 || c_11 );
            zen.clone( ans, 0, zen.row(), 0, other.col() );
            return zen;
        }
        zen_type& operator*=( const zen_type& other ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            assert( zen.col() == other.row() );
            static const size_type threshold = 17;
            const size_type max_dims         = std::max( std::max( zen.row(), zen.col() ), other.col() );
            const size_type min_dims         = std::min( std::min( zen.row(), zen.col() ), other.col() );

            if ( ( max_dims < threshold ) || ( min_dims == 1 ) )
            {
                return direct_multiply( other );
            }

            const size_type R  = zen.row();
            const size_type C  = zen.col();
            const size_type OC = other.col();

            if ( R & 1 )
            {
                if ( R & 2 )
                {
                    return rr1( other );
                }

                return rr2( other );
            }

            if ( C & 1 )
            {
                if ( C & 2 )
                {
                    return cc1( other );
                }

                return cc2( other );
            }

            if ( OC & 1 )
            {
                if ( OC & 2 )
                {
                    return oc1( other );
                }

                return oc2( other );
            }

            return strassen_multiply( other );
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_plus_equal_operator
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::value_type value_type;
        zen_type& operator+=( const value_type& rhs ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );

            for ( auto& v : zen )
                v += rhs;

            return zen;
        }
        zen_type& operator+=( const zen_type& rhs ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            std::transform( zen.begin(), zen.end(), rhs.begin(), zen.begin(), std::plus< value_type >() );
            return zen;
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_prefix_minus
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::value_type value_type;
        const zen_type operator-() const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            zen_type ans{ zen };
            std::transform( ans.begin(), ans.end(), ans.begin(), []( value_type x )
            {
                return -x;
            } );
            return ans;
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_prefix_plus
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::value_type value_type;
        typedef typename type_proxy_type::size_type size_type;
        const zen_type operator+() const noexcept
        {
            return static_cast< zen_type const& >( *this );
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_reshape
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        void reshape( const size_type new_row, const size_type new_col ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            assert( new_row * new_col == zen.row() * zen.col() && "error: size before and after reshape does not agree, use resize() instead!" );
            zen.row_ = new_row;
            zen.col_ = new_col;
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_resize
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::value_type value_type;
        zen_type& resize( const size_type new_row, const size_type new_col ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );

            if ( zen.size() == new_row * new_col )
            {
                zen.reshape( new_row, new_col );
                return zen;
            }

            zen_type ans{ zen.get_allocator(), new_row, new_col };
            zen.swap( ans );
            return zen;
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_row_col_size
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        size_type row() const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return zen.row_;
        }
        size_type col() const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return zen.col_;
        }
        size_type size() const noexcept
        {
            return row()*col();
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_row_iterator
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::row_type row_type;
        typedef typename type_proxy_type::const_row_type const_row_type;
        typedef typename type_proxy_type::reverse_row_type reverse_row_type;
        typedef typename type_proxy_type::const_reverse_row_type const_reverse_row_type;

        row_type row_begin( const size_type index = 0 ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            return row_type( zen.begin() + index * zen.col() );
        }
        row_type row_end( const size_type index = 0 ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            return row_begin( index ) + zen.col();
        }
        const_row_type row_begin( const size_type index = 0 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return const_row_type( zen.begin() + index * zen.col() );
        }
        const_row_type row_end( const size_type index = 0 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return row_begin( index ) + zen.col();
        }
        const_row_type row_cbegin( const size_type index = 0 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return const_row_type( zen.begin() + index * zen.col() );
        }
        const_row_type row_cend( const size_type index = 0 ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return row_begin( index ) + zen.col();
        }
        reverse_row_type row_rbegin( const size_type index = 0 ) noexcept
        {
            return reverse_row_type( row_end( index ) );
        }
        reverse_row_type row_rend( const size_type index = 0 ) noexcept
        {
            return reverse_row_type( row_begin( index ) );
        }
        const_reverse_row_type row_rbegin( const size_type index = 0 ) const noexcept
        {
            return const_reverse_row_type( row_end( index ) );
        }
        const_reverse_row_type row_rend( const size_type index = 0 ) const noexcept
        {
            return const_reverse_row_type( row_begin( index ) );
        }
        const_reverse_row_type row_crbegin( const size_type index = 0 ) const noexcept
        {
            return const_reverse_row_type( row_end( index ) );
        }
        const_reverse_row_type row_crend( const size_type index = 0 ) const noexcept
        {
            return const_reverse_row_type( row_begin( index ) );
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_save_as_txt
    {
        typedef Matrix zen_type;
        typedef Type value_type;
        bool save_as_txt( std::string const& file_name ) const noexcept
        {
            return save_as_txt( file_name.c_str() );
        }
        bool save_as_txt( const char* const file_name ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            std::ofstream ofs( file_name );

            if ( !ofs )
            {
                std::cerr << "Error: failed write matrix to txt file - " << file_name << "\n";
                return false;
            }

            ofs.precision( 16 );
            ofs << zen;
            ofs.close();
            return true;
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_save_as_binary
    {
        typedef Matrix zen_type;
        typedef Type value_type;
        bool save_as_binary( std::string const& file_name ) const
        {
            return save_as_binary( file_name.c_str() );
        }
        bool save_as_binary( char const* const file_name ) const
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            std::ofstream ofs( file_name, std::ios::out | std::ios::binary );

            if ( !ofs )
            {
                std::cerr << "Error: failed write matrix to binary file - " << file_name << "!\n";
                return false;
            }

            auto const r = zen.row();
            ofs.write( reinterpret_cast< char const* >( std::addressof( r ) ), sizeof( r ) );
            auto const c = zen.col();
            ofs.write( reinterpret_cast< char const* >( std::addressof( c ) ), sizeof( c ) );
            ofs.write( reinterpret_cast< char const* >( zen.data() ), sizeof( Type ) * zen.size() );

            if ( !ofs.good() )
            {
                std::cerr << "Error: failed write matrix to binary file - " << file_name << " - something bad happened!\n";
                return false;
            }

            ofs.close();
            return true;
        }
    };
    namespace crtp_save_as_bmp_private
    {
        typedef std::function< double( double ) > converter_type;
        typedef std::function< converter_type( double, double ) > transform_value_type;
        static const std::map< std::string, transform_value_type > transforms
        {
            std::make_pair( std::string{ "default" }, transform_value_type{
                []( double mx, double mn )
                {
                    return converter_type
                    {
                        [ = ]( double v )
                        {
                            return ( v - mn ) / ( mx - mn ) + 1.0e-10;
                        }
                    };
                } } ),
            std::make_pair( std::string{ "log" }, transform_value_type{ []( double mx, double mn )
            {
                return converter_type{ [ = ]( double v )
                {
                    return std::log( 1.0 + ( v - mn ) / ( mx - mn ) + 1.0e-10 );
                } };
            } } ),
            std::make_pair( std::string{ "log1" }, transform_value_type{ []( double mx, double mn )
            {
                return converter_type{ [ = ]( double v )
                {
                    return std::log( 1.0 + ( v - mn ) / ( mx - mn ) * 10.0 + 1.0e-10 );
                } };
            } } ),
            std::make_pair( std::string{ "logpi" }, transform_value_type{ []( double mx, double mn )
            {
                return converter_type{ [ = ]( double v )
                {
                    return std::log( 1.0 + ( v - mn ) / ( mx - mn ) * 3.14159265358979323846 + 1.0e-10 );
                } };
            } } ),
            std::make_pair( std::string{ "logx" }, transform_value_type{ []( double mx, double mn )
            {
                return converter_type{ [ = ]( double v )
                {
                    return std::log( 1.0 + ( v - mn ) / ( mx - mn ) * 1.64872127070012814685 + 1.0e-10 );
                } };
            } } ),
            std::make_pair( std::string{ "log_1" }, transform_value_type{ []( double mx, double mn )
            {
                return converter_type{ [ = ]( double v )
                {
                    return std::log( 1.0 + ( v - mn ) / ( mx - mn ) / 10.0 + 1.0e-10 );
                } };
            } } ),
            std::make_pair( std::string{ "log2" }, transform_value_type{ []( double mx, double mn )
            {
                return converter_type{ [ = ]( double v )
                {
                    return std::log( 1.0 + ( v - mn ) / ( mx - mn ) * 100.0 + 1.0e-10 );
                } };
            } } ),
            std::make_pair( std::string{ "log_2" }, transform_value_type{ []( double mx, double mn )
            {
                return converter_type{ [ = ]( double v )
                {
                    return std::log( 1.0 + ( v - mn ) / ( mx - mn ) / 100.0 + 1.0e-10 );
                } };
            } } ),
            std::make_pair( std::string{ "log3" }, transform_value_type{ []( double mx, double mn )
            {
                return converter_type{ [ = ]( double v )
                {
                    return std::log( 1.0 + ( v - mn ) / ( mx - mn ) * 1000.0 + 1.0e-10 );
                } };
            } } ),
            std::make_pair( std::string{ "log_3" }, transform_value_type{ []( double mx, double mn )
            {
                return converter_type{ [ = ]( double v )
                {
                    return std::log( 1.0 + ( v - mn ) / ( mx - mn ) / 1000.0 + 1.0e-10 );
                } };
            } } ),
            std::make_pair( std::string{ "log4" }, transform_value_type{ []( double mx, double mn )
            {
                return converter_type{ [ = ]( double v )
                {
                    return std::log( 1.0 + ( v - mn ) / ( mx - mn ) * 10000.0 + 1.0e-10 );
                } };
            } } ),
            std::make_pair( std::string{ "log_4" }, transform_value_type{ []( double mx, double mn )
            {
                return converter_type{ [ = ]( double v )
                {
                    return std::log( 1.0 + ( v - mn ) / ( mx - mn ) / 10000.0 + 1.0e-10 );
                } };
            } } )
        };
        auto&& make_array = []( unsigned char a, unsigned char b, unsigned char c )
        {
            std::array< unsigned char, 3 > ans;
            ans[0] = a;
            ans[1] = b;
            ans[2] = c;
            return ans;
        };
        typedef std::function< std::array< unsigned char, 3 >( double ) > color_value_type;
        static const std::map< std::string, color_value_type > color_maps
        {
            std::make_pair(
                std::string{ "default" },
                color_value_type{
                []( double x )
                {
                    typedef unsigned char type;
                    auto&& ch = []( double x )
                    {
                        return static_cast< type >( static_cast< int >( x * 766.0 ) );
                    };

                    if ( 3.0 * x < 1.0 )
                        return make_array( type{ 0 }, type{ 0 }, type{ ch( x ) } );

                    if ( 3.0 * x < 2.0 )
                        return make_array( type{ 0 }, type{ ch( x - 1.0 / 3.0 ) }, type{ 255 } );
                    return make_array( type{ ch( x - 2.0 / 3.0 ) }, type{ 255 }, type{ 255 } );
                }
                            }
                        ),
            std::make_pair(
                std::string{ "parula" },
                color_value_type{
                []( double x )
                {
                    typedef unsigned char type;
                    auto const& r = [](double v)
                    {
                        if (v*3.0<1.0)
                        {
                            double const ratio = 1.0 - v*3.0;
                            return static_cast< type >( static_cast< int >(7 + 47.0*ratio) );
                        }
                        double const ratio = (3.0*v - 1.0) / 2.0;
                        return static_cast< type >( static_cast< int >(7+242*ratio));
                    };
                    auto const& g = [](double v)
                    {
                        return static_cast< type >( static_cast< int >(42+210.0*v));
                    };
                    auto const& b = [](double v)
                    {
                        if (v*9.0<1.0)
                        {
                            double const ratio = v*9.0;
                            return static_cast< type >( static_cast< int >(135+87*ratio));
                        }
                        double const ratio = (9.0 - 9.0*v) / 8.0;
                        return static_cast< type >( static_cast< int >(14*208*ratio));
                    };

                    return make_array( type{r(x)}, type{g(x)}, type{b(x)} );
                }
                            }
                        ),
            std::make_pair(
                std::string{ "hotblue" },
            color_value_type{
                []( double x )
                {
                    typedef unsigned char type;
                    auto&& ch = []( double x )
                    {
                        return static_cast< type >( static_cast< int >( x * 766.0 ) );
                    };

                    if ( 3.0 * x < 1.0 )
                        return make_array( type{ ch( 1.0 / 3.0 - x ) }, type{ 255 }, type{ 255 } );

                    if ( 3.0 * x < 2.0 )
                        return make_array( type{ 0 }, type{ ch( 2.0 / 3.0 - x ) }, type{ 255 } );
                    return make_array( type{ 0 }, type{ 0 }, type{ ch( 1.0 - x ) } );
                } } ),
            std::make_pair(
                std::string{ "jet" },
            color_value_type{
                []( double x )
                {
                    typedef unsigned char type;
                    auto&& ch = []( double x )
                    {
                        return static_cast< type >( static_cast< int >( x * 766.0 ) );
                    };

                    if ( 3.0 * x < 1.0 )
                        return make_array( type{ 0 }, type{ ch( x ) }, type{ 255 } );

                    if ( 3.0 * x < 2.0 )
                        return make_array( type{ ch( x - 1.0 / 3.0 ) }, type{ 255 }, type{ ch( 2.0 / 3.0 - x ) } );
                    return make_array( type{ 255 }, type{ ch( 1.0 - x ) }, type{ 0 } );
                } } ),
            std::make_pair(
                std::string{ "obscure" },
            color_value_type{
                []( double x )
                {
                    typedef unsigned char type;
                    auto&& ch = []( double x )
                    {
                        return static_cast< type >( static_cast< int >( x * 256.0 ) );
                    };
                    type const b = ch( 1.0 - x );

                    if ( 4.0 * x < 1 )
                        return make_array( ch( 1.0 - 4.0 * x ), ch( 1.0 - 4.0 * x ), b );

                    type const r = ch( ( x - 0.25 ) * 4.0 / 3.0 );

                    if ( 2.0 * x < 1 )
                        return make_array( r, ch( ( x - 0.25 ) * 4.0 ), b );

                    return make_array( r, ch( ( 1.0 - x ) * 2.0 ), b );
                } } ),
            std::make_pair(
                std::string{ "gray" },
            color_value_type{
                []( double x )
                {
                    typedef unsigned char type;
                    auto&& ch = []( double x )
                    {
                        return static_cast< type >( static_cast< int >( x * 256.0 ) );
                    };
                    unsigned char val = ch( x );
                    return make_array( val, val, val );
                } } )
        };
    }

    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_save_as_bmp
    {
        typedef Matrix zen_type;
        bool save_as_bmp( const std::string& file_name, std::string const& color_map = std::string{ "parula" }, std::string const& transform = std::string{ "default" } ) const
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            assert( zen.row() && "save_as_bmp: matrix row cannot be zero" );
            assert( zen.col() && "save_as_bmp: matrix column cannot be zero" );
            std::string new_file_name{ file_name };
            std::string const extension{ ".bmp" };

            if ( ( new_file_name.size() < 4 ) || ( std::string{ new_file_name.begin() + new_file_name.size() - 4, new_file_name.end() } != extension ) )
                new_file_name += extension;
            std::ofstream stream( new_file_name.c_str(), std::ios_base::out | std::ios_base::binary );

            if ( !stream )
            {
                return false;
            }

            using namespace crtp_save_as_bmp_private;
            std::string const& map_name       = ( color_maps.find( color_map ) == color_maps.end() ) ? std::string{ "default" } : color_map;
            std::string const& transform_name = ( transforms.find( transform ) == transforms.end() ) ? std::string{ "default" } : transform;
            auto&& selected_map               = ( *( color_maps.find( map_name ) ) ).second;
            auto&& selected_transform         = ( *( transforms.find( transform_name ) ) ).second;
            unsigned char file[14]            = { 'B', 'M', 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0 };
            unsigned char info[40]            = { 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 24, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x13, 0x0B, 0, 0, 0x13, 0x0B, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, };
            unsigned long const the_col   = zen.col();
            unsigned long const the_row   = zen.row();
            unsigned long const pad_size  = ( 4 - ( ( the_col * 3 ) & 0x3 ) ) & 0x3;
            unsigned long const data_size = the_col * the_row * 3 + the_row * pad_size;
            unsigned long const all_size  = data_size + sizeof( file ) + sizeof( info );
            auto const& ul_to_uc          = []( unsigned long val ) { return static_cast< unsigned char >( val & 0xffUL ); };
            file[2]  = ul_to_uc( all_size );
            file[3]  = ul_to_uc( all_size >> 8 );
            file[4]  = ul_to_uc( all_size >> 16 );
            file[5]  = ul_to_uc( all_size >> 24 );
            info[4]  = ul_to_uc( the_col );
            info[5]  = ul_to_uc( the_col >> 8 );
            info[6]  = ul_to_uc( the_col >> 16 );
            info[7]  = ul_to_uc( the_col >> 24 );
            info[8]  = ul_to_uc( the_row );
            info[9]  = ul_to_uc( the_row >> 8 );
            info[10] = ul_to_uc( the_row >> 16 );
            info[11] = ul_to_uc( the_row >> 24 );
            info[20] = ul_to_uc( data_size );
            info[21] = ul_to_uc( data_size >> 8 );
            info[22] = ul_to_uc( data_size >> 16 );
            info[23] = ul_to_uc( data_size >> 24 );
            stream.write( reinterpret_cast< char* >( file ), sizeof( file ) );
            stream.write( reinterpret_cast< char* >( info ), sizeof( info ) );
            unsigned char pad[3] = { 0, 0, 0 };
            unsigned char pixel[3];
            double max_val = static_cast< double >( *std::max_element( zen.begin(), zen.end() ) );
            double min_val = static_cast< double >( *std::min_element( zen.begin(), zen.end() ) );

            if ( max_val - min_val < 1.0e-10 )
            {
                max_val += 1.0e-10;
                min_val -= 1.0e-10;
            }

            for ( unsigned long r = 0; r < the_row; r++ )
            {
                for ( unsigned long c = 0; c < the_col; c++ )
                {
                    auto const r_ = the_row - r - 1;
                    auto const c_ = c;
                    auto const& rgb = selected_map( selected_transform( max_val, min_val )( zen[r_][c_] ) );
                    pixel[2]        = rgb[0];
                    pixel[1]        = rgb[1];
                    pixel[0]        = rgb[2];
                    stream.write( reinterpret_cast< char* >( pixel ), 3 );
                }

                stream.write( reinterpret_cast< char* >( pad ), pad_size );
            }

            stream.close();
            return true;
        }
        bool save_as_bmp( const char* const file_name ) const
        {
            return save_as_bmp( std::string{ file_name } );
        }
    };

    namespace crtp_save_as_pgm_private
    {
        typedef std::function< double( double ) > converter_type;
        typedef std::function< converter_type( double, double ) > transform_value_type;
        static const std::map< std::string, transform_value_type > transforms
        {
            std::make_pair( std::string{ "default" }, transform_value_type{
                []( double mx, double mn )
                {
                    return converter_type
                    {
                        [ = ]( double v )
                        {
                            return ( v - mn ) / ( mx - mn ) + 1.0e-10;
                        }
                    };
                } } ),
            std::make_pair( std::string{ "log" }, transform_value_type{ []( double mx, double mn )
            {
                return converter_type{ [ = ]( double v )
                {
                    return std::log( 1.0 + ( v - mn ) / ( mx - mn ) + 1.0e-10 );
                } };
            } } ),
            std::make_pair( std::string{ "log1" }, transform_value_type{ []( double mx, double mn )
            {
                return converter_type{ [ = ]( double v )
                {
                    return std::log( 1.0 + ( v - mn ) / ( mx - mn ) * 10.0 + 1.0e-10 );
                } };
            } } ),
            std::make_pair( std::string{ "logpi" }, transform_value_type{ []( double mx, double mn )
            {
                return converter_type{ [ = ]( double v )
                {
                    return std::log( 1.0 + ( v - mn ) / ( mx - mn ) * 3.14159265358979323846 + 1.0e-10 );
                } };
            } } ),
            std::make_pair( std::string{ "logx" }, transform_value_type{ []( double mx, double mn )
            {
                return converter_type{ [ = ]( double v )
                {
                    return std::log( 1.0 + ( v - mn ) / ( mx - mn ) * 1.64872127070012814685 + 1.0e-10 );
                } };
            } } ),
            std::make_pair( std::string{ "log_1" }, transform_value_type{ []( double mx, double mn )
            {
                return converter_type{ [ = ]( double v )
                {
                    return std::log( 1.0 + ( v - mn ) / ( mx - mn ) / 10.0 + 1.0e-10 );
                } };
            } } ),
            std::make_pair( std::string{ "log2" }, transform_value_type{ []( double mx, double mn )
            {
                return converter_type{ [ = ]( double v )
                {
                    return std::log( 1.0 + ( v - mn ) / ( mx - mn ) * 100.0 + 1.0e-10 );
                } };
            } } ),
            std::make_pair( std::string{ "log_2" }, transform_value_type{ []( double mx, double mn )
            {
                return converter_type{ [ = ]( double v )
                {
                    return std::log( 1.0 + ( v - mn ) / ( mx - mn ) / 100.0 + 1.0e-10 );
                } };
            } } ),
            std::make_pair( std::string{ "log3" }, transform_value_type{ []( double mx, double mn )
            {
                return converter_type{ [ = ]( double v )
                {
                    return std::log( 1.0 + ( v - mn ) / ( mx - mn ) * 1000.0 + 1.0e-10 );
                } };
            } } ),
            std::make_pair( std::string{ "log_3" }, transform_value_type{ []( double mx, double mn )
            {
                return converter_type{ [ = ]( double v )
                {
                    return std::log( 1.0 + ( v - mn ) / ( mx - mn ) / 1000.0 + 1.0e-10 );
                } };
            } } ),
            std::make_pair( std::string{ "log4" }, transform_value_type{ []( double mx, double mn )
            {
                return converter_type{ [ = ]( double v )
                {
                    return std::log( 1.0 + ( v - mn ) / ( mx - mn ) * 10000.0 + 1.0e-10 );
                } };
            } } ),
            std::make_pair( std::string{ "log_4" }, transform_value_type{ []( double mx, double mn )
            {
                return converter_type{ [ = ]( double v )
                {
                    return std::log( 1.0 + ( v - mn ) / ( mx - mn ) / 10000.0 + 1.0e-10 );
                } };
            } } )
        };
    }
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_save_as_pgm
    {
        typedef Matrix zen_type;
        bool save_as_pgm( const std::string& file_name, std::string const& transform = std::string{ "default" } ) const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            std::string new_file_name{ file_name };
            std::string const extension{ ".pgm" };

            if ( ( new_file_name.size() < 4 ) || ( std::string{ new_file_name.begin() + new_file_name.size() - 4, new_file_name.end() } != extension ) )
                new_file_name += extension;
            std::ofstream stream( new_file_name.c_str() );

            if ( !stream )
            {
                std::cerr << "Error: failed save matrix to pgm - " << file_name << "!\n";
                return false;
            }

            using namespace crtp_save_as_pgm_private;
            std::string const& transform_name = ( transforms.find( transform ) == transforms.end() ) ? std::string{ "default" } :
                                                transform;
            {
                stream << "P2\n";
                stream << zen.col() << " " << zen.row() << "\n";
                stream << "255\n";
                stream << "//Generated Portable GrayMap image for path [" << file_name << "]";
                stream << "\n//and colormap [" << transform_name << "]\n";
            }
            auto const& selected_map = []( double x )
            {
                assert( x >= 0.0 && "Negative x passed!" );
                assert( x <= 1.0 && "X exceeds boundary!" );
                typedef std::uint16_t type;
                auto const& ch = []( double x )
                {
                    return static_cast< type >( static_cast< int >( x * 256.0 ) );
                };
                return ch( x );
            };
            auto&& selected_transform = ( *( transforms.find( transform_name ) ) ).second;
            double const max_val      = static_cast< double >( *std::max_element( zen.begin(), zen.end() ) );
            double const min_val      = static_cast< double >( *std::min_element( zen.begin(), zen.end() ) );
            double const mmax         = selected_transform( max_val, min_val )( max_val );
            double const mmin         = selected_transform( max_val, min_val )( min_val );
            double const divider      = mmax - mmin;

            for ( unsigned long r = 0; r < zen.row(); r++ )
            {
                for ( unsigned long c = 0; c < zen.col(); c++ )
                {
                    unsigned long const r_ = zen.row() - r - 1;
                    auto rgb = selected_map( ( selected_transform( max_val, min_val )( zen[zen.row() - 1 - r_][c] ) - mmin ) / divider );
                    //auto rgb = selected_map( ( selected_transform( max_val, min_val )( zen[zen.row() - 1 - r][c] ) - mmin ) / divider );

                    if ( rgb > 255.0 )
                        rgb = 255.0;

                    if ( rgb < 0.0 )
                        rgb = 0.0;

                    stream << rgb << " ";
                }

                stream << "\n";
            }

            stream.close();
            return true;
        }
        bool save_as_pgm( const char* const file_name ) const
        {
            return save_as_pgm( std::string{ file_name } );
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_shrink_to_size
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::value_type value_type;
        zen_type& shrink_to_size( const size_type new_row, const size_type new_col ) noexcept
        {
            assert( new_row && new_col );
            zen_type& zen = static_cast< zen_type& >( *this );

            if ( new_row == zen.row() && new_col == zen.col() )
                return zen;

            zen_type other{ zen.get_allocator(), new_row, new_col };
            std::fill( other.begin(), other.end(), value_type{} );
            size_type const the_rows_to_copy = std::min( zen.row(), new_row );
            size_type const the_cols_to_copy = std::min( zen.col(), new_col );

            for ( size_type r = 0; r != the_rows_to_copy; ++r )
                std::copy( zen.row_begin( r ), zen.row_begin( r ) + the_rows_to_copy, other.row_begin( r ) );

            zen.swap( other );
            return zen;
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_stream_operator
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        typedef typename type_proxy_type::value_type value_type;
        friend std::ostream& operator<<( std::ostream& lhs, zen_type const& rhs )
        {
            lhs.precision( 18 );

            for ( size_type i = 0; i < rhs.row(); ++i )
            {
                std::copy( rhs.row_begin( i ), rhs.row_end( i ), std::ostream_iterator< value_type >( lhs, "\t" ) );
                lhs << "\n";
            }

            return lhs;
        }
        friend std::istream& operator>>( std::istream& is, zen_type& rhs )
        {
            std::vector< std::string > row_element;
            std::string string_line;

            while ( std::getline( is, string_line, '\n' ) )
                row_element.push_back( string_line );

            size_type const row = row_element.size();
            size_type const col = std::count_if( row_element[0].begin(), row_element[0].end(), []( char ch ) { return '\t' == ch; } );

            if ( row == 0 || col == 0 )
            {
                is.setstate( std::ios::failbit );
                return is;
            }

            rhs.resize( row, col );

            for ( size_type r = 0; r != row; ++r )
            {
                std::istringstream the_row( row_element[r] );
                std::copy( std::istream_iterator< value_type >( the_row ), std::istream_iterator< value_type >(), rhs.row_begin( r ) );
            }

            return is;
        }
    };
    template < typename Matrix, typename T, typename A >
    struct crtp_swap
    {
        typedef Matrix zen_type;
        void swap( zen_type& other ) noexcept
        {
            zen_type& zen = static_cast< zen_type& >( *this );
            std::swap( zen.row_, other.row_ );
            std::swap( zen.col_, other.col_ );
            std::swap( zen.dat_, other.dat_ );
            std::swap( zen.allocator_, other.allocator_ );
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_tr
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::value_type value_type;
        const value_type tr() const noexcept
        {
            zen_type const& zen = static_cast< zen_type const& >( *this );
            return std::accumulate( zen.diag_begin(), zen.diag_end(), value_type() );
        }
    };
    template < typename Matrix, typename Type, typename Allocator >
    struct crtp_transpose
    {
        typedef Matrix zen_type;
        typedef crtp_typedef< Type, Allocator > type_proxy_type;
        typedef typename type_proxy_type::size_type size_type;
        const zen_type transpose() const noexcept
        {
            const zen_type& zen = static_cast< zen_type const& >( *this );
            zen_type ans( zen.get_allocator(), zen.col(), zen.row() );

            for ( size_type i = 0; i < zen.col(); ++i )
                std::copy( zen.col_begin( i ), zen.col_end( i ), ans.row_begin( i ) );

            return ans;
        }
    };

    template < typename Type, class Allocator = std::allocator<Type> >
    struct matrix : crtp_anti_diag_iterator< matrix< Type, Allocator >, Type, Allocator >
        , crtp_apply< matrix< Type, Allocator >, Type, Allocator >
        , crtp_bracket_operator< matrix< Type, Allocator >, Type, Allocator >
        , crtp_clear< matrix< Type, Allocator >, Type, Allocator >
        , crtp_clone< matrix< Type, Allocator >, Type, Allocator >
        , crtp_col_iterator< matrix< Type, Allocator >, Type, Allocator >
        , crtp_copy< matrix< Type, Allocator >, Type, Allocator >
        , crtp_data< matrix< Type, Allocator >, Type, Allocator >
        , crtp_det< matrix< Type, Allocator >, Type, Allocator >
        , crtp_diag_iterator< matrix< Type, Allocator >, Type, Allocator >
        , crtp_direct_iterator< matrix< Type, Allocator >, Type, Allocator >
        , crtp_divide_equal_operator< matrix< Type, Allocator >, Type, Allocator >
        , crtp_get_allocator< matrix< Type, Allocator >, Type, Allocator >
        , crtp_inverse< matrix< Type, Allocator >, Type, Allocator >
        , crtp_load_binary< matrix< Type, Allocator >, Type, Allocator >
        , crtp_load_txt< matrix< Type, Allocator >, Type, Allocator >
        , crtp_minus_equal_operator< matrix< Type, Allocator >, Type, Allocator >
        , crtp_multiply_equal_operator< matrix< Type, Allocator >, Type, Allocator >
        , crtp_plus_equal_operator< matrix< Type, Allocator >, Type, Allocator >
        , crtp_prefix_minus< matrix< Type, Allocator >, Type, Allocator >
        , crtp_prefix_plus< matrix< Type, Allocator >, Type, Allocator >
        , crtp_reshape< matrix< Type, Allocator >, Type, Allocator >
        , crtp_resize< matrix< Type, Allocator >, Type, Allocator >
        , crtp_row_col_size< matrix< Type, Allocator >, Type, Allocator >
        , crtp_row_iterator< matrix< Type, Allocator >, Type, Allocator >
        , crtp_save_as_binary< matrix< Type, Allocator >, Type, Allocator >
        , crtp_save_as_bmp< matrix< Type, Allocator >, Type, Allocator >
        , crtp_save_as_pgm< matrix< Type, Allocator >, Type, Allocator >
        , crtp_save_as_txt< matrix< Type, Allocator >, Type, Allocator >
        , crtp_shape< matrix< Type, Allocator >, Type, Allocator >
        , crtp_shrink_to_size< matrix< Type, Allocator >, Type, Allocator >
        , crtp_stream_operator< matrix< Type, Allocator >, Type, Allocator >
        , crtp_swap< matrix< Type, Allocator >, Type, Allocator >
        , crtp_transpose< matrix< Type, Allocator >, Type, Allocator >
    {
        typedef matrix self_type;
        typedef crtp_typedef< Type, Allocator >                         type_proxy_type;
        typedef typename type_proxy_type::value_type                    value_type;
        typedef typename type_proxy_type::size_type                     size_type;
        typedef typename type_proxy_type::difference_type               difference_type;
        typedef typename type_proxy_type::pointer                       pointer;
        typedef typename type_proxy_type::allocator_type                allocator_type;
        typedef typename type_proxy_type::range_type                    range_type;

        size_type                                                       row_;
        size_type                                                       col_;
        pointer                                                         dat_;
        allocator_type                                                  allocator_;

        ~matrix()
        {
            (*this).clear();
        }

        matrix( self_type&& other ) noexcept : matrix{}
        {
            (*this).swap( other );
        }
        self_type& operator = ( self_type&& other )
        {
            (*this).clear();
            (*this).swap( other );
            return *this;
        }

        matrix( const self_type& other ) noexcept : row_{ other.row_ }, col_{ other.col_ }, dat_{ nullptr }, allocator_{ other.allocator_ }
        {
            if ( row_*col_ != 0 )
            {
                dat_ = allocator_.allocate( row_*col_ );
                std::copy_n( other.dat_, row_*col_, dat_ );
            }
        }

        template < typename T, typename A >
        matrix( matrix< T, A> const& other ) noexcept : row_{ other.row_ }, col_{ other.col_ }, dat_{ nullptr }, allocator_{ other.allocator_ }
        {
            if ( row_*col_ != 0 )
            {
                dat_ = allocator_.allocate( row_*col_ );
                std::copy_n( other.dat_, row_*col_, dat_ );
            }
        }

        explicit matrix( const size_type r = 0, const size_type c = 0, value_type const& v = value_type{} ) noexcept : row_{ r } , col_{ c }, dat_{ nullptr }
        {
            if ( r*c != 0 )
            {
                dat_ = allocator_.allocate( r*c );
                std::fill_n( dat_, r*c, v );
            }
        }

        explicit matrix( allocator_type a, const size_type r = 0, const size_type c = 0 ) noexcept : row_{ r } , col_{ c }, dat_{ nullptr }, allocator_{ a }
        {
            if ( r*c != 0 )
                dat_ = allocator_.allocate( r*c );
        }

        template< typename T, typename A >
        matrix( matrix<T,A> const& other, std::initializer_list<size_type> rr, std::initializer_list<size_type> rc ) noexcept : row_{0}, col_{0}, dat_{nullptr}, allocator_{ other.get_allocator() }
        {
            assert( rr.size() == 2 && "row dims not match!" );
            assert( rc.size() == 2 && "col dims not match!" );
            auto [rr0, rr1] = std::make_pair( *(rr.begin()), *(rr.begin()+1) );
            auto [rc0, rc1] = std::make_pair( *(rc.begin()), *(rc.begin()+1) );
            (*this).clone( other, rr0, rr1, rc0, rc1 );
        }

        matrix( matrix const& other, std::initializer_list<size_type> rr, std::initializer_list<size_type> rc ) noexcept : row_{0}, col_{0}, dat_{nullptr}, allocator_{ other.get_allocator() }
        {
            assert( rr.size() == 2 && "row dims not match!" );
            assert( rc.size() == 2 && "col dims not match!" );
            auto [rr0, rr1] = std::make_pair( *(rr.begin()), *(rr.begin()+1) );
            auto [rc0, rc1] = std::make_pair( *(rc.begin()), *(rc.begin()+1) );
            (*this).clone( other, rr0, rr1, rc0, rc1 );
        }

#if 0
        template < typename T, typename A >
        matrix( const matrix< T, A >& other, const range_type& rr, const range_type& rc ) noexcept : row_{0}, col_{0}, dat_{nullptr}, allocator_{ other.get_allocator() }
        {
            ( *this ).clone( other, rr.first, rr.second, rc.first, rc.second );
        }
        matrix( matrix const& other, range_type const& rr, range_type const& rc ) noexcept : row_{0}, col_{0}, dat_{nullptr}, allocator_{ other.get_allocator() }
        {
            ( *this ).clone( other, rr.first, rr.second, rc.first, rc.second );
        }
        template < typename T, typename A >
        matrix( const matrix< T, A >& other, size_type r0, size_type r1, size_type c0, size_type c1 ) noexcept : row_{0}, col_{0}, dat_{nullptr}, allocator_{ other.get_allocator() }
        {
            ( *this ).clone( other, r0, r1, c0, c1 );
        }
#else
        template < typename T, typename A >
        matrix( const matrix< T, A >& other, const range_type& rr, const range_type& rc ) noexcept : matrix{ other, {rr.first, rr.second}, {rc.first, rc.second} }
        {}
        matrix( matrix const& other, range_type const& rr, range_type const& rc ) noexcept : matrix{ other, {rr.first, rr.second}, {rc.first, rc.second} }
        {}
        template < typename T, typename A >
        matrix( const matrix< T, A >& other, size_type r0, size_type r1, size_type c0, size_type c1 ) noexcept : matrix{ other, {r0, r1}, {c0, c1} }
        {}
        matrix( self_type const& other, size_type r0, size_type r1, size_type c0, size_type c1 ) noexcept : matrix{ other, {r0, r1}, {c0, c1} }
        {}

#endif
        self_type& operator = ( const self_type& rhs ) noexcept
        {
            ( *this ).copy( rhs );
            return *this;
        }
        template < typename T, typename A >
        self_type& operator = ( const matrix< T, A >& rhs ) noexcept
        {
            ( *this ).copy( rhs );
            return *this;
        }
        //self_type& operator =( self_type&& ) = default;
        self_type& operator = ( const value_type& v )
        {
            std::fill( ( *this ).diag_begin(), ( *this ).diag_end(), v ); //TODO:should move to crtp_xxx
            return *this;
        }
    };//struct matrix
    template < typename T1, typename A1, typename T2, typename A2 >
    const matrix< T1, A1 >
    operator+( const matrix< T1, A1 >& lhs, const matrix< T2, A2 >& rhs )
    {
        matrix< T1, A1 > ans( lhs );
        ans += rhs;
        return ans;
    }
    template < typename T1, typename A1, typename T2, typename A2 >
    const matrix< T1, A1 >
    operator-( const matrix< T1, A1 >& lhs, const matrix< T2, A2 >& rhs )
    {
        matrix< T1, A1 > ans( lhs );
        ans -= rhs;
        return ans;
    }
    template < typename T1, typename A1, typename T2, typename A2 >
    const matrix< T1, A1 >
    operator*( const matrix< T1, A1 >& lhs, const matrix< T2, A2 >& rhs )
    {
        matrix< T1, A1 > ans( lhs );
        ans *= rhs;
        return ans;
    }
    template < typename T1, typename A1, typename T2, typename A2 >
    const matrix< T1, A1 >
    operator/( const matrix< T1, A1 >& lhs, const matrix< T2, A2 >& rhs )
    {
        matrix< T1, A1 > ans( lhs );
        ans /= rhs;
        return ans;
    }
    template < typename T1, typename A1, typename T2, typename A2 >
    bool operator<( const matrix< T1, A1 >& lhs, const matrix< T2, A2 >& rhs )
    {
        assert( lhs.row() == rhs.row() );
        assert( lhs.col() == rhs.col() );
        return std::lexicographical_compare( lhs.begin(), lhs.end(), rhs.begin(), rhs.end() );
    }
    template < typename T1, typename A1, typename T2, typename A2 >
    bool operator==( const matrix< T1, A1 >& lhs, const matrix< T2, A2 >& rhs )
    {
        assert( lhs.row() == rhs.row() );
        assert( lhs.col() == rhs.col() );
        return std::equal( lhs.begin(), lhs.end(), rhs.begin() );
    }
    template < typename T1, typename A1, typename T2, typename A2 >
    bool operator>( const matrix< T1, A1 >& lhs, const matrix< T2, A2 >& rhs )
    {
        return !( ( lhs < rhs ) || ( lhs == rhs ) );
    }
    template < typename T1, typename A1, typename T2, typename A2 >
    bool operator>=( const matrix< T1, A1 >& lhs, const matrix< T2, A2 >& rhs )
    {
        return !( lhs < rhs );
    }
    template < typename T1, typename A1, typename T2, typename A2 >
    bool operator<=( const matrix< T1, A1 >& lhs, const matrix< T2, A2 >& rhs )
    {
        return !( lhs > rhs );
    }
    template < typename T1, typename A1, typename T2, typename A2 >
    const matrix< T1, A1 >
    operator||( const matrix< T1, A1 >& lhs, const matrix< T2, A2 >& rhs )
    {
        if ( lhs.row() == 0 )
            return rhs;

        if ( rhs.row() == 0 )
            return lhs;

        assert( lhs.row() == rhs.row() );
        typedef matrix< T1, A1 > matrix_type;
        typedef typename matrix_type ::size_type size_type;
        const size_type row = lhs.row();
        const size_type col = lhs.col() + rhs.col();
        matrix_type ans( row, col );

        for ( size_type i = 0; i < row; ++i )
        {
            std::copy( lhs.row_begin( i ), lhs.row_end( i ), ans.row_begin( i ) );
            std::copy( rhs.row_begin( i ), rhs.row_end( i ), ans.row_begin( i ) + lhs.col() );
        }

        return ans;
    }
    template < typename T1, typename A1, typename T2, typename A2 >
    const matrix< T1, A1 >
    operator&&( const matrix< T1, A1 >& lhs, const matrix< T2, A2 >& rhs )
    {
        if ( lhs.col() == 0 )
            return rhs;

        if ( rhs.col() == 0 )
            return lhs;

        assert( lhs.col() == rhs.col() );
        typedef matrix< T1, A1 > matrix_type;
        typedef typename matrix_type ::size_type size_type;
        const size_type row = lhs.row() + rhs.row();
        const size_type col = lhs.col();
        matrix_type ans( row, col );

        for ( size_type i = 0; i < col; ++i )
        {
            std::copy( lhs.col_begin( i ), lhs.col_end( i ), ans.col_begin( i ) );
            std::copy( rhs.col_begin( i ), rhs.col_end( i ), ans.col_begin( i ) + lhs.row() );
        }

        return ans;
    }
    template < typename T, typename A, typename T_ >
    const matrix< T, A >
    operator*( const matrix< T, A >& lhs, const T_* const rhs )
    {
        matrix< T, A > ans( lhs.row(), 1 );

        for ( std::size_t i = 0; i < lhs.row(); ++i )
            ans[i][0] = std::inner_product( lhs.row_begin( i ), lhs.row_end( i ), rhs, T() );

        return ans;
    }
    template < typename T, typename A, typename T_ >
    const matrix< T, A >
    operator*( const T_* lhs, const matrix< T, A >& rhs )
    {
        matrix< T, A > ans( 1, rhs.col() );

        for ( std::size_t i = 0; i < rhs.col(); ++i )
            ans[0][i] = std::inner_product( lhs, lhs + rhs.row(), rhs.col_begin( i ), T() );

        return ans;
    }
    template < typename T, typename A, typename T_ >
    const matrix< T, A >
    operator*( const matrix< T, A >& lhs, const std::valarray< T_ >& rhs )
    {
        assert( lhs.col() == rhs.size() );
        matrix< T, A > ans( lhs.row(), 1 );

        for ( std::size_t i = 0; i < lhs.row(); ++i )
            ans[i][0] = std::inner_product( lhs.row_begin( i ), lhs.row_end( i ), std::begin( rhs ), T() );

        return ans;
    }
    template < typename T, typename A, typename T_ >
    const matrix< T, A >
    operator*( const std::valarray< T_ >& lhs, const matrix< T, A >& rhs )
    {
        assert( rhs.row() == lhs.size() );
        matrix< T, A > ans( 1, lhs.row() );

        for ( std::size_t i = 0; i < rhs.col(); ++i )
            ans[0][i] = std::inner_product( std::begin( lhs ), std::begin( lhs ) + rhs.row(), rhs.col_begin( i ), T() );

        return ans;
    }
    template < typename T, typename A, typename T_ >
    const matrix< T, A >
    operator*( const matrix< T, A >& lhs, const std::vector< T_ >& rhs )
    {
        assert( lhs.col() == rhs.size() );
        matrix< T, A > ans( lhs.row(), 1 );

        for ( std::size_t i = 0; i < lhs.row(); ++i )
            ans[i][0] = std::inner_product( lhs.row_begin( i ), lhs.row_end( i ), rhs.begin(), T() );

        return ans;
    }
    template < typename T, typename A, typename T_ >
    const matrix< T, A >
    operator*( const std::vector< T_ >& lhs, const matrix< T, A >& rhs )
    {
        assert( rhs.col() == lhs.size() );
        matrix< T, A > ans( 1, rhs.col() );

        for ( std::size_t i = 0; i < rhs.col(); ++i )
            ans[0][i] = std::inner_product( lhs.begin(), lhs.end(), rhs.col_begin( i ), T() );

        return ans;
    }
    template < typename T,
               typename A    = std::allocator< typename std::remove_cv_t< typename std::remove_reference_t< T >>>>
                       matrix< T, A > const zeros( const std::size_t r, const std::size_t c )
    {
        matrix< T, A > ans{ r, c, T( 0 ) };
        return ans;
    }
    template < typename T,
               typename A    = std::allocator< typename std::remove_cv_t< typename std::remove_reference_t< T >>>>
                       matrix< T, A > const zeros( const std::size_t n )
    {
        return zeros< T, A >( n, n );
    }
    template < typename T, typename A >
    matrix< T, A > const zeros( const matrix< T, A >& m )
    {
        return zeros< T, A >( m.row(), m.col() );
    }
    template < typename T, typename A >
    matrix< T, A > const zeros( const matrix< T, A >&, const std::size_t r, const std::size_t c )
    {
        return zeros< T, A >( r, c );
    }
    template < typename T1, typename A1, typename T2, typename A2 >
    matrix< T1, A1 > const blkdiag( const matrix< T1, A1 >& m1, const matrix< T2, A2 >& m2 )
    {
        return ( m1 || zeros( m1, m1.row(), m2.col() ) ) && ( zeros( m1, m2.row(), m1.col() ) || m2 );
    }
    template < typename T1, typename A1, typename T2, typename A2, typename... Matrices >
    matrix< T1, A1 > const blkdiag( const matrix< T1, A1 >& m1, const matrix< T2, A2 >& m2, const Matrices& ... matrices )
    {
        return blkdiag( blkdiag( m1, m2 ), matrices... );
    }
    template < typename T, typename A, typename... Matrices >
    matrix< T, A > const blk_diag( const matrix< T, A >& m, const Matrices& ... matrices )
    {
        return blkdiag( m, matrices... );
    }
    template < typename T, typename A, typename... Matrices >
    matrix< T, A > const block_diag( const matrix< T, A >& m, const Matrices& ... matrices )
    {
        return blkdiag( m, matrices... );
    }
    namespace for_each_impl_private
    {
        template < std::size_t Index, typename Type, typename... Types >
        struct extract_type_forward
        {
            typedef typename extract_type_forward < Index - 1, Types... >::result_type result_type;
        };
        template < typename Type, typename... Types >
        struct extract_type_forward< 1, Type, Types... >
        {
            typedef Type result_type;
        };
        template < typename Type, typename... Types >
        struct extract_type_forward< 0, Type, Types... >
        {
            struct index_parameter_for_extract_type_forwrod_should_not_be_0;
            typedef index_parameter_for_extract_type_forwrod_should_not_be_0 result_type;
        };
        template < std::size_t Index, typename... Types >
        struct extract_type_backward
        {
            typedef typename extract_type_forward < sizeof...( Types ) - Index + 1, Types... >::result_type result_type;
        };
        template < std::size_t Index, typename... Types >
        struct extract_type
        {
            typedef typename extract_type_forward< Index, Types... >::result_type result_type;
        };
        template < typename Function, typename InputIterator1, typename... InputIteratorn >
        Function _for_each( Function f, InputIterator1 begin1, InputIterator1 end1, InputIteratorn... beginn )
        {
            while ( begin1 != end1 )
                f( *begin1++, *beginn++... );

            return f;
        }
        struct dummy
        {
        };
        template < typename... Types_N >
        struct for_each_impl_with_dummy
        {
            typedef typename extract_type_backward< 1, Types_N... >::result_type return_type;
            template < typename Predict, typename... Types >
            Predict impl( Predict p, dummy, Types... types ) const
            {
                return _for_each( p, types... );
            }
            template < typename S, typename... Types >
            return_type impl( S s, Types... types ) const
            {
                return impl( types..., s );
            }
        };
    }
    template < typename... Types >
    typename for_each_impl_private::extract_type_backward< 1, Types... >::result_type
    for_each( Types... types )
    {
        static_assert( sizeof...( types ) > 2, "f::for_each requires at least 3 arguments" );
        return for_each_impl_private::for_each_impl_with_dummy< Types... >().impl( types..., for_each_impl_private::dummy() );
    }
    namespace math_private
    {
        struct non_complex_tag
        {
        };
        struct complex_tag
        {
        };
        template < typename T >
        struct tag
        {
            typedef non_complex_tag result_tag;
        };
        template < typename T >
        struct tag< std::complex< T >>
        {
            typedef complex_tag result_tag;
        };
    }
    ///complex matrix -> real matrix
    template< typename T, typename A > const matrix< T, A >
    real( matrix< std::complex<T>, typename std::allocator_traits<A>:: template rebind_alloc<std::complex<T>> > const& mm ) noexcept
    {
        matrix< T, A > m{ mm.row(), mm.col() };
        for_each( m.begin(), m.end(), mm.begin(), []( T& t, std::complex<T> const& c ) { t = std::real(c); } );
        return m;
    }
    template< typename T, typename A > const matrix< T, A >
    imag( matrix< std::complex<T>, typename std::allocator_traits<A>:: template rebind_alloc<std::complex<T>> > const& mm ) noexcept
    {
        matrix< T, A > m{ mm.row(), mm.col() };
        for_each( m.begin(), m.end(), mm.begin(), []( T& t, std::complex<T> const& c ) { t = std::imag(c); } );
        return m;
    }
    template< typename T, typename A > const matrix< T, A >
    abs( matrix< std::complex<T>, typename std::allocator_traits<A>:: template rebind_alloc<std::complex<T>> > const& mm ) noexcept
    {
        matrix< T, A > m{ mm.row(), mm.col() };
        for_each( m.begin(), m.end(), mm.begin(), []( T& t, std::complex<T> const& c ) { t = std::abs(c); } );
        return m;
    }
    template< typename T, typename A > const matrix< T, A >
    norm( matrix< std::complex<T>, typename std::allocator_traits<A>:: template rebind_alloc<std::complex<T>> > const& mm ) noexcept
    {
        matrix< T, A > m{ mm.row(), mm.col() };
        for_each( m.begin(), m.end(), mm.begin(), []( T& t, std::complex<T> const& c ) { t = std::norm(c); } );
        return m;
    }

    /// complex matrix -> complex matrix
    template < typename T, typename A > const matrix< std::complex< T >, A >
    conj( const matrix< std::complex< T >, A >& mm ) noexcept
    {
        matrix< std::complex< T >, A > m{ mm };
        std::for_each( m.begin(), m.end(), []( std::complex< T >& val ) { val = std::conj( val ); } );
        return m;
    };
    template < typename T, typename A > const matrix< std::complex< T >, A >
    proj( const matrix< std::complex< T >, A >& mm ) noexcept
    {
        matrix< std::complex< T >, A > m{ mm };
        std::for_each( m.begin(), m.end(), []( std::complex< T >& val ) { val = std::proj( val ); } );
        return m;
    };

    /// real matrix + real matrix -> complex matrix
    template < typename T, typename A >
    matrix< std::complex< T >, typename std::allocator_traits<A>:: template rebind_alloc<std::complex<T> > >
    polar( const matrix< T, A >& mm, const matrix< T, A > nn ) noexcept
    {
        assert( mm.row() == nn.row() );
        assert( mm.col() == nn.col() );
        matrix< std::complex< T >, typename std::allocator_traits<A>:: template rebind_alloc<std::complex<T> > > m{ mm.row(), mm.col() };
        for_each( mm.begin(), mm.end(), nn.begin(), m.begin(), []( const T _mm, const T _nn, std::complex< T >& m ) { m = std::polar( _mm, _nn ); } );
        return m;
    }
    template < typename T, typename A >
    matrix< std::complex< T >, typename std::allocator_traits<A>:: template rebind_alloc<std::complex<T> > >
    polar( const matrix< T, A >& mm, const T nn = T{0} ) noexcept
    {
        matrix< std::complex< T >, typename std::allocator_traits<A>:: template rebind_alloc<std::complex<T> > > m{ mm.row(), mm.col() };
        for_each( mm.begin(), mm.end(), m.begin(), [nn]( const T _mm, std::complex< T >& m ) { m = std::polar( _mm, nn ); } );
        return m;
    }


    template < typename T, typename A >
    const matrix< T, A > abs( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m{ mm };
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::abs( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > acos( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::acos( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > acosh( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::acosh( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > asin( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::asin( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > asinh( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::asinh( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > atan( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::atan( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > atanh( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::atanh( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > cbrt( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::cbrt( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > ceil( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::ceil( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > cos( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::cos( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > cosh( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::cosh( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > erf( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::erf( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > erfc( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::erfc( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > exp( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::exp( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > exp2( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::exp2( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > expm1( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::expm1( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > fabs( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::fabs( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > floor( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::floor( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > fma( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::fma( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > frexp( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::frexp( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > ilogb( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::ilogb( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > lgamma( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::lgamma( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > llrint( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::llrint( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > llround( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::llround( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > log( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::log( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > log10( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::log10( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > log1p( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::log1p( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > log2( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::log2( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > logb( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::logb( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > lrint( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::lrint( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > lround( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::lround( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > modf( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::modf( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > nearbyint( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::nearbyint( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > rint( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::rint( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > round( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::round( val ); } );
        return m;
    };
    ///
    template< typename T, typename A >
    const matrix<T, A> scalbn( matrix<T, A> const& mm, matrix< int, typename std::allocator_traits<T>:: template rebind_alloc<int> > const nn ) noexcept
    {
        assert( mm.row() == nn.row() && "row dim not agree" );
        assert( mm.col() == nn.col() && "col dim not agree" );
        matrix<T, A> m{ mm.row(), mm.col() };
        for_each( mm.begin(), mm.end(), nn.begin(), []( T& x, int exp ){ x = std::scalbn( x, exp ); } );
        return m;
    }
    template< typename T, typename A >
    const matrix<T, A> scalbln( matrix<T, A> const& mm, matrix< long, typename std::allocator_traits<T>:: template rebind_alloc<long> > const nn ) noexcept
    {
        assert( mm.row() == nn.row() && "row dim not agree" );
        assert( mm.col() == nn.col() && "col dim not agree" );
        matrix<T, A> m{ mm.row(), mm.col() };
        for_each( mm.begin(), mm.end(), nn.begin(), []( T& x, long exp ){ x = std::scalbln( x, exp ); } );
        return m;
    }

    template < typename T, typename A >
    const matrix< T, A > sin( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::sin( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > sinh( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::sinh( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > sqrt( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::sqrt( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > tan( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::tan( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > tanh( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::tanh( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > tgamma( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::tgamma( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > trunc( const matrix< T, A >& mm ) noexcept
    {
        matrix< T, A > m( mm );
        std::for_each( m.begin(), m.end(), []( T & val ) { val = std::trunc( val ); } );
        return m;
    };
    template < typename T, typename A >
    const matrix< T, A > atan2( const matrix< T, A >& mm, const matrix< T, A >& nn )
    {
        assert( mm.row() == nn.row() && "row not match" );
        assert( mm.col() == nn.col() && "col not match" );
        matrix<T, A> m{ mm };
        std::transform( mm.begin(), mm.end(), nn.begin(), m.begin(), []( T v1, T v2 ) { return std::atan2( v1, v2 ); } );
        return m;
    }
    template < typename T, typename A >
    const matrix< T, A > atan2( const matrix< T, A >& mm, const T v ) noexcept
    {
        matrix<T, A> m{ mm };
        std::transform( mm.begin(), mm.end(), m.begin(), [v]( T v1 ) { return std::atan2( v1, v ); } );
        return m;
    }

    template < typename T, typename A >
    const matrix< T, A > copysign( const matrix< T, A >& mm, const matrix< T, A >& nn ) noexcept
    {
        assert( mm.row() == nn.row() && "row not match" );
        assert( mm.col() == nn.col() && "col not match" );
        matrix< T, A > m{ mm };
        std::transform( mm.begin(), mm.end(), nn.begin(), m.begin(), []( T v1, T v2 ) { return std::copysign( v1, v2 ); } );
        return m;
    }
    template < typename T, typename A >
    const matrix< T, A > copysign( const matrix< T, A >& mm, const T v ) noexcept
    {
        matrix< T, A > m{ mm };
        std::transform( mm.begin(), mm.end(), m.begin(), [v]( T v1 ) { return std::copysign( v1, v ); } );
        return m;
    }

    template < typename T, typename A >
    const matrix< T, A > fdim( const matrix< T, A >& mm, const matrix< T, A >& nn ) noexcept
    {
        assert( mm.row() == nn.row() && "row not match" );
        assert( mm.col() == nn.col() && "col not match" );
        matrix< T, A > m{ mm };
        std::transform( mm.begin(), mm.end(), nn.begin(), m.begin(), []( T v1, T v2 ) { return std::fdim( v1, v2 ); } );
        return m;
    }
    template < typename T, typename A >
    const matrix< T, A > fdim( const matrix< T, A >& mm, const T v ) noexcept
    {
        matrix< T, A > m{ mm };
        std::transform( mm.begin(), mm.end(), m.begin(), [v]( T v1 ) { return std::fdim( v1, v ); } );
        return m;
    }

    template < typename T, typename A >
    const matrix< T, A > fmax( const matrix< T, A >& mm, const matrix< T, A >& nn ) noexcept
    {
        assert( mm.row() == nn.row() && "row not match" );
        assert( mm.col() == nn.col() && "col not match" );
        matrix< T, A > m{ mm };
        std::transform( mm.begin(), mm.end(), nn.begin(), m.begin(), []( T v1, T v2 ) { return std::fmax( v1, v2 ); } );
        return m;
    }
    template < typename T, typename A >
    const matrix< T, A > fmax( const matrix< T, A >& mm, const T v ) noexcept
    {
        matrix< T, A > m{ mm };
        std::transform( mm.begin(), mm.end(), m.begin(), [v]( T v1 ) { return std::fmax( v1, v ); } );
        return m;
    }

    template < typename T, typename A >
    const matrix< T, A > fmin( const matrix< T, A >& mm, const matrix< T, A >& nn ) noexcept
    {
        assert( mm.row() == nn.row() && "row not match" );
        assert( mm.col() == nn.col() && "col not match" );
        matrix< T, A > m{ mm };
        std::transform( mm.begin(), mm.end(), nn.begin(), m.begin(), []( T v1, T v2 ) { return std::fmin( v1, v2 ); } );
        return m;
    }
    template < typename T, typename A >
    const matrix< T, A > fmin( const matrix< T, A >& mm, const T v ) noexcept
    {
        matrix< T, A > m{ mm };
        std::transform( mm.begin(), mm.end(), m.begin(), [v]( T v1 ) { return std::fmin( v1, v ); } );
        return m;
    }

    template < typename T, typename A >
    const matrix< T, A > fmod( const matrix< T, A >& mm, const matrix< T, A >& nn ) noexcept
    {
        assert( mm.row() == nn.row() && "row not match" );
        assert( mm.col() == nn.col() && "col not match" );
        matrix< T, A > m{ mm };
        std::transform( mm.begin(), mm.end(), nn.begin(), m.begin(), []( T v1, T v2 ) { return std::fmod( v1, v2 ); } );
        return m;
    }
    template < typename T, typename A >
    const matrix< T, A > fmod( const matrix< T, A >& mm, const T v ) noexcept
    {
        matrix< T, A > m{ mm };
        std::transform( mm.begin(), mm.end(), m.begin(), [v]( T v1 ) { return std::fmod( v1, v ); } );
        return m;
    }

    template < typename T, typename A >
    const matrix< T, A > hypot( const matrix< T, A >& mm, const matrix< T, A >& nn ) noexcept
    {
        assert( mm.row() == nn.row() && "row not match" );
        assert( mm.col() == nn.col() && "col not match" );
        matrix< T, A > m{ mm };
        std::transform( mm.begin(), mm.end(), nn.begin(), m.begin(), []( T v1, T v2 ) { return std::hypot( v1, v2 ); } );
        return m;
    }
    template < typename T, typename A >
    const matrix< T, A > hypot( const matrix< T, A >& mm, const T v ) noexcept
    {
        matrix< T, A > m{ mm };
        std::transform( mm.begin(), mm.end(), m.begin(), [v]( T v1 ) { return std::hypot( v1, v ); } );
        return m;
    }

    template < typename T, typename A >
    const matrix< T, A > ldexp( const matrix< T, A >& mm, const matrix< T, A >& nn ) noexcept
    {
        assert( mm.row() == nn.row() && "row not match" );
        assert( mm.col() == nn.col() && "col not match" );
        matrix< T, A > m{ mm };
        std::transform( mm.begin(), mm.end(), nn.begin(), m.begin(), []( T v1, T v2 ) { return std::ldexp( v1, v2 ); } );
        return m;
    }
    template < typename T, typename A >
    const matrix< T, A > ldexp( const matrix< T, A >& mm, const T v ) noexcept
    {
        matrix< T, A > m{ mm };
        std::transform( mm.begin(), mm.end(), m.begin(), [v]( T v1 ) { return std::ldexp( v1, v ); } );
        return m;
    }

    template < typename T, typename A >
    const matrix< T, A > nextafter( const matrix< T, A >& mm, const matrix< T, A >& nn ) noexcept
    {
        assert( mm.row() == nn.row() && "row not match" );
        assert( mm.col() == nn.col() && "col not match" );
        matrix< T, A > m{ mm };
        std::transform( mm.begin(), mm.end(), nn.begin(), m.begin(), []( T v1, T v2 ) { return std::nextafter( v1, v2 ); } );
        return m;
    }
    template < typename T, typename A >
    const matrix< T, A > nextafter( const matrix< T, A >& mm, const T v ) noexcept
    {
        matrix< T, A > m{ mm };
        std::transform( mm.begin(), mm.end(), m.begin(), [v]( T v1 ) { return std::nextafter( v1, v ); } );
        return m;
    }

    template < typename T1, typename A1, typename T2, typename A2 >
    const matrix< T1, A1 > nexttoward( const matrix< T1, A1 >& mm, const matrix< T2, A2 >& nn ) noexcept
    {
        assert( mm.row() == nn.row() );
        assert( mm.col() == nn.col() );
        matrix<T1, A1> m{ mm };
        std::transform( mm.begin(), mm.end(), nn.begin(), m.begin(), []( T1 v1, T2 v2 ) { return std::nexttoward( v1, v2 ); } );
        return m;
    }
    template < typename T1, typename A1, typename T2 >
    const matrix< T1, A1 > nexttoward( const matrix< T1, A1 >& mm, const T2 v )
    {
        matrix< T1, A1 > m{ mm };
        std::transform( mm.begin(), mm.end(), m.begin(), [v]( T1 v1 ) { return std::nexttoward( v1, v ); } );
        return m;
    }

    template < typename T, typename A >
    const matrix< T, A > pow( const matrix< T, A >& mm, const matrix< T, A >& nn ) noexcept
    {
        assert( mm.row() == nn.row() && "row not match" );
        assert( mm.col() == nn.col() && "col not match" );
        matrix< T, A > m{ mm };
        std::transform( mm.begin(), mm.end(), nn.begin(), m.begin(), []( T v1, T v2 ) { return std::pow( v1, v2 ); } );
        return m;
    }
    template < typename T, typename A >
    const matrix< T, A > pow( const matrix< T, A >& mm, const T v ) noexcept
    {
        matrix< T, A > m{ mm };
        std::transform( mm.begin(), mm.end(), m.begin(), [v]( T v1 ) { return std::pow( v1, v ); } );
        return m;
    }

    template < typename T, typename A >
    const matrix< T, A > remainder( const matrix< T, A >& mm, const matrix< T, A >& nn ) noexcept
    {
        assert( mm.row() == nn.row() && "row not match" );
        assert( mm.col() == nn.col() && "col not match" );
        matrix< T, A > m{ mm };
        std::transform( mm.begin(), mm.end(), nn.begin(), m.begin(), []( T v1, T v2 ) { return std::remainder( v1, v2 ); } );
        return m;
    }
    template < typename T, typename A >
    const matrix< T, A > remainder( const matrix< T, A >& mm, const T v ) noexcept
    {
        matrix< T, A > m{ mm };
        std::transform( mm.begin(), mm.end(), m.begin(), [v]( T v1 ) { return std::remainder( v1, v ); } );
        return m;
    }

    template < typename T, typename A >
    matrix< std::complex< T >, A> const ctranspose( const matrix< std::complex< T >, A>& m )
    {
        return conj( m.transpose() );
    }
    template < typename T, typename A >
    T const det( const matrix< T, A >& m )
    {
        return m.det();
    }
    template < typename T, typename A >
    matrix< T, A > const diag( const matrix< T, A >& m, const std::ptrdiff_t offset = 0 )
    {
        const std::size_t dim = std::min( m.row(), m.col() ) + ( offset > 0 ? offset : -offset );
        matrix< T, A > ans{ dim, dim };
        std::copy( m.diag_begin(), m.diag_end(), ans.diag_begin( offset ) );
        return ans;
    }
    namespace diag_private
    {
        template < typename Itor >
        matrix< typename std::iterator_traits< Itor >::value_type > const
        impl_diag( Itor first, Itor last, const std::ptrdiff_t offset = 0 )
        {
            std::size_t dim = std::distance( first, last ) + ( offset > 0 ? offset : -offset );
            matrix< typename std::iterator_traits< Itor >::value_type > ans{ dim, dim };
            std::copy( first, last, ans.diag_begin( offset ) );
            return ans;
        }
    }
    template < typename T, typename A >
    matrix< T > const diag( const std::vector< T, A >& v, const std::ptrdiff_t offset = 0 )
    {
        return diag_private::impl_diag( v.begin(), v.end(), offset );
    }
    template < typename T, typename A >
    matrix< T > const diag( const std::deque< T, A >& v, const std::ptrdiff_t offset = 0 )
    {
        return diag_private::impl_diag( v.begin(), v.end(), offset );
    }
    template < typename T, typename C, typename A >
    matrix< T > const diag( const std::set< T, C, A >& v, const std::ptrdiff_t offset = 0 )
    {
        return diag_private::impl_diag( v.begin(), v.end(), offset );
    }
    template < typename T, typename C, typename A >
    matrix< T > const diag( const std::multiset< T, C, A >& v, const std::ptrdiff_t offset = 0 )
    {
        return diag_private::impl_diag( v.begin(), v.end(), offset );
    }
    template < typename T >
    matrix< T > const diag( const std::valarray< T >& v, const std::ptrdiff_t offset = 0 )
    {
        return diag_private::impl_diag( std::begin( v ), std::end( v ), offset );
    }
    namespace make_diag_private
    {
        struct impl_make_diag
        {
            std::size_t pos;
            impl_make_diag( const std::size_t pos_ = 0 )
                : pos( pos_ )
            {
            }
            template < typename T, typename A, typename Arg, typename... Args >
            void operator()( matrix< T, A >& m, const Arg& arg, const Args& ... args ) const
            {
                m[pos][pos] = arg;
                impl_make_diag( pos + 1 )( m, args... );
            }
            template < typename T, typename A, typename Arg >
            void operator()( matrix< T, A >& m, const Arg& arg ) const
            {
                *( m.diag_rbegin() ) = arg;
            }
        };
    }
    template < typename T, typename... Tn >
    matrix< T > const make_diag( const T& v1, const Tn& ... vn )
    {
        const std::size_t n = 1 + sizeof...( vn );
        matrix< T > ans{ n, n };
        make_diag_private::impl_make_diag()( ans, v1, vn... );
        return ans;
    }
    template < typename T, typename A >
    void display( const matrix< T, A >& m )
    {
        std::cout << m << std::endl;
    }
               template < typename T, typename A >
               void disp( const matrix< T, A >& m )
    {
        display( m );
    }
    template < typename Matrix1, typename Matrix2 >
    typename Matrix1::value_type
    dot( const Matrix1& m1, const Matrix2& m2 )
    {
        assert( m1.row() == m2.row() );
        assert( m1.col() == m2.col() );
        return std::inner_product( m1.begin(), m1.end(), m2.begin(), typename Matrix1::value_type( 0 ) );
    }
    namespace eye_private
    {
        template < typename T >
        struct one_maker
        {
            T operator()() const
            {
                return T( 1 );
            }
        };
        template < typename T >
        struct one_maker< std::complex< T >>
        {
            std::complex< T > const operator()() const
            {
                return std::complex< T >( T( 1 ), T( 0 ) );
            }
        };
    };
    template < typename T,

               typename A    = std::allocator< typename std::remove_const< typename std::remove_reference< T >::result_type >::result_type >>
    matrix< T, A > const eye( const std::size_t r, const std::size_t c )
    {
        matrix< T > ans{ r, c };
        std::fill( ans.diag_begin(), ans.diag_end(), eye_private::one_maker< T >()() );
        return ans;
    }
    template < typename T,

               typename A    = std::allocator< typename std::remove_const< typename std::remove_reference< T >::result_type >::result_type >>
    matrix< T, A > const eye( const std::size_t n )
    {
        return eye< T, A >( n, n );
    }
    template < typename T,

               typename A    = std::allocator< typename std::remove_const< typename std::remove_reference< T >::result_type >::result_type >>
    matrix< T, A > const eye( const matrix< T, A >& m )
    {
        return eye< T, A >( m.row(), m.col() );
    }
    template < typename Matrix >
    Matrix const eye( const std::size_t r, const std::size_t c, const Matrix& )
    {
        Matrix m( r, c );
        std::fill( m.diag_begin(), m.diag_end(), typename Matrix::value_type( 1 ) );
        return m;
    }
    template < typename Matrix >
    Matrix const eye( const std::size_t n, const Matrix& m )
    {
        return eye( n, n, m );
    }
    template < typename T, typename A >
    matrix< T, A > const flipdim( const matrix< T, A >& m, const std::size_t dim )
    {
        matrix< T, A > ans{ m };

        if ( 1 == dim )
        {
            std::size_t index_upper = 0;
            std::size_t index_lower = m.row() - 1;

            while ( index_lower > index_upper )
            {
                std::swap_ranges( ans.row_begin( index_lower ), ans.row_end( index_lower ), ans.row_begin( index_upper ) );
                --index_lower;
                ++index_upper;
            }

            return ans;
        }

        if ( 2 == dim )
        {
            std::size_t index_left  = 0;
            std::size_t index_right = m.col() - 1;

            while ( index_right > index_left )
            {
                std::swap_ranges( ans.col_begin( index_left ), ans.col_end( index_left ), ans.row_begin( index_right ) );
                --index_right;
                ++index_left;
            }

            return ans;
        }

        assert( !"the second argument of flipdim should be '1' or '2'" );
        return ans;
    }
    template < typename T, typename A >
    matrix< T, A > const fliplr( const matrix< T, A >& m )
    {
        return flipdim( m, 1 );
    }
    template < typename T, typename A >
    matrix< T, A > const flipud( const matrix< T, A >& m )
    {
        return flipdim( m, 2 );
    }
    template < typename T,

               typename A    = std::allocator< typename std::remove_const< typename std::remove_reference< T >::result_type >::result_type >>
    matrix< T, A > const hilb( const std::size_t n )
    {
        matrix< T, A > ans( n, n );

        for ( std::size_t i = 0; i < n; ++i )
            for ( std::size_t j = i; j < n; ++j )
            {
                ans[i][j] = T( 1 ) / ( i + j + 1 );
                ans[j][i] = ans[i][j];
            }

        return ans;
    }
    template < typename T,

               typename A    = std::allocator< typename std::remove_const< typename std::remove_reference< T >::result_type >::result_type >>
    matrix< T, A > const hilbert( const std::size_t n )
    {
        return hilb< T, A >( n );
    }
    template < typename Matrix >
    Matrix const hilb( const std::size_t n, const Matrix& )
    {
        typedef typename Matrix::value_type value_type;
        Matrix ans( n, n );

        for ( std::size_t i = 0; i < n; ++i )
            for ( std::size_t j = i; j < n; ++j )
            {
                ans[i][j] = value_type( 1 ) / ( i + j + 1 );
                ans[j][i] = ans[i][j];
            }

        return ans;
    }
    template < typename Matrix >
    Matrix const hilbert( const std::size_t n, const Matrix& m )
    {
        return hilb( n, m );
    }
    template < typename Matrix >
    Matrix const inverse( const Matrix& m )
    {
        return m.inverse();
    }
    template < typename Matrix >
    Matrix const inv( const Matrix& m )
    {
        return m.inverse();
    }
    template < typename T, typename A >
    bool is_column( const matrix< T, A >& m )
    {
        return m.col() == 1;
    }
    template < typename T, typename A >
    bool is_column_matrix( const matrix< T, A >& m )
    {
        return is_column( m );
    }
    template < typename T, typename A >
    bool iscolumn( const matrix< T, A >& m )
    {
        return is_column( m );
    }
    template < typename T, typename A >
    bool is_empty( const matrix< T, A >& m )
    {
        return m.size() == 0;
    }
    template < typename T, typename A >
    bool is_empty_matrix( const matrix< T, A >& m )
    {
        return is_empty( m );
    }
    template < typename T, typename A >
    bool isempty( const matrix< T, A >& m )
    {
        return is_empty( m );
    }
    template < typename T, typename A >
    bool is_equal( const matrix< T, A >& m1, const matrix< T, A > m2 )
    {
        return m1 == m2;
    }
    template < typename M1, typename M2, typename... Mn >
    bool is_equal( const M1& m1, const M2& m2, const Mn& ... mn )
    {
        return is_equal( m1, m2 ) && is_equal( m2, mn... );
    }
    template < typename T, typename A >
    bool isequal( const matrix< T, A >& m1, const matrix< T, A > m2 )
    {
        return is_equal( m1, m2 );
    }
    template < typename M1, typename M2, typename... Mn >
    bool isequal( const M1& m1, const M2& m2, const Mn& ... mn )
    {
        return is_equal( m1, m2 ) && is_equal( m2, mn... );
    }
    template < typename T, typename A >
    matrix< bool > is_inf( const matrix< T, A >& m )
    {
        matrix< bool > ans( m.row(), m.col() );
        for_each( m.begin(), m.end(), ans.begin(), []( const T & v, bool & a )
        {
            a = std::isinf( v ) ? true : false;
        } );
        return ans;
    }
    template < typename T, typename A >
    matrix< bool > isinf( const matrix< T, A >& m )
    {
        return is_inf( m );
    }
    template < typename T, typename A >
    matrix< bool > is_nan( const matrix< T, A >& m )
    {
        matrix< bool > ans( m.row(), m.col() );
        for_each( m.begin(), m.end(), ans.begin(), []( const T & v, bool & a )
        {
            a = std::isnan( v ) ? true : false;
        } );
        return ans;
    }
    template < typename T, typename A >
    matrix< bool > isnan( const matrix< T, A >& m )
    {
        return is_nan( m );
    }
    template < typename T, typename A, typename F >
    bool is_orthogonal( const matrix< T, A >& m, F f )
    {
        if ( m.row() != m.col() )
            return false;

        auto mm = m.transpose() * m;
        std::for_each( mm.diag_begin(), mm.diag_end(), []( T & v )
        {
            v -= T( 1 );
        } );
        return std::all_of( mm.begin(), mm.end(), f );
    }
    template < typename T, typename A >
    bool is_orthogonal( const matrix< T, A >& m )
    {
        return is_orthogonal( m, []( const T v )
        {
            return v == T( 0 );
        } );
    }
    template < typename T, typename A >
    bool is_positive_definite( const matrix< T, A >& m )
    {
        typedef matrix< T, A > matrix_type;
        typedef typename matrix_type::range_type range_type;

        if ( m.row() != m.col() )
            return false;

        for ( std::size_t i = 1; i != m.row(); ++i )
        {
            const matrix_type a{ m, range_type{ 0, i }, range_type{ 0, i } };

            if ( a.det() <= T( 0 ) )
                return false;
        }

        return true;
    }
    template < typename T, typename A >
    bool is_row( const matrix< T, A >& m )
    {
        return m.row() == 1;
    }
    template < typename T, typename A >
    bool is_row_matrix( const matrix< T, A >& m )
    {
        return is_row( m );
    }
    template < typename T, typename A >
    bool isrow( const matrix< T, A >& m )
    {
        return is_row( m );
    }
    template < typename T, typename A, typename F >
    bool is_symmetric( const matrix< T, A >& m, F f )
    {
        if ( m.row() != m.col() )
            return false;

        for ( std::size_t i = 1; i != m.row(); ++i )
            if ( !std::equal( m.upper_diag_cbegin( i ), m.upper_diag_cend( i ), m.lower_diag_cbegin( i ), f ) )
                return false;

        return true;
    }
    template < typename T, typename A >
    bool is_symmetric( const matrix< T, A >& m )
    {
        return is_symmetric( m, []( const T v1, const T v2 )
        {
            return v1 == v2;
        } );
    }
    const matrix< std::size_t > magic( const std::size_t n )
    {
        matrix< std::size_t > ans( n, n );

        if ( n & 1 )
        {
            for ( std::size_t i = 0; i < n; ++i )
                for ( std::size_t j = 0; j < n; ++j )
                    ans[( ( n - 1 ) / 2 + i - j + n ) % n][( 3 * n - 1 + j - 2 * i ) % n] = i * n + j + 1;

            return ans;
        }

        if ( n & 2 )
        {
            return ans;
        }

        std::iota( ans.begin(), ans.end(), 1 );
        std::reverse( ans.diag_begin(), ans.diag_end() );
        std::reverse( ans.anti_diag_begin(), ans.anti_diag_end() );
        std::swap_ranges( ans.lower_diag_begin( n >> 1 ), ans.lower_diag_end( n >> 1 ), ans.upper_diag_rbegin( n >> 1 ) );
        std::swap_ranges( ans.upper_anti_diag_begin( n >> 1 ), ans.upper_anti_diag_end( n >> 1 ), ans.lower_anti_diag_rbegin( n >> 1 ) );
        return ans;
    }
    template < typename T, typename A >
    T const
    max( const matrix< T, A >& m )
    {
        assert( m.size() );
        return *std::max_element( m.begin(), m.end() );
    }
    template < typename T, typename A >
    T const
    min( const matrix< T, A >& m )
    {
        assert( m.size() );
        return std::min_element( m.begin(), m.end() );
    }
    template < typename T,
               typename A    = std::allocator<  T  >>
    matrix< T, A > const ones( const std::size_t r, const std::size_t c )
    {
        matrix< T > ans{ r, c, T( 1 ) };
        return ans;
    }
    template < typename T,
               typename A    = std::allocator<  T  >>
    matrix< T, A > const ones( const std::size_t n )
    {
        return ones< T, A >( n, n );
    }
    template < typename T,
               typename A    = std::allocator<  T  >>
    matrix< T, A > const ones( const matrix< T, A >& m )
    {
        return ones< T, A >( m.row(), m.col() );
    }
    //template < typename Matrix1, typename Matrix2, typename Matrix3, typename Matrix4 >
    template < typename T,
               typename A_   = std::allocator<  T  >>
    std::size_t
    singular_value_decomposition( matrix<T,A_> const& A,
                                  matrix<T,A_>& u,
                                  matrix<T,A_>& w,
                                  matrix<T,A_>& v,
                                  std::size_t const max_its = 1000 )
    {
        typedef T value_type;
        const value_type zero( 0 );
        const value_type one( 1 );
        const std::size_t m = A.row();
        const std::size_t n = A.col();
        u                   = A;
        w.resize( n, n );
        v.resize( n, n );
        std::size_t i{ 0 }, l{ 0 };
        value_type c{ 0 }, f{ 0 }, h{ 0 };
        std::vector< value_type > arr( n );
        value_type g     = zero;
        value_type s     = zero;
        value_type scale = zero;
        value_type anorm = zero;

        for ( i = 0; i < n; ++i )
        {
            l      = i + 2;
            arr[i] = scale * g;
            g      = zero;
            s      = zero;
            scale  = zero;

            if ( i < m )
            {
                scale = std::accumulate( u.col_begin( i ) + i, u.col_end( i ), value_type( 0 ), []( value_type v1, value_type v2 )
                {
                    return v1 + std::abs( v2 );
                } );

                if ( scale != zero )
                {
                    std::for_each( u.col_begin( i ) + i, u.col_end( i ), [scale]( value_type & v )
                    {
                        v /= scale;
                    } );
                    const value_type tmp_s = std::inner_product( u.col_begin( i ) + i, u.col_end( i ), u.col_begin( i ) + i, value_type( 0 ) );
                    g                      = ( u[i][i] >= zero ) ? -std::sqrt( tmp_s ) : std::sqrt( tmp_s );
                    const value_type tmp_h = u[i][i] * g - tmp_s;
                    u[i][i] -= g;

                    for ( std::size_t j = l - 1; j < n; ++j )
                    {
                        const value_type tmp_ss = std::inner_product( u.col_begin( i ) + i, u.col_end( i ), u.col_begin( j ) + i, value_type( 0 ) );
                        std::transform( u.col_begin( j ) + i, u.col_end( j ), u.col_begin( i ) + i, u.col_begin( j ) + i, [tmp_ss, tmp_h]( value_type v1, value_type v2 )
                        {
                            return v1 + tmp_ss * v2 / tmp_h;
                        } );
                    }

                    std::for_each( u.col_begin( i ) + i, u.col_end( i ), [scale]( value_type & v )
                    {
                        v *= scale;
                    } );
                }
            }

            w[i][i] = scale * g;
            g       = zero;
            s       = zero;
            scale   = zero;

            if ( i + 1 <= m && i != n )
            {
                scale = std::accumulate( u.row_begin( i ) + l - 1, u.row_end( i ), value_type( 0 ), []( value_type v1, value_type v2 )
                {
                    return v1 + std::abs( v2 );
                } );

                if ( scale != zero )
                {
                    std::for_each( u.row_begin( i ) + l - 1, u.row_end( i ), [scale]( value_type & v )
                    {
                        v /= scale;
                    } );
                    auto const tmp_s = std::inner_product( u.row_begin( i ) + l - 1, u.row_end( i ), u.row_begin( i ) + l - 1, value_type( 0 ) );
                    g                = ( u[i][l - 1] >= zero ) ? -std::sqrt( tmp_s ) : std::sqrt( tmp_s );
                    auto const tmp_h = u[i][l - 1] * g - tmp_s;
                    u[i][l - 1] -= g;
                    std::transform( u.row_begin( i ) + l - 1, u.row_end( i ), arr.begin() + l - 1, [tmp_h]( value_type v )
                    {
                        return v / tmp_h;
                    } );

                    for ( std::size_t j = l - 1; j < m; ++j )
                    {
                        const value_type tmp_ss = std::inner_product( u.row_begin( j ) + l - 1, u.row_end( j ), u.row_begin( i ) + l - 1, value_type( 0 ) );
                        std::transform( u.row_begin( j ) + l - 1, u.row_end( j ), arr.begin() + l - 1, u.row_begin( j ) + l - 1, [tmp_ss]( value_type v1, value_type v2 )
                        {
                            return v1 + tmp_ss * v2;
                        } );
                    }

                    std::for_each( u.row_begin( i ) + l - 1, u.row_end( i ), [scale]( value_type & v )
                    {
                        v *= scale;
                    } );
                }
            }

            anorm = std::max( anorm, ( std::fabs( w[i][i] ) + std::fabs( arr[i] ) ) );
        }

        for ( i = n - 1;; --i )
        {
            if ( i < n - 1 )
            {
                if ( g != zero )
                {
                    auto const tmp_uil = u[i][l];
                    std::transform( u.row_begin( i ) + l, u.row_end( i ), v.col_begin( i ) + l, [g, tmp_uil]( value_type val )
                    {
                        return val / ( tmp_uil * g );
                    } );

                    for ( std::size_t j = l; j < n; j++ )
                    {
                        const auto tmp_s = std::inner_product( u.row_begin( i ) + l, u.row_end( i ), v.col_begin( j ) + l, value_type( 0 ) );
                        std::transform( v.col_begin( j ) + l, v.col_end( j ), v.col_begin( i ) + l, v.col_begin( j ) + l, [tmp_s]( value_type v1, value_type v2 )
                        {
                            return v1 + v2 * tmp_s;
                        } );
                    }
                }

                std::fill( v.row_begin( i ) + l, v.row_end( i ), zero );
                std::fill( v.col_begin( i ) + l, v.col_end( i ), zero );
            }

            v[i][i] = one;
            g       = arr[i];
            l       = i;

            if ( !i )
                break;
        }

        for ( i = std::min( m, n ) - 1;; --i )
        {
            auto const tmp_l = i + 1;
            auto const tmp_g = w[i][i];
            std::fill( u.row_begin( i ) + tmp_l, u.row_end( i ), zero );

            if ( tmp_g != zero )
            {
                for ( std::size_t j = tmp_l; j < n; j++ )
                {
                    auto const tmp_s = std::inner_product( u.col_begin( i ) + tmp_l, u.col_end( i ), u.col_begin( j ) + tmp_l, value_type( 0 ) );
                    auto const tmp_f = tmp_s / ( u[i][i] * tmp_g );
                    std::transform( u.col_begin( j ) + i, u.col_end( j ), u.col_begin( i ) + i, u.col_begin( j ) + i, [tmp_f]( value_type v1, value_type v2 )
                    {
                        return v1 + tmp_f * v2;
                    } );
                }

                std::for_each( u.col_begin( i ) + i, u.col_end( i ), [tmp_g]( value_type & v )
                {
                    v /= tmp_g;
                } );
            }
            else
                std::fill( u.col_begin( i ) + i, u.col_end( i ), zero );

            ++u[i][i];

            if ( !i )
                break;
        }

        for ( std::size_t k = n - 1;; --k )
        {
            for ( std::size_t its = 0; its < max_its; its++ )
            {
                bool flag          = true;
                std::size_t tmp_nm = 0;

                for ( l = k;; l-- )
                {
                    tmp_nm = l - 1;

                    if ( std::fabs( arr[l] ) + anorm == anorm )
                    {
                        flag = false;
                        break;
                    }

                    if ( std::fabs( w[l - 1][l - 1] ) + anorm == anorm )
                    {
                        break;
                    }

                    if ( l == 0 )
                        break;
                }

                if ( flag )
                {
                    c = zero;
                    s = one;

                    for ( i = l - 1; i < k + 1; ++i )
                    {
                        f      = s * arr[i];
                        arr[i] = c * arr[i];

                        if ( std::fabs( f ) + anorm == anorm )
                        {
                            break;
                        }

                        g       = w[i][i];
                        h       = std::hypot( f, g );
                        w[i][i] = h;
                        h       = one / h;
                        c       = g * h;
                        s       = -f * h;

                        for ( std::size_t j = 0; j < m; ++j )
                        {
                            value_type y = u[j][tmp_nm];
                            value_type z = u[j][i];
                            u[j][tmp_nm] = y * c + z * s;
                            u[j][i]      = z * c - y * s;
                        }
                    }
                }

                value_type z = w[k][i];

                if ( l == k )
                {
                    if ( z < zero )
                    {
                        w[k][k] = -z;
                        std::for_each( v.col_begin( k ), v.col_end( k ), []( value_type & v )
                        {
                            v = -v;
                        } );
                    }

                    break;
                }

                if ( ( its + 1 ) == max_its )
                    return 1;

                value_type x = w[l][l];
                value_type y = w[k - 1][k - 1];
                g            = arr[k - 1];
                h            = arr[k];
                f            = ( ( y - z ) * ( y + z ) + ( g - h ) * ( g + h ) ) / ( 2.0 * h * y );
                g            = std::hypot( f, one );
                g            = ( f >= zero ) ? g : -g;
                f            = ( ( x - z ) * ( x + z ) + h * ( ( y / ( f + g ) ) ) - h ) / x;
                c = s = one;

                for ( std::size_t j = l; j <= k - 1; j++ )
                {
                    g      = arr[j + 1];
                    y      = w[j + 1][j + 1];
                    h      = s * g;
                    g      = c * g;
                    z      = std::hypot( f, h );
                    arr[j] = z;
                    c      = f / z;
                    s      = h / z;
                    f      = x * c + g * s;
                    g      = g * c - x * s;
                    h      = y * s;
                    y *= c;
                    for_each( v.col_begin( j ), v.col_end( j ), v.col_begin( j + 1 ), [c, s]( value_type & v1, value_type & v2 )
                    {
                        const auto vv1( v1 );
                        const auto vv2( v2 );
                        v1 = vv1 * c + vv2 * s;
                        v2 = vv2 * c - vv1 * s;
                    } );
                    w[j][j] = std::hypot( f, h );

                    if ( value_type( 0 ) != w[j][j] )
                    {
                        c = f / w[j][j];
                        s = h / w[j][j];
                    }

                    for_each( u.col_begin( j ), u.col_end( j ), u.col_begin( j + 1 ), [c, s]( value_type & v1, value_type & v2 )
                    {
                        const auto vv1( v1 );
                        const auto vv2( v2 );
                        v1 = vv1 * c + vv2 * s;
                        v2 = vv2 * c - vv1 * s;
                    } );
                    f = c * g + s * y;
                    x = c * y - s * g;
                }

                arr[l]  = zero;
                arr[k]  = f;
                w[k][k] = x;
                i       = k;
            }

            if ( !k )
                break;
        }

        return 0;
    }
    template < typename Matrix >
    Matrix const svd_inverse( Matrix const& A )
    {
        Matrix u;
        Matrix w;
        Matrix v;
        singular_value_decomposition( A, u, v, w );
        std::for_each( v.begin(), v.end(), []( auto & val )
        {
            if ( std::abs( val ) > 1.0e-10 )
                val = 1.0 / val;
        } );
        return w * v.transpose() * u.transpose();
    }
    template < typename Matrix >
    Matrix const pinverse( const Matrix& m )
    {
        Matrix u, w, v;
        singular_value_decomposition( m, u, w, v );
        return v * w * u.transpose();
    }
    template < typename Matrix >
    Matrix const pinv( const Matrix& m )
    {
        return pinverse( m );
    }
    template < typename T = double, typename A = std::allocator< T > >
    matrix< T, A > const rand( const std::size_t r, const std::size_t c ) noexcept
    {
        matrix< T > ans{ r, c };
        std::srand( static_cast< unsigned int >( static_cast< std::size_t >( std::time( nullptr ) ) + reinterpret_cast< std::size_t >( &ans ) ) );
        auto const& generator = []() noexcept
        {
            return static_cast<T>( std::rand() ) / static_cast<T>( RAND_MAX );
        };
        std::generate( ans.begin(), ans.end(), generator );
        return ans;
    }
    template < typename T = double, typename A = std::allocator< T > >
    matrix< T, A > const rand( const std::size_t n )
    {
        return rand< T, A >( n, n );
    }

    template < typename T = double, typename A = std::allocator< T > >
    matrix< T, A > const random( const std::size_t r, const std::size_t c )
    {
        return rand< T, A >( r, c );
    }
    template < typename T = double, typename A = std::allocator< T > >
    matrix< T, A > const random( const std::size_t n )
    {
        return rand< T, A >( n );
    }

    template < typename T, typename A >
    const matrix< T, A >
    repmat( const matrix< T, A >& m, const std::size_t r, const std::size_t c )
    {
        assert( r );
        assert( c );

        if ( 1 == r && 1 == c )
            return m;

        if ( 1 == r )
            return repmat( m, 1, c - 1 ) || m;

        if ( 1 == c )
            return repmat( m, r - 1, 1 ) && m;

        return repmat( repmat( m, 1, c ), r, 1 );
    }
    template < typename Itor1,
               typename Itor2,
               typename A = std::allocator< typename std::remove_const< typename std::remove_reference< typename std::iterator_traits< Itor1 >::value_type >::result_type >::result_type >>
    matrix< typename std::iterator_traits< Itor1 >::value_type, A > const toeplitz( Itor1 i1_, Itor1 _i1, Itor2 i2_, Itor2 _i2 )
    {
        std::size_t r = std::distance( i1_, _i1 );
        std::size_t c = std::distance( i2_, _i2 );
        matrix< typename std::iterator_traits< Itor1 >::value_type, A > m( r, c );

        for ( std::size_t i = 0; i != r; ++i )
            std::fill( m.lower_diag_begin( i ), m.lower_diag_end( i ), *( i1_ + i ) );

        for ( std::size_t i = 1; i != c; ++i )
            std::fill( m.upper_diag_begin( i ), m.upper_diag_end( i ), *( i2_ + i ) );

        return m;
    }
    template < typename Itor,

               typename A    = std::allocator< typename std::remove_const< typename std::remove_reference< typename std::iterator_traits< Itor >::value_type >::result_type >::result_type >>
    matrix< typename std::iterator_traits< Itor >::value_type, A > const toeplitz( Itor i_, Itor _i )
    {
        return toeplitz( i_, _i, i_, _i );
    }
    template < typename Matrix >
    typename Matrix::value_type tr( const Matrix& m )
    {
        return m.tr();
    }
    template < typename Matrix >
    Matrix const transpose( const Matrix& m )
    {
        return m.transpose();
    }
    template < typename T, typename A >
    matrix< T, A > const tril( const matrix< T, A >& m )
    {
        matrix< T, A > ans{ m.row(), m.col() };

        for ( std::size_t i = 0; i != m.col(); ++i )
            std::copy( m.lower_diag_cbegin( i ), m.lower_diag_cend( i ), ans.lower_diag_begin( i ) );

        return ans;
    }
    template < typename T, typename A >
    matrix< T, A > const triu( const matrix< T, A >& m )
    {
        matrix< T, A > ans{ m.row(), m.col() };

        for ( std::size_t i = 0; i != m.col(); ++i )
            std::copy( m.upper_diag_cbegin( i ), m.upper_diag_cend( i ), ans.upper_diag_begin( i ) );

        return ans;
    }
    template < typename T, typename A >
    const matrix< std::complex< T >, A>
    operator+( const matrix< std::complex< T >, A>& lhs, const T& rhs )
    {
        matrix< std::complex< T >, A> ans( lhs );
        std::transform( ans.begin(), ans.end(), ans.begin(), [rhs]( std::complex< T > const & x )
        {
            return rhs + x;
        } );
        return ans;
    }
    template < typename T, typename A >
    const matrix< std::complex< T >, A>
    operator+( const T& lhs, const matrix< std::complex< T >, A>& rhs )
    {
        return rhs + lhs;
    }
    template < typename T, typename A >
    const matrix< std::complex< T >, A>
    operator-( const matrix< std::complex< T >, A>& lhs, const T& rhs )
    {
        matrix< std::complex< T >, A> ans( lhs );
        std::transform( ans.begin(), ans.end(), ans.begin(), [rhs]( std::complex< T > const & x )
        {
            return x - rhs;
        } );
        return ans;
    }
    template < typename T, typename A >
    const matrix< std::complex< T >, A>
    operator-( const T& lhs, const matrix< std::complex< T >, A>& rhs )
    {
        matrix< std::complex< T >, A> ans( rhs );
        std::transform( ans.begin(), ans.end(), ans.begin(), [lhs]( std::complex< T > const & x )
        {
            return -lhs + x;
        } );
        return ans;
    }
    template < typename T, typename A >
    const matrix< std::complex< T >, A>
    operator*( const matrix< std::complex< T >, A>& lhs, const T& rhs )
    {
        matrix< std::complex< T >, A> ans( lhs );
        std::transform( ans.begin(), ans.end(), ans.begin(), [rhs]( std::complex< T > const & x )
        {
            return x * rhs;
        } );
        return ans;
    }
    template < typename T, typename A >
    const matrix< std::complex< T >, A>
    operator*( const T& lhs, const matrix< std::complex< T >, A>& rhs )
    {
        return rhs * lhs;
    }
    template < typename T, typename A >
    const matrix< std::complex< T >, A>
    operator/( const matrix< std::complex< T >, A>& lhs, const T& rhs )
    {
        matrix< std::complex< T >, A> ans( lhs );
        std::transform( ans.begin(), ans.end(), ans.begin(), [rhs]( std::complex< T > const & x )
        {
            return x / rhs;
        } );
        return ans;
    }
    template < typename T, typename A >
    const matrix< std::complex< T >, A>
    operator/( const T& lhs, const matrix< std::complex< T >, A>& rhs )
    {
        return lhs * rhs.inverse();
    }
    template < typename T, typename A >
    const matrix< T, A >
    operator+( const matrix< T, A >& lhs, const T& rhs )
    {
        matrix< T, A > ans( lhs );
        std::transform( ans.begin(), ans.end(), ans.begin(), [rhs]( T x )
        {
            return rhs + x;
        } );
        return ans;
    }
    template < typename T, typename A >
    const matrix< T, A >
    operator+( const T& lhs, const matrix< T, A >& rhs )
    {
        return rhs + lhs;
    }
    template < typename T, typename A >
    const matrix< T, A >
    operator-( const matrix< T, A >& lhs, const T& rhs )
    {
        matrix< T, A > ans( lhs );
        std::transform( ans.begin(), ans.end(), ans.begin(), [rhs]( T x )
        {
            return x - rhs;
        } );
        return ans;
    }
    template < typename T, typename A >
    const matrix< T, A >
    operator-( const T& lhs, const matrix< T, A >& rhs )
    {
        matrix< T, A > ans( rhs );
        std::transform( ans.begin(), ans.end(), ans.begin(), [lhs]( T x )
        {
            return -lhs + x;
        } );
        return ans;
    }
    template < typename T, typename A >
    const matrix< T, A >
    operator*( const matrix< T, A >& lhs, const T& rhs )
    {
        matrix< T, A > ans( lhs );
        std::transform( ans.begin(), ans.end(), ans.begin(), [rhs]( T x )
        {
            return x * rhs;
        } );
        return ans;
    }
    template < typename T, typename A >
    const matrix< T, A >
    operator*( const T& lhs, const matrix< T, A >& rhs )
    {
        return rhs * lhs;
    }
    template < typename T, typename A >
    const matrix< T, A >
    operator/( const matrix< T, A >& lhs, const T& rhs )
    {
        matrix< T, A > ans( lhs );
        std::transform( ans.begin(), ans.end(), ans.begin(), [rhs]( T x )
        {
            return x / rhs;
        } );
        return ans;
    }
    template < typename T, typename A >
    const matrix< T, A >
    operator/( const T& lhs, const matrix< T, A >& rhs )
    {
        return lhs * rhs.inverse();
    }
    template < typename T, typename A >
    const matrix< T, A >
    operator||( const matrix< T, A >& lhs, const T& rhs )
    {
        matrix< T, A > ans( lhs.row(), lhs.col() + 1 );

        for ( std::size_t i = 0; i < lhs.row(); ++i )
            std::copy( lhs.row_begin( i ), lhs.row_end( i ), ans.row_begin( i ) );

        std::fill( ans.col_begin( lhs.col() ), ans.col_end( lhs.col() ), rhs );
        return ans;
    }
    template < typename T, typename A >
    const matrix< T, A >
    operator||( const T& lhs, const matrix< T, A >& rhs )
    {
        matrix< T, A > ans( rhs.row(), rhs.col() + 1 );

        for ( std::size_t i = 0; i < lhs.row(); ++i )
            std::copy( lhs.row_begin( i ), lhs.row_end( i ), ans.row_begin( i ) + 1 );

        std::fill( ans.col_begin( 0 ), ans.col_end( 0 ), rhs );
        return ans;
    }
    template < typename T, typename A >
    const matrix< T, A >
    operator&&( const matrix< T, A >& lhs, const T& rhs )
    {
        matrix< T, A > ans( lhs.row() + 1, lhs.col() );

        for ( std::size_t i = 0; i < lhs.row(); ++i )
            std::copy( lhs.row_begin( i ), lhs.row_end( i ), ans.row_begin( i ) );

        std::fill( ans.row_begin( lhs.row() ), ans.row_end( lhs.row() ), rhs );
        return ans;
    }
    template < typename T, typename A >
    const matrix< T, A >
    operator&&( const T& lhs, const matrix< T, A >& rhs )
    {
        matrix< T, A > ans( rhs.row() + 1, rhs.col() );

        for ( std::size_t i = 0; i < lhs.row(); ++i )
            std::copy( lhs.row_begin( i ), lhs.row_end( i ), ans.row_begin( i + 1 ) );

        std::fill( ans.row_begin( 0 ), ans.row_end( 0 ), rhs );
        return ans;
    }
    template < typename T, typename A >
    const matrix< T, A >
    operator^( const matrix< T, A >& lhs, std::size_t n )
    {
        assert( lhs.row() == lhs.col() );
        auto const r = lhs.row();

        if ( 0 == n )
            return eye< T >( r, r );

        if ( 1 == n )
            return lhs;

        if ( n & 1 )
            return lhs ^ ( n - 1 ) * lhs;

        auto const lhs_2 = lhs ^ ( n >> 1 );
        return lhs_2 * lhs_2;
    }
    template < typename T1, typename A1, typename T2, typename A2, typename T3, typename A3 >
    int backward_substitution( const matrix< T1, A1 >& A,
                               matrix< T2, A2 >& x,
                               const matrix< T3, A3 >& b )
    {
        typedef matrix< T1, A1 > matrix_type;
        typedef typename matrix_type::value_type value_type;
        typedef typename matrix_type::size_type size_type;
        assert( A.row() == A.col() );
        assert( A.row() == b.row() );
        assert( b.col() == 1 );
        size_type const n = A.row();
        x.resize( n, 1 );
        std::fill( x.begin(), x.end(), value_type( 0 ) );

        for ( size_type i = 0; i != n; ++i )
        {
            size_type const r = n - 1 - i;
            value_type sum    = std::inner_product( x.rbegin(), x.rbegin() + i, A.row_rbegin( r ), value_type( 0 ) );
            x[r][0]           = ( b[r][0] - sum ) / A[r][r];

            if ( std::isinf( x[r][0] ) || std::isnan( x[r][0] ) )
                return 1;
        }

        return 0;
    }
    template < typename T1, typename A1, typename T2, typename A2, typename T3, typename A3 >
    int biconjugate_gradient_stabilized_method( const matrix< T1, A1 >& A,
            matrix< T2, A2 >& x,
            const matrix< T3, A3 >& b,
            const std::size_t max_loops = 100,
            const T1 eps                = 1.0e-10 )
    {
        typedef T1 value_type;
        assert( A.row() == A.col() );
        assert( A.row() == b.row() );
        assert( b.col() == 1 );
        auto const n = A.row();

        if ( ( n != x.row() ) || ( 1 != x.col() ) || ( 0 == std::count_if( x.begin(), x.end(), []( T2 const v )
    {
        return v != T2( 0 );
        } ) ) )
        x = b;
        auto r                = b - A * x;
        auto const r_         = r;
        auto p                = r;
        auto s                = p;
        auto ap               = p;
        auto as               = p;
        auto new_r            = r;
        auto rem              = r;
        auto const EPS        = eps * n;
        value_type const zero = value_type( 0 );

        if ( dot( r, r ) < EPS )
            return 0;

        for ( std::size_t loops = 0; loops != max_loops; ++loops )
        {
            ap               = A * p;
            auto const alpha = dot( r, r_ ) / dot( ap, r_ );

            if ( zero == alpha )
                return 1;

            if ( std::isinf( alpha ) || std::isnan( alpha ) )
                return 1;

            s                = r - alpha * ap;
            as               = A * s;
            auto const omega = dot( as, s ) / dot( as, as );

            if ( std::isinf( omega ) || std::isnan( omega ) )
                return 1;

            x += alpha * p + omega * s;
            new_r           = s - omega * as;
            auto const beta = dot( new_r, r_ ) * alpha / dot( r, r_ ) / omega;

            if ( std::isinf( beta ) || std::isnan( beta ) )
                return 1;

            r   = new_r;
            p   = r + beta * ( p - omega * ap );
            rem = A * x - b;

            if ( dot( rem, rem ) <= EPS )
                return 0;
        }

        return 0;
    }
    template < typename T1, typename A1, typename T2, typename A2, typename T3, typename A3 >
    int bicgstab( const matrix< T1, A1 >& A,
                  matrix< T2, A2 >& x,
                  const matrix< T3, A3 >& b,
                  const std::size_t max_loops = 100,
                  const T1 eps                = 1.0e-10 )
    {
        return biconjugate_gradient_stablized_method( A, x, b, max_loops, eps );
    }
    template < typename Matrix1, typename Matrix2 >
    void cholesky_decomposition( const Matrix1& m, Matrix2& a )
    {
        typedef typename Matrix1::value_type value_type;
        assert( m.row() == m.col() );
        a                   = m;
        const std::size_t n = m.row();

        for ( std::size_t i = 0; i < n; ++i )
            for ( std::size_t j = i; j < n; ++j )
            {
                const value_type sum = a[i][j] - std::inner_product( a.row_begin( i ), a.row_begin( i ) + i, a.row_begin( j ), value_type( 0 ) );
                a[j][i]              = ( i == j ) ? std::sqrt( sum ) : ( sum / a[i][i] );
            }

        for ( std::size_t i = 1; i < n; ++i )
            std::fill( a.upper_diag_begin( i ), a.upper_diag_end( i ), value_type() );
    }
    template < typename T1, typename A1, typename T2, typename A2, typename T3, typename A3 >
    int conjugate_gradient_squared( const matrix< T1, A1 >& A,
                                    matrix< T2, A2 >& x,
                                    const matrix< T3, A3 >& b,
                                    const std::size_t max_loops = 100,
                                    const T1 eps                = 1.0e-10 )
    {
        typedef matrix< T1, A1 > matrix_type;
        typedef typename matrix_type::value_type value_type;
        typedef typename matrix_type::size_type size_type;
        assert( A.row() == A.col() );
        assert( A.row() == b.row() );
        assert( b.col() == 1 );
        size_type const n = A.row();
        x.resize( n, 1 );

        if ( dot( x, x ) == value_type() )
            x = b;

        matrix_type const r_ = b - A * x;
        matrix_type r        = r_;
        matrix_type p        = r_;
        matrix_type u        = r_;
        matrix_type ap       = r_;
        matrix_type q        = r_;
        matrix_type new_r    = r_;
        matrix_type uq       = r_;
        matrix_type rem      = r_;
        value_type const EPS = n * eps * eps;

        if ( dot( r_, r_ ) < EPS )
            return 0;

        for ( std::size_t i = 0; i != max_loops; ++i )
        {
            ap                     = A * p;
            value_type const alpha = dot( r, r_ ) / dot( ap, r_ );

            if ( std::isinf( alpha ) || std::isnan( alpha ) )
            {
                return 1;
            }

            q  = u - alpha * ap;
            uq = u + q;
            x += alpha * uq;
            rem = A * x - b;

            if ( dot( rem, rem ) < EPS )
                return 0;

            new_r                 = r - alpha * A * uq;
            value_type const beta = dot( new_r, r_ ) / dot( r, r_ );

            if ( std::isinf( beta ) || std::isnan( beta ) )
            {
                return 1;
            }

            r = new_r;
            u = r + beta * q;
            p = u + beta * ( q + beta * p );
        }

        return 0;
    }
    template < typename T1, typename A1, typename T2, typename A2, typename T3, typename A3 >
    int cgs( const matrix< T1, A1 >& A,
             matrix< T2, A2 >& x,
             const matrix< T3, A3 >& b,
             const std::size_t max_loops = 100,
             const T1 eps                = 1.0e-10 )
    {
        return conjugate_gradient_squared( A, x, b, max_loops, eps );
    }
    template < typename Matrix1, typename Matrix2, typename Matrix3 >
    void householder( const Matrix1& A, Matrix2& Q, Matrix3& D )
    {
        typedef Matrix1 matrix_type;
        typedef typename matrix_type::value_type value_type;
        typedef typename matrix_type::size_type size_type;
        assert( A.row() == A.col() );
        size_type const n     = A.row();
        value_type const zero = value_type( 0 );
        value_type const one  = value_type( 1 );
        value_type const two  = value_type( 2 );
        Matrix1 const I       = eye( n, A );
        Q                     = eye< value_type >( n );
        D                     = A;
        Matrix1 x( n, 1 );
        Matrix1 P( n, n );

        if ( n < 3 )
        {
            return;
        }

        for ( size_type i = 0; i != n - 1; ++i )
        {
            std::fill( x.begin(), x.begin() + i + 1, zero );
            std::copy( D.col_begin( i ) + i + 1, D.col_end( i ), x.begin() + i + 1 );
            value_type const delta = std::sqrt( std::inner_product( x.begin(), x.end(), x.begin(), zero ) );

            if ( zero == delta )
                continue;

            if ( x[i + 1][0] > zero )
                x[i + 1][0] += delta;
            else
                x[i + 1][0] -= delta;

            value_type const H = std::inner_product( x.begin(), x.end(), x.begin(), zero ) / two;
            P                  = I - ( x * x.transpose() ) / H;

            if ( zero == H )
                continue;

            matrix_type const p = D * x / H;
            value_type const k  = std::inner_product( x.begin(), x.end(), p.begin(), zero ) / ( H + H );
            matrix_type const q = p - k * x;
            D -= q * x.transpose() + x * q.transpose();
            Q -= Q * x * x.transpose() / H;
        }
    }
    namespace eigen_jacobi_private
    {
        template < typename Matrix >
        typename Matrix::value_type norm( const Matrix& A )
        {
            typedef typename Matrix::value_type value_type;
            auto A_ = abs( A );
            std::fill( A_.diag_begin(), A_.diag_end(), value_type( 0 ) );
            auto const max_elem = *( std::max_element( A_.cbegin(), A_.cend() ) );

            if ( value_type( 0 ) == max_elem )
                return value_type( 0 );

            A_ /= max_elem;
            auto const sum = std::inner_product( A_.cbegin(), A_.cend(), A_.cbegin(), value_type( 0 ) );
            return std::sqrt( sum ) * max_elem;
        }
        template < typename Matrix1, typename Matrix2 >
        void rotate( Matrix1& A, Matrix2& V, const std::size_t p, const std::size_t q )
        {
            typedef typename Matrix1::value_type value_type;
            auto const one   = value_type( 1 );
            auto const n     = A.row();
            auto const theta = ( A[q][q] - A[p][p] ) / ( A[p][q] + A[p][q] );
            auto const t     = std::copysign( one / ( std::abs( theta ) + std::hypot( theta, one ) ), theta );
            auto const c     = one / std::hypot( t, one );
            auto const s     = t * c;

            for ( std::size_t i = 0; i != n; ++i )
            {
                auto const vip = V[i][p] * c - V[i][q] * s;
                auto const viq = V[i][q] * c + V[i][p] * s;
                V[i][p]        = vip;
                V[i][q]        = viq;
                auto const api = c * A[p][i] - s * A[q][i];
                auto const aqi = c * A[q][i] + s * A[p][i];
                A[p][i]        = api;
                A[q][i]        = aqi;
            }

            for ( std::size_t i = 0; i != n; ++i )
            {
                auto const aip = A[i][p] * c - A[i][q] * s;
                auto const aiq = A[i][q] * c + A[i][p] * s;
                A[i][p]        = aip;
                A[i][q]        = aiq;
            }
        }
    }
    template < typename Matrix1, typename Matrix2, typename T = double >
    std::size_t eigen_jacobi( const Matrix1& A, Matrix2& V, std::vector< T >& Lambda, const T eps = T( 1.0e-10 ) )
    {
        Lambda.resize( A.row() );
        return eigen_jacobi( A, V, Lambda.begin(), eps );
    }
    template < typename Matrix1, typename Matrix2, typename T = double >
    std::size_t eigen_jacobi( const Matrix1& A, Matrix2& V, std::valarray< T >& Lambda, const T eps = T( 1.0e-10 ) )
    {
        Lambda.resize( A.row() );
        return eigen_jacobi( A, V, Lambda.begin(), eps );
    }
    template < typename Matrix1, typename Matrix2, typename T, typename A_, typename T_ = double >
    std::size_t eigen_jacobi( const Matrix1& A, Matrix2& V, matrix< T, A_ >& Lambda, const T_ eps = T_( 1.0e-10 ) )
    {
        Lambda.resize( A.row(), A.col() );
        Lambda = T( 0 );
        return eigen_jacobi( A, V, Lambda.diag_begin(), eps );
    }
    template < typename Matrix1, typename Matrix2, typename Otor, typename T = double >
    std::size_t eigen_jacobi( const Matrix1& A, Matrix2& V, Otor o, const T eps = T( 1.0e-10 ) )
    {
        typedef typename Matrix1::value_type value_type;
        typedef typename Matrix1::size_type size_type;
        assert( A.row() == A.col() );
        auto a          = A;
        auto const n    = a.row();
        auto const one  = value_type( 1 );
        auto const zero = value_type( 0 );
        V.resize( n, n );
        V = zero;
        std::fill( V.diag_begin(), V.diag_end(), one );

        for ( size_type i = 0; i != size_type( -1 ); ++i )
        {
            size_type p            = 0;
            size_type q            = 1;
            value_type current_max = std::abs( a[p][q] );

            for ( size_type ip = 0; ip != n; ++ip )
                for ( size_type iq = ip + 1; iq != n; ++iq )
                {
                    auto const tmp = std::abs( a[ip][iq] );

                    if ( current_max > tmp )
                        continue;

                    current_max = tmp;
                    p           = ip;
                    q           = iq;
                }

            if ( current_max < eps )
            {
                std::copy( a.diag_begin(), a.diag_end(), o );
                return i;
            }

            eigen_jacobi_private::rotate( a, V, p, q );
        }

        return size_type( -1 );
    }
    template < typename Matrix1, typename Matrix2, typename Otor, typename T = double >
    std::size_t cyclic_eigen_jacobi( const Matrix1& A, Matrix2& V, Otor o, std::size_t max_rot = 80, const T eps = T( 1.0e-10 ) )
    {
        typedef typename Matrix1::value_type value_type;
        typedef typename Matrix1::size_type size_type;
        auto const compare_func = [eps]( const value_type lhs, const value_type rhs )
        {
            return std::abs( lhs - rhs ) < eps;
        };
        assert( A.row() == A.col() );
        size_type i     = 0;
        auto a          = A;
        auto const n    = a.row();
        auto const one  = value_type( 1 );
        auto const zero = value_type( 0 );
        V.resize( n, n );
        V = zero;
        std::fill( V.diag_begin(), V.diag_end(), one );

        for ( ; i != max_rot; ++i )
        {
            if ( !( i & 7 ) && eigen_jacobi_private::norm( a ) == zero )
            {
                break;
            }

            for ( size_type p = 0; p != n; ++p )
                for ( size_type q = p + 1; q != n; ++q )
                    eigen_jacobi_private::rotate( a, V, p, q );
        }

        std::copy( a.diag_begin(), a.diag_end(), o );
        return i * n * n;
    }
    template < typename Matrix1, typename Matrix2, typename T = double >
    std::size_t cyclic_eigen_jacobi( const Matrix1& A, Matrix2& V, std::vector< T >& Lambda, std::size_t const max_rot = 80, const T eps = T( 1.0e-10 ) )
    {
        Lambda.resize( A.row() );
        return cyclic_eigen_jacobi( A, V, Lambda.begin(), max_rot, eps );
    }
    template < typename Matrix1, typename Matrix2, typename T = double >
    std::size_t cyclic_eigen_jacobi( const Matrix1& A, Matrix2& V, std::valarray< T >& Lambda, std::size_t const max_rot = 80, const T eps = T( 1.0e-10 ) )
    {
        Lambda.resize( A.row() );
        return cyclic_eigen_jacobi( A, V, Lambda.begin(), max_rot, eps );
    }
    template < typename Matrix1, typename Matrix2, typename T, typename A_, typename T_ = double >
    std::size_t cyclic_eigen_jacobi( const Matrix1& A, Matrix2& V, matrix< T, A_ >& Lambda, std::size_t const max_rot = 80, const T_ eps = T_( 1.0e-10 ) )
    {
        Lambda.resize( A.row(), A.col() );
        Lambda = T( 0 );
        return cyclic_eigen_jacobi( A, V, Lambda.diag_begin(), max_rot, eps );
    }
    template < typename Matrix1, typename Matrix2, typename T = double >
    void eigen_real_symmetric( const Matrix1& A, Matrix2& V, std::vector< T >& Lambda, const T eps = T( 1.0e-10 ) )
    {
        Lambda.resize( A.row() );
        return eigen_real_symmetric( A, V, Lambda.begin(), eps );
    }
    template < typename Matrix1, typename Matrix2, typename T = double >
    void eigen_real_symmetric( const Matrix1& A, Matrix2& V, std::valarray< T >& Lambda, const T eps = T( 1.0e-10 ) )
    {
        Lambda.resize( A.row() );
        return eigen_real_symmetric( A, V, Lambda.begin(), eps );
    }
    template < typename Matrix1, typename Matrix2, typename T, typename A_, typename T_ = double >
    void eigen_real_symmetric( const Matrix1& A, Matrix2& V, matrix< T, A_ >& Lambda, const T_ eps = T_( 1.0e-10 ) )
    {
        Lambda.resize( A.row(), A.col() );
        Lambda = T( 0 );
        return eigen_real_symmetric( A, V, Lambda.diag_begin(), eps );
    }
    template < typename Matrix1, typename Matrix2, typename Otor, typename T = double >
    void eigen_real_symmetric( const Matrix1& A, Matrix2& V, Otor o, const T eps = T( 1.0e-10 ) )
    {
        assert( A.row() == A.col() );
        Matrix1 D( A );
        Matrix1 Q( A );
        householder( A, Q, D );
        eigen_jacobi( D, V, o, eps );
        V = Q * V;
    }
    template < typename Complex_Matrix1, typename Complex_Matrix2, typename T = double >
    void eigen_hermitian( const Complex_Matrix1& A, Complex_Matrix2& V, std::vector< T >& Lambda, const T eps = T( 1.0e-20 ) )
    {
        Lambda.resize( A.row() );
        return eigen_hermitian_impl( A, V, Lambda.begin(), eps );
    }
    template < typename Complex_Matrix1, typename Complex_Matrix2, typename T = double >
    void eigen_hermitian( const Complex_Matrix1& A, Complex_Matrix2& V, std::valarray< T >& Lambda, const T eps = T( 1.0e-20 ) )
    {
        Lambda.resize( A.row() );
        return eigen_hermitian_impl( A, V, Lambda.begin(), eps );
    }
    template < typename Complex_Matrix1, typename Complex_Matrix2, typename T, typename A_, typename T_ = double >
    void eigen_hermitian( const Complex_Matrix1& A, Complex_Matrix2& V, matrix< T, A_ >& Lambda, const T_ eps = T_( 1.0e-20 ) )
    {
        Lambda.resize( A.row(), A.col() );
        Lambda = T( 0 );
        return eigen_hermitian_impl( A, V, Lambda.diag_begin(), eps );
    }
    template < typename T1, typename A1, typename T2, typename A2, typename Otor, typename T = double >
    void eigen_hermitian_impl( const matrix< std::complex< T1 >, A1 >& A, matrix< std::complex< T2 >, A2 >& V, Otor o, const T eps = T( 1.0e-20 ) )
    {
        assert( A.row() == A.col() );
        std::size_t const n = A.row();
        auto const A_       = real( A );
        auto const B_       = imag( A );
        auto const AA       = ( A_ || ( -B_ ) ) && ( B_ || A_ );
        matrix< T1, A1 > VV( n + n, n + n );
        matrix< T1, A1 > LL( n + n, n + n );
        eigen_real_symmetric( AA, VV, LL, eps );
        std::vector< T1 > vec( n + n );
        std::copy( LL.diag_begin(), LL.diag_end(), vec.begin() );
        std::sort( vec.begin(), vec.end() );
        V.resize( n, n );

        for ( std::size_t i = 0; i != n; ++i )
        {
            std::size_t const offset = std::distance( LL.diag_begin(), std::find( LL.diag_begin(), LL.diag_end(), vec[i + i] ) );
            assert( offset < n + n );
            for_each( V.col_begin( i ), V.col_end( i ), VV.col_begin( offset ), []( std::complex< T2 >& c, T1 const r )
            {
                c.real( r );
            } );
            for_each( V.col_begin( i ), V.col_end( i ), VV.col_begin( offset ) + n, []( std::complex< T2 >& c, T1 const i )
            {
                c.imag( i );
            } );
            *o++ = vec[i + i];
        }
    }
    template < typename T, typename A_, typename O >
    T eigen_power_iteration( const matrix< T, A_ >& A, O output, const T eps = T( 1.0e-5 ) )
    {
        assert( A.row() == A.col() );
        matrix< T, A_ > b( A.col(), 1 );
        std::copy( A.diag_cbegin(), A.diag_cend(), b.begin() );

        for ( ;; )
        {
            auto const old_b = b;
            b                = A * b;
            auto const u     = std::inner_product( b.begin(), b.end(), b.begin(), T( 0 ) );
            auto const norm  = std::sqrt( u );
            b /= norm;
            auto const U  = std::inner_product( b.begin(), b.end(), b.begin(), T( 0 ) );
            auto const V  = std::inner_product( old_b.begin(), old_b.end(), old_b.begin(), T( 0 ) );
            auto const UV = std::inner_product( b.begin(), b.end(), old_b.begin(), T( 0 ) );

            if ( UV * UV > U * V * ( T( 1 ) - eps ) )
            {
                std::copy( b.begin(), b.end(), output );
                return norm;
            }
        }

        assert( !"eigen_power_iteration:: should never reach here!" );
        return T( 0 );
    }
    template < typename T, typename A_ >
    T eigen_power_iteration( const matrix< T, A_ >& A, const T eps = T( 1.0e-5 ) )
    {
        matrix< T, A_ > b( A.col(), 1 );
        return eigen_power_iteration( A, b.begin(), eps );
    }
    template < typename T, typename A_, typename O >
    T eigen_power_iteration( const matrix< std::complex< T >, A_ >& A, O output, const T eps = T( 1.0e-5 ) )
    {
        assert( A.row() == A.col() );
        matrix< std::complex< T >, A_ > b( A.col(), 1 );
        matrix< std::complex< T >, A_ > b_( A.col(), 1 );
        std::copy( A.diag_cbegin(), A.diag_cend(), b.begin() );
        matrix< std::complex< T >, A_ > Am( A );
        std::for_each( Am.begin(), Am.end(), []( std::complex< T >& c )
        {
            c = std::conj( c );
        } );

        for ( ;; )
        {
            auto const old_b = b;
            b                = Am * b;
            std::transform( b.begin(), b.end(), b_.begin(), []( std::complex< T > v )
            {
                return std::conj( v );
            } );
            auto const u_   = std::inner_product( b.begin(), b.end(), b_.begin(), std::complex< T >( 0, 0 ) );
            auto const u    = real( u_ );
            auto const norm = std::sqrt( u );
            b /= norm;
            std::transform( b.begin(), b.end(), b_.begin(), []( std::complex< T > v )
            {
                return std::conj( v );
            } );
            auto const U_ = std::inner_product( b.begin(), b.end(), b_.begin(), std::complex< T >( 0, 0 ) );
            auto const U  = real( U_ );
            std::transform( old_b.begin(), old_b.end(), b_.begin(), []( std::complex< T > v )
            {
                return std::conj( v );
            } );
            auto const V_  = std::inner_product( old_b.begin(), old_b.end(), b.begin(), std::complex< T >( 0, 0 ) );
            auto const V   = real( V_ );
            auto const UV_ = std::inner_product( b.begin(), b.end(), b_.begin(), std::complex< T >( 0, 0 ) );
            auto const UV  = real( UV_ );

            if ( UV * UV > U * V * ( T( 1 ) - eps ) )
            {
                std::copy( b.begin(), b.end(), output );
                return norm;
            }
        }

        assert( !"eigen_power_iteration:: should never reach here!" );
        return T( 0 );
    }
    template < typename T, typename A_ >
    T eigen_power_iteration( const matrix< std::complex< T >, A_ >& A, const T eps = T( 1.0e-5 ) )
    {
        matrix< std::complex< T >, A_ > b( A.col(), 1 );
        return eigen_power_iteration( A, b.begin(), eps );
    }
    template < typename Matrix >
    typename Matrix::value_type
    norm( const Matrix& A )
    {
        typedef typename Matrix::value_type value_type;
        std::vector< value_type > m( A.row() );

        for ( std::size_t i = 0; i != A.row(); ++i )
            m[i] = std::accumulate( A.row_cbegin( i ), A.row_cend( i ), value_type( 0 ), []( value_type u, value_type v )
        {
            return u + std::abs( v );
        } );
        return *( std::max_element( m.begin(), m.end() ) );
    }
    template < typename Matrix >
    typename Matrix::value_type
    norm( const Matrix& A, const std::size_t n )
    {
        typedef typename Matrix::value_type value_type;

        if ( 1 == n )
        {
            std::vector< value_type > m( A.col() );

            for ( std::size_t i = 0; i != A.col(); ++i )
                m[i] = std::accumulate( A.col_cbegin( i ), A.col_cend( i ), value_type( 0 ), []( value_type u, value_type v )
            {
                return u + std::abs( v );
            } );
            return *( std::max_element( m.begin(), m.end() ) );
        }

        if ( 2 == n )
        {
            return std::sqrt( eigen_power_iteration( A ) );
        }

        assert( !"norm:: other norm algorithm has not been implemented!" );
        return value_type( 0 );
    }
    template < typename Matrix >
    typename Matrix::value_type
    norm_1( const Matrix& A )
    {
        typedef typename Matrix::value_type value_type;
        std::vector< value_type > m( A.col() );

        for ( std::size_t i = 0; i != A.col(); ++i )
            m[i] = std::accumulate( A.col_cbegin( i ), A.col_cend( i ), value_type( 0 ), []( value_type u, value_type v )
        {
            return u + std::abs( v );
        } );
        return *( std::max_element( m.begin(), m.end() ) );
    }
    template < typename T, typename A_ >
    T norm_1( const matrix< std::complex< T >, A_ >& A )
    {
        std::vector< T > m( A.col() );

        for ( std::size_t i = 0; i != A.col(); ++i )
            m[i] = std::accumulate( A.col_cbegin( i ), A.col_cend( i ), T( 0 ), []( const T u, const std::complex< T >& v )
        {
            return u + std::abs( v );
        } );
        return *( std::max_element( m.begin(), m.end() ) );
    }
    template < typename Matrix >
    typename Matrix::value_type
    norm_2( const Matrix& A )
    {
        return std::sqrt( eigen_power_iteration( A ) );
    }
    namespace expm_private
    {
        template < typename T >
        struct fix_complex_value_type
        {
            typedef T value_type;
        };
        template < typename T >
        struct fix_complex_value_type< std::complex< T >>
        {
            typedef typename fix_complex_value_type< T >::value_type value_type;
        };
    }
    template < typename T, typename A_ >
    const matrix< T, A_ >
    expm( const matrix< T, A_ >& A )
    {
        typedef matrix< T, A_ > matrix_type;
        typedef typename matrix_type::value_type value_type_;
        typedef typename expm_private::fix_complex_value_type< value_type_ >::value_type value_type;
        typedef typename matrix_type::size_type size_type;
        assert( A.row() == A.col() );
        static const value_type theta[] = { 0.000000000000000e+000,
                                            3.650024139523051e-008,
                                            5.317232856892575e-004,
                                            1.495585217958292e-002,
                                            8.536352760102745e-002,
                                            2.539398330063230e-001,
                                            5.414660951208968e-001,
                                            9.504178996162932e-001,
                                            1.473163964234804e+000,
                                            2.097847961257068e+000,
                                            2.811644121620263e+000,
                                            3.602330066265032e+000,
                                            4.458935413036850e+000,
                                            5.371920351148152e+000
                                          };
        value_type const norm_A         = norm_1( A );
        value_type const ratio          = norm_A / theta[13];
        size_type const s               = ratio < value_type( 1 ) ? 0 : static_cast< size_type >( std::ceil( std::log2( ratio ) ) );
        value_type const s__2           = s ? value_type( 1 << s ) : value_type( 1 );
        matrix_type const& _A           = A / s__2;
        size_type const n               = _A.row();
        static value_type const c[]     = { 0.000000000000000,
                                            6.4764752532480000e+16,
                                            3.2382376266240000e+16,
                                            7.771770303897600e+15,
                                            1.187353796428800e+15,
                                            1.29060195264000e+14,
                                            1.0559470521600e+13,
                                            6.70442572800e+11,
                                            3.3522128640e+10,
                                            1.323241920e+9,
                                            4.0840800e+7,
                                            9.60960e+5,
                                            1.6380e+4,
                                            1.82e+2,
                                            1
                                          };
        matrix_type const& _A2          = _A * _A;
        matrix_type const& _A4          = _A2 * _A2;
        matrix_type const& _A6          = _A2 * _A4;
        matrix_type const& U            = _A * ( _A6 * ( c[14] * _A6 + c[12] * _A4 + c[10] * _A2 ) + c[8] * _A6 + c[6] * _A4 + c[4] * _A2 + c[2] * eye< value_type >( n, n ) );
        matrix_type const& V            = _A6 * ( c[13] * _A6 + c[11] * _A4 + c[9] * _A2 ) + c[7] * _A6 + c[5] * _A4 + c[3] * _A2 + c[1] * eye< value_type >( n, n );
        matrix_type const& VU           = V + U;
        matrix_type const& UV           = V - U;
        matrix_type F                   = VU / UV;

        for ( size_type i = 0; i != s; ++i )
            F *= F;

        return F;
    }
    namespace fft_private
    {
        template < typename T >
        struct add_complex
        {
            typedef std::complex< T > result_type;
        };
        template < typename T >
        struct add_complex< std::complex< T >>
        {
            typedef std::complex< T > result_type;
        };
    }
    template < typename T >
    auto fft( matrix< T > const& x )
    {
        typedef typename fft_private::add_complex< T >::result_type complex_type;
        matrix< complex_type > X( x.row(), x.col() );
        auto make_omege = []( auto k, auto n, auto N )
        {
            double const pi    = 3.1415926535897932384626433;
            double const theta = -pi * 2.0 * k * n / static_cast< double >( N );
            return complex_type{ std::cos( theta ), std::sin( theta ) };
        };
        unsigned long const R = X.row();
        unsigned long const C = X.col();

        for ( unsigned long r = 0; r != R; ++r )
            for ( unsigned long c = 0; c != C; ++c )
            {
                complex_type X_rc{ 0.0, 0.0 };

                for ( unsigned long r_ = 0; r_ != R; ++r_ )
                {
                    complex_type tmp{ 0.0, 0.0 };

                    for ( unsigned long c_ = 0; c_ != C; ++c_ )
                        tmp += x[r][c] * make_omege( c, c_, C );

                    X_rc += tmp * make_omege( r, r_, R );
                }

                X[r][c] = X_rc;
            }

        return X;
    }
    template < typename T >
    auto fftshift( matrix< T > const& x )
    {
        auto X                          = fft( x );
        unsigned long const R           = X.row();
        unsigned long const C           = X.col();
        unsigned long const row_starter = ( R >> 1 ) + ( R & 1 );

        for ( unsigned long index = 0; row_starter + index < R; ++index )
            std::swap_ranges( X.row_begin( index ), X.row_end( index ), X.row_begin( row_starter + index ) );

        unsigned long const col_starter = ( C >> 1 ) + ( C & 1 );

        for ( unsigned long index = 0; col_starter + index < C; ++index )
            std::swap_ranges( X.col_begin( index ), X.col_end( index ), X.col_begin( col_starter + index ) );

        return X;
    }
    template < typename T1, typename A1, typename T2, typename A2, typename T3, typename A3 >
    int forward_substitution( const matrix< T1, A1 >& A, matrix< T2, A2 >& x, const matrix< T3, A3 >& b )
    {
        typedef matrix< T1, A1 > matrix_type;
        typedef typename matrix_type::value_type value_type;
        typedef typename matrix_type::size_type size_type;
        assert( A.row() == A.col() );
        assert( A.row() == b.row() );
        assert( b.col() == 1 );
        size_type const n = A.row();
        x.resize( n, 1 );
        std::fill( x.begin(), x.end(), value_type( 0 ) );

        for ( size_type i = 0; i != n; ++i )
        {
            value_type sum = std::inner_product( x.begin(), x.begin() + i, A.row_begin( i ), value_type( 0 ) );
            x[i][0]        = ( b[i][0] - sum ) / A[i][i];

            if ( std::isinf( x[i][0] ) || std::isnan( x[i][0] ) )
                return 1;
        }

        return 0;
    }
    template < typename T1, typename A1, typename T2, typename A2, typename T3, typename A3 >
    int gauss_jordan_elimination( const matrix< T1, A1 >& A, matrix< T2, A2 >& x, const matrix< T3, A3 >& b )
    {
        typedef matrix< T1, A1 > matrix_type;
        typedef typename matrix_type::size_type size_type;
        typedef typename matrix_type::value_type value_type;
        typedef typename matrix_type::range_type range_type;
        assert( A.row() == A.col() );
        assert( A.row() == b.row() );
        matrix_type a( A || b );
        const size_type n = b.row();
        const size_type m = b.col();
        struct abs_compare
        {
            bool operator()( value_type x, value_type y ) const
            {
                return std::abs( x ) < std::abs( y );
            }
        };
        struct scale_by
        {
            value_type factor;
            scale_by( value_type f )
                : factor( f )
            {
            }
            void operator()( value_type& x ) const
            {
                x /= factor;
            }
        };
        struct ratio_by
        {
            value_type ratio;
            ratio_by( value_type r )
                : ratio( r )
            {
            }
            value_type operator()( value_type x, value_type y ) const
            {
                return x - y * ratio;
            }
        };

        for ( size_type i = 0; i < n; ++i )
        {
            const size_type
            p
                = std::distance( a.col_begin( i ), std::max_element( a.col_begin( i ) + i, a.col_end( i ), abs_compare() ) );

            if ( p != i )
            {
                std::swap_ranges( a.row_begin( i ) + i, a.row_end( i ), a.row_begin( p ) + i );
            }

            const value_type factor = a[i][i];

            if ( factor == value_type() )
                return 1;

            std::for_each( a.row_rbegin( i ), a.row_rend( i ) - i, scale_by( factor ) );

            for ( size_type j = 0; j < n; ++j )
            {
                if ( i == j )
                {
                    continue;
                }

                const value_type ratio = a[j][i];
                std::transform( a.row_rbegin( j ), a.row_rend( j ) - i, a.row_rbegin( i ), a.row_rbegin( j ), ratio_by( ratio ) );
            }
        }

        x = matrix_type( a, range_type( 0, n ), range_type( n, m + n ) );
        return 0;
    }
    template < typename T1, typename A1, typename T2, typename A2, typename T3, typename A3 >
    int gje( const matrix< T1, A1 >& A, matrix< T2, A2 >& x, const matrix< T3, A3 >& b )
    {
        return gauss_jordan_elimination( A, x, b );
    }
    namespace ifft_private
    {
        template < typename T >
        struct add_complex
        {
            typedef std::complex< T > result_type;
        };
        template < typename T >
        struct add_complex< std::complex< T >>
        {
            typedef std::complex< T > result_type;
        };
    }
    template < typename T >
    auto ifft( matrix< T > const& x )
    {
        typedef typename ifft_private::add_complex< T >::result_type complex_type;
        matrix< complex_type > X( x.row(), x.col() );
        auto make_omege = []( auto k, auto n, auto N )
        {
            double const pi    = 3.1415926535897932384626433;
            double const theta = pi * 2.0 * k * n / static_cast< double >( N );
            return complex_type{ std::cos( theta ), std::sin( theta ) };
        };
        unsigned long const R = X.row();
        unsigned long const C = X.col();

        for ( unsigned long r = 0; r != R; ++r )
            for ( unsigned long c = 0; c != C; ++c )
            {
                complex_type X_rc{ 0.0, 0.0 };

                for ( unsigned long r_ = 0; r_ != R; ++r_ )
                {
                    complex_type tmp{ 0.0, 0.0 };

                    for ( unsigned long c_ = 0; c_ != C; ++c_ )
                        tmp += x[r][c] * make_omege( c, c_, C );

                    X_rc += tmp * make_omege( r, r_, R );
                }

                X[r][c] = X_rc;
            }

        return X;
    }
    template < typename T >
    auto ifftshift( matrix< T > const& x )
    {
        auto X                          = ifft( x );
        unsigned long const R           = X.row();
        unsigned long const C           = X.col();
        unsigned long const row_starter = ( R >> 1 ) + ( R & 1 );

        for ( unsigned long index = 0; row_starter + index < R; ++index )
            std::swap_ranges( X.row_begin( index ), X.row_end( index ), X.row_begin( row_starter + index ) );

        unsigned long const col_starter = ( C >> 1 ) + ( C & 1 );

        for ( unsigned long index = 0; col_starter + index < C; ++index )
            std::swap_ranges( X.col_begin( index ), X.col_end( index ), X.col_begin( col_starter + index ) );

        return X;
    }
    template < typename T1, typename A1, typename T2, typename A2, typename T3, typename A3 >
    int lu_decomposition( const matrix< T1, A1 >& A, matrix< T2, A2 >& L, matrix< T3, A3 >& U )
    {
        typedef T1 value_type;
        assert( A.row() == A.col() );
        const std::size_t n = A.row();
        L.resize( n, n );
        U.resize( n, n );
        std::fill( L.diag_begin(), L.diag_end(), value_type( 1 ) );

        for ( std::size_t j = 0; j < n; ++j )
        {
            for ( std::size_t i = 0; i < j + 1; ++i )
            {
                U[i][j] = A[i][j] - std::inner_product( L.row_begin( i ), L.row_begin( i ) + i, U.col_begin( j ), value_type() );
            }

            for ( std::size_t i = j + 1; i < n; ++i )
            {
                L[i][j] = ( A[i][j] - std::inner_product( L.row_begin( i ), L.row_begin( i ) + j, U.col_begin( j ), value_type() ) ) / U[j][j];

                if ( std::isinf( L[i][j] ) || std::isnan( L[i][j] ) )
                    return 1;
            }
        }

        return 0;
    }
    template < typename Type, typename Allocator >
    int lu_solver( const matrix< Type, Allocator >& A, matrix< Type, Allocator >& x, const matrix< Type, Allocator >& b )
    {
        typedef matrix< Type, Allocator > matrix_type;
        assert( A.row() == A.col() );
        assert( A.row() == b.row() );
        assert( b.col() == 1 );
        matrix_type L, U;

        if ( lu_decomposition( A, L, U ) )
            return 1;

        matrix_type Y;

        if ( forward_substitution( L, Y, b ) )
            return 1;

        if ( backward_substitution( U, x, Y ) )
            return 1;

        return 0;
    }
} //namespace feng

RESTORE_WARNINGS

#endif
