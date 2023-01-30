#include <iostream>
#include <cmath>
#include <cstdio>

#include "rcsc/soccer_math.h"
#include "rcsc/common/server_param.h"

using namespace rcsc;


int
main()
{
    const char * var_name = "M_movable_table_v18_wide";
    constexpr double default_qstep = 0.01;
    constexpr double noise_term = 0.5; // v18+ narrow
    // constexpr double noise_term = 0.75; // v18+ normal
    // constexpr double noise_term = 1.0; // v18+ wide
    double qstep = default_qstep * noise_term;

    const double max_dist = std::sqrt( std::pow( ServerParam::DEFAULT_PITCH_LENGTH + ServerParam::DEFAULT_PITCH_MARGIN*2 + 10.0, 2 )
                                       + std::pow( ServerParam::DEFAULT_PITCH_WIDTH + ServerParam::DEFAULT_PITCH_MARGIN*2 + 10.0, 2 ) );

    double unq_dist = 0.0;
    double prev_start_unq_dist = 0.0;
    double prev_quant_dist = 0.0;

    std::cerr << "qstep = " << qstep << std::endl;

    for ( unq_dist = 0.0; unq_dist < max_dist; unq_dist += 1.0e-3 )
    {
        double quant_dist = quantize_dist( unq_dist, qstep );
        if ( std::fabs( quant_dist - prev_quant_dist ) > 0.01 )
        {
            double prev_mean = ( unq_dist + prev_start_unq_dist ) * 0.5;
            double prev_err = ( unq_dist - prev_start_unq_dist ) * 0.5;
            std::printf( "    %s.emplace_back( %.2f, %.6f, %.6f );\n",
                         var_name, prev_quant_dist, prev_mean, prev_err );

            prev_quant_dist = quant_dist;
            prev_start_unq_dist = unq_dist;
        }
    }
}
