#ifndef MEYE_HPP_INCLUDED_SOFIJ398UAFSLKJ3498YUFSKDHJCVJKDFSAUH948YASKFHOQE9YSDFY7478YTSFUKDHREUFKUH
#define MEYE_HPP_INCLUDED_SOFIJ398UAFSLKJ3498YUFSKDHJCVJKDFSAUH948YASKFHOQE9YSDFY7478YTSFUKDHREUFKUH

namespace f
{
    namespace eye_private
    {
        template< typename T >
        struct one_maker
        {
            T operator()() const
            {
                return T( 1 );
            }
        };

        template< typename T >
        struct one_maker< std::complex<T>>
        {
            std::complex<T> const operator()() const
            {
                return std::complex<T>( T( 1 ), T( 0 ) );
            }
        };
    };


    template<typename T,

             typename A = std::allocator<typename std::remove_const<typename std::remove_reference<T>::type>::type>>
    matrix<T, A> const eye( const std::size_t r, const std::size_t c )
    {
        matrix<T> ans{ r, c };
        //std::fill( ans.diag_begin(), ans.diag_end(), T(1) );
        std::fill( ans.diag_begin(), ans.diag_end(), eye_private::one_maker<T>()() );
        return ans;
    }


    template<typename T,

             typename A = std::allocator<typename std::remove_const<typename std::remove_reference<T>::type>::type>>
    matrix<T, A> const eye( const std::size_t n )
    {
        return eye<T, A>( n, n );
    }

    template<typename T,

             typename A = std::allocator<typename std::remove_const<typename std::remove_reference<T>::type>::type>>
    matrix<T, A> const eye( const matrix<T, A>& m )
    {
        return eye<T, A>( m.row(), m.col() );
    }

    template<typename Matrix>
    Matrix const eye( const std::size_t r, const std::size_t c, const Matrix& )
    {
        Matrix m( r, c );
        std::fill( m.diag_begin(), m.diag_end(), typename Matrix::value_type( 1 ) );
        return m;
    }

    template<typename Matrix>
    Matrix const eye( const std::size_t n, const Matrix& m )
    {
        return eye( n, n, m );
    }

}//namespace f

#endif//_EYE_HPP_INCLUDED_SOFIJ398UAFSLKJ3498YUFSKDHJCVJKDFSAUH948YASKFHOQE9YSDFY7478YTSFUKDHREUFKUH

