// -*-c++-*-

/*!
  \file param_map.cpp
  \brief parameter registry map Source File
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "param_map.h"

#include <sstream>
#include <algorithm>

namespace rcsc {

namespace {

/*-------------------------------------------------------------------*/
inline
bool
is_true( const std::string & value_str )
{
    return ( value_str == "true"
             || value_str == "on"
             || value_str == "1"
             || value_str == "yes" );
}

/*-------------------------------------------------------------------*/
inline
bool
is_false( const std::string & value_str )
{
    return ( value_str == "false"
             || value_str == "off"
             || value_str == "0"
             || value_str == "no" );
}

/*-------------------------------------------------------------------*/
struct IsSwitch {

    bool operator()( int * )
      {
          return false;
      }

    bool operator()( size_t * )
      {
          return false;
      }

    bool operator()( double * )
      {
          return false;
      }

    bool operator()( bool * )
      {
          return false;
      }

    bool operator()( NegateBool )
      {
          return false;
      }

    bool operator()( BoolSwitch )
      {
          return true;
      }

    bool operator()( NegateSwitch )
      {
          return true;
      }

    bool operator()( std::string * )
      {
          return false;
      }

};

/*-------------------------------------------------------------------*/
struct ValueParser {
    const std::string & value_str;

    ValueParser( const std::string & str )
        : value_str( str )
      { }

    void operator()( int * ptr )
      {
          *ptr = std::stoi( value_str );
      }

    void operator()( size_t * ptr )
      {
          *ptr = std::stoul( value_str );
      }

    void operator()( double * ptr )
      {
          *ptr = std::stod( value_str );
      }

    void operator()( bool * ptr )
      {
          if ( is_true( value_str ) )
          {
              *ptr = true;
          }
          else if ( is_false( value_str ) )
          {
              *ptr = false;
          }
          else
          {
              throw( std::invalid_argument( "Unknown bool string." ) );
          }
      }

    void operator()( NegateBool ptr )
      {
          if ( is_true( value_str ) )
          {
              *(ptr.ptr_) = false;
          }
          else if ( is_false( value_str ) )
          {
              *(ptr.ptr_) = true;
          }
          else
          {
              throw( std::invalid_argument( "Unknown bool string." ) );
          }
      }

    void operator()( BoolSwitch ptr )
      {
          *(ptr.ptr_) = true;
      }

    void operator()( NegateSwitch ptr )
      {
          *(ptr.ptr_) = false;
      }

    void operator()( std::string * ptr )
      {
          *ptr = value_str;
      }

};

/*-------------------------------------------------------------------*/
struct PointerCheck {

    bool operator()( int * ptr )
      {
          return ptr;
      }

    bool operator()( size_t * ptr )
      {
          return ptr;
      }

    bool operator()( double * ptr )
      {
          return ptr;
      }

    bool operator()( bool * ptr )
      {
          return ptr;
      }

    bool operator()( NegateBool ptr )
      {
          return ptr.ptr_;
      }

    bool operator()( BoolSwitch ptr )
      {
          return ptr.ptr_;
      }

    bool operator()( NegateSwitch ptr )
      {
          return ptr.ptr_;
      }

    bool operator()( std::string * ptr )
      {
          return ptr;
      }

};

/*-------------------------------------------------------------------*/
struct ValuePrinter {
    std::ostream & os_;

    ValuePrinter( std::ostream & os )
        : os_( os )
      { }

    void operator()( int * ptr )
      {
          os_ << *ptr;
      }

    void operator()( size_t * ptr )
      {
          os_ << *ptr;
      }

    void operator()( double * ptr )
      {
          os_ << *ptr;
      }

    void operator()( bool * ptr )
      {
          os_ << std::boolalpha << *ptr;
      }

    void operator()( NegateBool ptr )
      {
          os_ << std::boolalpha << !*(ptr.ptr_);
      }

    void operator()( BoolSwitch ptr )
      {
          os_ << std::boolalpha << *(ptr.ptr_);
      }

    void operator()( NegateSwitch ptr )
      {
          os_ << std::boolalpha << !*(ptr.ptr_);
      }

    void operator()( std::string * ptr )
      {
          os_ << *ptr;
      }
};

}

/*-------------------------------------------------------------------*/
bool
ParamEntity::isSwitch() const
{
    try
    {
        return std::visit( IsSwitch(), M_value_ptr );
    }
    catch ( std::exception & e )
    {
        std::cerr << "(ParamEntity::isSwitch) " << e.what() << std::endl;
    }
    return false;
}

/*-------------------------------------------------------------------*/
bool
ParamEntity::analyze( const std::string & value_str )
{
    try
    {
        std::visit( ValueParser( value_str ), M_value_ptr );
    }
    catch ( std::exception & e )
    {
        std::cerr << "(ParamEntity::analyze) parse error. " << e.what() << std::endl;
        return false;
    }
    return true;
}

/*-------------------------------------------------------------------*/
std::ostream &
ParamEntity::printFormat( std::ostream & os ) const
{
    os << "--" << longName();
    if ( ! shortName().empty() )
    {
        os << " [ -" << shortName() << " ]";
    }

    os << " <Value>";
    return os;
}

/*-------------------------------------------------------------------*/
std::ostream &
ParamEntity::printValue( std::ostream & os ) const
{
    try
    {
        std::visit( ValuePrinter( os ), M_value_ptr );
    }
    catch ( std::exception & e )
    {
        std::cerr << "(ParamEntity::printValue) " << e.what() << std::endl;
    }
    return os;
}

/*-------------------------------------------------------------------*/
ParamMap::Registrar &
ParamMap::Registrar::operator()( const std::string & long_name,
                                 const std::string & short_name,
                                 ParamEntity::ValuePtr value_ptr,
                                 const char * description )
{
    if ( ! checkName( long_name, short_name ) )
    {
        M_param_map.M_valid = false;
        return *this;
    }

    try
    {
        if ( ! std::visit( PointerCheck(), value_ptr ) )
        {
            std::cerr << "***ERROR*** detected null pointer for the option "
                      << long_name << std::endl;
            M_param_map.M_valid = false;
            return *this;
        }
    }
    catch ( std::exception & e )
    {
        std::cerr << "(ParamMap::Regstrar) ERROR " << long_name << "\n"
                  << e.what() << std::endl;
        M_param_map.M_valid = false;
        return *this;
    }

    ParamEntity::Ptr ptr( new ParamEntity( long_name, short_name, value_ptr, description ) );
    M_param_map.add( ptr );
    return *this;
}

/*-------------------------------------------------------------------*/
bool
ParamMap::Registrar::checkName( const std::string & long_name,
                                const std::string & short_name ) const
{
    if ( long_name.empty() )
    {
        std::cerr << "***ERROR*** parameter long_name is empty."
                  << std::endl;
        return false;
    }

    if ( long_name.length() >= 40 )
    {
        std::cerr << "***ERROR*** parameter long_name have to be less than 40. ["
                  << long_name << ']' << std::endl;
        return false;
    }

    if ( short_name.length() >= 8 )
    {
        std::cerr << "***ERROR*** parameter short_name have to be less than 8. ["
                  << short_name << ']' << std::endl;
        return false;
    }

    return true;
}

/*-------------------------------------------------------------------*/
ParamMap::Registrar &
ParamMap::add( ParamEntity::Ptr param )
{
    if ( ! param )
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " ***ERROR*** ParamMap::add(). "
                  << "detected null ParamEntity::Ptr."
                  << std::endl;
        return M_registrar;
    }

    if ( param->longName().empty() )
    {
        std::cerr << __FILE__ << ':' << __LINE__
                  << " ***ERROR*** ParamMap::add(). "
                  << "Empty parameter name! parameter was not registered."
                  << std::endl;
        return M_registrar;
    }

    if ( param->longName().find( '=' ) != std::string::npos
         || param->shortName().find( '=' ) != std::string::npos )
    {
        std::cerr << " ***ERROR*** "
                  << " the option name [" << param->longName()
                  << "] or [" << param->shortName()
                  << "] contains an illegal character '='."
                  << std::endl;
        return M_registrar;
    }

    if ( param->longName() == "-"
         || param->shortName() == "-" )
    {
        std::cerr << " ***ERROR*** "
                  << " '-' cannot be used as the option name."
                  << std::endl;
        return M_registrar;
    }

    if ( M_long_name_map.find( param->longName() ) != M_long_name_map.end()
         || M_short_name_map.find( param->shortName() ) != M_short_name_map.end() )
    {
        std::cerr << " ***ERROR*** "
                  << " the option name [" << param->longName()
                  << "] or [" << param->shortName()
                  << "] has already been registered."
                  << std::endl;
        return M_registrar;
    }

    M_parameters.push_back( param );

    M_long_name_map[ param->longName() ] = param;

    if ( ! param->shortName().empty() )
    {
        M_short_name_map[ param->shortName() ] = param;
    }

    return M_registrar;
}

/*-------------------------------------------------------------------*/
void
ParamMap::remove( const std::string & long_name )
{
    M_parameters.erase( std::remove_if( M_parameters.begin(),
                                        M_parameters.end(),
                                        [&]( const ParamEntity::Ptr & v )
                                          {
                                              return v->longName() == long_name;
                                          } ),
                        M_parameters.end() );

    Map::iterator it_long = M_long_name_map.find( long_name );
    if ( it_long != M_long_name_map.end() )
    {
        if ( ! it_long->second->shortName().empty() )
        {
            std::unordered_map< std::string, ParamEntity::Ptr >::iterator it_short = M_short_name_map.find( it_long->second->shortName() );
            M_short_name_map.erase( it_short );
        }

        M_long_name_map.erase( it_long );
    }
}

/*-------------------------------------------------------------------*/
ParamEntity::Ptr
ParamMap::findLongName( const std::string & long_name )
{
    Map::iterator it = M_long_name_map.find( long_name );

    if ( it != M_long_name_map.end() )
    {
        return it->second;
    }

    return ParamEntity::Ptr();
}

/*-------------------------------------------------------------------*/
ParamEntity::Ptr
ParamMap::findShortName( const std::string & short_name )
{
    Map::iterator it = M_short_name_map.find( short_name );

    if ( it != M_short_name_map.end() )
    {
        return it->second;
    }

    return ParamEntity::Ptr();
}

/*-------------------------------------------------------------------*/
std::ostream &
ParamMap::printHelp( std::ostream & os,
                     const bool with_default ) const
{
    //const std::string indent_nl = "\n      ";

    os << '\n';

    if ( ! M_group_name.empty() )
    {
        os << ' ' << M_group_name << ":\n";
    }

    size_t width = 22;

    for ( const ParamEntity::Ptr & p : M_parameters )
    {
        std::ostringstream ostr;
        ostr << "  ";
        p->printFormat( ostr );

        width = std::max( width, ostr.str().length() );
    }

    ++width;

    const std::size_t max_len = 80;
    const std::string indent( width, ' ' );

    for ( const ParamEntity::Ptr & param : M_parameters )
    {
        std::ostringstream ostr;
        ostr << "  ";
        param->printFormat( ostr );
        os << ostr.str();

        const size_t padding = width - ostr.str().length();

        const std::string & desc = param->description();
        if ( ! desc.empty() )
        {
            for ( size_t p = padding; p > 0; --p )
            {
                os.put( ' ' );
            }

            if ( padding > max_len - 6 )
            {
                os << '\n';
            }

            // format description message
            std::string::size_type nl_pos = 0;
            for ( std::string::size_type pos = desc.find( ' ' );
                  pos != std::string::npos;
                  pos = desc.find( ' ', pos + 1 ) )
            {
                if ( pos - nl_pos + width > max_len - 6 )
                {
                    os << desc.substr( nl_pos, pos - nl_pos )
                       << '\n' << indent;
                    nl_pos = pos + 1;
                }
            }
            os << desc.substr( nl_pos );
        }

        if ( with_default )
        {
            if ( ! desc.empty() )
            {
                os << '\n' << indent;
            }
            else
            {
                for ( size_t p = padding; p > 0; --p )
                {
                    os.put( ' ' );
                }
            }

            if ( param->isSwitch() )
            {
                os << "(Switch Default: ";
            }
            else
            {
                os << "(Default: ";
            }

            param->printValue( os ) << ')';
        }

        os << '\n';
    }

    return os << std::flush;
}

/*-------------------------------------------------------------------*/
/*!

 */
std::ostream &
ParamMap::printValues( std::ostream & os ) const
{
    for ( const ParamEntity::Ptr & param : M_parameters )
    {
        os << param->longName() << '\t';
        param->printValue( os );
        os << '\n';
    }
    return os << std::flush;
}

}
