/*
 *  $Id: MhMsgC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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
#ifndef _MhMsgC_h_
#define _MhMsgC_h_

#include "MsgC.h"
#include <hgl/StringC.h>

#include <stdio.h>

class MhMsgC : public MsgC {

protected:

   int		openLevel;

   void		ScanHead();
   void		ScanBody();

public:

   StringC	filename;
   FILE		*fp;
   Boolean	deleteFile;

   MhMsgC(FolderC*, const char*, int num=0, Boolean del=False);
   ~MhMsgC();

   void		 SetFile(const char*, Boolean del=False);
   Boolean	 OpenFile();
   FILE		*OpenFile() const;
   void		 CloseFile(Boolean force=False);
   void		 CloseFile(FILE*) const;

//
// Info
//
   Boolean	GetPartText(const MsgPartC*, StringC&, Boolean getHead=False,
   			    Boolean getExtHead=False, Boolean getBody=True)
			    const;
   Boolean	GetPartText(const MsgPartC*, StringC&, Boolean getHead=False,
   			    Boolean getExtHead=False, Boolean getBody=True);
   Boolean	GetFileData(MsgPartC*, StringC&) const;
   Boolean	GetFileData(MsgPartC*, StringC&);
   Boolean	GetHeaderText(StringC&) const;
   Boolean	GetHeaderText(StringC&);

//
// Output
//
   Boolean	WriteBody(FILE*, Boolean addBlank, Boolean protectFroms);
};

#endif // _MhMsgC_h_
