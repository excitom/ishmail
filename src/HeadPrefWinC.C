/*
 *  $Id: HeadPrefWinC.C,v 1.2 2000/05/07 12:26:11 fnevgeny Exp $
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
#include "HeadPrefWinC.h"
#include "HeadPrefC.h"
#include "IshAppC.h"
#include "ReadWinC.h"

#include <hgl/WArgList.h>
#include <hgl/rsrc.h>
#include <hgl/HalAppC.h>
#include <hgl/RegexC.h>

#include <Xm/Frame.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include <Xm/ArrowB.h>
#include <Xm/Label.h>
#include <Xm/ToggleB.h>
#include <Xm/DragDrop.h>
#include <Xm/AtomMgr.h>

#include <X11/Xmu/Xct.h>

/*---------------------------------------------------------------
 *  Main window constructor
 */

							    // historical name
HeadPrefWinC::HeadPrefWinC(Widget parent) : OptWinC(parent, "headerWin")
{
   WArgList	args;
   Widget 	wlist[10];

//
// Create appForm hierarchy
//
// appForm
//    Form		listForm
//    Frame		choiceFrame
//
   args.Reset();
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.TopAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   choiceFrame = XmCreateFrame(appForm, "choiceFrame", ARGS);

   args.Reset();
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.TopAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_WIDGET, choiceFrame);
   listForm    = XmCreateForm (appForm, "listForm", ARGS);

//
//
// Create listForm hierarchy
//
// listForm
//    Frame		dListFrame
//    Form		arrowForm
//    Frame		iListFrame
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   arrowForm  = XmCreateForm (listForm, "arrowForm",  ARGS);
   dListFrame = XmCreateFrame(listForm, "dListFrame", ARGS);
   iListFrame = XmCreateFrame(listForm, "iListFrame", ARGS);

//
// Create choiceFrame hierarchy
//
// choiceFrame
//    RadioBox		choiceRadio
//	 ToggleButton	choiceDisplayTB
//	 ToggleButton	choiceIgnoreTB
//
   args.Reset();
   args.Orientation(XmHORIZONTAL);
   args.Packing(XmPACK_COLUMN);
   Widget	choiceRadio = XmCreateRadioBox(choiceFrame, "choiceRadio",ARGS);

   choiceDisplayTB = XmCreateToggleButton(choiceRadio, "choiceDisplayTB", 0,0);
   choiceIgnoreTB  = XmCreateToggleButton(choiceRadio, "choiceIgnoreTB",  0,0);

   wlist[0] = choiceDisplayTB;
   wlist[1] = choiceIgnoreTB;
   XtManageChildren(wlist, 2);	// choiceRadio children
   XtManageChild(choiceRadio);

//
// Create arrowForm hierarchy
//
// arrowForm
//    ArrowButton	dtoiAB
//    ArrowButton	itodAB
//
   args.Reset();
   args.ArrowDirection(XmARROW_RIGHT);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.TopAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_POSITION, 50);
   dtoiAB = XmCreateArrowButton(arrowForm, "dtoiAB", ARGS);

   args.ArrowDirection(XmARROW_LEFT);
   args.TopAttachment(XmATTACH_POSITION, 50);
   args.BottomAttachment(XmATTACH_NONE);
   itodAB = XmCreateArrowButton(arrowForm, "itodAB", ARGS);

   XtSetSensitive(dtoiAB, False);
   XtSetSensitive(itodAB, False);

   XtAddCallback(dtoiAB, XmNactivateCallback, (XtCallbackProc)MoveDToI,
		 (XtPointer)this);
   XtAddCallback(itodAB, XmNactivateCallback, (XtCallbackProc)MoveIToD,
		 (XtPointer)this);

   wlist[0] = dtoiAB;
   wlist[1] = itodAB;
   XtManageChildren(wlist, 2);

//
// Create dListFrame hierarchy
//
// dListFrame
//    Label		dListTitle
//    ListBoxC		dListBox
//
   args.Reset();
   args.ChildType(XmFRAME_TITLE_CHILD);
   args.ChildHorizontalAlignment(XmALIGNMENT_BEGINNING);
   Widget	dListTitle = XmCreateLabel(dListFrame, "dListTitle", ARGS);

   dListBox = new ListBoxC(dListFrame, "dListBox");
   XtUnmanageChild(dListBox->RepButton());
   dListBox->AllowDuplicates(False);
   dListBox->SetSorted(True);
   dListBox->AddSelectChangeCallback((CallbackFn *)ChangeListSelection, dtoiAB);
   dListBox->AddItemChangeCallback((CallbackFn *)ChangeDList, this);
   dListBox->AddVerifyCallback((CallbackFn *)VerifyHeader, this);

   wlist[0] = dListTitle;
   wlist[1] = *dListBox;
   XtManageChildren(wlist, 2);

//
// Create iListFrame hierarchy
//
// iListFrame
//    Label		iListTitle
//    ListBoxC		iListBox
//
   args.Reset();
   args.ChildType(XmFRAME_TITLE_CHILD);
   args.ChildHorizontalAlignment(XmALIGNMENT_BEGINNING);
   Widget	iListTitle = XmCreateLabel(iListFrame, "iListTitle", ARGS);

   iListBox = new ListBoxC(iListFrame, "iListBox");
   XtUnmanageChild(iListBox->RepButton());
   iListBox->AllowDuplicates(False);
   iListBox->SetSorted(True);
   iListBox->AddSelectChangeCallback((CallbackFn *)ChangeListSelection, itodAB);
   iListBox->AddItemChangeCallback((CallbackFn *)ChangeIList, this);
   iListBox->AddVerifyCallback((CallbackFn *)VerifyHeader, this);

   wlist[0] = iListTitle;
   wlist[1] = *iListBox;
   XtManageChildren(wlist, 2);

   wlist[0] = dListFrame;
   wlist[1] = arrowForm;
   wlist[2] = iListFrame;
   XtManageChildren(wlist, 3);	// listForm children

   wlist[0] = listForm;
   wlist[1] = choiceFrame;
   XtManageChildren(wlist, 2);	// appForm children

   HandleHelp();

//
// Set up drag-and-drop between lists
//
   XmDropSiteUnregister(*dListBox);
   XmDropSiteUnregister(*iListBox);

   args.Reset();
   args.UserData(this);
   XtSetValues(*dListBox, ARGS);
   XtSetValues(*iListBox, ARGS);

   Atom impAtoms[1];
   impAtoms[0] = XmInternAtom(halApp->display, "COMPOUND_TEXT", False);

   args.Reset();
   args.ImportTargets(impAtoms);
   args.NumImportTargets(1);
   args.DropProc((XtCallbackProc)HandleDrop);
   XmDropSiteRegister(*dListBox, ARGS);
   XmDropSiteRegister(*iListBox, ARGS);

//
// Make last minute adjustments
//
   XtAddCallback(*this, XmNpopupCallback, (XtCallbackProc)DoPopup,
		 (XtPointer)this);

} // End constructor

/*---------------------------------------------------------------
 *  Main window destructor
  */

HeadPrefWinC::~HeadPrefWinC()
{
   delete dListBox;
   delete iListBox;
}

/*---------------------------------------------------------------
 *  Callback to handle initial display
 */

void
HeadPrefWinC::DoPopup(Widget, HeadPrefWinC *This, XtPointer)
{
   XtRemoveCallback(*This, XmNpopupCallback, (XtCallbackProc)DoPopup,
		    (XtPointer)This);

   WArgList	args;

//
// Center choice frame
//
   Dimension	wd;
   XtVaGetValues(This->choiceFrame, XmNwidth, &wd, NULL);
   int	off = (int)wd/2;

   args.Reset();
   args.LeftAttachment(XmATTACH_POSITION, 50, -off);
   args.RightAttachment(XmATTACH_NONE);
   XtSetValues(This->choiceFrame, ARGS);

//
// Center arrow buttons
//
   XtVaGetValues(This->arrowForm, XmNwidth, &wd, NULL);
   off = (int)wd/2;

   args.Reset();
   args.LeftAttachment(XmATTACH_POSITION, 50, -off);
   args.RightAttachment(XmATTACH_NONE);
   XtSetValues(This->arrowForm, ARGS);

   args.Reset();
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_WIDGET, This->arrowForm);
   args.Resizable(False);
   XtSetValues(This->dListFrame, ARGS);

   args.Reset();
   args.LeftAttachment(XmATTACH_WIDGET, This->arrowForm);
   args.RightAttachment(XmATTACH_FORM);
   args.Resizable(False);
   XtSetValues(This->iListFrame, ARGS);

//
// Initialize settings
//
   HeadPrefC	*prefs = ishApp->headPrefs;

   This->dListBox->SetItems(prefs->showHeadList);
   This->iListBox->SetItems(prefs->hideHeadList);
   if ( prefs->useShowHeadList )
      XmToggleButtonSetState(This->choiceDisplayTB, True, True);
   else
      XmToggleButtonSetState(This->choiceIgnoreTB, True, True);

} // End DoPopup

/*---------------------------------------------------------------
 *  Method to apply settings
 */

Boolean
HeadPrefWinC::Apply()
{
   HeadPrefC	*prefs = ishApp->headPrefs;

   BusyCursor(True);

   prefs->showHeadList    = dListBox->Items();
   prefs->hideHeadList    = iListBox->Items();
   prefs->useShowHeadList = XmToggleButtonGetState(choiceDisplayTB);

//
// Redisplay visible messages
//
   u_int	count = ishApp->readWinList.size();
   for (int i=0; i<count; i++) {
      ReadWinC	*readWin = (ReadWinC*)*ishApp->readWinList[i];
      if ( readWin->IsShown() )
	 readWin->RedisplayHeaders();
   }

   prefs->WriteDatabase();

//
// Write to file if necessary
//
   if ( applyAll ) prefs->WriteFile();

   BusyCursor(False);

   return True;

} // End Apply

/*---------------------------------------------------------------
 *  Method to update arrow button sensitivity
 */

void
HeadPrefWinC::ChangeListSelection(ListBoxC *lb, Widget ab)
{
   XtSetSensitive(ab, lb->SelectCount()>0);
}

/*---------------------------------------------------------------
 *  Callback to move displayed headers to ignored list
 */

void
HeadPrefWinC::MoveDToI(Widget, HeadPrefWinC *This, XtPointer)
{
//
// Get selected items from dListBox
//
   StringListC	items;
   items.AllowDuplicates(TRUE);
   This->dListBox->GetSelectedItems(items);

//
// Remove items from dListBox and add them to iListBox
//
   This->dListBox->RemoveItems(items);
   This->iListBox->AddItems(items);

   XtSetSensitive(This->dtoiAB, False);

} // End MoveDToI

/*---------------------------------------------------------------
 *  Callback to move ignored headers to displayed list
 */

void
HeadPrefWinC::MoveIToD(Widget, HeadPrefWinC *This, XtPointer)
{
//
// Get selected items from iListBox
//
   StringListC	items;
   items.AllowDuplicates(TRUE);
   This->iListBox->GetSelectedItems(items);

//
// Remove items from iListBox and add them to dListBox
//
   This->iListBox->RemoveItems(items);
   This->dListBox->AddItems(items);

   XtSetSensitive(This->itodAB, False);

} // End MoveIToD

/*---------------------------------------------------------------
 *  Method to handle change in items in dList
 */

void
HeadPrefWinC::ChangeDList(ListBoxC*, HeadPrefWinC *This)
{
   StringListC&	dList = This->dListBox->Items();
   StringListC&	iList = This->iListBox->Items();

//
// The dList changed, and the user may have added items.  If any of them
//    are in the iList, remove them from there.
//
   unsigned	count = dList.size();
   for (int i=0; i<count; i++) {
      StringC	*item = dList[i];
      if ( iList.includes(*item) ) This->iListBox->RemoveItem(*item);
   }

} // End ChangeDList

/*---------------------------------------------------------------
 *  Method to handle change in items in iList
 */

void
HeadPrefWinC::ChangeIList(ListBoxC*, HeadPrefWinC *This)
{
   StringListC&	iList = This->iListBox->Items();
   StringListC&	dList = This->dListBox->Items();

//
// The iList changed, and the user may have added items.  If any of them
//    are in the dList, remove them from there.
//
   unsigned	count = iList.size();
   for (int i=0; i<count; i++) {
      StringC	*item = iList[i];
      if ( dList.includes(*item) ) This->dListBox->RemoveItem(*item);
   }

} // End ChangeDList

/*---------------------------------------------------------------
 *  Method to check headers before they are added
 */

void
HeadPrefWinC::VerifyHeader(ListBoxVerifyT *verify, HeadPrefWinC *This)
{
   StringC&	text = *verify->text;
   //cout <<"Verifying header <" <<text <<">" NL;

//
// Remove whitespace from ends
//
   text.Trim();

//
// If the last character is :, remove it
//
   int	pos = text.size() - 1;
   if ( text[pos] == ':' ) text(pos,1) = "";

//
// At this point, the header may not contain whitespace or a :
//
   static RegexC	*nonheader = NULL;
   if ( !nonheader ) nonheader = new RegexC("[ \t\n:]");

   if ( nonheader->search(text) >= 0 ) {
      set_invalid(verify->listBox->ItemText(), True, True);
      This->PopupMessage("Headers may not contain whitespace or a ':'");
      verify->ok = False;
      return;
   }

   //cout <<"Header is ok <" <<*(verify->text) <<">" NL;
   verify->ok = True;
   return;

} // End VerifyHeader

/*-----------------------------------------------------------------------
 *  Handle drop event
 */

void
HeadPrefWinC::HandleDrop(Widget w, XtPointer, XmDropProcCallbackStruct *dp)
{
   if ( debuglev > 0 ) cout <<"Got drop on list widget" <<endl;

   WArgList	args;

//
// See if this is a valid drop
//
   unsigned char	status = XmTRANSFER_FAILURE;
   if ( dp->dropSiteStatus == XmVALID_DROP_SITE ) {

//
// See what was dropped
//
      Atom		*expAtoms;
      Cardinal	atomCount;
      XtVaGetValues(dp->dragContext, XmNexportTargets, &expAtoms,
				     XmNnumExportTargets, &atomCount, NULL);
      if ( atomCount > 0 ) {

	 Atom textAtom = XmInternAtom(halApp->display, "COMPOUND_TEXT", False);
	 if ( expAtoms[0] == textAtom ) {

	    XmDropTransferEntryRec	transferEntries[1];
	    XmDropTransferEntry		transferList;
	    transferEntries[0].target = textAtom;
	    transferEntries[0].client_data = (XtPointer)w;
	    transferList = transferEntries;
	    args.DropTransfers(transferList);
	    args.NumDropTransfers(1);
	    args.TransferProc((XtSelectionCallbackProc)TransferText);

	    status = XmTRANSFER_SUCCESS;
	 }

	 else if ( debuglev > 0 ) cout <<"Dropped atom is not text" <<endl;

      } // End if there are dropped atoms
      
      else if ( debuglev > 0 ) cout <<"No atoms were dropped" <<endl;

   } // End if this is a valid drop site

   else if ( debuglev > 0 ) cout <<"Drop site status is not valid" <<endl;

//
// Drop must be acknowledged
//
   args.TransferStatus(status);
   XmDropTransferStart(dp->dragContext, ARGS);

} // End HandleDrop

/*-----------------------------------------------------------------------
 *  Handle drop event
 */

void
HeadPrefWinC::TransferText(Widget, Widget listBoxW, Atom*, Atom*,
			 XtPointer value, unsigned long *len, int*)
{
   if ( debuglev > 0 ) cout <<"Entering TransferText" <<endl;

//
// Get the window pointer
//
   HeadPrefWinC	*This;
   XtVaGetValues(listBoxW, XmNuserData, &This, NULL);

//
// Get the string that was dropped
//
   XctData	ctData = XctCreate((XctString)value, (int)*len, 0);
   XctResult	result;
   while ( (result=XctNextItem(ctData)) != XctError &&
      	    result                      != XctEndOfText ) {

      if ( result != XctSegment ) continue;

      StringC	str;
      str.Set((char*)ctData->item, ctData->item_length);
      if ( debuglev > 0 ) cout <<"text is -" <<str <<"-" <<endl;

//
// Build a list of names
//
      StringListC	nameList;
      StringC	name;
      char	*namep = strtok(str, "\n");
      if ( namep ) {
	 while ( namep ) {
	    name = namep;
	    nameList.add(name);
	    namep = strtok(NULL, "\n");
	 }
      }
      else {
	 nameList.add(str);
      }

//
// Loop through dropped items
//
      unsigned	count = nameList.size();
      for (int i=0; i<count; i++) {

	 StringC	*name = nameList[i];

	 if ( debuglev > 0 ) cout <<"checking name -" <<*name <<"-" <<endl;

//
// See if we're dropping on display list or ignore list
//
	 if ( listBoxW == (Widget)*This->iListBox ) {
	    This->dListBox->RemoveItem(*name);
	    This->iListBox->AddItem(*name);
	 }

	 else {
	    This->iListBox->RemoveItem(*name);
	    This->dListBox->AddItem(*name);
	 }

      } // End for each name

   } // End for each item in compound text

   XctFree(ctData);

} // End TransferText
