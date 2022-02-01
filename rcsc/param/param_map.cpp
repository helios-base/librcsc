// -*-c++-*-

/*!
  \file param_map.cpp
  \brief parameter registry map Source File
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

#include "param_map.h"

#include <functional>

namespace rcsc {

namespace {

/*-------------------------------------------------------------------*/
/*!

 */
inline
bool
is_true( const std::string & value_str )
{
    return ( value_str == "true"
             || value_str == "on"
             || value_str == "1"
             || value_str == "yes" );
}

/*-------------------------------------------------------------------*/
/*!

 */
inline
bool
is_false( const std::string & value_str )
{
    return ( value_str == "false"
             || value_str == "off"
             || value_str == "0"
             || value_str == "no" );
}

}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
ParamEntity::printFormat( std::ostream & os ) const
{
    os << "--" << longName();
    if ( ! shortName().empty() )
    {
        os << " [ -" << shortName() << " ]";
    }

    os << " <Value>";
    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
ParamGeneric< bool >::analyze( const std::string & value_str )
{
    if ( value_str.empty() )
    {
        return false;
    }

    if ( is_true( value_str ) )
    {
        *M_value_ptr = true;
    }
    else if ( is_false( value_str ) )
    {
        *M_value_ptr = false;
    }
    else
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " ***ERROR*** Unexpected value string: type bool. ["
                  << value_str << "]"
                  << std::endl;
        return false;
    }

    if ( M_negate )
    {
        *M_value_ptr = ! *M_value_ptr;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
ParamGeneric< bool >::printValue( std::ostream & os ) const
{
    if ( M_negate )
    {
        os << std::boolalpha << ! *M_value_ptr;
    }
    else
    {
        os << std::boolalpha << *M_value_ptr;
    }
    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
ParamSwitch::analyze( const std::string & value_str )
{
    if ( value_str.empty()
         || is_true( value_str ) )
    {
        *M_value_ptr = true;
    }
    else if ( is_false( value_str ) )
    {
        *M_value_ptr = false;
    }
    else
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " ***ERROR*** Unexpected value string: type switch. ["
                  << value_str << "]"
                  << std::endl;
        return false;
    }

    if ( M_negate )
    {
        *M_value_ptr = ! *M_value_ptr;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
ParamSwitch::printFormat( std::ostream & os ) const
{
    os << "--" << longName();
    if ( ! shortName().empty() )
    {
        os << " [ -" << shortName() << " ]";
    }

    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
ParamSwitch::printValue( std::ostream & os ) const
{
    if ( M_negate )
    {
        os << ( *M_value_ptr ? "off" : "on" );
    }
    else
    {
        os << ( *M_value_ptr ? "on" : "off" );
    }
    return os;
}

/*-------------------------------------------------------------------*/
/*!

 */
ParamMap::Registrar &
ParamMap::Registrar::operator()( const std::string & long_name,
                                 const std::string & short_name,
                                 const NegateBool & value,
                                 const char * description )
{
    if ( ! checkName( long_name, short_name ) )
    {
        M_param_map.M_valid = false;
        return *this;
    }

    if ( ! value.ptr_ )
    {
        std::cerr << "***ERROR*** detected null pointer for the option "
                  << long_name << std::endl;
        M_param_map.M_valid = false;
        return *this;
    }

    ParamEntity::Ptr ptr( new ParamGeneric< bool >( long_name,
                                                    short_name,
                                                    value,
                                                    description ) );

    M_param_map.add( ptr );

    return *this;
}


/*-------------------------------------------------------------------*/
/*!

 */
ParamMap::Registrar &
ParamMap::Registrar::operator()( const std::string & long_name,
                                 const std::string & short_name,
                                 const BoolSwitch & value,
                                 const char * description )
{
    if ( ! checkName( long_name, short_name ) )
    {
        M_param_map.M_valid = false;
        return *this;
    }

    if ( ! value.ptr_ )
    {
        std::cerr << "***ERROR*** detected null pointer for the option "
                  << long_name << std::endl;
        M_param_map.M_valid = false;
        return *this;
    }

    ParamEntity::Ptr ptr( new ParamSwitch( long_name,
                                           short_name,
                                           value.ptr_,
                                           description ) );
    M_param_map.add( ptr );

    return *this;
}

/*-------------------------------------------------------------------*/
/*!

 */
ParamMap::Registrar &
ParamMap::Registrar::operator()( const std::string & long_name,
                                 const std::string & short_name,
                                 const NegateSwitch & value,
                                 const char * description )
{
    if ( ! checkName( long_name, short_name ) )
    {
        M_param_map.M_valid = false;
        return *this;
    }

    if ( ! value.ptr_ )
    {
        std::cerr << "***ERROR*** detected null pointer for the option "
                  << long_name << std::endl;
        M_param_map.M_valid = false;
        return *this;
    }

    ParamEntity::Ptr ptr( new ParamSwitch( long_name,
                                           short_name,
                                           value,
                                           description ) );
    M_param_map.add( ptr );

    return *this;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
ParamMap::Registrar::checkName( const std::string & long_name,
                                const std::string & short_name ) const
{
    if ( long_name.empty() )
    {
        std::cerr << "***ERROR*** parameter long_name is empty."
                  << std::endl;
        return false;
    }

    if ( long_name.length() >= 40 )
    {
        std::cerr << "***ERROR*** parameter long_name have to be less than 40. ["
                  << long_name << ']' << std::endl;
        return false;
    }

    if ( short_name.length() >= 8 )
    {
        std::cerr << "***ERROR*** parameter short_name have to be less than 8. ["
                  << short_name << ']' << std::endl;
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
// ParamMap &
// ParamMap::add( ParamMap & param_map )
// {
//     if ( this == &param_map )
//     {
//         return *this;
//     }
//
//     for ( std::vector< ParamEntity::Ptr >::iterator it = param_map.M_parameters.begin();
//           it != param_map.M_parameters.end();
//           ++it )
//     {
//         add( *it );
//     }
//
//     return *this;
// }

/*-------------------------------------------------------------------*/
/*!

 */
ParamMap::Registrar &
ParamMap::add( ParamEntity::Ptr param )
{
    if ( ! param )
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " ***ERROR*** ParamMap::add(). "
                  << "detected null ParamEntity::Ptr."
                  << std::endl;
        return M_registrar;
    }

    if ( param->longName().empty() )
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " ***ERROR*** ParamMap::add(). "
                  << "Empty parameter name! parameter was not registered."
                  << std::endl;
        return M_registrar;
    }

    if ( param->longName().find( '=' ) != std::string::npos
         || param->shortName().find( '=' ) != std::string::npos )
    {
        std::cerr << " ***ERROR*** "
                  << " the option name [" << param->longName()
                  << "] or [" << param->shortName()
                  << "] contains an illegal character '='."
                  << std::endl;
        return M_registrar;
    }

    if ( param->longName() == "-"
         || param->shortName() == "-" )
    {
        std::cerr << " ***ERROR*** "
                  << " '-' cannot be used as the option name."
                  << std::endl;
        return M_registrar;
    }

    if ( M_long_name_map.find( param->longName() ) != M_long_name_map.end()
         || M_short_name_map.find( param->shortName() ) != M_short_name_map.end() )
    {
        std::cerr << " ***ERROR*** "
                  << " the option name [" << param->longName()
                  << "] or [" << param->shortName()
                  << "] has already been registered."
                  << std::endl;
        return M_registrar;
    }

    M_parameters.push_back( param );

    M_long_name_map[ param->longName() ] = param;

    if ( ! param->shortName().empty() )
    {
        M_short_name_map[ param->shortName() ] = param;
    }

    return M_registrar;
}

/*-------------------------------------------------------------------*/
/*!

 */
void
ParamMap::remove( const std::string & long_name )
{
    M_parameters.erase( std::remove_if( M_parameters.begin(),
                                        M_parameters.end(),
                                        [&]( const ParamEntity::Ptr & v )
                                          {
                                              return v->longName() == long_name;
                                          } ),
                        M_parameters.end() );

    std::map< std::string, ParamEntity::Ptr >::iterator it_long = M_long_name_map.find( long_name );
    if ( it_long != M_long_name_map.end() )
    {
        if ( ! it_long->second->shortName().empty() )
        {
            std::map< std::string, ParamEntity::Ptr >::iterator it_short = M_short_name_map.find( it_long->second->shortName() );
            M_short_name_map.erase( it_short );
        }

        M_long_name_map.erase( it_long );
    }
}

/*-------------------------------------------------------------------*/
/*!

 */
ParamEntity::Ptr
ParamMap::findLongName( const std::string & long_name )
{
    std::map< std::string, ParamEntity::Ptr >::iterator it = M_long_name_map.find( long_name );

    if ( it != M_long_name_map.end() )
    {
        return it->second;
    }

    return ParamEntity::Ptr();
}

/*-------------------------------------------------------------------*/
/*!

 */
ParamEntity::Ptr
ParamMap::findShortName( const std::string & short_name )
{
    std::map< std::string, ParamEntity::Ptr >::iterator it = M_short_name_map.find( short_name );

    if ( it != M_short_name_map.end() )
    {
        return it->second;
    }

    return ParamEntity::Ptr();
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
ParamMap::printHelp( std::ostream & os,
                     const bool with_default ) const
{
    //const std::string indent_nl = "\n      ";

    os << '\n';

    if ( ! M_group_name.empty() )
    {
        os << ' ' << M_group_name << ":\n";
    }

    size_t width = 22;

    for ( const ParamEntity::Ptr & p : M_parameters )
    {
        std::ostringstream ostr;
        ostr << "  ";
        p->printFormat( ostr );

        width = std::max( width, ostr.str().length() );
    }

    ++width;

    const std::size_t max_len = 80;
    const std::string indent( width, ' ' );

    for ( const ParamEntity::Ptr & param : M_parameters )
    {
        std::ostringstream ostr;
        ostr << "  ";
        param->printFormat( ostr );
        os << ostr.str();

        const size_t padding = width - ostr.str().length();

        const std::string & desc = param->description();
        if ( ! desc.empty() )
        {
            for ( size_t p = padding; p > 0; --p )
            {
                os.put( ' ' );
            }

            if ( padding > max_len - 6 )
            {
                os << '\n';
            }

            // format description message
            std::string::size_type nl_pos = 0;
            for ( std::string::size_type pos = desc.find( ' ' );
                  pos != std::string::npos;
                  pos = desc.find( ' ', pos + 1 ) )
            {
                if ( pos - nl_pos + width > max_len - 6 )
                {
                    os << desc.substr( nl_pos, pos - nl_pos )
                       << '\n' << indent;
                    nl_pos = pos + 1;
                }
            }
            os << desc.substr( nl_pos );
        }

        if ( with_default )
        {
            if ( ! desc.empty() )
            {
                os << '\n' << indent;
            }
            else
            {
                for ( size_t p = padding; p > 0; --p )
                {
                    os.put( ' ' );
                }
            }

            if ( param->isSwitch() )
            {
                os << "(Switch Default: ";
            }
            else
            {
                os << "(Default: ";
            }

            param->printValue( os ) << ')';
        }

        os << '\n';
    }

    return os << std::flush;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
ParamMap::printValues( std::ostream & os ) const
{
    for ( const ParamEntity::Ptr & param : M_parameters )
    {
        os << param->longName() << '\t';
        param->printValue( os );
        os << '\n';
    }
    return os << std::flush;
}

}
