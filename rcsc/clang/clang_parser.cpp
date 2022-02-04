// -*-c++-*-

/*!
  \file clang_parser.cpp
  \brief clang parser class Source File
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

// #define BOOST_SPIRIT_DEBUG

#include "clang_parser.h"

#include "clang_action.h"
#include "clang_condition.h"
#include "clang_directive.h"
#include "clang_info_message.h"
#include "clang_token.h"
#include "clang_unum.h"

#include <boost/spirit/include/classic.hpp>
//#include <boost/bind.hpp>

#include <functional>
#include <stack>
#include <utility>
#include <iostream>
#include <cstring>
#include <cassert>

namespace bsc = BOOST_SPIRIT_CLASSIC_NS;

namespace rcsc {

namespace {

/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
struct Item {

    enum Type {
        NONE,
        TOKEN,
        DIRECTIVE,
        CONDITION,
        ACTION,
        UNUM_SET,
        UNUM,
        TTL,
        HETERO_TYPE,
        POSITIVE,
        TEAM_OUR,
        STRING,
        VARIABLE,
    };

    union Value {
        CLangToken * token_;
        CLangDirective * directive_;
        CLangCondition * condition_;
        CLangAction * action_;
        CLangUnumSet * unum_set_;
        int int_value_;
        bool bool_value_;
        std::string * string_value_;
    };

    Type type_;
    Value value_;

    Item()
        : type_( NONE )
      {
          std::memset( &value_, 0, sizeof( Value ) );
      }
    Item( CLangToken * token )
        : type_( TOKEN )
      {
          value_.token_ = token;
      }
    Item( CLangDirective * dir )
        : type_( DIRECTIVE )
      {
          value_.directive_ = dir;
      }
    Item( CLangCondition * cond )
        : type_( CONDITION )
      {
          value_.condition_ = cond;
      }
    Item( CLangAction * act )
        : type_( ACTION )
      {
          value_.action_ = act;
      }
    Item( CLangUnumSet * uset )
        : type_( UNUM_SET )
      {
          value_.unum_set_ = uset;
      }
    Item( Type type,
          int value )
        : type_( type )
      {
          value_.int_value_ = value;
      }
    Item( Type type,
          bool value )
        : type_( type )
      {
          value_.bool_value_ = value;
      }

    Item( Type type,
          std::string * str )
        : type_( type )
      {
          value_.string_value_ = str;
      }

    void clear()
      {
          switch ( type_ ) {
          case TOKEN:
              delete value_.token_;
              value_.token_ = nullptr;
              break;
          case DIRECTIVE:
              delete value_.directive_;
              value_.directive_ = nullptr;
              break;
          case CONDITION:
              delete value_.condition_;
              value_.condition_ = nullptr;
              break;
          case ACTION:
              delete value_.action_;
              value_.action_ = nullptr;
              break;
          case UNUM_SET:
              delete value_.unum_set_;
              value_.unum_set_ = nullptr;
              break;
          case UNUM:
          case TTL:
          case HETERO_TYPE:
              value_.int_value_ = 0;
              break;
          case POSITIVE:
          case TEAM_OUR:
              value_.bool_value_ = false;
              break;
          case STRING:
          case VARIABLE:
              delete value_.string_value_;
              value_.string_value_ = nullptr;
              break;
          default:
              std::cerr << __FILE__ << ' ' << __LINE__
                        << ": unknown item type " << type_ << std::endl;
              break;
          }

          type_ = NONE;
      }
};

}

/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
/*-------------------------------------------------------------------*/
class CLangParser::Impl
    : public bsc::grammar< CLangParser::Impl > {
private:

    CLangParser & M_parser;

    std::stack< Item > M_item_stack;

public:

    Impl( CLangParser & parser )
        : M_parser( parser )
      { }

    ~Impl()
      {
          clear();
      }

    void clear()
      {
          while ( ! M_item_stack.empty() )
          {
              //std::cerr << "clear stack item" << std::endl;
              M_item_stack.top().clear();
              M_item_stack.pop();
          }
      }

    bool stackIsEmpty() const
      {
          return M_item_stack.empty();
      }


    template < typename S >
    struct definition {

        bsc::rule< S > lp_p;
        bsc::rule< S > rp_p;

        bsc::rule< S > lb_p;
        bsc::rule< S > rb_p;

        bsc::rule< S > clang_true_p;
        bsc::rule< S > clang_false_p;

        bsc::rule< S > clang_raw_str_p;
        bsc::rule< S > clang_str_p;
        bsc::rule< S > clang_var_p;

        bsc::rule< S > clang_unum_p;
        bsc::rule< S > clang_unum_set_p;

        bsc::rule< S > clang_cond_bool_p;
        bsc::rule< S > clang_cond_p;

        bsc::rule< S > clang_act_mark_p;
        bsc::rule< S > clang_act_htype_p;
        bsc::rule< S > clang_act_hold_p;
        bsc::rule< S > clang_act_bto_p;

        bsc::rule< S > clang_act_p;

        bsc::rule< S > clang_do_dont_p;

        bsc::rule< S > clang_team_p;

        bsc::rule< S > clang_directive_p;
        bsc::rule< S > clang_token_p;

        bsc::rule< S > clang_info_msg_p;

        bsc::rule< S > clang_msg_p;

        definition( const Impl & self )
          {
              using std::placeholders::_1;
              using std::placeholders::_2;

              lp_p = bsc::ch_p( '(' );
              rp_p = bsc::ch_p( ')' );

              lb_p = bsc::ch_p( '{' );
              rb_p = bsc::ch_p( '}' );

              clang_true_p = bsc::str_p( "true" );
              clang_false_p = bsc::str_p( "false" );

              // str: \"[0-9A-Za-z\(\)\.\+\-\*\/\?\<\>\_ ]+\"
              // var: [abe-oqrt-zA-Z_][a-zA-Z0-9_]*

              clang_raw_str_p = *( bsc::alnum_p | bsc::chset_p( "().+-*/?<>_ " ) );
              clang_str_p = ( bsc::ch_p( '"' )
                              >> clang_raw_str_p
                              >> bsc::ch_p( '"' )
                              )[std::bind( &Impl::handleString, &self, _1, _2 )];
              clang_var_p = ( ( bsc::chset_p( "abefghijklmnoqrtuvwxyz" )
                                | bsc::upper_p
                                | bsc::ch_p( '_' ) )
                              >> +( bsc::alnum_p | bsc::ch_p( '_' ) )
                              )[std::bind( &Impl::handleVariable, &self, _1, _2 )];

              clang_unum_p = bsc::uint_p[std::bind( &Impl::handleUnum, &self, _1 )];
              clang_unum_set_p = ( lb_p >> *clang_unum_p >> rb_p
                                   )[std::bind( &Impl::handleUnumSet, &self )];

              clang_cond_bool_p
                  = ( lp_p >> clang_true_p >> rp_p )[std::bind( &Impl::handleConditionBool, &self, true )]
                  | ( lp_p >> clang_false_p >> rp_p )[std::bind( &Impl::handleConditionBool, &self, false )];
              clang_cond_p = clang_cond_bool_p;

              clang_act_mark_p = ( lp_p
                                   >> bsc::str_p( "mark" ) >> clang_unum_set_p
                                   >> rp_p );
              clang_act_htype_p = ( lp_p
                                    >> bsc::str_p( "htype" )
                                    >> bsc::int_p[std::bind( &Impl::handleActHeteroTypeId, &self, _1 )]
                                    >> rp_p );
              clang_act_hold_p = ( lp_p
                                   >> bsc::str_p( "hold" )
                                   >> rp_p );
              clang_act_bto_p = ( lp_p
                                  >> bsc::str_p( "bto" ) >> clang_unum_set_p
                                  >> rp_p );
              clang_act_p = ( clang_act_mark_p[std::bind( &Impl::handleActMark, &self )]
                              | clang_act_htype_p[std::bind( &Impl::handleActHeteroType, &self )]
                              | clang_act_hold_p[std::bind( &Impl::handleActHold, &self )]
                              | clang_act_bto_p[std::bind( &Impl::handleActBallTo, &self )]
                              );

              clang_do_dont_p
                  = bsc::str_p( "dont" )[std::bind( &Impl::handlePositive, &self, false )]
                  | bsc::str_p( "do" )[std::bind( &Impl::handlePositive, &self, true )];

              clang_team_p
                  = bsc::str_p( "our" )[std::bind( &Impl::handleTeam, &self, true )]
                  | bsc::str_p( "opp" )[std::bind( &Impl::handleTeam, &self, false )];

              clang_directive_p = ( ( lp_p
                                      >> clang_do_dont_p
                                      >> clang_team_p
                                      >> clang_unum_set_p
                                      >> *clang_act_p
                                      >> rp_p )[std::bind( &Impl::handleDirectiveCommon, &self )]
                                    | clang_str_p[std::bind( &Impl::handleDirectiveNamed, &self )]
                                    );

              clang_token_p = ( ( lp_p
                                  >> bsc::int_p[std::bind( &Impl::handleTokenTTL, &self, _1 )]
                                  >> clang_cond_p //[std::bind( &Impl::handleCondition, &self )]
                                  >> *clang_directive_p
                                  >> rp_p
                                  )[std::bind( &Impl::handleTokenRule, &self )]
                                | ( lp_p >> bsc::str_p( "clear" ) >> rp_p
                                    )[std::bind( &Impl::handleTokenClear, &self )]
                                );

              clang_info_msg_p = ( lp_p
                                   >> bsc::str_p( "info" )
                                   >> *clang_token_p
                                   >> rp_p
                                   )[std::bind( &Impl::handleInfoMessage, &self )];

              clang_msg_p = clang_info_msg_p;

#ifdef BOOST_SPIRIT_DEBUG
              std::cerr << "boost spirit debug" << std::endl;
              BOOST_SPIRIT_DEBUG_RULE( clang_true_p );
              BOOST_SPIRIT_DEBUG_RULE( clang_false_p );
              BOOST_SPIRIT_DEBUG_RULE( clang_raw_str_p );
              BOOST_SPIRIT_DEBUG_RULE( clang_str_p );
              BOOST_SPIRIT_DEBUG_RULE( clang_var_p );
              BOOST_SPIRIT_DEBUG_RULE( clang_unum_p );
              BOOST_SPIRIT_DEBUG_RULE( clang_unum_set_p );
              BOOST_SPIRIT_DEBUG_RULE( clang_cond_bool_p );
              BOOST_SPIRIT_DEBUG_RULE( clang_cond_p );
              BOOST_SPIRIT_DEBUG_RULE( clang_act_mark_p );
              BOOST_SPIRIT_DEBUG_RULE( clang_act_htype_p );
              BOOST_SPIRIT_DEBUG_RULE( clang_act_hold_p );
              BOOST_SPIRIT_DEBUG_RULE( clang_act_bto_p );
              BOOST_SPIRIT_DEBUG_RULE( clang_act_p );
              BOOST_SPIRIT_DEBUG_RULE( clang_do_dont_p );
              BOOST_SPIRIT_DEBUG_RULE( clang_team_p );
              BOOST_SPIRIT_DEBUG_RULE( clang_directive_p );
              BOOST_SPIRIT_DEBUG_RULE( clang_token_p );
              BOOST_SPIRIT_DEBUG_RULE( clang_info_msg_p );
              BOOST_SPIRIT_DEBUG_RULE( clang_msg_p );
#endif
          }


        const bsc::rule< S > & start() const
          {
              return clang_msg_p;
          }
    };


    bool handleString( const char * first,
                       const char * last ) const
      {
          return M_parser.handleString( first, last );
      }

    bool handleVariable( const char * first,
                         const char * last ) const
      {
          return M_parser.handleVariable( first, last );
      }

    bool handleUnum( unsigned int value ) const
      {
          return M_parser.handleUnum( value );
      }

    bool handleUnumSet() const
      {
          return M_parser.handleUnumSet();
      }

    bool handleConditionBool( bool value ) const
      {
          //std::cerr << "handleConditionBool " << value << std::endl;
          return M_parser.handleConditionBool( value );
      }

    bool handlePositive( bool on ) const
      {
          //std::cerr << "handlePositive " << on << std::endl;
          return M_parser.handlePositive( on );
      }

    bool handleTeam( bool our ) const
      {
          //std::cerr << "handleTeam " << our << std::endl;
          return M_parser.handleTeam( our );
      }

    bool handleActMark() const
      {
          return M_parser.handleActMark();
      }

    bool handleActHeteroType() const
      {
          return M_parser.handleActHeteroType();
      }

    bool handleActHeteroTypeId( int type ) const
      {
          return M_parser.handleActHeteroTypeId( type );
      }

    bool handleActHold() const
      {
          return M_parser.handleActHold();
      }

    bool handleActBallTo() const
      {
          return M_parser.handleActBallTo();
      }

    bool handleDirectiveCommon() const
      {
          return M_parser.handleDirectiveCommon();
      }

    bool handleDirectiveNamed() const
      {
          return M_parser.handleDirectiveNamed();
      }

    bool handleTokenTTL( int ttl ) const
      {
          return M_parser.handleTokenTTL( ttl );
      }

    bool handleTokenRule() const
      {
          return M_parser.handleTokenRule();
      }

    bool handleTokenClear() const
      {
          return M_parser.handleTokenClear();
      }

    bool handleInfoMessage() const
      {
          return M_parser.handleInfoMessage();
      }

    void pushToken( CLangToken * tok )
      {
          M_item_stack.push( Item( tok ) );
      }

    CLangToken * popToken()
      {
          if ( M_item_stack.empty()
               || M_item_stack.top().type_ != Item::TOKEN )
          {
              return nullptr;
          }

          CLangToken * tok = M_item_stack.top().value_.token_;
          M_item_stack.pop();
          return tok;
      }

    void pushDirective( CLangDirective * dir )
      {
          M_item_stack.push( Item( dir ) );
      }

    CLangDirective * popDirective()
      {
          if ( M_item_stack.empty()
               || M_item_stack.top().type_ != Item::DIRECTIVE )
          {
              return nullptr;
          }

          CLangDirective * dir = M_item_stack.top().value_.directive_;
          M_item_stack.pop();
          return dir;
      }

    void pushCondition( CLangCondition * cond )
      {
          M_item_stack.push( Item( cond ) );
      }

    CLangCondition * popCondition()
      {
          if ( M_item_stack.empty()
               || M_item_stack.top().type_ != Item::CONDITION )
          {
              return nullptr;
          }

          CLangCondition * cond = M_item_stack.top().value_.condition_;
          M_item_stack.pop();
          return cond;
      }

    void pushAction( CLangAction * act )
      {
          M_item_stack.push( Item( act ) );
      }

    CLangAction * popAction()
      {
          if ( M_item_stack.empty()
               || M_item_stack.top().type_ != Item::ACTION )
          {
              return nullptr;
          }

          CLangAction * act = M_item_stack.top().value_.action_;
          M_item_stack.pop();
          return act;
      }

    void pushUnumSet( CLangUnumSet * uset )
      {
          M_item_stack.push( Item( uset ) );
      }

    CLangUnumSet * popUnumSet()
      {
          if ( M_item_stack.empty()
               || M_item_stack.top().type_ != Item::UNUM_SET )
          {
              return nullptr;
          }

          CLangUnumSet * uset = M_item_stack.top().value_.unum_set_;
          M_item_stack.pop();
          return uset;
      }

    void pushIntValue( Item::Type type,
                       int value )
      {
          M_item_stack.push( Item( type, value ) );
      }

    bool popIntValue( Item::Type type,
                      int * rval )
      {
          if ( M_item_stack.empty()
               || M_item_stack.top().type_ != type )
          {
              return false;
          }

          *rval = M_item_stack.top().value_.int_value_;
          M_item_stack.pop();
          return true;
      }

    void pushBoolValue( Item::Type type,
                        bool value )
      {
          M_item_stack.push( Item( type, value ) );
      }

    bool popBoolValue( Item::Type type,
                       bool * rval )
      {
          if ( M_item_stack.empty()
               || M_item_stack.top().type_ != type )
          {
              return false;
          }

          *rval = M_item_stack.top().value_.bool_value_;
          M_item_stack.pop();
          return true;
      }

    void pushStringValue( Item::Type type,
                          std::string * str )
      {
          M_item_stack.push( Item( type, str ) );
      }

    std::string * popStringValue( Item::Type type )
      {
          if ( M_item_stack.empty()
               || M_item_stack.top().type_ != type )
          {
              return nullptr;
          }

          std::string * str = M_item_stack.top().value_.string_value_;
          M_item_stack.pop();
          return str;
      }
};

/*-------------------------------------------------------------------*/
/*!

 */
CLangParser::CLangParser()
    : M_impl( new Impl( *this ) )
{

}

/*-------------------------------------------------------------------*/
/*!

 */
CLangParser::~CLangParser()
{

}

/*-------------------------------------------------------------------*/
/*!

 */
void
CLangParser::clear()
{
    M_impl->clear();
    M_message.reset();
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CLangParser::parse( const std::string & msg )
{
    clear();

    bsc::parse_info<> info = bsc::parse( msg.c_str(), *M_impl, bsc::space_p );

    return info.full;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CLangParser::handleString( const char * first,
                           const char * last )
{
    M_impl->pushStringValue( Item::STRING,
                             new std::string( first + 1, last - 1 ) );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CLangParser::handleVariable( const char * first,
                             const char * last )
{
    M_impl->pushStringValue( Item::VARIABLE,
                             new std::string( first, last ) );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CLangParser::handleUnum( const int unum )
{
    M_impl->pushIntValue( Item::UNUM, unum );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CLangParser::handleUnumSet()
{
    CLangUnumSet * uset = new CLangUnumSet();

    int unum = -1;
    while ( M_impl->popIntValue( Item::UNUM, &unum ) )
    {
        uset->add( unum );
    }

    M_impl->pushUnumSet( uset );

    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CLangParser::handleConditionBool( bool value )
{
    M_impl->pushCondition( new CLangConditionBool( value ) );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CLangParser::handlePositive( bool on )
{
    M_impl->pushBoolValue( Item::POSITIVE, on );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CLangParser::handleTeam( bool our )
{
    //std::cout << "handleTeam " << ( our ? "our" : "opp" ) << std::endl;
    M_impl->pushBoolValue( Item::TEAM_OUR, our );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CLangParser::handleActMark()
{
    CLangUnumSet * uset = M_impl->popUnumSet();
    if ( ! uset )
    {
        std::cerr << __FILE__ << ' ' << __LINE__
                  << ": (handleActMark) could not get unum set from the stack."
                  << std::endl;
        return false;
    }

    CLangActionMark * act = new CLangActionMark( uset );
    M_impl->pushAction( act );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CLangParser::handleActHeteroTypeId( const int type )
{
    M_impl->pushIntValue( Item::HETERO_TYPE, type );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CLangParser::handleActHeteroType()
{
    int type = -1000;
    if ( ! M_impl->popIntValue( Item::HETERO_TYPE, &type ) )
    {
        std::cerr << __FILE__ << ' ' << __LINE__
                  << ": (handleActHeteroType) could not get hetero type from the stack."
                  << std::endl;
        return false;
    }

    CLangActionHeteroType * act = new CLangActionHeteroType( type );
    M_impl->pushAction( act );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CLangParser::handleActHold()
{
    CLangActionHold * act = new CLangActionHold();
    M_impl->pushAction( act );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CLangParser::handleActBallTo()
{
    CLangUnumSet * uset = M_impl->popUnumSet();
    if ( ! uset )
    {
        std::cerr << __FILE__ << ' ' << __LINE__
                  << ": (handleActMark) could not get unum set from the stack."
                  << std::endl;
        return false;
    }

    CLangActionBallTo * act = new CLangActionBallTo( uset );
    M_impl->pushAction( act );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CLangParser::handleDirectiveCommon()
{
    CLangDirectiveCommon * dir = new CLangDirectiveCommon();

    // actions
    while ( CLangAction * act = M_impl->popAction() )
    {
        dir->addAction( act );
    }

    if ( dir->actions().empty() )
    {
        std::cerr << __FILE__ << ' ' << __LINE__
                  << ": (handleDirectiveCommon) empty action."
                  << std::endl;
        delete dir;
        return false;
    }

    // unum_set
    CLangUnumSet * uset = M_impl->popUnumSet();
    if ( ! uset )
    {
        std::cerr << __FILE__ << ' ' << __LINE__
                  << ": (handleDirectiveCommon) could not get unum set."
                  << std::endl;
        delete dir;
        return false;
    }
    dir->setPlayers( uset );

    // team
    bool our = false;
    if ( ! M_impl->popBoolValue( Item::TEAM_OUR, &our ) )
    {
        std::cerr << __FILE__ << ' ' << __LINE__
                  << ": (handleDirectiveCommon) could not get team."
                  << std::endl;
        delete dir;
        return false;
    }
    dir->setOur( our );

    // do,dont
    bool positive = false;
    if ( ! M_impl->popBoolValue( Item::POSITIVE, &positive ) )
    {
        std::cerr << __FILE__ << ' ' << __LINE__
                  << ": (handleDirectiveCommon) could not get do_dont."
                  << std::endl;
        delete dir;
        return false;
    }
    dir->setPositive( positive );

    M_impl->pushDirective( dir );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CLangParser::handleDirectiveNamed()
{
    std::cout << "handleDirectiveNamed " << std::endl;
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CLangParser::handleTokenTTL( int ttl )
{
    M_impl->pushIntValue( Item::TTL, ttl );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CLangParser::handleTokenRule()
{
    CLangTokenRule * tok = new CLangTokenRule();

    // directives
    while ( CLangDirective * dir = M_impl->popDirective() )
    {
        tok->addDirective( dir );
    }

    if ( tok->directives().empty() )
    {
        std::cerr << __FILE__ << ' ' << __LINE__
                  << ": (handleTokenRule) empty directive." << std::endl;
        delete tok;
        return false;
    }

    // condition
    CLangCondition * cond = M_impl->popCondition();
    if ( ! cond )
    {
        std::cerr << __FILE__ << ' ' << __LINE__
                  << ": (handleTokenRule) could not get the condition."
                  << std::endl;
        delete tok;
        return false;
    }
    tok->setCondition( cond );

    // ttl
    int ttl = -1;
    if ( ! M_impl->popIntValue( Item::TTL, &ttl ) )
    {
        delete tok;
        return false;
    }
    tok->setTTL( ttl );

    M_impl->pushToken( tok );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CLangParser::handleTokenClear()
{
    CLangTokenClear * tok = new CLangTokenClear();

    M_impl->pushToken( tok );
    return true;
}

/*-------------------------------------------------------------------*/
/*!

 */
bool
CLangParser::handleInfoMessage()
{
    CLangInfoMessage * info = new CLangInfoMessage();

    //std::cerr << "handleInfoMessage" << std::endl;
    while ( CLangToken * tok = M_impl->popToken() )
    {
        info->addToken( tok );
    }

    M_message = CLangMessage::ConstPtr( info );

    if ( ! M_impl->stackIsEmpty() )
    {
        std::cerr << __FILE__ << ' ' << __LINE__
                  << ": (handleInfoMessage) ERROR stack is not empty."
                  << std::endl;
        return false;
    }

    return true;
}

}
