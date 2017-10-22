#ifndef MMINUS_EQUAL_OPERATOR_HPP_INCLUDED_SDPOIJHASLKJSDLSFD9H4LHASF98Y4VKJBFDI
#define MMINUS_EQUAL_OPERATOR_HPP_INCLUDED_SDPOIJHASLKJSDLSFD9H4LHASF98Y4VKJBFDI

namespace f
{
    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_minus_equal_operator
    {
        typedef Matrix                                                          zen_type;
        typedef crtp_typedef<Type, Allocator>                          type_proxy_type;
        typedef typename type_proxy_type::value_type                            value_type;

        zen_type& operator -=( const value_type& rhs )
        {
            zen_type& zen = static_cast<zen_type&>( *this );

            //std::transform( zen.begin(), zen.end(), zen.begin(), std::bind2nd( std::minus<value_type>(), rhs ) );
            for ( auto& v : zen )
                v -= rhs;

            return zen;
        }

        zen_type& operator -=( const zen_type& rhs )
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            std::transform( zen.begin(), zen.end(), rhs.begin(), zen.begin(), std::minus<value_type>() );
            return zen;
        }

    };//struct crtp_minus_equal_operator

}

#endif//_MINUS_EQUAL_OPERATOR_HPP_INCLUDED_SDPOIJHASLKJSDLSFD9H4LHASF98Y4VKJBFDI

