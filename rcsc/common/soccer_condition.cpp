// -*-c++-*-

/*!
  \file soccer_condition.cpp
  \brief abstract condition class Source File
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

#include "soccer_condition.h"

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
AndCondition::AndCondition( const std::shared_ptr< const Condition > & p1,
                            const std::shared_ptr< const Condition > & p2 )
{
    M_condition_set.push_back( p1 );
    M_condition_set.push_back( p2 );
}

/*-------------------------------------------------------------------*/
/*!

*/
AndCondition::AndCondition( const std::shared_ptr< const Condition > & p1,
                            const std::shared_ptr< const Condition > & p2,
                            const std::shared_ptr< const Condition > & p3 )
{
    M_condition_set.push_back( p1 );
    M_condition_set.push_back( p2 );
    M_condition_set.push_back( p3 );
}

/*-------------------------------------------------------------------*/
/*!

*/
AndCondition::AndCondition( const std::shared_ptr< const Condition > & p1,
                            const std::shared_ptr< const Condition > & p2,
                            const std::shared_ptr< const Condition > & p3,
                            const std::shared_ptr< const Condition > & p4 )
{
    M_condition_set.push_back( p1 );
    M_condition_set.push_back( p2 );
    M_condition_set.push_back( p3 );
    M_condition_set.push_back( p4 );
}

/*-------------------------------------------------------------------*/
/*!

*/
AndCondition::AndCondition( const std::shared_ptr< const Condition > & p1,
                            const std::shared_ptr< const Condition > & p2,
                            const std::shared_ptr< const Condition > & p3,
                            const std::shared_ptr< const Condition > & p4,
                            const std::shared_ptr< const Condition > & p5 )
{
    M_condition_set.push_back( p1 );
    M_condition_set.push_back( p2 );
    M_condition_set.push_back( p3 );
    M_condition_set.push_back( p4 );
    M_condition_set.push_back( p5 );
}

/*-------------------------------------------------------------------*/
/*!

*/
AndCondition::AndCondition( const std::shared_ptr< const Condition > & p1,
                            const std::shared_ptr< const Condition > & p2,
                            const std::shared_ptr< const Condition > & p3,
                            const std::shared_ptr< const Condition > & p4,
                            const std::shared_ptr< const Condition > & p5,
                            const std::shared_ptr< const Condition > & p6 )
{
    M_condition_set.push_back( p1 );
    M_condition_set.push_back( p2 );
    M_condition_set.push_back( p3 );
    M_condition_set.push_back( p4 );
    M_condition_set.push_back( p5 );
    M_condition_set.push_back( p6 );
}

/*-------------------------------------------------------------------*/
/*!

*/
AndCondition::AndCondition( const std::shared_ptr< const Condition > & p1,
                            const std::shared_ptr< const Condition > & p2,
                            const std::shared_ptr< const Condition > & p3,
                            const std::shared_ptr< const Condition > & p4,
                            const std::shared_ptr< const Condition > & p5,
                            const std::shared_ptr< const Condition > & p6,
                            const std::shared_ptr< const Condition > & p7 )
{
    M_condition_set.push_back( p1 );
    M_condition_set.push_back( p2 );
    M_condition_set.push_back( p3 );
    M_condition_set.push_back( p4 );
    M_condition_set.push_back( p5 );
    M_condition_set.push_back( p6 );
    M_condition_set.push_back( p7 );
}

/*-------------------------------------------------------------------*/
/*!

*/
AndCondition::AndCondition( const std::shared_ptr< const Condition > & p1,
                            const std::shared_ptr< const Condition > & p2,
                            const std::shared_ptr< const Condition > & p3,
                            const std::shared_ptr< const Condition > & p4,
                            const std::shared_ptr< const Condition > & p5,
                            const std::shared_ptr< const Condition > & p6,
                            const std::shared_ptr< const Condition > & p7,
                            const std::shared_ptr< const Condition > & p8 )
{
    M_condition_set.push_back( p1 );
    M_condition_set.push_back( p2 );
    M_condition_set.push_back( p3 );
    M_condition_set.push_back( p4 );
    M_condition_set.push_back( p5 );
    M_condition_set.push_back( p6 );
    M_condition_set.push_back( p7 );
    M_condition_set.push_back( p8 );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
AndCondition::operator()( const PlayerAgent * agent ) const
{
    const std::vector< std::shared_ptr< const Condition > >::const_iterator
        end = M_condition_set.end();

    for ( std::vector< std::shared_ptr< const Condition > >::const_iterator
              it = M_condition_set.begin();
          it != end;
          ++it )
    {
        if ( ! (*it)->operator()( agent ) )
        {
            return false;
        }
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
OrCondition::OrCondition( const std::shared_ptr< const Condition > & p1,
                          const std::shared_ptr< const Condition > & p2 )
{
    M_condition_set.push_back( p1 );
    M_condition_set.push_back( p2 );
}

/*-------------------------------------------------------------------*/
/*!

*/
OrCondition::OrCondition( const std::shared_ptr< const Condition > & p1,
                          const std::shared_ptr< const Condition > & p2,
                          const std::shared_ptr< const Condition > & p3 )
{
    M_condition_set.push_back( p1 );
    M_condition_set.push_back( p2 );
    M_condition_set.push_back( p3 );
}

/*-------------------------------------------------------------------*/
/*!

*/
OrCondition::OrCondition( const std::shared_ptr< const Condition > & p1,
                          const std::shared_ptr< const Condition > & p2,
                          const std::shared_ptr< const Condition > & p3,
                          const std::shared_ptr< const Condition > & p4 )
{
    M_condition_set.push_back( p1 );
    M_condition_set.push_back( p2 );
    M_condition_set.push_back( p3 );
    M_condition_set.push_back( p4 );
}

/*-------------------------------------------------------------------*/
/*!

*/
OrCondition::OrCondition( const std::shared_ptr< const Condition > & p1,
                          const std::shared_ptr< const Condition > & p2,
                          const std::shared_ptr< const Condition > & p3,
                          const std::shared_ptr< const Condition > & p4,
                          const std::shared_ptr< const Condition > & p5 )
{
    M_condition_set.push_back( p1 );
    M_condition_set.push_back( p2 );
    M_condition_set.push_back( p3 );
    M_condition_set.push_back( p4 );
    M_condition_set.push_back( p5 );
}

/*-------------------------------------------------------------------*/
/*!

*/
OrCondition::OrCondition( const std::shared_ptr< const Condition > & p1,
                          const std::shared_ptr< const Condition > & p2,
                          const std::shared_ptr< const Condition > & p3,
                          const std::shared_ptr< const Condition > & p4,
                          const std::shared_ptr< const Condition > & p5,
                          const std::shared_ptr< const Condition > & p6 )
{
    M_condition_set.push_back( p1 );
    M_condition_set.push_back( p2 );
    M_condition_set.push_back( p3 );
    M_condition_set.push_back( p4 );
    M_condition_set.push_back( p5 );
    M_condition_set.push_back( p6 );
}

/*-------------------------------------------------------------------*/
/*!

*/
OrCondition::OrCondition( const std::shared_ptr< const Condition > & p1,
                          const std::shared_ptr< const Condition > & p2,
                          const std::shared_ptr< const Condition > & p3,
                          const std::shared_ptr< const Condition > & p4,
                          const std::shared_ptr< const Condition > & p5,
                          const std::shared_ptr< const Condition > & p6,
                          const std::shared_ptr< const Condition > & p7 )
{
    M_condition_set.push_back( p1 );
    M_condition_set.push_back( p2 );
    M_condition_set.push_back( p3 );
    M_condition_set.push_back( p4 );
    M_condition_set.push_back( p5 );
    M_condition_set.push_back( p6 );
    M_condition_set.push_back( p7 );
}

/*-------------------------------------------------------------------*/
/*!

*/
OrCondition::OrCondition( const std::shared_ptr< const Condition > & p1,
                          const std::shared_ptr< const Condition > & p2,
                          const std::shared_ptr< const Condition > & p3,
                          const std::shared_ptr< const Condition > & p4,
                          const std::shared_ptr< const Condition > & p5,
                          const std::shared_ptr< const Condition > & p6,
                          const std::shared_ptr< const Condition > & p7,
                          const std::shared_ptr< const Condition > & p8 )
{
    M_condition_set.push_back( p1 );
    M_condition_set.push_back( p2 );
    M_condition_set.push_back( p3 );
    M_condition_set.push_back( p4 );
    M_condition_set.push_back( p5 );
    M_condition_set.push_back( p6 );
    M_condition_set.push_back( p7 );
    M_condition_set.push_back( p8 );
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
OrCondition::operator()( const PlayerAgent * agent ) const
{
    for ( const std::shared_ptr< const Condition > & c : M_condition_set )
    {
        if ( (*c)( agent ) )
        {
            return true;
        }
    }

    return false;
}

}
