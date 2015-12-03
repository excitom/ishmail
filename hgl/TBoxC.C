/*
 * $Id: TBoxC.C,v 1.2 2000/05/07 12:26:11 fnevgeny Exp $
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

#include <config.h>

#include "HalAppC.h" 
#include "TBoxC.h"
#include "VBoxC.h"
#include "WXmString.h"
#include "WArgList.h"
#include "rsrc.h"
#include "ButtonBox.h"

#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>

/*----------------------------------------------------------------------
 * Method to build the widget hierarchy
 */

TBoxC::TBoxC(Widget parent, const char *name)
{
   WArgList	args;		// Used to set all resources at once

//
//      Frame		- taskFrame
//
// Traditional Setup
//
   //         Form		- taskForm
   //            Frame		- taskButtonFrame
   //               RowColumn	- taskButtonBox
   //            VBoxC		- taskVBox
//
// Alternate Setup
//
   //         Form		- taskForm
   //            ButtonBox	- taskButtonBox
   //            VBoxC		- taskVBox
//
//      Label		- taskTitle
//

//
// Create the overall frame for the task box
//
   taskFrame = XmCreateFrame(parent, (char *)name, 0,0);

//
// Create the title string
//
   args.Reset();
   args.ChildType(XmFRAME_TITLE_CHILD);
   taskTitle = XmCreateLabel(taskFrame, "taskTitle", ARGS);
   XtManageChild(taskTitle);

//
// Create the form to contain the button box and view box
//
   taskForm = XmCreateForm(taskFrame, "taskForm", 0,0);

   Boolean      useRowCol = get_boolean(taskFrame, "useRowCol", True);

   if ( useRowCol ) {	// Traditional
//
// Create the button frame using west gravity
//
      args.Reset();
      args.ShadowThickness(0);
      args.MarginWidth(0);
      args.MarginHeight(0);
      args.TopAttachment(XmATTACH_FORM);
      args.LeftAttachment(XmATTACH_FORM);
      args.RightAttachment(XmATTACH_NONE);
      args.BottomAttachment(XmATTACH_FORM);
      args.PositionIndex(0);
      taskButtonFrame = XmCreateFrame(taskForm, "taskButtonFrame", ARGS);

//
// Create the button box
//
      args.Reset();
      args.Packing(XmPACK_COLUMN);
      args.Orientation(XmVERTICAL);
      args.AdjustLast(False);
      args.EntryAlignment(XmALIGNMENT_CENTER);
      args.ChildType(XmFRAME_TITLE_CHILD);
      args.ChildHorizontalAlignment(XmALIGNMENT_CENTER);
      args.ChildVerticalAlignment(XmALIGNMENT_WIDGET_TOP);
      taskButtonBox = XmCreateRowColumn(taskButtonFrame, "taskButtonBox", ARGS);
      XtManageChild(taskButtonBox);
      XtManageChild(taskButtonFrame);

   } // End if traditional

   else {

      taskButtonFrame = NULL;

//
// Create the button box
//
      args.Reset();
      args.TopAttachment(XmATTACH_FORM);
      args.LeftAttachment(XmATTACH_FORM);
      args.RightAttachment(XmATTACH_NONE);
      args.BottomAttachment(XmATTACH_FORM);
      args.ResizeWidth(True);
      args.ResizeHeight(False);
      args.Orientation(XmVERTICAL);
      taskButtonBox = CreateButtonBox(taskForm, "taskButtonBox", ARGS);
      XtManageChild(taskButtonBox);

   } // End if non-traditional

//
// Create the view box
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   if ( taskButtonFrame ) args.LeftAttachment(XmATTACH_WIDGET, taskButtonFrame);
   else			  args.LeftAttachment(XmATTACH_WIDGET, taskButtonBox);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   taskVBox = new VBoxC(taskForm, "taskVBox");
   Widget	viewForm = taskVBox->ViewForm();
   XtSetValues(viewForm, ARGS);

   XtManageChild(taskForm);
   XtManageChild(taskFrame);

//
// Get the desired gravity
//
   buttonGravity = WestGravity;
   int gravity = get_gravity("TBoxC", taskFrame, "buttonGravity", "West");
   SetButtonGravity(gravity);

} // End Constructor

/*----------------------------------------------------------------------
 * Task box destructor
 */

TBoxC::~TBoxC()
{
   delete taskVBox;

   if ( halApp->xRunning )
      XtDestroyWidget(taskFrame);
}

/*----------------------------------------------------------------------
 * Method to set title
 */

void
TBoxC::Title(const char *title)
{
   WXmString str((char *)title);
   XtVaSetValues(taskTitle, XmNlabelString, (XmString)str, NULL);
}

/*----------------------------------------------------------------------
 * Method to set button gravity
 */

void
TBoxC::SetButtonGravity(int gravity)
{
   if ( buttonGravity == gravity ) return;

   if ( gravity == NorthEastGravity ||
	gravity == SouthEastGravity ) gravity = EastGravity;

   else if ( gravity == NorthWestGravity ||
	     gravity == SouthWestGravity ) gravity = WestGravity;

#if 0
//
// Keep form from resizing
//
   unsigned char        policy;
   XtVaGetValues(taskForm, XmNresizePolicy, &policy, NULL);
   XtVaSetValues(taskForm, XmNresizePolicy, XmRESIZE_NONE, NULL);
#endif

   Widget	viewForm = taskVBox->ViewForm();

#if 0
//
// Unmanage widgets
//
   Widget       wlist[2];
   wlist[0] = taskButtonFrame;
   wlist[1] = viewForm;
   XtUnmanageChildren(wlist, 2);
#endif

   WidgetList	wlist;
   Cardinal	wcount;
   XtVaGetValues(taskButtonBox, XmNchildren, &wlist, XmNnumChildren, &wcount,0);
   XtUnmanageChildren(wlist, wcount);

   WArgList	targs;
   Widget	tw = taskButtonFrame ? taskButtonFrame : taskButtonBox;

//
// Remove current attachments
//
   targs.TopAttachment(XmATTACH_FORM);
   targs.LeftAttachment(XmATTACH_FORM);
   targs.RightAttachment(XmATTACH_NONE);
   targs.BottomAttachment(XmATTACH_NONE);
   XtSetValues(tw,       targs.Args(), targs.NumArgs());
   XtSetValues(viewForm, targs.Args(), targs.NumArgs());

#if 0
   targs.Reset();
   targs.Width(10);
   targs.Height(10);
   XtSetValues(viewForm, targs.Args(), targs.NumArgs());
#endif

//
// Position the buttons.  Also add an event handler which will keep the
// button box in line with the view box.
//
   targs.Reset();
   targs.TopAttachment(XmATTACH_FORM);
   targs.LeftAttachment(XmATTACH_FORM);
   targs.RightAttachment(XmATTACH_FORM);
   targs.BottomAttachment(XmATTACH_FORM);

   WArgList	vargs;
   vargs.TopAttachment(XmATTACH_FORM);
   vargs.LeftAttachment(XmATTACH_FORM);
   vargs.RightAttachment(XmATTACH_FORM);
   vargs.BottomAttachment(XmATTACH_FORM);

   switch (gravity) {

      case (NorthGravity):
	 targs.BottomAttachment(XmATTACH_NONE);
	 vargs.TopAttachment(XmATTACH_WIDGET, tw);
	 // vargs.BottomAttachment(XmATTACH_NONE);
	 if ( taskButtonFrame )
	    XtVaSetValues(taskButtonBox, XmNorientation, XmHORIZONTAL, NULL);
	 else
	    XtVaSetValues(taskButtonBox, XmNorientation, XmHORIZONTAL,
	    				 XmNresizeWidth, False,
					 XmNresizeHeight, True,
					 NULL);

	 XtRemoveEventHandler(*taskVBox, StructureNotifyMask, False,
			      (XtEventHandler)HandleResize, (XtPointer)this);
	 break;

      case (NorthEastGravity):
      case (SouthEastGravity):
	 gravity = EastGravity;
      case (EastGravity):
	 targs.LeftAttachment(XmATTACH_NONE);
	 vargs.RightAttachment(XmATTACH_WIDGET, tw);
	 // vargs.LeftAttachment(XmATTACH_NONE);
	 if ( taskButtonFrame )
	    XtVaSetValues(taskButtonBox, XmNorientation, XmVERTICAL, NULL);
	 else
	    XtVaSetValues(taskButtonBox, XmNorientation, XmVERTICAL,
	    				 XmNresizeWidth, True,
					 XmNresizeHeight, False,
					 NULL);

	 XtAddEventHandler(*taskVBox, StructureNotifyMask, False,
			   (XtEventHandler)HandleResize, (XtPointer)this);
	 break;

      case (SouthGravity):
	 targs.TopAttachment(XmATTACH_NONE);
	 vargs.BottomAttachment(XmATTACH_WIDGET, tw);
	 // vargs.TopAttachment(XmATTACH_NONE);
	 if ( taskButtonFrame )
	    XtVaSetValues(taskButtonBox, XmNorientation, XmHORIZONTAL, NULL);
	 else
	    XtVaSetValues(taskButtonBox, XmNorientation, XmHORIZONTAL,
	    				 XmNresizeWidth, False,
					 XmNresizeHeight, True,
					 NULL);

	 XtRemoveEventHandler(*taskVBox, StructureNotifyMask, False,
			      (XtEventHandler)HandleResize, (XtPointer)this);
	 break;

      case (SouthWestGravity):
      case (NorthWestGravity):
      default:
	 gravity = WestGravity;
      case (WestGravity):
	 targs.RightAttachment(XmATTACH_NONE);
	 vargs.LeftAttachment(XmATTACH_WIDGET, tw);
	 // vargs.RightAttachment(XmATTACH_NONE);
	 if ( taskButtonFrame )
	    XtVaSetValues(taskButtonBox, XmNorientation, XmVERTICAL, NULL);
	 else
	    XtVaSetValues(taskButtonBox, XmNorientation, XmVERTICAL,
	    				 XmNresizeWidth, True,
					 XmNresizeHeight, False,
					 NULL);

	 XtAddEventHandler(*taskVBox, StructureNotifyMask, False,
			   (XtEventHandler)HandleResize, (XtPointer)this);
	 break;

   } // End switch button gravity

   XtSetValues(tw,       targs.Args(), targs.NumArgs());
   XtSetValues(viewForm, vargs.Args(), vargs.NumArgs());

   XtManageChildren(wlist, wcount);

#if 0
   XtManageChildren(wlist, 2);
#endif

#if 0
   XtVaSetValues(taskForm, XmNresizePolicy, policy, NULL);
#endif

   buttonGravity = gravity;

} // End SetButtonGravity

/*----------------------------------------------------------------------
 * Convenience methods
 */

int
TBoxC::num_selected() const {
   return taskVBox->SelItems().size();
}

VItemListC&
TBoxC::get_selected() {
   return taskVBox->SelItems();
}

const VItemListC&
TBoxC::get_selected() const {
   return taskVBox->SelItems();
}

void
TBoxC::Set(const WArgList& args) {
   XtSetValues(taskFrame, ARGS);
}

void
TBoxC::Manage()   const
{
   XtManageChild(taskFrame);
}

void
TBoxC::Unmanage() const
{
   XtUnmanageChild(taskFrame);
}

void
TBoxC::HandleResize(Widget, TBoxC* This, XEvent* ev, Boolean*)
{
   if ( ev->type == ConfigureNotify )
   {
      Position y;
      XtVaGetValues(*This->taskVBox, XmNy, &y, NULL);
      XtVaSetValues(This->ButtonBox(), XmNtopOffset, (int)y, NULL);
   }
}
