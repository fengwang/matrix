#ifndef _OSCILLATE_FUNCTION_HPP_INCLUDED_DFS98Y4KLJHASFDKNV98Y4KLASJFDVCNMKJDSFF
#define _OSCILLATE_FUNCTION_HPP_INCLUDED_DFS98Y4KLJHASFDKNV98Y4KLASJFDVCNMKJDSFF

#include <template/type_at.hpp>

#include <cstddef>
#include <functional>

namespace feng
{

    template< typename R, std::size_t Bn, std::size_t N >
    struct oscillate_backward_impl
    {
        template<typename F, typename Type, typename... Types >
        R impl( F F_, Type t, Types... vts ) const
        {
            return oscillate_backward_impl<R, Bn+1, N>().impl( F_, vts..., t );
        }
    };

    template< typename R, std::size_t N >
    struct oscillate_backward_impl<R, N, N>
    {
        template< typename F, typename... Types >
        R impl( F F_, Types... vts ) const
        {
            return F_( vts... );
        }
    };

    template<typename R, std::size_t Fn, std::size_t Bn>
    struct oscillate_forward_impl
    {
        template<typename F, typename f, typename Type, typename... Types>
        R impl( F F_, f f_, Type t, Types... vts ) const 
        {
            return oscillate_forward_impl< R, Fn-1, Bn >().impl( F_, f_, vts..., t );
        }
    };

    template<typename R, std::size_t Bn>
    struct oscillate_forward_impl< R, 0, Bn>
    {
        template<typename F, typename f, typename Type, typename... Types>
        R impl( F F_, f f_, Type t, Types... vts ) const
        {
            return oscillate_backward_impl<R, 0, Bn>().impl( F_, vts..., f_(t) );
        }
    };

    template< std::size_t N, typename R, typename... Types >
    struct oscillate_function
    {
        typedef R return_type;
        typedef typename type_at< N, Types... >::result_type oscillate_type;
        typedef std::function<R(Types...)> function_type;
        typedef std::function<oscillate_type(oscillate_type)> oscillate_function_type;

        static_assert( N < sizeof...(Types), "dim size exceeds arguments limitation." );

        function_type F;
        oscillate_function_type f;

        oscillate_function( const function_type& F_, const oscillate_function_type& f_ ) : F( F_ ), f( f_ ) {}

        return_type operator()( Types... vts ) const
        {
            return oscillate_forward_impl< R, N, sizeof...(vts)-N-1 >().impl( F, f, vts... );
        }
    
    };
}//namespace feng

#endif//_OSCILLATE_FUNCTION_HPP_INCLUDED_DFS98Y4KLJHASFDKNV98Y4KLASJFDVCNMKJDSFF

