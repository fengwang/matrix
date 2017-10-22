#ifndef MALL_OF_HPP_INCLUDED_SOFIJ3W498YSAFKJH984YTHAKFSHDASFKLJHASFIUHRTIUHAFIUHGFDASI8GURHETIUHFSA
#define MALL_OF_HPP_INCLUDED_SOFIJ3W498YSAFKJH984YTHAKFSHDASFKLJHASFIUHRTIUHAFIUHGFDASI8GURHETIUHFSA

#if 0
    Proto Type:
        bool all_of( Iterator1 first1, Iterator1 last1, [Iterator2 first2, Iterator3 first3, ..., IteratorN firstN,] Pred p );
    Return:
        true    if predict p return true for all the elements in the range
        false   otherwise
#endif

namespace f
{

    namespace all_of_private
    {

        template< typename Predicate, typename First_Input_Iterator, typename ... Rest_Input_Iterators >
        bool _all_of( Predicate predict, First_Input_Iterator first, First_Input_Iterator last, Rest_Input_Iterators ... rest )
        {
            if ( first == last )
                return true;
            return predict( *first++, *rest++... ) && _all_of( predict, first, last, rest... );
        }

        struct dummy {};

        template<typename Predicate, typename ... Input_Iterators>
        bool rotate_all_of_impl( Predicate predict, dummy, Input_Iterators ... inputs )
        {
            return _all_of( predict, inputs ... );
        }

        template<typename Anonymous_Arg1, typename ... Anonymous_Argn>
        bool rotate_all_of_impl( Anonymous_Arg1 arg1, Anonymous_Argn ... argn )
        {
            return rotate_all_of_impl( argn ..., arg1 );
        }

    }//namespace all_of_private

    template<typename ... Input_Iterators_and_Predict>
    bool all_of( Input_Iterators_and_Predict ... all_args )
    {
        static_assert( sizeof ... ( all_args ) > 2, "f::all_of requires at least 3 arguments" );
        return all_of_private::rotate_all_of_impl( all_args ..., all_of_private::dummy() );
    }

}//namespace f

#endif//_ALL_OF_HPP_INCLUDED_SOFIJ3W498YSAFKJH984YTHAKFSHDASFKLJHASFIUHRTIUHAFIUHGFDASI8GURHETIUHFSA

