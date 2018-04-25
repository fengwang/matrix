#include "../matrix.hpp"

SUPPRESS_WARNINGS
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

int main()
{
    std::cout << "running create.\n";
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


    std::cout << "running apply.\n";
    _0000_apply();

    std::cout << "running access.\n";
    _0000_access();

    std::cout << "running clone.\n";
    _0000_clone();

    std::cout << "running data.\n";
    _0000_data();

    std::cout << "running det.\n";
    _0000_det();

    std::cout << "running divide_equal.\n";
    _0000_divide_equal();

    std::cout << "running slicing.\n";
    _0000_slicing();

    std::cout << "running inverse.\n";
    _0000_inverse();

    std::cout << "running save_load.\n";
    _0000_save_load();

    std::cout << "running minus_equal.\n";
    _0000_minus_equal();

    std::cout << "running multiply_equal.\n";
    _0000_multiply_equal();

    std::cout << "running plus_equal.\n";
    _0000_plus_equal();

    std::cout << "running prefix.\n";
    _0000_prefix();

    std::cout << "running sin.\n";
    _0000_sin();

    std::cout << "running sinh.\n";
    _0000_sinh();

    std::cout << "running eye.\n";
    _0000_eye();
    std::cout << "running make_view.\n";
    _0000_make_view();


    return 0;
}

RESTORE_WARNINGS

