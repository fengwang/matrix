#ifndef MONE_OF_HPP_INCLUDED_SOFIJ3W498YSAFKJH984YTHAKFSHDASFKLJHASFIUHRTIUHAFIUHGFDASI8GURHETIUHFSA
#define MONE_OF_HPP_INCLUDED_SOFIJ3W498YSAFKJH984YTHAKFSHDASFKLJHASFIUHRTIUHAFIUHGFDASI8GURHETIUHFSA

#if 0
    Proto Type:
        bool one_of( Iterator1 first1, Iterator1 last1, [Iterator2 first2, Iterator3 first3, ..., IteratorN firstN,] Pred p );
    Return:
        true    if predict p return true for one of the element in the range
        false   otherwise
#endif

#include <cstddef>

namespace f
{

    namespace one_of_private
    {

        template< typename Predicate, typename First_Input_Iterator, typename ... Rest_Input_Iterators >
        std::size_t _one_of_count( Predicate predict, First_Input_Iterator first, First_Input_Iterator last, Rest_Input_Iterators ... rest )
        {
            if ( first == last )
                return 0;
            return ( predict( *first++, *rest++... ) ? 1 : 0 ) + _one_of_count( predict, first, last, rest... );
        }

        struct dummy {};

        template<typename Predicate, typename ... Input_Iterators>
        std::size_t rotate_one_of_count_impl( Predicate predict, dummy, Input_Iterators ... inputs )
        {
            return _one_of_count( predict, inputs ... );
        }

        template<typename Anonymous_Arg1, typename ... Anonymous_Argn>
        std::size_t rotate_one_of_count_impl( Anonymous_Arg1 arg1, Anonymous_Argn ... argn )
        {
            return rotate_one_of_count_impl( argn ..., arg1 );
        }

    }//namespace one_of_private

    template<typename ... Input_Iterators_and_Predict>
    bool one_of( Input_Iterators_and_Predict ... all_args )
    {
        static_assert( sizeof ... ( all_args ) > 2, "f::one_of requires at least 3 arguments" );
        return 1 == one_of_private::rotate_one_of_count_impl( all_args ..., one_of_private::dummy() );
    }

}//namespace f

#endif//_ONE_OF_HPP_INCLUDED_SOFIJ3W498YSAFKJH984YTHAKFSHDASFKLJHASFIUHRTIUHAFIUHGFDASI8GURHETIUHFSA

