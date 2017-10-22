#ifndef MCLONE_HPP_INCLUDED_DFSPJ3POIJHAF98YH4KLJBNFVKLJAFD984YHKJJHSFAD9IUH4FSD
#define MCLONE_HPP_INCLUDED_DFSPJ3POIJHAF98YH4KLJBNFVKLJAFD984YHKJJHSFAD9IUH4FSD

namespace f
{
    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_clone
    {
        typedef Matrix                                                          zen_type;
        typedef crtp_typedef<Type, Allocator>                          type_proxy_type;
        typedef typename type_proxy_type::size_type                             size_type;

        //TODO: Matrix concept check
        template<typename Other_Matrix>
        zen_type& clone( const Other_Matrix& other, size_type const r0, size_type const r1, size_type const c0, size_type const c1 )
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            assert( r1 > r0 );
            assert( c1 > c0 );
            zen.resize( r1 - r0, c1 - c0 );

            if ( zen.row() < zen.col() )
                for ( size_type i = r0; i != r1; ++i )
                    std::copy( other.row_begin( i ) + c0, other.row_begin( i ) + c1, zen.row_begin( i - r0 ) );
            else
                for ( size_type i = c0; i != c1; ++i )
                    std::copy( other.col_begin( i ) + r0, other.col_begin( i ) + r1, zen.col_begin( i - c0 ) );

            return zen;
        }

    };//struct

}

#endif//_CLONE_HPP_INCLUDED_DFSPJ3POIJHAF98YH4KLJBNFVKLJAFD984YHKJJHSFAD9IUH4FSD

