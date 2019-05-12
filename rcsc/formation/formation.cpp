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

//#include "formation_bpn.h"
//#include "formation_cdt.h"
#include "formation_dt.h"
//#include "formation_knn.h"
//#include "formation_ngnet.h"
//#include "formation_rbf.h"
//#include "formation_sbsp.h"
#include "formation_static.h"
//#include "formation_uva.h"

#include <boost/algorithm/string.hpp>

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
    //else if ( name == FormationBPN::NAME ) ptr = FormationBPN::create();
    //else if ( name == FormationCDT::NAME ) ptr = FormationCDT::create();
    else if ( name == FormationDT::NAME ) ptr = FormationDT::create();
    //else if ( name == FormationKNN::NAME ) ptr = FormationKNN::create();
    //else if ( name == FormationNGNet::NAME ) ptr = FormationNGNet::create();
    //else if ( name == FormationRBF::NAME ) ptr = FormationRBF::create();
    //else if ( name == FormationSBSP::NAME ) ptr = FormationSBSP::create();
    else if ( name == FormationStatic::NAME ) ptr = FormationStatic::create();
    //else if ( name == FormationUvA::NAME ) ptr = FormationUvA::create();

    return ptr;
}

/*-------------------------------------------------------------------*/
/*!

*/
Formation::Ptr
Formation::create( std::istream & is )
{
    std::string line_buf;
    std::string method_name;

    while ( std::getline( is, line_buf ) )
    {
        if ( line_buf.empty()
             || line_buf[0] == '#'
             || ! line_buf.compare( 0, 2, "//" ) )
        {
            continue;
        }

        std::vector< std::string > tokens;
        boost::split( tokens, line_buf, boost::is_any_of( " ,") );

        if ( tokens.size() < 2 )
        {
            std::cerr << __FILE__ << ':'
                      << " (create) ERROR Illegal header line. [" << line_buf << ']'
                      << std::endl;
            return Formation::Ptr();
        }

        method_name = tokens[1];

        break;
    }

    is.seekg( 0 );
    return create( method_name );
}


/*-------------------------------------------------------------------*/
/*!

 */
Formation::Formation()
    : M_version( 0 ),
      M_samples( new SampleDataSet() )
{
    for ( int i = 0; i < 11; ++i )
    {
        M_role_type[i] = Unknown_Role;
        M_symmetry_number[i] = -1;
        M_marker[i] = false;
        M_setplay_marker[i] = false;
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
void
Formation::setRoleType( const int unum,
                        const std::string & role_type )
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** invalid unum " << unum
                  << std::endl;
        return;
    }

    if ( role_type == "G" ) M_role_type[unum - 1] = Goalie;
    else if ( role_type == "DF" ) M_role_type[unum - 1] = Defender;
    else if ( role_type == "MF" ) M_role_type[unum - 1] = MidFielder;
    else if ( role_type == "FW" ) M_role_type[unum - 1] = Forward;
    else
    {
        std::cerr << __FILE__ << " (setRoleType)"
                  << " *** ERROR *** unknown role type [" << role_type << "]"
                  << std::endl;
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
void
Formation::setMarker( const int unum,
                      const std::string & marker,
                      const std::string & smarker )
{
    if ( unum < 1 || 11 < unum )
    {
        std::cerr << __FILE__ << ":" << __LINE__
                  << " *** ERROR *** (setMarker) illegal unum " << unum
                  << std::endl;
        return;
    }

    M_marker[unum - 1] = ( marker == "marker" );
    M_setplay_marker[unum - 1] = ( smarker == "setplay_marker" );
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
Formation::updateRoleType( const int unum,
                           const RoleType type )
{
    if ( unum < 1 || 11 < unum )
    {
        return false;
    }

    if ( M_role_type[unum-1] != type )
    {
        M_role_type[unum-1] = type;
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
Formation::updateMarker( const int unum,
                         const bool marker,
                         const bool setplay_marker )
{
    if ( unum < 1 || 11 < unum )
    {
        return false;
    }

    if ( M_marker[unum-1] != marker
         || M_setplay_marker[unum-1] != setplay_marker )
    {
        M_marker[unum-1] = marker;
        M_setplay_marker[unum-1] = setplay_marker;
        return true;
    }

    return false;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
Formation::printComment( std::ostream & os,
                         const std::string & msg ) const
{
    if ( os ) os << "# " << msg << '\n';

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
Formation::readMethodName( std::istream & is )
{
    std::string line_buf;

    while ( std::getline( is, line_buf ) )
    {
        if ( line_buf.empty()
             || line_buf[0] == '#' )
        {
            continue;
        }

        break;
    }

    std::vector< std::string > tokens;
    boost::split( tokens, line_buf, boost::is_any_of( " ," ) );

    if ( tokens.size() < 2 )
    {
        std::cerr << __FILE__ << ':'
                  << " (readMethodName) ERROR Illegal header line. [" << line_buf << ']'
                  << std::endl;
        return false;
    }

    if ( tokens[1] != methodName() )
    {
        std::cerr << __FILE__ << ':'
                  << " (readMethodName) Unsupported formation method "
                  << " [" << tokens[1] << "]" << std::endl;
        return false;
    }

    return true;
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
bool
Formation::readSamplesCSV( std::istream & is )
{
    M_samples = SampleDataSet::Ptr( new SampleDataSet() );

    if ( ! M_samples->readCSV( is ) )
    {
        M_samples.reset();
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
Formation::checkSymmetryNumber() const
{
    // check symmetry number circuration reference
    for ( int i = 0; i < 11; ++i )
    {
        int referred_unum = M_symmetry_number[i];
        if ( referred_unum <= 0 ) continue;
        if ( M_symmetry_number[referred_unum - 1] > 0 )
        {
            return false;
        }
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
Formation::printMethodName( std::ostream & os ) const
{
    os << "Method," << methodName() << '\n';
    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
Formation::printSamplesOld( std::ostream & os ) const
{
    if ( M_samples )
    {
        M_samples->printOld( os );
    }

    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
Formation::printSamplesCSV( std::ostream & os ) const
{
    if ( M_samples )
    {
        M_samples->printCSV( os );
    }

    return os;
}

}
