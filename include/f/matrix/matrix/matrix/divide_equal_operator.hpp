#ifndef MDIVIDE_EQUAL_OPERATOR_HPP_INCLUDED_SFDPOJISAFLIJ4089UAJSFO4J0YU8ASFDIOU
#define MDIVIDE_EQUAL_OPERATOR_HPP_INCLUDED_SFDPOJISAFLIJ4089UAJSFO4J0YU8ASFDIOU

namespace f
{
    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_divide_equal_operator
    {
        typedef Matrix                                                          zen_type;
        typedef crtp_typedef<Type, Allocator>                                   type_proxy_type;
        typedef typename type_proxy_type::value_type                            value_type;

        zen_type& operator /=( const value_type& rhs )
        {
            zen_type& zen = static_cast<zen_type&>( *this );

            //std::transform( zen.begin(), zen.end(), zen.begin(), std::bind2nd( std::divides<value_type>(), rhs ) );
            for ( auto& v : zen )
                v /= rhs;

            return zen;
        }

        zen_type& operator /=( const zen_type& rhs )
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            zen *= rhs.inverse();
            return zen;
        }

    };//struct

}

#endif//_DIVIDE_EQUAL_OPERATOR_HPP_INCLUDED_SFDPOJISAFLIJ4089UAJSFO4J0YU8ASFDIOU

