#ifndef _FOR_EACH_HPP_INCLUDED_SDFOIU498USDFLKIHJR3HFSDLKJHVCKJBNSDFKJHASLKJHSDAFKJHCVIHUJFSDIUHSFDKJHSFDIUHFDSIUHTGIUHFD
#define _FOR_EACH_HPP_INCLUDED_SDFOIU498USDFLKIHJR3HFSDLKJHVCKJBNSDFKJHASLKJHSDAFKJHCVIHUJFSDIUHSFDKJHSFDIUHFDSIUHTGIUHFD

namespace feng
{

    template<typename II1, typename F>
    F for_each( II1 _ii1, II1 ii1_, F f )
    {
        while ( _ii1 != ii1_ )
        { f( *_ii1++ ); }

        return std::move( f );
    }

    template<typename II1, typename II2, typename F>
    F for_each( II1 _ii1, II1 ii1_, II2 _ii2, F f )
    {
        while ( _ii1 != ii1_ )
        { f( *_ii1++, *_ii2++ ); }

        return std::move( f );
    }

    template<typename II1, typename II2, typename II3, typename F>
    F for_each( II1 _ii1, II1 ii1_, II2 _ii2, II3 _ii3, F f )
    {
        while ( _ii1 != ii1_ )
        { f( *_ii1++, *_ii2++, *_ii3++ ); }

        return std::move( f );
    }

}//namespace feng

#endif//_FOR_EACH_HPP_INCLUDED_SDFOIU498USDFLKIHJR3HFSDLKJHVCKJBNSDFKJHASLKJHSDAFKJHCVIHUJFSDIUHSFDKJHSFDIUHFDSIUHTGIUHFD

