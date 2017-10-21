#ifndef MTRIL_HPP_INCLUDED_SDFOIJU4309U8OFISDJ89TUR598ULSDFHJKJFDKJHREIOJHGDIOGDEIOJREIJDGLKJROIJGOF
#define MTRIL_HPP_INCLUDED_SDFOIJU4309U8OFISDJ89TUR598ULSDFHJKJFDKJHREIOJHGDIOGDEIOJREIJDGLKJROIJGOF

namespace f
{
    template<typename T, typename A >
    matrix<T, A> const tril( const matrix<T, A>& m )
    {
        matrix<T, A> ans{m.row(), m.col()};

        for ( std::size_t i = 0; i != m.col(); ++i )
            std::copy( m.lower_diag_cbegin( i ), m.lower_diag_cend( i ), ans.lower_diag_begin( i ) );

        return ans;
    }

}//namespace f

#endif//_TRIL_HPP_INCLUDED_SDFOIJU4309U8OFISDJ89TUR598ULSDFHJKJFDKJHREIOJHGDIOGDEIOJREIJDGLKJROIJGOF

