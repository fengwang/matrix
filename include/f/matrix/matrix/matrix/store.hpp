#ifndef MSTORE_HPP_INCLUDED_SDPOIASD9UHWEJHASKLJHAS94EYHALSFUKHSAIOUFHSFDAKLHSAF
#define MSTORE_HPP_INCLUDED_SDPOIASD9UHWEJHASKLJHAS94EYHALSFUKHSAIOUFHSFDAKLHSAF


namespace f
{
    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_store
    {
        typedef Matrix                                                          zen_type;
        typedef crtp_typedef<Type, Allocator>                                   type_proxy_type;

        // TODO:
        //compress and store
        bool store( const char* const ) const
        {
            return true;
        }

        // TODO:
        //restore from a compressed file
        bool restore( const char* const )
        {
            return true;
        }

        bool restore( std::string const& file_path )
        {
            return restore( file_path.c_str() );
        }

    };//struct crtp_store

}

#endif//_STORE_HPP_INCLUDED_SDPOIASD9UHWEJHASKLJHAS94EYHALSFUKHSAIOUFHSFDAKLHSAF

