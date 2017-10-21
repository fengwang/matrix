#ifndef MTRIU_HPP_INCLUDED_SDFOIJU4309U8OFISDJ89TUR598ULSDFHJKJFDKJHREIOJHGDIOGDEIOJREIJDGLKJROIJGOF
#define MTRIU_HPP_INCLUDED_SDFOIJU4309U8OFISDJ89TUR598ULSDFHJKJFDKJHREIOJHGDIOGDEIOJREIJDGLKJROIJGOF

namespace f
{
    template<typename T, typename A >
    matrix<T, A> const triu( const matrix<T, A>& m )
    {
        matrix<T, A> ans{m.row(), m.col()};

        for ( std::size_t i = 0; i != m.col(); ++i )
            std::copy( m.upper_diag_cbegin( i ), m.upper_diag_cend( i ), ans.upper_diag_begin( i ) );

        return ans;
    }

}//namespace f

#endif//_TRIU_HPP_INCLUDED_SDFOIJU4309U8OFISDJ89TUR598ULSDFHJKJFDKJHREIOJHGDIOGDEIOJREIJDGLKJROIJGOF

