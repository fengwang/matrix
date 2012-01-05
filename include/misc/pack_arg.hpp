#ifndef _PACK_ARG_HPP_INCLUDED_SDOIFJ498YUSFDKJHSDAFKJHCVKJDHFSIU43OYAHUSDFLKJVCIEWUF4YU7SDKFUHAKSJFHDIUWAEFKSAHFREIUFHKS
#define _PACK_ARG_HPP_INCLUDED_SDOIFJ498YUSFDKJHSDAFKJHCVKJDHFSIU43OYAHUSDFLKJVCIEWUF4YU7SDKFUHAKSJFHDIUWAEFKSAHFREIUFHKS

namespace feng
{

#if 0
        Example:
            std::vector<double> v;
            feng::pack_arg( back_inserter(v), 0.0, 1.0, 2.0. 3.0 );
#endif

template<typename OI, typename V>
OI pack_arg( OI oi_, V v )
{
    *oi_++ = v;
    return oi_;
}

template<typename OI, typename V, typename... ARG>
OI pack_arg( OI oi_, V v, ARG... arg )
{
    *oi_++ = v;
    return pack_arg( oi_, arg... );
}

}//namespace feng

#endif//_PACK_ARG_HPP_INCLUDED_SDOIFJ498YUSFDKJHSDAFKJHCVKJDHFSIU43OYAHUSDFLKJVCIEWUF4YU7SDKFUHAKSJFHDIUWAEFKSAHFREIUFHKS

