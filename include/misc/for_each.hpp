#ifndef _FOR_EACH_HPP_INCLUDED_SDFOIU498USDFLKIHJR3HFSDLKJHVCKJBNSDFKJHASLKJHSDAFKJHCVIHUJFSDIUHSFDKJHSFDIUHFDSIUHTGIUHFD
#define _FOR_EACH_HPP_INCLUDED_SDFOIU498USDFLKIHJR3HFSDLKJHVCKJBNSDFKJHASLKJHSDAFKJHCVIHUJFSDIUHSFDKJHSFDIUHFDSIUHTGIUHFD

namespace feng
{
#if 0
    template<typename II1, typename F>
    F for_each( II1 _ii1, II1 ii1_, F f )
    {
        while ( _ii1 != ii1_ )
        { f( *_ii1++ ); }

        return f;
    }

    template<typename II1, typename II2, typename F>
    F for_each( II1 _ii1, II1 ii1_, II2 _ii2, F f )
    {
        while ( _ii1 != ii1_ )
        { f( *_ii1++, *_ii2++ ); }

        return f;
    }

    template<typename II1, typename II2, typename II3, typename F>
    F for_each( II1 _ii1, II1 ii1_, II2 _ii2, II3 _ii3, F f )
    {
        while ( _ii1 != ii1_ )
        { f( *_ii1++, *_ii2++, *_ii3++ ); }

        return f;
    }

    template<typename II1, typename II2, typename II3, typename II4, typename F>
    F for_each( II1 _ii1, II1 ii1_, II2 _ii2, II3 _ii3, II4 _ii4, F f )
    {
        while ( _ii1 != ii1_ )
        { f( *_ii1++, *_ii2++, *_ii3++, *_ii4++ ); }

        return f;
    }
#endif

namespace for_each_impl_private
{
    template< typename F, typename InputIterator1, typename ... InputIteratorn >
    F _for_each( F f, InputIterator1 begin1, InputIterator1 end1, InputIteratorn ... beginn )
    {
        while ( begin1 != end1 )
            f( *begin1++, *beginn++... );
        return f;
    }

    struct dummy {};

    template< typename S, typename ... T >
    void rotate_then_impl( S s, T ... t )
    {
        rotate_then_impl( t..., s );
    }

    template< typename S, typename ... T>
    void rotate_then_impl( S s, dummy, T ... t )
    {
        _for_each( s, t... );
    }

}//namespace for_each_impl_private

template< typename ... T >
void for_each( T ... t )
{
    static_assert( sizeof ... ( t ) > 2, "for_each requires at least 3 arguments" );
    for_each_impl_private::rotate_then_impl( t..., for_each_impl_private::dummy() );
}


}//namespace feng

#endif//_FOR_EACH_HPP_INCLUDED_SDFOIU498USDFLKIHJR3HFSDLKJHVCKJBNSDFKJHASLKJHSDAFKJHCVIHUJFSDIUHSFDKJHSFDIUHFDSIUHTGIUHFD

