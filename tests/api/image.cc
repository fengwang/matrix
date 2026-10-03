// S1-R4 API family `image`: save_as_bmp, save_as_png, save_as_pgm, load_bmp, plot, colour maps. Compiled alone
// by `tools/check.sh api` and never run; file names point under build/tmp.
#include "../../matrix.hpp"

#include <cstdint>
#include <string>

namespace
{
    template < typename T >
    int exercise_image()
    {
        feng::matrix<T> a{ 4, 4, T{ 1 } };
        std::string const base{ "build/tmp/api_image" };
        int acc = 0;

        // member writers, with the default and a named colour map
        acc += a.save_as_bmp( base + "_m.bmp" ) ? 1 : 0;
        acc += a.save_as_bmp( base + "_jet.bmp", "jet" ) ? 1 : 0;
        acc += a.save_as_bmp( ( base + "_c.bmp" ).c_str() ) ? 1 : 0;
        acc += a.save_as_png( base + "_m.png" ) ? 1 : 0;
        acc += a.save_as_png( base + "_gray.png", "gray" ) ? 1 : 0;
        acc += a.save_as_pgm( base + "_m.pgm" ) ? 1 : 0;
        acc += a.save_as_pgm( ( base + "_c.pgm" ).c_str() ) ? 1 : 0;
        acc += a.plot( base + "_plot.bmp" ) ? 1 : 0;
        acc += a.plot( base + "_hot.bmp", "hot" ) ? 1 : 0;

        // free writers: one channel with a colour map, and three channels; each returns bool (S5-R4)
        acc += feng::save_as_bmp( base + "_f.bmp", a ) ? 1 : 0;
        acc += feng::save_as_bmp( base + "_fj.bmp", a, "jet" ) ? 1 : 0;
        bool const rgb_ok = feng::save_as_bmp( base + "_rgb.bmp", a, a, a );
        acc += rgb_ok ? 1 : 0;
        return acc;
    }

    int exercise_load_and_maps()
    {
        int acc = 0;
        auto const loaded = feng::load_bmp( "build/tmp/api_image_rgb.bmp" );
        if ( loaded )
            acc += static_cast<int>( ( *loaded )[0].size() );

        // colour maps: the named table and a custom map
        auto const& maps = feng::matrix_details::bmp_details::color_maps;
        auto const [r, g, b] = maps.at( "parula" )( 0.5 );
        acc += r + g + b;
        auto const custom = feng::matrix_details::bmp_details::make_color_map( { 0.0, 1.0 }, { { 0, 0, 0 }, { 255, 255, 255 } } );
        auto const [cr, cg, cb] = custom( 0.25 );
        acc += cr + cg + cb;
        return acc;
    }
} // namespace

int api_image_double() { return exercise_image<double>() + exercise_load_and_maps(); }
int api_image_float() { return exercise_image<float>(); }
int api_image_int() { return exercise_image<int>(); }
int api_image_uint8() { return exercise_image<std::uint8_t>(); }
