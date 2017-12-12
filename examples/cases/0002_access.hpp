void _0000_access()
{
    feng::matrix<double> m{ 64, 256 };

    int starter = 0;

    double const keys[] = { 1, 2, 3, 4, 5, 6, 7, 8 };

    for ( auto& x : m )
    {
        int val = starter++ & 0x7;
        x = keys[val];
    }
    m.save_as_bmp( "./images/0000_access.bmp" );
}
