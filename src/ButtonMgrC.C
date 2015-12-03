/*
 * $Id: ButtonMgrC.C,v 1.2 2000/05/07 12:26:11 fnevgeny Exp $
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

#include "ButtonMgrC.h"

#include <hgl/HalShellC.h>
#include <hgl/TBoxC.h>
#include <hgl/WArgList.h>
#include <hgl/RegexC.h>
#include <hgl/rsrc.h>
#include <hgl/HalAppC.h>
#include <hgl/WidgetListC.h>
#include <hgl/WXmString.h>

#include <Xm/PushB.h>

#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>

extern int	debuglev;

/*---------------------------------------------------------------
 *  Constructor
 */

ButtonMgrC::ButtonMgrC(HalShellC *sh, Widget mb, Widget bb, char *str,
		       TBoxC *tb)
{
   shell     = sh;
   taskBox   = tb;
   menuBar   = mb;
   buttonBox = bb;
   StringC	buttonStr(str);

//
// Initialize resource string for help
//
   resPrefix = "Ishmail*";
   resPrefix += XtName(*shell);
   resPrefix += ".mainWindow*";
   if ( tb ) { 	// For backwards compatibility
      resPrefix += XtName(*tb);
      resPrefix += "*";
   }
   else {
      resPrefix += XtName(buttonBox);
      resPrefix += ".";
   }

//
// Create buttons
//
   WArgList	args;
   short	posIndex = 1;

   if ( buttonStr.size() > 0 ) {

      static RegexC	*wordPat = NULL;
      if ( !wordPat ) wordPat = new RegexC("[^ \t]+");

//
// Extract button widget names
//
      StringC	name;
      int	pos = 0;
      while ( (pos=wordPat->search(buttonStr,pos)) >= 0 ) {

	 RangeC	wordR((*wordPat)[0]);
	 name = buttonStr(wordR);
	 pos += wordR.length();

	 if ( debuglev > 1 ) cout <<"button widget name is " <<name NL;

//
// Create pushbutton widget with label from text field and user data pointing
//    to original button in main window
//
	 args.PositionIndex(posIndex);
	 AddButton(name, args);
	 posIndex++;

      } // End for each button

//
// Manage all children
//
      ManageButtons();

   } // End if any buttons were found

//
// Update the button gravity to get the buttons to show
//
   if ( taskBox ) taskBox->SetButtonGravity(taskBox->ButtonGravity());
   else		  shell->SetButtonGravity(shell->ButtonGravity());

} // End constructor

/*---------------------------------------------------------------
 *  Destructor
 */

ButtonMgrC::~ButtonMgrC()
{
}

/*---------------------------------------------------------------
 *  Enable buttons
 */

Widget
ButtonMgrC::AddButton(char *name, WArgList& args)
{
//
// See if we can look up the real button by name
//
   StringC	starname("*");
   starname += name;
   Widget	realPB = XtNameToWidget(menuBar, starname);
   if ( !realPB ) return NULL;

   args.Mnemonic(0);
   args.UserData((void *)realPB);
   args.Sensitive(XtIsSensitive(realPB));
   Widget	pb = XmCreatePushButton(buttonBox, name, ARGS);

//
// Add callback to handle activation.  That callback will call the callbacks
//    for the real button.
//
   XtAddCallback(pb, XmNactivateCallback, (XtCallbackProc)DoActivate, realPB);

//
// Add help strings to resource database
//
   StringC	valStr, resStr;
   XrmDatabase	db = XtDatabase(halApp->display);

   valStr = get_string(realPB, "quickHelp");
   if ( valStr.size() > 0 ) {
      resStr = resPrefix + name + ".quickHelp";
      XrmPutStringResource(&db, resStr, valStr);
   }

   valStr = get_string(realPB, "contextHelp");
   if ( valStr.size() > 0 ) {
      resStr = resPrefix + name + ".contextHelp";
      XrmPutStringResource(&db, resStr, valStr);
   }

   valStr = get_string(realPB, "helpcard");
   if ( valStr.size() > 0 ) {
      resStr = resPrefix + name + ".helpcard";
      XrmPutStringResource(&db, resStr, valStr);
   }

   shell->HandleHelp(pb);

   return pb;

} // End AddButton

/*---------------------------------------------------------------
 *  Manage all buttons
 */

void
ButtonMgrC::ManageButtons()
{
//
// Manage all children
//
   WidgetList	wlist;
   Cardinal	wcount;
   XtVaGetValues(buttonBox, XmNnumChildren, &wcount, XmNchildren, &wlist, 0);

//
// Remove widgets that are being destroyed
//
   WidgetListC	list;
   for (int i=0; i<wcount; i++) {
      Widget	w = wlist[i];
      if ( !w->core.being_destroyed ) list.add(w);
   }

   if ( list.size() > 0 )
      XtManageChildren(list.start(), list.size());

} // End ManageButtons

/*---------------------------------------------------------------
 *  Enable buttons
 */

void
ButtonMgrC::EnableButtons()
{
//
// Loop through buttons and match sensitivities to real button
//
   WidgetList	wlist;
   Cardinal	wcount;
   XtVaGetValues(buttonBox, XmNnumChildren, &wcount, XmNchildren, &wlist, 0);

   for (int i=0; i<wcount; i++) {

//
// Get real widget
//
      Widget	w;
      XtVaGetValues(wlist[i], XmNuserData, &w, NULL);

//
// See if widget or any of its ancestors is insensitive
//
      Boolean	sensitive = XtIsSensitive(w);
      while ( sensitive && XtParent(w) ) {
	 w = XtParent(w);
	 sensitive = XtIsSensitive(w);
      }

//
// Set sensitivity
//
      XtSetSensitive(wlist[i], sensitive);

   } // End for each widget in button box

} // End EnableButtons

/*---------------------------------------------------------------
 *  Remove all buttons
 */

void
ButtonMgrC::RemoveButtons(WidgetList wlist, Cardinal wcount)
{
   XtUnmanageChildren(wlist, wcount);
   for (int i=0; i<wcount; i++) XtDestroyWidget(wlist[i]);
}

/*---------------------------------------------------------------
 *  Return button gravity
 */

int
ButtonMgrC::Gravity()
{
   if ( taskBox ) return taskBox->ButtonGravity();
   else		  return shell->ButtonGravity();
}

/*---------------------------------------------------------------
 *  Set button gravity
 */

void
ButtonMgrC::SetGravity(int gravity)
{
   if ( taskBox ) taskBox->SetButtonGravity(gravity);
   else		  shell->SetButtonGravity(gravity);
}

/*---------------------------------------------------------------
 *  Call real callbacks for a custom button
 */

void
ButtonMgrC::DoActivate(Widget, Widget pb, XtPointer callData)
{
   XtCallCallbacks(pb, XmNactivateCallback, callData);
}

/*---------------------------------------------------------------
 *  Find the custom button that corresponds to the specified real button
 */

Widget
ButtonMgrC::ButtonFor(Widget realPB)
{
//
// Loop through buttons and look for a match on the real button
//
   WidgetList	wlist;
   Cardinal	wcount;
   XtVaGetValues(buttonBox, XmNnumChildren, &wcount, XmNchildren, &wlist, 0);

   for (int i=0; i<wcount; i++) {

      Widget	w;
      XtVaGetValues(wlist[i], XmNuserData, &w, NULL);

      if ( w == realPB ) return wlist[i];

   } // End for each widget in button box

   return NULL;

} // End ButtonFor

/*---------------------------------------------------------------
 *  If a custom button exists for the given button, update its sensitivity
 */

void
ButtonMgrC::SensitivityChanged(Widget realPB)
{
   Widget	pb = ButtonFor(realPB);
   if ( pb ) XtSetSensitive(pb, XtIsSensitive(realPB));
}
