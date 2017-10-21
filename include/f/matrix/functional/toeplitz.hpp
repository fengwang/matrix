#ifndef MTOEPLITZ_HPP_INCLUDED_DSOIFJ4ID9UIF498OSDIFAKLSFDJVNVCXNKSFDJAHSFDAOI4HOSDIFT4UISFDIOU489YJ
#define MTOEPLITZ_HPP_INCLUDED_DSOIFJ4ID9UIF498OSDIFAKLSFDJVNVCXNKSFDJAHSFDAOI4HOSDIFT4UISFDIOU489YJ


namespace f
{
    template<typename Itor1,
             typename Itor2,

             typename A = std::allocator<typename remove_const<typename remove_reference<typename std::iterator_traits<Itor1>::value_type>::result_type>::result_type>>
    matrix<typename std::iterator_traits<Itor1>::value_type, D, A> const toeplitz( Itor1 i1_, Itor1 _i1, Itor2 i2_, Itor2 _i2 )
    {
        std::size_t r = std::distance( i1_, _i1 );
        std::size_t c = std::distance( i2_, _i2 );
        matrix<typename std::iterator_traits<Itor1>::value_type, D, A> m( r, c );

        for ( std::size_t i = 0; i != r; ++i )
            std::fill( m.lower_diag_begin( i ), m.lower_diag_end( i ), *( i1_ + i ) );

        for ( std::size_t i = 1; i != c; ++i )
            std::fill( m.upper_diag_begin( i ), m.upper_diag_end( i ), *( i2_ + i ) );

        return m;
    }

    template<typename Itor,

             typename A = std::allocator<typename remove_const<typename remove_reference<typename std::iterator_traits<Itor>::value_type>::result_type>::result_type>>
    matrix<typename std::iterator_traits<Itor>::value_type, D, A> const toeplitz( Itor i_, Itor _i )
    {
        return toeplitz( i_, _i, i_, _i );
    }

}//namespace f

#endif//_TOEPLITZ_HPP_INCLUDED_DSOIFJ4ID9UIF498OSDIFAKLSFDJVNVCXNKSFDJAHSFDAOI4HOSDIFT4UISFDIOU489YJ

