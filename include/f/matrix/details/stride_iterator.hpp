#ifndef MSTRIDE_ITERATOR_HPP_INCLUDED_SDPOIANSDWU9HASFKLJANSFKLJN439UISAHFLKJ49U
#define MSTRIDE_ITERATOR_HPP_INCLUDED_SDPOIANSDWU9HASFKLJANSFKLJN439UISAHFLKJ49U

#include "./std.hpp"

namespace f
{

/*
 *      Stride iterator is an iterator adaptor that takes a random access iterator range and provides
 *      a random access iterator on it goes through a stride (a sequence of iterator n steps apart),
 *      which is useful when striding a matrix column or diagonal.
 */
template< typename Iterator_Type >
struct stride_iterator
{
    typedef typename std::iterator_traits<Iterator_Type>::value_type                value_type;
    typedef typename std::iterator_traits<Iterator_Type>::reference                 reference;
    typedef typename std::iterator_traits<Iterator_Type>::difference_type           difference_type;
    typedef typename std::iterator_traits<Iterator_Type>::pointer                   pointer;
    typedef typename std::iterator_traits<Iterator_Type>::iterator_category         iterator_category;
    //typedef std::random_access_iterator_tag                                       iterator_category;
    typedef stride_iterator                                                         self_type;

    //  default ctor.
    stride_iterator() : iterator_(0), step_(1) { }

    stride_iterator(const self_type&) = default;

    stride_iterator(self_type&&) = default;

    self_type& operator = (const self_type&) = default;

    self_type& operator = (self_type&&) = default;

    //  ctor from an iterator and a step
    stride_iterator(const Iterator_Type& it, const difference_type& dt) : iterator_(it), step_(dt) { }

    //  prefix increasement
    self_type & operator++()
    {
        iterator_ += step_;
        return *this;
    }

    // postfix increasement
    const self_type operator ++(int)
    {
        self_type ans(*this);
        operator++();
        return ans;
    }

    // operator +=
    self_type & operator+=(const difference_type dt)
    {
        iterator_ += dt*step_;
        return *this;
    }

    // operator +
    friend const self_type operator + ( const self_type& lhs, const difference_type rhs )
    {
        self_type ans(lhs);
        ans += rhs;
        return ans;
    }

    // operator +
    friend const self_type operator + ( const difference_type lhs, const self_type& rhs )
    {
        return rhs + lhs;
    }

    // prefix decreasement
    self_type & operator--()
    {
        iterator_ -= step_;
        return *this;
    }

    // postfix decreasement
    const self_type operator -- ( int )
    {
        self_type ans(*this);
        operator--();
        return ans;
    }

    // operator -=
    self_type & operator-=(const difference_type dt)
    {
        iterator_ -= dt*step_;
        return *this;
    }

    // operator -
    friend const self_type operator - ( const self_type& lhs, const difference_type rhs )
    {
        self_type ans(lhs);
        ans -= rhs;
        return ans;
    }

    // lvalue accessment
    reference operator[](const difference_type dt)
    {
        return iterator_[dt * step_];
    }

    // const lvalue accessment
    const reference operator[](const difference_type dt) const
    {
        return iterator_[dt * step_];
    }

    // deptr
    reference operator*()
    {
        return *iterator_;
    }

    // const dptr
    const reference operator*() const
    {
        return *iterator_;
    }

    // operator ==
    friend bool operator == (const self_type& lhs, const self_type& rhs)
    {
        assert(lhs.step_ == rhs.step_);
        return lhs.iterator_ == rhs.iterator_;
    }

    // operator !=
    friend bool operator != (const self_type& lhs, const self_type& rhs)
    {
        assert(lhs.step_ == rhs.step_);
        return lhs.iterator_ != rhs.iterator_;
    }

    // operator <
    friend bool operator < (const self_type& lhs, const self_type& rhs)
    {
        assert(lhs.step_ == rhs.step_);
        return lhs.iterator_ < rhs.iterator_;
    }

    // operator <=
    friend bool operator <= (const self_type& lhs, const self_type& rhs)
    {
        assert(lhs.step_ == rhs.step_);
        return lhs.iterator_ <= rhs.iterator_;
    }

    // operator >
    friend bool operator > (const self_type& lhs, const self_type& rhs)
    {
        assert(lhs.step_ == rhs.step_);
        return lhs.iterator_ > rhs.iterator_;
    }

    // operator >=
    friend bool operator >= (const self_type& lhs, const self_type& rhs)
    {
        assert(lhs.step_ == rhs.step_);
        return lhs.iterator_ >= rhs.iterator_;
    }

    // operator -
    friend difference_type operator - (const self_type& lhs, const self_type& rhs)
    {
        assert(lhs.step_ == rhs.step_);
        return (lhs.iterator_ - rhs.iterator_) / lhs.step_;
    }

    // return the step
    difference_type step() const
    {
        return step_;
    }

    // reset step
    void reset_step( const difference_type step )
    {
        step_ = step;
    }

    // reset step
    void step( const difference_type step )
    {
        step_ = step;
    }

private:
    // iterator stored
    Iterator_Type iterator_;
    // stride step
    difference_type step_;

}; //stride_iterator

}//namespace f

#endif//_STRIDE_ITERATOR_HPP_INCLUDED_SDPOIANSDWU9HASFKLJANSFKLJN439UISAHFLKJ49U

