#ifndef MREPMAT_HPP_INCLUDED_FDSOIJ4398USFLAKJLSFKJLVCSJFDF89IU4098UAKSFJDLJFSKS2
#define MREPMAT_HPP_INCLUDED_FDSOIJ4398USFLAKJLSFKJLVCSJFDF89IU4098UAKSFJDLJFSKS2

#include "../operators/operators.hpp"


namespace f
{
    template<typename T, typename A>
    const matrix<T, A>
    repmat( const matrix<T, A>& m, const std::size_t r, const std::size_t c )
    {
        assert( r );
        assert( c );

        if ( 1 == r  && 1 == c ) return m;

        if ( 1 == r ) return repmat( m, 1, c - 1 ) || m;

        if ( 1 == c ) return repmat( m, r - 1, 1 ) && m;

        return repmat( repmat( m, 1, c ), r, 1 );
    }

}//namespace f

#endif//_REPMAT_HPP_INCLUDED_FDSOIJ4398USFLAKJLSFKJLVCSJFDF89IU4098UAKSFJDLJFSKS2

