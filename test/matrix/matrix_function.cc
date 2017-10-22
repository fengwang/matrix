#include <f/matrix/matrix.hpp>
#include <algorithm>
#include <iterator>
#include <vector>
#include <iostream>

#include <functional>

int main()
{

    unsigned long const r = 10;
    unsigned long const c = 7;

    double x[] = { 1.0, 2.0 };

    typedef std::function<double(double*)> function_type;
    typedef f::matrix<function_type> matrix_type;

    matrix_type fm{ r, c };

    for ( unsigned long r_ = 0; r != r_; ++r_ )
    {
        for ( unsigned long c_ = 0; c != c_; ++c_ )
        {
            fm[r_][c_] = [r_,c_]( double* x ) { return x[0] * r_ + x[1] * c_; };
        }
    }


    for ( unsigned long r_ = 0; r != r_; ++r_ )
    {
        for ( unsigned long c_ = 0; c != c_; ++c_ )
            std::cout << r_ << "," << c_ << "=" << fm[r_][c_](x) << "\t";
        std::cout << "\n";
    }

    matrix_type fs{ fm };


    for ( unsigned long r_ = 0; r != r_; ++r_ )
    {
        for ( unsigned long c_ = 0; c != c_; ++c_ )
        {
            fs[r_][c_] = [&fm, r_,c_]( double* x ) { return x[0] * r_ + x[1] * c_ + (fm[r_][c_])(x); };
        }
    }


    for ( unsigned long r_ = 0; r != r_; ++r_ )
    {
        for ( unsigned long c_ = 0; c != c_; ++c_ )
            std::cout << r_ << "," << c_ << "=" << fs[r_][c_](x) << "\t";
        std::cout << "\n";
    }

#if 0

    for ( unsigned long r_ = 0; r != r_; ++r_ )
    {
        for ( unsigned long c_ = 0; c != c_; ++c_ )
        {
            fm[r_][c_] = [r_,c_]( double* x ) { return x[0] * r_ + x[1] * c_ + c_; };
        }
    }


    for ( unsigned long r_ = 0; r != r_; ++r_ )
    {
        for ( unsigned long c_ = 0; c != c_; ++c_ )
            std::cout << r_ << "," << c_ << "=" << fm[r_][c_](x) << "\t";
        std::cout << "\n";
    }
#endif

    return 0;
}

