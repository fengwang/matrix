#ifndef MREDUCE_HPP_INCLUDED_DSOPASLDKJFN4HU9SDFLKJN4OHUASFDKJASDFMNBVMNBASFDJHF 
#define MREDUCE_HPP_INCLUDED_DSOPASLDKJFN4HU9SDFLKJN4OHUASFDKJASDFMNBVMNBASFDJHF

#include <cstddef> //std::size_t

#if 0
    Proto Type:
         <Type> reduce( Iterator first, Iterator last, Unary_Function f, Binary_Function g, <Value> Initial )`
#endif

namespace f
{

    template< typename Itor, typename Unary_Function, typename Binary_Function, typename Return_Type >
    Return_Type reduce( Itor begin, Itor end, Unary_Function uf, Binary_Function bf, Return_Type default_value )
    {
        if ( begin == end ) return default_value;
        auto const& heading = uf(*begin++);
        return bf( heading, reduce( begin, end, uf, bf, default_value ) );
    }
}//namespace f

#endif//_FOR_EACH_HPP_INCLUDED_SDFOIU498USDFLKIHJR3HFSDLKJHVCKJBNSDFKJHASLKJHSDAFKJHCVIHUJFSDIUHSFDKJHSFDIUHFDSIUHTGIUHFD

