/*
 *  $Id: ReplyPrefWinC.C,v 1.3 2000/06/05 14:46:48 evgeny Exp $
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

#include <config.h>

#include "ReplyPrefWinC.h"
#include "ReplyPrefC.h"
#include "IshAppC.h"
#include "CompPrefC.h"

#include <hgl/WArgList.h>
#include <hgl/rsrc.h>
#include <hgl/RowColC.h>
#include <hgl/TextMisc.h>
#include <hgl/CharC.h>

#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>
#include <Xm/Frame.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>

#include <signal.h>

/*---------------------------------------------------------------
 *  Main window constructor
 */

ReplyPrefWinC::ReplyPrefWinC(Widget par) : OptWinC(par, "replyPrefWin")
{
   WArgList	args;
   Widget 	wlist[24];

//
// Create appForm hierarchy
//
// appForm
//    RowColC		checkRC
//    RowColC		fieldRC
//    
   args.Reset();
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.TopAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_NONE);
   checkRC = new RowColC(appForm, "checkRC", ARGS);

   args.TopAttachment(XmATTACH_WIDGET, *checkRC);
   args.BottomAttachment(XmATTACH_FORM);
   args.BorderWidth(1);
   fieldRC = new RowColC(appForm, "fieldRC", ARGS);

//
// Set up 1 columns in checkRC
//
   checkRC->Defer(True);
   checkRC->SetOrientation(RcROW_MAJOR);
   checkRC->SetColCount(1);
   checkRC->SetColAlignment(XmALIGNMENT_BEGINNING);
   checkRC->SetColWidthAdjust(RcADJUST_EQUAL);
   checkRC->SetColResize(False);

//
// Set up 2 columns in fieldRC
//
   fieldRC->Defer(True);
   fieldRC->SetOrientation(RcROW_MAJOR);
   fieldRC->SetColCount(2);
   fieldRC->SetColAlignment(XmALIGNMENT_CENTER);
   fieldRC->SetColWidthAdjust(0, RcADJUST_EQUAL);
   fieldRC->SetColWidthAdjust(1, RcADJUST_ATTACH);
   fieldRC->SetColResize(0, False);
   fieldRC->SetColResize(1, True);
   fieldRC->SetUniformRows(False);

//
// Create checkRC hierarchy
//
// checkRC
//    ToggleButton	removeMeTB
//    ToggleButton	numberTB
//    ToggleButton	stripTB
//
   removeMeTB = XmCreateToggleButton(*checkRC, "removeMeTB", 0,0);
   numberTB   = XmCreateToggleButton(*checkRC, "numberTB",   0,0);
   stripTB    = XmCreateToggleButton(*checkRC, "stripTB",    0,0);

   wlist[0] = removeMeTB;
   wlist[1] = numberTB;
   wlist[2] = stripTB;
   checkRC->SetChildren(wlist, 3);

//
// Create fieldRC hierarchy
//
// fieldRC
//    Label		indentLabel
//    TextField		indentTF
//    Label		attribLabel
//    TextField		attribText
//    Label		begForwardLabel
//    TextField		begForwardText
//    Label		endForwardLabel
//    TextField		endForwardText
//    Label		forwardHeadLabel
//    Frame		forwardHeadFrame
//
   Widget indentLabel      = XmCreateLabel    (*fieldRC, "indentLabel",    0,0);
   indentTF                = CreateTextField(*fieldRC, "indentTF",       0,0);
   Widget attribLabel      = XmCreateLabel    (*fieldRC, "attribLabel",    0,0);
   attribText              = CreateTextField(*fieldRC, "attribText",     0,0);
   Widget begForwardLabel  = XmCreateLabel    (*fieldRC, "begForwardLabel",0,0);
   begForwardText          = CreateTextField(*fieldRC, "begForwardText", 0,0);
   Widget endForwardLabel  = XmCreateLabel    (*fieldRC, "endForwardLabel",0,0);
   endForwardText          = CreateTextField(*fieldRC, "endForwardText", 0,0);
   Widget forwardHeadLabel = XmCreateLabel   (*fieldRC, "forwardHeadLabel",0,0);
   Widget forwardHeadFrame = XmCreateFrame   (*fieldRC, "forwardHeadFrame",0,0);

//
// Create forwardHeadFrame hierarchy
//
// forwardHeadFrame
//    RadioBox		forwardHeadRadio
//       ToggleButton      forwardAllTB
//       ToggleButton      forwardDispTB
//       ToggleButton      forwardNoneTB
//
   args.Reset();
   args.Orientation(XmHORIZONTAL);
   args.Packing(XmPACK_TIGHT);
   Widget forwardHeadRadio = XmCreateRadioBox(forwardHeadFrame,
					      "forwardHeadRadio", ARGS);

   forwardAllTB  = XmCreateToggleButton(forwardHeadRadio, "forwardAllTB",  0,0);
   forwardDispTB = XmCreateToggleButton(forwardHeadRadio, "forwardDispTB", 0,0);
   forwardNoneTB = XmCreateToggleButton(forwardHeadRadio, "forwardNoneTB", 0,0);

   wlist[0] = forwardAllTB;
   wlist[1] = forwardDispTB;
   wlist[2] = forwardNoneTB;
   XtManageChildren(wlist, 3);       // forwardHeadRadio children

   XtManageChild(forwardHeadRadio);  // forwardHeadFrame children

   int	count = 0;
   wlist[count++] = indentLabel;
   wlist[count++] = indentTF;
   wlist[count++] = attribLabel;
   wlist[count++] = attribText;
   wlist[count++] = begForwardLabel;
   wlist[count++] = begForwardText;
   wlist[count++] = endForwardLabel;
   wlist[count++] = endForwardText;
   wlist[count++] = forwardHeadLabel;
   wlist[count++] = forwardHeadFrame;
   fieldRC->SetChildren(wlist, count);

   wlist[0] = *checkRC;
   wlist[1] = *fieldRC;
   XtManageChildren(wlist, 2);	// appForm children

   checkRC->Defer(False);
   fieldRC->Defer(False);

   HandleHelp();

   XtAddCallback(shell, XmNpopupCallback, (XtCallbackProc)DoPopup,
                    (XtPointer)this);

} // End constructor

/*---------------------------------------------------------------
 *  Destructor
 */

ReplyPrefWinC::~ReplyPrefWinC()
{
   delete fieldRC;
   delete checkRC;
}

/*---------------------------------------------------------------
 *  Callback routine to handle initial display.
 */

void
ReplyPrefWinC::DoPopup(Widget, ReplyPrefWinC *This, XtPointer)
{
   This->fieldRC->SetRowResize(1, True);	// attrib row
   This->fieldRC->SetRowResize(2, True);	// begin forward row
   This->fieldRC->SetRowResize(3, True);	// end forward row
   This->fieldRC->SetRowHeightAdjust(1, RcADJUST_ATTACH);
   This->fieldRC->SetRowHeightAdjust(2, RcADJUST_ATTACH);
   This->fieldRC->SetRowHeightAdjust(3, RcADJUST_ATTACH);
}

/*---------------------------------------------------------------
 *  Method to handle display
 */

void
ReplyPrefWinC::Show(Widget parent)
{
   ReplyPrefC	*prefs = ishApp->replyPrefs;

//
// Initialize settings
//
   XmToggleButtonSetState(removeMeTB,	prefs->removeMe,	False);
   XmToggleButtonSetState(numberTB,	prefs->numberReplies,	False);
   XmToggleButtonSetState(stripTB,	prefs->stripComments,	False);
   XmTextFieldSetString(indentTF,       prefs->indentPrefix);

   XmTextFieldSetString(attribText,	prefs->attribStr);
   XmTextFieldSetString(begForwardText,	prefs->beginForwardStr);
   XmTextFieldSetString(endForwardText,	prefs->endForwardStr);

   if ( prefs->forwardAllHeaders )
      XmToggleButtonSetState(forwardAllTB, True, True);
   else if ( prefs->forwardNoHeaders )
      XmToggleButtonSetState(forwardNoneTB, True, True);
   else
      XmToggleButtonSetState(forwardDispTB, True, True);

   OptWinC::Show(parent);

} // End Show

void
ReplyPrefWinC::Show()
{
   Show(XtParent(*this));
}

/*---------------------------------------------------------------
 *  Method to apply settings
 */

Boolean
ReplyPrefWinC::Apply()
{
   ReplyPrefC	*prefs = ishApp->replyPrefs;

   BusyCursor(True);

   prefs->removeMe      = XmToggleButtonGetState(removeMeTB);
   prefs->numberReplies = XmToggleButtonGetState(numberTB);
   prefs->stripComments = XmToggleButtonGetState(stripTB);
   if ( !prefs->stripComments ) ishApp->compPrefs->spaceEndsAddr = False;

//
// Set indent prefix
//
   char	*cs = XmTextFieldGetString(indentTF);
   prefs->indentPrefix = cs;
   XtFree(cs);

//
// Set attribution string
//
   prefs->attribStr.Clear();
   cs = XmTextFieldGetString(attribText);
   prefs->attribStr = cs;
   XtFree(cs);

//
// Set forwarding delimiters
//
   prefs->beginForwardStr.Clear();
   prefs->endForwardStr.Clear();
   cs = XmTextFieldGetString(begForwardText);
   prefs->beginForwardStr = cs;
   XtFree(cs);
   cs = XmTextFieldGetString(endForwardText);
   prefs->endForwardStr = cs;
   XtFree(cs);

   if ( XmToggleButtonGetState(forwardNoneTB) ) {
      prefs->forwardAllHeaders = False;
      prefs->forwardNoHeaders  = True;
   }
   else if ( XmToggleButtonGetState(forwardDispTB) ) {
      prefs->forwardAllHeaders = False;
      prefs->forwardNoHeaders  = False;
   }
   else {
      prefs->forwardAllHeaders = True;
      prefs->forwardNoHeaders  = False;
   }

   prefs->WriteDatabase();

//
// Write to file if necessary
//
   if ( applyAll ) prefs->WriteFile();

   BusyCursor(False);

   return True;

} // End Apply

