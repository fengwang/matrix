#ifndef MREMOVE_REFERENCE_HPP_INCLUDED_OSDFJ4E8USDFLKJASFOIDJREOIULJDFSLKJHSDFLKHJGROIHJSFDLKJSFDLKJSFLKJDKJSF8IU498EFSD9
#define MREMOVE_REFERENCE_HPP_INCLUDED_OSDFJ4E8USDFLKJASFOIDJREOIULJDFSLKJHSDFLKHJGROIHJSFDLKJSFDLKJSFLKJDKJSF8IU498EFSD9

namespace f
{
    //TODO: replace legacy code with c++11 <type_traits> header
    /*
    template<typename T>
    struct remove_reference
    {
        typedef T result_type;
    };

    template<typename T>
    struct remove_reference<T&>
    {
        typedef T result_type;
    };

    template<typename T>
    struct remove_const
    {
        typedef T result_type;
    };

    template<typename T>
    struct remove_const<T const>
    {
        typedef T result_type;
    };
    */

    struct range
    {
        typedef range self_type;
        typedef std::size_t value_type;

        value_type first;
        value_type second;

        range( const value_type first_, const value_type second_ ) : first(first_), second(second_) {}
        range( const self_type& other ) : first(other.first), second(other.second) {}

        friend const self_type
        operator + ( const self_type lhs, const std::size_t n )
        {
            return self_type( lhs.first+n, lhs.second+n );
        }
        friend const self_type
        operator - ( const self_type lhs, const std::size_t n )
        {
            return self_type( lhs.first-n, lhs.second-n );
        }
    };


}

#endif//_REMOVE_REFERENCE_HPP_INCLUDED_OSDFJ4E8USDFLKJASFOIDJREOIULJDFSLKJHSDFLKHJGROIHJSFDLKJSFDLKJSFLKJDKJSF8IU498EFSD9

