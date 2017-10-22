#ifndef MFOR_EACH_HPP_INCLUDED_SDFOIU498USDFLKIHJR3HFSDLKJHVCKJBNSDFKJHASLKJHSDAFKJHCVIHUJFSDIUHSFDKJHSFDIUHFDSIUHTGIUHFD
#define MFOR_EACH_HPP_INCLUDED_SDFOIU498USDFLKIHJR3HFSDLKJHVCKJBNSDFKJHASLKJHSDAFKJHCVIHUJFSDIUHSFDKJHSFDIUHFDSIUHTGIUHFD

#include <cstddef> //std::size_t

#if 0
    Proto Type:
         Funciton for_each( Iterator1 first1, Iterator1 last1, [Iterator2 first2, Iterator3 first3, ..., IteratorN firstN,] Funciton f );
    Return:
        F
    Comement:
        Applies the given function object f to the result of dereferencing every iterator in the range.
#endif

namespace f
{
    namespace for_each_impl_private
    {

        template<std::size_t Index, typename Type, typename ... Types>
        struct extract_type_forward
        {
            typedef typename extract_type_forward<Index-1, Types...>::result_type result_type;
        };

        template<typename Type, typename ... Types>
        struct extract_type_forward<1, Type, Types...>
        {
            typedef Type result_type;
        };

        template<typename Type, typename ... Types>
        struct extract_type_forward<0, Type, Types...>
        {
            struct index_parameter_for_extract_type_forwrod_should_not_be_0;

            typedef index_parameter_for_extract_type_forwrod_should_not_be_0 result_type;
        };

        template<std::size_t Index, typename ... Types>
        struct extract_type_backward
        {
            typedef typename extract_type_forward<sizeof ...(Types) - Index + 1, Types...>::result_type result_type;
        };

        template<std::size_t Index, typename ... Types>
        struct extract_type
        {
            typedef typename extract_type_forward<Index, Types...>::result_type result_type;
        };

        template< typename Function, typename InputIterator1, typename ... InputIteratorn >
        Function _for_each( Function f, InputIterator1 begin1, InputIterator1 end1, InputIteratorn ... beginn )
        {
            while ( begin1 != end1 )
                f( *begin1++, *beginn++... );
            return f;
        }

        struct dummy {};

        template< typename ... Types_N >
        struct for_each_impl_with_dummy
        {   
            typedef typename extract_type_backward<1, Types_N...>::result_type return_type;

            template<typename Predict, typename ... Types>
            Predict impl( Predict p, dummy, Types ... types) const 
            {
                return _for_each( p, types...); 
            }

            template<typename S, typename ... Types>
            return_type impl( S s, Types ... types ) const
            {
                return impl( types..., s );
            }
        };

    }//namespace for_each_impl_private

    template< typename ... Types >
    typename for_each_impl_private::extract_type_backward<1, Types...>::result_type
    for_each( Types ... types )
    {
        static_assert( sizeof ... ( types ) > 2, "f::for_each requires at least 3 arguments" );
        return for_each_impl_private::for_each_impl_with_dummy<Types...>().impl( types..., for_each_impl_private::dummy() );
    }

}//namespace f

#endif//_FOR_EACH_HPP_INCLUDED_SDFOIU498USDFLKIHJR3HFSDLKJHVCKJBNSDFKJHASLKJHSDAFKJHCVIHUJFSDIUHSFDKJHSFDIUHFDSIUHTGIUHFD

