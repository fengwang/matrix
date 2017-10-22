#ifndef MAPPLY_HPP_INCLUDED_DSPOISA3089UAFSDJK4980YHADSLKJVMKBNADSF9Y849IUHASFDK
#define MAPPLY_HPP_INCLUDED_DSPOISA3089UAFSDJK4980YHADSLKJVMKBNADSF9Y849IUHASFDK

namespace f
{
    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_apply
    {
        typedef Matrix                                                          zen_type;
        typedef crtp_typedef<Type, Allocator>                                   type_proxy_type;

        template <typename Function>
        zen_type const apply( const Function& f ) const
        {
            //TODO: function concept check
            zen_type const& zen = static_cast<zen_type const&>( *this );
            zen_type ans( zen.row(), zen.col() );
            std::transform( zen.begin(), zen.end(), ans.begin(), f );
            return ans;
        }

    };//struct

}

#endif//_APPLY_HPP_INCLUDED_DSPOISA3089UAFSDJK4980YHADSLKJVMKBNADSF9Y849IUHASFDK

