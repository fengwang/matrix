#ifndef CUUHUDKNNGBOIUAVQOWMWYFBRQMOMMLHSJVLYJEUPURBFBDYKNHLOWSTBLLJDAJRPGWGOXRQD
#define CUUHUDKNNGBOIUAVQOWMWYFBRQMOMMLHSJVLYJEUPURBFBDYKNHLOWSTBLLJDAJRPGWGOXRQD

namespace f
{
    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_save_as_full_bmp
    {
        typedef Matrix      zen_type;

        bool save_as_full_bmp( const std::string& file_name ) const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            std::string new_file_name{ file_name };
            std::string const extension{ ".bmp" };

            if ( ( new_file_name.size() < 4 )  || ( std::string{ new_file_name.begin() + new_file_name.size() - 4, new_file_name.end() } != extension ) )
                new_file_name += extension;
            std::ofstream stream( new_file_name.c_str(), std::ios_base::out | std::ios_base::binary );

            if ( !stream )
            {
                return false;
            }

            unsigned char file[14] =
            {
                'B', 'M', // magic
                0, 0, 0, 0, // size in bytes
                0, 0, // app data
                0, 0, // app data
                54, 0, 0, 0 // start of data offset
            };
            unsigned char info[40] =
            {
                40, 0, 0, 0, // info hd size
                0, 0, 0, 0, // width
                0, 0, 0, 0, // heigth
                1, 0, // number color planes
                24, 0, // bits per pixel
                0, 0, 0, 0, // compression is none
                0, 0, 0, 0, // image bits size
                0x13, 0x0B, 0, 0, // horz resoluition in pixel / m
                0x13, 0x0B, 0, 0, // vert resolutions (0x03C3 = 96 dpi, 0x0B13 = 72 dpi)
                0, 0, 0, 0, // #colors in pallete
                0, 0, 0, 0, // #important colors
            };
            unsigned long const the_col = zen.col();
            unsigned long const the_row = zen.row();
            unsigned long const pad_size  = ( 4 - ( the_col * 3 ) % 4 ) % 4;
            unsigned long const data_size = the_col * the_row * 3 + the_row * pad_size;
            unsigned long const all_size  = data_size + sizeof( file ) + sizeof( info );
            auto const& ul_to_uc = []( unsigned long val )
            {
                return static_cast<unsigned char>( val & 0xffUL );
            };
            auto const& db_to_ul = []( double val, double ac, double dc )
            {
                return static_cast<unsigned long>( std::floor( ( static_cast<double>( 0xFFFFFF ) + 0.9 ) * ( val * ac + dc ) ) );
            };
            file[ 2] = ul_to_uc( all_size       );
            file[ 3] = ul_to_uc( all_size >> 8  );
            file[ 4] = ul_to_uc( all_size >> 16 );
            file[ 5] = ul_to_uc( all_size >> 24 );
            info[ 4] = ul_to_uc( the_col       );
            info[ 5] = ul_to_uc( the_col >> 8  );
            info[ 6] = ul_to_uc( the_col >> 16 );
            info[ 7] = ul_to_uc( the_col >> 24 );
            info[ 8] = ul_to_uc( the_row       );
            info[ 9] = ul_to_uc( the_row >> 8  );
            info[10] = ul_to_uc( the_row >> 16 );
            info[11] = ul_to_uc( the_row >> 24 );
            info[20] = ul_to_uc( data_size       );
            info[21] = ul_to_uc( data_size >> 8  );
            info[22] = ul_to_uc( data_size >> 16 );
            info[23] = ul_to_uc( data_size >> 24 );
            stream.write( reinterpret_cast<char*>( file ), sizeof( file ) );
            stream.write( reinterpret_cast<char*>( info ), sizeof( info ) );
            unsigned char pad[3] = {0, 0, 0};
            unsigned char pixel[3];
            double const max_val = static_cast<double>( *std::max_element( zen.begin(), zen.end() ) );
            double const min_val = static_cast<double>( *std::min_element( zen.begin(), zen.end() ) );
            double const ac_factor = 1.0 / ( max_val - min_val + 1.0e-40 );
            double const dc_factor = min_val / ( min_val - max_val - 1.0e-40 );

            for ( unsigned long r = 0; r < the_row; r++ )
            {
                for ( unsigned long c = 0; c < the_col; c++ )
                {
                    unsigned long const index = db_to_ul( zen[r][c], ac_factor, dc_factor );
                    pixel[0] = index >> 16;
                    pixel[1] = ( index >> 8 ) & 0xFF;
                    pixel[2] = index & 0xFF;
                    stream.write( reinterpret_cast<char*>( pixel ), 3 );
                }

                stream.write( reinterpret_cast<char*>( pad ), pad_size );
            }

            stream.close();
            return true;
        }

        bool save_as_full_bmp( const char* const file_name ) const
        {
            return save_as_full_bmp( std::string{file_name} );
        }

    };//struct crtp_save_as_full_bmp

}

#endif//CUUHUDKNNGBOIUAVQOWMWYFBRQMOMMLHSJVLYJEUPURBFBDYKNHLOWSTBLLJDAJRPGWGOXRQD

