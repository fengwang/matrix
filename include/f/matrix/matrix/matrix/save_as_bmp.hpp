#ifndef BRSNNVBLOHMSYMQEJFQMGFFHKMLYMOTSQLUYLKPEDSHPWMQNMIKCGAVHAGSOKPVKDTLIODNUS
#define BRSNNVBLOHMSYMQEJFQMGFFHKMLYMOTSQLUYLKPEDSHPWMQNMIKCGAVHAGSOKPVKDTLIODNUS

namespace f
{

    namespace crtp_save_as_bmp_private
    {

        typedef std::function<double( double )> converter_type;
        typedef std::function<converter_type( double, double )> transform_value_type;

        //typedef std::function<std::function<double(double)>(double, double)> transform_value_type;

        static const std::map< std::string, transform_value_type > transforms
        {
            std::make_pair( std::string{"default" }, transform_value_type{[]( double mx, double mn )
                {
                    return converter_type{ [ = ]( double v )
                        {
                            return ( v - mn ) / ( mx - mn ) + 1.0e-10;
                        }};
                }} ),
            std::make_pair( std::string{"log" }, transform_value_type{[]( double mx, double mn )
            {
                return converter_type{ [ = ]( double v )
                {
                    return std::log( 1.0 + ( v - mn ) / ( mx - mn ) + 1.0e-10 );
                }};
            }} ),
            std::make_pair( std::string{"log1" }, transform_value_type{[]( double mx, double mn )
            {
                return converter_type{ [ = ]( double v )
                {
                    return std::log( 1.0 + ( v - mn ) / ( mx - mn ) * 10.0 + 1.0e-10 );
                }};
            }} ),
            //std::make_pair( std::string{"logx" }, transform_value_type{[]( double mx, double mn ) { return converter_type{ [=]( double v ) { return std::log(1.0+(v-mn)/(mx-mn)*2.71828182845904523536+1.0e-10); }}; }} ),
            std::make_pair( std::string{"logpi" }, transform_value_type{[]( double mx, double mn )
            {
                return converter_type{ [ = ]( double v )
                {
                    return std::log( 1.0 + ( v - mn ) / ( mx - mn ) * 3.14159265358979323846 + 1.0e-10 );
                }};
            }} ),
            std::make_pair( std::string{"logx" }, transform_value_type{[]( double mx, double mn )
            {
                return converter_type{ [ = ]( double v )
                {
                    return std::log( 1.0 + ( v - mn ) / ( mx - mn ) * 1.64872127070012814685 + 1.0e-10 );
                }};
            }} ),
            std::make_pair( std::string{"log_1" }, transform_value_type{[]( double mx, double mn )
            {
                return converter_type{ [ = ]( double v )
                {
                    return std::log( 1.0 + ( v - mn ) / ( mx - mn ) / 10.0 + 1.0e-10 );
                }};
            }} ),
            std::make_pair( std::string{"log2" }, transform_value_type{[]( double mx, double mn )
            {
                return converter_type{ [ = ]( double v )
                {
                    return std::log( 1.0 + ( v - mn ) / ( mx - mn ) * 100.0 + 1.0e-10 );
                }};
            }} ),
            std::make_pair( std::string{"log_2" }, transform_value_type{[]( double mx, double mn )
            {
                return converter_type{ [ = ]( double v )
                {
                    return std::log( 1.0 + ( v - mn ) / ( mx - mn ) / 100.0 + 1.0e-10 );
                }};
            }} ),
            std::make_pair( std::string{"log3" }, transform_value_type{[]( double mx, double mn )
            {
                return converter_type{ [ = ]( double v )
                {
                    return std::log( 1.0 + ( v - mn ) / ( mx - mn ) * 1000.0 + 1.0e-10 );
                }};
            }} ),
            std::make_pair( std::string{"log_3" }, transform_value_type{[]( double mx, double mn )
            {
                return converter_type{ [ = ]( double v )
                {
                    return std::log( 1.0 + ( v - mn ) / ( mx - mn ) / 1000.0 + 1.0e-10 );
                }};
            }} ),
            std::make_pair( std::string{"log4" }, transform_value_type{[]( double mx, double mn )
            {
                return converter_type{ [ = ]( double v )
                {
                    return std::log( 1.0 + ( v - mn ) / ( mx - mn ) * 10000.0 + 1.0e-10 );
                }};
            }} ),
            std::make_pair( std::string{"log_4" }, transform_value_type{[]( double mx, double mn )
            {
                return converter_type{ [ = ]( double v )
                {
                    return std::log( 1.0 + ( v - mn ) / ( mx - mn ) / 10000.0 + 1.0e-10 );
                }};
            }} )
        };
        //val = transforms["default"](0.5, 0.1)( 0.2 ); // for linear,val = 0.25


        auto&& make_array = []( unsigned char a, unsigned char b, unsigned char c )
        {
            std::array<unsigned char, 3> ans;
            ans[0] = a;
            ans[1] = b;
            ans[2] = c;
            return ans;
        };

        typedef std::function<std::array<unsigned char, 3>( double )> color_value_type;
        static const std::map<std::string, color_value_type > color_maps
        {
            std::make_pair
            (
            std::string{"default"},
            color_value_type
            {
                []( double x )
                {
                    typedef unsigned char type;
                    auto&& ch = []( double x )
                    {
                        return static_cast<type>( static_cast<int>( x * 766.0 ) );
                    };

                    if ( 3.0 * x < 1.0 )
                        return make_array( type{0}, type{0}, type{ch( x )} );

                    if ( 3.0 * x < 2.0 )
                        return make_array( type{0}, type{ ch( x - 1.0 / 3.0 ) }, type{255} );
                    return make_array( type{ ch( x - 2.0 / 3.0 ) }, type{255}, type{255} );
                }
            }
            ),
            std::make_pair
            (
                std::string{"hotblue"},
                color_value_type
            {
                []( double x )
                {
                    typedef unsigned char type;
                    auto&& ch = []( double x )
                    {
                        return static_cast<type>( static_cast<int>( x * 766.0 ) );
                    };

                    if ( 3.0 * x < 1.0 )
                        return make_array( type{ch( 1.0 / 3.0 - x )}, type{255}, type{255} );

                    if ( 3.0 * x < 2.0 )
                        return make_array( type{0}, type{ ch( 2.0 / 3.0 - x ) }, type{255} );
                    return make_array( type{0}, type{0}, type{ch( 1.0 - x )} );
                }
            }
            ),
            std::make_pair
            (
                std::string{"jet"},
                color_value_type
            {
                []( double x )
                {
                    typedef unsigned char type;
                    auto&& ch = []( double x )
                    {
                        return static_cast<type>( static_cast<int>( x * 766.0 ) );
                    };

                    if ( 3.0 * x < 1.0 )
                        return make_array( type{0}, type{ch( x )}, type{255} );
                    //return make_array( type{0}, type{255}, type{ch(x)} );

                    if ( 3.0 * x < 2.0 )
                        return make_array( type{ch( x - 1.0 / 3.0 )}, type{ 255 }, type{ch( 2.0 / 3.0 - x )} );
                    return make_array( type{ 255 }, type{ch( 1.0 - x )}, type{0} );
                }
            }
            ),
            std::make_pair
            (
                std::string{"obscure"},
                color_value_type
            {
                []( double x )
                {
                    typedef unsigned char type;
                    auto&& ch = []( double x )
                    {
                        return static_cast<type>( static_cast<int>( x * 256.0 ) );
                    };
                    type const b = ch( 1.0 - x );

                    if ( 4.0 * x < 1 )
                        return make_array( ch( 1.0 - 4.0 * x ), ch( 1.0 - 4.0 * x ), b );

                    type const r = ch( ( x - 0.25 ) * 4.0 / 3.0 );

                    if ( 2.0 * x < 1 )
                        return make_array( r, ch( ( x - 0.25 ) * 4.0 ), b );

                    return make_array( r, ch( ( 1.0 - x ) * 2.0 ), b );
                }
            }
            ),
            std::make_pair
            (
                std::string{"gray"},
                color_value_type
            {
                []( double x )
                {
                    typedef unsigned char type;
                    auto&& ch = []( double x )
                    {
                        return static_cast<type>( static_cast<int>( x * 256.0 ) );
                    };
                    unsigned char val = ch( x );
                    return make_array( val, val, val );
                }
            }
            )
        };


    }//namespace crtp_save_as_bmp_private

    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_save_as_bmp
    {
        typedef Matrix      zen_type;

        bool save_as_bmp( const std::string& file_name, std::string const& color_map = std::string{"default"}, std::string const& transform = std::string{"default"} ) const
        //bool save_as_bmp( const std::string& file_name ) const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            assert( zen.row() && "save_as_bmp: matrix row cannot be zero" );
            assert( zen.col() && "save_as_bmp: matrix column cannot be zero" );
            std::string new_file_name{ file_name };
            std::string const extension{ ".bmp" };

            if ( ( new_file_name.size() < 4 )  || ( std::string{ new_file_name.begin() + new_file_name.size() - 4, new_file_name.end() } != extension ) )
                new_file_name += extension;
            std::ofstream stream( new_file_name.c_str(), std::ios_base::out | std::ios_base::binary );

            if ( !stream )
            {
                return false;
            }

            using namespace crtp_save_as_bmp_private;
            std::string const& map_name = ( color_maps.find( color_map ) == color_maps.end() ) ? std::string{"default"} :
                                          color_map;
            std::string const& transform_name = ( transforms.find( transform ) == transforms.end() ) ? std::string{"default"} :
                                                transform;
            auto&& selected_map = ( *( color_maps.find( map_name ) ) ).second;
            auto&& selected_transform = ( *( transforms.find( transform_name ) ) ).second;
            unsigned char file[14] =
            {
                'B', 'M',           // magic
                0, 0, 0, 0,         // size in bytes
                0, 0,               // app data
                0, 0,               // app data
                54, 0, 0, 0         // start of data offset
            };
            unsigned char info[40] =
            {
                40, 0, 0, 0,        // info hd size
                0, 0, 0, 0,         // width
                0, 0, 0, 0,         // heigth
                1, 0,               // number color planes
                24, 0,              // bits per pixel
                0, 0, 0, 0,         // compression is none
                0, 0, 0, 0,         // image bits size
                0x13, 0x0B, 0, 0,   // horz resoluition in pixel / m
                0x13, 0x0B, 0, 0,   // vert resolutions (0x03C3 = 96 dpi, 0x0B13 = 72 dpi)
                0, 0, 0, 0,         // #colors in pallete
                0, 0, 0, 0,         // #important colors
            };
            unsigned long const the_col = zen.col();
            unsigned long const the_row = zen.row();
            //unsigned long const pad_size  = ( 4 - ( the_col * 3 ) % 4 ) % 4;
            unsigned long const pad_size  = ( 4 - ( ( the_col * 3 ) & 0x3 ) ) & 0x3;
            unsigned long const data_size = the_col * the_row * 3 + the_row * pad_size;
            unsigned long const all_size  = data_size + sizeof( file ) + sizeof( info );
            auto const& ul_to_uc = []( unsigned long val )
            {
                return static_cast<unsigned char>( val & 0xffUL );
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

            for ( unsigned long r = 0; r < the_row; r++ )
            {
                for ( unsigned long c = 0; c < the_col; c++ )
                {
                    auto const& rgb = selected_map( selected_transform( max_val, min_val )( zen[r][c] ) );
                    pixel[2] = rgb[0];
                    pixel[1] = rgb[1];
                    pixel[0] = rgb[2];
                    stream.write( reinterpret_cast<char*>( pixel ), 3 );
                }

                stream.write( reinterpret_cast<char*>( pad ), pad_size );
            }

            stream.close();
            return true;
        }

        bool save_as_bmp( const char* const file_name ) const
        {
            return save_as_bmp( std::string{file_name} );
        }

    };//struct crtp_save_as_bmp

}

#endif//BRSNNVBLOHMSYMQEJFQMGFFHKMLYMOTSQLUYLKPEDSHPWMQNMIKCGAVHAGSOKPVKDTLIODNUS

