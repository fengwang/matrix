#ifndef MMATRIX_ARRAY_OPERATORS_HPP_INCLUDED_DSFOIU4989RW8OISFDOIFSLKSFDJLKJVCXLKSLKJFSDLFSKOIEWEURSIDFUSLKJFEOIUFSDLIJDD
#define MMATRIX_ARRAY_OPERATORS_HPP_INCLUDED_DSFOIU4989RW8OISFDOIFSLKSFDJLKJVCXLKSLKJFSDLFSKOIEWEURSIDFUSLKJFEOIUFSDLIJDD

namespace f
{

    //matrix -- valarray
    //matrix -- ptr
    //matrix -- vector

    template<typename T, typename A, typename T_>
    const matrix<T, A>
    operator * ( const matrix<T, A>& lhs,  const T_* const rhs )
    {
        matrix<T, A> ans( lhs.row(), 1 );

        for ( std::size_t i = 0; i < lhs.row(); ++i )
            ans[i][0] = std::inner_product( lhs.row_begin( i ), lhs.row_end( i ), rhs, T() );

        return ans;
    }

    template<typename T, typename A, typename T_>
    const matrix<T, A>
    operator * ( const T_* lhs, const matrix<T, A>& rhs )
    {
        matrix<T, A> ans( 1, rhs.col() );

        for ( std::size_t i = 0; i < rhs.col(); ++i )
            ans[0][i] = std::inner_product( lhs, lhs + rhs.row(), rhs.col_begin( i ), T() );

        return ans;
    }

    template<typename T, typename A, typename T_>
    const matrix<T, A>
    operator * ( const matrix<T, A>& lhs,  const std::valarray<T_>& rhs )
    {
        assert( lhs.col() == rhs.size() );
        matrix<T, A> ans( lhs.row(), 1 );

        for ( std::size_t i = 0; i < lhs.row(); ++i )
            ans[i][0] = std::inner_product( lhs.row_begin( i ), lhs.row_end( i ), std::begin( rhs ), T() );

        return ans;
    }

    template<typename T, typename A, typename T_>
    const matrix<T, A>
    operator * ( const std::valarray<T_>& lhs, const matrix<T, A>& rhs )
    {
        assert( rhs.row() == lhs.size() );
        matrix<T, A> ans( 1, lhs.row() );

        for ( std::size_t i = 0; i < rhs.col(); ++i )
            ans[0][i] = std::inner_product( std::begin( lhs ), std::begin( lhs ) + rhs.row(), rhs.col_begin( i ), T() );

        return ans;
    }

    template<typename T, typename A, typename T_>
    const matrix<T, A>
    operator * ( const matrix<T, A>& lhs,  const std::vector<T_>& rhs )
    {
        assert( lhs.col() == rhs.size() );
        matrix<T, A> ans( lhs.row(), 1 );

        for ( std::size_t i = 0; i < lhs.row(); ++i )
            ans[i][0] = std::inner_product( lhs.row_begin( i ), lhs.row_end( i ), rhs.begin(), T() );

        return ans;
    }

    template<typename T, typename A, typename T_>
    const matrix<T, A>
    operator * ( const std::vector<T_>& lhs, const matrix<T, A>& rhs )
    {
        assert( rhs.col() == lhs.size() );
        matrix<T, A> ans( 1, rhs.col() );

        for ( std::size_t i = 0; i < rhs.col(); ++i )
            ans[0][i] = std::inner_product( lhs.begin(), lhs.end(), rhs.col_begin( i ), T() );

        return ans;
    }

}//namespace f

#endif//_MATRIX_ARRAY_OPERATORS_HPP_INCLUDED_DSFOIU4989RW8OISFDOIFSLKSFDJLKJVCXLKSLKJFSDLFSKOIEWEURSIDFUSLKJFEOIUFSDLIJDD

