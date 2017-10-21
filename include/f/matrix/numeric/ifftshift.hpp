#ifndef XCWXWHCBTHDOMMKNICEYGROOCIGLLYIQAYVKIUYNYRHIXUGFLKPMEDRHCWBMYGBGILHENPGCP
#define XCWXWHCBTHDOMMKNICEYGROOCIGLLYIQAYVKIUYNYRHIXUGFLKPMEDRHCWBMYGBGILHENPGCP

#include "./ifft.hpp"

namespace f
{

    template< typename T >
    auto ifftshift( matrix<T> const& x )
    {
        auto X = ifft( x );
        unsigned long const R = X.row();
        unsigned long const C = X.col();
        unsigned long const row_starter = ( R >> 1 ) + ( R & 1 );

        for ( unsigned long index = 0; row_starter + index < R; ++index )
            std::swap_ranges( X.row_begin( index ), X.row_end( index ), X.row_begin( row_starter + index ) );

        unsigned long const col_starter = ( C >> 1 ) + ( C & 1 );

        for ( unsigned long index = 0; col_starter + index < C; ++index )
            std::swap_ranges( X.col_begin( index ), X.col_end( index ), X.col_begin( col_starter + index ) );

        return X;
    }

}//namespace f

#endif//XCWXWHCBTHDOMMKNICEYGROOCIGLLYIQAYVKIUYNYRHIXUGFLKPMEDRHCWBMYGBGILHENPGCP
