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

#include <boost/random.hpp>

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
    static boost::mt19937 gen( std::time( 0 ) );

    double min_w = min_weight;
    double max_w = max_weight;
    if ( min_weight >= max_weight )
    {
        min_w = max_weight;
        max_w = min_weight;
    }

    boost::uniform_real<> dst( min_w, max_w );
    boost::variate_generator< boost::mt19937 &, boost::uniform_real<> >
        rng( gen, dst );

    std::generate( weights_.begin(),
                   weights_.end(),
                   rng );

    sigma_ = initial_sigma;
}

/*-------------------------------------------------------------------*/
/*!

*/
NGNet::NGNet()
        : M_eta( 0.1 )
        , M_alpha( 0.9 )
        , M_min_weight( -100.0 )
        , M_max_weight( 100.0 )
        , M_initial_sigma( 100.0 )
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

    const std::vector< Unit >::const_iterator end = M_units.end();
    for ( std::vector< Unit >::const_iterator it = M_units.begin();
          it != end;
          ++it )
    {
        const double unit_value = it->calc( input );
        sum_unit_value += unit_value;
        for ( std::size_t i = 0; i < OUTPUT; ++ i )
        {
            output[i] += unit_value * it->weights_[i];
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
    for ( output_vector::iterator it = output.begin();
          it != output.end();
          ++it )
    {
        *it /= sum_unit_value;
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

    const std::vector< Unit >::iterator end = M_units.end();

    double sum_unit_value = 0.0;
    for ( std::vector< Unit >::const_iterator it = M_units.begin();
          it != end;
          ++it )
    {
        sum_unit_value += it->calc( input );
    }

    //std::cerr << "train(). sum_unit_value = " << sum_unit_value << std::endl;

    // update each unit
    for ( std::vector< Unit >::iterator it = M_units.begin();
          it != end;
          ++it )
    {
        const double unit_value = it->calc( input );

        // update weights
        for ( std::size_t i = 0; i < OUTPUT; ++i )
        {
            it->delta_weights_[i]
                = M_eta * output_back[i] * ( unit_value / sum_unit_value )
                + M_alpha * it->delta_weights_[i];

            it->weights_[i] += it->delta_weights_[i];
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
    const std::vector< Unit >::const_iterator end = M_units.end();
    for ( std::vector< Unit >::const_iterator it = M_units.begin();
          it != end;
          ++it )
    {
        // center
        std::copy( it->center_.begin(),
                   it->center_.end(),
                   std::ostream_iterator< double >( os, " " ) );
        // weights
        std::copy( it->weights_.begin(),
                   it->weights_.end(),
                   std::ostream_iterator< double >( os, " " ) );
        // sigma
        os << it->sigma_ << " ";
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
    const std::vector< Unit >::const_iterator end = M_units.end();
    for ( std::vector< Unit >::const_iterator it = M_units.begin();
          it != end;
          ++it )
    {
        std::cerr << " unit " << ++count
                  << " center = (" << it->center_[0] << ","
                  << it->center_[1] << "): ";
        std::cerr << "  sigma = " << it->sigma_
                  << " delta = " << it->delta_sigma_;
        std::cerr << "  weights = ";
        for ( std::size_t i = 0; i < OUTPUT; ++i )
        {
            std::cerr << it->weights_[i]
                      << " delta = " << it->delta_weights_[i] << " ";
        }
        std::cerr << '\n';
    }

    return os << std::flush;
}

}
