#ifndef KFAEJYMFTFVDCIPXHGAIUAIFPSUQBFBTPKRNKRBPPXYGBOMLPGAQTGYQPPMVOTEDOWDVGBLUQ
#define KFAEJYMFTFVDCIPXHGAIUAIFPSUQBFBTPKRNKRBPPXYGBOMLPGAQTGYQPPMVOTEDOWDVGBLUQ

namespace f
{
    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_clear
    {
        typedef Matrix                                                 zen_type;
        typedef crtp_typedef<Type, Allocator>                          type_proxy_type;

        void clear()
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            zen.resize( 0, 0 );
        }

    };//struct crtp_clear

}

#endif//KFAEJYMFTFVDCIPXHGAIUAIFPSUQBFBTPKRNKRBPPXYGBOMLPGAQTGYQPPMVOTEDOWDVGBLUQ

