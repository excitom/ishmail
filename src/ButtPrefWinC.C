/*
 *  $Id: ButtPrefWinC.C,v 1.2 2000/05/07 12:26:11 fnevgeny Exp $
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
#include "ButtPrefWinC.h"
#include "ButtonMgrC.h"
#include "ButtonEditWinC.h"

#include <hgl/WArgList.h>
#include <hgl/WXmString.h>
#include <hgl/rsrc.h>
#include <hgl/HalAppC.h>
#include <hgl/RegexC.h>
#include <hgl/TextMisc.h>
#include <hgl/RowColC.h>
#include <hgl/ButtonBox.h>
#include <hgl/StringListC.h>

#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/CascadeB.h>
#include <Xm/PushB.h>
#include <Xm/Frame.h>
#include <Xm/ToggleB.h>
#include <Xm/TextF.h>
#include <Xm/PanedW.h>
#include <Xm/ArrowB.h>
#include <Xm/List.h>
#include <Xm/AtomMgr.h>
#include <X11/Xmu/Xct.h>

extern int	debuglev;

/*------------------------------------------------------------------------
 * Function used to sort the entries by name.
 */

static int
SortByLabel(const void *a, const void *b)
{
   ButtonEntryC	*ba = *(ButtonEntryC **)a;
   ButtonEntryC	*bb = *(ButtonEntryC **)b;

   return ba->label.compare(bb->label);
}

/*------------------------------------------------------------------------
 * Function used to sort the entries by position index.
 */

static int
SortByIndex(const void *a, const void *b)
{
   ButtonEntryC	*ba = *(ButtonEntryC **)a;
   ButtonEntryC	*bb = *(ButtonEntryC **)b;

   if ( ba->posIndex > bb->posIndex ) return  1;
   if ( ba->posIndex < bb->posIndex ) return -1;
   return 0;
}

/*---------------------------------------------------------------
 *  Main window constructor
 */

ButtPrefWinC::ButtPrefWinC(Widget par, ButtonMgrC *mgr)
 : OptWinC(par, "buttonWin")
{
   WArgList	args;
   Widget	wlist[10];

   buttMgr = NULL;
   editWin = NULL;
   buttMgrList.AllowDuplicates(False);
   availList.AllowDuplicates(FALSE);
   availList.SetSorted(FALSE);

   AddManager(mgr);

//
// Create appForm hierarchy
//
// appForm
//    PanedWindow	panedWin
//
   args.Reset();
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.TopAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   Widget	panedWin = XmCreatePanedWindow(appForm, "panedWin", ARGS);

//
// Create panedWin hierarchy
//
// panedWin
//    Form		availForm
//    Form		arrowForm
//    Form		usedForm
//    Form		placeForm
//
   Widget	availForm = XmCreateForm(panedWin, "availForm", 0,0);
                arrowForm = XmCreateForm(panedWin, "arrowForm", 0,0);
   Widget	usedForm  = XmCreateForm(panedWin, "userForm",  0,0);

   args.Reset();
   args.AllowResize(True);
   placeForm = XmCreateForm(panedWin, "placeForm", ARGS);

//
// Create availForm hierarchy
//
// availForm
//    Label	availTitle
//    List	availListW
//
   args.Reset();
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.TopAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_NONE);
   args.Alignment(XmALIGNMENT_BEGINNING);
   Widget	availTitle = XmCreateLabel(availForm, "availTitle", ARGS);
   XtManageChild(availTitle);

   args.Reset();
   args.TopAttachment(XmATTACH_WIDGET, availTitle);
   args.BottomAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.SelectionPolicy(XmEXTENDED_SELECT);
   args.ListSizePolicy(XmCONSTANT);
   args.UserData(this);
   availListW = XmCreateScrolledList(availForm, "availList", ARGS);
   XtManageChild(availListW);

   XtAddCallback(availListW, XmNextendedSelectionCallback,
		 (XtCallbackProc)SelectAvail, (XtPointer)this);

//
// Create arrowForm hierarchy
//
// arrowForm
//    ArrowButton	addAB
//    ArrowButton	remAB
//
   args.Reset();
   args.ArrowDirection(XmARROW_DOWN);
   args.TopAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_NONE);
   args.RightAttachment(XmATTACH_POSITION, 50);
   addAB = XmCreateArrowButton(arrowForm, "addAB", ARGS);

   args.ArrowDirection(XmARROW_UP);
   args.LeftAttachment(XmATTACH_POSITION, 50);
   args.RightAttachment(XmATTACH_NONE);
   remAB = XmCreateArrowButton(arrowForm, "remAB", ARGS);

   XtSetSensitive(addAB, False);
   XtSetSensitive(remAB, False);

   XtAddCallback(addAB, XmNactivateCallback, (XtCallbackProc)HandleAdd,
		 (XtPointer)this);
   XtAddCallback(remAB, XmNactivateCallback, (XtCallbackProc)HandleRem,
		 (XtPointer)this);

   wlist[0] = addAB;
   wlist[1] = remAB;
   XtManageChildren(wlist, 2);

//
// Create usedForm hierarchy
//
// usedForm
//    Label	usedTitle
//    List	usedListW
//
   args.Reset();
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.TopAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_NONE);
   args.Alignment(XmALIGNMENT_BEGINNING);
   Widget	usedTitle = XmCreateLabel(usedForm, "userTitle", ARGS);
   XtManageChild(usedTitle);

   args.Reset();
   args.TopAttachment(XmATTACH_WIDGET, usedTitle);
   args.BottomAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.SelectionPolicy(XmEXTENDED_SELECT);
   args.ListSizePolicy(XmCONSTANT);
   args.UserData(this);
   usedListW = XmCreateScrolledList(usedForm, "userList", ARGS);
   XtManageChild(usedListW);

   XtAddCallback(usedListW, XmNextendedSelectionCallback,
		 (XtCallbackProc)SelectUsed, (XtPointer)this);
   XtAddCallback(usedListW, XmNdefaultActionCallback,
		 (XtCallbackProc)HandleEdit, (XtPointer)this);

//
// Create placeForm hierarchy
//
// placeForm
//    Label		placeTitle
//    Frame		gravFrame
//    ToggleButton	sameSizeTB
//
   args.Reset();
   args.TopAttachment(XmATTACH_NONE);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   Widget	gravFrame = XmCreateFrame(placeForm, "gravFrame", ARGS);

   args.Reset();
   args.TopAttachment(XmATTACH_NONE);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_OPPOSITE_WIDGET, gravFrame);
   args.BottomAttachment(XmATTACH_WIDGET, gravFrame);
   Widget	placeTitle = XmCreateLabel(placeForm, "placeTitle", ARGS);

   args.Reset();
   args.TopAttachment(XmATTACH_OPPOSITE_WIDGET, gravFrame);
   args.LeftAttachment(XmATTACH_WIDGET, gravFrame);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_NONE);
   sameSizeTB = XmCreateToggleButton(placeForm, "sameSizeTB", ARGS);

//
// Create gravFrame hierarchy
//
// gravFrame
//    RowColC	gravRC
//
   gravRC = new RowColC(gravFrame, "gravRC", 0,0);

//
// Set up 3 Columns
//
   gravRC->Defer(True);
   gravRC->SetOrientation(RcROW_MAJOR);
   gravRC->SetColCount(3);
   gravRC->SetColAlignment(XmALIGNMENT_CENTER);
   gravRC->SetColWidthAdjust(RcADJUST_EQUAL);
   gravRC->SetColResize(False);

//
// Create gravRC hierarchy
//
// gravRC
//    ToggleButton	northTB
//    ToggleButton	westTB
//    ToggleButton	eastTB
//    ToggleButton	southTB
//
   args.Reset();
   args.IndicatorType(XmONE_OF_MANY);
   northTB = XmCreateToggleButton(*gravRC, "northTB", ARGS);
   westTB  = XmCreateToggleButton(*gravRC, "westTB",  ARGS);
   eastTB  = XmCreateToggleButton(*gravRC, "eastTB",  ARGS);
   southTB = XmCreateToggleButton(*gravRC, "southTB", ARGS);

   XtAddCallback(northTB, XmNvalueChangedCallback, (XtCallbackProc)DoNorth,
   		 (XtPointer)this);
   XtAddCallback(westTB, XmNvalueChangedCallback, (XtCallbackProc)DoWest,
   		 (XtPointer)this);
   XtAddCallback(eastTB, XmNvalueChangedCallback, (XtCallbackProc)DoEast,
   		 (XtPointer)this);
   XtAddCallback(southTB, XmNvalueChangedCallback, (XtCallbackProc)DoSouth,
   		 (XtPointer)this);

   wlist[0] = northTB;
   wlist[1] = westTB;
   wlist[2] = eastTB;
   wlist[3] = southTB;
   XtManageChildren(wlist, 4);	// gravRC children

   XtManageChild(*gravRC);

   wlist[0] = NULL;
   wlist[1] = northTB;
   wlist[2] = NULL;
   wlist[3] = westTB;
   wlist[4] = NULL;
   wlist[5] = eastTB;
   wlist[6] = NULL;
   wlist[7] = southTB;
   wlist[8] = NULL;
   gravRC->SetChildren(wlist, 9);
   gravRC->Defer(False);

   wlist[0] = gravFrame;
   wlist[1] = placeTitle;
   wlist[2] = sameSizeTB;
   XtManageChildren(wlist, 3);	// placeForm children

   wlist[0] = availForm;
   wlist[1] = arrowForm;
   wlist[2] = usedForm;
   wlist[3] = placeForm;
   XtManageChildren(wlist, 4);	// panedWin children

   XtManageChild(panedWin);	// appForm children

//
// Build list of buttons
//
   Widget	helpCB;
   WidgetList	cbList;
   Cardinal	cbCount;
   XtVaGetValues(mgr->MenuBar(), XmNmenuHelpWidget, &helpCB,
				 XmNnumChildren,    &cbCount,
				 XmNchildren,       &cbList, NULL);
   if ( debuglev > 1 ) cout <<"Found " <<cbCount <<" children in menu bar" NL;

//
// Look for cascade buttons
//
   StringC	labStr;
   for (int i=0; i<cbCount; i++) {

      Widget	cb = cbList[i];
      if ( debuglev > 1 ) cout <<"Checking menu bar widget " <<XtName(cb) NL;

//
// Ignore non-cascade buttons and the help button
//
      if ( cb != helpCB && XmIsCascadeButton(cb) ) {
	 labStr = get_string(cb, XmNlabelString);
         ProcessCascade(cb, labStr);
      }

   } // End for each menuBar child

   HandleHelp();

//
// Sort the available button list alphabetically
//
   availList.sort(SortByLabel);

//
// Set up drag-and-drop
//
   XmDropSiteUnregister(availListW);
   XmDropSiteUnregister(usedListW);

   Atom impAtoms[1];
   impAtoms[0] = XmInternAtom(halApp->display, "COMPOUND_TEXT", False);

   args.Reset();
   args.ImportTargets(impAtoms);
   args.NumImportTargets(1);
   args.DropProc((XtCallbackProc)HandleDrop);
   XmDropSiteRegister(availListW, ARGS);
   XmDropSiteRegister(usedListW,  ARGS);

//
// Prepare for final initialization
//
   XtAddCallback(shell, XmNpopupCallback, (XtCallbackProc)DoPopup,
		 (XtPointer)this);

} // End constructor

/*---------------------------------------------------------------
 *  Main window destructor
 */

ButtPrefWinC::~ButtPrefWinC()
{
   unsigned	count = availList.size();
   int	i;
   for (i=0; i<count; i++) delete availList[i];

   count = usedList.size();
   for (i=0; i<count; i++) delete usedList[i];

   delete gravRC;
   delete editWin;
}

/*---------------------------------------------------------------
 *  Method to look at widgets in cascade button hierarchy
 */

void
ButtPrefWinC::ProcessCascade(Widget cb, StringC label)
{
//
// Get the button name and the pulldown menu widget
//
   Widget	pd;
   XtVaGetValues(cb, XmNsubMenuId, &pd, NULL);
   if ( debuglev > 1 )
      cout <<"Found pulldown: " <<XtName(pd) <<" for cascade: " <<label NL;

   if ( !pd ) return;

//
// Get the children of the pulldown menu
//
   WidgetList	pdList;
   Cardinal	pdCount;
   XtVaGetValues(pd, XmNnumChildren, &pdCount, XmNchildren, &pdList, NULL);
   if ( debuglev > 1 ) cout <<"Found " <<pdCount <<" children in pulldown" NL;

//
// Look for push buttons and cascade buttons
//
   StringC	title;
   for (int i=0; i<pdCount; i++) {

      Widget	w = pdList[i];
      if ( debuglev > 1 ) cout <<"Checking pulldown widget " <<XtName(w) NL;

      if ( !XmIsLabel(w) ) continue;

      title = label;
      title += "/";
      title += get_string(w, XmNlabelString);

      if ( XmIsCascadeButton(w) ) {

//
// Skip special cases of quick and recent folder menus.  These contents of
//   these menus are continually changing and there would be no point in
//   offering custom buttons for them.
//
	 StringC	name = XtName(w);
	 if ( !name.EndsWith("RecentCB") && !name.EndsWith("QuickCB") )
	    ProcessCascade(w, title);
      }

      else if ( XmIsPushButton(w) ) {
	 StringC	abbrev = get_string(w, "abbrev", title);
	 ButtonEntryC	*be = new ButtonEntryC(XtName(w), title, abbrev);
	 availList.add(be);
      }

   } // End for each pulldown child

} // End ProcessCascade

/*---------------------------------------------------------------
 *  Callback routine to handle initial display.
 */

void
ButtPrefWinC::DoPopup(Widget, ButtPrefWinC *This, XtPointer)
{
   XtRemoveCallback(This->shell, XmNpopupCallback, (XtCallbackProc)DoPopup,
		    (XtPointer)This);

//
// Fix sizes of panes
//
   Dimension	ht;
   XtVaGetValues(This->arrowForm,  XmNheight, &ht, NULL);
   XtVaSetValues(This->arrowForm,  XmNpaneMinimum, ht, XmNpaneMaximum, ht, 0);

   This->gravRC->Refresh();
   XtVaGetValues(This->placeForm, XmNheight, &ht, NULL);
   XtVaSetValues(This->placeForm, XmNpaneMinimum, ht, XmNpaneMaximum, ht, 0);

} // End DoPopup

/*---------------------------------------------------------------
 *  Method to display window
 */

void
ButtPrefWinC::Show()
{
   Show(XtParent(*this));
}

void
ButtPrefWinC::Show(Widget w)
{
   if ( shown ) return;

//
// Move used entries back to available list
//
   Boolean	sortNeeded = False;
   if ( usedList.size() > 0 ) {

      unsigned	count = usedList.size();
      for (int i=0; i<count; i++) {
	 ButtonEntryC	*entry = usedList[i];
	 availList.add(entry);
      }

      sortNeeded = True;
      usedList.removeAll();
   }

//
// Find out which buttons are to be used
//
   WidgetList	custList;
   Cardinal	custCount;
   XtVaGetValues(buttMgr->ButtonBox(), XmNnumChildren, &custCount,
				       XmNchildren, &custList, NULL);

   ButtonEntryC	*entry;
   int	i;
   for (i=0; i<custCount; i++) {

//
// Get widget id of real button
//
      Widget	pb = custList[i];
      short	posIndex;
      XmString	xstr;
      XtVaGetValues(pb, XmNpositionIndex, &posIndex, XmNlabelString, &xstr, 0);

//
// See which entry uses this button
//
      int	index = EntryIndex(availList, XtName(pb));
      if ( index == availList.NULL_INDEX ) continue;

//
// Move this button to the used list
//
      entry = availList[index];
      usedList.add(entry);
      availList.replace(NULL, index);
      entry->posIndex = posIndex;

//
// Display the user's label
//
      WXmString	wstr = xstr;
      char	*cs  = wstr;
      entry->abbrev  = cs;
      XtFree(cs);

      sortNeeded = True;

   } // End for each used button

   if ( sortNeeded ) {
      availList.removeNulls();
      availList.sort(SortByLabel);
      usedList.sort(SortByIndex);
   }

//
// Add names to avail list
//
   XmListDeleteAllItems(availListW);

   WXmString	wstr;
   unsigned	count = availList.size();
   for (i=0; i<count; i++) {

      entry = availList[i];
      wstr = (char*)entry->label;
      XmListAddItem(availListW, wstr, 0);
   }

//
// Add names to used list
//
   XmListDeleteAllItems(usedListW);

   count = usedList.size();
   for (i=0; i<count; i++) {

      entry = usedList[i];
      wstr = (char*)entry->abbrev;
      XmListAddItem(usedListW, wstr, 0);
   }

//
// Initialize placement info
//
   switch ( buttMgr->Gravity() ) {

      case (WestGravity):
	 XmToggleButtonSetState(westTB, True, True);
	 break;

      case (EastGravity):
	 XmToggleButtonSetState(eastTB, True, True);
	 break;

      case (NorthGravity):
	 XmToggleButtonSetState(northTB, True, True);
	 break;

      case (SouthGravity):
      default:
	 XmToggleButtonSetState(southTB, True, True);
	 break;

   } // End switch gravity

   Boolean	sameSize;
   XtVaGetValues(buttMgr->ButtonBox(), BbNuniformCols, &sameSize, NULL);
   XmToggleButtonSetState(sameSizeTB, sameSize, True);

   OptWinC::Show(w);

} // End Show

/*---------------------------------------------------------------
 *  Callback to handle press of North button
 */

void
ButtPrefWinC::DoNorth(Widget, ButtPrefWinC *This, XmToggleButtonCallbackStruct *tb)
{
   if ( tb->set ) {
      XmToggleButtonSetState(This->southTB, False, False);
      XmToggleButtonSetState(This->eastTB,  False, False);
      XmToggleButtonSetState(This->westTB,  False, False);
   } else {
      XmToggleButtonSetState(This->northTB, True, False);
   }

} // End DoNorth

/*---------------------------------------------------------------
 *  Callback to handle press of South button
 */

void
ButtPrefWinC::DoSouth(Widget, ButtPrefWinC *This, XmToggleButtonCallbackStruct *tb)
{
   if ( tb->set ) {
      XmToggleButtonSetState(This->northTB, False, False);
      XmToggleButtonSetState(This->eastTB,  False, False);
      XmToggleButtonSetState(This->westTB,  False, False);
   } else {
      XmToggleButtonSetState(This->southTB, True, False);
   }

} // End DoSouth

/*---------------------------------------------------------------
 *  Callback to handle press of East button
 */

void
ButtPrefWinC::DoEast(Widget, ButtPrefWinC *This, XmToggleButtonCallbackStruct *tb)
{
   if ( tb->set ) {
      XmToggleButtonSetState(This->northTB, False, False);
      XmToggleButtonSetState(This->southTB, False, False);
      XmToggleButtonSetState(This->westTB,  False, False);
   } else {
      XmToggleButtonSetState(This->eastTB, True, False);
   }

} // End DoEast

/*---------------------------------------------------------------
 *  Callback to handle press of West button
 */

void
ButtPrefWinC::DoWest(Widget, ButtPrefWinC *This, XmToggleButtonCallbackStruct *tb)
{
   if ( tb->set ) {
      XmToggleButtonSetState(This->northTB, False, False);
      XmToggleButtonSetState(This->southTB, False, False);
      XmToggleButtonSetState(This->eastTB,  False, False);
   } else {
      XmToggleButtonSetState(This->westTB, True, False);
   }

} // End DoWest

/*---------------------------------------------------------------
 *  Method to apply settings
 */

Boolean
ButtPrefWinC::Apply()
{
   BusyCursor(True);

//
// Read button gravity
//
   int	gravity;
   if      ( XmToggleButtonGetState(northTB) ) gravity = NorthGravity;
   else if ( XmToggleButtonGetState(eastTB)  ) gravity = EastGravity;
   else if ( XmToggleButtonGetState(westTB)  ) gravity = WestGravity;
   else					       gravity = SouthGravity;

   Boolean	sameSize = XmToggleButtonGetState(sameSizeTB);

//
// Loop for each button manager
//
   u_int	mgrCount = buttMgrList.size();
   for (int m=0; m<mgrCount; m++) {

      ButtonMgrC	*mgr = (ButtonMgrC*)*buttMgrList[m];

//
// Read list of existing buttons
//
      WidgetList	custList;
      Cardinal		custCount;
      XtVaGetValues(mgr->ButtonBox(), XmNnumChildren, &custCount,
				      XmNchildren,    &custList, NULL);

//
// Build list of button names and copy the widget ids.  When we change the
//    position index, the original list gets rearranged.
//
      StringC		name;
      StringListC	nameList;
      WidgetListC	buttList;
      nameList.SetSorted(FALSE);
      nameList.AllowDuplicates(TRUE);

      int	i;
      for (i=0; i<custCount; i++) {
	 Widget	pb = custList[i];
	 name = XtName(pb);
	 nameList.add(name);
	 buttList.add(pb);
      }
      u_int	nameCount = nameList.size();

//
// Remove any buttons that are no longer needed
//
      WidgetListC	remList;
      for (i=0; i<nameCount; i++) {

//
// If "name" doesn't appear in used list, it's not needed
//
	 int	index = EntryIndex(usedList, *nameList[i]);
	 if ( index == usedList.NULL_INDEX )
	    remList.add(*buttList[i]);
      }

//
// Delete obsolete buttons
//
      if ( remList.size() > 0 )
	 mgr->RemoveButtons(remList.start(), remList.size());

//
// Loop through the buttons we need
//
      WArgList	args;
      u_int	count = usedList.size();
      for (i=0; i<count; i++) {

	 ButtonEntryC	*entry = usedList[i];
	 int		index = nameList.indexOf(entry->name);

//
// Add button if not present
//
	 if ( index == nameList.NULL_INDEX ) {

//
// Set the label string and position index
//
	    XmString xmstr = XmStringCreateLtoR(entry->abbrev,
						XmFONTLIST_DEFAULT_TAG);
	    args.LabelString(xmstr);
	    args.PositionIndex(entry->posIndex);

//
// Add a new button to the manager
//
	    Widget	pb = mgr->AddButton(entry->name, args);

	    XmStringFree(xmstr);

	 } // End if a button for this entry does not exist

//
// Update label and position index if button is already present
//
	 else {

	    WXmString	wstr = (char*)entry->abbrev;
	    XmString	xstr = wstr;
	    short	posIndex = entry->posIndex;
	    XtVaSetValues(*buttList[index], XmNlabelString, xstr,
					    XmNpositionIndex, posIndex, NULL);
	 }

      } // End for each used entry

//
// Update the button placement
//
      XtVaSetValues(mgr->ButtonBox(), BbNuniformCols, sameSize,
      				      BbNuniformRows, sameSize, NULL);
      mgr->SetGravity(gravity);

//
// Turn on the buttons
//
      mgr->ManageButtons();

   } // End for each button manager

//
// Write to database and file if necessary
//
   Write();

   BusyCursor(False);

   return True;

} // End Apply

/*---------------------------------------------------------------
 *  Add a button manager to the list
 */

void
ButtPrefWinC::AddManager(ButtonMgrC *mgr)
{
   if ( !buttMgr ) buttMgr = mgr;

   void	*tmp = (void*)mgr;
   buttMgrList.add(tmp);
}

/*---------------------------------------------------------------
 *  Remove a button manager from the list
 */

void
ButtPrefWinC::RemoveManager(ButtonMgrC *mgr)
{
   void	*tmp = (void*)mgr;
   buttMgrList.remove(tmp);
}

/*---------------------------------------------------------------
 *  Callback to handle selection of available buttons
 */

void
ButtPrefWinC::SelectAvail(Widget,ButtPrefWinC *This, XmListCallbackStruct *listData)
{
   XtSetSensitive(This->addAB, listData->selected_item_count > 0);
}

/*---------------------------------------------------------------
 *  Callback to handle press of down arrow
 */

void
ButtPrefWinC::HandleAdd(Widget, ButtPrefWinC *This, XtPointer)
{
//
// Get positions of selected items in available list
//
   int	*posList;
   int	posCount;
   XmListGetSelectedPos(This->availListW, &posList, &posCount);

   This->AvailToUsed(posList, posCount, 0);

   XtSetSensitive(This->addAB, False);

} // End HandleAdd

/*---------------------------------------------------------------
 *  Callback to handle press of up arrow
 */

void
ButtPrefWinC::HandleRem(Widget, ButtPrefWinC *This, XtPointer)
{
//
// Get positions of selected items in used list
//
   int	*posList;
   int	posCount;
   XmListGetSelectedPos(This->usedListW, &posList, &posCount);

   This->UsedToAvail(posList, posCount);

   XtSetSensitive(This->remAB, False);

} // End HandleRem

/*---------------------------------------------------------------
 *  Callback to handle selection of used buttons
 */

void
ButtPrefWinC::SelectUsed(Widget, ButtPrefWinC *This, XmListCallbackStruct *listData)
{
   XtSetSensitive(This->remAB, listData->selected_item_count > 0);
}

/*---------------------------------------------------------------
 *  Callback to handle edit of a used button
 */

void
ButtPrefWinC::HandleEdit(Widget, ButtPrefWinC *This, XmListCallbackStruct *listData)
{
   if ( !This->editWin ) {
      This->editWin = new ButtonEditWinC(*This);
      This->editWin->AddApplyCallback((CallbackFn*)ApplyEdit, This);
   }

   This->editEntry = This->usedList[listData->item_position-1];
   This->saveEntry = *This->editEntry;
   This->editWin->Show(This->editEntry);

} // End HandleEdit

/*---------------------------------------------------------------
 *  Callback to apply of button edit
 */

void
ButtPrefWinC::ApplyEdit(ButtonEditWinC*, ButtPrefWinC *This)
{
//
// See if the abbreviation has changed
//
   if ( This->editEntry->abbrev != This->saveEntry.abbrev ) {
      WXmString	wstr = (char*)This->editEntry->abbrev;
      XmString	xstr = wstr;
      int	oldIndex = This->usedList.indexOf(This->editEntry);
      XmListReplaceItemsPos(This->usedListW, &xstr, 1, oldIndex+1);
   }

//
// See if the position index has changed
//
   if ( This->editEntry->posIndex > This->usedList.size() )
      This->editEntry->posIndex = This->usedList.size();

   if ( This->editEntry->posIndex < 1 )
      This->editEntry->posIndex = 1;

   if ( This->editEntry->posIndex != This->saveEntry.posIndex ) {

      int	oldIndex = This->usedList.indexOf(This->editEntry);
      This->usedList.remove(oldIndex);
      XmListDeletePos(This->usedListW, oldIndex+1);

      This->usedList.insert(This->editEntry, This->editEntry->posIndex-1);
      WXmString	wstr = (char*)This->editEntry->abbrev;
      XmListAddItem(This->usedListW, wstr, This->editEntry->posIndex);

//
// Update position indexes after the one that changed
//
      int	start = This->editEntry->posIndex;
      int	count = This->usedList.size();
      for (int i=start; i<count; i++) {
	 ButtonEntryC	*entry = This->usedList[i];
	 entry->posIndex = i+1;
      }
   }

   int	itemCount;
   XtVaGetValues(This->usedListW, XmNselectedItemCount, &itemCount, NULL);
   XtSetSensitive(This->remAB, itemCount>0);

   This->saveEntry = *This->editEntry;

} // End ApplyEdit

/*-----------------------------------------------------------------------
 *  Handle drop event in used list
 */

void
ButtPrefWinC::HandleDrop(Widget w, XtPointer, XmDropProcCallbackStruct *dp)
{
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

//
// Get the window pointer
//
	    ButtPrefWinC	*This;
	    XtVaGetValues(w, XmNuserData, &This, NULL);
	    This->dropY = dp->y;

	    XmDropTransferEntryRec	transferEntries[1];
	    XmDropTransferEntry		transferList;
	    transferEntries[0].target = textAtom;
	    transferEntries[0].client_data = (XtPointer)w;
	    transferList = transferEntries;
	    args.DropTransfers(transferList);
	    args.NumDropTransfers(1);
	    args.TransferProc((XtSelectionCallbackProc)FinishDrop);

	    status = XmTRANSFER_SUCCESS;
	 }

      } // End if there are dropped atoms

   } // End if this is a valid drop site

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
ButtPrefWinC::FinishDrop(Widget, Widget dropListW, Atom*, Atom*,
		       XtPointer value, unsigned long *len, int)
{
//
// Get the window pointer
//
   ButtPrefWinC	*This;
   XtVaGetValues(dropListW, XmNuserData, &This, NULL);

//
// Find the source positions of the items that were dropped
//
   IntListC	posList;
   Widget	dragListW;
   This->GetDragPositions(value, *len, &dragListW, posList);

//
// See which list we're dropping in
//
   if ( dropListW == This->usedListW ) {

//
// Find the position of the drop
//
      int	dropPos = XmListYToPos(dropListW, This->dropY);

      if ( debuglev > 1 )
	 cout <<posList.size() <<" items dropped on used list at position "
	      <<dropPos <<endl;

      if ( dragListW == This->usedListW )
	 This->UsedToUsed(posList.start(), posList.size(), dropPos);
      else
	 This->AvailToUsed(posList.start(), posList.size(), dropPos);
   }

   else if ( dragListW == This->usedListW ) {

      if ( debuglev > 1 )
	 cout <<posList.size() <<" items dropped on available list" <<endl;

      This->UsedToAvail(posList.start(), posList.size());
   }

//
// Update arrow button sensitivities
//
   int	count;
   XtVaGetValues(This->availListW, XmNselectedItemCount, &count, NULL);
   XtSetSensitive(This->addAB, count>0);

   XtVaGetValues(This->usedListW, XmNselectedItemCount, &count, NULL);
   XtSetSensitive(This->remAB, count>0);

} // End FinishDrop

/*-----------------------------------------------------------------------
 *  Method to return the button entry with the given real widget
 */

int
ButtPrefWinC::EntryIndex(ButtonEntryListC& list, const char *name)
{
   unsigned	count = list.size();
   for (int i=0; i<count; i++) {
      ButtonEntryC	*entry = list[i];
      if ( entry && entry->name == name ) return i;
   }

   return list.NULL_INDEX;
}

/*---------------------------------------------------------------
 *  Method to move one or more entries from the available list to
 *     the used list.
 */

void
ButtPrefWinC::AvailToUsed(int *srcList, int srcCount, int dstPos)
{
//
// Loop through source items
//
   WXmString	wstr;
   ButtonEntryC	*entry;
   int	i;
   for (i=0; i<srcCount; i++) {

      int	srcPos = srcList[i];
      entry = availList[srcPos-1];

//
// Add item to used list widget
//
      wstr = entry->abbrev;
      XmListAddItem(usedListW, wstr, dstPos);

//
// Add entry to used list
//
      if ( dstPos == 0 )
	 usedList.add(entry);

      else {
	 usedList.insert(entry, dstPos-1);
	 dstPos++;
      }

      availList.replace(NULL, srcPos-1);

   } // End for each moved entry

//
// Remove the entries from the available list
//
   availList.removeNulls();
   XmListDeletePositions(availListW, srcList, srcCount);

//
// Update positions in the used list
//
   u_int	count = usedList.size();
   for (i=0; i<count; i++) {
      entry = usedList[i];
      entry->posIndex = i+1;
   }

} // End AvailToUsed

/*---------------------------------------------------------------
 *  Method to move one or more entries from the used list to
 *     the available list.
 */

void
ButtPrefWinC::UsedToAvail(int *srcList, int srcCount)
{
//
// Add items to the available list
//
   ButtonEntryC	*entry;
   int		srcPos;
   int	i;
   for (i=0; i<srcCount; i++) {
      srcPos = srcList[i];
      entry  = usedList[srcPos-1];
      availList.add(entry);
   }

//
// Sort the available list
//
   availList.sort(SortByLabel);

//
// Add items to available list widget
//
   WXmString	wstr;
   for (i=0; i<srcCount; i++) {

      srcPos = srcList[i];
      entry = usedList[srcPos-1];
      int		index = availList.indexOf(entry);

      wstr = (char*)entry->label;
      XmListAddItem(availListW, wstr, index+1);

//
// Remove entry from used list
//
      entry->posIndex = 1;
      usedList.replace(NULL, srcPos-1);
      
   } // End for each selected item

//
// Remove the entries from the used list
//
   usedList.removeNulls();
   XmListDeletePositions(usedListW, srcList, srcCount);

//
// Update positions in the used list
//
   u_int	count = usedList.size();
   for (i=0; i<count; i++) {
      entry = usedList[i];
      entry->posIndex = i+1;
   }

} // UsedToAvail

/*---------------------------------------------------------------
 *  Method to move one or more entries to another location in the
 *     used list.
 */

void
ButtPrefWinC::UsedToUsed(int *srcList, int srcCount, int dstPos)
{
   if ( srcCount <= 0 || srcList[0] == dstPos ) return;

//
// Remove all entries from their old positions
//
   ButtonEntryListC	moveList;
   ButtonEntryC		*entry;
   int	i;
   for (i=0; i<srcCount; i++) {

      int	srcPos = srcList[i];

      entry = usedList[srcPos-1];
      usedList.replace(NULL, srcPos-1);
      moveList.add(entry);

   } // End for each moved entry

//
// Add entries in new positions
//
   WXmString	wstr;
   u_int	count = moveList.size();
   for (i=0; i<count; i++) {

      entry = moveList[i];
      wstr = (char*)entry->abbrev;
      XmListAddItem(usedListW, wstr, dstPos);

      if ( dstPos == 0 ) {
	 usedList.add(entry);
      }
      else {
	 usedList.insert(entry, dstPos-1);
	 dstPos++;
      }
   }

//
// Find new positions of null entries
//
   count = usedList.size();
   int	index = 0;
   for (i=0; i<count; i++) {
      if ( usedList[i] == NULL ) srcList[index++] = i+1;
   }

//
// Finally remove nulls and update screen
//
   usedList.removeNulls();
   XmListDeletePositions(usedListW, srcList, srcCount);

//
// Update position indexes
//
   count = usedList.size();
   for (i=0; i<count; i++) {
      entry = usedList[i];
      entry->posIndex = i+1;
   }

} // End UsedToUsed

/*---------------------------------------------------------------
 *  Method to loop through the compound text input (a list of names)
 *     and return the source list positions of the names.
 */

void
ButtPrefWinC::GetDragPositions(XtPointer value, u_long len, Widget *dragListW,
			     IntListC& posList)
{
//
// Get the string that was dropped
//
   StringC	str;
   StringListC	nameList;
   StringC	name;
   WXmString	wstr;	
   XctData	ctData = XctCreate((XctString)value, (int)len, 0);
   XctResult	result;
   while ( (result=XctNextItem(ctData)) != XctError &&
      	    result                      != XctEndOfText ) {
      if ( result != XctSegment ) continue;

      str.Set((char*)ctData->item, ctData->item_length);

//
// Build a list of names
//
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
// Loop through names
//
      u_int	count = nameList.size();
      for (int i=0; i<count; i++) {
	 name = *nameList[i];
         wstr = (char*)name;
	 int pos = XmListItemPos(usedListW, wstr);
	 if ( pos > 0 ) {
	    posList.add(pos);
	    *dragListW = usedListW;
	 }
	 else {
	    *dragListW = availListW;
	    pos = XmListItemPos(availListW, wstr);
	    if ( pos > 0 ) posList.add(pos);
	 }
      }

   } // End while more compound text

   XctFree(ctData);

} // End GetDragPositions
