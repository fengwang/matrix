#ifndef TTYPEDEF_HPP_INCLUDED_DFSOPJI389UASFLKJ4JKNMVLJKFAHSD9YU84KLJSAHFUIJH498
#define TTYPEDEF_HPP_INCLUDED_DFSOPJI389UASFLKJ4JKNMVLJKFAHSD9YU84KLJSAHFUIJH498

#include "../../details/range.hpp"
#include "../../details/matrix_allocator.hpp"
#include "../../details/stride_iterator.hpp"

namespace f
{
    template<typename Type, typename Allocator>
    struct crtp_typedef
    {
        typedef typename std::decay<Type>::type                             value_type;
        typedef value_type*                                                 iterator;
        typedef const value_type*                                           const_iterator;
        typedef matrix_allocator<value_type, Allocator>                     storage_type;
        typedef std::uint64_t                                               size_type;
        typedef std::ptrdiff_t                                              difference_type;
        typedef range                                                       range_type;
        typedef typename Allocator::pointer                                 pointer;
        typedef typename Allocator::const_pointer                           const_pointer;
        typedef stride_iterator<value_type*>                                matrix_stride_iterator;
        typedef value_type*                                                 row_type;
        typedef const value_type*                                           const_row_type;
        typedef stride_iterator<value_type*>                                col_type;
        typedef stride_iterator<const value_type*>                          const_col_type;
        typedef stride_iterator<value_type*>                                diag_type;
        typedef stride_iterator<const value_type*>                          const_diag_type;
        typedef stride_iterator<value_type*>                                anti_diag_type;
        typedef stride_iterator<const value_type*>                          const_anti_diag_type;
        typedef std::reverse_iterator<iterator>                             reverse_iterator;
        typedef std::reverse_iterator<const_iterator>                       const_reverse_iterator;
        typedef std::reverse_iterator<matrix_stride_iterator>               reverse_matrix_stride_iterator;
        typedef std::reverse_iterator<row_type>                             reverse_row_type;
        typedef std::reverse_iterator<const_row_type>                       const_reverse_row_type;
        typedef std::reverse_iterator<col_type>                             reverse_col_type;
        typedef std::reverse_iterator<const_col_type>                       const_reverse_col_type;
        typedef std::reverse_iterator<diag_type>                            reverse_upper_diag_type;
        typedef std::reverse_iterator<const_diag_type>                      const_reverse_upper_diag_type;
        typedef std::reverse_iterator<diag_type>                            reverse_lower_diag_type;
        typedef std::reverse_iterator<const_diag_type>                      const_reverse_lower_diag_type;
        typedef std::reverse_iterator<diag_type>                            reverse_diag_type;
        typedef std::reverse_iterator<const_diag_type>                      const_reverse_diag_type;
        typedef std::reverse_iterator<anti_diag_type>                       reverse_anti_diag_type;
        typedef std::reverse_iterator<const_anti_diag_type>                 const_reverse_anti_diag_type;

    };//struct crtp_import

}//namespace f

#endif//_TYPEDEF_HPP_INCLUDED_DFSOPJI389UASFLKJ4JKNMVLJKFAHSD9YU84KLJSAHFUIJH498

