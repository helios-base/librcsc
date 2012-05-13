// -*-c++-*-

/*!
  \file conf_file_parser.cpp
  \brief config file parser Source File
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

#include "conf_file_parser.h"

#include "param_map.h"

#include <fstream>
#include <sstream>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
ConfFileParser::ConfFileParser( const std::string & file_path,
                                const std::string & delim,
                                const std::string & realm )
    : M_file_path( file_path ),
      M_delimiters( delim ),
      M_realm( realm )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
bool
ConfFileParser::parse( ParamMap & param_map )
{
    if ( ! param_map.isValid() )
    {
        std::cerr << __FILE__ << ": ***ERROR*** detected invalid ParamMap "
                  << param_map.groupName() << std::endl;
        return false;
    }

    std::ifstream fin( M_file_path.c_str() );

    if ( ! fin )
    {
        std::cerr << __FILE__ << ": ***ERROR*** Failed to open config file ["
                  << M_file_path << "]" << std::endl;
        return false;
    }

    int n_line = 0;
    int n_params = 0;
    std::string line_buf;
    while ( std::getline( fin, line_buf ) )
    {
        ++n_line;
        std::string::size_type first_pos = line_buf.find_first_not_of( " \t" );
        if ( first_pos == std::string::npos
             || line_buf[first_pos] == '#'
             || line_buf[first_pos] == ';'
             || ! line_buf.compare( first_pos, 2, "//" ) )
        {
            // comment or empty line
            continue;
        }

        if ( ! M_realm.empty() )
        {
            char realm[256];
            int n_read = 0;
            if ( std::sscanf( line_buf.c_str(), " %255[^ :] :: %n ",
                              realm, &n_read ) != 1
                 || n_read == 0 )
            {
                // illegal relm format
                //std:: cerr << "***WARNING*** ConfFileParser "
                //           << " Illegal realm format ["
                //           << line_buf << "]" << std::endl;
                continue;
            }

            if ( M_realm != realm )
            {
                // namespace does not match.
                continue;
            }

            line_buf.erase( 0, n_read );
        }

        // search delimiter charecters
        std::string::size_type delim_pos = line_buf.find_first_of( M_delimiters );
        if ( delim_pos != std::string::npos )
        {
            line_buf.replace( delim_pos, 1, 1, ' ' );
        }

        // get name and value string
        std::istringstream istrm( line_buf );
        std::string name_str, value_str;
        istrm >> name_str >> value_str;

        if ( name_str.empty() )
        {
            continue;
        }

        // get parameter entry from map
        ParamEntity::Ptr param_ptr = param_map.findLongName( name_str );

        if ( ! param_ptr )
        {
            continue;
        }

        // analyze value string
        if ( param_ptr->analyze( value_str ) )
        {
            ++n_params;
        }
        else
        {
            std::cerr << __FILE__ << ": ***ERROR*** Parse error at line " << n_line
                      << " name=[" << name_str << "]"
                      << " value=[" << value_str << "]"
                      << std::endl;
        }
    }

    if ( ! fin.eof() )
    {
        std::cerr << __FILE__ << ": ***ERROR*** Parser did not reach the end of file."
                  << " [" << M_file_path << "]"
                  << std::endl;
    }

    fin.close();
#ifdef DEBUG
    std::cerr << __FILE__": [" << M_file_path << "] read " << n_line << " lines. "
              << n_params << " params."
              << std::endl;
#endif
    return true;
}

}
