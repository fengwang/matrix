#ifndef _LEVENBERG_MARQUARDT_HPP_INCLUDED_SDFOIU498FDSIUHVSKJHNC98Y4KJHFDSSIUHY4KJHSDFMJNBCKJHSASKJHSKJHCKJH4EU7KJSDFVNKJ
#define _LEVENBERG_MARQUARDT_HPP_INCLUDED_SDFOIU498FDSIUHVSKJHNC98Y4KJHFDSSIUHY4KJHSDFMJNBCKJHSASKJHSKJHCKJH4EU7KJSDFVNKJ

#include <matrix.hpp>

#include <vector>
#include <algorithm>
#include <iterator>
#include <numeric>
#include <cstddef>

namespace feng 
{

template< typename T = double >
struct levenberg_marquardt 
{
    typedef T                           value_type;
    typedef std::vector<value_type>     array_type;
    typedef feng::matrix<value_type>    matrix_type;

    private:
    value_type          lambda;
    value_type          factor;
    value_type          eps;
    value_type          chi_square;
    array_type          beta;
    array_type          a;
    array_type          delta_a;
    matrix_type         alpha;

    public:
#if 0
    Description:
        the data sets:
                            (x_0, y_0), (x_1, y_1), ..., ( x_{n-1}, y_{n-1})
        the weigh of data sets:
                            \sigma_0, \sigma_1, ..., \sigma_{n-1}
        the model to fit :
                            y = f( x; a_0, a_1, ..., a_{M-1} )
                            y = f( x; a[] )
        the merit function:
                            \Chi^2{a[]} = \sum_{i=0}^{n-1} \[\frac{y_i-f(x_i;a[])}{\sigma_i}\]^2
        the Jacobian partial derivative function array p[]
                               \partial f         \partial f            \partial f 
                            ----------------,  ----------------, ..., ----------------
                              \pratial a_0       \partial a_1         \partial a_{M-1}
    Inputs:
        x_                  first position of data set array x[]
        _x                  last position of data set array x[]
        y_                  first position of data set array y[]
        sigma_              first position of weigh array \sigma[]
        f                   the model to fit
        p_                  the first position of Jacobian function array p[]
        _p                  the last position of Jacobian function array p[]
        a_                  the first position of fitted parameter array a[]
#endif 
    template<typename IIX, typename IIY, typename IIW, typename F, typename IIP, typename OIA>
    OIA operator (  IIX x_, IIX _x,   // x[]
                    IIY y_,           // y[]
                    IIW sigma_,       // sigma[]
                    F   f,            // y=f(x;a)
                    IIP p_, IIP _p,   // \frac{\partial y}{\partial {a_i}}[]
                    OIA a_            // fitted a_i[]
                 )
    {
        initialize_a( std::distance( p_, _p ) );
        update_chi_square( x_, _x, a, y_, sigma_, f );
        value_type old_chi = chi_square;
        for (;;)
        {
            update_beta(x_, _x, a, y_, sigma_, f, p_, _p) ;
            update_alpha(x_, _x, a, sigma_, p_, _p);
            update_alpha_();
            update_delta_a();
            update_a();
            update_chi_square( x_, _x, a, y_, sigma_, f );

            if ( chi_square < old_chi && 
                 old_chi < eps*chi_square + chi_square )
                break;

            if ( new_chi >= old_chi )
            {
                lambda *= factor;
                outdate_a();
            }
            else
            {
                old_chi = chi_square;
                lambda /= factor;
            }
        }
        return std::copy( a.begin(), a.end(), a_ );
    }

    public:
    void 
    initialize_a( const std::size_t m )
    {
        if ( !a.empty() ) return;                       //break if a[] has been initialized
        a.resize( m );                                  //resize a[]
        std::fill( a.begin(), a.end(), value_type(1) ); //set a[] to {1.0, 1.0, ..., 1.0}
    }

    public:
    levenberg_marquardt( const value_type lambda_ = 0.001, 
                         const value_type factor_ = 2.0,
                         const value_type eps_ = 1.0e-5 ) :
                         : lambda( lambda_ ), factor( factor_ ), eps( eps_ )
    {   //in case of invald arguments passed
        if ( lambda <= value_type(0) ) lambda = value_type(0.001 );
        if ( factor <= value_type(1) ) factor = value_type(2.000 );
        if ( eps <= value_type(0) ) eps = value_type(0.001 );
    }

    template< typename IIA >
    levenberg_marquardt( IIA iia_, IIA _iia,                // initial guess for a[]
                         const value_type lambda_ = 0.001, 
                         const value_type factor_ = 2.0,
                         const value_type eps_ = 1.0e-5 ) :
                         : lambda( lambda_ ), factor( factor_ ), eps( eps_ )
    {   //in case of invald arguments passed
        if ( lambda <= value_type(0) ) lambda = value_type(0.001 );
        if ( factor <= value_type(1) ) factor = value_type(2.000 );
        if ( eps <= value_type(0) ) eps = value_type(0.001 );
        a.clear();
        std::copy( iia_, _iia, std::back_inserter( a ) );
    }

    pretected:

    //  Function:
    //              calculate \chi^{2} with weigh factor \sigma[n]
    //          \chi^{2}(a) = \sum^{N}_{i=0} \[ \frac{y_i - y(x_i;a)}{\sigma_i} \]^2
    template<typename IIX, typename A, typename IIY, typename IIW, typename F>
    void 
    update_chi_square(IIX x_, IIX _x, A a, IIY y_, IIW sigma_, F f) 
    {
        chi_square = feng::inner_product( x_, _x, y_, sigma_, value_type(0), 
                                          [f,a]( value_type x, value_type y, value_type sig )
                                          { auto const tmp =  (y-f(x,a))/sig; return tmp*tmp; } 
                                        );
    }

    //  Function:
    //              calculate \chi^{2} 
    //          \chi^{2}(a) = \sum^{N}_{i=0} \[ y_i - y(x_i;a) \]^2
    template<typename IIX, typename A, typename IIY, typename F>
    void 
    update_chi_square(IIX x_, IIX _x, A a, IIY y_, F f) 
    {
        chi_square = feng::inner_product( x_, _x, y_, value_type(0), 
                                          [f,a]( value_type x, value_type y )
                                          { auto const tmp =  y-f(x,a); return tmp*tmp; } 
                                        );
    }

    //  Function:
    //              calculate array \beta with weigh factor \sigma[n]
    //          \beta_k = \sum_{i=0}^{n} \frac{y_i - y(x_i; a)}{\sigma_i^2} \frac{\partial y(x_i; a)}{\partial a_k}
    template<typename IIX, typename A, typename IIY, typename IIW, typename F>
    void 
    update_beta(IIX x_, IIX _x, A a, IIY y_, IIW sigma_, F f) 
    {
    
    }

    //  Function:
    //              calculate array \beta with weigh factor 
    //          \beta_k = \sum_{i=0}^{n} \[y_i - y(x_i; a)\] \frac{\partial y(x_i; a)}{\partial a_k}
    template<typename IIX, typename A, typename IIY typename F>
    void 
    update_beta(IIX x_, IIX _x, A a, IIY y_, F f) 
    {
    
    }

    //  Function:
    //              calculate array \beta with weigh factor \sigma[n]
    //          \beta_k = \sum_{i=0}^{n} \frac{y_i - y(x_i; a)}{\sigma_i^2} \frac{\partial y(x_i; a)}{\partial a_k}
    template<typename IIX, typename A, typename IIY, typename IIW, typename F, typename IIP>
    void 
    update_beta(IIX x_, IIX _x, A a, IIY y_, IIW sigma_, F f, IIP p_, IIP _p) 
    {
        beta.clear();

        for ( auto p__ = p_; p__ != _p; ++_p )
        {
            auto const tmp = feng::inner_product( x_, _x, y_, sigma_, value_type(),
                                                  [a, f, p__]( value_type x, value_type y, value_type sig )
                                                  { return (y-f(x,a)) * ((*p__)(x,a)) / ( sig * sig ); } 
                                                );
            beta.push_back( tmp );
        }
    }

    //  Function:
    //              calculate array \beta with weigh factor \sigma[n]
    //          \beta_k = \sum_{i=0}^{n} \[y_i - y(x_i; a)\] \frac{\partial y(x_i; a)}{\partial a_k}
    template<typename IIX, typename A, typename IIY, typename F, typename IIP>
    void 
    update_beta(IIX x_, IIX _x, A a, IIY y_, F f, IIP p_, IIP _p) 
    {
        beta.clear();

        for ( auto p__ = p_; p__ != _p; ++_p )
        {
            auto const tmp = feng::inner_product( x_, _x, y_, value_type(),
                                                  [a, f, p__]( value_type x, value_type y )
                                                  { return (y-f(x,a)) * ((*p__)(x,a)); } 
                                                );
            beta.push_back( tmp );
        }
    }

    //  Function:
    //              calculate matrix \alpha with weigh factor \sigma[n]
    //          \alpha_{kl} = \sum^{n}_{i=0} \frac{1}{\sigma_i} 
    //                                       \[ \frac{\partial y(x_i;a)}{\partial a_k} 
    //                                          \frac{\partial y(x_i; a)}{\partial a_l} \]
    template<typename IIX, typename A, typename IIW, typename IIP>
    void 
    update_alpha(IIX x_, IIX _x, A a, IIW sigma_, IIP p_, IIP _p) 
    {
        auto const N = std::distance( x_, _x );
        auto const M = std::distance( p_, _p );
        alpha.resize( M, M );

        matrix_type<value_type> mat( M, N );
        for ( std::size_t k = 0, auto p__ = p_; p__ != _p; ++p__, ++k )
            for ( std::size_t l = 0, auto x__ = x_; x__ != _x; ++x__, ++l )
                mat[k][l] = (*p__)(*x__, a);

        for ( std::size_t k = 0; k < M; ++k )
            for ( std::size_t l = 0; l < M; ++l )
                alpha[k][l] = feng::inner_product( mat.row_begin(k), mat.row_end(k), mat_row_begin(l), sigma_, value_type() ); 

    }

    //  Function:
    //              calculate matrix \alpha 
    //          \alpha_{kl} = \sum^{n}_{i=0} \[ \frac{\partial y(x_i;a)}{\partial a_k} 
    //                                          \frac{\partial y(x_i; a)}{\partial a_l} \]
    template<typename IIX, typename A, typename IIP>
    void 
    update_alpha(IIX x_, IIX _x, A a, IIP p_, IIP _p) 
    {
        auto const N = std::distance( x_, _x );
        auto const M = std::distance( p_, _p );
        alpha.resize( M, M );

        matrix_type<value_type> mat( M, N );
        for ( std::size_t k = 0, auto p__ = p_; p__ != _p; ++p__, ++k )
            for ( std::size_t l = 0, auto x__ = x_; x__ != _x; ++x__, ++l )
                mat[k][l] = (*p__)(*x__, a);

        for ( std::size_t k = 0; k < M; ++k )
            for ( std::size_t l = 0; l < M; ++l )
                alpha[k][l] = feng::inner_product( mat.row_begin(k), mat.row_end(k), mat_row_begin(l), value_type() ); 

    }

    //  Function:
    //              calculate \alpha^{'}
    //          \alpha_{k,l}^{'} = \alpha_{kl} + \lambda I
    void 
    update_alpha_()
    {
        std::for_each( alpha.diag_begin(), alpha.diag_end(), [this](value_type& v){ v *= this->lambda + value_type(1); } );
    }

    // Function:
    //              calculate \delta a
    //          \alpha^{'} \times \delta a = \beta
    void
    update_delta_a()
    {
        delta_a.clear();
        matrix_type u, w, v;
        feng::singular_value_decomposition( alpha, u, w, v );
        std::for_each( w.diag_begin(), w.diag_end(), [](value_type& v){ v = value_type(1)/v; } );
        auto const ans_b = v * w * (~u) * beta;
        std::copy( ans_b.begin(), ans_b.end(), std::back_inserter(delta_a) );
    }

    // Function:
    //              calculate a
    //          a = a + \delta a
    void 
    update_a()
    {
        std::transform( a.begin(), a.end(), delta_a.begin(), a.begin(), 
                        []( value_type a, value_type d_a ) { return a+d_a; } 
                      );
    }

    // Function:
    //              calculate a
    //          a = a - \delta a
    void 
    outdate_a()
    {
        std::transform( a.begin(), a.end(), delta_a.begin(), a.begin(), 
                        []( value_type a, value_type d_a ) { return a-d_a; } 
                      );
    }

    // Function:
    //              calculate \lambda
    //          \lambda = \lambda * factor         
    void
    update_lambda_increase()
    {
        lambda *= factor;
    }

    // Function:
    //              calculate \lambda
    //          \lambda = \lambda / factor         
    void
    update_lambda_decrease()
    {
        lambda /= factor;
    }

};//struct levenberg

}//namespace feng

#endif//_LEVENBERG_MARQUARDT_HPP_INCLUDED_SDFOIU498FDSIUHVSKJHNC98Y4KJHFDSSIUHY4KJHSDFMJNBCKJHSASKJHSKJHCKJH4EU7KJSDFVNKJ


