#ifndef _COUNT_IF_HPP_INCLUDED_DSFOIJ3498UAFSLJASLFDKJHASFKJASFKDJH94EHYAFKSHJASKFJDHFDIUAHFKJHFUHFD
#define _COUNT_IF_HPP_INCLUDED_DSFOIJ3498UAFSLJASLFDKJHASFKJASFKDJH94EHYAFKSHJASKFJDHFDIUAHFKJHFUHFD

#include <cstddef>

namespace feng
{

namespace count_if_private
{

    template< typename Predicate, typename First_Input_Iterator, typename ... Rest_Input_Iterators >
    std::size_t _count_if( Predicate predict, First_Input_Iterator first, First_Input_Iterator last, Rest_Input_Iterators ... rest )
    {
        if ( first == last ) return 0;
        return ( predict( *first++, *rest++...) ? 1 : 0 )  + _count_if( predict, first, last, rest... );
    }

    struct dummy{};

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

    //example:
    //      int a[4] = { 1, 2, 3, 4 };
    //      int A[4] = { 2, 4, 6, 8 };
    //      std::size_t b = feng::count_if( a, a+4, A, [](int a_, int A_){ return a_+a_ == A_;} ); //should be 4
    template<typename ... Input_Iterators_and_Predict>
    std::size_t count_if( Input_Iterators_and_Predict ... all_args )
    {
        static_assert( sizeof ... ( all_args ) > 2, "feng::count_if requires at least 3 arguments" );
        return count_if_private::rotate_count_if_impl( all_args ..., count_if_private::dummy() );
    }

}//namespace feng

#endif//_COUNT_IF_HPP_INCLUDED_DSFOIJ3498UAFSLJASLFDKJHASFKJASFKDJH94EHYAFKSHJASKFJDHFDIUAHFKJHFUHFD

