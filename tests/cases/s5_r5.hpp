// S5-R5 (PR-7): replays the fuzz regressions (tests/fuzz/regressions/<target>/, if any) and the seed corpora
// (tests/fuzz/corpus/<target>/) through the parsers with the harness checks of tests/fuzz/fuzz_checks.hpp.
// Runs from the repo root.
#include "../fuzz/fuzz_checks.hpp"

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace s5_r5
{
    using check_fn = char const* (*)( std::uint8_t const*, std::size_t );

    struct target
    {
        char const* name;
        check_fn check;
    };

    inline std::vector< std::filesystem::path > files_under( std::filesystem::path const& dir )
    {
        std::vector< std::filesystem::path > ans;
        std::error_code ec;
        if ( !std::filesystem::is_directory( dir, ec ) ) return ans;
        for ( auto const& e : std::filesystem::directory_iterator( dir ) )
            if ( e.is_regular_file() ) ans.push_back( e.path() );
        return ans;
    }
}

TEST_CASE( "S5 fuzz regressions replay without failure", "[S5][S5-R5]" )
{
    s5_r5::target const targets[] = {
        { "npy", fuzz_checks::check_npy },
        { "bmp", fuzz_checks::check_bmp },
        { "binary", fuzz_checks::check_binary },
        { "text", fuzz_checks::check_text },
    };
    for ( auto const& t : targets )
    {
        std::size_t seeds = 0;
        for ( char const* root : { "./tests/fuzz/regressions/", "./tests/fuzz/corpus/" } )
        {
            auto const files = s5_r5::files_under( std::string{ root } + t.name );
            if ( std::string{ root }.find( "corpus" ) != std::string::npos ) seeds = files.size();
            for ( auto const& f : files )
            {
                std::vector< std::uint8_t > bytes;
                REQUIRE( feng::matrix_details::read_file( f.string().c_str(), bytes ) );
                INFO( "input " << f.string() );
                char const* const failure = t.check( bytes.data(), bytes.size() );
                REQUIRE( failure == nullptr );
            }
        }
        INFO( "target " << t.name );
        REQUIRE( seeds > 0 );
    }
}
