#ifndef _OFSDJINSA498ASDOIFHJASKFHJDSV98YU49U8HSADFKHJASDFUIYH4IUHSDFAKLJHFDSIUH4
#define _OFSDJINSA498ASDOIFHJASKFHJDSV98YU49U8HSADFKHJASDFUIYH4IUHSDFAKLJHFDSIUH4

#include "../details/range/range.hpp"

namespace f
{

    // extract the coordinates configure of matrix A[rows][cols]
    // generate the coordinates of D[r][c] += w A[m][n]
    template< typename T >
    auto make_gradient_x2_config( std::size_t rows, std::size_t cols ) noexcept
    {
        assert( rows > 3 && "make_gradient_x2: matrix row is too small" );
        assert( cols > 3 && "make_gradient_x2: matrix column is too small" );
        typedef T value_type;
        value_type const one{ 1.0 };
        value_type const two{ 2.0 };
        value_type const four{ 4.0 };
        value_type const five{ 5.0 };
        std::vector<std::size_t> D_r;
        std::vector<std::size_t> D_c;
        std::vector<std::size_t> A_m;
        std::vector<std::size_t> A_n;
        std::vector<value_type>  w;
        auto const& append = [&]( std::size_t r, std::size_t c, std::size_t m, std::size_t n, value_type v ) noexcept
        {
            D_r.push_back( r );
            D_c.push_back( c );
            A_m.push_back( m );
            A_n.push_back( n );
            w.push_back( v );
        };
        //left boundary
        auto const& make_left_boundary = [&]( std::size_t r ) noexcept
        {
            // D[r][0] = 2A[r][0] - 5 A[r][1] + 4 A[r][2] - A[r][3]
            append( r, 0, r, 0, two );
            append( r, 0, r, 1, -five );
            append( r, 0, r, 2, four );
            append( r, 0, r, 3, -one );
        };
        //middle
        auto const& make_middle = [&]( std::size_t r, std::size_t c ) noexcept
        {
            // D[r][c] = A[r][c-1] - 2 A[r][c] + A[r][c+1]
            append( r, c, r, c - 1, one );
            append( r, c, r, c, -two );
            append( r, c, r, c + 1, one );
        };
        //right boundary
        auto const& make_right_boundary = [&]( std::size_t r ) noexcept
        {
            // D[r][cols-1] = 2 A[r][cols-1] - 5 A[r][cols-2] + 4 A[r][cols-3] - A[r][cols-4]
            append( r, cols - 1, r, cols - 1, two );
            append( r, cols - 1, r, cols - 2, -five );
            append( r, cols - 1, r, cols - 3, four );
            append( r, cols - 1, r, cols - 4, -one );
        };
        {
            //pre-allocate memory
            std::size_t const vol = rows * cols * 3 + rows * 2;
            D_r.reserve( vol );
            D_c.reserve( vol );
            A_m.reserve( vol );
            A_n.reserve( vol );
        }

        for ( auto r : range( 0UL, rows ) )
        {
            make_left_boundary( r ) ;
            make_right_boundary( r ) ;
        }

        for ( auto r : range( 0UL, rows ) )
            for ( auto c : range( 1UL, cols - 1 ) )
                make_middle( r, c );

        return std::make_tuple( std::move( D_r ), std::move( D_c ), std::move( A_m ), std::move( A_n ), std::move( w ) );
    }


    template< typename T, typename A >
    matrix<T, A> const gradient_x2( const matrix<T, A>& G ) noexcept
    {
        typedef T value_type;
        std::vector<std::size_t> D_r;
        std::vector<std::size_t> D_c;
        std::vector<std::size_t> A_m;
        std::vector<std::size_t> A_n;
        std::vector<value_type>  w;
        std::tie( D_r, D_c, A_m, A_n, w ) = make_gradient_x2_config<value_type>( G.row(), G.col() );
        matrix<T, A> ans{ G.row(), G.col() };
        std::fill( ans.begin(), ans.end(), value_type{0} );

        for ( auto idx : range( 0UL, D_r.size() ) )
            ans[D_r[idx]][D_c[idx]] += w[idx] * G[A_m[idx]][A_n[idx]];

        return ans;
    }

}//namespace f

#endif

