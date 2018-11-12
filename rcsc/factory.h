// -*-c++-*-

/***************************************************************************
                                factory.h
                             -------------------
    Template singleton for creating polymorphic objects based on some idx
    begin                : 2002-10-08
    copyright            : (C) 2002 by The RoboCup Soccer Simulator
                           Maintenance Group.
    email                : sserver-admin@lists.sourceforge.net
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU LGPL as published by the Free Software  *
 *   Foundation; either version 2 of the License, or (at your option) any  *
 *   later version.                                                        *
 *                                                                         *
 ***************************************************************************/

#ifndef RCSSFACTORY_H
#define RCSSFACTORY_H

#include <map>
#include <list>
#include <stack>
#include <iostream>
#include <memory>
#include <cstring>

namespace rcss {

/*!
  \class less
  \brief template less compare function
 */
template< typename X >
class less {
public:
    /*!
      \brief compare operation function
      \param a left hand side variable
      \param b right hand side variable
      \return compared result
     */
    bool operator()( const X & a,
                     const X & b ) const
      {
          return a < b;
      }
};

/*!
  \class less< const char * >
  \brief less compare function specialized for 'const char *'
 */
template<>
class less< const char * > {
public:
    /*!
      \brief compare operation function
      \param a left hand side variable
      \param b right hand side variable
      \return compared result
     */
    bool operator()( const char * a,
                     const char * b ) const
      {
          return std::strcmp( a, b ) < 0;
      }
};

/*!
  \class less< char * >
  \brief less compare function specialized for 'char *'
 */
template<>
class less< char * > {
public:
    /*!
      \brief compare operation function
      \param a left hand side variable
      \param b right hand side variable
      \return compared result
     */
    bool operator()( char * a,
                     char * b ) const
      {
          return std::strcmp( a, b ) < 0;
      }
};


/*!
  \class RegHolderImpl
  \brief Base type for AutoReger
*/
class RegHolderImpl {
public:
    /*!
      \brief constructor
     */
    RegHolderImpl()
      { }

    /*!
      \brief virtual destructor
     */
    virtual
    ~RegHolderImpl()
      { }
};

//! RegHolder type
typedef std::unique_ptr< RegHolderImpl > RegHolder;


/*!
  \class AutoReger
  \brief Used for automatic registration.

  NOTE: Auto registration Cannot be used in dynamic libraries
*/
template< typename OF >
class AutoReger
    : public RegHolderImpl {
public:
    typedef OF Factory; //!< factory type
    typedef typename Factory::Creator Creator; //!< creator type
    typedef typename Factory::Index Index; //!< index type

    /*!
      \brief register creator with index
      \param fact reference to the factory
      \param creator creator object
      \param idx index value
     */
    AutoReger( Factory & fact,
               Creator creator,
               const Index & idx )
        : M_fact( fact ),
          M_idx( idx )
      {
          M_fact.reg( creator, idx );
      }

    /*!
      \brief deregister the create
     */
    virtual
    ~AutoReger()
      {
          M_fact.dereg( M_idx );
      }

private:
    template< class OF2 >
    AutoReger( const AutoReger< OF2 > & ); // not used

    template< class OF2 >
    AutoReger & operator=( const AutoReger< OF2 > & ); // not used

private:
    Factory & M_fact; //!< reference to the factory
    const Index & M_idx; //!< const reference to the index variable
};


/*!
  \class Factory
  \brief An Generic Object Factory (aka Class Store)
*/
template< class Cre,
          class I = const char *,
          class Com = less< I > >
class Factory {
public:
    typedef Cre Creator; //!< creator type
    typedef I Index; //!< index type
    typedef Com Compare; //!< compare function type
private:
    typedef std::map< Index, std::stack< Creator >, Compare > Map; //!< map container type
private:
    Map M_creators; //!< creator container

public:
    /*!
      \brief constructor
     */
    Factory()
      { }

    /*!
      \brief destructor
     */
    ~Factory()
      { }

    /*!
      \brief register the creator with index variable
      \param c creator object
      \param idx const reference to the index variable
     */
    void reg( Creator c,
              const Index & idx )
      {
          M_creators[ idx ].push( c );
      }

    /*!
      \brief deregister the specified creator
      \param idx deregistered index value
     */
    void dereg( const Index & idx )
      {
          //std::cerr << "dereg [" << idx << "]" << std::endl;
          typename Map::iterator i = M_creators.find( idx );
          if ( i != M_creators.end() )
          {
              if ( ! i->second.empty() )
              {
                  i->second.pop();
              }

              if ( i->second.empty() )
              {
                  M_creators.erase( i );
              }
          }
      }

    /*!
      \brief get the specified create object
      \param c reference to the creator variable
      \param idx index value
      \return if creator is found, return true
     */
    bool getCreator( Creator & c,
                     const Index & idx ) const
      {
          typename Map::const_iterator i = M_creators.find( idx );
          if ( i != M_creators.end()
               && ! i->second.empty() )
          {
              c = i->second.top();
              return true;
          }
          return false;
      }

    /*!
      \brief get the index list
      \return index list
     */
    std::list< Index > list() const
      {
          std::list< Index > rval;
          for ( typename Map::const_iterator i = M_creators.begin();
                i != M_creators.end();
                ++i )
          {
              rval.push_back( i->first );
          }
          return rval;
      }

    /*!
      \brief print index and the size of creators
      \param o reference to the output stream
      \return reference to the output stream
     */
    std::ostream & printList( std::ostream & o = std::cout ) const
      {
          for ( typename Map::const_iterator i = M_creators.begin();
                i != M_creators.end();
                ++i )
          {
              o << "\t" << i->first
                << "(" << i->second.size() << ")"
                << std::endl;
          }
          return o;
      }

    /*!
      \brief get the size of registered index
      \return the size of registered index
     */
    size_t size() const
      {
          return M_creators.size();
      }

    /*!
      \brief get the size of registered creators for the input index
      \param idx index value
      \return the size of registered creators
     */
    size_t size( const Index & idx ) const
      {
          for ( typename Map::const_iterator i = M_creators.begin();
                i != M_creators.end();
                ++i )
          {
              if ( ! Compare()( idx, i->first )
                   && ! Compare()( i->first, idx ) )
              {
                  return i->second.size();
              }
          }
          return 0;
      }

    /*!
      \brief register the creator using AutoReger. used for the automatic registration.
      \param c creator object
      \param i const reference to the index variable
      \return RegHolder instance
     */
    RegHolder autoReg( Creator c,
                       const Index & i )
      {
          //std::cerr << "autoReg: " << i << std::endl;
          return RegHolder( new AutoReger< Factory< Creator, Index, Compare > >( *this, c, i ) );
      }
};

}

#endif
