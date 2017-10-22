#ifndef JWIOUFKWFIOTBKSAYOBGSJLLNTIRPGMWFYJMPGDIHTRNVMAQRWFCPFGLQJEIOETWBUCNNGAHV
#define JWIOUFKWFIOTBKSAYOBGSJLLNTIRPGMWFYJMPGDIHTRNVMAQRWFCPFGLQJEIOETWBUCNNGAHV

namespace f
{
    namespace crtp_save_as_balanced_inverse_bmp_private
    {
        template< typename T >
        struct crtp_save_as_balanced_inverse_bmp_data
        {
            unsigned long row;
            unsigned long col;
            T value;

            crtp_save_as_balanced_inverse_bmp_data() {}
            crtp_save_as_balanced_inverse_bmp_data( unsigned long r, unsigned long c, T v ) : row( r ), col( c ), value( v ) {}
        };

        template< typename T >
        bool operator < ( crtp_save_as_balanced_inverse_bmp_data<T> const& lhs, crtp_save_as_balanced_inverse_bmp_data<T> const& rhs )
        {
            return lhs.value < rhs.value;
        }
    }


    template<typename Matrix, typename Type, typename Allocator>
    struct crtp_save_as_balanced_inverse_bmp
    {
        typedef Type            value_type;
        typedef unsigned long   size_type;
        typedef Matrix          zen_type;

        bool save_as_balanced_inverse_bmp( const std::string& file_name ) const
        {
            zen_type const& zen = static_cast<zen_type const&>( *this );
            value_type const mx = *std::max_element( zen.begin(), zen.end() );
            value_type const mn = *std::min_element( zen.begin(), zen.end() );
            double const residual = 1.0 / std::sqrt( static_cast<double>( zen.size() ) );
            using namespace crtp_save_as_balanced_inverse_bmp_private;
            std::vector<crtp_save_as_balanced_inverse_bmp_data<double>> cache_vector;
            cache_vector.resize( zen.size() );
            double total_energy = 0.0;

            for ( size_type r = 0; r != zen.row(); ++r )
                for ( size_type c = 0; c != zen.col(); ++c )
                {
                    unsigned long offset = r * zen.col() + c;
                    double const val = static_cast<double>( zen[r][c] - mn ) / static_cast<double>( mx - mn ) + residual;
                    cache_vector[offset] = crtp_save_as_balanced_inverse_bmp_data<value_type> { r, c, val };
                    total_energy += val;
                }

            double const energy_per_bin = total_energy / 766.0;
            std::sort( cache_vector.begin(), cache_vector.end() );
            zen_type clone{ zen.row(), zen.col() };
            auto setter = [energy_per_bin, &clone, &cache_vector]( unsigned long start, unsigned long end, unsigned long index, auto recursion_function )
            {
                if ( start >= end ) return;

                double current_energy = 0.0;
                unsigned long next_index = start;

                for ( ; next_index != end; ++next_index )
                {
                    if ( current_energy >= energy_per_bin ) break;

                    clone[cache_vector[next_index].row][cache_vector[next_index].col] = index;
                    current_energy += cache_vector[next_index].value;
                }

                recursion_function( next_index, end, index + 1, recursion_function );
            };
            setter( 0, cache_vector.size(), 0, setter );
            return clone.save_as_inverse_bmp( file_name );
        }

        bool save_as_balanced_inverse_bmp( const char* const file_name ) const
        {
            return save_as_balanced_inverse_bmp( std::string{file_name} );
        }

    };//struct crtp_save_as_balanced_inverse_bmp

}

#endif//JWIOUFKWFIOTBKSAYOBGSJLLNTIRPGMWFYJMPGDIHTRNVMAQRWFCPFGLQJEIOETWBUCNNGAHV

