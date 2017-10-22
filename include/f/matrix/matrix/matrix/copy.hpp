#ifndef MCOPY_HPP_INCLUDED_DPIHASDLKJDSA98HE3AISFDH8H3E9HI88888888888888888DF3DZ
#define MCOPY_HPP_INCLUDED_DPIHASDLKJDSA98HE3AISFDH8H3E9HI88888888888888888DF3DZ

namespace f
{
    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_copy
    {
        typedef Matrix                                                 zen_type;
        typedef crtp_typedef<Type, Allocator>                          type_proxy_type;

        template<typename Other_Matrix>
        void copy( const Other_Matrix& rhs )
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            zen.row_ = rhs.row();
            zen.col_ = rhs.col();
            zen.data_.assign( rhs.begin(), rhs.end() );
        }

    };//struct

}

#endif//_COPY_HPP_INCLUDED_DPIHASDLKJDSA98HE3AISFDH8H3E9HI88888888888888888DF3DZ
