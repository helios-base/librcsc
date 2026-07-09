// -*-c++-*-

/*!
  \file ini_parser.cpp
  \brief INI file parser Source File
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

#include "ini_parser.h"

#include "param_map.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
IniParser::IniParser( const std::string & file_path,
                       const std::string & realm )
    : M_file_path( file_path ),
      M_realm( realm )
{

}

/*-------------------------------------------------------------------*/
/*!

*/
bool
IniParser::parse( ParamMap & param_map )
{
    if ( ! param_map.isValid() )
    {
        std::cerr << __FILE__ << ": ***ERROR*** detected invalid ParamMap "
                  << param_map.groupName() << std::endl;
        return false;
    }

    boost::property_tree::ptree doc;
    try
    {
        boost::property_tree::read_ini( M_file_path, doc );
    }
    catch ( std::exception & e )
    {
        std::cerr << __FILE__ << ": ***ERROR*** Failed to read ini file ["
                  << M_file_path << "]. " << e.what() << std::endl;
        return false;
    }

    // if a realm (section name) is specified, only that section is
    // analyzed. otherwise, the top level (unsectioned) entries are used.
    const boost::property_tree::ptree * section = &doc;
    if ( ! M_realm.empty() )
    {
        const auto it = doc.find( M_realm );
        if ( it == doc.not_found() )
        {
            // the specified realm does not exist in this file.
            // this is not treated as a fatal error.
            return true;
        }

        section = &( it->second );
    }

    std::size_t n_params = 0;
    for ( const auto & entry : *section )
    {
        const std::string & name_str = entry.first;
        const std::string & value_str = entry.second.data();

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
            std::cerr << __FILE__ << ": ***ERROR*** Parse error."
                      << " name=[" << name_str << "]"
                      << " value=[" << value_str << "]"
                      << std::endl;
        }
    }

#ifdef DEBUG
    std::cerr << __FILE__ << ": [" << M_file_path << "] realm=[" << M_realm << "] read "
              << n_params << " params."
              << std::endl;
#endif

    return true;
}

}
