#ifndef MDATA_HPP_INCLUDED_DSPIJ4HSDAKJNVAIUDH4HA9OP209AFDIOJAFLDKJVLKDSFOIJ43FS
#define MDATA_HPP_INCLUDED_DSPIJ4HSDAKJNVAIUDH4HA9OP209AFDIOJAFLDKJVLKDSFOIJ43FS


namespace f
{
    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_data
    {
        typedef Matrix                                                          zen_type;
        typedef crtp_typedef<Type, Allocator>                                   type_proxy_type;
        typedef typename type_proxy_type::pointer                               pointer;
        typedef typename type_proxy_type::const_pointer                         const_pointer;

        pointer data()
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            return zen.data_.data();
        }

        const_pointer data() const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            return zen.data_.data();
        }

    };//struct

}

#endif//_DATA_HPP_INCLUDED_DSPIJ4HSDAKJNVAIUDH4HA9OP209AFDIOJAFLDKJVLKDSFOIJ43FS

