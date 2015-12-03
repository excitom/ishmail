/*
 *  $Id: MsgStatus.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
 *  
 *  Copyright (c) 1994 HAL Computer Systems International, Ltd.
 * 
 *          HAL COMPUTER SYSTEMS INTERNATIONAL, LTD.
 *                  1315 Dell Avenue
 *                  Campbell, CA  95008
 *
 * Author: Greg Hilton
 * Contributors: Tom Lang, Frank Bieser, and others
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * http://www.gnu.org/copyleft/gpl.html
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef _MsgStatus_h_
#define _MsgStatus_h_

enum MsgStatusT {

//
// User level status flags
//
   MSG_NEW		=	(1<< 0),
   MSG_READ		=	(1<< 1),
   MSG_DELETED		=	(1<< 2),
   MSG_SAVED		=	(1<< 3),
   MSG_REPLIED		=	(1<< 4),
   MSG_FORWARDED	=	(1<< 5),
   MSG_RESENT		=	(1<< 6),
   MSG_PRINTED		=	(1<< 7),
   MSG_FILTERED		=	(1<< 8),
   MSG_FLAGGED		=	(1<< 9),

//
// Internal status flags
//
   MSG_MIME		=	(1<<16),
   MSG_SUN_ATTACH	=	(1<<17),
   MSG_OPEN_FROMS	=	(1<<18), // Body contains "From "
   MSG_SAFE_FROMS	=	(1<<19), // Body contains ">From "
   MSG_FROMS_CHECKED	=	(1<<20),
   MSG_VIEWED 		=	(1<<21), // Being read
   MSG_IN_USE		=	(1<<22), // Being read, resent, replied-to or
   					 // Forwarded
   MSG_PARTIAL		=	(1<<23),
   MSG_CHANGED		=	(1<<24),
   MSG_ANNOUNCED	=	(1<<25)
};

//
// Format of the status line in the index file:
//    hex-status bytes-in-body lines-in-body message-id
//
#define MSG_INDEX_WFMT	"%08x %08x %08x %s\n"
#define MSG_INDEX_RFMT	"%08x %08x %08x %[^\n]\n"

#endif // _MsgStatus_h_
