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

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <variant>
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

    //! value pointer type
    using ValuePtr = std::variant< int*, size_t*, double*, bool*, NegateBool, BoolSwitch, NegateSwitch, std::string* >;

    //! ParamEntity smart pointer type
    using Ptr = std::shared_ptr< ParamEntity >;

private:
    //! long parameter name
    std::string M_long_name;
    //! short parameter name
    std::string M_short_name;
    // parameter value pointer holder as variant
    ValuePtr M_value_ptr;
    //! parameter description
    std::string M_description;

    //! not used
    ParamEntity() = delete;

public:

    /*!
      \brief construct with all arguments
      \param long_name long parameter name
      \param short_name short parameter name
      \param value_ptr pointer type to the paramter variable
      \param description description message about this parameter
    */
    ParamEntity( const std::string & long_name,
                 const std::string & short_name,
                 ValuePtr value_ptr,
                 const char * description = "" )
        : M_long_name( long_name ),
          M_short_name( short_name ),
          M_value_ptr( value_ptr ),
          M_description( description )
      { }


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
      \brief check if this parameter is switch type or not.
      \return true if this parameter is switch type..
    */
    bool isSwitch() const;

    /*!
      \brief pure virtual method. analyze value string.
      \return boolean status of analysis result
    */
    bool analyze( const std::string & value_str );

    /*!
      \brief print help name strings
      \param os reference to the stream
      \return reference to the stream
     */
    std::ostream & printFormat( std::ostream & os ) const;

    /*!
      \brief pure virtual method. print value to stream
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
public:
    //! map type
    using Map = std::unordered_map< std::string, ParamEntity::Ptr >;
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
        Registrar & operator()( const std::string & long_name,
                                const std::string & short_name,
                                ParamEntity::ValuePtr value_ptr,
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
    Map M_long_name_map;

    //! short name option map
    Map M_short_name_map;


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
    const Map & longNameMap() const
      {
          return M_long_name_map;
      }

    /*!
      \brief get the short name parameter map
      \return const reference to the container instance
     */
    const Map & shortNameMap() const
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
