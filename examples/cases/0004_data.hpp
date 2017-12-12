void _0000_data()
{
    feng::matrix<double> m{ 64, 256 };
    m.save_as_bmp( "./images/0000_data.bmp" );

    auto ptr = m.data();
    for (  auto idx = 0UL; idx != m.size(); ++idx )
        ptr[idx] = std::sin( idx*idx*0.1 );

    m.save_as_bmp( "./images/0001_data.bmp" );
}
