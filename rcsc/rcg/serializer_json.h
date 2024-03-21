// -*-c++-*-

/*!
  \file serializer_json.h
  \brief json format rcg serializer class Header File.
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

#ifndef RCSC_RCG_SERIALIZER_JSON_H
#define RCSC_RCG_SERIALIZER_JSON_H

#include <rcsc/rcg/serializer.h>

namespace rcsc {
namespace rcg {

class SerializerJSON
    : public Serializer {
private:

    Int32 M_time; //!< temporal time holder
    Int32 M_stime; //!< temporal time holder

public:

    /*!
      \brief constructor.
     */
    SerializerJSON()
        : M_time( 0 ),
          M_stime( 0 )
      { }

    /*!
      \brief write header
      \param os reference to the output stream
      \return reference to the output stream
    */
    std::ostream & serializeHeader( std::ostream & os ) override;

    /*!
      \brief write the end of file
      \param os reference to the output stream
      \return reference to the output stream
     */
    std::ostream & serializeEnd( std::ostream & os ) override;

    /*!
      \brief write header
      \param os reference to the output stream
      \param param server_params_t variable by network byte order
      \return reference to the output stream
    */
    std::ostream & serialize( std::ostream & os,
                              const server_params_t & param ) override;

    /*!
      \brief write header
      \param os reference to the output stream
      \param pparam player_params_t variable by network byte order
      \return reference to the output stream
    */
    std::ostream & serialize( std::ostream & os,
                              const player_params_t & pparam ) override;

    /*!
      \brief write header
      \param os reference to the output stream
      \param type player_type_t variable by network byte order
      \return reference to the output stream
    */
    std::ostream & serialize( std::ostream & os,
                              const player_type_t & type ) override;

    /*!
      \brief write dispinfo_t.
      \param os reference to the output stream
      \param disp network byte order data
      \return reference to the output stream
     */
    std::ostream & serialize( std::ostream & os,
                              const dispinfo_t & disp ) override;

    /*!
      \brief write showinfo_t.
      \param os reference to the output stream
      \param show network byte order data
      \return reference to the output stream
     */
    std::ostream & serialize( std::ostream & os,
                              const showinfo_t & show ) override;

    /*!
      \brief write showinfo_t2.
      \param os reference to the output stream
      \param show2 network byte order data
      \return reference to the output stream
     */
    std::ostream & serialize( std::ostream & os,
                              const showinfo_t2 & show2 ) override;

    /*!
      \brief write short_showinfo_t2.
      \param os reference to the output stream
      \param show2 network byte order data
      \return reference to the output stream
     */
    std::ostream & serialize( std::ostream & os,
                              const short_showinfo_t2 & show2 ) override;

    /*!
      \brief write message info
      \param os reference to the output stream
      \param msg msginfo_t variable by network byte order
      \return reference to the output stream
    */
    std::ostream & serialize( std::ostream & os,
                              const msginfo_t & msg ) override;

    /*!
      \brief write message info
      \param os reference to the output stream
      \param board message board type
      \param msg message string
      \return reference to the output stream
    */
    std::ostream & serialize( std::ostream & os,
                              const Int16 board,
                              const std::string & msg ) override;

    /*!
      \brief write drawinfo_t
      \param os reference to the output stream
      \param draw drawinfo_t variable
      \return reference to the output stream
    */
    std::ostream & serialize( std::ostream & os,
                              const drawinfo_t & draw ) override;

    /*!
      \brief write playmode
      \param os reference to the output stream
      \param playmode play mode variable
      \return reference to the output stream
    */
    std::ostream & serialize( std::ostream & os,
                              const char playmode ) override;

    /*!
      \brief write team info
      \param os reference to the output stream
      \param team_l left team variable
      \param team_r right team variable
      \return reference to the output stream
    */
    std::ostream & serialize( std::ostream & os,
                              const team_t & team_l,
                              const team_t & team_r ) override;

    /*!
      \brief write team info
      \param os reference to the output stream
      \param team_l left team variable
      \param team_r right team variable
      \return reference to the output stream
    */
    std::ostream & serialize( std::ostream & os,
                              const TeamT & team_l,
                              const TeamT & team_r ) override;

    /*!
      \brief write ShowInfoT
      \param os reference to the output stream
      \param show data to be written
      \return reference to the output stream
     */
    std::ostream & serialize( std::ostream & os,
                              const ShowInfoT & show ) override;

    /*!
      \brief write DispInfoT
      \param os reference to the output stream
      \param disp data to be written
      \return reference to the output stream
     */
    std::ostream & serialize( std::ostream & os,
                              const DispInfoT & disp ) override;

    /*!
      \brief write ServerParamT
      \param os reference to the output stream
      \param param data to be written
      \return reference to the output stream
     */
    std::ostream & serialize( std::ostream & os,
                              const ServerParamT & param ) override;

    /*!
      \brief write PlayerParamT
      \param os reference to the output stream
      \param param data to be written
      \return reference to the output stream
     */
    std::ostream & serialize( std::ostream & os,
                              const PlayerParamT & param ) override;
    /*!
      \brief write PlayerTypeT
      \param os reference to the output stream
      \param param data to be written
      \return reference to the output stream
     */
    std::ostream & serialize( std::ostream & os,
                              const PlayerTypeT & param ) override;

    /*!
      \brief write team_graphic
      \param os output stream
      \param side team side
      \param x index of the xpm_tile
      \param y index of the xpm_tile
      \param xpm xpm tile
     */
    std::ostream & serialize( std::ostream & os,
                              const SideID side,
                              const int x,
                              const int y,
                              const std::vector< std::string > & xpm ) override;
};

}
}

#endif
