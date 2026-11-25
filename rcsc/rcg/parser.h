// -*-c++-*-

/*!
  \file parser.h
  \brief abstract rcg parser class Header File.
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

#ifndef RCSC_RCG_PARSER_H
#define RCSC_RCG_PARSER_H

#include <rcsc/factory.h>

#include <cstddef>
#include <memory>
#include <istream>
#include <string>

namespace rcsc {
namespace rcg {

class Handler;

/////////////////////////////////////////////////////////////////////

/*!
  \class Parser
  \brief rcg stream parser interface class
*/
class Parser {
public:

    using Ptr = std::shared_ptr< Parser >; //!< rcg parser pointer type
    using Creator = Ptr(*)(); //!< rcg parser creator function
    using Creators = rcss::Factory< Creator, int >; //!< creator function holder

    /*!
      \brief factory holder singleton
      \return reference to the factory holder instance
     */
    static
    Creators & creators();

    /*!
      \brief create a suitable version parser instance depending on the input stream.
      \param is reference to the imput stream.
      \return smart pointer to the rcg parser instance

      poionted index of istream becomes 4.
     */
    static
    Ptr create( std::istream & is );

    /*!
      \brief create a suitable version parser instance depending on the data in memory.
      \param data the whole game log in memory (e.g. read by rcsc::gzread_file())
      \param size the size of data
      \return smart pointer to the rcg parser instance
     */
    static
    Ptr create( const char * data,
                const std::size_t size );

protected:

    /*!
      \brief constructor is accessible only from the derived classes.
     */
    Parser() = default;

public:

    /*!
      \brief virtual destructor
    */
    virtual
    ~Parser()
      { }

    /*!
      \brief (pure virtual) get log version
      \return version number
     */
    virtual
    int version() const = 0;

    /*!
      \brief (pure virtual) analyze log data from input stream
      \param is reference to the imput stream (usually ifstream/gzifstream).
      \param handler reference to the rcg data handler.
      \retval true, if successfuly parsed.
      \retval false, if incorrect format is detected.
     */
    virtual
    bool parse( std::istream & is,
                Handler & handler ) const = 0;

    /*!
      \brief analyze the game log file. The file is read into memory by rcsc::gzread_file(),
      so a gzip file can be read, and parsed by parse( const char *, std::size_t, Handler & ).
      \param filepath the path of the game log file
      \param handler reference to the rcg data handler.
      \retval true, if successfuly parsed.
      \retval false, if the file cannot be read or incorrect format is detected.
     */
    virtual
    bool parse( const std::string & filepath,
                Handler & handler ) const;

    /*!
      \brief analyze the game log data in memory. This is faster than parse( std::istream &, Handler & ).
      The default implementation calls parse( std::istream &, Handler & ) with a stream over the data.
      \param data the whole game log in memory, including the header (e.g. read by rcsc::gzread_file())
      \param size the size of data
      \param handler reference to the rcg data handler.
      \retval true, if successfuly parsed.
      \retval false, if incorrect format is detected.
     */
    virtual
    bool parse( const char * data,
                const std::size_t size,
                Handler & handler ) const;
};

} // end of namespace
} // end of namespace

#endif
