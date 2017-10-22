#ifndef MONES_HPP_INCLUDED_SOFD4IJ489USAFIJSFKLJXVCNSFDJSFDJKLHALSFKJHASOFIU498SDFAIJHORUISAIFDJOEIR
#define MONES_HPP_INCLUDED_SOFD4IJ489USAFIJSFKLJXVCNSFDJSFDJKLHALSFKJHASOFIU498SDFAIJHORUISAIFDJOEIR


namespace f
{
    template<typename T,

             typename A = std::allocator<typename std::remove_const<typename std::remove_reference<T>::type>::type>>
    matrix<T, A> const ones( const std::size_t r, const std::size_t c )
    {
        matrix<T> ans{ r, c, T( 1 ) };
        return ans;
    }

    template<typename T,

             typename A = std::allocator<typename std::remove_const<typename std::remove_reference<T>::type>::type>>
    matrix<T, A> const ones( const std::size_t n )
    {
        return ones<T, A>( n, n );
    }

    template<typename T,

             typename A = std::allocator<typename std::remove_const<typename std::remove_reference<T>::type>::type>>
    matrix<T, A> const ones( const matrix<T, A>& m )
    {
        return ones<T, A>( m.row(), m.col() );
    }

}//namespace f

#endif//_ONES_HPP_INCLUDED_SOFD4IJ489USAFIJSFKLJXVCNSFDJSFDJKLHALSFKJHASOFIU498SDFAIJHORUISAIFDJOEIR

