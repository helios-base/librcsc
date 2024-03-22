// -*-c++-*-

/*!
  \file csv_logger.h
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

#ifndef CSV_LOGGER_H
#define CSV_LOGGER_H

#include <boost/lexical_cast.hpp>

#include <vector>
#include <string>
#include <fstream>

namespace rcsc {

/*!
  \class CSVLogger
  \brief logging data as a CSV file with fixed column size
*/
class CSVLogger {
private:

    //! file path to store the data
    const std::string M_file_path;

    std::ofstream M_fout;

    std::vector< std::string > M_header;
    std::vector< std::string > M_data;

    // not used
    CSVLogger();
    CSVLogger( const CSVLogger & );
    CSVLogger & operator=( const CSVLogger & );

public:

    /*!
      \brief constructor
      \param columns column size
      \param filepath file path to store the data
     */
    CSVLogger( const size_t columns,
               const std::string & filepath );

    /*!
      \brief constructor
      \param header column header names
      \param filepath file path to store the data
     */
    CSVLogger( const std::vector< std::string > & header,
               const std::string & filepath );

    /*
      \brief destructor
     */
    ~CSVLogger();

    /*!
      \brief get the file path string
      \return file path string
     */
    const std::string & filePath() const
      {
          return M_file_path;
      }

    /*!
      \brief check if the output stream is valid or not
      \return checked status
     */
    bool isValid() const;

    /*
      \brief set data to specified column
      \param col column index
      \param data data value
     */
    template < typename T >
    bool setData( const size_t col,
                  const T & data )
      {
          return setRawTextData( col, boost::lexical_cast< std::string >( data ) );
      }

    /*
      \brief set data to specified column (specialized version)
      \param col column index
      \param data data value
     */
    bool setData( const size_t col,
                  const std::string & data )
      {
          return setStringData( col, data );
      }

    /*
      \brief set data to specified column (specialized version)
      \param col column index
      \param data data value
     */
    bool setData( const size_t col,
                  const char * data )
      {
          return setStringData( col, data );
      }


    /*
      \brief write data to the file and reset cached data.
      \return true if success, otherwise false
     */
    bool commit();

private:

    /*!
      \brief create a new file or open the exisiting one
      \return true if successfully created or opened, otherwise false
     */
    bool openFile();

    /*!
      \brief check if the csv files already exists or not.
      \return true if file and same header names are exists.
     */
    bool existsFile();

    /*
      \brief write header line
     */
    void writeHeaderLine();

    /*!
      \brief set string data. the data is escaped.
      \param col column index
      \param data string data
     */
    bool setStringData( const size_t col,
                        const std::string & data );

    /*!
      \brief set raw string data. the data is not escaped.
      \param col column index
      \param data string data
     */
    bool setRawTextData( const size_t col,
                         const std::string & data );

};

}

#endif
