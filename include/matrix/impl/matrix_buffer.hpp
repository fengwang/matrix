/* 
 * File:   matrix_buffer.hpp
 * Author: feng
 *
 * Created on October 2, 2009, 4:03 PM
 */

#ifndef _DYNAMIC__MATRIX_BUFFER_HPP_INCLUDED
#define _DYNAMIC_MATRIX_BUFFER_HPP_INCLUDED

#include <cstddef>      //for std::size_t and std::ptrdiff_t
#include <memory>       //for allocator
#include <algorithm>    //for copy
#include <cstring>      //form memset memcpy

namespace feng{

/*
 *      This class privides memory allocation and deallocation for matrix.
 *      Stack memory allocation is far fast than heap, but must be of fixed size
 *      determined during compiling time; Heap memory is slower but can be of any
 *      size determined during runtime. matrix_buffer maintains a fixed
 *      sized stack buffer, from where allocation is made if possible; If the 
 *      requested size exceeds the buffer, a slower heap allocation is made.
 *
 */
template<
            typename Type,                          // type of elements stored
            std::size_t SPACE = 64,                 // default stack size used
            class Allocator = std::allocator<Type>  // fall-back heap allocator
        >
class matrix_buffer : public Allocator
{
public:
    typedef Type                                    value_type;
    typedef matrix_buffer                           self_type;
    typedef value_type*                             storage_type;
    typedef value_type*                             pointer;
    typedef const value_type*                       const_pointer;
    typedef value_type&                             reference;
    typedef const value_type&                       const_reference;
    typedef value_type*                             iterator;
    typedef std::reverse_iterator<iterator>         reverse_iterator;
    typedef const value_type*                       const_iterator;
    typedef std::reverse_iterator<const_iterator>   const_reverse_iterator;
    typedef std::size_t                             size_type;
    typedef std::ptrdiff_t                          difference_type;
    
private:
    // elements size that could be stored in stack buffer
    enum {  var_length = SPACE ? SPACE : 1 };
    //enum {  var_length = SPACE*sizeof(std::size_t)/sizeof(Type) };

public:

    // Description:
    //              ctor from a size
    //              allocate heap space if requested space 'Type[dims]' 
    //              exceeds stack buffer
    //
    explicit matrix_buffer(const size_type dims = 0)
    {
        items_ = dims;
        
        if (items_ <= var_length)
        {
            buffer_ = &internal_[0];
        }
        else
            buffer_ = static_cast<pointer>(Allocator::allocate(items_));
     
        memset(buffer_, 0, items_*sizeof(Type));
    }

    // Description:
    //              ctor from two iterators
    //              allocate heap space if requested space 
    //              'distance(begin_, end_)' exceeds stack buffer
    //
    template<typename Input_Iterator>
    matrix_buffer(Input_Iterator begin_, Input_Iterator end_)
    {
        if (!is_internal_alloc())
        {
            Allocator::deallocate(buffer_, items_);
        }
        items_ = std::distance(begin_, end_);
        if (items_ <= var_length)
            buffer_ = &internal_[0];
        else
            buffer_ = static_cast<pointer>(Allocator::allocate(items_));
    
        std::copy(begin_, end_, begin());
    }

    // Description:
    //              dtor
    //              delete heap memory if allocated
    //
    ~matrix_buffer()
    {
        if (!is_internal_alloc())
            Allocator::deallocate(buffer_, items_);
    }

public:

    // Description:
    //              copy ctor, implemented by operator=
    //
    template< typename T, std::size_t D, typename A >
    matrix_buffer(const matrix_buffer<T,D,A>& rhs)
    {
        operator = <T,D,A> (rhs);
    }
    
    // Description:
    //              copy ctor, implemented by do_copy
    //
    template< typename T, std::size_t D, typename A >
    self_type &
    operator=(const matrix_buffer<T,D,A>& rhs)
    {
        do_copy<T,D,A> (rhs);
        return *this;
    }

public:

    // Description:
    //              test if matrix_buffer holds any elements
    // Returns:
    //              true    :   no elements hold
    //              false   :   not empty
    //
    bool 
    empty()const
    {
        return ( 0 == items_);
    }

    // Description:
    // Return:
    //              elements stored in matrix_buffer
    //
    size_type 
    size()const
    {
        return items_;
    }

    // Description:
    // Return:      elements that could be stored in stack buffer
    //              
    size_type 
    internal_size()const
    {
        return var_length;
    }

public:

    // Description:
    // Return:      the first element position stroed in matrix_buffer
    //
    iterator 
    begin()
    {
        return &buffer_[0];
    }

    // Description:
    // Return:      the first element position stroed in matrix_buffer
    const_iterator 
    begin()const
    {
        return &buffer_[0];
    }

    // Description:
    // Return:      the first position following the stroed in matrix_buffer
    //
    iterator 
    end()
    {
        return begin() + size();
    }

    // Description:
    // Return:      the first position following the stroed in matrix_buffer
    //
    const_iterator 
    end()const
    {
        return begin() + size();
    }
    
    // see begin()
    //
    reverse_iterator 
    rbegin()
    {
        return reverse_iterator( end() );
    }
    
    // see begin()
    //
    const_reverse_iterator 
    rbegin() const
    {
        return const_reverse_iterator( end() );
    }
    
    // see end()
    //
    reverse_iterator 
    rend()
    {
        return reverse_iterator( begin() );
    }
    
    // see end()
    //
    const_reverse_iterator 
    rend() const
    {
        return const_reverse_iterator( begin() );
    }

public:
    
    // Description :
    //                  lvalue of elements stored in matrix_buffer
    //
    reference 
    operator[](const size_type index)
    {
        return buffer_[index];
    }

    // Description :
    //                  const lvalue of elements stored in matrix_buffer
    //
    const_reference 
    operator[](const size_type index) const
    {
        return buffer_[index];
    }

private:

    // Description:
    //                  ctor implementation
    //
    template<typename T, std::size_t D, typename A>
    void do_copy(const matrix_buffer<T,D,A>& rhs)
    {
        const size_type dis = std::distance(rhs.begin(), rhs.end());

        if (!is_internal_alloc())
        {
            buffer_ = static_cast<pointer>(Allocator::allocate(items_));
        }
        items_ = dis;
        if (items_ <= var_length)
            buffer_ = &internal_[0];
        else
        buffer_ = static_cast<pointer>(Allocator::allocate(items_));
        std::copy(rhs.begin(), rhs.end(), begin());
    }

    // Description:
    //                  ctor implementation
    //
    void do_copy(const self_type& rhs)
    {
        const size_type dis = std::distance(rhs.begin(), rhs.end());

        if (!is_internal_alloc())
        {
            buffer_ = static_cast<pointer>(Allocator::allocate(items_));
        }
        items_ = dis;
        if (items_ <= var_length)
            buffer_ = &internal_[0];
        else
        buffer_ = static_cast<pointer>(Allocator::allocate(items_));
        memcpy(buffer_, rhs.buffer_, items_*sizeof(Type));
    }

public:

    // Description:
    //                  ctor implementation
    //
    template<typename Input_Iterator>
    void assign(Input_Iterator begin_, Input_Iterator end_)
    {
        const size_type dis = std::distance(begin_, end_);

        if (!is_internal_alloc())
        {
            buffer_ = static_cast<pointer>(Allocator::allocate(items_));
        }
        items_ = dis;
        if (items_ <= var_length)
            buffer_ = &internal_[0];
        else
            buffer_ = static_cast<pointer>(Allocator::allocate(items_));
        std::copy(begin_, end_, begin());
    }

private:

    // Description:
    //                  test if stack memory used
    // Returns:
    //                  true    :   using stack memory
    //                  false   :   using heap memory
    //
    bool is_internal_alloc()const
    {
        return items_ <= var_length;
    }

public: 

    // Description:
    //                  ctor
    //
    matrix_buffer(const self_type& rhs)
    : Allocator(rhs) 
    {
        operator = (rhs);
    }

    // Description:
    //                  ctor
    //
    self_type & 
    operator=(const self_type& rhs)
    {
        do_copy(rhs);
        return *this;
    }

public:

    matrix_buffer( self_type&& other )
    {
        std::copy( other.internal_, other.internal_+var_length, internal_ );
        buffer_ = other.buffer_;
        items_ = other.items_;
    }

    self_type& operator = ( self_type&& other )
    {
        std::copy( other.internal_, other.internal_+var_length, internal_ );
        buffer_ = other.buffer_;
        items_ = other.items_;
        return *this;
    }

private:
    // stack buffer
    value_type internal_[var_length];
    // ptr to elements stored
    pointer buffer_;
    // size of elements stored
    size_type items_;

};

}//namespace feng

#endif  /* _DYNAMIC_MATRIX_BUFFER_HPP_INCLUDED */
