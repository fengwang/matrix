#ifndef MZEROS_HPP_INCLUDED_SOFD4IJ489USAFIJSFKLJXVCNSFDJSFDJKLHALSFKJHASOFIU498SDFAIJHORUISAIFDJOEIR
#define MZEROS_HPP_INCLUDED_SOFD4IJ489USAFIJSFKLJXVCNSFDJSFDJKLHALSFKJHASOFIU498SDFAIJHORUISAIFDJOEIR

namespace f
{
    template<typename T,
             typename A = std::allocator<typename std::remove_const<typename std::remove_reference<T>::type>::type>>
    matrix<T, A> const zeros( const std::size_t r, const std::size_t c )
    {
        matrix<T, A> ans{ r, c, T( 0 ) };
        return ans;
    }

    template<typename T,
             typename A = std::allocator<typename std::remove_const<typename std::remove_reference<T>::type>::type>>
    matrix<T, A> const zeros( const std::size_t n )
    {
        return zeros<T, A>( n, n );
    }

    template<typename T, typename A >
    matrix<T, A> const zeros( const matrix<T, A>& m )
    {
        return zeros<T, A>( m.row(), m.col() );
    }

    template<typename T, typename A >
    matrix<T, A> const zeros( const matrix<T, A>&, const std::size_t r, const std::size_t c )
    {
        return zeros<T, A>( r, c );
    }

}//namespace f

#endif//_ZEROS_HPP_INCLUDED_SOFD4IJ489USAFIJSFKLJXVCNSFDJSFDJKLHALSFKJHASOFIU498SDFAIJHORUISAIFDJOEIR

