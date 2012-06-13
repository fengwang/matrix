#ifndef _MEMORY_LIST_HPP_INCLUDED_SFDOI498USAFOJI4089USAFLJNSDAKFJHSAOIF4398YSAOFIJHOEUITISFJOIUROI
#define _MEMORY_LIST_HPP_INCLUDED_SFDOI498USAFOJI4089USAFLJNSDAKFJHSAOIF4398YSAOFIJHOEUITISFJOIUROI

#include <map>
#include <algorithm>
#include <mutex>
#include <cstddef>
#include <cassert>
#include <cstdlib>
#include <memory>
#include <list>
#include <cmath>

namespace feng
{
    namespace memory_list_private
    {
        struct left_most_position
        {
            std::size_t operator()( std::size_t n ) const 
            {
               return std::ceil(std::log2(n)); 
            }
        };
    }//namespace memory_list_private 

    struct memory_list
    {
        typedef std::size_t                                 size_type;
        typedef void*                                       address_type;
        typedef std::list<address_type>                    container_type;
        typedef std::map< size_type, container_type >       size_container_associate_type;
        typedef std::mutex                                  mutex_type;
        typedef std::lock_guard<mutex_type>                 lock_guard_type;

        size_container_associate_type       available_tree_;
        mutable mutex_type                  mutex_available_tree_;
        size_container_associate_type       occupied_tree_;
        mutable mutex_type                  mutex_occupied_tree_;

        /*
         * 1) free available tree
         * 2) free occupied tree
         *  2.1) for every list in the tree
         *   2.2.1) for every memory in the list
         *  2.2) clear the list
         */
        ~memory_list()
        {
            // 1)
            destroy_idle();

            // 2) // 2.1)
            for ( auto& size_list_ : occupied_tree_ ) // ( size_type, list<void*> )
            {   // 2.1.1)
                for ( auto add : size_list_.second ) // ( void* )
                    std::free( add );
                // 2.2)
                size_list_.second.clear();
            }

            occupied_tree_.clear();
        }

        /*
         * 1) free cached memory
         * 2) free tree node
         */
        void destroy_idle()
        {
            lock_guard_type l { mutex_available_tree_ };

            for ( auto& size_list_ : available_tree_ ) // ( size_type, list<void*> )
            { 
                for ( auto add : size_list_.second ) // ( void* )
                    // 1)
                    std::free( add );
                size_list_.second.clear();
            }
            // 2)
            available_tree_.clear();
        }

        /*
         * 1) try to find an idle node from available tree
         * 2) if found
         *   2.1) if not empty
         *   2.2) copy list top to occupied_tree_ 
         *   2.3) pop up the top of available_tree_ 
         * 3) if not found
         *   3.1) malloc memeory
         *   3.2) push to occupied tree
         */
        void* allocate( size_type n )
        {
            n = memory_list_private::left_most_position()(n);
            n = n > 6 ? n : 6;
            size_t N = 1 << n;

            mutex_available_tree_.lock();
            // 1)
            auto it = available_tree_.find( n );

            // 2)
            if ( it != available_tree_.end() )
                // 2.1)
                if ( !((*it).second.empty()) )
                {
                    lock_guard_type lo { mutex_occupied_tree_ };
                    auto mem = (*it).second.front();
                    // 2.2)
                    occupied_tree_[n].push_front( mem );
                    // 2.3)
                    (*it).second.pop_front();
                    return mem;
                }
            mutex_available_tree_.unlock();

            // 3)
            // 3.1)
            auto mem = std::malloc( N );
            // 3.2)
            lock_guard_type lo { mutex_occupied_tree_ };
            occupied_tree_[n].push_front(mem);
            return mem;
        }

        /*
         * 1) find memory block p within occupied_tree
         * 2) check if have enough free space within availaible tree
         *   2.1) remove block p from occupied tree and free it
         * 3) copy memory block p to available_tree_
         * 4) remove block p
         */
        void deallocate( void* p, size_t n )
        {
            n = memory_list_private::left_most_position()(n);
            n = n > 6 ? n : 6;

            lock_guard_type lo { mutex_occupied_tree_ };
            
            for ( auto itor = occupied_tree_[n].begin(); itor != occupied_tree_[n].end(); ++itor )
            {   // 1) 
                if ( (*itor) == p )
                {
                    lock_guard_type la { mutex_available_tree_ };
                    // 2)
                    if ( available_tree_[n].size() >= 1024 )
                    {   // 2.1)
                        occupied_tree_[n].erase( itor );
                        std::free( p );
                        return;
                    }
                    // 3)
                    available_tree_[n].push_front(p);
                    // 4)
                    occupied_tree_[n].erase( itor );

                    return;
                }
            }

            assert( !"Should never reach here!" );
        }
    };

}//namespace feng

#endif//_MEMORY_LIST_HPP_INCLUDED_SFDOI498USAFOJI4089USAFLJNSDAKFJHSAOIF4398YSAOFIJHOEUITISFJOIUROI

