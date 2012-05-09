// -*-c++-*-

/*!
  \file view_mode.h
  \brief player view mode data classes Header File
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

#ifndef RCSC_PLAYER_VIEW_MODE_H
#define RCSC_PLAYER_VIEW_MODE_H

#include <string>

namespace rcsc {

/*!
  \class ViewWidth
  \brief view width data class
*/
class ViewWidth {
public:
    /*!
      \brief types of view width
    */
    enum Type {
        NARROW = 60, //45,
        NORMAL = 120, //90,
        WIDE = 180,
        ILLEGAL = 0,
    };

private:
    //! type Id
    Type M_type;

public:
    /*!
      \brief init by NORMAL
    */
    ViewWidth()
        : M_type( ViewWidth::NORMAL )
      { }
    /*!
      \brief construct with 't'. NO explicit
      \param t type Id
    */
    ViewWidth( const ViewWidth::Type t )
        : M_type( t )
      { }

    /*!
      \brief substituion operator by type Id
      \param t type Id
      \return reference to itself
    */
    ViewWidth & operator=( const ViewWidth::Type t )
      {
          M_type = t;
          return *this;
      }

    /*!
      \brief type operator
      \return type Id
    */
    operator ViewWidth::Type() const
      {
          return M_type;
      }

    /*!
      \brief get type Id
      \return type Id
    */
    ViewWidth::Type type() const
      {
          return M_type;
      }

    /*!
      \brief compare operator.
      \param w value to be compared
      \return compared result

      This operator should declare and defined at the global scope...
    */
    bool operator==( const ViewWidth & w ) const
      {
          return this->type() == w.type();
      }

    /*!
      \brief compare operator.
      \param t type Id to be compared
      \return compared result
    */
    bool operator==( const ViewWidth::Type t ) const
      {
          return this->type() == t;
      }

    /*!
      \brief compare operator.
      \param w value to be compared
      \return compared result

      This operator should declare and defined at the global scope...
    */
    bool operator!=( const ViewWidth & w ) const
      {
          return this->type() != w.type();
      }

    /*!
      \brief compare operator.
      \param t type Id to be compared
      \return compared result
    */
    bool operator!=( const ViewWidth::Type t ) const
      {
          return this->type() != t;
      }

    /*!
      \brief increase view width type
      \return const reference to this object
     */
    const
    ViewWidth & operator++();

    /*!
      \brief increase view width type
      \return old value
     */
    const
    ViewWidth operator++( int );

    /*!
      \brief decrease view width type
      \return const reference to this object
     */
    const
    ViewWidth & operator--();

    /*!
      \brief increase view width type
      \return old value
     */
    const
    ViewWidth operator--( int );

    /*!
      \brief get degree value depending on the current type and server param
      \return degree value
    */
    double width() const
      {
          return width( this->type() );
      }

    /*!
      \brief get width type name
      \return the name string of width type
    */
    std::string str() const;

    /*!
      \brief get degree value depending on the current type and server param
      \param type view width type
      \return degree value
    */
    static
    double width( const ViewWidth::Type type );

    /*!
      \brief static method. get the type described by string
      \param msg string to be analyzed
      \return type Id
    */
    static
    ViewWidth::Type parse( const char * msg );
};


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

/*!
  \class ViewQuality
  \brief view quality data class
*/
class ViewQuality {
public:
    /*!
      \enum Type
      \brief types of view quality
    */
    enum Type {
        HIGH,
        LOW,
        ILLEGAL
    };

private:
    //! type Id
    Type M_type;

public:
    /*!
      \brief init by HIGH
    */
    ViewQuality()
        : M_type( ViewQuality::HIGH )
      { }

    /*!
      \brief init by t
      \param t type Id
    */
    ViewQuality( const ViewQuality::Type t )
        : M_type( t )
      { }

    /*!
      \brief substitution operator by type Id
      \param t type Id
      \return reference to itself
    */
    ViewQuality & operator=( const ViewQuality::Type t )
      {
          M_type = t;
          return *this;
      }

    /*!
      \brief type operator
      \return type Id
    */
    operator ViewQuality::Type() const
      {
          return M_type;
      }

    /*!
      \brief get type Id
      \return type Id
    */
    ViewQuality::Type type() const
      {
          return M_type;
      }

    /*!
      \brief compare operator.
      \param q value to be compared
      \return compared result

      This operator should declare and defined at the global scope...
    */
    bool operator==( const ViewQuality & q ) const
      {
          return this->type() == q.type();
      }

    /*!
      \brief compare operator.
      \param t type Id to be compared
      \return compared result
    */
    bool operator==( const ViewQuality::Type t ) const
      {
          return this->type() == t;
      }

    /*!
      \brief compare operator.
      \param q value to be compared
      \return compared result

      This operator should declare and defined at the global scope...
    */
    bool operator!=( const ViewQuality & q ) const
      {
          return this->type() != q.type();
      }

    /*!
      \brief compare operator.
      \param t type Id to be compared
      \return compared result
    */
    bool operator!=( const ViewQuality::Type& t ) const
      {
          return this->type() != t;
      }

    /*!
      \brief get quality type name
      \return the name string of quality type
    */
    //! return the name of current view quality
    std::string str() const;

    /*!
      \brief static method. get the type described by string
      \param msg string to be analyzed
      \return type Id
    */
    static
    ViewQuality::Type parse( const char * msg );
};

}

#endif
