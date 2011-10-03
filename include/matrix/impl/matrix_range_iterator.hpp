// -*- C++ -*-
/*
 * File:   matrix_range_iterator.hpp
 * Author: feng
 *
 */

#ifndef _DYNAMIC_MATRIX_RANGE_ITERATOR_HPP_INCLUDED
#define _DYNAMIC_MATRIX_RANGE_ITERATOR_HPP_INCLUDED

#include <cassert>
#include <iterator>

namespace dynamic{


template<typename Iterator_Type>
struct matrix_range_iterator
{
    public:

        typedef Iterator_Type                           value_type;
        typedef matrix_range_iterator           self_type;
        typedef self_type&                              reference;
        typedef self_type&                              pointer;
        typedef const self_type&                        const_reference;
        typedef std::size_t                             size_type;
        typedef std::ptrdiff_t                          difference_type;
        typedef std::random_access_iterator_tag         iterator_category;

        typedef value_type                              iterator;
        typedef const value_type                        const_iterator;
        typedef std::reverse_iterator<iterator>         reverse_iterator;
        typedef std::reverse_iterator<const_iterator>   const_reverse_iterator;

    public:

        matrix_range_iterator(  const value_type& first__,
                                const value_type& last__,
                                const difference_type step__
                             )
            : first_( first__ ), last_( last__ ), step_( step__ )
        { }

        matrix_range_iterator( const self_type& other )
            : first_( other.first_ ), last_( other.last_ ), step_( other.step_ )
        { }

        self_type& 
		operator = ( const self_type& other )
        {
            do_copy( other );
            return *this;
        }

        reference 
		operator*()
        {
            return *this;
        }

        const_reference 
		operator*() const
        {
            return *this;
        }

        self_type&
        operator ++()
        {
            ++first_;
            ++last_;
            return *this;
        }

        const self_type
        operator ++( int )
        {
            self_type ans( *this );
            operator++();
            return ans;
        }

        self_type&
        operator --()
        {
            --first_;
            --last_;
            return *this;
        }

        const self_type
        operator --( int )
        {
            self_type ans( *this );
            operator--();
            return ans;
        }

    public:

        iterator
        begin()
        {
            return first_;
        }

        const_iterator
        begin() const
        {
            return first_;
        }

        iterator
        end()
        {
            return last_ + 1;
        }

        const_iterator
        end() const
        {
            return last_ + 1;
        }

        reverse_iterator
        rbegin()
        {
            return reverse_iterator( end() );
        }

        const_reverse_iterator
        rbegin() const
        {
            return reverse_iterator( end() );
        }

        reverse_iterator
        rend()
        {
            return reverse_iterator( begin() );
        }

        const_reverse_iterator
        rend() const
        {
            return reverse_iterator( begin() );
        }

    public:
        self_type&
        operator += ( const difference_type size__ )
        {
            first_ += size__;
            last_ += size__;
            return *this;
        }

        self_type&
        operator -= ( const difference_type size__ )
        {
            first_ -= size__;
            last_ -= size__;
            return *this;
        }

    public:
        friend
        const self_type
        operator + ( const self_type& lhs, const difference_type rhs )
        {
            self_type ans( lhs );
            ans += rhs;
            return ans;
        }

        friend
        const self_type
        operator + ( const difference_type& lhs, const self_type rhs )
        {
            return rhs + lhs;
        }

        friend
        const self_type
        operator - ( const self_type& lhs, const difference_type rhs )
        {
            self_type ans( lhs );
            ans -= rhs;
            return ans;
        }

        friend
        difference_type
        operator - ( const self_type& lhs, const self_type& rhs )
        {
            
			assert( std::distance( lhs.first_, lhs.last_ ) ==
                    std::distance( rhs.first_, rhs.last_ )
                  );
            assert( lhs.step_ == rhs.step_ );
            return lhs.first_ - rhs.first_;
        }

        friend
        bool operator > ( const self_type& lhs, const self_type& rhs )
        {
            assert( std::distance( lhs.first_, lhs.last_ ) ==
                    std::distance( rhs.first_, rhs.last_ )
                  );
            assert( lhs.step_ == rhs.step_ );
            return lhs.first_ > rhs.first_;
        }

        friend
        bool operator >= ( const self_type& lhs, const self_type& rhs )
        {
            assert( std::distance( lhs.first_, lhs.last_ ) ==
                    std::distance( rhs.first_, rhs.last_ )
                  );
            assert( lhs.step_ == rhs.step_ );
            return lhs.first_ >= rhs.first_;
        }

        friend
        bool operator == ( const self_type& lhs, const self_type& rhs )
        {
            assert( std::distance( lhs.first_, lhs.last_ ) ==
                    std::distance( rhs.first_, rhs.last_ )
                  );
            assert( lhs.step_ == rhs.step_ );
            return lhs.first_ == rhs.first_;
        }

        friend
        bool operator != ( const self_type& lhs, const self_type& rhs )
        {
            assert( std::distance( lhs.first_, lhs.last_ ) ==
                    std::distance( rhs.first_, rhs.last_ )
                  );
            assert( lhs.step_ == rhs.step_ );
            return lhs.first_ != rhs.first_;
        }

        friend
        bool operator < ( const self_type& lhs, const self_type& rhs )
        {
            assert( std::distance( lhs.first_, lhs.last_ ) ==
                    std::distance( rhs.first_, rhs.last_ )
                  );
            assert( lhs.step_ == rhs.step_ );
            return lhs.first_ < rhs.first_;
        }

        friend
        bool operator <= ( const self_type& lhs, const self_type& rhs )
        {
            assert( std::distance( lhs.first_, lhs.last_ ) ==
                    std::distance( rhs.first_, rhs.last_ )
                  );
            assert( lhs.step_ == rhs.step_ );
            return lhs.first_ <= rhs.first_;
        }

    private:
        void do_copy( const self_type& other )
        {

            const difference_type step__ = other.step_;
            value_type first__ = other.first_;
            value_type last__ = other.last_;
            value_type target_first_ = first_;
            first__.reset_step( step__ );
            last__.reset_step( step__ );
            target_first_.reset_step( step_ );

            while ( first__ <= last__ )
                {
                    *target_first_ = *first__;
					++target_first_;
					++first__;
                }
        }

    private:
        value_type first_;
        value_type last_;
        difference_type step_;

};//matrix_range_iterator


}//namespace dynamic

#endif  /* _DYNAMIC_MATRIX_RAGNE_ITERATOR_HPP_INCLUDED */

