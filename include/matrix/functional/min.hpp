#ifndef _MIN_HPP_INCLUDED_SFDOJI4938YUALFKJALKHDSFIUHVCJSBANFDKIJUH4EU7HFDSKJHASIUH4E87UGHFASIDUHASFI
#define _MIN_HPP_INCLUDED_SFDOJI4938YUALFKJALKHDSFIUHVCJSBANFDKIJUH4EU7HFDSKJHASIUH4E87UGHFASIDUHASFI

#include <matrix/matrix.hpp>

#include <algorithm>
#include <cassert>
#include <cstddef>

namespace feng
{
    template< typename T, std::size_t D, typename A>
    T const 
    min( const matrix<T,D,A>& m )
    {
        assert( m.size() ) > 0;

        return std::min_element( m.begin(), m.end() );
    }

}//namespace feng

#endif//_MIN_HPP_INCLUDED_SFDOJI4938YUALFKJALKHDSFIUHVCJSBANFDKIJUH4EU7HFDSKJHASIUH4E87UGHFASIDUHASFI

