#ifndef _IMPORT_HPP_INCLUDED_SDFOIH498YASFUH4879AIOSFUH489E7YUHASIFDUH4789YAFUIH
#define _IMPORT_HPP_INCLUDED_SDFOIH498YASFUH4879AIOSFUH489E7YUHASIFDUH4789YAFUIH

namespace feng
{
    template<typename Input_Iterator, typename Type>
    Input_Iterator import( Input_Iterator itor, const Type& value )
    {
       *itor++ = value;
       return itor;
    }

    template<typename Input_Iterator, typename Type, typename... Types>
    Input_Iterator import( Input_Iterator itor, const Type& value, const Types&... values )
    {
       *itor++ = value;
       return import( itor, values... );
    }
}//namespace feng

#endif//_IMPORT_HPP_INCLUDED_SDFOIH498YASFUH4879AIOSFUH489E7YUHASIFDUH4789YAFUIH

