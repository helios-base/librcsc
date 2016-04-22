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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "sirms_model.h"

#include "sirm.h"

#include <iostream>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

 */
SIRMsModel::SIRMsModel( int num_sirms ):
    M_num_sirms( num_sirms )
{
    M_sirm.clear();
    M_sirm.resize( M_num_sirms );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
SIRMsModel::setModuleName( const size_t index,
                           const std::string & name )
{
    if ( index >= M_sirm.size() )
    {
        std::cerr << __FILE__ << ' ' << __LINE__
                  << ": illegal module index " << index << std::endl;
        return;
    }

    M_sirm[index].setModuleName( name );
}

/*-------------------------------------------------------------------*/
/*!

 */
double
SIRMsModel::calculateOutput( const std::vector< double > & input )
{
    double result = 0.0;

    int i = 0;
    for ( std::vector< SIRM >::iterator it = M_sirm.begin(), end = M_sirm.end();
          it != end;
          ++it, ++i )
    {
        double y = it->calculateOutput( input[i] );

        /*
        std::cout << "Output from " << i << "-th module: " << y[i];
        std::cout << " weight: " << (*it).weight() << std::endl;
        */

        result += it->weight() * y;
    }

    return result;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
SIRMsModel::specifyNumPartitions( const int index_module,
                                  const int num_partitions )
{
    M_sirm[index_module].setNumPartitions( num_partitions );
}

/*-------------------------------------------------------------------*/
/*!

 */
void
SIRMsModel::train( const double target,
                   const double actual )
{
    for ( std::vector< SIRM >::iterator it = M_sirm.begin(), end = M_sirm.end();
          it != end;
          ++it )
    {
        //
        // TODO: add it->calculateOutput() to get actual value and update SIRM::M_membership
        //

        it->trainSIRM( target, actual );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
SIRMsModel::specifyDomain( const int index_attribute,
                           const double min_domain,
                           const double max_domain )
{
    M_sirm[index_attribute].setDomain( min_domain, max_domain );
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
SIRMsModel::saveParameters( const std::string & prefix )
{
    for ( std::vector< SIRM >::iterator it = M_sirm.begin(), end = M_sirm.end();
          it != end;
          ++it )
    {
        if ( ! it->saveParameters( prefix ) )
        {
            return false;
        }
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
SIRMsModel::loadParameters( const std::string & prefix )
{
    for ( std::vector< SIRM >::iterator it = M_sirm.begin(), end = M_sirm.end();
          it != end;
          ++it )
    {
        if ( ! it->loadParameters( prefix ) )
        {
            return false;
        }
    }

    return true;
}

}
