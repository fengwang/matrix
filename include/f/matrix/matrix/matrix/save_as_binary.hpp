#ifndef SAVE_AS_BINARY_HPP_INCLUDED_DSPOIASLKD498UALFKSDJ498HAFSKJHASFU9H4IUHFSD
#define SAVE_AS_BINARY_HPP_INCLUDED_DSPOIASLKD498UALFKSDJ498HAFSKJHASFU9H4IUHFSD

namespace f
{
    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_save_as_binary
    {
        typedef Matrix      zen_type;
        typedef Type        value_type;

        bool save_as_binary( std::string const& file_name ) const
        {
            return save_as_binary( file_name.c_str() );
        }

        bool save_as_binary( char const* const file_name ) const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            std::ofstream ofs( file_name, std::ios::out | std::ios::binary );

            if ( !ofs )
            {
                return false;
            }

            // store row and col
            auto const r = zen.row();
            ofs.write( reinterpret_cast<char const*>( std::addressof( r ) ), sizeof( r ) );
            auto const c = zen.col();
            ofs.write( reinterpret_cast<char const*>( std::addressof( c ) ), sizeof( c ) );
            // store data as binary
            ofs.write( reinterpret_cast<char const*>( zen.data() ), sizeof( Type )*zen.size() );

            if ( !ofs.good() ) return false;

            ofs.close();
            return true;
        }

    };//struct crtp_save_as_binary

}

#endif//SAVE_AS_BINARY_HPP_INCLUDED_DSPOIASLKD498UALFKSDJ498HAFSKJHASFU9H4IUHFSD

