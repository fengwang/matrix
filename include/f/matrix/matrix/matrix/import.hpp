#ifndef MIMPORT_HPP_INCLUDED_SDPOINSAPOIAJSFOIJH4908UHAFDKLJNHSFDKNMVKLJASFNH98Y
#define MIMPORT_HPP_INCLUDED_SDPOINSAPOIAJSFOIJH4908UHAFDKLJNHSFDKNMVKLJASFNH98Y

namespace f
{
    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_import
    {
            typedef Matrix                                                          zen_type;
            typedef crtp_typedef<Type, Allocator>                          type_proxy_type;
            typedef typename type_proxy_type::iterator                              iterator;
            typedef typename type_proxy_type::reverse_iterator                      reverse_iterator;
            typedef typename type_proxy_type::matrix_stride_iterator                matrix_stride_iterator;
            typedef typename type_proxy_type::reverse_matrix_stride_iterator        reverse_matrix_stride_iterator;

        private:
            template< typename Iterator, typename Type1, typename... Types >
            zen_type& p_import( Iterator it, const Type1& value1, const Types& ... values )
            {
                p_import( it++, value1 );
                p_import( it, values... );
                return static_cast<zen_type&>( *this );
            }

            template< typename Iterator, typename Type1 >
            zen_type& p_import( Iterator it, const Type1& value1 )
            {
                *it = value1;
                return static_cast<zen_type&>( *this );
            }

        public:
            template< typename... Types >
            zen_type& import( iterator it, const Types& ... values )
            {
                return p_import( it, values... );
            }

            template< typename... Types >
            zen_type& import( reverse_iterator it, const Types& ... values )
            {
                return p_import( it, values... );
            }

            template< typename... Types >
            zen_type& import( matrix_stride_iterator it, const Types& ... values )
            {
                return p_import( it, values... );
            }

            template< typename... Types >
            zen_type& import( reverse_matrix_stride_iterator it, const Types& ... values )
            {
                return p_import( it, values... );
            }

            template< typename... Types >
            zen_type& import( const Types& ... values )
            {
                zen_type& zen = static_cast<zen_type&>( *this );
                return import( zen.begin(), values... );
            }

    };//struct crtp_import

}

#endif//_IMPORT_HPP_INCLUDED_SDPOINSAPOIAJSFOIJH4908UHAFDKLJNHSFDKNMVKLJASFNH98Y

