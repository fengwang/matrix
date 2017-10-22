#ifndef LLOAD_HPP_INCLUDED_SDPONASD329P8HASFLKDJH498YHSDAKH39H8SFDIUH439HF39HFDF
#define LLOAD_HPP_INCLUDED_SDPONASD329P8HASFLKDJH498YHSDAKH39H8SFDIUH439HF39HFDF

namespace f
{
    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_load
    {
        typedef Matrix                                                          zen_type;
        typedef typename crtp_typedef<Type, Allocator>::value_type     value_type;
        typedef typename crtp_typedef<Type, Allocator>::size_type      size_type;

#if 0
        bool load_binary( char const* const file_name )
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            std::ifstream ifs( file_name, std::ios::binary );
            assert( ifs && "matrix::load_binary -- failed to open file" );
            std::vector<char> buffer{( std::istreambuf_iterator<char>( ifs ) ), ( std::istreambuf_iterator<char>() )};
            size_type r;
            std::copy( buffer.begin(), buffer.begin() + sizeof( r ), reinterpret_cast<char*>( std::addressof( r ) ) );
            size_type c;
            std::copy( buffer.begin() + sizeof( r ), buffer.begin() + sizeof( r ) + sizeof( c ), reinterpret_cast<char*>( std::addressof( c ) ) );
            zen.resize( r, c );
            assert( buffer.size() == sizeof( r ) + sizeof( c ) + sizeof( Type )*zen.size() && "matrix::load_binary -- file damaged" );
            std::copy( buffer.begin() + sizeof( r ) + sizeof( c ), buffer.end(), reinterpret_cast<char*>( zen.data() ) );
        }
#endif

        bool load( const char* const file_name )
        {
            /*
             * TODO:
             * 1) trim right of file name
             * 2) if file name with '.mat' extension
             *        call load_mat
             * 3) else
             *        call load_ascii
             */
            return load_ascii( file_name );
        }

        bool load( const std::string& file_name )
        {
            return load( file_name.c_str() );
        }

        bool load_from( const char* const file_name )
        {
            return load( file_name );
        }

        bool load_from( const std::string& file_name )
        {
            return load( file_name.c_str() );
        }

        bool load_ascii( const char* const file_name )
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            //std::ifstream ifs( file_name,  std::ios::in | std::ios::binary );
            std::ifstream ifs( file_name );
            assert( ifs && "matrix::load_ascii -- failed to open file" );
            //read the file content into a string stream
            std::stringstream iss;
            std::copy( std::istreambuf_iterator<char>( ifs ), std::istreambuf_iterator<char>(), std::ostreambuf_iterator<char>( iss ) );
#if 0
            const std::string& stream_buff = iss.str();
            // counting the row
            size_type const r_ = std::count( stream_buff.begin(), stream_buff.end(), '\n' );
            size_type const r  = *( stream_buff.rbegin() ) == '\n' ? r_ : r_ + 1;
            // counting the column
            //// find first '\n', if not found, return last itor
            auto space_itor = std::find( stream_buff.begin(), stream_buff.end(), '\n' );

            while ( std::isspace( *--space_itor ) ); //reverse find first non-space charactor

            size_type const c = std::count( stream_buff.begin(), space_itor, '\t' ) + 1; // or space?
            size_type const total_elements = r * c;
            std::vector<value_type> buff;
            buff.reserve( total_elements );
            std::copy( std::istream_iterator<value_type>( iss ), std::istream_iterator<value_type>(), std::back_inserter( buff ) );

            if ( buff.size() != total_elements )
            {
                std::cerr << "Error: Failed to match matrix size.\n \tthe size of matrix stored in file \"" << file_name << "\" is " << buff.size() << ".\n";
                std::cerr << " \tthe size of the destination matrix is " << r << " by " << c << " i.e. " << total_elements << " elements.\n";
                return false;
            }

#endif
#if 1
            std::string cache = iss.str();
            std::for_each( cache.begin(), cache.end(), []( auto & ch )
            {
                if ( ch == ',' || ch == ';' ) ch = ' ';
            } );
            iss.str( cache );
            std::vector<value_type> buff;
            std::copy( std::istream_iterator<value_type>( iss ), std::istream_iterator<value_type>(), std::back_inserter( buff ) );
            size_type const total_elements = buff.size();
            const std::string& stream_buff = iss.str();
            // counting the row
            size_type const r_ = std::count( stream_buff.begin(), stream_buff.end(), '\n' );
            size_type const r  = *( stream_buff.rbegin() ) == '\n' ? r_ : r_ + 1;
            size_type const c = total_elements / r;

            if ( r * c != total_elements )
            {
                std::cerr << "Error: Failed to match matrix size.\n \tthe size of matrix stored in file \"" << file_name << "\" is " << buff.size() << ".\n";
                std::cerr << " \tthe size of the destination matrix is " << r << " by " << c << " elements.\n";
                std::cerr << " \tthe buff is:\n";
                std::copy( buff.begin(), buff.end(), std::ostream_iterator<value_type>( std::cerr, "\t" ) );
                std::cerr << "\n";
                std::cerr << " \tthe stream_buffer is:\n" << stream_buff << "\n";
                return false;
            }

#endif
            zen.resize( r, c );
            std::copy( buff.begin(), buff.end(), zen.begin() );
            ifs.close();
            return true;
        }

        //TODO: read matlab file format and impl here
        bool load_mat( const char* const /*file_name*/ )
        {
            return true;
        }

    };//struct crtp_load

}

#endif//_LOAD_HPP_INCLUDED_SDPONASD329P8HASFLKDJH498YHSDAKH39H8SFDIUH439HF39HFDF

