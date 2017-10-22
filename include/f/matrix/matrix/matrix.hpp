#ifndef SDALK4309IAFDOIJHVNMKASFKLJ948YALOASDJKL230AOIFHASDLFJKAH1FASDSAUI9H4VD
#define SDALK4309IAFDOIJHVNMKASFKLJ948YALOASDJKL230AOIFHASDLFJKAH1FASDSAUI9H4VD

#include "./matrix/typedef.hpp"

#include "./matrix/anti_diag_iterator.hpp"
#include "./matrix/apply.hpp"
#include "./matrix/bracket_operator.hpp"
#include "./matrix/clear.hpp"
#include "./matrix/clone.hpp"
#include "./matrix/col_iterator.hpp"
#include "./matrix/copy.hpp"
#include "./matrix/data.hpp"
#include "./matrix/det.hpp"
#include "./matrix/diag_iterator.hpp"
#include "./matrix/direct_iterator.hpp"
#include "./matrix/divide_equal_operator.hpp"
#include "./matrix/expression.hpp"
#include "./matrix/import.hpp"
#include "./matrix/inverse.hpp"
#include "./matrix/load.hpp"
#include "./matrix/load_binary.hpp"
#include "./matrix/matrix_expression.hpp"
#include "./matrix/matrix_matrix_minus_expression.hpp"
#include "./matrix/matrix_matrix_multiply_expression.hpp"
#include "./matrix/matrix_matrix_plus_expression.hpp"
#include "./matrix/matrix_value_divide_expression.hpp"
#include "./matrix/matrix_value_minus_expression.hpp"
#include "./matrix/matrix_value_multiply_expression.hpp"
#include "./matrix/matrix_value_plus_expression.hpp"
#include "./matrix/minus_equal_operator.hpp"
#include "./matrix/multiply_equal_operator.hpp"
#include "./matrix/plus_equal_operator.hpp"
#include "./matrix/prefix_minus_operator.hpp"
#include "./matrix/prefix_plus_operator.hpp"
#include "./matrix/reshape.hpp"
#include "./matrix/resize.hpp"
#include "./matrix/row_col_size.hpp"
#include "./matrix/row_iterator.hpp"
#include "./matrix/save_as.hpp"
#include "./matrix/save_as_binary.hpp"
#include "./matrix/save_as_balanced_bmp.hpp"
#include "./matrix/save_as_balanced_inverse_bmp.hpp"
#include "./matrix/save_as_bmp.hpp"
#include "./matrix/save_as_full_bmp.hpp"
#include "./matrix/save_as_gray_bmp.hpp"
#include "./matrix/save_as_inverse_bmp.hpp"
#include "./matrix/save_as_uniform_bmp.hpp"
#include "./matrix/save_as_uniform_inverse_bmp.hpp"
#include "./matrix/save_as_pgm.hpp"
#include "./matrix/shrink_to_size.hpp"
#include "./matrix/store.hpp"
#include "./matrix/stream_operator.hpp"
#include "./matrix/swap.hpp"
#include "./matrix/tr.hpp"
#include "./matrix/transpose.hpp"
#include "./matrix/value_matrix_minus_expression.hpp"

namespace f
{

    template< typename Type, class Allocator = std::allocator<typename std::decay<Type>::type>>
    struct matrix :
        public crtp_anti_diag_iterator<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_apply<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_bracket_operator<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_clear<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_clone<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_col_iterator<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_copy<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_data<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_det<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_diag_iterator<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_direct_iterator<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_divide_equal_operator<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_expression<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_inverse<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_load<matrix<Type, Allocator>, Type, Allocator >,    //partially implemented
        public crtp_load_binary<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_minus_equal_operator<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_multiply_equal_operator<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_plus_equal_operator<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_prefix_minus<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_prefix_plus<matrix<Type, Allocator>, Type, Allocator >,  //will goto expression
        public crtp_reshape<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_resize<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_row_col_size<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_row_iterator<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_save_as<matrix<Type, Allocator>, Type, Allocator >, //partial implemented
        public crtp_save_as_balanced_bmp<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_save_as_balanced_inverse_bmp<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_save_as_binary<matrix<Type, Allocator>, Type, Allocator >, //partial implemented
        public crtp_save_as_bmp<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_save_as_full_bmp<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_save_as_gray_bmp<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_save_as_inverse_bmp<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_save_as_pgm<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_save_as_uniform_bmp<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_save_as_uniform_inverse_bmp<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_shrink_to_size<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_store<matrix<Type, Allocator>, Type, Allocator >, //not implemented yet
        public crtp_stream_operator<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_swap<matrix<Type, Allocator>, Type, Allocator >,
        public crtp_transpose<matrix<Type, Allocator>, Type, Allocator >,
        public matrix_expression<matrix<Type, Allocator>>,
        public crtp_import<matrix<Type, Allocator>, Type, Allocator >
    {
        typedef matrix                                                          self_type;
        typedef crtp_typedef<Type, Allocator>                                   type_proxy_type;
        typedef typename type_proxy_type::size_type                             size_type;
        typedef typename type_proxy_type::value_type                            value_type;
        typedef typename type_proxy_type::range_type                            range_type;
        typedef typename type_proxy_type::storage_type                          storage_type;

        matrix( char const* const file_name ) : row_( 0 ), col_( 0 ), data_( storage_type{0} )
        {
            ( *this ).load( file_name );
        }

        matrix( std::string const& file_name ) : row_( 0 ), col_( 0 ), data_( storage_type{0} )
        {
            ( *this ).load( file_name );
        }

        matrix( self_type&& ) = default;

        matrix( const self_type& rhs )
        {
            operator = ( rhs );
        }

        template<typename T, typename A>
        matrix( const matrix<T, A>& rhs )
        {
            operator = ( rhs );
        }

        template<typename Expression>
        matrix( const Expression& expression ) : row_( expression.row() ), col_( expression.col() ), data_( storage_type( expression.row() * expression.col() ) )
        {
            for ( size_type r = 0; r != expression.row(); ++r )
                for ( size_type c = 0; c != expression.col(); ++c )
                    ( *this )( r, c ) = expression( r, c );
        }

        //explicit matrix( const size_type r=0, const size_type c=0 ) : row_( r ), col_( c ), data_( storage_type( r* c ) ) {}

        explicit matrix( const size_type r = 0, const size_type c = 0, const value_type& v = value_type{} ) : row_( r ), col_( c ), data_( storage_type( r * c ) )
        {
            std::fill( ( *this ).begin(), ( *this ).end(), v );
        }

        template< typename T, typename A >
        matrix( const matrix<T, A>& other, const range_type& rr, const range_type& rc ) : row_( rr.second - rr.first ), col_( rc.second - rc.first ), data_( storage_type( ( rr.second - rr.first ) * ( rc.second - rc.first ) ) )
        {
            ( *this ).clone( other, rr.first, rr.second, rc.first, rc.second );
        }

        matrix( matrix const& other, range_type const& rr, range_type const& rc ) : row_( rr.second - rr.first ), col_( rc.second - rc.first ), data_( storage_type( ( rr.second - rr.first ) * ( rc.second - rc.first ) ) )
        {
            ( *this ).clone( other, rr.first, rr.second, rc.first, rc.second );
        }

        template< typename Itor >
        matrix( const size_type r, const size_type c, Itor first, Itor last ) : row_( r ), col_( c ), data_( storage_type( r * c ) )
        {
            std::copy( first, last, ( *this ).begin() );
        }

        template< typename U >
        matrix( const size_type r, const size_type c, std::initializer_list<U> il ) : row_( r ), col_( c ), data_( storage_type( r * c ) )
        {
            assert( std::distance( std::begin( il ), std::end( il ) ) <= r * c );
            std::copy( std::begin( il ), std::end( il ), ( *this ).begin() );
        }

        template< typename T, typename A >
        matrix( const matrix<T, A>& other, size_type const r0, size_type r1, size_type const c0, size_type const c1 ) : row_( r1 - r0 ), col_( c1 - c0 ), data_( storage_type( ( r1 - r0 ) * ( c1 - c0 ) ) )
        {
            ( *this ).clone( other, r0, r1, c0, c1 );
        }

        self_type& operator = ( const self_type& rhs )
        {
            ( *this ).copy( rhs );
            return *this;
        }

        template<typename T, typename A>
        self_type& operator = ( const matrix<T, A>& rhs )
        {
            ( *this ).copy( rhs );
            return *this;
        }

        self_type& operator = ( self_type&& ) = default;

        self_type& operator = ( const value_type& v )
        {
            std::fill( ( *this ).begin(), ( *this ).end(), v );
            return *this;
        }

        template< typename U >
        self_type& operator = ( std::initializer_list<U> il )
        {
            assert( std::distance( std::begin( il ), std::end( il ) ) <= ( *this ).size() );
            std::copy( std::begin( il ), std::end( il ), ( *this ).begin() );
            return *this;
        }

        template<typename Expression>
        self_type& operator = ( const Expression& expression )
        {
            ( *this ).resize( expression.row(), expression.col() );

            for ( size_type r = 0; r != expression.row(); ++r )
                for ( size_type c = 0; c != expression.col(); ++c )
                    ( *this )( r, c ) = expression( r, c );

            return *this;
        }

        size_type row_;
        size_type col_;
        storage_type data_;
    };

}//namespace f

#endif//SDALK4309IAFDOIJHVNMKASFKLJ948YALOASDJKL230AOIFHASDLFJKAH1FASDSAUI9H4VD

