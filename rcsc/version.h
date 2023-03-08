// -*-c++-*-

/*!
  \file version.h
  \brief version number Header File
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

#ifndef RCSC_VERSION_H
#define RCSC_VERSION_H

namespace rcsc {

//! supported client protocol version
constexpr double MAX_PROTOCOL_VERSION = 18.0;

/*-------------------------------------------------------------------*/
/*!
  \brief get the copyright message.
  \return copyright message.
 */
const char *
copyright();

/*-------------------------------------------------------------------*/
/*!
  \brief get the package version number string.
  \return package version number string.
 */
const char *
version();

}

#endif
