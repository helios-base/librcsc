// -*-c++-*-

/*!
  \file rbf.cpp
  \brief Basic Radial Basis Function Network class Source File.
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

#include "rbf.h"

#include <random>
#include <algorithm>
#include <numeric>
#include <string>
#include <sstream>
#include <ctime>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
RBFNetwork::Unit::Unit( const std::size_t input_dim,
                        const std::size_t output_dim )
    : center_( input_dim, 0.0 ),
      weights_( output_dim, 0.0 ),
      delta_weights_( output_dim, 0.0 ),
      sigma_( 100.0 ),
      delta_sigma_( 0.0 )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
void
RBFNetwork::Unit::randomize( const double & min_weight,
                             const double & max_weight,
                             const double & initial_sigma )
{
    //static std::mt19937 s_engine( std::random_device()() );
    // static std::random_device rng;
    // static std::mt19937 s_engine( rng() );
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
RBFNetwork::RBFNetwork( const std::size_t input_dim,
                        const std::size_t output_dim )
        : M_input_dim( input_dim ),
          M_output_dim( output_dim ),
          M_eta( 0.1 ),
          M_alpha( 0.5 ),
          M_min_weight( -100.0 ),
          M_max_weight( 100.0 ),
          M_initial_sigma( 100.0 )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
void
RBFNetwork::addCenter( const input_vector & center )
{
    M_units.push_back( Unit( M_input_dim, M_output_dim ) );

    M_units.back().center_ = center;
    M_units.back().randomize( M_min_weight, M_max_weight, M_initial_sigma );

    // adjust the deviation of gaussian function

    const std::size_t MAX = M_units.size();

    if ( MAX <= 1 )
    {
        return;
    }

    double dist_sum = 0.0;

    for ( std::size_t i = 0; i < MAX; ++i )
    {
        // get the length to the nearest unit
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
}

/*-------------------------------------------------------------------*/
/*!

*/
void
RBFNetwork::propagate( const input_vector & input,
                       output_vector & output ) const
{
    if ( input.size() != M_input_dim )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << "  illegal input vector size. " << input.size()
                  << "(input) != " << M_input_dim << "(required)"
                  << std::endl;
        return;
    }

    // clear all value
    output.resize( M_output_dim, 0.0 );
    std::fill( output.begin(), output.end(), 0.0 );

    const std::size_t OUTPUT = M_output_dim;

    for ( const Unit & unit : M_units )
    {
        const double unit_value = unit.calc( input );
        for ( std::size_t i = 0; i < OUTPUT; ++i )
        {
            output[i] += unit_value * unit.weights_[i];
        }
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
double
RBFNetwork::train( const input_vector & input,
                   const output_vector & teacher )
{
    if ( input.size() != M_input_dim )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << "  illegal input vector size. " << input.size()
                  << "(input) != " << M_input_dim << "(required)"
                  << std::endl;
        return 0.0;
    }

    if ( teacher.size() != M_output_dim )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << "  illegal output vector size. " << teacher.size()
                  << "(input) != " << M_output_dim << "(required)"
                  << std::endl;
        return 0.0;
    }

    const std::size_t OUTPUT = M_output_dim;

    output_vector output( OUTPUT, 0.0 );
    propagate( input, output );

    output_vector output_back( OUTPUT, 0.0 );

    // calculate output error back
    for ( std::size_t i = 0; i < OUTPUT; ++i )
    {
        output_back[i] = teacher[i] - output[i];
    }

    // update each unit
    for ( Unit & unit : M_units )
    {
        const double unit_value = unit.calc( input );
#if 0
        const double dist2 = unit.dist2( input );
        const double sigma3 = std::pow( unit.sigma_, 3 );

        double back_sum = 0.0;
        for ( std::size_t i = 0; i < OUTPUT; ++i )
        {
            back_sum += output_back[i] * unit.weights_[i];
        }

        // update sigma (d_gaussian)
        unit.delta_sigma_
            = M_eta * back_sum * ( dist2 / sigma3 ) * unit_value
            + M_alpha * unit.delta_sigma_;
        unit.sigma_ += unit.delta_sigma_;
#endif
        // update weights
        for ( std::size_t i = 0; i < OUTPUT; ++i )
        {
            unit.delta_weights_[i]
                = M_eta * output_back[i] * unit_value
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
RBFNetwork::read( std::istream & is )
{
    int unit_size = 0;
    is >> unit_size;

    for ( int i = 0; i < unit_size; ++i )
    {
        Unit unit( M_input_dim, M_output_dim );

        for ( std::size_t i = 0; i < M_input_dim; ++i )
        {
            if ( ! is.good() ) return false;
            is >> unit.center_[i];
        }

        for ( std::size_t i = 0; i < M_output_dim; ++i )
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
RBFNetwork::print( std::ostream & os ) const
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
RBFNetwork::printUnits( std::ostream & os ) const
{
    // update each unit
    int count = 0;
    for ( const Unit & unit : M_units )
    {
        os << "unit " << ++count;
        os << ": center = (";
        for ( const double & c : unit.center_ )
        {
            os << c << ' ';
        }
        os << ')';

        os << " sigma = " << unit.sigma_
           << " delta = " << unit.delta_sigma_;

        os << "  weights(delta) : ";
        for ( std::size_t i = 0; i < M_input_dim; ++i )
        {
            os << " (" <<unit.weights_[i]
               << ' ' << unit.delta_weights_[i] << ')';
        }
        os << ')';
        os << '\n';
    }

    return os << std::flush;
}

}
