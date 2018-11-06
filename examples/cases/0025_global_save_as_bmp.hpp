// All examples are stolen from this page: https://codegolf.stackexchange.com/questions/35569/tweetable-mathematical-art

#include <cmath>

double RD(unsigned long i,unsigned long j){
   double a=0,b=0,c,d,n=0;
   while((c=a*a)+(d=b*b)<4&&n++<880)
   {b=2*a*b+j*8e-9-.645411;a=c-d+i*8e-9+.356888;}
   return 255*std::pow((n-80)/800,3.);
}
double GR(unsigned long i,unsigned long j){
   double a=0,b=0,c,d,n=0;
   while((c=a*a)+(d=b*b)<4&&n++<880)
   {b=2*a*b+j*8e-9-.645411;a=c-d+i*8e-9+.356888;}
   return 255*std::pow((n-80)/800,.7);
}
double BL(unsigned long i,unsigned long j){
   double a=0,b=0,c,d,n=0;
   while((c=a*a)+(d=b*b)<4&&n++<880)
   {b=2*a*b+j*8e-9-.645411;a=c-d+i*8e-9+.356888;}
   return 255*std::pow((n-80)/800,.5);
}

void _0000_global_save_as_bmp()
{
    unsigned long const n = 1024;
    feng::matrix<double> red{ n, n };
    feng::matrix<double> green{ n, n };
    feng::matrix<double> blue{ n, n };

    for ( auto r = 0UL; r != n; ++r )
        for ( auto c = 0UL; c != n; ++c )
        {
            red[r][c] = RD( r, c );
            green[r][c] = GR( r, c );
            blue[r][c] = BL( r, c );
        }

    feng::save_as_bmp( "./images/0000_global_save_as_bmp.bmp", red, green, blue );
}

