/*
 *  $Id: ReplyPrefC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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
#ifndef _ReplyPrefC_h
#define _ReplyPrefC_h

#include "PrefC.h"

class ReplyPrefWinC;

class ReplyPrefC : public PrefC {

   ReplyPrefWinC	*prefWin;
   void			UpdateVarPat(StringC&);

public:

//
// Public data
//
   Boolean		removeMe;	// Remove user id from To: of reply
   Boolean		numberReplies;	// Re[n]: as opposed to Re:
   StringC		indentPrefix;	// Used in replies
   StringC		attribStr;
   StringC		beginForwardStr;
   StringC		endForwardStr;
   Boolean		forwardAllHeaders;
   Boolean		forwardNoHeaders;
   Boolean		stripComments;

//
// Public methods
//
    ReplyPrefC();
   ~ReplyPrefC();

   void		Edit(Widget);
   Boolean	WriteDatabase();
   Boolean	WriteFile();
};

#endif // _ReplyPrefC_h
