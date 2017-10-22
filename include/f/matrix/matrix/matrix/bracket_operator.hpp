#ifndef MBRACKET_OPERATOR_HPP_INCLUDED_SDOIJASF983AFLJKVCMNASKLJHA9H834AKLJSFFDE
#define MBRACKET_OPERATOR_HPP_INCLUDED_SDOIJASF983AFLJKVCMNASKLJHA9H834AKLJSFFDE

namespace f
{
    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_bracket_operator
    {
        typedef Matrix                                                          zen_type;
        typedef crtp_typedef<Type, Allocator>                                   type_proxy_type;
        typedef typename type_proxy_type::value_type                            value_type;
        typedef typename type_proxy_type::size_type                             size_type;
        typedef typename type_proxy_type::row_type                              row_type;
        typedef typename type_proxy_type::const_row_type                        const_row_type;

        row_type operator[]( const size_type index )
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            assert( index < zen.row() && "Row index outof boundary!" );
            return zen.row_begin( index );
        }

        const_row_type operator[]( const size_type index ) const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            assert( index < zen.row() && "Row index outof boundary!" );
            return zen.row_begin( index );
        }
        /*
                value_type& operator()( const size_type r = 0, const size_type c = 0 )
                {
                    zen_type& zen = static_cast<zen_type&>( *this );
                    return *( zen.row_begin( r ) + c );
                }

                value_type operator()( const size_type r = 0, const size_type c = 0 ) const
                {
                    zen_type const& zen = static_cast<zen_type const&>( *this );
                    return *( zen.row_cbegin( r ) + c );
                }
        */
    };//struct crtp_typedef

}

#endif//_BRACKET_OPERATOR_HPP_INCLUDED_SDOIJASF983AFLJKVCMNASKLJHA9H834AKLJSFFDE

