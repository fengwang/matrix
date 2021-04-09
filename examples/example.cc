#include <cassert>

#include "../matrix.hpp"

#include "./cases/0000_create.hpp"
#include "./cases/0001_apply.hpp"
#include "./cases/0002_access.hpp"
#include "./cases/0003_clone.hpp"
#include "./cases/0004_data.hpp"
#include "./cases/0005_det.hpp"
#include "./cases/0006_devide_equal.hpp"
#include "./cases/0007_slicing.hpp"
#include "./cases/0008_inverse.hpp"
#include "./cases/0009_save_load.hpp"
#include "./cases/0010_minus_equal.hpp"
#include "./cases/0011_multiply_equal.hpp"
#include "./cases/0012_plus_equal.hpp"
#include "./cases/0013_prefix.hpp"
#include "./cases/0014_sin.hpp"
#include "./cases/0015_sinh.hpp"
#include "./cases/0016_eye.hpp"
#include "./cases/0017_make_view.hpp"
#include "./cases/0018_conv.hpp"
#include "./cases/0019_lu_decomposition.hpp"
#include "./cases/0020_gauss_jordan_elimination.hpp"
#include "./cases/0021_singular_value_decomposition.hpp"
#include "./cases/0022_save_with_colromap.hpp"
#include "./cases/0023_magic.hpp"
#include "./cases/0024_pooling.hpp"
#include "./cases/0025_global_save_as_bmp.hpp"
#include "./cases/0026_mandelbrot.hpp"
#include "./cases/0027_julia_set.hpp"
#include "./cases/0028_plot.hpp"
#include "./cases/0029_meshgrid.hpp"
#include "./cases/0030_arange.hpp"
#include "./cases/0031_clip.hpp"
#include "./cases/0032_empty.hpp"
#include "./cases/0033_linspace.hpp"
#include "./cases/0034_astype.hpp"

int main()
{
    std::cout << "running create.\n" << std::endl;
    _0000_create();
    _0001_create();
    _0002_create();
    _0003_create();
    _0004_create();
    _0005_create();
    _0006_create();
    _0007_create();
    _0008_create();
    _0009_create();
    _0010_create();
    _0011_create();
    _0012_create();
    _0013_create();
    _0014_create();
    _0015_create();
    _0016_create();
    _0017_create();
    _0018_create();
    _0019_create();
    _0020_create();
    _0021_create();


    std::cout << "running apply.\n" << std::endl;
    _0000_apply();

    std::cout << "running access.\n" << std::endl;
    _0000_access();

    std::cout << "running clone.\n" << std::endl;
    _0000_clone();

    std::cout << "running data.\n" << std::endl;
    _0000_data();

    std::cout << "running det.\n" << std::endl;
    _0000_det();

    std::cout << "running divide_equal.\n" << std::endl;
    _0000_divide_equal();

    std::cout << "running slicing.\n" << std::endl;
    _0000_slicing();

    std::cout << "running inverse.\n" << std::endl;
    _0000_inverse();

    std::cout << "running save_load.\n" << std::endl;
    _0000_save_load();
    _0001_save_load();

    std::cout << "running minus_equal.\n" << std::endl;
    _0000_minus_equal();

    std::cout << "running multiply_equal.\n" << std::endl;
    _0000_multiply_equal();

    std::cout << "running plus_equal.\n" << std::endl;
    _0000_plus_equal();

    std::cout << "running prefix.\n" << std::endl;
    _0000_prefix();

    std::cout << "running sin.\n" << std::endl;
    _0000_sin();

    std::cout << "running sinh.\n" << std::endl;
    _0000_sinh();

    std::cout << "running eye.\n" << std::endl;
    _0000_eye();

    std::cout << "running make_view.\n" << std::endl;
    _0000_make_view();

    std::cout << "running conv.\n" << std::endl;
    _0000_conv();

    std::cout << "running lu_decomposition.\n" << std::endl;
    _0000_lu_decomposition();

    std::cout << "running gauss_jordan_elimination.\n" << std::endl;
    _0000_gauss_jordan_elimination();

    std::cout << "running singular value decomposition.\n" << std::endl;
    _0000_singular_value_decomposition();
    _0001_singular_value_decomposition();

    std::cout << "running save_with_colormap.\n" << std::endl;
    _0000_save_with_colormap();
    _0001_save_with_colormap();

    std::cout << "running magic.\n" << std::endl;
    _0000_magic();
    _0001_magic();

    std::cout << "running pooling.\n" << std::endl;
    _0000_pooling();

    std::cout << "running global_save_as_bmp.\n" << std::endl;
    _0000_global_save_as_bmp();

    std::cout << "running mandelbrot.\n" << std::endl;
    _0000_mandelbrot();

    std::cout << "running mandelbrot::1.\n" << std::endl;
    _0001_mandelbrot();

    std::cout << "running mandelbrot::2.\n" << std::endl;
    //TOO time consuming
    _0002_mandelbrot();
    //
    //std::cout << "running julia_set::0.\n" << std::endl;
    //_0000_julia_set();
    //std::cout << "running julia_set::1.\n" << std::endl;
    //_0001_julia_set();
    //std::cout << "running julia_set::2.\n" << std::endl;
    //_0002_julia_set();

    std::cout << "running plot.\n" << std::endl;
    _0000_plot();
    _0001_plot();

    std::cout << "running meshgrid.\n" << std::endl;
    _0000_meshgrid();

    std::cout << "running arange.\n" << std::endl;
    _0000_arange();

    std::cout << "running clip.\n" << std::endl;
    _0000_clip();

    std::cout << "running empty.\n" << std::endl;
    _0000_empty();

    std::cout << "running linspace.\n" << std::endl;
    _0000_linspace();

    std::cout << "running astype.\n" << std::endl;
    _0000_astype();


    return 0;
}


