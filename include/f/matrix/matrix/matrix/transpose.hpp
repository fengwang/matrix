#ifndef MTRANSPOSE_HPP_INCLUDED_FD09JI4EP9HIJSAFDLKJASF49HASFDKLJNSADFKLJNSDF9UH
#define MTRANSPOSE_HPP_INCLUDED_FD09JI4EP9HIJSAFDLKJASF49HASFDKLJNSADFKLJNSDF9UH

namespace f
{
    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_transpose
    {
        typedef Matrix                                                          zen_type;
        typedef crtp_typedef<Type, Allocator>                                   type_proxy_type;
        typedef typename type_proxy_type::size_type                             size_type;

        const zen_type transpose() const
        {
            const zen_type& zen = static_cast<zen_type const&>( *this );
            zen_type ans( zen.col(), zen.row() );

            for ( size_type i = 0; i < zen.col(); ++i )
            {
                std::copy( zen.col_begin( i ), zen.col_end( i ), ans.row_begin( i ) );
            }

            return ans;
        }


    };//struct crtp_transpose

}

#endif//_TRANSPOSE_HPP_INCLUDED_FD09JI4EP9HIJSAFDLKJASF49HASFDKLJNSADFKLJNSDF9UH

