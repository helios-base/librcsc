// -*-c++-*-

/*!
  \file rcss_param_parser.cpp
  \brief rcssserver parameter message parser Source File
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

#include "rcss_param_parser.h"

#include "param_map.h"

#include <iostream>
#include <cstdio>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
RCSSParamParser::RCSSParamParser( const char * msg )
{
    if ( ! init( msg ) )
    {
        M_str_pairs.clear();
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
RCSSParamParser::parse( ParamMap & param_map )
{
    if ( ! param_map.isValid() )
    {
        std::cerr << __FILE__ << ": ***ERROR*** detected invalid ParamMap "
                  << param_map.groupName() << std::endl;
        return false;
    }

    if ( M_str_pairs.empty() )
    {
        return false;
    }

    int n_params = 0;
    for ( StrPairVec::iterator it = M_str_pairs.begin();
          it != M_str_pairs.end();
          ++it )
    {
        // get parameter entry from map
        ParamEntity::Ptr param_ptr = param_map.findLongName( it->first );

        // analyze value string
        if ( param_ptr
             && param_ptr->analyze( it->second ) )
        {
            ++n_params;
        }
        else
        {
            std::cerr << __FILE__ << ": ***ERROR*** unknown parameter name or invalid value."
                      << " name=[" << it->first << "]"
                      << " value=[" << it->second << "]"
                      << std::endl;
        }
    }

#ifdef DEBUG
    std::cerr << __FILE__ << ": [" << M_param_name << "] read " << n_params << " params."
              << std::endl;
#endif
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
std::string
RCSSParamParser::cleanString( std::string str )
{
    if( str.empty() )
    {
				return str;
    }

    if ( *str.begin() == '\'' )
    {
				if( *str.rbegin() == '\''  )
        {
            str = str.substr( 1, str.length() - 2 );
        }
				else
        {
            return str;
        }

        // replace escape character
        for ( std::string::size_type escape_pos = str.find( "\\'" );
              escape_pos != std::string::npos;
              escape_pos = str.find( "\\'" ) )
				{
            // replace "\'" with "'"
            str.replace( escape_pos, 2, "'" );
				}
    }
    else if ( *str.begin() == '"' )
    {
				if ( *str.rbegin() == '"'  )
        {
            str = str.substr( 1, str.length() - 2 );
        }
				else
        {
            return str;
        }

        // replace escape character
        for ( std::string::size_type escape_pos = str.find( "\\\"" );
              escape_pos != std::string::npos;
              escape_pos = str.find( "\\\"" ) )
				{
            // replace "\"" with """
            str.replace( escape_pos, 2, "\"" );
				}
    }

    return str;
}


/*-------------------------------------------------------------------*/
/*!

*/
bool
RCSSParamParser::init( const char * msg )
{
    char buf[32];
    int n_read = 0;
    if ( std::sscanf( msg, " ( %31s %n", buf, &n_read ) != 1 )
    {
        std::cerr << __FILE__ << ": ***ERROR*** "
                  << "Failed to parse parameter type name. " << msg << std::endl;
        return false;
    }

    M_param_name = buf;

    const std::string msg_str( msg );

    for ( std::string::size_type pos = msg_str.find_first_of( '(', n_read );
          pos != std::string::npos;
          pos = msg_str.find_first_of( '(', pos ) )
    {
        std::string::size_type end_pos = msg_str.find_first_of( ' ', pos );
        if ( end_pos == std::string::npos )
        {
            std::cerr << __FILE__ << ": ***ERROR*** "
                      << "Failed to parse parameter name. " << msg << std::endl;
            return false;
        }

        pos += 1;
        const std::string name_str( msg_str, pos, end_pos - pos );

        pos = end_pos;
        // search end paren or double quatation
        end_pos = msg_str.find_first_of( ")\"", end_pos ); //"
        if ( end_pos == std::string::npos )
        {
            std::cerr << __FILE__ << ": ***ERROR*** "
                      << "Failed to parse parameter value for [" << name_str << "] in "
                      << msg << std::endl;
            return false;
        }

        // found string type value
        if ( msg_str[end_pos] == '\"' ) // "
        {
            pos = end_pos;
            end_pos = msg_str.find_first_of( '\"', end_pos + 1 ); //"
            if ( end_pos == std::string::npos )
            {
                std::cerr << __FILE__ << ": ***ERROR*** "
                          << "Failed to parse string parameter value for [" << name_str << "] in "
                          << msg << std::endl;
                return false;
            }
            end_pos += 1; // skip double quatation
        }
        else
        {
            pos += 1; // skip white space
        }

        std::string value_str( msg_str, pos, end_pos - pos );
        value_str = cleanString( value_str );

        M_str_pairs.push_back( std::make_pair( name_str, value_str ) );

        pos = end_pos;
    }

    return true;
}

}
