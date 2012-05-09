
// to build
// g++ table_maker.cc

// to output
// ./a.out QSTEP
// Server default QSTEP is 0.1 or 0.01.


#include <iostream>
#include <cmath>
#include <cstdlib>
#include <cstdio>

using namespace std;


const double SERVER_EPS = 1.0e-10;

//////////////////////////////////////////////////////////

double
quantize( const double& value, const double& qstep )
{
    return std::round( value / qstep ) * qstep;
}

//////////////////////////////////////////////////////////

double
quantize_dist( const double& unq_dist, const double& qstep )
{
    /*
      quantize process

      d1 = log( unq_dist + EPS )

      d2 = quantize( d1 , qstep )

      d3 = exp( d2 )

      quant_dist = quantize( d3, 0.1 )
    */

    return quantize( std::exp ( quantize( std::log( unq_dist + SERVER_EPS ), qstep) ), 0.1 );
}

//////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
    if ( argc < 2 )
    {
        cerr << "input qstep" << endl;
        return 1;
    }

    const double qstep = atof(argv[1]);

    if ( qstep > 1.0 )
    {
        cout << "qstep is too big" << endl;
        return 1;
    }

    cout << "qstep = " << qstep << "\n\n";


    const double dist_inc = 1.0e-6;


    double min_dist = 0.0, max_dist = 0.0;
    double prev_see_dist = 0.0;
    for ( double dist = 0.0; dist < 200.0; dist += dist_inc )
    {
        double see_dist = quantize_dist( dist, qstep );

        if ( static_cast< int >( std::round( prev_see_dist*100.0 ) )
             == static_cast< int >( std::round( see_dist*100.0 ) ) )
        {
            continue;
        }

        max_dist = dist - dist_inc;

        // see_dist,  average_dist,  dist_error
        fprintf( stdout, "%6.2f, %10.6f, %9.6f\n",
                 prev_see_dist,
                 ( max_dist + min_dist ) * 0.5,
                 ( max_dist - min_dist ) * 0.5 );

        min_dist = dist;

#if 1
        dist += ( see_dist - prev_see_dist ) - 0.08;
#endif
        prev_see_dist = see_dist;
    }

    return 0;
}
