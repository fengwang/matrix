#ifndef _MEMORY_DIRECT_HPP_INCLUDED_SFDOI498ULSFKJKVJKSFDJAOIU4POSFDIJAOPISAIOJSFOIRRU8SDIOJUIGIOJFGOIF
#define _MEMORY_DIRECT_HPP_INCLUDED_SFDOI498ULSFKJKVJKSFDJAOIU4POSFDIJAOPISAIOJSFOIRRU8SDIOJUIGIOJFGOIF

#include <cstddef>
#include <cassert>
#include <cstdlib>

namespace feng
{
    struct memory_direct
    {
        typedef std::size_t      size_type;
        typedef void*            address_type;

        address_type allocate( size_type n ) const
        {
            return std::malloc(n);
        }

        void deallocate( address_type p, size_type ) const
        {
            std::free(p);
        }

    };

}//namespace feng

#endif//_MEMORY_DIRECT_HPP_INCLUDED_SFDOI498ULSFKJKVJKSFDJAOIU4POSFDIJAOPISAIOJSFOIRRU8SDIOJUIGIOJFGOIF

