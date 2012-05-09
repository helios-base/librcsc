// -*-c++-*-

/*!
  \file formation.cpp
  \brief formation data classes Source File.
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

#include "formation.h"

#include "formation_bpn.h"
#include "formation_cdt.h"
#include "formation_dt.h"
#include "formation_knn.h"
#include "formation_ngnet.h"
#include "formation_rbf.h"
#include "formation_sbsp.h"
#include "formation_static.h"
#include "formation_uva.h"

#include <sstream>

namespace rcsc {

using namespace formation;

/*-------------------------------------------------------------------*/
/*!

*/
Formation::Creators &
Formation::creators()
{
    static Creators s_instance;
    return s_instance;
}

/*-------------------------------------------------------------------*/
/*!

*/
Formation::Ptr
Formation::create( const std::string & name )
{
    Formation::Ptr ptr( static_cast< Formation * >( 0 ) );

    Formation::Creator creator;
    if ( Formation::creators().getCreator( creator, name ) )
    {
        ptr = creator();
    }
    else if ( name == FormationBPN::NAME ) ptr = FormationBPN::create();
    else if ( name == FormationCDT::NAME ) ptr = FormationCDT::create();
    else if ( name == FormationDT::NAME ) ptr = FormationDT::create();
    else if ( name == FormationKNN::NAME ) ptr = FormationKNN::create();
    else if ( name == FormationNGNet::NAME ) ptr = FormationNGNet::create();
    else if ( name == FormationRBF::NAME ) ptr = FormationRBF::create();
    else if ( name == FormationSBSP::NAME ) ptr = FormationSBSP::create();
    else if ( name == FormationStatic::NAME ) ptr = FormationStatic::create();
    else if ( name == FormationUvA::NAME ) ptr = FormationUvA::create();

    return ptr;
}

/*-------------------------------------------------------------------*/
/*!

*/
Formation::Ptr
Formation::create( std::istream & is )
{
    std::string temp, type;
    is >> temp >> type;

    is.seekg( 0 );
    return create( type );
}


/*-------------------------------------------------------------------*/
/*!

 */
Formation::Formation()
    : M_version( 0 )
    , M_samples( new SampleDataSet() )
{
    for ( int i = 0; i < 11; ++i )
    {
        M_symmetry_number[i] = -1;
    }
}


/*-------------------------------------------------------------------*/
/*!

 */
void
Formation::setCenterType( const int unum )
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** invalid unum " << unum
                  << std::endl;
        return;
    }

    M_symmetry_number[unum - 1] = 0;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Formation::setSideType( const int unum )
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** invalid unum " << unum
                  << std::endl;
        return;
    }

    M_symmetry_number[unum - 1] = -1;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
Formation::setSymmetryType( const int unum,
                            const int symmetry_unum,
                            const std::string & role_name )
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** Invalid unum " << unum
                  << std::endl;
        return false;
    }
    if ( symmetry_unum < 1 || 11 < symmetry_unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** Invalid symmetry unum " << unum
                  << std::endl;
        return false;
    }
    if ( symmetry_unum == unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** Never symmetry itself. unum=" << unum
                  << "  symmetry=" << symmetry_unum
                  << std::endl;
        return false;
    }
    if ( M_symmetry_number[symmetry_unum - 1] > 0 )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** " << symmetry_unum << " is already a symmetrical player. "
                  << std::endl;
        return false;
    }
    if ( M_symmetry_number[symmetry_unum - 1] == 0 )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** " << symmetry_unum << " is a center type player. "
                  << std::endl;
        return false;
    }


    // check if unum is already assigned as original side type player.
    for ( int i = 0; i < 11; ++i )
    {
        if ( i + 1 == unum ) continue;
        if ( M_symmetry_number[i] == symmetry_unum )
        {
            // refered by other player.
            std::cerr << __FILE__ << ":" << __LINE__
                      << " *** ERROR *** player " << unum
                      << " has already refered by player " << i + 1
                      << std::endl;
            return false;
        }
    }

    M_symmetry_number[unum - 1] = symmetry_unum;

    if ( role_name.empty() )
    {
        setRoleName( unum, getRoleName( symmetry_unum ) );
    }
    else
    {
        setRoleName( unum, role_name );
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
Formation::updateRole( const int unum,
                       const int symmetry_unum,
                       const std::string & role_name )
{
    if ( getSymmetryNumber( unum ) != symmetry_unum )
    {
        if ( symmetry_unum == 0 )
        {
            createNewRole( unum, role_name, Formation::CENTER );
            return true;
        }

        if ( symmetry_unum < 0 )
        {
            createNewRole( unum, role_name, Formation::SIDE );
            return true;
        }

        // ( symmetry_unum > 0 )

        if ( ! isSideType( symmetry_unum ) )
        {
            std::cerr << __FILE__ << ":" << __LINE__
                      << " You cannot use the player number " << symmetry_unum
                      << " as a symmetry number."
                      << std::endl;
            return false;
        }

        for ( int i = 1; i <= 11; ++i )
        {
            if ( i == unum || i == symmetry_unum ) continue;
            if ( getSymmetryNumber( i ) == symmetry_unum )
            {
                std::cerr << __FILE__ << ":" << __LINE__
                          << " player number " << symmetry_unum
                          << " has already refered by player " << i
                          << std::endl;
                return false;
            }
        }

        setSymmetryType( unum, symmetry_unum, role_name );
        return true;
    }

    if ( ! role_name.empty()
         && getRoleName( unum ) != role_name )
    {
        setRoleName( unum, role_name );
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
Formation::read( std::istream & is )
{
    if ( ! readHeader( is ) ) return false;
    if ( ! readConf( is ) ) return false;
    if ( ! readSamples( is ) ) return false;

    // check symmetry number circuration reference
    for ( int i = 0; i < 11; ++i )
    {
        int refered_unum = M_symmetry_number[i];
        if ( refered_unum <= 0 ) continue;
        if ( M_symmetry_number[refered_unum - 1] > 0 )
        {
            std::cerr << __FILE__ << ":" << __LINE__
                      << " *** ERROR *** failed to read formation."
                      << " Bad symmetrying. player "
                      << i + 1
                      << " mirro = " << refered_unum
                      << " is already symmetrying player"
                      << std::endl;
            return false;
        }
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
Formation::print( std::ostream & os ) const
{
    if ( os ) printHeader( os );
    if ( os ) printConf( os );
    if ( os ) printSamples( os );

    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
Formation::readHeader( std::istream & is )
{
    std::string line_buf;

    while ( std::getline( is, line_buf ) )
    {
        if ( line_buf.empty()
             || line_buf[0] == '#'
             || ! line_buf.compare( 0, 2, "//" ) )
        {
            continue;
        }

        std::istringstream istr( line_buf );

        std::string tag;
        istr >> tag;
        if ( tag != "Formation" )
        {
            std::cerr << __FILE__ << ":" << __LINE__
                      << " *** ERROR *** Found invalid tag ["
                      << tag << "]"
                      << std::endl;
            return false;
        }

        std::string name;
        istr >> name;
        if ( name != methodName() )
        {
            std::cerr << __FILE__ << ":" << __LINE__
                      << " *** ERROR *** Unsupported formation type name "
                      << " [" << name << "]."
                      << " The name has to be " << methodName()
                      << std::endl;
            return false;
        }

        int ver = 0;
        if ( istr >> ver )
        {
            if ( ver < 0 )
            {
                std::cerr << __FILE__ << ":" << __LINE__
                          << " *** ERROR *** Illegas format version "
                          << ver
                          << std::endl;
                return false;
            }

            M_version = ver;
        }
        else
        {
            M_version = 0;
        }

        return true;
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
Formation::readSamples( std::istream & is )
{
    M_samples = SampleDataSet::Ptr( new SampleDataSet() );

    if ( ! M_samples->read( is ) )
    {
        M_samples.reset();
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
Formation::printHeader( std::ostream & os ) const
{
    os << "Formation " << methodName() << ' ' << version() << '\n';
    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
Formation::printSamples( std::ostream & os ) const
{
    if ( M_samples )
    {
        M_samples->print( os );
    }

    return os;
}

}
