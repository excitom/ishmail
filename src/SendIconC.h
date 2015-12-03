/*
 *  $Id: SendIconC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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
#ifndef _SendIconC_h_
#define _SendIconC_h_

#include "MimeIconC.h"

class IncludeWinC;
class MsgC;
class MsgPartC;
class SendWinC;
class StringListC;

class SendIconC : public MimeIconC {

//
// Private Data
//
   SendWinC		*sendWin;
   Pixel		dropColor;

   Boolean		BuildPartFile(FILE*, IncludeWinC*);
   Boolean		BuildPartFile(FILE*, char*, char*, char*);
   Boolean		BuildPartFile(FILE*, MsgC*);
   Boolean		BuildPartFile(FILE*, MsgPartC*);
   void			CalcBodySize(FILE *fp=NULL);
   FILE			*CreatePart();
   Boolean		GetPartText(MsgPartC*, StringC&);

public:

//
// Public data
//
   MsgPartC		*data;
   StringC		popupLabel;	// For popup menu

//
// Public Methods
//
   SendIconC(SendWinC*, IncludeWinC*);
   SendIconC(SendWinC*, char *type, char *ifile=NULL, char *ofile=NULL);
   SendIconC(SendWinC*, MsgC*);
   SendIconC(SendWinC*, MsgPartC*);
   ~SendIconC();

   void			Update(IncludeWinC*);
   void			InitIcon();
   Boolean		GetText(StringC&);
   Boolean		Write(FILE *fp, StringListC *headList=NULL);
   Boolean		SetSourceFile(char*, Boolean);
};

#endif // _SendIconC_h_
