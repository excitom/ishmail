/*
 *  $Id: ImapMsgC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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
#ifndef _ImapMsgC_h_
#define _ImapMsgC_h_

#include "MsgC.h"

class ImapFolderC;
class ImapServerC;

class ImapMsgC : public MsgC {

protected:

   ImapServerC	*server;
   ImapFolderC  *folder;

   void		GetExternalInfo(MsgPartC*);
   void		ParseFlags(CharC);
   void		ScanHead();
   void		ScanBody();
   char		*ScanAddress    (char*, MsgPartC*);
   char		*ScanAddressList(char*, MsgPartC*);
   char		*ScanEnvelope   (char*, MsgPartC*);
   char		*ScanNum        (char*, CharC&);
   char		*ScanParams     (char*, StringC&);
   char		*ScanPart       (char*, MsgPartC*);
   char		*ScanText       (char*, CharC&);

public:

   ImapMsgC(ImapFolderC*, ImapServerC*, int num);
   ~ImapMsgC();

//
// Info
//
   int		BodyLines();
   void		ClearStatus(MsgStatusT, Boolean write=True);
   void		SetStatus(MsgStatusT, Boolean write=True);

   Boolean	GetPartText(const MsgPartC*, StringC&, Boolean getHead=False,
   			    Boolean getExtHead=False, Boolean getBody=True)
			    const;
   Boolean	GetPartText(const MsgPartC*, StringC&, Boolean getHead=False,
   			    Boolean getExtHead=False, Boolean getBody=True);
   Boolean	GetFileData(MsgPartC*, StringC&) const;
   Boolean	GetFileData(MsgPartC*, StringC&);

   Boolean	GetBodyText(StringC& text) const;
   Boolean	GetBodyText(StringC& text);
   Boolean	GetHeaderText(StringC&) const;
   Boolean	GetHeaderText(StringC&);

//
// Output
//
   Boolean	WriteBody(FILE*, Boolean addBlank, Boolean protectFroms);
};

#endif // _ImapMsgC_h_
