// -*-c++-*-

/*!
  \file csv_logger.cpp
  \brief CSV logger class Header File
*/

/*
 *Copyright:

 Copyright (C) Hidehisa Akiyama

 This code is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or (at your option)
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

#include "../csv_logger.h"

//#include <boost/interprocess/sync/file_lock.hpp>
//#include <boost/interprocess/sync/scoped_lock.hpp>

#include <iostream>
#include <fstream>

namespace {

/*-------------------------------------------------------------------*/
/*!

*/
std::string
escape_string( const std::string & original )
{
    std::string result;

    for ( std::string::const_iterator it = original.begin(), end = original.end();
          it != end;
          ++it )
    {
        const char ch = *it;
        if ( ch == ',' || ch == '\\' || ch == '\n' )
        {
            result += '\\';
        }

        result += ch;
    }

    return result;
}

}

namespace rcsc {

/*-------------------------------------------------------------------*/
/*!

*/
CSVLogger::CSVLogger( const size_t columns,
                      const std::string & filepath )
    : M_file_path( filepath )
{
    M_header.resize( columns );
    M_data.resize( columns );

    openFile();
}

/*-------------------------------------------------------------------*/
/*!

*/
CSVLogger::CSVLogger( const std::vector< std::string > & header,
                      const std::string & filepath )
    : M_file_path( filepath ),
      M_header( header ),
      M_data( header.size() )
{
    openFile();
}

/*-------------------------------------------------------------------*/
/*!

*/
CSVLogger::~CSVLogger()
{
    if ( M_fout.is_open() )
    {
        M_fout.flush();
        M_fout.close();
    }
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
CSVLogger::openFile()
{
    bool exists_file = existsFile();

    M_fout.open( M_file_path.c_str(), std::ofstream::out | std::ofstream::app );

    if ( ! M_fout.is_open() )
    {
        return false;
    }

    if ( ! exists_file )
    {
        writeHeaderLine();
    }

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
CSVLogger::existsFile()
{
    std::ifstream fin( M_file_path.c_str() );
    if ( ! fin )
    {
        return false;
    }

    std::string header;
    if ( ! std::getline( fin, header ) )
    {
        // empty file
        fin.close();
        return false;
    }

    size_t count = 0;

    std::string::size_type i = 0;
    for ( std::vector< std::string >::const_iterator it = M_header.begin(), end = M_header.end();
          it != end;
          ++it )
    {
        std::string::size_type start = 0;
        std::string name;

        while ( i < header.size()
                && header[i] == ' ' )
        {
            ++i;
        }

        start = i;

        // read quated string
        if ( header[i] == '\"' )
        {
            ++i;
            while ( i < header.size()
                    && header[i] != '\"' )
            {
                ++i;
            }

            if ( i < header.size()
                 && header[i] == '\"' )
            {
                name = header.substr( start, i - start );
                ++i;
            }
        }
        // read raw text
        else
        {
            while ( i < header.size()
                    && header[i] != ',' )
            {
                ++i;
            }
            if ( i < header.size() )
            {
                name = header.substr( start, i - start );
            }
        }

        // skip comma
        while ( i < header.size()
                && ( header[i] != ','
                     || header[i] != ' ' ) )
        {
            ++i;
        }


        // erase space characters
        while ( ! name.empty()
                && name.back() == ' ' )
        {
            name.pop_back();
        }

        if ( name != *it )
        {
            std::cerr << "CSVLogger found mismatched header [" << name "]."
                      << " expected [" << *it << "]"
                      << std::endl;
            return false;
        }

        count += 1;
    }

    if ( count == 0 )
    {
        fin.close();
        return false;
    }

    if ( count != M_header.size() )
    {
        std::cerr << "CSVLogger header size mismatached." << std::endl;
        fin.close();
        return false;
    }

    fin.close();
    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
CSVLogger::isValid() const
{
    return M_fout.is_open()
        && M_header.size() > 0
        && M_header.size() == M_data.size();
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
CSVLogger::commit()
{
    if ( ! isValid() )
    {
        return false;
    }

    const std::vector< std::string >::iterator end = M_data.end();
    std::vector< std::string >::iterator it = M_data.begin();

    M_fout << *it;
    it->clear();
    ++it;

    for ( ; it != end; ++it )
    {
        M_fout << ',' << *it;
        it->clear();
    }
    M_fout << '\n';

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
void
CSVLogger::writeHeaderLine()
{
    if ( ! isValid() )
    {
        std::cerr << "ERROR: CSVLogger"
                  << " file=[" << M_file_path << "]" << std::endl;
        return;
    }

    const std::vector< std::string >::const_iterator end = M_header.end();
    std::vector< std::string >::const_iterator it = M_header.begin();

    M_fout << *it;
    ++it;
    for ( ; it != end; ++it )
    {
        M_fout << ',' << *it;
    }
    M_fout << '\n';
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
CSVLogger::setStringData( const size_t col,
                          const std::string & data )
{
    if ( col >= M_data.size() )
    {
        std::cerr <<  "ERROR: CSVLogger illegal column index "
                  << col << " size=" << M_data.size() << std::endl;
        return false;
    }

    M_data[col] = escape_string( data );

    return true;
}

/*-------------------------------------------------------------------*/
/*!

*/
bool
CSVLogger::setRawTextData( const size_t col,
                           const std::string & data )
{
    if ( col >= M_data.size() )
    {
        std::cerr << "ERROR: CSVLogger illegal column index "
                  << col << " size=" << M_data.size() << std::endl;
        return false;
    }

    M_data[col] = data;

    return true;
}

}
