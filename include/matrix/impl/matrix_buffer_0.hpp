// template specialization when STACK buffer is totally not used
// see matrix_buffer.hpp
//
#ifndef _DYNAMIC_MATRIX_BUFFER_0_HPP_INCLUDED
#define	_DYNAMIC_MATRIX_BUFFER_0_HPP_INCLUDED

#include <cstddef> 		//for std::size_t and std::ptrdiff_t
#include <memory> 		//for allocator
#include <algorithm> 	//for copy
#include <cstring> 		//form memset memcpy

namespace dynamic{

template<
        	typename Type,
			class Allocator // = std::allocator<Type>
        >
class matrix_buffer<Type, 0, Allocator> : public Allocator
{

public:
    typedef Type 											value_type;
    typedef matrix_buffer 							self_type;
    typedef value_type* 									storage_type;
    typedef value_type* 									pointer;
    typedef const value_type* 								const_pointer;
    typedef value_type& 									reference;
    typedef const value_type& 								const_reference;
    typedef value_type* 									iterator;
	typedef std::reverse_iterator<iterator> 				reverse_iterator;
    typedef const value_type* 								const_iterator;
	typedef std::reverse_iterator<const_iterator> 			const_reverse_iterator;
    typedef std::size_t 									size_type;
    typedef std::ptrdiff_t 									difference_type;

public:

    explicit matrix_buffer(const size_type dims = 0)
    {
        items_ = dims;
		buffer_ = 0;
        
		if ( dims )
		{
			buffer_ = static_cast<pointer>(Allocator::allocate(items_));
			memset(buffer_, 0, items_*sizeof(Type));
		}
    }

    template<typename Input_Iterator>
    matrix_buffer(Input_Iterator begin_, Input_Iterator end_)
    {
        items_ = std::distance(begin_, end_);
		buffer_ = static_cast<pointer>(Allocator::allocate(items_));
		std::copy(begin_, end_, begin());
    }

    ~matrix_buffer()
    {
		if ( buffer_ )
			Allocator::deallocate(buffer_, items_);
    }

public:

    template< typename T, std::size_t D, typename A >
    matrix_buffer(const matrix_buffer<T,D,A>& rhs)
    {
        operator=<T,D,A>(rhs);
    }

    template< typename T, std::size_t D, typename A >
    self_type & operator=(const matrix_buffer<T,D,A>& rhs)
    {
        do_copy<T,D,A>(rhs);
        return *this;
    }

public:

    bool empty()const
    {
        return ( 0 == items_);
    }

    size_type size()const
    {
        return items_;
    }

    size_type internal_size()const
    {
        return 0;
    }

public:

    iterator begin()
    {
        return buffer_;
    }


    const_iterator begin()const
    {
        return buffer_;
    }

    iterator end()
    {
        return begin() + size();
    }

    const_iterator end()const
    {
        return begin() + size();
    }
	
	reverse_iterator rbegin()
	{
		return reverse_iterator( end() );
	}
	
	const_reverse_iterator rbegin() const
	{
		return const_reverse_iterator( end() );
	}
	
	reverse_iterator rend()
	{
		return reverse_iterator( begin() );
	}
	
	const_reverse_iterator rend() const
	{
		return const_reverse_iterator( begin() );
	}

public:

    reference operator[](const size_type index)
    {
        return buffer_[index];
    }

    const_reference operator[](const size_type index) const
    {
        return buffer_[index];
    }

private:

    template<typename T, std::size_t D, typename A>
    void do_copy(const matrix_buffer<T,D,A>& rhs)
    {
		if ( buffer_ )
			Allocator::deallocate(buffer_, items_);
        items_ = std::distance(rhs.begin(), rhs.end());
	    buffer_ = static_cast<pointer>(Allocator::allocate(items_));
        std::copy(rhs.begin(), rhs.end(), begin());
    }

    void do_copy(const self_type& rhs)
    {
		if ( buffer_ )
			Allocator::deallocate(buffer_, items_);
        items_ = std::distance(rhs.begin(), rhs.end());
	    buffer_ = static_cast<pointer>(Allocator::allocate(items_));
        memcpy(buffer_, rhs.buffer_, items_*sizeof(Type));
    }

public:

    template<typename Input_Iterator>
    void assign(Input_Iterator begin_, Input_Iterator end_)
    {
		if ( buffer_ )
			Allocator::deallocate(buffer_, items_);
        items_ = std::distance(begin_, end_);
	    buffer_ = static_cast<pointer>(Allocator::allocate(items_));
        std::copy(begin_, end_, begin());
    }

private:

    bool is_internal_alloc()const
    {
		return false;
    }

public: 

    matrix_buffer(const self_type& rhs)
	: Allocator(rhs)
    {
        operator=(rhs);
    }

    self_type & operator=(const self_type& rhs)
    {
        do_copy(rhs);
        return *this;
    }

private:
    pointer buffer_;
    size_type items_;

};

}//namespace

#endif	/* _DYNAMIC_MATRIX_BUFFER_0_HPP_INCLUDED */

