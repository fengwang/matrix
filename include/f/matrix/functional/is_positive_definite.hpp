#ifndef MIS_POSITIVE_DEFINITE_HPP_INCLUDED_SFDOIJDLFKJDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD3248IUSFLIJ
#define MIS_POSITIVE_DEFINITE_HPP_INCLUDED_SFDOIJDLFKJDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD3248IUSFLIJ

namespace f
{
    template< typename T, typename A >
    bool is_positive_definite( const matrix<T, A>& m )
    {
        typedef matrix<T, A> matrix_type;
        typedef typename matrix_type::range_type range_type;

        if ( m.row() != m.col() ) return false;

        for ( std::size_t i = 1; i != m.row(); ++i )
        {
            const matrix_type a{ m, range_type{0, i}, range_type{0, i} };

            if ( a.det() <= T( 0 ) ) return false;
        }

        return true;
    }

}//namespace f

#endif//_IS_POSITIVE_DEFINITE_HPP_INCLUDED_SFDOIJDLFKJDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD3248IUSFLIJ

