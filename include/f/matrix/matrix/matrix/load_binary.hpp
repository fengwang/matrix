#ifndef LOAD_BINARY_HPP_INCLUDED_SDOIH4OIUHADSF984YALFKHJ498YAFKHSAF9UHDS987UH4F
#define LOAD_BINARY_HPP_INCLUDED_SDOIH4OIUHADSF984YALFKHJ498YAFKHSAF9UHDS987UH4F

namespace f
{
    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_load_binary
    {
        typedef Matrix                                                 zen_type;
        typedef typename crtp_typedef<Type, Allocator>::value_type     value_type;
        typedef typename crtp_typedef<Type, Allocator>::size_type      size_type;

        bool load_binary( std::string const& file_name )
        {
            return load_binary( file_name.c_str() );
        }

        bool load_binary( char const* const file_name )
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            std::ifstream ifs( file_name, std::ios::binary );
            assert( ifs && "matrix::load_binary -- failed to open file" );

            if ( !ifs ) return false;

            std::vector<char> buffer{( std::istreambuf_iterator<char>( ifs ) ), ( std::istreambuf_iterator<char>() )};
            assert( buffer.size() >= sizeof( size_type ) + sizeof( size_type ) && "matrix::load_library -- file too small, must be damaged" );

            if ( buffer.size() <= sizeof( size_type ) + sizeof( size_type ) ) return false;

            size_type r;
            std::copy( buffer.begin(), buffer.begin() + sizeof( r ), reinterpret_cast<char*>( std::addressof( r ) ) );
            size_type c;
            std::copy( buffer.begin() + sizeof( r ), buffer.begin() + sizeof( r ) + sizeof( c ), reinterpret_cast<char*>( std::addressof( c ) ) );
            zen.resize( r, c );
            assert( buffer.size() == sizeof( r ) + sizeof( c ) + sizeof( Type )*zen.size() && "matrix::load_binary -- data does not match, file damaged" );

            if ( buffer.size() != sizeof( r ) + sizeof( c ) + sizeof( Type )*zen.size() ) return false;

            std::copy( buffer.begin() + sizeof( r ) + sizeof( c ), buffer.end(), reinterpret_cast<char*>( zen.data() ) );
            return true;
        }

    };//struct crtp_load_binary

}

#endif//LOAD_BINARY_HPP_INCLUDED_SDOIH4OIUHADSF984YALFKHJ498YAFKHSAF9UHDS987UH4F

