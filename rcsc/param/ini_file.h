// -*-c++-*-

/*!
  \file ini_file.h
  \brief ini format file class Header File
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

#ifndef RCSC_PARAM_INI_FILE_H
#define RCSC_PARAM_INI_FILE_H

#include <rcsc/param/param_map.h>

#include <string>
#include <map>

namespace rcsc {

/*!
  \class IniFile
  \brief ini format file
 */
class IniFile {
private:
    //! file path string
    const std::string M_file_path;

    //! current group name. '/' is used as delimiter.
    std::string M_group_name;

    //! all parameter map. key: group name, value: parameter map.
    std::map< std::string, std::map< std::string, std::string > > M_parameters;

    // non copyable
    IniFile( const IniFile & );
    IniFile & operator=( const IniFile & );
public:

    /*!
      \brief construct with file path and delimiters
      \param file_path file path string of the opened ini file.
     */
    explicit
    IniFile( const std::string & file_path );

    /*!
      \brief write all data entries to file.
     */
    bool sync();

    /*!
      \brief remove all entries.
     */
    void clear();

    /*!
      \brief push new group to the group stack.
      \param group new group name.
     */
    void beginGroup( const std::string & group );

    /*!
      \brief pop group from the group stack.
     */
    void endGroup();

    /*!
      \brief get the current group name.
      \return current group name.
     */
    std::string groupName() const;

    /*!
      \brief set new parameter entry
      \param param new parameter
     */
    void setValue( const ParamEntity & param );

    /*!
      \brief read parameter.
      \param param result variable. this variable has to contain parameter name.
      \return if value is read, return true. otherwise false.
     */
    bool getValue( ParamEntity & param ) const;

    /*!
      \brief remove the specified parameter entry.
      \param name parameter name.
      \return true if successfully removed, otherwise false.
     */
    bool remove( const std::string & name );

};

}

#endif
