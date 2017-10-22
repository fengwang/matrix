#ifndef LKVQKPHGYRVVUMBFEYJXSYTHAEMVLYBKERVXSPRGWHPRJSVYARUEYBWAGDTLVRNSTOFDDIJEY
#define LKVQKPHGYRVVUMBFEYJXSYTHAEMVLYBKERVXSPRGWHPRJSVYARUEYBWAGDTLVRNSTOFDDIJEY

namespace f
{
    namespace crtp_save_as_uniform_inverse_bmp_private
    {
        template< typename T >
        struct crtp_save_as_uniform_inverse_bmp_data
        {
            unsigned long row;
            unsigned long col;
            T value;

            crtp_save_as_uniform_inverse_bmp_data() {}
            crtp_save_as_uniform_inverse_bmp_data( unsigned long r, unsigned long c, T v ) : row( r ), col( c ), value( v ) {}
        };

        template< typename T >
        bool operator < ( crtp_save_as_uniform_inverse_bmp_data<T> const& lhs, crtp_save_as_uniform_inverse_bmp_data<T> const& rhs )
        {
            return lhs.value < rhs.value;
        }
    }


    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_save_as_uniform_inverse_bmp
    {
        typedef Type            value_type;
        typedef unsigned long   size_type;
        typedef Matrix          zen_type;

        bool save_as_uniform_inverse_bmp( const std::string& file_name ) const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            using namespace crtp_save_as_uniform_inverse_bmp_private;
            std::vector<crtp_save_as_uniform_inverse_bmp_data<value_type>> cache_vector;
            cache_vector.resize( zen.size() );

            for ( size_type r = 0; r != zen.row(); ++r )
                for ( size_type c = 0; c != zen.col(); ++c )
                {
                    unsigned long offset = r * zen.col() + c;
                    cache_vector[offset] = crtp_save_as_uniform_inverse_bmp_data<value_type> { r, c, zen[r][c] };
                }

            std::sort( cache_vector.begin(), cache_vector.end() );
            zen_type clone{ zen.row(), zen.col() };

            if ( 0 )
            {
                unsigned long const pixels_per_bin = ( zen.size() + 755 ) / 766;
                unsigned long const max_offset = zen.size();

                for ( unsigned long bin_index = 0; bin_index != 766; ++bin_index )
                {
                    unsigned long const assign_start = bin_index * pixels_per_bin;
                    unsigned long const assign_end = std::min( max_offset, assign_start + pixels_per_bin );
                    value_type val_of_this_bin = bin_index;
                    value_type const start_value = cache_vector[assign_start].value;

                    for ( unsigned long rev = bin_index; rev != 0; --rev )
                    {
                        unsigned long const other_start = rev * pixels_per_bin;
                        value_type const other_start_value = cache_vector[other_start].value;

                        if ( start_value > other_start_value ) break;

                        val_of_this_bin = rev;
                    }

                    for ( auto itor = cache_vector.begin() + assign_start; itor != cache_vector.begin() + assign_end; ++itor )
                    {
                        clone[( *itor ).row][( *itor ).col] = val_of_this_bin;
                    }
                }
            }
            else
            {
                auto setter = [&clone, &cache_vector]( unsigned long start, unsigned long end, unsigned long index, auto recursion_function )
                {
                    if ( start >= end ) return;

                    unsigned long const bin_left = 766 - index;
                    unsigned long const pix_per_bin = ( end - start + bin_left - 1 ) / bin_left;
                    unsigned long next_index = start + pix_per_bin;
                    value_type const current_val_start = cache_vector[start].value;

                    for ( ; next_index != end; ++ next_index )
                    {
                        if ( cache_vector[next_index].value > current_val_start ) break;
                    }

                    for ( unsigned long idx = start; idx != next_index; ++idx )
                        clone[cache_vector[idx].row][cache_vector[idx].col] = index;

                    recursion_function( next_index, end, index + 1, recursion_function );
                };
                setter( 0, cache_vector.size(), 0, setter );
            }

            return clone.save_as_inverse_bmp( file_name );
        }
        bool save_as_uniform_inverse_bmp( const char* const file_name ) const
        {
            return save_as_uniform_inverse_bmp( std::string{file_name} );
        }

    };//struct crtp_save_as_uniform_inverse_bmp

}

#endif//LKVQKPHGYRVVUMBFEYJXSYTHAEMVLYBKERVXSPRGWHPRJSVYARUEYBWAGDTLVRNSTOFDDIJEY

