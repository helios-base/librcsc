// -*-c++-*-

/*!
  \file bpn1.h
  \brief Basic 3 layer Back Propagation Neural Network class Header File.
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

#ifndef RCSC_ANN_BPN1_H
#define RCSC_ANN_BPN1_H

#include <array>
#include <algorithm>
#include <numeric> // inner_product
#include <iostream>
#include <cmath>

namespace rcsc {

////////////////////////////////////////////////////////////////

/*!
  \struct SigmoidFunc
  \brief sigmoid function and differencial function object
*/
struct SigmoidFunc {
    /*!
      \brief functional operator
      \param x input value
      \return output value of sigmoid function
     */
    double operator()( const double & x ) const
      {
          return 1.0 / ( 1.0 + std::exp( - x ) );
      }

    /*!
      \brief inverse function
      \param y output value of this function.
      \return x value for y
     */
    double inverse( const double & y ) const
      {
          return - std::log( 1.0 / y - 1.0 );
      }

    /*!
      =brief differencial function for x
      \param x input value
      \return differencial value for input
     */
    double diffAtX( const double & x ) const
      {
          double s = this->operator()( x );
          return s * ( 1.0 - s );
      }

    /*!
      \brief differencial function for y
      \param y some output value of this function
      \return differencial value for output
     */
    double diffAtY( const double & y ) const
      {
          // return diffAtX( inverse( y ) );
          return y * ( 1.0 - y );
      }
};

////////////////////////////////////////////////////////////////

/*!
  \struct LinearFunc
  \brief linear function and differenceial function objcet
*/
struct LinearFunc {
    /*!
      \brief functional operator
      \param x input value
      \return output value of linear function
     */
    double operator()( const double & x ) const
      {
          return x;
      }

    /*!
      \brief inverse function
      \param y output value of this function.
      \return x value for y
     */
    double inverse( const double & y ) const
      {
          return y;
      }

    /*!
      \brief differencial function for x
      \return output value of differencial linear function
     */
    double diffAtX( const double & ) const
      {
          return 1.0;
      }

    /*!
      \brief differencial function for y
      \return output value of differencial linear function
     */
    double diffAtY( const double & ) const
      {
          return 1.0;
      }

};

////////////////////////////////////////////////////////////////

/*!
  \brief Back Propagetion Neural Network

  This class can take only one hidden layer,
  but unit number and activation function can be specified
  by template parameters.
*/
template < std::size_t INPUT,
           std::size_t HIDDEN,
           std::size_t OUTPUT,
           typename FuncH = SigmoidFunc,
           typename FuncO = SigmoidFunc >
class BPNetwork1 {
public:
    typedef double value_type; //!< typedef of the value type

    //! typedef of the input array type that uses template parameter.
    typedef std::array< value_type, INPUT > input_array;
    //! typedef of the output array type that uses template parameter.
    typedef std::array< value_type, OUTPUT > output_array;

private:

    //! learning parameter
    const value_type M_eta;
    //! learning parameter
    const value_type M_alpha;

    //! connection between input and hidden layer. bias weight is included.
    std::array< value_type, INPUT + 1 > M_weight_i_to_h[HIDDEN];
    //! delta weight between input and hidden layer. bias weight is included.
    std::array< value_type, INPUT + 1 > M_delta_weight_i_to_h[HIDDEN];

    //! connection between hidden and output layer. bias weight is included.
    std::array< value_type, HIDDEN + 1 > M_weight_h_to_o[OUTPUT];
    //! delta weight between hidden and output layer. bias weight is included.
    std::array< value_type, HIDDEN + 1 > M_delta_weight_h_to_o[OUTPUT];

    /*!
      internal value holder.
      last point value is used as bias input, so back value must be 1.
    */
    mutable std::array< value_type, HIDDEN + 1 > M_hidden_layer;

public:
    /*!
      \brief default constructor

      default training parameter is set.
     */
    BPNetwork1()
        : M_eta( 0.3 )
        , M_alpha( 0.9 )
      {
          init();
      }

    /*!
      \brief create with learning parameter
      \param eta training parameter
      \param alpha training parameter
     */
    BPNetwork1( const value_type & eta,
                const value_type & alpha )
        : M_eta( eta )
        , M_alpha( alpha )
      {
          init();
      }

    /*!
      \brief create with random number generator
      \param eta training parameter
      \param alpha training parameter
      \param rng referenct to the random number generator object
     */
    template < typename RNG >
    BPNetwork1( const value_type & eta,
                const value_type & alpha,
                RNG & rng )
        : M_eta( eta )
        , M_alpha( alpha )
      {
          init();
          randomize( rng );
      }

    /*!
      \brief init member variables
     */
    void init()
      {
          M_hidden_layer.assign( 0 );
          M_hidden_layer.back() = 1;
          for ( std::size_t i = 0; i < HIDDEN; ++i )
          {
              M_weight_i_to_h[i].assign( 0 );
              M_delta_weight_i_to_h[i].assign( 0 );
          }
          for ( std::size_t i = 0; i < OUTPUT; ++i )
          {
              M_weight_h_to_o[i].assign( 0 );
              M_delta_weight_h_to_o[i].assign( 0 );
          }
      }

    /*!
      \brief create unit connection randomly
      \param rng referenct to the random number generator object
     */
    template < typename RNG >
    void randomize( RNG & rng )
      {
          for ( std::size_t i = 0; i < HIDDEN; ++i )
          {
              std::generate( M_weight_i_to_h[i].begin(),
                             M_weight_i_to_h[i].end(),
                             rng );
          }
          for ( std::size_t i = 0; i < OUTPUT; ++i )
          {
              std::generate( M_weight_h_to_o[i].begin(),
                             M_weight_h_to_o[i].end(),
                             rng );
          }
      }

    /*!
      \brief simulate network.
      \param input input data
      \param output reference to the data holder variable
    */
    void propagate( const input_array & input,
                    output_array & output ) const
      {
          // Input to Hidden
          FuncH func_h;
          for ( std::size_t i = 0; i < HIDDEN; ++i )
          {
              value_type sum = std::inner_product( input.begin(),
                                                   input.end(),
                                                   M_weight_i_to_h[i].begin(),
                                                   static_cast< value_type >( 0 ) );
              // add bias
              sum += M_weight_i_to_h[i].back();
              M_hidden_layer[i] = func_h( sum );
          }
          // Hidden to Output
          FuncO func_o;
          for ( std::size_t i = 0; i < OUTPUT; ++i )
          {
              value_type sum = std::inner_product( M_hidden_layer.begin(),
                                                   M_hidden_layer.end(),
                                                   M_weight_h_to_o[i].begin(),
                                                   static_cast< value_type >( 0 ) );
              // bias is already added
              output[i] = func_o( sum );
          }
      }

    /*!
      \brief update unit connection weights using teacher signal
      \param input input data
      \param teacher teaching signal data
    */
    value_type train( const input_array & input,
                      const output_array & teacher )
      {
          output_array output;
          propagate( input, output );

          // error value mulitiplied by differential
          output_array output_back;

          // caluculate output error back
          FuncO func_o;
          for ( std::size_t i = 0; i < OUTPUT; ++i )
          {
              value_type err = teacher[i] - output[i];
              output_back[i] = err * func_o.diffAtY( output[i] );
          }

          // caluculate hidden layer error back
          std::array< value_type, HIDDEN > hidden_back;
          FuncH func_h;
          for ( std::size_t i = 0; i < HIDDEN; ++i )
          {
              value_type sum = 0;
              for ( std::size_t j = 0; j < OUTPUT; ++j )
              {
                  sum += output_back[j] * M_weight_h_to_o[j][i];
              }
              hidden_back[i] = sum * func_h.diffAtY( M_hidden_layer[i] );
          }

          // update weights hidden to out
          for ( std::size_t i = 0; i < OUTPUT; ++i )
          {
              for ( std::size_t j = 0; j < HIDDEN + 1; ++j )
              {
                  M_delta_weight_h_to_o[i][j]
                      = M_eta * M_hidden_layer[j] * output_back[i]
                      + M_alpha * M_delta_weight_h_to_o[i][j];
                  M_weight_h_to_o[i][j]
                      += M_delta_weight_h_to_o[i][j];
                  //+= M_epsilon * ( M_hidden_layer[i] * output_back[j] );
              }
          }

          // update weights input to hidden
          for ( std::size_t i = 0; i < HIDDEN; ++i )
          {
              for ( std::size_t j = 0; j < INPUT; ++j )
              {
                  M_delta_weight_i_to_h[i][j]
                      = M_eta * input[j] * hidden_back[i]
                      + M_alpha * M_delta_weight_i_to_h[i][j];
                  M_weight_i_to_h[i][j]
                      += M_delta_weight_i_to_h[i][j];
                  //+= M_epsilon * ( input[j] * hidden_back[i] );
              }
          }
          // update input layer bias
          for ( std::size_t i = 0; i < HIDDEN; ++i )
          {
              M_delta_weight_i_to_h[i][INPUT]
                  = M_eta * 1.0 * hidden_back[i]
                  + M_alpha * M_delta_weight_i_to_h[i][INPUT];
              M_weight_i_to_h[i][INPUT]
                  += M_delta_weight_i_to_h[i][INPUT];
              //+= M_epsilon * ( input[j] * hidden_back[i] );
          }

          // calcluate error after training
          value_type total_error = 0;
          propagate( input, output );
          for ( std::size_t i = 0; i < OUTPUT; ++i )
          {
              total_error += std::pow( teacher[i] - output[i], 2 );
          }
          //std::cout << "  error = " << total_error << std::endl;
          return total_error;
      }


    ///////////////////////////////////////////////////
    // stream I/O

    /*!
      \brief read network structure from input stream.
      \param is reference to the input stream
      \return true if successfully ended.
     */
    bool read( std::istream & is )
      {
          for ( std::size_t i = 0; i < HIDDEN; ++i )
          {
              for ( std::size_t j = 0; j < INPUT + 1; ++j )
              {
                  if ( ! is.good() ) return false;
                  is >> M_weight_i_to_h[i][j];
              }
          }
          for ( std::size_t i = 0; i < OUTPUT; ++i )
          {
              for ( std::size_t j = 0; j < HIDDEN + 1; ++ j )
              {
                  if ( ! is.good() ) return false;
                  is >> M_weight_h_to_o[i][j];
              }
          }
          return true;
      }

    /*!
      \brief put network structure to stream by "one" line
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & print( std::ostream & os ) const
      {
          for ( std::size_t i = 0; i < HIDDEN; ++i )
          {
              std::copy( M_weight_i_to_h[i].begin(),
                         M_weight_i_to_h[i].end(),
                         std::ostream_iterator< value_type >( os, " " ) );
          }
          for ( std::size_t i = 0; i < OUTPUT; ++i )
          {
              std::copy( M_weight_h_to_o[i].begin(),
                         M_weight_h_to_o[i].end(),
                         std::ostream_iterator< value_type >( os, " " ) );
          }
          return os;
      }
};

}

#endif
