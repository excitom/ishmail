/*
 * $Id: TBoxC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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

#ifndef _TBoxC_h_
#define _TBoxC_h_

#include "Base.h"
#include <X11/Intrinsic.h>

class VBoxC;
class WArgList;
class VItemListC;

class TBoxC {

   // Widget ids for widgets in task box. Var names are same as ref names.

//
//      Frame		- taskFrame
//         Form		- taskForm
//            Frame		- taskButtonFrame
//               RowColumn	- taskButtonBox
//            VBoxC		- taskVBox
//
//      Label		- taskTitle
//
   Widget	taskFrame;
   Widget	taskForm;
   Widget	taskButtonFrame;
   Widget	taskButtonBox;
   VBoxC	*taskVBox;
   Widget	taskTitle;

   int		buttonGravity;

   static  void HandleResize(Widget, TBoxC*, XEvent*, Boolean*);

public:

// These are the methods of the object. The code for each is contained in the
// file TBoxC.C

// Object constructors and destructor

   TBoxC(Widget parent, const char *name);
   virtual ~TBoxC();

   // Set title
   void 		Title(const char *title);
   void 		SetButtonGravity(int);

   // Return the number of selected items
   int			num_selected() const;
   
   // Return a pointer to the selected items list
   VItemListC&	get_selected();
   const VItemListC&	get_selected() const;

   // Methods applying to root widget
   void			Set(const WArgList& args);
   void			Manage()   const;
   void			Unmanage() const;

//
// Cast to widget
//
   inline operator	Widget() const		{ return taskFrame; }

   // Return widgets
   MEMBER_QUERY(Widget,		ButtonBox,	taskButtonBox)
   MEMBER_QUERY(int,		ButtonGravity,	buttonGravity)
   MEMBER_QUERY(Widget,		Frame,		taskFrame)
   MEMBER_QUERY(Widget,		Form,		taskForm)
   MEMBER_QUERY(VBoxC&,		VBox,		*taskVBox)
};

#endif // _TBoxC_h_



