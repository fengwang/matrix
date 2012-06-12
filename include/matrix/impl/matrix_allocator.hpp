#ifndef _MATRIX_ALLOCATOR_HPP_INCLUDED_SDOIJEISFIJFFFFFFFFFFFFFFFFFFFFFFFFFFFFWE888888888888WEWEFSDF
#define _MATRIX_ALLOCATOR_HPP_INCLUDED_SDOIJEISFIJFFFFFFFFFFFFFFFFFFFFFFFFFFFFWE888888888888WEWEFSDF

#include <matrix/impl/memory_tree.hpp>
#include <misc/singleton.hpp>

#include <cstddef>

namespace feng
{

    template<typename T>
    struct matrix_allocator
    {
        typedef T    value_type;
        typedef T*   pointer;
        typedef std::size_t size_type;

        memory_tree& tree;

        matrix_allocator() : tree( singleton<memory_tree>::instance() )
        {}

        pointer allocate( size_type n )
        {
            return static_cast<pointer>(tree.allocate( n * sizeof( value_type ) ));
        }

        void deallocate( pointer p, size_type n )
        {
            tree.deallocate( p, n * sizeof( value_type ) );
        }

    };//struct matrix_allocator

}//namespace feng

#endif//_MATRIX_ALLOCATOR_HPP_INCLUDED_SDOIJEISFIJFFFFFFFFFFFFFFFFFFFFFFFFFFFFWE888888888888WEWEFSDF

