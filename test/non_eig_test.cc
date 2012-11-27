#include <matrix.hpp>
#include <vg.hpp>

#include <iostream>
#include <algorithm>

int main()
{
    using namespace feng;

    const unsigned int n = 4;

    vg::vg<double> vg;

    matrix<double> V( n, n );
    matrix<double> L( n, n );
    matrix<double> O( n, n );

    std::copy( vg.begin(), vg.begin()+n*n, V.begin() );
    std::copy( vg.begin(), vg.begin()+n, L.diag_begin() );

    auto const V_ = V.inverse();

    auto const A = V * L * V_;
    auto const At = A.transpose();

#if 0
    auto const AA =  (O || A) && ( A.transpose() || O );

    matrix<double> VV, LL;

    eigen_real_symmetric( AA, VV, LL );


    feng::for_each( L.begin(), L.end(), [](double&d){ if (std::abs(d) < 1.0e-7) d = 0; } );
    feng::for_each( V.begin(), V.end(), [](double&d){ if (std::abs(d) < 1.0e-7) d = 0; } );
    feng::for_each( LL.begin(), LL.end(), [](double&d){ if (std::abs(d) < 1.0e-7) d = 0; } );
    feng::for_each( VV.begin(), VV.end(), [](double&d){ if (std::abs(d) < 1.0e-7) d = 0; } );

    std::cout << "\nL=\n" << L;
    std::cout << "\nLL=\n" << LL;

    std::cout << "\nA=\n" << A;
    std::cout << "\nAA=\n" << AA;

    std::cout << "\nV=\n" << V;
    std::cout << "\nVV=\n" << VV;

#endif
    std::cout << "\nA=\n" << A;
    std::cout << "\nV=\n" << V;
    std::cout << "\nL=\n" << L;

    matrix<double> v(n,1);
    auto const lmax = eigen_power_iteration( A, v.begin(), 1.0e-15 );

    std::cout << "\nthe max eigen value of matrix A is: " << lmax;
    std::cout << "\nthe corresponding eigen vector is : " << v;


    matrix<double> A_(A, range(0,1), range(0,1));
    matrix<double> B_(A, range(0,1), range(1,n));
    matrix<double> C_(A, range(1,n), range(0,1));
    matrix<double> D_(A, range(1,n), range(1,n));

    matrix<double> v0(v, range(0,1), range(0,1));
    matrix<double> v2(v, range(1,n), range(0,1));

    auto cd = C_*v0 + D_*v2;

    std::cout << "\ncd = " << cd;
    std::cout << "\nv2 = " << v2;
     
    for_each( cd.begin(), cd.end(), v2.begin(), [](double& cd_, double const v2_ ){ cd_ /= v2_; } );

    std::cout << "\nthe rest eigen values could be: " << cd;
    


    


    


    return 0;
}


