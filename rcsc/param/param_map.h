// -*-c++-*-

/*!
  \file param_map.h
  \brief parameter registry map Header File
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

#ifndef RCSC_PARAM_PARAM_MAP_H
#define RCSC_PARAM_PARAM_MAP_H

#include <boost/lexical_cast.hpp>

#include <memory>
#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <cassert>

namespace rcsc {

/*!
  \struct NegateBool
  \brief negate wrapper.
 */
struct NegateBool {
    bool * ptr_; //!< raw pointer to the parameter variable

    /*!
      \brief constructor
      \param ptr raw pointer to the parameter variable.
     */
    explicit
    NegateBool( bool * ptr )
        : ptr_( ptr )
      {
          assert( ptr );
      }

private:
    // not used
    NegateBool() = delete;
};

/*!
  \struct BoolSwitch
  \brief wrapper for bool switch.
 */
struct BoolSwitch {
    bool * ptr_; //!< raw pointer to the parameter variable

    /*!
      \brief constructor
      \param ptr raw pointer to the parameter variable.
     */
    explicit
    BoolSwitch( bool * ptr )
        : ptr_( ptr )
      {
          assert( ptr );
      }

private:
    // not used
    BoolSwitch() = delete;
};


/*!
  \struct NegateSwitch
  \brief negate wrapper for bool switch.
*/
struct NegateSwitch {
    bool * ptr_; //!< pointer to the parameter variable.

    /*!
      \brief constructor
      \param ptr raw pointer to the parameter variable.
     */
    explicit
    NegateSwitch( bool * ptr )
        : ptr_( ptr )
      {
          assert( ptr );
      }

    /*!
      \brief constructor
      \param value bool switch object
     */
    explicit
    NegateSwitch( const BoolSwitch & value )
        : ptr_( value.ptr_ )
      {
          assert( ptr_ );
      }

private:
    // not used
    NegateSwitch() = delete;
};

/*-------------------------------------------------------------------*/
/*!
  \class ParamEntity
  \brief abstract parameter
*/
class ParamEntity {
public:

    //! ParamEntity smart pointer type
    typedef std::shared_ptr< ParamEntity > Ptr;

private:
    //! long parameter name
    std::string M_long_name;
    //! short parameter name
    std::string M_short_name;
    //! parameter description
    std::string M_description;

    //! not used
    ParamEntity() = delete;

protected:

    /*!
      \brief construct with all arguments
      \param long_name long parameter name
      \param short_name short parameter name
      \param description description message about this parameter
    */
    ParamEntity( const std::string & long_name,
                const std::string & short_name,
                const char * description = "" )
        : M_long_name( long_name ),
          M_short_name( short_name ),
          M_description( description )
      { }

public:

    /*!
      \brief destructor as virtual method
    */
    virtual
    ~ParamEntity()
      { }

    /*!
      \brief get long name of parameter
      \return const reference to the name string
    */
    const std::string & longName() const
      {
          return M_long_name;
      }

    /*!
      \brief get long name of parameter
      \return const reference to the short string
    */
    const std::string & shortName() const
      {
          return M_short_name;
      }
    /*!
      \brief get description message
      \return const reference to the descriptin message
    */
    const std::string & description() const
      {
          return M_description;
      }

    /*!
      \brief get the formatted name string for help messages
      \return formatted name string
     */
    std::string helpName() const;

    /*!
      \brief (virtual) check if this parameter is switch type or not.
      \return true if this parameter is switch type..
    */
    virtual
    bool isSwitch() const
      {
          return false;
      }

    /*!
      \brief pure virtual method. analyze value string.
      \return boolean status of analysis result
    */
    virtual
    bool analyze( const std::string & value_str ) = 0;

    /*!
      \brief print help name strings
      \param os reference to the stream
      \return reference to the stream
     */
    virtual
    std::ostream & printFormat( std::ostream & os ) const;

    /*!
      \brief pure virtual method. print value to stream
      \param os reference to the stream
      \return reference to the stream
     */
    virtual
    std::ostream & printValue( std::ostream & os ) const = 0;

};

/*-------------------------------------------------------------------*/
/*!
  \class ParamGeneric
  \brief generic parameter
*/
template < typename ValueType >
class ParamGeneric
    : public ParamEntity {
public:
    typedef ValueType Type; //!< value type alias

private:
    //! pointer to parameter variable instance
    Type * M_value_ptr;

public:

    /*!
      \brief constructor
      \param long_name parameter's long name string
      \param short_name parameter's short name string(ommitable)
      \param value_ptr pointer to variable instance
      \param description description message(ommitable)
     */
    ParamGeneric( const std::string & long_name,
                  const std::string & short_name,
                  Type * value_ptr,
                  const char * description = "" )
        : ParamEntity( long_name, short_name, description ),
          M_value_ptr( value_ptr )
      {
          assert( value_ptr );
      }

    /*!
      \brief analyze value string and substitute it to variable.
      \param value_str string that contains value
      \return boolean status of analysis result
     */
    bool analyze( const std::string & value_str )
      {
          try
          {
              *M_value_ptr = boost::lexical_cast< Type >( value_str );
              return true;
          }
          catch ( boost::bad_lexical_cast & e )
          {
              std::cerr << e.what() << "  [" << value_str << "]"
                        << std::endl;
              return false;
          }
      }

private:

    template < typename T >
    std::ostream & printValueImpl( std::ostream & os,
                                   const T & value ) const
      {
          os << value;
          return os;
      }

    std::ostream & printValueImpl( std::ostream & os,
                                   const std::string & value ) const
      {
          os << '"' << value << '"';
          return os;
      }

public:
    /*!
      \brief print value to stream
      \param os reference to the stream
      \return reference to the stream
     */
    std::ostream & printValue( std::ostream & os ) const
      {
          return printValueImpl( os, *M_value_ptr );
      }

};

/*-------------------------------------------------------------------*/
/*!
  \class ParamGeneric< bool >
  \brief specialized template class
*/
template <>
class ParamGeneric< bool >
    : public ParamEntity {
private:
    const bool M_negate;

    //! pointer to parameter variable instance
    bool * M_value_ptr;

public:

    /*!
      \brief constructor
      \param long_name parameter's long name string
      \param short_name parameter's short name string(ommitable)
      \param value_ptr pointer to variable instance
      \param description description message(ommitable)
     */
    ParamGeneric( const std::string & long_name,
                  const std::string & short_name,
                  bool * value_ptr,
                  const char * description = "" )
        : ParamEntity( long_name, short_name, description ),
          M_negate( false ),
          M_value_ptr( value_ptr )
      {
          assert( value_ptr );
      }

    /*!
      \brief constructor
      \param long_name parameter's long name string
      \param short_name parameter's short name string(ommitable)
      \param value negate bool object
      \param description description message(ommitable)
     */
    ParamGeneric( const std::string & long_name,
                  const std::string & short_name,
                  const NegateBool & value,
                  const char * description = "" )
        : ParamEntity( long_name, short_name, description ),
          M_negate( true ),
          M_value_ptr( value.ptr_ )
      {
          assert( M_value_ptr );
      }

    /*!
      \brief analyze value string and substitute it to variable.
      \param value_str string that contains value
      \return boolean status of analysis result
    */
    bool analyze( const std::string & value_str );

    /*!
      \brief print value to stream
      \param os reference to the stream
      \return reference to the stream
     */
    std::ostream & printValue( std::ostream & os ) const;

};

/*-------------------------------------------------------------------*/
/*!
  \class ParamSwitch
  \brief switch type parameter
*/
class ParamSwitch
    : public ParamEntity {
private:
    const bool M_negate;

    //! pointer to parameter variable instance
    bool * M_value_ptr;

public:

    /*!
      \brief constructor
      \param long_name parameter's long name string
      \param short_name parameter's short name string(ommitable)
      \param value_ptr pointer to variable instance
      \param description description message(ommitable)
     */
    ParamSwitch( const std::string & long_name,
                 const std::string & short_name,
                 bool * value_ptr,
                 const char * description = "" )
        : ParamEntity( long_name, short_name, description ),
          M_negate( false ),
          M_value_ptr( value_ptr )
      {
          assert( value_ptr );
      }


    /*!
      \brief constructor
      \param long_name parameter's long name string
      \param short_name parameter's short name string(ommitable)
      \param value negate bool object
      \param description description message(ommitable)
     */
    ParamSwitch( const std::string & long_name,
                 const std::string & short_name,
                 const NegateSwitch & value,
                 const char * description = "" )
        : ParamEntity( long_name, short_name, description ),
          M_negate( true ),
          M_value_ptr( value.ptr_ )
      {
          assert( M_value_ptr );
      }

    /*!
      \brief (virtual) check if this parameter is switch type or not.
      \return always true.
    */
    virtual
    bool isSwitch() const
      {
          return true;
      }

    /*!
      \brief analyze value string and substitute it to variable.
      \param value_str string that contains value
      \return boolean status of analysis result
    */
    bool analyze( const std::string & value_str );

    /*!
      \brief print usage format.
      \param os reference to the stream
      \return reference to the stream
     */
    std::ostream & printFormat( std::ostream & os ) const;

    /*!
      \brief print value to stream
      \param os reference to the stream
      \return reference to the stream
     */
    std::ostream & printValue( std::ostream & os ) const;

};


/*-------------------------------------------------------------------*/
/*!
  \class ParamMap
  \brief parameter container
*/
class ParamMap {
private:

    /*!
      \class Registrar
      \brief convinience class to simplify parameter registration
     */
    class Registrar {
    private:
        //! reference to parameter container
        ParamMap & M_param_map;
        //! not used
        Registrar() = delete;
    public:
        /*!
          \brief construct with parameter map
          \param pmap reference to parameter map instance
         */
        explicit
        Registrar( ParamMap & pmap )
            : M_param_map( pmap )
          { }

        /*!
          \brief parameter registration operator
          \param long_name parameter's long name
          \param short_name parameter's short name(ommitable)
          \param value_ptr pointer to parameter variable instance
          \param description parameter's description message(ommitable)
          \return reference to itself
         */
        template < typename ValueType >
        Registrar & operator()( const std::string & long_name,
                                const std::string & short_name,
                                ValueType * value_ptr,
                                const char * description = "" )
          {
              if ( ! checkName( long_name, short_name ) )
              {
                  M_param_map.M_valid = false;
                  return *this;
              }

              if ( ! value_ptr )
              {
                  std::cerr << "***ERROR*** detected null pointer for the option "
                            << long_name << std::endl;
                  M_param_map.M_valid = false;
                  return *this;
              }

              ParamEntity::Ptr ptr( new ParamGeneric< ValueType >( long_name,
                                                                   short_name,
                                                                   value_ptr,
                                                                   description ) );
              M_param_map.add( ptr );
              return *this;
          }

        /*!
          \brief parameter registration operator for negate bool
          \param long_name parameter's long name
          \param short_name parameter's short name(ommitable)
          \param value wrapper object
          \param description parameter's description message(ommitable)
          \return reference to itself
         */
        Registrar & operator()( const std::string & long_name,
                                const std::string & short_name,
                                const NegateBool & value,
                                const char * description = "" );

        /*!
          \brief parameter registration operator for bool switch
          \param long_name parameter's long name
          \param short_name parameter's short name(ommitable)
          \param value wrapper object
          \param description parameter's description message(ommitable)
          \return reference to itself
         */
        Registrar & operator()( const std::string & long_name,
                                const std::string & short_name,
                                const BoolSwitch & value,
                                const char * description = "" );

       /*!
          \brief parameter registration operator for negate bool switch
          \param long_name parameter's long name
          \param short_name parameter's short name(ommitable)
          \param value wrapper object
          \param description parameter's description message(ommitable)
          \return reference to itself
         */
        Registrar & operator()( const std::string & long_name,
                                const std::string & short_name,
                                const NegateSwitch & value,
                                const char * description = "" );

    private:

        /*!
         \brief check if given names are valid or not.
         \return checked result
        */
        bool checkName( const std::string & long_nam,
                        const std::string & short_name ) const;
    };

    //! validation flag
    bool M_valid;

    //! parameter registrar
    Registrar M_registrar;

    //! option group name: output before help messages
    std::string M_group_name;

    //! parameter container
    std::vector< ParamEntity::Ptr > M_parameters;

    //! long name option map
    std::map< std::string, ParamEntity::Ptr > M_long_name_map;

    //! short name option map
    std::map< std::string, ParamEntity::Ptr > M_short_name_map;


    // no copyable
    ParamMap( const ParamMap & );
    ParamMap & operator=( const ParamMap & );

public:

    /*!
      \brief default constructor. create registrer
     */
    ParamMap()
        : M_valid( true ),
          M_registrar( *this )
      { }

    /*!
      \brief construct with option name string
      \param group_name option group name string
     */
    explicit
    ParamMap( const std::string & group_name )
        : M_valid( true ),
          M_registrar( *this ),
          M_group_name( group_name )
      { }

    /*!
      \brief destructor. nothing to do
     */
    ~ParamMap()
      { }

    /*!
      \brief check if all registered options are valid or not.
      \return checked result.
     */
    bool isValid() const
      {
          return M_valid;
      }

    /*!
      \brief get the name of parameter group
      \return name string
     */
    const std::string & groupName() const
      {
          return M_group_name;
      }

    /*!
      \brief get the container of all parameters
      \return const reference to the container instance
     */
    const std::vector< ParamEntity::Ptr > & parameters() const
      {
          return M_parameters;
      }

    /*!
      \brief get the long name parameter map
      \return const reference to the container instance
     */
    const std::map< std::string, ParamEntity::Ptr > & longNameMap() const
      {
          return M_long_name_map;
      }

    /*!
      \brief get the short name parameter map
      \return const reference to the container instance
     */
    const std::map< std::string, ParamEntity::Ptr > & shortNameMap() const
      {
          return M_short_name_map;
      }

    /*!
      \brief get a parameter registrar
      \return reference to the parameger registrar
     */
    Registrar & add()
      {
          return M_registrar;
      }

    /*!
      \brief add new parameter entry
      \param param shared pointer of parameter entry
     */
    Registrar & add( ParamEntity::Ptr param );

    /*!
      \brief remove registered parameter pointer
      \param long_name parameter name string
     */
    void remove( const std::string & long_name );

    /*!
      \brief get parameter entry that has the argument name
      \param long_name long version parameter name string
      \return parameter entry pointer. if not found, NULL is returned.
     */
    ParamEntity::Ptr findLongName( const std::string & long_name );

    /*!
      \brief get parameter entry that has the argument name
      \param short_name set of the parameter name character
      \return parameter entry pointer. if not found, NULL is returned.
     */
    ParamEntity::Ptr findShortName( const std::string & short_name );

    /*!
      \brief output parameter usage by command line option style
      \param os reference to output streamf
      \param with_default if true, default value is printed.
      \return reference to output stream
    */
    std::ostream & printHelp( std::ostream & os,
                              const bool with_default = true ) const;

    /*!
      \brief output parameter name and value
      \param os reference to output stream
      \return reference to output stream
     */
    std::ostream & printValues( std::ostream & os ) const;
};

}

#endif
