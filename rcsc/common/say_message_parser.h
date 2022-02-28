// -*-c++-*-

/*!
  \file say_message_parser.h
  \brief player's say message parser Header File
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

#ifndef RCSC_COMMON_SAY_MESSAGE_PARSER_H
#define RCSC_COMMON_SAY_MESSAGE_PARSER_H

#include <rcsc/types.h>

#include <memory>
#include <string>

namespace rcsc {

class AudioMemory;
class GameTime;

/*-------------------------------------------------------------------*/
/*!
  \class SayMessageParser
  \brief abstract player's say message parser
 */
class SayMessageParser {
public:

    //! pointer type alias
    typedef std::shared_ptr< SayMessageParser > Ptr;

private:

    // not used
    SayMessageParser( const SayMessageParser & ) = delete;
    SayMessageParser & operator=( const SayMessageParser & ) = delete;

protected:

    /*!
      \brief protected constructer
     */
    SayMessageParser() = default;

public:

    /*!
      \brief virtual destruct. do nothing.
     */
    virtual
    ~SayMessageParser() = default;

    /*!
      \brief pure virtual method that returns header character.
      \return header character.
     */
    virtual
    char header() const = 0;

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    virtual
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current ) = 0;

};

/*-------------------------------------------------------------------*/
/*!
  \class BallMessageParser
  \brief ball info message parser

  format:
  "b<pos_vel:5>"
  the length of message == 6
 */
class BallMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    std::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    BallMessageParser( std::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader() { return 'b'; }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const { return sheader(); }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength() { return 6; }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};

/*-------------------------------------------------------------------*/
/*!
  \class PassMessageParser
  \brief pass info message parser

  format:
  "p<unum_pos:4><pos_vel:5>"
  the length of message == 10
 */
class PassMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    std::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    PassMessageParser( std::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader() { return 'p'; }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const { return sheader(); }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength() { return 10; }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};

/*-------------------------------------------------------------------*/
/*!
  \class InterceptMessageParser
  \brief intercept info message parser

  format:
  "i<unum:1><cycle:1>"
  the length of message == 3  format:
 */
class InterceptMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    std::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    InterceptMessageParser( std::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader() { return 'i'; }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const { return sheader(); }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength() { return 3; }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};

/*-------------------------------------------------------------------*/
/*!
  \class GoalieMessageParser
  \brief opponent goalie info message parser

  format:
  "g<pos_body:4>"
  the length of message == 5
 */
class GoalieMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    std::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    GoalieMessageParser( std::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader() { return 'g'; }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const { return sheader(); }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength() { return 5; }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};


/*-------------------------------------------------------------------*/
/*!
  \class GoalieAndPlayerMessageParser
  \brief opponent goalie & field player message parser

  format:
  "g<pos_body:4,unum_pos:3>"
  the length of message == 8
 */
class GoalieAndPlayerMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    std::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    GoalieAndPlayerMessageParser( std::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader() { return 'e'; }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const { return sheader(); }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength() { return 8; }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};


/*-------------------------------------------------------------------*/
/*!
  \class OffsideLineMessageParser
  \brief offside line info message parser

  format:
  "o<x_rate:1>"
  the length of message == 2
 */
class OffsideLineMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    std::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    OffsideLineMessageParser( std::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader() { return 'o'; }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const { return sheader(); }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength() { return 2; }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};

/*-------------------------------------------------------------------*/
/*!
  \class DefenseLineMessageParser
  \brief offside line info message parser

  format:
  "d<x_rate:1>"
  the length of message == 2
 */
class DefenseLineMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    std::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    DefenseLineMessageParser( std::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader() { return 'd'; }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const { return sheader(); }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength() { return 2; }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};

/*-------------------------------------------------------------------*/
/*!
  \class WaitRequestMessageParser
  \brief wait request message parser

  format:
  "w"
  the length of message == 1
 */
class WaitRequestMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    std::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    WaitRequestMessageParser( std::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader() { return 'w'; }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const { return sheader(); }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength() { return 1; }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};

/*-------------------------------------------------------------------*/
/*!
  \class SetplayMessageParser
  \brief setplay information message parser

  format:
  "F<wait>"
  the length of message == 2
 */
class SetplayMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    std::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    SetplayMessageParser( std::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader() { return 'F'; }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const { return sheader(); }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength() { return 2; }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};

/*-------------------------------------------------------------------*/
/*!
  \class PassRequestMessageParser
  \brief pass request (hey pass) message parser

  format:
  "h<pos:3>"
  the length of message == 4
 */
class PassRequestMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    std::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    PassRequestMessageParser( std::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader() { return 'h'; }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const { return sheader(); }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength() { return 4; }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};

/*-------------------------------------------------------------------*/
/*!
  \class StaminaMessageParser
  \brief stamina rate value message parser

  format:
  "s<rate:1>"
  the length of message == 2
 */
class StaminaMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    std::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    StaminaMessageParser( std::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader() { return 's'; }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const { return sheader(); }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength() { return 2; }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};

/*-------------------------------------------------------------------*/
/*!
  \class RecoveryMessageParser
  \brief recovery rate value message parser

  format:
  "r<rate:1>"
  the length of message == 2
 */
class RecoveryMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    std::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    RecoveryMessageParser( std::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader() { return 'r'; }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const { return sheader(); }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength() { return 2; }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};

/*-------------------------------------------------------------------*/
/*!
  \class StaminaCapacityMessageParser
  \brief stamina capacity rate value message parser

  format:
  "c<rate:1>"
  the length of message == 2
 */
class StaminaCapacityMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    std::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    StaminaCapacityMessageParser( std::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader() { return 'c'; }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const { return sheader(); }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength() { return 2; }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};

/*-------------------------------------------------------------------*/
/*!
  \class DribbleMessageParser
  \brief dribble target point message parser

  format:
  "D<count_pos:3>"
  the length of message == 4
 */
class DribbleMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    std::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    DribbleMessageParser( std::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader() { return 'D'; }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const { return sheader(); }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength() { return 4; }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};

/*-------------------------------------------------------------------*/
/*!
  \class BallGoalieMessageParser
  \brief ball & goalie info message parser

  format:
  "G<bpos_bvel_gpos_gbody:9>"
  the length of message == 10
 */
class BallGoalieMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    std::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    BallGoalieMessageParser( std::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader() { return 'G'; }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const { return sheader(); }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength() { return 10; }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};

/*-------------------------------------------------------------------*/
/*!
  \class OnePlayerMessageParser
  \brief player info message parser

  format:
  "P<unum_pos:3>"
  the length of message == 4
 */
class OnePlayerMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    std::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    OnePlayerMessageParser( std::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader() { return 'P'; }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const { return sheader(); }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength() { return 4; }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};

/*-------------------------------------------------------------------*/
/*!
  \class TwoPlayerMessageParser
  \brief 2 players info message parser

  format:
  "Q<unum_pos:3,unum_pos:3>"
  the length of message == 7
 */
class TwoPlayerMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    std::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    TwoPlayerMessageParser( std::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader() { return 'Q'; }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const { return sheader(); }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength() { return 7; }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};

/*-------------------------------------------------------------------*/
/*!
  \class ThreePlayerMessageParser
  \brief 3 players info message parser

  format:
  "R<unum_pos:3,unum_pos:3,unum_pos:3>"
  the length of message == 10
 */
class ThreePlayerMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    std::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    ThreePlayerMessageParser( std::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader() { return 'R'; }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const { return sheader(); }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength() { return 10; }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};

/*-------------------------------------------------------------------*/
/*!
  \class SelfMessageParser
  \brief self info message parser

  format:
  "S<pos_body_stamina>"
  the length of message == 5
 */
class SelfMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    std::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    SelfMessageParser( std::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader() { return 'S'; }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const { return sheader(); }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength() { return 5; }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};

/*-------------------------------------------------------------------*/
/*!
  \class TeammateMessageParser
  \brief teammate info message parser

  format:
  "T<unum_pos_body>"
  the length of message == 5
 */
class TeammateMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    std::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    TeammateMessageParser( std::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader() { return 'T'; }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const { return sheader(); }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength() { return 5; }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};

/*-------------------------------------------------------------------*/
/*!
  \class OpponentMessageParser
  \brief opponent info message parser

  format:
  "O<unum_pos_body>"
  the length of message == 5
 */
class OpponentMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    std::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    OpponentMessageParser( std::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader() { return 'O'; }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const { return sheader(); }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength() { return 5; }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};

/*-------------------------------------------------------------------*/
/*!
  \class BallPlayerMessageParser
  \brief ball & player info message parser

  format:
  "B<bpos_bvel_unum_ppos_pbody:9>"
  the length of message == 10
 */
class BallPlayerMessageParser
    : public SayMessageParser {
private:

    //! pointer to the audio memory
    std::shared_ptr< AudioMemory > M_memory;

public:

    /*!
      \brief construct with audio memory
      \param memory pointer to the memory
     */
    explicit
    BallPlayerMessageParser( std::shared_ptr< AudioMemory > memory );

    /*!
      \brief get the header character.
      \return header character.
     */
    static
    char sheader() { return 'B'; }

    /*!
      \brief get the header character.
      \return header character.
     */
    char header() const { return sheader(); }

    /*!
      \brief get the length of this message.
      \return the length of encoded message
    */
    static
    int slength() { return 10; }

    /*!
      \brief virtual method which analyzes audio messages.
      \param sender sender's uniform number
      \param dir sender's direction
      \param msg raw audio message
      \param current current game time
      \retval bytes read if success
      \retval 0 message ID is not match. other parser should be tried.
      \retval -1 failed to parse
    */
    int parse( const int sender,
               const double & dir,
               const char * msg,
               const GameTime & current );

};

}

#endif
