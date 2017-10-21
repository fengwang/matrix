#ifndef MDISPLAY_HPP_INCLUDED_SOFDIJW4EOIJASFLD0JKSAF9LKDJSAFLKJSFDOIJSLKJASFODIJASFOIJWREOIJSAFL3JD
#define MDISPLAY_HPP_INCLUDED_SOFDIJW4EOIJASFLD0JKSAF9LKDJSAFLKJSFDOIJSLKJASFODIJASFOIJWREOIJSAFL3JD


namespace f
{
    template<typename T,  typename A>
    void display( const matrix<T, A>& m )
    {
        std::cout << m << std::endl;
    }

    template<typename T,  typename A>
    void disp( const matrix<T, A>& m )
    {
        display( m );
    }

}//namespace f

#endif//_DISPLAY_HPP_INCLUDED_SOFDIJW4EOIJASFLD0JKSAF9LKDJSAFLKJSFDOIJSLKJASFODIJASFOIJWREOIJSAFL3JD

