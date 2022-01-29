// -*-c++-*-

/*!
  \file clang_unum.h
  \brief clang unum_set class Header File
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

#ifndef RCSC_CLANG_UNUM_H
#define RCSC_CLANG_UNUM_H

#include <memory>
#include <set>
#include <iostream>

namespace rcsc {

/*!
  \class CLangUnumSet
  \brief set of uniform number
 */
class CLangUnumSet {
public:

    //! smart pointer type
    typedef std::shared_ptr< CLangUnumSet > Ptr;

    //! set container type
    typedef std::set< int > Set;

    static const int All;

private:

    //! uniform number set
    Set M_entries;

public:

    /*!
      \brief create empty uniform number set.
     */
    CLangUnumSet()
      { }

    /*!
      \brief create with uniform number set.
      \param unum_set uniform number set.
     */
    explicit
    CLangUnumSet( const Set & unum_set )
        : M_entries( unum_set )
      { }

    /*!
      \brief create with an uniform number.
      \param unum uniform number entry.
     */
    explicit
    CLangUnumSet( const int unum )
      {
          M_entries.insert( unum );
      }

    // ~CLangUnumSet()
    //   {
    //       std::cerr << "delete CLangUnumSet " << *this << std::endl;
    //   }

    /*!
      \brief get entry numbers
      \return the set of uniform numbers
     */
    const Set & entries() const
      {
          return M_entries;
      }

    /*!
      \brief add new entry
      \param unum added uniform number
     */
    void add( const int unum )
      {
          M_entries.insert( unum );
      }

    /*!
      \brief check if the set contains '0' that indicates all uniform number.
      \return checked result
     */
    bool isAll() const
      {
          return M_entries.find( All ) != M_entries.end();
      }

    /*!
      \brief check if the set contains the uniform number.
      \param unum checked uniform number
      \return checked result
     */
    bool contains( const int unum ) const
      {
          return M_entries.find( unum ) != M_entries.end();
      }

    /*!
      \brief output clang format message
      \param os reference to the output stream
      \return os reference to the output stream
     */
    std::ostream & print( std::ostream & os ) const;

};

}

/*!
  \brief stream operator.
  \param os reference to the output stream.
  \param unum unum set object.
  \return reference to the output stream.
 */
inline
std::ostream &
operator<<( std::ostream & os,
            const rcsc::CLangUnumSet & unums )
{
    return unums.print( os );
}

#endif
