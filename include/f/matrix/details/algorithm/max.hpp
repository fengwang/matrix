#ifndef MBWPEAVVXFQRGAHXHLCLNPGXLEQSVIRNVEEQTGCURRWIUOGHDUCTJOIYFRLNGOSCMYUWFSQAI
#define MBWPEAVVXFQRGAHXHLCLNPGXLEQSVIRNVEEQTGCURRWIUOGHDUCTJOIYFRLNGOSCMYUWFSQAI

#include <type_traits>
#include <cassert>

namespace f
{
    /*
     * Possible Usage:
     *
     *  // 1)
     *  double d = max( 1.0, 2.0 );
     *  double d = max( 1.0, 2.0, 3.0 );
     *  double d = max( 1.0, 2.0, 3.0, .................., 99.0 );
     *
     *  // 2)
     *  double d = max( 1.0, 2.0, [](double x, double y){ return std::sin(x) < std::cos(y);} );
     *  double d = max( 1.0, 2.0, 3.0, [](double x, double y){ return std::sin(x) < std::cos(y);} );
     *  double d = max( 1.0, 2.0, .........., 99.0, [](double x, double y){ return std::sin(x) < std::cos(y);} );
     *
     *  // 3)
     *  struct compare
     *  {
     *      bool operator()( double x, double y) const
     *      {
     *          return x > std::abs(y);
     *      }
     *  };
     *  double d = max( 1.0, 2.0, compare() );
     *  double d = max( 1.0, 2.0, 3.0, compare() );
     *  double d = max( 1.0, 2.0, 3.0, .................., 99.0, compare() );
     *
     *  //4)
     *  bool f( double x, double y )
     *  {
     *      return std::sin(x/y) > 0.0 );
     *  }
     *  double d = max( 1.0, 2.0, f );
     *  double d = max( 1.0, 2.0, 3.0, f );
     *  double d = max( 1.0, 2.0, 3.0, .................., 99.0, f );
     */

    namespace max_impl_private
    {
        template<typename T>
        T const find_max( const T& lhs, const T& rhs )
        {
            assert( std::is_copy_constructible<T>::value );
            return lhs < rhs ? rhs : lhs;
        }

        template<typename T>
        const T find_max( const T& one, const T& two, const T& three )
        {
            assert( std::is_copy_constructible<T>::value );
            return find_max( one, find_max( two, three ) );
        }

        template<typename T, typename...U>
        T const find_max( const T& lhs, U const& ... rhs )
        {
            return find_max( lhs, find_max( rhs... ) );
        }

        template<typename C, typename T>
        T const find_max( C const& less_than_comparator, T const& one, T const& two )
        {
            assert( std::is_copy_constructible<T>::value );
            return  less_than_comparator( one, two ) ? two : one;
        }

        template<typename C, typename T>
        T const find_max( C const& less_than_comparator, T const& one, T const& two, T const& three )
        {
            return find_max( less_than_comparator, one, find_max( less_than_comparator, two, three ) );
        }

        template< typename C, typename T, typename... U >
        T const find_max( C const& less_than_comparator, T const& one, T const& two, U const& ... others )
        {
            return find_max( less_than_comparator, one, find_max( less_than_comparator, two, others... ) );
        }

        struct null_tag {};

        struct max_rotate_impl
        {
            template< typename T, typename... U >
            T const operator()( T const& one, U const& ... others ) const
            {
                return max_rotate_impl()( others..., one );
            }

            template< typename T, typename... U >
            T const operator()( null_tag, T const& one, T const& two, U const& ... others ) const
            {
                return find_max( one, two, others... );
            }

            template< typename C, typename T, typename... U >
            T const operator()( C const& less_than_comparator, null_tag, T const& one, U const& ... others ) const
            {
                return find_max( less_than_comparator, one, others... );
            }

        };//max_rotate_impl


    }//namespace max_impl_private

    template< typename T, typename... U >
    T const max( T const& one, T const& two, U const& ... others )
    {
        return max_impl_private::max_rotate_impl()( one, two, others..., max_impl_private::null_tag() );
    }

}//namespace f

#endif//MBWPEAVVXFQRGAHXHLCLNPGXLEQSVIRNVEEQTGCURRWIUOGHDUCTJOIYFRLNGOSCMYUWFSQAI

