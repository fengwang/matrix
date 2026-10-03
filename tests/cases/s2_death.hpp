// S2-R1: fork-based death tests. The callable runs in a child process whose stderr goes to a pipe; the parent
// collects that text and the wait status. A child whose callable returns exits 0 through _exit, which fails the
// death requirement. The child restores SIG_DFL for SIGABRT, sends stdout to /dev/null and exits 2 if the callable
// throws.
#ifndef S2_DEATH_HPP_INCLUDED
#define S2_DEATH_HPP_INCLUDED

#include <cerrno>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <string>
#include <utility>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

namespace s2_death
{
    struct outcome
    {
        bool signaled = false;   // WIFSIGNALED
        int signal = 0;          // WTERMSIG when signaled
        bool exited = false;     // WIFEXITED
        int exit_code = -1;      // WEXITSTATUS when exited
        std::string err;         // everything the child wrote to stderr
    };

    template < typename Callable >
    outcome run( Callable&& fn )
    {
        outcome out;
        int fds[2];
        if ( ::pipe( fds ) != 0 )
        {
            out.err = "s2_death: pipe failed";
            return out;
        }
        std::fflush( stdout );
        std::fflush( stderr );
        pid_t const pid = ::fork();
        if ( pid < 0 )
        {
            ::close( fds[0] );
            ::close( fds[1] );
            out.err = "s2_death: fork failed";
            return out;
        }
        if ( pid == 0 )
        {
            ::close( fds[0] );
            ::dup2( fds[1], STDERR_FILENO );
            ::close( fds[1] );
            // Catch's SIGABRT handler would report a fatal error from the child; the default disposition keeps
            // the child's death silent apart from the contract message.
            ::signal( SIGABRT, SIG_DFL );
            int const null_fd = ::open( "/dev/null", O_WRONLY );
            if ( null_fd >= 0 )
            {
                ::dup2( null_fd, STDOUT_FILENO );
                if ( null_fd != STDOUT_FILENO ) ::close( null_fd );
            }
#if defined( __cpp_exceptions )
            try { std::forward< Callable >( fn )(); }
            catch ( ... ) { ::_exit( 2 ); }
#else
            std::forward< Callable >( fn )();
#endif
            ::_exit( 0 );
        }
        ::close( fds[1] );
        char buf[4096];
        for ( ;; )
        {
            ssize_t const n = ::read( fds[0], buf, sizeof( buf ) );
            if ( n > 0 ) { out.err.append( buf, static_cast< std::size_t >( n ) ); continue; }
            if ( n < 0 && errno == EINTR ) continue;
            break;
        }
        ::close( fds[0] );
        int status = 0;
        while ( ::waitpid( pid, &status, 0 ) < 0 && errno == EINTR ) {}
        if ( WIFSIGNALED( status ) ) { out.signaled = true; out.signal = WTERMSIG( status ); }
        if ( WIFEXITED( status ) ) { out.exited = true; out.exit_code = WEXITSTATUS( status ); }
        return out;
    }

    inline bool contains( std::string const& text, std::string const& needle )
    {
        return text.find( needle ) != std::string::npos;
    }

    inline std::size_t line_count( std::string const& text )
    {
        std::size_t n = 0;
        for ( char ch : text ) n += ( ch == '\n' );
        if ( !text.empty() && text.back() != '\n' ) ++n;
        return n;
    }
}

// Requires that `callable` aborts through the contract-violation path with `expected` in its message and that
// no sanitizer diagnostic appears. A function, so lambdas with commas need no extra parentheses.
namespace s2_death
{
    template < typename Callable >
    void require_death( Callable&& fn, std::string const& expected )
    {
        outcome const out = run( std::forward< Callable >( fn ) );
        INFO( "child stderr: " << out.err );
        INFO( "expected substring: " << expected );
        REQUIRE( out.signaled );
        REQUIRE( out.signal == SIGABRT );
        REQUIRE( contains( out.err, "contract violation" ) );
        REQUIRE( contains( out.err, expected ) );
        REQUIRE_FALSE( contains( out.err, "AddressSanitizer" ) );
        REQUIRE_FALSE( contains( out.err, "runtime error:" ) );
    }
}
#define S2_REQUIRE_DEATH( ... ) s2_death::require_death( __VA_ARGS__ )

#endif // S2_DEATH_HPP_INCLUDED
