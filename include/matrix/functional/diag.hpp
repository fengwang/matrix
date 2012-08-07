#ifndef _DIAG_HPP_INCLUDED_SDFOIJ48USAFLJ4UJINKFDJHORUDJKLSLKJOT4UIDFJFDLKJDSOGIJROIJDGLIJROIJGDIOJF
#define _DIAG_HPP_INCLUDED_SDFOIJ48USAFLJ4UJINKFDJHORUDJKLSLKJOT4UIDFJFDLKJDSOGIJROIJDGLIJROIJGDIOJF

#include <matrix/matrix.hpp>

#include <algorithm>
#include <cstddef>

namespace feng
{
    namespace diag_private
    {
        struct pass_diag
        {
            std::size_t pos;

            pass_diag( const std::size_t pos_ = 0 ) : pos(pos_) {}

            template<typename T, std::size_t D, typename A, typename Arg, typename ... Args>
            void operator()( matrix<T,D,A>& m, const Arg& arg, const Args& ... args ) const
            {
                m[pos][pos] = arg;
                pass_diag(pos+1)(m, args...);
            }

            template<typename T, std::size_t D, typename A, typename Arg>
            void operator()( matrix<T,D,A>& m, const Arg& arg ) const
            {
                *(m.diag_rbegin()) = arg;
            }
        };//struct pass_diag
    
    }//namespace diag_private 

    template<typename T, typename ... Tn>
    matrix<T> const diag( const T& v1, const Tn& ... vn )
    {
       const std::size_t n = 1 + sizeof...(vn);
       matrix<T> ans{n,n};
       diag_private::pass_diag()(ans, v1, vn...);
       return ans;
    }

    template<typename T, std::size_t D, typename A>
    matrix<T,D,A> const diag( const matrix<T,D,A>& m )
    {
        matrix<T,D,A> ans{m.row(), m.col()};
        std::copy( m.diag_begin(), m.diag_end(), ans.diag_begin() );
        return ans;
    }
}//namespace feng

#endif//_DIAG_HPP_INCLUDED_SDFOIJ48USAFLJ4UJINKFDJHORUDJKLSLKJOT4UIDFJFDLKJDSOGIJROIJDGLIJROIJGDIOJF

