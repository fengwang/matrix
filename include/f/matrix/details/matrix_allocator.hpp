#ifndef AALLOCATOR_HPP_ONDSL219UHSDAFKLJ4OUIHSFDKJLALFKJHAS94U8HEKLJFDSKJSF49UEA
#define AALLOCATOR_HPP_ONDSL219UHSDAFKLJ4OUIHSFDKJLALFKJHAS94U8HEKLJFDSKJSF49UEA


//TODO
//          rewrite (buffered_)allocator using crtp pattern

namespace f
{

template< typename Type, class Allocator = std::allocator<Type> >
struct matrix_allocator : public Allocator
{
    typedef Type                                    value_type;
    typedef matrix_allocator                        self_type;
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
    typedef Allocator                               host_allocator_type;

    matrix_allocator( self_type&& other )
    {
        operator = ( other );
    }

    self_type& operator = ( self_type&& other )
    {
        Allocator::operator = (std::move(other));
        buffer_ = other.buffer_;
        items_ = other.items_;
        other.buffer_ = nullptr;
        other.items_ = 0;
        return *this;
    }

    matrix_allocator(const self_type& rhs) : Allocator(rhs)
    {
        operator = (rhs);
    }

    self_type & operator = (const self_type& rhs)
    {
        items_ = 0;
        buffer_ = nullptr;

        if ( rhs.items_ && rhs.buffer_ )
        {
            items_ = rhs.items_;
            buffer_ = static_cast<pointer>(Allocator::allocate(items_));
            //std::cerr << "\n[A]llocating with items " << items_ << ", buffer " << reinterpret_cast<unsigned long>(buffer_) << "\n";
            std::copy( rhs.begin(), rhs.end(), begin() );
        }

        return *this;
    }

    explicit matrix_allocator(const size_type dims = 0)
    {
        items_ = dims;
        buffer_ = nullptr;

        if (items_ )
        {
            buffer_ = static_cast<pointer>(Allocator::allocate(items_));
            std::memset(buffer_, 0, items_*sizeof(Type));
            //std::cerr << "\n[A]llocating with items " << items_ << ", buffer " << std::hex << reinterpret_cast<unsigned long>(buffer_) << "\n";
        }
    }

    template<typename Input_Iterator>
    matrix_allocator(Input_Iterator begin_, Input_Iterator end_)
    {
        items_ = std::distance(begin_, end_);
        buffer_ = nullptr;

        if ( items_ )
        {
            buffer_ = static_cast<pointer>(Allocator::allocate(items_));
            std::copy(begin_, end_, begin());
            //std::cerr << "\n[A]llocating with items " << items_ << ", buffer " << reinterpret_cast<unsigned long>(buffer_) << "\n";
        }
    }

    ~matrix_allocator()
    {
        if ( items_ && buffer_ )
        {
            //std::cerr << "\n[D]eallocating with items " << items_ << ", buffer " << reinterpret_cast<unsigned long>(buffer_) << "\n";
            Allocator::deallocate(buffer_, items_);
            buffer_ = nullptr;
            items_ = 0;
        }
    }

    bool empty()const
    {
        return ( 0 == items_);
    }

    size_type size()const
    {
        return items_;
    }

    iterator begin()
    {
        return &buffer_[0];
    }

    const_iterator begin() const
    {
        return &buffer_[0];
    }

    const_iterator cbegin() const
    {
        return begin();
    }

    iterator end()
    {
        return begin() + size();
    }

    const_iterator end() const
    {
        return begin() + size();
    }

    const_iterator cend() const
    {
        return end();
    }

    reverse_iterator rbegin()
    {
        return reverse_iterator( end() );
    }

    const_reverse_iterator rbegin() const
    {
        return const_reverse_iterator( end() );
    }

    const_reverse_iterator crbegin() const
    {
        return rbegin();
    }

    reverse_iterator rend()
    {
        return reverse_iterator( begin() );
    }

    const_reverse_iterator rend() const
    {
        return const_reverse_iterator( begin() );
    }

    const_reverse_iterator crend() const
    {
        return rend();
    }

    reference operator[](const size_type index)
    {
        return buffer_[index];
    }

    const_reference operator[](const size_type index) const
    {
        return buffer_[index];
    }

private:
    void do_copy(const self_type& rhs)
    {
        assign( rhs.begin(), rhs.end() );
    }

public:
    template<typename Input_Iterator>
    void assign(Input_Iterator begin_, Input_Iterator end_)
    {
        const size_type dis = std::distance(begin_, end_);

        if ( items_ != dis  )
        {
            if ( items_ && buffer_ )
            {
                //std::cerr << "\n[D]eallocating with items " << items_ << ", buffer " << reinterpret_cast<unsigned long>(buffer_) << "\n";
                Allocator::deallocate( buffer_, items_ );
                items_ = 0;
                buffer_ = nullptr;
            }
            items_ = dis;
            if (items_ )
            {
                buffer_ = static_cast<pointer>(Allocator::allocate(items_));
                //std::cerr << "\n[A]llocating with items " << items_ << ", buffer " << reinterpret_cast<unsigned long>(buffer_) << "\n";
            }
        }

        std::copy(begin_, end_, begin());
    }

    void swap( self_type& other )
    {
        std::swap( buffer_, other.buffer_ );
        std::swap( items_, other.items_ );
    }

    pointer data()
    {
        return buffer_;
    }

    const_pointer data() const
    {
        return buffer_;
    }

    pointer buffer_;
    size_type items_;

};

template< typename T1, typename A1, typename T2, typename A2 >
bool operator == ( matrix_allocator<T1, A1> const&, matrix_allocator<T2, A2> const& )
{
    return true;
}

template< typename T1, typename A1, typename T2, typename A2 >
bool operator != ( matrix_allocator<T1, A1> const&, matrix_allocator<T2, A2> const& )
{
    return false;
}

template< typename T, typename A >
void swap( matrix_allocator<T,A>& one, matrix_allocator<T,A>& another )
{
    one.swap( another );
}


}//namespace f

#endif//_ALLOCATOR_HPP_ONDSL219UHSDAFKLJ4OUIHSFDKJLALFKJHAS94U8HEKLJFDSKJSF49UEA

