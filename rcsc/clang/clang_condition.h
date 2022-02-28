// -*-c++-*-

/*!
  \file clang_condition.h
  \brief clang condition class Header File
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

#ifndef RCSC_CLANG_CONDITION_H
#define RCSC_CLANG_CONDITION_H

#include <memory>
#include <iosfwd>

namespace rcsc {

/*!
  \class CLangCondition
  \brief abstract clang condition
 */
class CLangCondition {
public:

    /*!
      \enum Type
      \brief clang condition types
     */
    enum Type {
        BOOL, //!< true or false
        PPOS, //!< player position
        BPOS, //!< ball position
        BOWNER, //!< ball owner
        PLAY_MODE,
        AND,
        OR,
        NOT,
        NAMED,
        COND_COMP, // time_comp, opp_goal_comp, our_goal_comp, goal_diff_comp
        UNUM,
        MAX_TYPE,
    };

    //! smart pointer type
    typedef std::shared_ptr< CLangCondition > Ptr;

private:

    // not used
    CLangCondition( const CLangCondition & ) = delete;
    CLangCondition & operator=( const CLangCondition & ) = delete;

protected:

    /*!
      \brief protected constructor
     */
    CLangCondition() = default;

public:

    /*!
      \brief virtual destructor
     */
    virtual
    ~CLangCondition() = default;

    /*!
      \brief get type id.
      \return type id.
     */
    virtual
    Type type() const = 0;

    /*!
      \brief print clang message to the output stream
      \param os reference to the output stream
      \return reference to the output stream
     */
    virtual
    std::ostream & print( std::ostream & os ) const = 0;

};

}

inline
std::ostream &
operator<<( std::ostream & os,
            const rcsc::CLangCondition & cond )
{
    return cond.print( os );
}


namespace rcsc {

/*!
  \class CLangConditionBool
  \brief simple boolean condition
 */
class CLangConditionBool
    : public CLangCondition {
private:

    const bool M_value; //!< condition value

    // not used
    CLangConditionBool() = delete;

public:

    /*!
      \create object with condition value
      \param val condition value
     */
    CLangConditionBool( const bool val )
        : M_value( val )
      { }

    // ~CLangConditionBool()
    //   {
    //       std::cerr << "delete CLangConditionBool " << M_value << std::endl;
    //   }

    /*!
      \brief get type id.
      \return type id.
     */
    virtual
    Type type() const
      {
          return BOOL;
      }

    /*!
      \brief get condition value
      \return condition value
     */
    bool value() const
      {
          return M_value;
      }


    /*!
      \brief print clang message to the output stream
      \param os reference to the output stream
      \return reference to the output stream
     */
    virtual
    std::ostream & print( std::ostream & os ) const;

};

//
// TODO:
//
// PPos
// BallPos
// BallOwner
// PlayMode
// And
// Or
// Not
// STRING
// COND_COMP
// Unum
//

}

#endif
