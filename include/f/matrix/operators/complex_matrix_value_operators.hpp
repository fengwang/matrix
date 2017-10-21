#ifndef NTPKCELBVXUUCCFPYCNSFXADHJPHSMMJTDRXTUIRWYDPFNRHADWUABXDYMSDKPSEBUQMVQXRC
#define NTPKCELBVXUUCCFPYCNSFXADHJPHSMMJTDRXTUIRWYDPFNRHADWUABXDYMSDKPSEBUQMVQXRC

namespace f
{

    template< typename T, typename A>
    const matrix<std::complex<T>, A>
    operator + ( const matrix<std::complex<T>, A>& lhs, const T& rhs )
    {
        matrix<std::complex<T>, A> ans( lhs );
        std::transform( ans.begin(), ans.end(), ans.begin(), [rhs]( std::complex<T> const & x )
        {
            return rhs + x;
        } );
        return ans;
    }

    template< typename T, typename A>
    const matrix<std::complex<T>, A>
    operator + ( const T& lhs, const matrix<std::complex<T>, A>& rhs )
    {
        return rhs + lhs;
    }

    template< typename T, typename A>
    const matrix<std::complex<T>, A>
    operator - ( const matrix<std::complex<T>, A>& lhs, const T& rhs )
    {
        matrix<std::complex<T>, A> ans( lhs );
        std::transform( ans.begin(), ans.end(), ans.begin(), [rhs]( std::complex<T> const & x )
        {
            return x - rhs;
        } );
        return ans;
    }

    template< typename T, typename A>
    const matrix<std::complex<T>, A>
    operator - ( const T& lhs, const matrix<std::complex<T>, A>& rhs )
    {
        matrix<std::complex<T>, A> ans( rhs );
        std::transform( ans.begin(), ans.end(), ans.begin(), [lhs]( std::complex<T> const & x )
        {
            return -lhs + x;
        } );
        return ans;
    }

    template< typename T, typename A>
    const matrix<std::complex<T>, A>
    operator * ( const matrix<std::complex<T>, A>& lhs, const T& rhs )
    {
        matrix<std::complex<T>, A> ans( lhs );
        std::transform( ans.begin(), ans.end(), ans.begin(), [rhs]( std::complex<T> const & x )
        {
            return x * rhs;
        } );
        return ans;
    }

    template< typename T, typename A>
    const matrix<std::complex<T>, A>
    operator * ( const T& lhs, const matrix<std::complex<T>, A>& rhs )
    {
        return rhs * lhs;
    }

    template< typename T, typename A>
    const matrix<std::complex<T>, A>
    operator / ( const matrix<std::complex<T>, A>& lhs, const T& rhs )
    {
        matrix<std::complex<T>, A> ans( lhs );
        std::transform( ans.begin(), ans.end(), ans.begin(), [rhs]( std::complex<T> const & x )
        {
            return x / rhs;
        } );
        return ans;
    }
    template< typename T, typename A>
    const matrix<std::complex<T>, A>
    operator / ( const T& lhs, const matrix<std::complex<T>, A>& rhs )
    {
        return lhs * rhs.inverse();
    }

}//namespace f

#endif//NTPKCELBVXUUCCFPYCNSFXADHJPHSMMJTDRXTUIRWYDPFNRHADWUABXDYMSDKPSEBUQMVQXRC

