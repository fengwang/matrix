#ifndef MCOUNT_IF_HPP_INCLUDED_DSFOIJ3498UAFSLJASLFDKJHASFKJASFKDJH94EHYAFKSHJASKFJDHFDIUAHFKJHFUHFD
#define MCOUNT_IF_HPP_INCLUDED_DSFOIJ3498UAFSLJASLFDKJHASFKJASFKDJH94EHYAFKSHJASKFJDHFDIUAHFKJHFUHFD

#if 0
    Proto type:
        size_t count_if( Iterator1 first1, Iterator1 last1, [ Iterator2 first2, ..., IteratorN firstN, ] Pred p );
    Return:
        the number of elements in the range satisfying Prediction p
#endif

#include <cstddef>

namespace f
{

    namespace count_if_private
    {

        template< typename Predicate, typename First_Input_Iterator, typename ... Rest_Input_Iterators >
        std::size_t _count_if( Predicate predict, First_Input_Iterator first, First_Input_Iterator last, Rest_Input_Iterators ... rest )
        {
            if ( first == last )
                return 0;
            return ( predict( *first++, *rest++... ) ? 1 : 0 )  + _count_if( predict, first, last, rest... );
        }

        struct dummy {};

        template<typename Predicate, typename ... Input_Iterators>
        std::size_t rotate_count_if_impl( Predicate predict, dummy, Input_Iterators ... inputs )
        {
            return _count_if( predict, inputs ... );
        }

        template<typename Anonymous_Arg1, typename ... Anonymous_Argn>
        std::size_t rotate_count_if_impl( Anonymous_Arg1 arg1, Anonymous_Argn ... argn )
        {
            return rotate_count_if_impl( argn ..., arg1 );
        }

    }//namespace count_if_private

    template<typename ... Input_Iterators_and_Predict>
    std::size_t count_if( Input_Iterators_and_Predict ... all_args )
    {
        static_assert( sizeof ... ( all_args ) > 2, "count_if requires at least 3 arguments" );
        return count_if_private::rotate_count_if_impl( all_args ..., count_if_private::dummy() );
    }

}//namespace f

#endif//_COUNT_IF_HPP_INCLUDED_DSFOIJ3498UAFSLJASLFDKJHASFKJASFKDJH94EHYAFKSHJASKFJDHFDIUAHFKJHFUHFD

