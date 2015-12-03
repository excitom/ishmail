/*
 * $Id: FolderPrefWinC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
 *
 * Copyright (c) 1994 HAL Computer Systems International, Ltd.
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

#ifndef _FolderPrefWinC_h_
#define _FolderPrefWinC_h_

#include "OptWinC.h"

class MimeRichTextC;

class FolderPrefWinC : public OptWinC {

protected:

   Widget		viewLargeTB;
   Widget		viewSmallTB;
   Widget		viewNameTB;
   Widget		sortTB;
   Widget		statusTB;

   Widget		useUnixTB;
   Widget		useMhTB;
   Widget		useMmdfTB;
   Widget		useNoneTB;
   Widget		defUnixTB;
   Widget		defMhTB;
   Widget		defMmdfTB;
   Widget		defImapTB;
   Widget		confirmTypeTB;

   MimeRichTextC	*initText;
   Widget		rememberTB;

//
// Callbacks
//
   static void		DoToggleLocalType(Widget, FolderPrefWinC*,
   					  XmToggleButtonCallbackStruct*);

//
// Private methods
//
   Boolean		Apply();
   void			Write();

public:

// Methods

   FolderPrefWinC(Widget);
   ~FolderPrefWinC();

   void			Show();
};

#endif // _FolderPrefWinC_h_
