#ifndef MMATRIX_EXPRESSION_HPP_INCLUDED_DPOIJSDF3LKDSAFLKJDF1LKJDFSAHI843LKJAFSD
#define MMATRIX_EXPRESSION_HPP_INCLUDED_DPOIJSDF3LKDSAFLKJDF1LKJDFSAHI843LKJAFSD

namespace f
{
    template<typename Matrix>
    struct matrix_expression
    {
        typedef Matrix                                          zen_type;

        operator zen_type const& () const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            return zen;
        }

        operator zen_type& ()
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            return zen;
        }

    };//struct

}

#endif//_MATRIX_EXPRESSION_HPP_INCLUDED_DPOIJSDF3LKDSAFLKJDF1LKJDFSAHI843LKJAFSD

