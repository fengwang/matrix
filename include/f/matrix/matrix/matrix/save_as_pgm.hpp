#ifndef AXTETBBNNGJIUUVYBVDQVOPVLEFNAJYMJODASLPDDLBJJHPSDANXDUYNOFBTHPXKCWGLLNJHS
#define AXTETBBNNGJIUUVYBVDQVOPVLEFNAJYMJODASLPDDLBJJHPSDANXDUYNOFBTHPXKCWGLLNJHS

namespace f
{

    namespace crtp_save_as_pgm_private
    {

        typedef std::function<double( double )> converter_type;
        typedef std::function<converter_type( double, double )> transform_value_type;

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

    }//namespace crtp_save_as_pgm_private

    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_save_as_pgm
    {
        typedef Matrix      zen_type;

        bool save_as_pgm( const std::string& file_name, std::string const& transform = std::string{"default"} ) const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            std::string new_file_name{ file_name };
            std::string const extension{ ".pgm" };

            if ( ( new_file_name.size() < 4 )  || ( std::string{ new_file_name.begin() + new_file_name.size() - 4, new_file_name.end() } != extension ) )
                new_file_name += extension;
            std::ofstream stream( new_file_name.c_str() );

            if ( !stream )
            {
                return false;
            }

            using namespace crtp_save_as_pgm_private;
            std::string const& transform_name = ( transforms.find( transform ) == transforms.end() ) ? std::string{"default"} :
                                                transform;
            //write header
            {
                stream << "P2\n";
                stream << zen.col() << " " << zen.row() << "\n";
                stream << "255\n";
                stream << "# Generated Portable GrayMap image for path [" << file_name << "]";
                stream << "\n# and colormap [" << transform_name << "]\n";
            }
            auto const& selected_map = []( double x )
            {
                assert( x >= 0.0 && "Negative x passed!" );
                assert( x <= 1.0 && "X exceeds boundary!" );
                typedef std::uint16_t  type;
                auto const& ch = []( double x )
                {
                    return static_cast<type>( static_cast<int>( x * 256.0 ) );
                };
                return  ch( x );
            };
            auto&& selected_transform = ( *( transforms.find( transform_name ) ) ).second;
            double const max_val = static_cast<double>( *std::max_element( zen.begin(), zen.end() ) );
            double const min_val = static_cast<double>( *std::min_element( zen.begin(), zen.end() ) );
            double const mmax = selected_transform( max_val, min_val )( max_val );
            double const mmin = selected_transform( max_val, min_val )( min_val );
            double const divider = mmax - mmin;

            /*
            std::for_each( zen.rbegin(), zen.rend(),
                           [&]( Type val )
                           {
                               auto const rgb = selected_map( (selected_transform( max_val, min_val )( val ) - mmin) / divider );
                               stream << rgb << " ";
                           }
                         );
            */
            for ( unsigned long r = 0; r < zen.row(); r++ )
            {
                for ( unsigned long c = 0; c < zen.col(); c++ )
                {
                    auto rgb = selected_map( ( selected_transform( max_val, min_val )( zen[zen.row() - 1 - r][c] ) - mmin ) / divider );

                    if ( rgb > 255.0 ) rgb = 255.0;

                    if ( rgb < 0.0 ) rgb = 0.0;

                    stream << rgb << " ";
                }

                stream << "\n";
            }

            stream.close();
            return true;
        }

        bool save_as_pgm( const char* const file_name ) const
        {
            return save_as_pgm( std::string{file_name} );
        }

    };//struct crtp_save_as_pgm

}

#endif//AXTETBBNNGJIUUVYBVDQVOPVLEFNAJYMJODASLPDDLBJJHPSDANXDUYNOFBTHPXKCWGLLNJHS

