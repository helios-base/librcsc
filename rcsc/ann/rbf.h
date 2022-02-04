// -*-c++-*-

/*!
  \file rbf.h
  \brief Basic Radial Basis Function Network class Header File.
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

#ifndef RCSC_ANN_RBF_H
#define RCSC_ANN_RBF_H

#include <boost/array.hpp>

#include <vector>
#include <iostream>
#include <cmath>

namespace rcsc {

////////////////////////////////////////////////////////////////

/*!
  \class RBFNetwork
  \brief Radial Basis Function Network.
*/
class RBFNetwork {
public:

    //! typedef of the input value container
    typedef std::vector< double > input_vector;
    //! typedef of the output value container
    typedef std::vector< double > output_vector;

    /*!
      \struct Unit
      \brief radial basis function unit
     */
    struct Unit {
        input_vector center_; //!< center point of this unit

        output_vector weights_; //!< weights to output
        output_vector delta_weights_; //!< delta of weights

        double sigma_; //!< function parameter. this value must be >0
        double delta_sigma_; //!< delta of sigma

    private:
        // not used
        Unit() = delete;

    public:
        /*!
          \brief all weight is initialized by 0. default sigma = 100.0
          \param input_dim the input dimension
          \param output_dim the output dimension
         */
        Unit( std::size_t input_dim,
              std::size_t output_dim );

        /*!
          \brief set unit parameter randomly
          \param min_weight minimum output weight
          \param max_weight maximum output weight
          \param initial_sigma initial sigma value
         */
        void randomize( const double & min_weight,
                        const double & max_weight,
                        const double & initial_sigma );

        /*!
          \brief calculate distance from the input vector
          \param input input value
          \return squared distance value
         */
        double dist2( const input_vector & input ) const
          {
              const std::size_t INPUT = input.size();
              if ( INPUT != center_.size() )
              {
                  return 0.0;
              }

              double d2 = 0.0;
              for ( std::size_t i = 0; i < INPUT; ++i )
              {
                  d2 += std::pow( center_[i] - input[i], 2 );
              }
              return d2;
          }

        /*!
          \brief calculate distance from the input vector
          \param input input value
          \return distance value
         */
        double dist( const input_vector & input ) const
          {
              return std::sqrt( dist2( input ) );
          }

        /*!
          \brief calculate output value for the input
          \param input value
         */
        double calc( const input_vector & input ) const
          {
              return std::exp( - dist2( input ) / ( 2.0 * sigma_ * sigma_ ) );
          }
    };

private:

    const std::size_t M_input_dim; //!< input dimension
    const std::size_t M_output_dim; //!< output dimension

    double M_eta; //!< learning rate parameter
    double M_alpha; //!< learning rate parameter
    double M_min_weight; //!< minimum connection weight
    double M_max_weight; //!< maximum connection weight
    double M_initial_sigma; //!< basis fucntion's initial sigma

    std::vector< Unit > M_units; //!< all units

    // not used
    RBFNetwork() = delete;

public:

    /*!
      \brief all weight is initialized by 0. default sigma = 100.0
      \param input_dim the input dimension
      \param output_dim the output dimension
    */
    RBFNetwork( const std::size_t input_dim,
                const std::size_t output_dim );

    /*!
      \brief set learning rate
      \param eta learning rate parameter
      \param alpha learning rate parameter
     */
    void setLearningRate( const double & eta,
                          const double & alpha )
      {
          M_eta = eta;
          M_alpha = alpha;
      }

    /*!
      \brief set connection weight range
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
      \brief set basis function's initial sigma
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
      \brief calculate output value
      \param input input value
      \param output reference to the result variable
     */
    void propagate( const input_vector & input,
                    output_vector & output ) const;

    /*!
      \brief train the connection weight
      \param input input value
      \param teacher teacher output value
      \return summed squared error value
     */
    double train( const input_vector & input,
                  const output_vector & teacher );

    /*!
      \brief read network structure from an input stream
      \param is reference to the input stream
      \return parsing result
     */
    bool read( std::istream & is );

    /*!
      \brief write network structure to an output stream
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & print( std::ostream & os ) const;

    /*!
      \brief write detailed unit information
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & printUnits( std::ostream & os ) const;

};

}

#endif
