// -*-c++-*-

/*!
  \file clang_token.h
  \brief clang token class Header File
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

#ifndef RCSC_CLANG_TOKEN_H
#define RCSC_CLANG_TOKEN_H

#include <rcsc/clang/clang_directive.h>
#include <rcsc/clang/clang_condition.h>

#include <memory>
#include <vector>
#include <iostream>

namespace rcsc {

/*!
  \class CLangToken
  \brief abstract clang token
 */
class CLangToken {
public:

    //! smart pointer type
    typedef std::shared_ptr< const CLangToken > ConstPtr;

    // container type
    typedef std::vector< ConstPtr > Cont;

protected:

    /*!
      \brief protected constructor
     */
    CLangToken()
      { }

public:

    /*!
      \brief virtual destructor
     */
    virtual
    ~CLangToken()
      { }

    /*!
      \brief print clang message to the output stream
      \param os reference to the output stream
      \return reference to the output stream
     */
    virtual
    std::ostream & print( std::ostream & os ) const = 0;

};

}

/*!
  \brief stream operator
  \param os reference to the output stream
  \param tok clang token object
  \return reference to the output stream
 */
inline
std::ostream &
operator<<( std::ostream & os,
            const rcsc::CLangToken & tok )
{
    return tok.print( os );
}

namespace rcsc {

/*!
  \class CLangTokenClear
  \brief clang clear token
 */
class CLangTokenClear
    : public CLangToken {
public:

    /*!
      \brief default constructor
     */
    CLangTokenClear()
        : CLangToken()
      { }

    /*!
      \brief print clang message to the output stream
      \param os reference to the output stream
      \return reference to the output stream
     */
    virtual
    std::ostream & print( std::ostream & os ) const;
};


/*!
  \class CLangTokenRule
  \brief clang rule token
 */
class CLangTokenRule
    : public CLangToken {
private:

    int M_ttl; //!< time to live
    CLangCondition::Ptr M_condition; //!< rule condition
    CLangDirective::Cont M_directives; //!< directive list

public:

    /*!
      \brief default constructor. ttl is set to 0.
     */
    CLangTokenRule()
        : CLangToken(),
          M_ttl( 0 )
      { }


    /*!
      \brief construct with ttl value.
      \param ttl time to live value
     */
    explicit
    CLangTokenRule( const int ttl )
        : CLangToken(),
          M_ttl( ttl )
      { }

    // ~CLangTokenRule()
    //   {
    //       std::cerr << "delete CLangTokenRule " << *this << std::endl;
    //   }

    /*!
      \brief set TTL value
      \param ttl TTL value
     */
    void setTTL( const int ttl )
      {
          M_ttl = ttl;
      }

    /*!
      \brief set rule condition
      \param cond new condition object pointer
     */
    void setCondition( CLangCondition * cond )
      {
          M_condition = CLangCondition::Ptr( cond );
      }

    /*!
      \brief add directive to this rule
      \param dir new directive object pointer
     */
    void addDirective( CLangDirective * dir )
      {
          M_directives.push_back( CLangDirective::ConstPtr( dir ) );
      }

    /*!
      \brief get TTL value
      \return TTL value
     */
    int ttl() const
      {
          return M_ttl;
      }

    /*!
      \brief get the rule condition
      \return condtion object pointer
     */
    const CLangCondition::Ptr & condition() const
      {
          return M_condition;
      }

    /*!
      \brief get the directive list
      \return directive list
     */
    const CLangDirective::Cont & directives() const
      {
          return M_directives;
      }


    /*!
      \brief print clang message to the output stream
      \param os reference to the output stream
      \return reference to the output stream
     */
    virtual
    std::ostream & print( std::ostream & os ) const;

};

}

#endif
