/*
 *  $Id: ImapFolderC.h,v 1.3 2000/12/13 17:34:40 evgeny Exp $
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
#ifndef _ImapFolderC_h_
#define _ImapFolderC_h_

#include "ImapServerC.h"
#include "FolderC.h"

class ImapServerC;

class ImapFolderC : public FolderC {

   StringC		serverName;

public:

   ImapServerC		*server;

   ImapFolderC(const char*, Boolean create=True);
   ~ImapFolderC();

   Boolean		fetchNeeded;
   int			msgCount;
   StringC		fetchTag;
   Boolean		AddMessage(MsgC*);
   Boolean		AddMessage(StringListC&, char*);
   Boolean		NewMail();
   Boolean		Save();
   Boolean		Scan();
   Boolean		Select();
};

#endif // _ImapFolderC_h_
