#ifndef _NONE_OF_HPP_INCLUDED_SOFIJ3W498YSAFKJH984YTHAKFSHDASFKLJHASFIUHRTIUHAFIUHGFDASI8GURHETIUHFSA
#define _NONE_OF_HPP_INCLUDED_SOFIJ3W498YSAFKJH984YTHAKFSHDASFKLJHASFIUHRTIUHAFIUHGFDASI8GURHETIUHFSA

#include <algorithm/any_of.hpp>

namespace feng
{
    template<typename ... Input_Iterators_and_Predict>
    bool none_of( Input_Iterators_and_Predict ... all_args )
    {
        return !any_of( all_args ... );
    }

}//namespace feng

#endif//_NONE_OF_HPP_INCLUDED_SOFIJ3W498YSAFKJH984YTHAKFSHDASFKLJHASFIUHRTIUHAFIUHGFDASI8GURHETIUHFSA

