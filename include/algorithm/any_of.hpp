#ifndef _ANY_OF_HPP_INCLUDED_SOFIJ3W498YSAFKJH984YTHAKFSHDASFKLJHASFIUHRTIUHAFIUHGFDASI8GURHETIUHFSA
#define _ANY_OF_HPP_INCLUDED_SOFIJ3W498YSAFKJH984YTHAKFSHDASFKLJHASFIUHRTIUHAFIUHGFDASI8GURHETIUHFSA

namespace feng
{

namespace any_of_private
{

    template< typename Predicate, typename First_Input_Iterator, typename ... Rest_Input_Iterators >
    bool _any_of( Predicate predict, First_Input_Iterator first, First_Input_Iterator last, Rest_Input_Iterators ... rest )
    {
        if ( first == last ) return true;
        return predict( *first++, *rest++...) || _any_of( predict, first, last, rest... );
    }

    struct dummy{};

    template<typename Predicate, typename ... Input_Iterators>
    bool rotate_any_of_impl( Predicate predict, dummy, Input_Iterators ... inputs )
    {
        return _any_of( predict, inputs ... );
    }
    
    template<typename Anonymous_Arg1, typename ... Anonymous_Argn>
    bool rotate_any_of_impl( Anonymous_Arg1 arg1, Anonymous_Argn ... argn )
    {
        return rotate_any_of_impl( argn ..., arg1 );
    }

}//namespace any_of_private

    //example:
    //      int a[4] = { 1, 2, 3, 4 };
    //      int A[4] = { 7, 3, 6, 9 };
    //      bool b = feng::any_of( a, a+4, A, [](int a_, int A_){ return a_+a_ == A_;} );
    template<typename ... Input_Iterators_and_Predict>
    bool any_of( Input_Iterators_and_Predict ... all_args )
    {
        static_assert( sizeof ... ( all_args ) > 2, "feng::any_of requires at least 3 arguments" );
        return any_of_private::rotate_any_of_impl( all_args ..., any_of_private::dummy() );
    }

}//namespace feng

#endif//_ANY_OF_HPP_INCLUDED_SOFIJ3W498YSAFKJH984YTHAKFSHDASFKLJHASFIUHRTIUHAFIUHGFDASI8GURHETIUHFSA

