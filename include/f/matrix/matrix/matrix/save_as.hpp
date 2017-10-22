#ifndef SSAVE_AS_HPP_INCLUDED_SDPOJNAS23O9UHAFSLKJAHF9YH4AKJFHASKLFJH39HUAFKJHSF
#define SSAVE_AS_HPP_INCLUDED_SDPOJNAS23O9UHAFSLKJAHF9YH4AKJFHASKLFJH39HUAFKJHSF


namespace f
{
    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_save_as
    {
        typedef Matrix      zen_type;
        typedef Type        value_type;

        bool save_as( const std::string& file_name ) const
        {
#if 0

            if ( file_name.size() < 3 )
            {
                std::string const& new_file_name = file_name + std::string{".txt"}
                                                   return save_as_txt( new_file_name.c_str() );
            }

            std::string const& extension = file_name.substr( file_name.size() - 4 );

            if ( extension == std::string{".bin"} || extension == std::string{".dat"} )
                return save_as_binary( file_name.c_str() );
#endif
            return save_as_txt( file_name.c_str() );
        }

        bool save_as( char const* const file_name ) const
        {
            return save_as( std::string{file_name} );
        }

#if 0
        bool save_as_binary( string const& file_name ) const
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
            auto const c = zen.col();
            ofs.write( &r, sizeof( r ) );
            ofs.write( &c, sizeof( c ) );
            // store data as binary
            ofs.write( reinterpret_cast<char const*>( zen.data() ), sizeof( Type )*zen.size() );
            ofs.close();
        }
#endif

        bool save_as_txt( std::string const& file_name ) const
        {
            return save_as_txt( file_name.c_str() );
        }

        bool save_as_txt( const char* const file_name ) const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            std::ofstream ofs( file_name );

            if ( !ofs )
            {
                return false;
            }

            ofs.precision( 16 );
            ofs << zen;
            ofs.close();
            return true;
        }

        bool save_to( const char* const file_name ) const
        {
            return save_as( file_name );
        }

        bool save_to( const std::string& file_name ) const
        {
            return save_as( file_name.c_str() );
        }

        bool save( const char* const file_name ) const
        {
            return save_as( file_name );
        }

        bool save( const std::string& file_name ) const
        {
            return save_as( file_name.c_str() );
        }

    };//struct crtp_save_as

}

#endif//SSAVE_AS_HPP_INCLUDED_SDPOJNAS23O9UHAFSLKJAHF9YH4AKJFHASKLFJH39HUAFKJHSF

