#ifndef WACWLBWONXXSFEOXLPCOHXGEGVIGAMTDTKAOWCIKKRGMMEPFSSICJJATFQVWEUFDMFLRQJHFW
#define WACWLBWONXXSFEOXLPCOHXGEGVIGAMTDTKAOWCIKKRGMMEPFSSICJJATFQVWEUFDMFLRQJHFW

#include "../details/range/range.hpp"
#include "./gradient_x2.hpp"

namespace f
{

    template< typename T, typename A >
    matrix<T, A> const laplace( const matrix<T, A>& G )
    {
#if 0
        typedef T                   value_type;
        typedef std::size_t         size_type;
        value_type const zero{ 0.0 };
        value_type const one{ 1.0 };
        value_type const two{ 2.0 };
        value_type const four{ 4.0 };
        value_type const five{ 5.0 };
        const std::array<value_type, 4> single_side_coefficients{ two, -five, four, -one };
        const std::array<value_type, 3> central_coefficients{ one, -two, one };
        size_type const M = G.row();
        size_type const N = G.col();
        assert( M > 4 && "laplace: matrix row is too small" );
        assert( N > 4 && "laplace: matrix column is too small" );
        matrix<T, D, A> L{ M, N };
        L[0][0] =    std::inner_product( single_side_coefficients.begin(), single_side_coefficients.end(), G.row_begin( 0 ), zero ) +
                     std::inner_product( single_side_coefficients.begin(), single_side_coefficients.end(), G.col_begin( 0 ), zero );
        L[0][N - 1] =  std::inner_product( single_side_coefficients.begin(), single_side_coefficients.end(), G.row_rbegin( 0 ), zero ) +
                       std::inner_product( single_side_coefficients.begin(), single_side_coefficients.end(), G.col_begin( N - 1 ), zero );
        L[M - 1][0] =  std::inner_product( single_side_coefficients.begin(), single_side_coefficients.end(), G.col_rbegin( 0 ), zero ) +
                       std::inner_product( single_side_coefficients.begin(), single_side_coefficients.end(), G.row_begin( M - 1 ), zero );
        L[M - 1][N - 1] = std::inner_product( single_side_coefficients.begin(), single_side_coefficients.end(), G.col_rbegin( N - 1 ), zero ) +
                          std::inner_product( single_side_coefficients.begin(), single_side_coefficients.end(), G.row_rbegin( M - 1 ), zero );

        // Left-most
        for ( size_type r = 1; r != M - 1; ++r )
        {
            auto col_start = G.col_begin( 0 ) + r - 1;
            L[r][0] = std::inner_product( central_coefficients.begin(), central_coefficients.end(), col_start, zero ) +
                      std::inner_product( single_side_coefficients.begin(), single_side_coefficients.end(), G.row_begin( r ), zero );
        }

        // Right-most
        for ( size_type r = 1; r != M - 1; ++r )
        {
            auto col_start = G.col_begin( N - 1 ) + r - 1;
            L[r][N - 1] = std::inner_product( central_coefficients.begin(), central_coefficients.end(), col_start, zero ) +
                          std::inner_product( single_side_coefficients.begin(), single_side_coefficients.end(), G.row_rbegin( r ), zero );
        }

        // Top
        for ( size_type c = 1; c != N - 1; ++c )
        {
            auto row_start = G.row_begin( 0 ) + c - 1;
            L[0][c] =   std::inner_product( central_coefficients.begin(), central_coefficients.end(), row_start, zero ) +
                        std::inner_product( single_side_coefficients.begin(), single_side_coefficients.end(), G.col_begin( c ), zero );
        }

        // Bottom
        for ( size_type c = 1; c != N - 1; ++c )
        {
            auto row_start = G.row_begin( M - 1 ) + c - 1;
            L[M - 1][c] =   std::inner_product( central_coefficients.begin(), central_coefficients.end(), row_start, zero ) +
                            std::inner_product( single_side_coefficients.begin(), single_side_coefficients.end(), G.col_rbegin( c ), zero );
        }

        // Central
        //for ( size_type r = 1; r != M-1; ++r )
        //    for ( size_type c = 1; c != N-1; ++c )
        for ( auto r : range( 1UL, M - 1 ) )
            for ( auto c : range( 1UL, N - 1 ) )
            {
                L[r][c] = G[r + 1][c] + G[r][c + 1] + G[r - 1][c] + G[r][c - 1] - four * G[r][c];
                /*
                L[r][c] =   - G[r-1][c-1] -   G[r-1][c] - G[r-1][c+1]
                            - G[r][c-1] + 8.0 * G[r][c] - G[r][c+1]
                            - G[r+1][c-1] -   G[r+1][c] - G[r+1][c+1];
                */
            }

        return L;
#else
        auto const& Gx = gradient_x2( G );
        auto const& Gy = gradient_x2( G.transpose() );
        return Gx + Gy.transpose();
#endif
    }

}//namespace f

#endif//WGCWLBWONXXSFEOXLPCOHXGEGVIGGMTDTKGOWCIKKRGMMEPFSSICJJGTFQVWEUFDMFLRQJHFW

