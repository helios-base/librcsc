// -*-c++-*-

/*!
  \file ngnet.h
  \brief Normalized Gaussian Network class Header File.
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

#ifndef RCSC_ANN_NGNET_H
#define RCSC_ANN_NGNET_H

#include <array>
#include <vector>
#include <iostream>
#include <cmath>

namespace rcsc {

////////////////////////////////////////////////////////////////

/*!
  \class NGNet
  \brief Normalized Gaussian Radial Basis Function Network
*/
class NGNet {
public:

    enum {
        INPUT = 2,
    };

    enum {
        OUTPUT = 2,
    };

    //! typedef of the input array type that uses fixed size
    typedef std::array< double, INPUT > input_vector;
    //! typedef of the output array type that uses fixed size
    typedef std::array< double, OUTPUT > output_vector;

    /*!
      \struct Unit
      \brief radial basis function unit
     */
    struct Unit {
        input_vector center_; //!< center point

        output_vector weights_; //!< weights to output
        output_vector delta_weights_; //!< last delta of the weith while learning

        double sigma_; //!< variance parameger. must be >0
        double delta_sigma_; //!< last delta of the sigma while learning

        /*!
          \brief initialize member variables
         */
        Unit();

        /*!
          \brief randomly initialize member variables with given range
          \param min_weight minimum weight
          \param max_weight maximum weight
          \param initial_sigma initial sigma value
         */
        void randomize( const double & min_weight,
                        const double & max_weight,
                        const double & initial_sigma );

        /*!
          \brief calculate the squared distance form this unit to the given point
          \param input input point
          \return squared distance to the input
         */
        double dist2( const input_vector & input ) const
          {
              double dist2 = 0.0;
              for ( std::size_t i = 0; i < INPUT; ++i )
              {
                  dist2 += std::pow( center_[i] - input[i], 2 );
              }
              return dist2;
          }

        /*!
          \brief calculate the unit output value with Gaussian function
          \param input input point
          \return unit output value
         */
        double calc( const input_vector & input ) const
          {
              return std::exp( - dist2( input ) / ( 2.0 * sigma_ * sigma_ ) );
          }
    };

private:

    double M_eta; //!< learning parameter
    double M_alpha; //!< learning parameter
    double M_min_weight; //!< minimum weight
    double M_max_weight; //!< maximum weight
    double M_initial_sigma; //!< initial sigma value

    std::vector< Unit > M_units; //!< container of the unit

public:

    /*!
      \brief initialize member variables
     */
    NGNet();

    /*!
      \brief assign learning parameters
      \param eta new learning parameter
      \param alpha new learning parameter
     */
    void setLearningRate( const double & eta,
                          const double & alpha )
      {
          M_eta = eta;
          M_alpha = alpha;
      }

    /*!
      \brief assign the range of the network connection weight
      \param min_weight minimum weight
      \param max_weight maximum weight
     */
    void setWeightRange( const double & min_weight,
                         const double & max_weight )
      {
          M_min_weight = min_weight;
          M_max_weight = max_weight;
      }

    /*!
      \brief assign the initial sigma value
      \param initial_sigma sigma value
     */
    void setInitialSigma( const double & initial_sigma )
      {
          M_initial_sigma = initial_sigma;
      }

    /*!
      \brief get the unit container
      \return const reference to the unit container
     */
    const
    std::vector< Unit > & units() const
      {
          return M_units;
      }

    /*!
      \brief add new center point
      \param center new center point
     */
    void addCenter( const input_vector & center );

    /*!
      \brief calculate the output of this network
      \param input input value
      \param output reference to the result variable
     */
    void propagate( const input_vector & input,
                    output_vector & output ) const;

    /*!
      \brief train this network with teacher signal
      \param input input value
      \param teacher teacher output value
      \return sum of the squared error value
     */
    double train( const input_vector & input,
                  const output_vector & teacher );

    /*!
      \brief load network structure from input stream
      \param is reference to the input stream
      \return true if successfully read
     */
    bool read( std::istream & is );

    /*!
      \brief print network structor
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & print( std::ostream & os ) const;

    /*!
      \brief print all units, not network connection weights
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & printUnits( std::ostream & os ) const;

};

}

#endif
