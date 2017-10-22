#ifndef MSWAP_HPP_INCLUDED_SDFPONASFDLKJH4HUASFKLJH498HUAFLKHJ498HAKLJNV98YH4FAD
#define MSWAP_HPP_INCLUDED_SDFPONASFDLKJH4HUASFKLJH498HUAFLKHJ498HAKLJNV98YH4FAD

namespace f
{
    template<typename Matrix, typename T, typename A>
    struct crtp_swap
    {
        typedef Matrix  zen_type;

        void swap( zen_type& other )
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            std::swap( zen.row_, other.row_ );
            std::swap( zen.col_, other.col_ );
            zen.data_.swap( other.data_ );
        }

    };//struct crtp_swap
}//namespace f

#endif//_SWAP_HPP_INCLUDED_SDFPONASFDLKJH4HUASFKLJH498HUAFLKHJ498HAKLJNV98YH4FAD

