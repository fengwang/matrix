void _0000_linspace()
{
    {
        auto const& m = feng::linspace<double>( 1, 10, 10 );
        std::cout << "linspace<double>(1, 10, 10):\n" << m << std::endl;
    }
    {
        auto const& m = feng::linspace<double>( 1, 10, 10, false );
        std::cout << "linspace<double>(1, 10, 10, false):\n" << m << std::endl;
    }
}
