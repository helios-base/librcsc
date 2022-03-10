// -*-c++-*-

/*!
  \file formation_writer_json.h
  \brief JSON style formation data writer Header File.
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

#ifndef RCSC_FORMATION_FORMATION_WRITER_JSON_H
#define RCSC_FORMATION_FORMATION_WRITER_JSON_H

#include <rcsc/formation/formation_writer.h>

namespace rcsc {

/*!
  \class FormationWriterJSON
  \brief JSON style formation data writer
*/
class FormationWriterJSON
    : public FormationWriter {
private:

    FormationWriterJSON( FormationWriterJSON & ) = delete;
    FormationWriterJSON & operator=( FormationWriterJSON & ) = delete;

protected:

    /*!
      \brief default constructor
     */
    FormationWriterJSON() = default;

public:

    /*!
      \brief virtual default destructor
     */
    virtual
    ~FormationWriterJSON() = default;

    /*!
      \brief write to the output stream
      \param os reference to the output stream
      \param data pointer to the written data
      \return true if success
     */
    virtual
    bool print( std::ostream & os,
                FormationData::ConstPtr & data ) const override;

};

}

#endif
