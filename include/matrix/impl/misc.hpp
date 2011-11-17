#ifndef _REMOVE_REFERENCE_HPP_INCLUDED_OSDFJ4E8USDFLKJASFOIDJREOIULJDFSLKJHSDFLKHJGROIHJSFDLKJSFDLKJSFLKJDKJSF8IU498EFSD9
#define _REMOVE_REFERENCE_HPP_INCLUDED_OSDFJ4E8USDFLKJASFOIDJREOIULJDFSLKJHSDFLKHJGROIHJSFDLKJSFDLKJSFLKJDKJSF8IU498EFSD9

namespace feng
{
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
}

#endif//_REMOVE_REFERENCE_HPP_INCLUDED_OSDFJ4E8USDFLKJASFOIDJREOIULJDFSLKJHSDFLKHJGROIHJSFDLKJSFDLKJSFLKJDKJSF8IU498EFSD9

