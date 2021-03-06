/*
 * $Id: FindDialogC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
 *
 * Copyright (c) 1992 HaL Computer Systems, Inc.  All rights reserved.
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

#ifndef _FindDialogC_h_
#define _FindDialogC_h_

#include "HalDialogC.h"
#include "RegexC.h"

#define  FIND_DIALOG_NAME		"Find"

class VBoxC;
class VItemC;

class FindDialogC : public HalDialogC {

   static void	DoFirst(Widget, FindDialogC*, XtPointer);
   static void	DoNext (Widget, FindDialogC*, XtPointer);
   static void	DoDone (Widget, FindDialogC*, XtPointer);

   static void	PatChange(Widget, FindDialogC*, XtPointer);

   static Boolean	Find(const VItemC&);

// Parent

   VBoxC	*viewBox;

// Widgets

   Widget	patField;

   static RegexC	findPat;

public:

// Constructor

   FindDialogC(VBoxC*);
};

#endif // _FindDialogC_h_
