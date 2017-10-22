#ifndef CIJNPRYULAKCWVCKLDQARWMCTQPYMVBRWSAWBFKCXRJSMELDUWCULEBJKQMSPSKJBSSEJWXHH
#define CIJNPRYULAKCWVCKLDQARWMCTQPYMVBRWSAWBFKCXRJSMELDUWCULEBJKQMSPSKJBSSEJWXHH

namespace f
{
    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_save_as_inverse_bmp
    {
        typedef Matrix      zen_type;
#if 0
        bool save_as_inverse_bmp( const std::string& file_name ) const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            std::string new_file_name{ file_name };
            std::string const extension{ ".bmp" };

            if ( ( new_file_name.size() < 4 )  || ( std::string{ new_file_name.begin() + new_file_name.size() - 4, new_file_name.end() } != extension ) )
                new_file_name += extension;
            //std::ofstream stream( new_file_name.c_str(), std::ios_base::out | std::ios_base::binary );
            auto stream = fopen( new_file_name.c_str(), "wb" );

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
            //auto const& db_to_ul = []( double val, double ac, double dc ){ return static_cast<unsigned long>( std::floor( 500.0 * (val*ac+dc) ) ); };
            auto const& db_to_ul = []( double val, double ac, double dc )
            {
                return static_cast<unsigned long>( std::floor( 765.9 * ( val * ac + dc ) ) );
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

            //stream.write( reinterpret_cast<char*>( file ), sizeof( file ) );
            //stream.write( reinterpret_cast<char*>( info ), sizeof( info ) );

            for ( unsigned char ch : file )
                fprintf( stream, "%c", ch );

            for ( unsigned char ch : info )
                fprintf( stream, "%c", ch );

            //unsigned char pad[3] = {0, 0, 0};
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
                    pixel[2] = blue_hot[index][0];
                    pixel[1] = blue_hot[index][1];
                    pixel[0] = blue_hot[index][2];

                    //stream.write( reinterpret_cast<char*>( pixel ), 3 );
                    for ( unsigned char ch : pixel )
                        fprintf( stream, "%c", ch );
                }

                //stream.write( reinterpret_cast<char*>( pad ), pad_size );
                for ( unsigned long index = 0; index != pad_size; ++index )
                    fprintf( stream, "%c", static_cast<unsigned char>( 0 ) );
            }

            //stream.close();
            fclose( stream );
            return true;
        }
#else

        bool save_as_inverse_bmp( const std::string& file_name ) const
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
            //auto const& db_to_ul = []( double val, double ac, double dc ){ return static_cast<unsigned long>( std::floor( 500.0 * (val*ac+dc) ) ); };
            auto const& db_to_ul = []( double val, double ac, double dc )
            {
                return static_cast<unsigned long>( std::floor( 765.9 * ( val * ac + dc ) ) );
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

            /*
            std::cout << "\nmax_val is " << max_val;
            std::cout << "\nmin_val is " << min_val;
            std::cout << "\nac_factor is " << ac_factor;
            std::cout << "\ndc_factor is " << dc_factor;
            */

            for ( unsigned long r = 0; r < the_row; r++ )
            {
                //std::cout << "Preparing to write row " << r << std::endl;
                for ( unsigned long c = 0; c < the_col; c++ )
                {
                    //unsigned long const index = db_to_ul( zen[r][c], ac_factor, dc_factor );
                    unsigned long const index = 765 - db_to_ul( zen[r][c], ac_factor, dc_factor );
                    pixel[2] = blue_hot[index][0];
                    pixel[1] = blue_hot[index][1];
                    pixel[0] = blue_hot[index][2];
                    /*
                    if ( r == 51 )
                    {
                        std::cout << "\nindex - " << index << "\t";
                        std::cout << "c - " << c << "\t";
                        std::cout << "elem - " << zen[r][c] << "\t";
                        std::cout << "pixel - " << static_cast<unsigned int>(pixel[2]) << "\t";
                        std::cout << " - " << static_cast<unsigned int>(pixel[1]) << "\t";
                        std::cout << " - " << static_cast<unsigned int>(pixel[0]) << "\n";
                        std::cout << std::endl;
                    }
                    */
                    stream.write( reinterpret_cast<char*>( pixel ), 3 );
                }

                stream.write( reinterpret_cast<char*>( pad ), pad_size );
            }

            stream.close();
            return true;
        }
#endif
        bool save_as_inverse_bmp( const char* const file_name ) const
        {
            return save_as_inverse_bmp( std::string{file_name} );
        }

        const unsigned char blue_hot[766][3] =
        {
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 0 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 1 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 2 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 3 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 4 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 5 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 6 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 7 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 8 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 9 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 10 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 11 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 12 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 13 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 14 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 15 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 16 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 17 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 18 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 19 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 20 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 21 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 22 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 23 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 24 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 25 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 26 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 27 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 28 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 29 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 30 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 31 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 32 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 33 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 34 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 35 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 36 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 37 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 38 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 39 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 40 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 41 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 42 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 43 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 44 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 45 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 46 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 47 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 48 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 49 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 50 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 51 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 52 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 53 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 54 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 55 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 56 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 57 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 58 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 59 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 60 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 61 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 62 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 63 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 64 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 65 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 66 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 67 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 68 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 69 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 70 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 71 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 72 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 73 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 74 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 75 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 76 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 77 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 78 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 79 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 80 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 81 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 82 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 83 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 84 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 85 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 86 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 87 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 88 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 89 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 90 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 91 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 92 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 93 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 94 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 95 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 96 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 97 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 98 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 99 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 100 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 101 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 102 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 103 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 104 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 105 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 106 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 107 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 108 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 109 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 110 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 111 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 112 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 113 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 114 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 115 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 116 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 117 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 118 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 119 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 120 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 121 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 122 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 123 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 124 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 125 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 126 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 127 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 128 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 129 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 130 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 131 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 132 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 133 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 134 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 135 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 136 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 137 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 138 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 139 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 140 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 141 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 142 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 143 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 144 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 145 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 146 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 147 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 148 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 149 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 150 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 151 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 152 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 153 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 154 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 155 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 156 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 157 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 158 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 159 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 160 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 161 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 162 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 163 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 164 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 165 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 166 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 167 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 168 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 169 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 170 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 171 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 172 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 173 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 174 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 175 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 176 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 177 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 178 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 179 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 180 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 181 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 182 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 183 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 184 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 185 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 186 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 187 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 188 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 189 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 190 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 191 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 192 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 193 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 194 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 195 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 196 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 197 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 198 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 199 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 200 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 201 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 202 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 203 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 204 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 205 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 206 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 207 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 208 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 209 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 210 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 211 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 212 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 213 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 214 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 215 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 216 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 217 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 218 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 219 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 220 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 221 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 222 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 223 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 224 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 225 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 226 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 227 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 228 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 229 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 230 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 231 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 232 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 233 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 234 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 235 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 236 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 237 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 238 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 239 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 240 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 241 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 242 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 243 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 244 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 245 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 246 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 247 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 248 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 249 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 250 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 251 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 252 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 253 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 254 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 0 ),  static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 1 ),  static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 2 ),  static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 3 ),  static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 4 ),  static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 5 ),  static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 6 ),  static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 7 ),  static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 8 ),  static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 9 ),  static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 10 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 11 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 12 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 13 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 14 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 15 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 16 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 17 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 18 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 19 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 20 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 21 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 22 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 23 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 24 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 25 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 26 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 27 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 28 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 29 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 30 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 31 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 32 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 33 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 34 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 35 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 36 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 37 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 38 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 39 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 40 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 41 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 42 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 43 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 44 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 45 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 46 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 47 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 48 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 49 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 50 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 51 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 52 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 53 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 54 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 55 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 56 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 57 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 58 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 59 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 60 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 61 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 62 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 63 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 64 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 65 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 66 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 67 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 68 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 69 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 70 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 71 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 72 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 73 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 74 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 75 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 76 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 77 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 78 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 79 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 80 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 81 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 82 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 83 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 84 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 85 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 86 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 87 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 88 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 89 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 90 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 91 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 92 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 93 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 94 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 95 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 96 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 97 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 98 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 99 ), static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 100 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 101 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 102 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 103 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 104 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 105 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 106 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 107 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 108 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 109 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 110 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 111 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 112 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 113 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 114 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 115 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 116 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 117 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 118 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 119 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 120 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 121 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 122 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 123 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 124 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 125 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 126 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 127 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 128 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 129 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 130 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 131 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 132 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 133 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 134 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 135 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 136 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 137 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 138 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 139 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 140 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 141 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 142 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 143 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 144 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 145 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 146 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 147 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 148 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 149 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 150 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 151 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 152 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 153 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 154 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 155 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 156 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 157 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 158 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 159 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 160 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 161 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 162 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 163 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 164 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 165 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 166 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 167 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 168 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 169 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 170 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 171 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 172 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 173 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 174 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 175 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 176 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 177 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 178 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 179 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 180 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 181 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 182 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 183 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 184 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 185 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 186 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 187 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 188 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 189 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 190 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 191 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 192 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 193 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 194 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 195 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 196 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 197 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 198 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 199 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 200 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 201 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 202 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 203 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 204 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 205 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 206 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 207 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 208 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 209 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 210 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 211 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 212 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 213 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 214 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 215 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 216 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 217 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 218 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 219 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 220 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 221 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 222 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 223 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 224 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 225 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 226 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 227 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 228 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 229 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 230 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 231 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 232 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 233 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 234 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 235 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 236 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 237 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 238 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 239 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 240 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 241 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 242 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 243 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 244 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 245 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 246 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 247 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 248 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 249 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 250 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 251 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 252 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 253 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 254 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 0 ),    static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 1 ),    static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 2 ),    static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 3 ),    static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 4 ),    static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 5 ),    static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 6 ),    static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 7 ),    static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 8 ),    static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 9 ),    static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 10 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 11 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 12 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 13 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 14 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 15 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 16 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 17 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 18 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 19 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 20 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 21 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 22 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 23 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 24 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 25 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 26 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 27 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 28 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 29 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 30 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 31 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 32 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 33 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 34 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 35 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 36 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 37 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 38 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 39 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 40 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 41 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 42 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 43 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 44 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 45 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 46 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 47 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 48 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 49 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 50 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 51 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 52 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 53 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 54 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 55 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 56 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 57 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 58 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 59 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 60 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 61 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 62 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 63 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 64 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 65 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 66 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 67 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 68 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 69 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 70 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 71 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 72 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 73 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 74 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 75 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 76 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 77 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 78 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 79 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 80 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 81 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 82 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 83 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 84 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 85 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 86 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 87 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 88 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 89 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 90 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 91 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 92 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 93 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 94 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 95 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 96 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 97 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 98 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 99 ),   static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 100 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 101 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 102 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 103 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 104 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 105 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 106 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 107 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 108 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 109 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 110 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 111 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 112 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 113 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 114 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 115 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 116 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 117 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 118 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 119 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 120 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 121 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 122 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 123 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 124 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 125 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 126 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 127 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 128 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 129 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 130 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 131 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 132 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 133 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 134 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 135 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 136 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 137 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 138 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 139 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 140 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 141 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 142 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 143 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 144 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 145 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 146 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 147 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 148 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 149 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 150 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 151 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 152 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 153 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 154 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 155 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 156 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 157 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 158 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 159 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 160 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 161 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 162 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 163 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 164 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 165 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 166 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 167 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 168 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 169 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 170 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 171 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 172 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 173 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 174 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 175 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 176 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 177 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 178 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 179 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 180 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 181 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 182 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 183 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 184 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 185 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 186 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 187 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 188 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 189 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 190 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 191 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 192 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 193 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 194 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 195 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 196 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 197 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 198 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 199 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 200 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 201 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 202 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 203 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 204 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 205 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 206 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 207 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 208 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 209 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 210 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 211 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 212 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 213 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 214 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 215 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 216 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 217 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 218 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 219 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 220 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 221 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 222 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 223 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 224 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 225 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 226 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 227 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 228 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 229 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 230 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 231 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 232 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 233 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 234 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 235 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 236 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 237 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 238 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 239 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 240 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 241 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 242 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 243 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 244 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 245 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 246 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 247 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 248 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 249 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 250 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 251 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 252 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 253 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 254 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) },
            { static_cast<unsigned char>( 255 ),  static_cast<unsigned char>( 255 ),    static_cast<unsigned char>( 255 ) }
        };

    };//struct crtp_save_as_inverse_bmp

}

#endif//CIJNPRYULAKCWVCKLDQARWMCTQPYMVBRWSAWBFKCXRJSMELDUWCULEBJKQMSPSKJBSSEJWXHH

