/*
 *  $Id: Query.C,v 1.4 2000/08/03 14:36:14 evgeny Exp $
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
#include "Query.h"
#include "Misc.h"

#include <hgl/HalAppC.h>
#include <hgl/StringC.h>
#include <hgl/rsrc.h>
#include <hgl/WXmString.h>
#include <hgl/CharC.h>
#include <hgl/WArgList.h>

#include <Xm/MessageB.h>
#include <Xm/PushB.h>
#include <Xm/Protocols.h>
#include <Xm/AtomMgr.h>

#include <unistd.h>

/*---------------------------------------------------------------
 *  Callback routine to handle response in a question dialog
 */

void
AnswerQuery(Widget, QueryAnswerT *answer, XmAnyCallbackStruct *cbs)
{
   switch (cbs->reason) {
      case(XmCR_OK):	   *answer = QUERY_YES;    break;
      case(XmCR_ACTIVATE): *answer = QUERY_NO;     break;
      case(XmCR_CANCEL):   *answer = QUERY_CANCEL; break;
   }
}

/*------------------------------------------------------------------------
 * Trap window manager close
 */

void
WmClose(Widget, QueryAnswerT *answer, XtPointer)
{
   *answer = QUERY_CANCEL;
}

/*---------------------------------------------------------------
 *  Function to query for file overwrite if necessary
 */

QueryAnswerT
Query(const char *msg, Widget parent, Boolean cancel)
{
   static QueryAnswerT	answer;
   static Widget	dialog = NULL;

//
// Create the dialog if necessary
//
   if ( !dialog ) {

      halApp->BusyCursor(True);
      WArgList	args;
      args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
      if ( halApp->questionPM ) args.SymbolPixmap(halApp->questionPM);
      dialog = XmCreateQuestionDialog(*halApp, "queryWin", ARGS);

      XtAddCallback(dialog, XmNokCallback, (XtCallbackProc)AnswerQuery,
		    (XtPointer)&answer);
      XtAddCallback(dialog, XmNcancelCallback,
		    (XtCallbackProc)AnswerQuery, (XtPointer)&answer);
      XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));
      // XtAddCallback(dialog, XmNhelpCallback, (XtCallbackProc)HalAppC::DoHelp,
      //		    (char *) "helpcard");

      Widget	noPB = XmCreatePushButton(dialog, "No", 0,0);
      XtManageChild(noPB);

      XtAddCallback(noPB, XmNactivateCallback, (XtCallbackProc)AnswerQuery,
      		    (XtPointer)&answer);

//
// Trap window manager close function
//
      XmAddWMProtocolCallback(XtParent(dialog), halApp->delWinAtom,
			      (XtCallbackProc)WmClose,
			      (caddr_t)&answer);

      halApp->BusyCursor(False);

   } // End if overwrite query dialog not created

   WXmString	wstr;
//
// Show/hide the "Cancel" button as needed
//
   if ( cancel ) {
      wstr = "OK";
      XtManageChild(XmMessageBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON));
   } else {
      wstr = "Yes";
      XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON));
   }
   XtVaSetValues(XmMessageBoxGetChild(dialog, XmDIALOG_OK_BUTTON),
      XmNlabelString, (XmString)wstr, NULL);
   
   wstr = (char *)msg;
   XtVaSetValues(dialog, XmNmessageString, (XmString)wstr, NULL);

//
// Display the dialog
//
   PopupOver(dialog, parent);

//
// Simulate the main event loop and wait for the answer
//
   answer = QUERY_NONE;
   while ( answer == QUERY_NONE ) {
      XtAppProcessEvent(halApp->context, XtIMXEvent);
      XSync(halApp->display, False);
   }

   XtUnmanageChild(dialog);
   XSync(halApp->display, False);
   XmUpdateDisplay(dialog);

   return answer;

} // End OverwriteQuery

/*---------------------------------------------------------------
 *  Function to query for file overwrite if necessary
 */

QueryAnswerT
OverwriteQuery(const char *file, Widget parent)
{
//
// If the file does not exist, then no need to confirm
//
   if ( access(file, F_OK) != 0 ) return QUERY_YES;

//
// Add the file name to the message
//
//   StringC	str = get_string(dialog, "messageString");
   StringC	str =    "$FILE already exists.\nShall I overwrite it or append the new data to the end?";
   str.Replace("$FILE", file);

   return Query(str, parent, True);

} // End OverwriteQuery

