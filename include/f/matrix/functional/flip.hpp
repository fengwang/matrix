#ifndef MFLIP_HPP_INCLUDED_FOISJ4938UALSFKDJLAFHASKJFHSDOFUIHSAFKLJHSFKJNSFDKAJHAFSUIJHEIUHSAFDUJHFI
#define MFLIP_HPP_INCLUDED_FOISJ4938UALSFKDJLAFHASKJFHSDOFUIHSAFKLJHSFKJNSFDKAJHAFSUIJHEIUHSAFDUJHFI

namespace f
{
    //
    // Input:
    //          m   ----    a random matrix
    //              [ 1 2 3 4 5 ]
    //              [ a b c d e ]
    //              [ $ % ^ & * ]
    //          dim ----    dimension index, 1 or 2
    // Output
    //          if dim is 1, then flip rows
    //              [ $ % ^ & * ]
    //              [ a b c d e ]
    //              [ 1 2 3 4 5 ]
    //          if dim is 2, then flip columns
    //              [ 5 4 3 2 1 ]
    //              [ e d c b a ]
    //              [ * & ^ % $ ]
    //
    template<typename T, typename A>
    matrix<T, A> const flipdim( const matrix<T, A>& m, const std::size_t dim )
    {
        matrix<T, A> ans{m};

        if ( 1 == dim ) // row flip
        {
            std::size_t index_upper = 0;
            std::size_t index_lower = m.row() - 1;

            while ( index_lower > index_upper )
            {
                std::swap_ranges( ans.row_begin( index_lower ), ans.row_end( index_lower ), ans.row_begin( index_upper ) );
                --index_lower;
                ++index_upper;
            }

            return ans;
        }

        if ( 2 == dim ) // col flip
        {
            std::size_t index_left = 0;
            std::size_t index_right = m.col() - 1;

            while ( index_right > index_left )
            {
                std::swap_ranges( ans.col_begin( index_left ), ans.col_end( index_left ), ans.row_begin( index_right ) );
                --index_right;
                ++index_left;
            }

            return ans;
        }

        assert( !"the second argument of flipdim should be '1' or '2'" );
        return ans; //just to kill warning, should never reach here
    }

    template<typename T, typename A>
    matrix<T, A> const fliplr( const matrix<T, A>& m )
    {
        return flipdim( m, 1 );
    }

    template<typename T, typename A>
    matrix<T, A> const flipud( const matrix<T, A>& m )
    {
        return flipdim( m, 2 );
    }

}//namespace f

#endif//_FLIP_HPP_INCLUDED_FOISJ4938UALSFKDJLAFHASKJFHSDOFUIHSAFKLJHSFKJNSFDKAJHAFSUIJHEIUHSAFDUJHFI

