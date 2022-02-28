// -*-c++-*-

/*!
  \file ngnet.cpp
  \brief Normalized Gaussian Network class Source File.
*/

/*
 *Copyright:

 Copyright (C) Hidehisa AKIYAMA

 This code is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 3 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 *EndCopyright:
 */

/////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ngnet.h"

#include <iterator>
#include <random>
#include <algorithm>
#include <numeric>
#include <limits>
#include <string>
#include <sstream>
#include <ctime>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
NGNet::Unit::Unit()
{
    std::fill( center_.begin(), center_.end(), 0.0 );
    std::fill( weights_.begin(), weights_.end(), 0.0 );
    std::fill( delta_weights_.begin(), delta_weights_.end(), 0.0 );
    sigma_ = 100.0;
    delta_sigma_ = 0.0;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
NGNet::Unit::randomize( const double & min_weight,
                        const double & max_weight,
                        const double & initial_sigma )
{
    static std::mt19937 s_engine( std::time( 0 ) );

    double min_w = min_weight;
    double max_w = max_weight;
    if ( min_weight >= max_weight )
    {
        min_w = max_weight;
        max_w = min_weight;
    }

    std::uniform_real_distribution<> dst( min_w, max_w );

    std::generate( weights_.begin(),
                   weights_.end(),
                   [&]() {
                       return dst( s_engine );
                   } );

    sigma_ = initial_sigma;
}

/*-------------------------------------------------------------------*/
/*!

*/
NGNet::NGNet()
        : M_eta( 0.1 ),
          M_alpha( 0.9 ),
          M_min_weight( -100.0 ),
          M_max_weight( 100.0 ),
          M_initial_sigma( 100.0 )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
void
NGNet::addCenter( const input_vector & center )
{
    M_units.push_back( Unit() );

    M_units.back().center_ = center;
    M_units.back().randomize( M_min_weight, M_max_weight, M_initial_sigma );

    // adjust the deviation of gaussian function

    const std::size_t MAX = M_units.size();

    if ( MAX <= 1 )
    {
        return;
    }

#if 0
    for ( std::size_t i = 0; i < MAX; ++i )
    {
        double min_dist2 = std::numeric_limits< double >::max();
        for ( std::size_t j = 0; j < MAX; ++j )
        {
            if ( i == j ) continue;

            double d2 = M_units[i].dist2( M_units[j].center_ );
            if ( d2 < min_dist2 )
            {
                min_dist2 = d2;
            }
        }

        //M_units[i].sigma_ = std::sqrt( std::sqrt( min_dist2 ) * 0.5 );
        M_units[i].sigma_ = std::sqrt( std::sqrt( min_dist2 ) );
        std::cerr << "unit " << i << " min_dist = " << std::sqrt( min_dist2 )
                  << " sigma = " << M_units[i].sigma_
                  << std::endl;
    }
#else
    double dist_sum = 0.0;

    for ( std::size_t i = 0; i < MAX; ++i )
    {
        double min_dist2 = std::numeric_limits< double >::max();
        for ( std::size_t j = 0; j < MAX; ++j )
        {
            if ( i == j ) continue;

            double d2 = M_units[i].dist2( M_units[j].center_ );
            if ( d2 < min_dist2 )
            {
                min_dist2 = d2;
            }
        }

        dist_sum += std::sqrt( min_dist2 );
    }

    //double mean_sigma = std::sqrt( dist_sum / MAX ) * 2.0;
    double mean_sigma = 2.8 * dist_sum / MAX;
    for ( std::size_t i = 0; i < MAX; ++i )
    {
        M_units[i].sigma_ = mean_sigma;
    }
#endif
}

/*-------------------------------------------------------------------*/
/*!

*/
void
NGNet::propagate( const input_vector & input,
                  output_vector & output ) const
{
    // clear all value
    std::fill( output.begin(), output.end(), 0.0 );

    double sum_unit_value = 0.0;

    for ( const Unit & unit : M_units )
    {
        const double unit_value = unit.calc( input );
        sum_unit_value += unit_value;
        for ( std::size_t i = 0; i < OUTPUT; ++ i )
        {
            output[i] += unit_value * unit.weights_[i];
        }
    }

    //std::cerr << "propagate(). sum_unit_value = " << sum_unit_value << std::endl;
    /*
    if ( M_units.size() <= 1
         || sum_unit_value == 0.0 )
    {
        return;
    }
    */

    // normalize
    for ( double & o : output )
    {
        o /= sum_unit_value;
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
double
NGNet::train( const input_vector & input,
              const output_vector & teacher )
{
    output_vector output;
    propagate( input, output );

    output_vector output_back;

    // calculate output error back
    for ( std::size_t i = 0; i < OUTPUT; ++i )
    {
        double err = teacher[i] - output[i];
        output_back[i] = err * 1.0;        // d_linear
    }

    double sum_unit_value = 0.0;
    for ( const Unit & unit : M_units )
    {
        sum_unit_value += unit.calc( input );
    }

    //std::cerr << "train(). sum_unit_value = " << sum_unit_value << std::endl;

    // update each unit
    for ( Unit & unit : M_units )
    {
        const double unit_value = unit.calc( input );

        // update weights
        for ( std::size_t i = 0; i < OUTPUT; ++i )
        {
            unit.delta_weights_[i]
                = M_eta * output_back[i] * ( unit_value / sum_unit_value )
                + M_alpha * unit.delta_weights_[i];

            unit.weights_[i] += unit.delta_weights_[i];
        }
    }

    propagate( input, output );
    double total_error = 0.0;
    for ( std::size_t i = 0; i < OUTPUT; ++i )
    {
        double err = teacher[i] - output[i];
        total_error += err * err;
    }

    return total_error;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
NGNet::read( std::istream & is )
{
    int unit_size = 0;
    is >> unit_size;

    for ( int i = 0; i < unit_size; ++i )
    {
        Unit unit;
        for ( std::size_t i = 0; i < INPUT; ++i )
        {
            if ( ! is.good() ) return false;
            is >> unit.center_[i];
        }

        for ( std::size_t i = 0; i < OUTPUT; ++i )
        {
            if ( ! is.good() ) return false;
            is >> unit.weights_[i];
        }

        if ( ! is.good() ) return false;
        is >> unit.sigma_;

        M_units.push_back( unit );
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
NGNet::print( std::ostream & os ) const
{
    os << M_units.size() << " ";

    // update each unit
    for ( const Unit & unit : M_units )
    {
        // center
        std::copy( unit.center_.begin(),
                   unit.center_.end(),
                   std::ostream_iterator< double >( os, " " ) );
        // weights
        std::copy( unit.weights_.begin(),
                   unit.weights_.end(),
                   std::ostream_iterator< double >( os, " " ) );
        // sigma
        os << unit.sigma_ << " ";
    }

    return os << std::flush;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::ostream &
NGNet::printUnits( std::ostream & os ) const
{
    // update each unit
    int count = 0;
    for ( const Unit & unit : M_units )
    {
        ++count;
        std::cerr << " unit " << count
                  << " center = (" << unit.center_[0] << ","
                  << unit.center_[1] << "): ";
        std::cerr << "  sigma = " << unit.sigma_
                  << " delta = " << unit.delta_sigma_;
        std::cerr << "  weights = ";
        for ( std::size_t i = 0; i < OUTPUT; ++i )
        {
            std::cerr << unit.weights_[i]
                      << " delta = " << unit.delta_weights_[i] << " ";
        }
        std::cerr << '\n';
    }

    return os << std::flush;
}

}
