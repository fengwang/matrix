#include <f/matrix/matrix.hpp>

int main()
{
    using namespace f;

    matrix<double> mat;
    mat.load("./matrix/regression/regression_intensity.txt");

    auto mat_ = mat.transpose();

    mat_.save_as("./matrix/regression/regression_intensity_t.txt");

    return 0;
}

