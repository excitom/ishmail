/*
 * $Id: EntryEditWinC.C,v 1.3 2000/06/05 13:28:02 evgeny Exp $
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

#include "EntryEditWinC.h"

#include <hgl/WArgList.h>
#include <hgl/rsrc.h>
#include <hgl/HalAppC.h>
#include <hgl/RowColC.h>
#include <hgl/RegexC.h>
#include <hgl/TextMisc.h>
#include <hgl/CharC.h>

#include <Xm/PushB.h>
#include <Xm/Label.h>
#include <Xm/TextF.h>

/*---------------------------------------------------------------
 *  Main window constructor
 */

EntryEditWinC::EntryEditWinC(Widget parent, const char *name)
: HalDialogC(name, parent)
{
   WArgList	args;

   keyString   = "";
   valueString = "";
   aliasMode   = False;

//
// Create appForm hierarchy
//
// appForm
//    RowColC	entryRC
//	Label		keyLabel
//	TextField	keyTF
//	Label		valueLabel
//	TextField	valueText
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   entryRC = new RowColC(appForm, "entryRC", ARGS);
   entryRC->Defer(True);

//
// Set up 2 Columns
//
   entryRC->SetOrientation(RcROW_MAJOR);
   entryRC->SetColCount(2);
   entryRC->SetColAlignment(XmALIGNMENT_END);
   entryRC->SetColWidthAdjust(RcADJUST_ATTACH);
   entryRC->SetColResize(0, False);
   entryRC->SetColResize(1, True);

   Widget	keyLabel = XmCreateLabel(*entryRC, "keyLabel", 0,0);
   keyTF = CreateTextField(*entryRC, "keyTF", 0,0);
   Widget	valueLabel = XmCreateLabel(*entryRC, "valueLabel", 0,0);

   valueText = CreateTextField(*entryRC, "valueText", 0,0);

   Widget	wlist[4];
   wlist[0] = keyLabel;
   wlist[1] = keyTF;
   wlist[2] = valueLabel;
   wlist[3] = valueText;
   XtManageChildren(wlist, 4);	// entryRC children

   XtManageChild(*entryRC);	// appForm children
   entryRC->SetChildren(wlist, 4);

   entryRC->Defer(False);

//
// Create buttonRC hierarchy
//
// buttonRC
//    PushButton	okPB
//    PushButton	applyPB
//    PushButton	donePB
//    PushButton	helpPB
//
   AddButtonBox();
   Widget
      okPB    = XmCreatePushButton(buttonRC, "okPB",	0,0),
      applyPB = XmCreatePushButton(buttonRC, "applyPB", 0,0),
      donePB  = XmCreatePushButton(buttonRC, "donePB",  0,0),
      helpPB  = XmCreatePushButton(buttonRC, "helpPB",	0,0);

   wlist[0] = okPB;
   wlist[1] = applyPB;
   wlist[2] = donePB;
   wlist[3] = helpPB;
   XtManageChildren(wlist, 4);	// buttonRC children

   XtAddCallback(okPB, XmNactivateCallback,	(XtCallbackProc)DoOk,
   		 (XtPointer)this);
   XtAddCallback(applyPB, XmNactivateCallback,	(XtCallbackProc)DoApply,
   		 (XtPointer)this);
   XtAddCallback(donePB, XmNactivateCallback, (XtCallbackProc)DoHide,
   		 (XtPointer)this);
   XtAddCallback(helpPB, XmNactivateCallback, (XtCallbackProc)HalAppC::DoHelp,
   		 (XtPointer)"helpcard");

   HandleHelp();

} // End EntryEditWinC constructor

/*---------------------------------------------------------------
 *  Destructor
 */

EntryEditWinC::~EntryEditWinC()
{
   delete entryRC;
}

/*---------------------------------------------------------------
 *  Method to display dialog
 */

void
EntryEditWinC::Show(CallbackFn *fn, void *data)
{
   applyCallback = CallbackC(fn, data);
   HalDialogC::Show();
}

/*---------------------------------------------------------------
 *  Method to turn on mode where commas are replaces with newlines
 */

void
EntryEditWinC::EnableAliases()
{
   aliasMode = True;
   entryRC->SetRowResize(0, False);
   entryRC->SetRowResize(1, True);
   entryRC->SetRowHeightAdjust(1, RcADJUST_ATTACH);
}

/*---------------------------------------------------------------
 *  Method to set key field
 */

void
EntryEditWinC::SetKey(const char *cs)
{
   XmTextFieldSetString(keyTF, (char *)cs);
   keyString = cs;
}

/*---------------------------------------------------------------
 *  Method to set value field
 */

void
EntryEditWinC::SetValue(const char *cs)
{
   valueString = cs;

//
// Replace commas and trailing whitespace with newlines if necessary
//
   if ( aliasMode ) {
      static RegexC	*commaPat = NULL;
      if ( !commaPat ) commaPat = new RegexC(",[ \t\n]*");
      while ( commaPat->search(valueString) >= 0 )
	 valueString((*commaPat)[0]) = "\n";
   }

   valueString.Trim();
   XmTextFieldSetString(valueText, (char *)cs);
}

/*---------------------------------------------------------------
 *  Callback to handle ok
 */

void
EntryEditWinC::DoOk(Widget, EntryEditWinC *This, XtPointer)
{
   if ( This->Apply() ) This->Hide();
}

/*---------------------------------------------------------------
 *  Callback to handle apply
 */

void
EntryEditWinC::DoApply(Widget, EntryEditWinC *This, XtPointer)
{
   This->Apply();
}

/*---------------------------------------------------------------
 *  Method to handle apply
 */

Boolean
EntryEditWinC::Apply()
{
//
// Check the key field
//
   if ( XmTextFieldGetLastPosition(keyTF) <= 0 ) {
      set_invalid(keyTF, True, True);
      PopupMessage("Please enter a value in the text field");
      return False;
   }

//
// Check the value field
//
   if ( XmTextFieldGetLastPosition(valueText) <= 0 ) {
      set_invalid(valueText, True, True);
      PopupMessage("Please enter a value in the text field");
      return False;
   }

//
// Save values
//
   char	*cs = XmTextFieldGetString(keyTF);
   keyString = cs;
   XtFree(cs);

   valueString.Clear();
   cs = XmTextFieldGetString(valueText);
   valueString = cs;
   XtFree(cs);

//
// Replace newlines with commas if necessary
//
   if ( aliasMode ) {
      valueString.Trim();
      for (int i=0; i<valueString.length(); i++) {
	 if ( valueString[i] == '\n' ) {
	    valueString(i,1) = ", ";
	    i++;
	 }
      }
   }

//
// Call callback
//
   applyCallback(this);
   return True;

} // End Apply
