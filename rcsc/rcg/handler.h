// -*-c++-*-

/*!
  \file handler.h
  \brief rcg data handler Header File.
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

#ifndef RCSC_RCG_HANDLER_H
#define RCSC_RCG_HANDLER_H

#include <rcsc/rcg/types.h>

#include <string>

namespace rcsc {
namespace rcg {

/*!
  \class Handler
  \brief abstract rcg data handler class.

  This is an interface class, all member methods are virtual.
*/
class Handler {
private:

    //! RCG version number(1-3, default:0)
    int M_log_version;

    //! last handled game time
    int M_read_time;

protected:

    /*!
      \brief default constructor. log version and time are set to zero.
    */
    Handler();

public:
    /*!
      \brief virtual destructor
    */
    virtual
    ~Handler();

    /*!
      \brief records rcg version
      \param ver log version.
      \return always true.

      This method is virtual.
      You can override this.
    */
    virtual
    bool handleLogVersion( const int ver )
      {
          M_log_version = ver;
          return true;
      }

    /*!
      \brief returns rcg version number
      \return rcg version number

      This method is virtual.
      You can override this.
    */
    virtual
    int logVersion() const
      {
          return M_log_version;
      }

    //
    // old version handers
    // all data are automatically converted to the intermediate format.
    //

    /*!
      \brief handle dispinfo_t.
      \param info new data
      \return result status
    */
    bool handleDispInfo( const dispinfo_t & info );

    /*!
      \brief handle dispinfo_t2.
      \param info new data
      \return result status
    */
    bool handleDispInfo2( const dispinfo_t2 & info );

    /*!
      \brief handle showinfo_t
      \param info handled data
      \return result status
    */
    bool handleShowInfo( const showinfo_t & info );

    /*!
      \brief handle showinfo_t2
      \param info handled data
      \return result status
    */
    bool handleShowInfo2( const showinfo_t2 & info );

    /*!
      \brief handle short_showinfo_t.
      \param info handled data
      \return result status
    */
    bool handleShortShowInfo2( const short_showinfo_t2 & info );

    /*!
      \brief handle msginfo_t
      \param board handled message type
      \param msg handled message data
      \return result status
    */
    bool handleMsgInfo( const Int16 board,
                        const std::string & msg );

    /*!
      \brief handle drawinfo_t
      \param draw new data
      \return result status
    */
    bool handleDrawInfo( const drawinfo_t & draw );

    /*!
      \brief handle playmode
      \param playmode handled playmode character
      \return result status
    */
    bool handlePlayMode( char playmode );

    /*!
      \brief handle team data
      \param team_l left team data
      \param team_r right team data
      \return result status
    */
    bool handleTeamInfo( const team_t & team_l,
                         const team_t & team_r );

    /*!
      \brief handle player_type_t
      \param type handled data
      \return result status
    */
    bool handlePlayerType( const player_type_t & type );

    /*!
      \brief handle server_params_t
      \param param handled data
      \return result status
    */
    bool handleServerParam( const server_params_t & param );

    /*!
      \brief handle player_params_t
      \param param handled data
      \return result status
    */
    bool handlePlayerParam( const player_params_t & param );

    /*!
      \brief (pure virtual) handle end of file
      \return result status
    */
    virtual
    bool handleEOF() = 0;


    //
    // version 4, 5
    //

    /*!
      \brief (pure virtual) handle the start of show info v4
      \param time game time of handled show info
      \param show read data
      \return result status
     */
    virtual
    bool handleShow( const ShowInfoT & show ) = 0;

    /*!
      \brief (pure virtual) handle msg info
      \param time game time of handled msg info
      \param board message board type
      \param msg new data
      \return result status
     */
    virtual
    bool handleMsg( const int time,
                    const int board,
                    const std::string & msg ) = 0;

    /*!
      \brief (pure virtual) handle draw info
      \param time game time of handled data
      \param draw new data
      \return result status
     */
    virtual
    bool handleDraw( const int time,
                     const drawinfo_t & draw ) = 0;

    /*!
      \brief handle playmode
      \param time game time of handled playmode info
      \param pm playmode id
      \return result status
     */
    virtual
    bool handlePlayMode( const int time,
                         const PlayMode pm ) = 0;

    /*!
      \brief handle team info
      \param time game time of handled team info
      \param team_l left team info
      \param team_r right team info
      \return result status
    */
    virtual
    bool handleTeam( const int time,
                     const TeamT & team_l,
                     const TeamT & team_r ) = 0;

    /*!
      \brief handle server_param message
      \param msg raw message string
      \return result status
    */
    virtual
    bool handleServerParam( const std::string & msg ) = 0;

    /*!
      \brief handle player_param message
      \param msg raw message string
      \return result status
    */
    virtual
    bool handlePlayerParam( const std::string & msg ) = 0;

    /*!
      \brief handle player_type message
      \param msg raw message string
      \return result status
    */
    virtual
    bool handlePlayerType( const std::string & msg ) = 0;

};


} // end of namespace
} // end of namespace

#endif
