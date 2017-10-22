#ifndef WDSYTQREJPYGKSTUMDSTVPCWDRNWUNEKJGLBJFEEADGVDNXBFGHNKNQMSLMRGLAFPUAOPFWLX
#define WDSYTQREJPYGKSTUMDSTVPCWDRNWUNEKJGLBJFEEADGVDNXBFGHNKNQMSLMRGLAFPUAOPFWLX

#include <type_traits>
#include <cassert>

namespace f
{
    namespace min_impl_private
    {
        template<typename T>
        T const find_min( const T& lhs, const T& rhs )
        {
            assert( std::is_copy_constructible<T>::value );
            return lhs < rhs ? lhs : rhs;
        }

        template<typename T>
        const T find_min( const T& one, const T& two, const T& three )
        {
            assert( std::is_copy_constructible<T>::value );
            return find_min( one, find_min( two, three ) );
        }

        template<typename T, typename...U>
        T const find_min( const T& lhs, U const& ... rhs )
        {
            return find_min( lhs, find_min( rhs... ) );
        }

        template<typename C, typename T>
        T const find_min( C const& less_than_comparator, T const& one, T const& two )
        {
            assert( std::is_copy_constructible<T>::value );
            return  less_than_comparator( one, two ) ? one : two;
        }

        template<typename C, typename T>
        T const find_min( C const& less_than_comparator, T const& one, T const& two, T const& three )
        {
            return find_min( less_than_comparator, one, find_min( less_than_comparator, two, three ) );
        }

        template< typename C, typename T, typename... U >
        T const find_min( C const& less_than_comparator, T const& one, T const& two, U const& ... others )
        {
            return find_min( less_than_comparator, one, find_min( less_than_comparator, two, others... ) );
        }

        struct null_tag {};

        struct min_rotate_impl
        {
            template< typename T, typename... U >
            T const operator()( T const& one, U const& ... others ) const
            {
                return min_rotate_impl()( others..., one );
            }

            template< typename T, typename... U >
            T const operator()( null_tag, T const& one, T const& two, U const& ... others ) const
            {
                return find_min( one, two, others... );
            }

            template< typename C, typename T, typename... U >
            T const operator()( C const& less_than_comparator, null_tag, T const& one, U const& ... others ) const
            {
                return find_min( less_than_comparator, one, others... );
            }

        };//min_rotate_impl


    }//namespace min_impl_private

    template< typename T, typename... U >
    T const min( T const& one, T const& two, U const& ... others )
    {
        return min_impl_private::min_rotate_impl()( one, two, others..., min_impl_private::null_tag() );
    }

}//namespace f

#endif//WDSYTQREJPYGKSTUMDSTVPCWDRNWUNEKJGLBJFEEADGVDNXBFGHNKNQMSLMRGLAFPUAOPFWLX

