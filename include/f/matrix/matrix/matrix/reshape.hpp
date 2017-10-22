#ifndef KYMQPGALQSXCXPIEGXUYJYXKWNSJSFWCYXVGFSQCGKDXRRAXBEIVVNYIYOUJAHTNLNXAXPDHW
#define KYMQPGALQSXCXPIEGXUYJYXKWNSJSFWCYXVGFSQCGKDXRRAXBEIVVNYIYOUJAHTNLNXAXPDHW


namespace f
{
    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_reshape
    {
        typedef Matrix                                                          zen_type;
        typedef crtp_typedef<Type, Allocator>                          type_proxy_type;
        typedef typename type_proxy_type::size_type                             size_type;

        //reshape conserves the data
        void reshape( const size_type new_row, const size_type new_col )
        {
            zen_type& zen = static_cast<zen_type&>( *this );
            assert( new_row * new_col == zen.row() * zen.col() );
            zen.row_ = new_row;
            zen.col_ = new_col;
        }

    };//struct crtp_reshape

}

#endif//KYMQPGALQSXCXPIEGXUYJYXKWNSJSFWCYXVGFSQCGKDXRRAXBEIVVNYIYOUJAHTNLNXAXPDHW

