// -*-c++-*-

/*
 *Copyright:

 Copyright (C) Tomoharu NAKASHIMA

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3, or (at your option)
 any later version.

 This code is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this code; see the file COPYING.  If not, write to
 the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

 *EndCopyright:
 */

/////////////////////////////////////////////////////////////////////

#ifndef RCSC_ANN_SIRMS_MODEL_H
#define RCSC_ANN_SIRMS_MODEL_H

#include <string>
#include <vector>
#include <cstddef> // size_t

namespace rcsc {

class SIRM;

/*!
  \class SIRMsModel
*/
class SIRMsModel {
private:
    int M_num_sirms;
    std::vector< SIRM > M_sirm;
public:

    explicit
    SIRMsModel( int num_sirms = 1 );

    int numSIRMs() const
      {
          return M_num_sirms;
      }

    void setModuleName( const size_t index,
                        const std::string & name );


    /*! calculate an output for an input vector */
    double calculateOutput( const std::vector< double > & input );

    /*! specify the number of fuzzy partitions of an SIRM */
    void specifyNumPartitions( const int index_module,
                               const int num_partitions );

    /*! specify the range of the target function for each attribute */
    void specifyDomain( const int index_attribute,
                        const double min_domain,
                        const double max_domain );

    /*! train the SIRMs model */
    void train( const double target,
                const double actual );

    /*! save parameters of SIRMs */
    bool saveParameters( const std::string & prefix );

    /*! load parameters of SIRMs */
    bool loadParameters( const std::string & prefix );
};

}

#endif
