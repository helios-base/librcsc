// -*-c++-*-

/*!
  \file formation_parser_json.cpp
  \brief json formation parser class Source File.
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

#include "formation_parser_json.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <cstring>

namespace rcsc {

/*-------------------------------------------------------------------*/
FormationData::Ptr
FormationParserJSON::parse( std::istream & is )
{
    boost::property_tree::ptree json;
    try
    {
        boost::property_tree::read_json( is, json );
    }
    catch ( std::exception & e )
    {
        std::cerr << "(FormationParserJSON::parse) ERROR: read_json. " << e.what() << std::endl;
        return FormationData::Ptr();
    }

    FormationData::Ptr ptr( new FormationData() );

    return ptr;
}

}
