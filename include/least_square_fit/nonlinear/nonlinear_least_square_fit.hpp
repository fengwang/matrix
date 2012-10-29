#ifndef _NONLINEAR_LEASE_SQUARE_FIT_HPP_INCLUDED_SDOFISFDIHJ498YFDSLKAIHFSDKJHVCKJHSFDKUHSFDIUHASOIUH487SAFDKJVKJBSKDFJHA
#define _NONLINEAR_LEASE_SQUARE_FIT_HPP_INCLUDED_SDOFISFDIHJ498YFDSLKAIHFSDKJHVCKJHSFDKUHSFDIUHASOIUH487SAFDKJVKJBSKDFJHA

#include <iterator>
#include <vector>

namespace feng
{

template<typename IIX, typename IIY, typename IIW, typename F, typename IIP, typename OIA, typename ALG>
OIA nonlinear_least_square_fit (  IIX x_, IIX _x,   // x[]
                                  IIY y_,           // y[]
                                  IIW w_,           // w[]
                                  F   f,            // y=f(x;a)
                                  IIP p_, IIP _p,   // \frac{\partial y}{\partial {a_i}}[]
                                  OIA a_,           // fitted a_i[]
                                  ALG a             // fit`ting aorithm 
                                )
{
    return a( x_, _x, y_, w_, f, p_, _p, a_ );
}

template<typename IIX, typename IIY, typename F, typename IIP, typename OIA, typename ALG>
OIA nonlinear_least_square_fit (  IIX x_, IIX _x,   // x[]
                                  IIY y_,           // y[]
                                  F   f,            // y=f(x;a)
                                  IIP p_, IIP _p,   // \frac{\partial y}{\partial {a_i}}[]
                                  OIA a_,           // fitted a_i[]
                                  ALG a             // fitting aorithm 
                                )
{
    auto const n = std::distance(x_, _x);
    typedef typename std::iterator_traits<IIX>::value_type value_type;
    std::vector<value_type> w( n, value_type(1) );
    return nonlinear_least_square_fit( x_, _x, y_, w.begin(), f, p_, _p, a_, a );
}

}//namespace feng

#endif//_NONLINEAR_LEASE_SQUARE_FIT_HPP_INCLUDED_SDOFISFDIHJ498YFDSLKAIHFSDKJHVCKJHSFDKUHSFDIUHASOIUH487SAFDKJVKJBSKDFJHA

