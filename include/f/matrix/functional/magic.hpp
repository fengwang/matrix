#ifndef MMAGIC_HPP_INCLUDED_ODSFIJ498USFDLIJ498YUFSDLIJHSFDLKJHVSFIUDY4987YFSDKHSFDKJHSFKUHIURWEKSFJHKSJFDHKSJFHKJSFHFHJD
#define MMAGIC_HPP_INCLUDED_ODSFIJ498USFDLIJ498YUFSDLIJHSFDLKJHVSFIUDY4987YFSDKHSFDKJHSFKUHIURWEKSFJHKSJFDHKSJFHKJSFHFHJD

namespace f
{
    const matrix<std::size_t> magic( const std::size_t n )
    {
        matrix<std::size_t> ans( n, n );

        if ( n & 1 )
        {
            for ( std::size_t i = 0; i < n; ++i )
                for ( std::size_t j = 0; j < n; ++j )
                    ans[( ( n - 1 ) / 2 + i - j + n ) % n][( 3 * n - 1 + j - 2 * i ) % n] = i * n + j + 1;

            return ans;
        }

        // single even
        if ( n & 2 )
        {
            //to be implemented
            return ans;
        }

        //NOT CORRECT
        // double even
        std::iota( ans.begin(), ans.end(), 1 ); //fill 1 ~ n^2 into matrix ans
        std::reverse( ans.diag_begin(), ans.diag_end() );
        std::reverse( ans.anti_diag_begin(), ans.anti_diag_end() );
        std::swap_ranges( ans.lower_diag_begin( n >> 1 ), ans.lower_diag_end( n >> 1 ), ans.upper_diag_rbegin( n >> 1 ) );
        std::swap_ranges( ans.upper_anti_diag_begin( n >> 1 ), ans.upper_anti_diag_end( n >> 1 ), ans.lower_anti_diag_rbegin( n >> 1 ) );
        return ans;
    }

}//namespace f

#endif//_MAGIC_HPP_INCLUDED_ODSFIJ498USFDLIJ498YUFSDLIJHSFDLKJHVSFIUDY4987YFSDKHSFDKJHSFKUHIURWEKSFJHKSJFDHKSJFHKJSFHFHJD

