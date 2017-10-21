#ifndef MDIAG_HPP_INCLUDED_SDFOIJ48USAFLJ4UJINKFDJHORUDJKLSLKJOT4UIDFJFDLKJDSOGIJROIJDGLIJROIJGDIOJF
#define MDIAG_HPP_INCLUDED_SDFOIJ48USAFLJ4UJINKFDJHORUDJKLSLKJOT4UIDFJFDLKJDSOGIJROIJDGLIJROIJGDIOJF

namespace f
{
    /*
        Input:
                a random matrix m with diag element { $ & ^ % ... }, row size r and col size c
                [ $ * * * ]
                [ * & * * ]
                [ * * ^ * ]
                [ * * * % ]
                [ * * * * ]
                a int number offset
        Output:
                a square matrix with size min(|offset|+r, |offset|+c)
                if offset is positive then put diag element in the offset-th upper diag,
                [ 0 $ 0 0 0 ]
                [ 0 0 & 0 0 ]
                [ 0 0 0 ^ 0 ] with offset = 1
                [ 0 0 0 0 % ]
                [ 0 0 0 0 0 ]
                otherwise, put in the -offset th lower diag
                [ 0 0 0 0 0 ]
                [ $ 0 0 0 0 ]
                [ 0 & 0 0 0 ] with offset = -1
                [ 0 0 ^ 0 0 ]
                [ 0 0 0 % 0 ]
      */
    template<typename T,  typename A>
    matrix<T, A> const diag( const matrix<T, A>& m, const std::ptrdiff_t offset = 0 )
    {
        const std::size_t dim = std::min( m.row(), m.col() ) + ( offset > 0 ? offset : -offset );
        matrix<T, A> ans{dim, dim};
        std::copy( m.diag_begin(), m.diag_end(), ans.diag_begin( offset ) );
        return ans;
    }

    namespace diag_private
    {
        template<typename Itor>
        matrix<typename std::iterator_traits<Itor>::value_type> const
        impl_diag( Itor first, Itor last, const std::ptrdiff_t offset = 0 )
        {
            std::size_t dim = std::distance( first, last ) + ( offset > 0 ? offset : -offset );
            matrix<typename std::iterator_traits<Itor>::value_type> ans{ dim, dim };
            std::copy( first, last, ans.diag_begin( offset ) );
            return ans;
        }
    }//namespace diag_private

    template<typename T, typename A>
    matrix<T> const diag( const std::vector<T, A>& v, const std::ptrdiff_t offset = 0 )
    {
        return diag_private::impl_diag( v.begin(), v.end(), offset );
    }

    template<typename T, typename A>
    matrix<T> const diag( const std::deque<T, A>& v, const std::ptrdiff_t offset = 0 )
    {
        return diag_private::impl_diag( v.begin(), v.end(), offset );
    }

    template<typename T, typename C, typename A>
    matrix<T> const diag( const std::set<T, C, A>& v, const std::ptrdiff_t offset = 0 )
    {
        return diag_private::impl_diag( v.begin(), v.end(), offset );
    }

    template<typename T, typename C, typename A>
    matrix<T> const diag( const std::multiset<T, C, A>& v, const std::ptrdiff_t offset = 0 )
    {
        return diag_private::impl_diag( v.begin(), v.end(), offset );
    }

    template<typename T>
    matrix<T> const diag( const std::valarray<T>& v, const std::ptrdiff_t offset = 0 )
    {
        return diag_private::impl_diag( std::begin( v ), std::end( v ), offset );
    }

    namespace make_diag_private
    {
        struct impl_make_diag
        {
            std::size_t pos;

            impl_make_diag( const std::size_t pos_ = 0 ) : pos( pos_ ) {}

            template<typename T,  typename A, typename Arg, typename ... Args>
            void operator()( matrix<T, A>& m, const Arg& arg, const Args& ... args ) const
            {
                m[pos][pos] = arg;
                impl_make_diag( pos + 1 )( m, args... );
            }

            template<typename T,  typename A, typename Arg>
            void operator()( matrix<T, A>& m, const Arg& arg ) const
            {
                *( m.diag_rbegin() ) = arg;
            }
        };//struct impl_make_diag

    }//namespace make_diag_private

    /*
        Input:
                a series of numbers: a, b, c, d, e, ...
        Output:
                a square diag matrix looks like
                    [ a 0 0 0 0 ... ]
                    [ 0 b 0 0 0 ... ]
                    [ 0 0 c 0 0 ... ]
                    [ 0 0 0 d 0 ... ]
                    [ 0 0 0 0 e ... ]
                    [ . . . . . ... ]
                    [ . . . . . ... ]
                    [ . . . . . ... ]
    */
    // !!!!
    // TODO :
    //          find out why make_diag can override function 'diag' silently if its name is changed to 'diag'
    // !!!!
    template<typename T, typename ... Tn>
    //matrix<T> const diag( const T& v1, const Tn& ... vn )
    matrix<T> const make_diag( const T& v1, const Tn& ... vn )
    {
        const std::size_t n = 1 + sizeof...( vn );
        matrix<T> ans{n, n};
        make_diag_private::impl_make_diag()( ans, v1, vn... );
        return ans;
    }




}//namespace f

#endif//_DIAG_HPP_INCLUDED_SDFOIJ48USAFLJ4UJINKFDJHORUDJKLSLKJOT4UIDFJFDLKJDSOGIJROIJDGLIJROIJGDIOJF

