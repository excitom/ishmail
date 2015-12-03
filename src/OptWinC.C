/*
 * $Id: OptWinC.C,v 1.3 2000/05/07 12:26:12 fnevgeny Exp $
 *
 * Copyright (c) 1993 HAL Computer Systems International, Ltd.
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

#include "OptWinC.h"

#include <hgl/HalAppC.h>
#include <hgl/WArgList.h>

#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/PushB.h>

/*---------------------------------------------------------------
 *  Main window constructor
 */

OptWinC::OptWinC(Widget parent, const char *name) : HalDialogC(name, parent)
{
   applyAll = True;

//
// Create buttonRC hierarchy
//
// buttonRC
//    PushButton	applyPB
//    PushButton	okPB
//    PushButton	donePB
//    PushButton	helpPB
//
   AddButtonBox();
   Widget
      okPB    = XmCreatePushButton(buttonRC, "okPB",	0,0),
      applyPB = XmCreatePushButton(buttonRC, "applyPB", 0,0),
      donePB  = XmCreatePushButton(buttonRC, "donePB",  0,0),
      helpPB  = XmCreatePushButton(buttonRC, "helpPB",	0,0);

   Widget	list[4];
   list[0] = okPB;
   list[1] = applyPB;
   list[2] = donePB;
   list[3] = helpPB;
   XtManageChildren(list, 4);	// buttonRC children

   XtAddCallback(okPB, XmNactivateCallback,	(XtCallbackProc)DoOk,
   		 (XtPointer)this);
   XtAddCallback(applyPB, XmNactivateCallback,	(XtCallbackProc)DoApply,
   		 (XtPointer)this);
   XtAddCallback(donePB, XmNactivateCallback, (XtCallbackProc)DoCancel,
   		 (XtPointer)this);
   XtAddCallback(helpPB, XmNactivateCallback, (XtCallbackProc)HalAppC::DoHelp,
   		 (char *) "helpcard");

//
// Create a frame between the button box and the button separator to show
//    the current/future choices
//
// topForm
//    Separator		buttonSep	(already created)
//    Frame		centerFrame
//       Frame		applyFrame
//          RadioBox	   applyRadio
//             ToggleButton      applyAllTB
//             ToggleButton      applyCurrentTB
//    Frame		buttonFrame	(already created)
//
   WArgList	args;
   args.ShadowThickness(0);
   args.MarginWidth(0);
   args.MarginHeight(0);
   args.TopAttachment(XmATTACH_NONE);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_WIDGET, buttonRC);
   Widget centerFrame = XmCreateFrame(topForm, "centerFrame", ARGS);

   args.Reset();
   args.ChildType(XmFRAME_TITLE_CHILD);
   args.ChildHorizontalAlignment(XmALIGNMENT_CENTER);
   args.ChildHorizontalSpacing(0);
   args.ChildVerticalAlignment(XmALIGNMENT_WIDGET_TOP);
   Widget applyFrame = XmCreateFrame(centerFrame, "applyFrame", ARGS);

   args.Reset();
   args.Orientation(XmHORIZONTAL);
   args.Packing(XmPACK_TIGHT);
   Widget applyRadio = XmCreateRadioBox(applyFrame, "applyRadio", ARGS);

   Widget applyAllTB     = XmCreateToggleButton(applyRadio, "applyAllTB", 0,0);
   Widget applyCurrentTB = XmCreateToggleButton(applyRadio, "applyCurrentTB",
   						0,0);
   XmToggleButtonSetState(applyAllTB, True, False);

   XtAddCallback(applyAllTB, XmNvalueChangedCallback,
		 (XtCallbackProc)SetApplyAll, (XtPointer)this);
   XtAddCallback(applyCurrentTB, XmNvalueChangedCallback,
		 (XtCallbackProc)SetApplyCurrent, (XtPointer)this);

   XtManageChild(applyAllTB);
   XtManageChild(applyCurrentTB);
   XtManageChild(applyRadio);
   XtManageChild(applyFrame);
   XtManageChild(centerFrame);

   args.Reset();
   args.BottomAttachment(XmATTACH_WIDGET, centerFrame);
   XtSetValues(buttonSep, ARGS);

} // End OptWinC constructor

/*---------------------------------------------------------------
 *  Main window destructor
 */

OptWinC::~OptWinC()
{
}

/*---------------------------------------------------------------
 *  Callback to handle ok
 */

void
OptWinC::DoOk(Widget, OptWinC *pw, XtPointer)
{
   if ( pw->Apply() ) pw->Cancel();
}

/*---------------------------------------------------------------
 *  Callback to handle apply
 */

void
OptWinC::DoApply(Widget, OptWinC *pw, XtPointer)
{
   pw->Apply();
}

/*---------------------------------------------------------------
 *  Callback to handle cancel
 */

void
OptWinC::DoCancel(Widget, OptWinC *pw, XtPointer)
{
   pw->Cancel();
}

void
OptWinC::Cancel()
{
   Hide();
}

/*---------------------------------------------------------------
 *  Callbacks to handle setting of apply type
 */

void
OptWinC::SetApplyCurrent(Widget, OptWinC *pw,XmToggleButtonCallbackStruct *tb)
{
   if ( !tb->set ) return;
   pw->applyAll = False;
}

void
OptWinC::SetApplyAll(Widget, OptWinC *pw, XmToggleButtonCallbackStruct *tb)
{
   if ( !tb->set ) return;
   pw->applyAll = True;
}

