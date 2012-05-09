// -*-c++-*-

/*!
  \file offline_client.h
  \brief offline soccer client class Header File.
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

#ifndef RCSC_COMMON_OFFLINE_CLIENT_H
#define RCSC_COMMON_OFFLINE_CLIENT_H

#include <rcsc/common/abstract_client.h>

#include <fstream>

namespace rcsc {

/*!
  \class OfflineClient
  \brief offline clientt class for debugging purpose.
 */
class OfflineClient
    : public AbstractClient {
private:

    //! input log file for offline client
    std::ifstream M_offline_in;

public:

    /*!
      \brief default constructor.
     */
    OfflineClient();

    /*!
      \brief destructor.
     */
    ~OfflineClient();

    /*!
      \brief program mainloop. This loop consumes computational resources as much as possible without wait.
      \param agent pointer to the soccer agent instance.
     */
    virtual
    void run( SoccerAgent * agent );

    /*!
      \brief do nothing.
      \param hostname dummy parameter.
      \param port dummy parameter.
      \return always true.
     */
    virtual
    bool connectTo( const char * hostname,
                    const int port );

    /*!
      \brief do nothing.
      \param msg not used.
      \return always 1.
     */
    virtual
    int sendMessage( const char * msg );

    /*!
      \brief read a recorded message from the offline log file.
      \return length of read message
     */
    virtual
    int receiveMessage();

    /*!
      \brief open the offline client log file.
      \param filepath file path string to be opened.
      \return result status.
     */
    virtual
    bool openOfflineLog( const std::string & filepath ) ;

    /*!
      \brief do nothing.
     */
    virtual
    void printOfflineThink();
};

}

#endif
