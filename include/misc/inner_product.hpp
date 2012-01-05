#ifndef _INNER_PRODUCT_HPP_INCLUDED_SFDOIJ4T98UDSFLKJHVBJNDSAKJHLDKJVHFKDJHGI98YDHGSLKGIU6HKDGHSKGJH54KUHGKBJVHKHVCDGIUHG
#define _INNER_PRODUCT_HPP_INCLUDED_SFDOIJ4T98UDSFLKJHVBJNDSAKJHLDKJVHFKDJHGI98YDHGSLKGIU6HKDGHSKGJH54KUHGKBJVHKHVCDGIUHG

namespace feng
{

template< typename II1, typename II2, typename T >
T inner_product( II1 ii1_, II1 _ii1, II2 ii2_, T init )
{
    while ( ii1_ != _ii1 )
        init += (*ii1_++) * (*ii2_++);
    return init;
}

template< typename II1, typename II2, typename T, typename OP >
T inner_product( II1 ii1_, II1 _ii1, II2 ii2_, T init, OP op )
{
    while ( ii1_ != _ii1 )
        init += op( *ii1_++, *ii2_++ );
    return init;
}

template< typename II1, typename II2, typename II3, typename T >
T inner_product( II1 ii1_, II1 _ii1, II2 ii2_, II3 ii3_, T init )
{
    while ( ii1_ != _ii1 )
        init += (*ii1_++) * (*ii2_++) * (*ii3_++);
    return init;
}

template< typename II1, typename II2, typename II3, typename T, typename OP >
T inner_product( II1 ii1_, II1 _ii1, II2 ii2_, II3 ii3_, T init, OP op )
{
    while ( ii1_ != _ii1 )
        init += op( *ii1_++, *ii2_++, *ii3_++ );
    return init;
}

template< typename II1, typename II2, typename II3, typename II4, typename T >
T inner_product( II1 ii1_, II1 _ii1, II2 ii2_, II3 ii3_, II4 ii4_, T init )
{
    while ( ii1_ != _ii1 )
        init += (*ii1_++) * (*ii2_++) * (*ii3_++) * (*ii4_++);
    return init;
}

template< typename II1, typename II2, typename II3, typename II4, typename T, typename OP >
T inner_product( II1 ii1_, II1 _ii1, II2 ii2_, II3 ii3_, II4 ii4_, T init, OP op )
{
    while ( ii1_ != _ii1 )
        init += op( *ii1_++, *ii2_++, *ii3_++, *ii4_++ );
    return init;
}


}//feng

#endif//_INNER_PRODUCT_HPP_INCLUDED_SFDOIJ4T98UDSFLKJHVBJNDSAKJHLDKJVHFKDJHGI98YDHGSLKGIU6HKDGHSKGJH54KUHGKBJVHKHVCDGIUHG

