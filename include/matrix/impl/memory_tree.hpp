#ifndef _MEMORY_TREE_HPP_INCLUDED_SFDOI498ULSFKJKVJKSFDJAOIU4POSFDIJAOPISAIOJSFOIRRU8SDIOJUIGIOJFGOIF
#define _MEMORY_TREE_HPP_INCLUDED_SFDOI498ULSFKJKVJKSFDJAOIU4POSFDIJAOPISAIOJSFOIRRU8SDIOJUIGIOJFGOIF

#include <map>
#include <algorithm>
#include <mutex>
#include <cstddef>
#include <cassert>
#include <cstdlib>

namespace feng
{
    struct memory_tree
    {
        typedef std::size_t                                 size_type;
        typedef void*                                       address_type;
        typedef std::multimap< size_type, address_type >    size_address_associate_type;
        typedef std::mutex                                  mutex_type;
        typedef std::lock_guard<mutex_type>                 lock_guard_type;
        typedef unsigned long                               unit_type;

        size_address_associate_type     available_tree_;
        mutable mutex_type              mutex_available_tree_;
        size_address_associate_type     occupied_tree_;
        mutable mutex_type              mutex_occupied_tree_;

        /*
         * 1) free available tree
         * 2) free occupied tree
         */
        ~memory_tree()
        {
            // 1)
            destroy_idle();

            // 2)
            for ( auto it = occupied_tree_.begin(); it != occupied_tree_.end(); ++it )
            { free( ( ( *it ).second ) ); }

            occupied_tree_.clear();
        }

        /*
         * 1) free cached memory
         * 2) free tree node
         */
        void destroy_idle()
        {
            lock_guard_type l { mutex_available_tree_ };

            // 1)
            for ( auto it = available_tree_.begin(); it != available_tree_.end(); ++it )
            { free( ( ( *it ).second ) ); }

            // 2)
            available_tree_.clear();
        }

        /*
         * 1) try to find an idle node from available tree
         * 2) if found
         *    2.1) insert the node to occupied tree
         *    2.2) delete the node from available tree
         * 3) if not found
         *    3.1) alloc a new node from heap
         *    3.2) insert the node to occupied tree
         */
        void* allocate( size_type n )
        {
            n = ((n >> 6) + 1) << 6;

            mutex_available_tree_.lock();
            // 1)
            auto it = available_tree_.find( n );

            // 2)
            if ( it != available_tree_.end() )
            {
                lock_guard_type lo1 { mutex_occupied_tree_ };
                auto mem = (*it).second;
                // 2.1)
                occupied_tree_.insert( *it );
                // 2.2)
                available_tree_.erase( it );
                return mem;
            }

            mutex_available_tree_.unlock();
            lock_guard_type lo2 { mutex_occupied_tree_ };
            // 3.1)
            auto node = std::make_pair( n, malloc( n ) );
            // 3.2
            occupied_tree_.insert( node );
            return node.second;
        }

        /*
         * 1) find lower boundary with key n within occupied tree
         * 2) find upper boundary with key n within occupied tree
         * 3) find target node within range [low_b, upp_b]
         * 4) insert the node to available tree
         * 5) move the node from occupied tree
         */
        void deallocate( void* p, size_t n )
        {
            n = ((n >> 6) + 1) << 6;

            lock_guard_type lo { mutex_occupied_tree_ };
            // 1)
            auto low_b = occupied_tree_.lower_bound( n );
            // 2)
            auto upp_b = occupied_tree_.upper_bound( n );
            assert( low_b != upp_b );

            // 3)
            for ( ; low_b != upp_b; ++low_b )
                if ( p == ( *low_b ).second )
                {
                    // 4)
                    lock_guard_type la { mutex_available_tree_ };
                    available_tree_.insert( *low_b );
                    // 5)
                    occupied_tree_.erase( low_b );
                    return;
                }

            assert( !"Should never reach here!" );
        }

    };

}//namespace feng

#endif//_MEMORY_TREE_HPP_INCLUDED_SFDOI498ULSFKJKVJKSFDJAOIU4POSFDIJAOPISAIOJSFOIRRU8SDIOJUIGIOJFGOIF

