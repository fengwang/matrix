#ifndef MDET_HPP_INCLUDED_SDFPOIHNSAFDLKJH39PUHAFSDKLJANSF3UHASFDKLJH439HU8ASDFK
#define MDET_HPP_INCLUDED_SDFPOIHNSAFDLKJH39PUHAFSDKLJANSF3UHASFDKLJH439HU8ASDFK

namespace f
{
    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_det
    {
        typedef Matrix                                                          zen_type;
        typedef crtp_typedef<Type, Allocator>                                   type_proxy_type;
        typedef typename type_proxy_type::size_type                             size_type;
        typedef typename type_proxy_type::value_type                            value_type;
        typedef typename type_proxy_type::range_type                            range_type;

        value_type det() const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            assert( zen.row() == zen.col() );

            if ( 0 == zen.size() )
            {
                return value_type();
            }

            if ( 1 == zen.size() )
            {
                return *( zen.begin() );
            }

            if ( 4 == zen.size() )
            {
                return zen[0][0] * zen[1][1] - zen[1][0] * zen[0][1];
            }

            size_type const n = zen.row();
            size_type const m = n >> 1;
            zen_type const P( zen, range_type( 0, m ), range_type( 0, m ) );
            zen_type const Q( zen, range_type( 0, m ), range_type( m, n ) );
            zen_type const R( zen, range_type( m, n ), range_type( 0, m ) );
            zen_type const S( zen, range_type( m, n ), range_type( m, n ) );
            zen_type const& tmp = S - ( R * ( P.inverse() ) * Q );
            return P.det() * tmp.det();
        }

    };//struct crtp_det

}

#endif//_DET_HPP_INCLUDED_SDFPOIHNSAFDLKJH39PUHAFSDKLJANSF3UHASFDKLJH439HU8ASDFK

