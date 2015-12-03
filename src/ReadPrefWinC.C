/*
 * $Id: ReadPrefWinC.C,v 1.2 2000/05/07 12:26:12 fnevgeny Exp $
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

#include "ReadPrefWinC.h"
#include "ReadPrefC.h"
#include "IshAppC.h"
#include "MainWinC.h"
#include "ReadWinC.h"

#include <hgl/WArgList.h>
#include <hgl/WXmString.h>
#include <hgl/RowColC.h>
#include <hgl/TextMisc.h>
#include <hgl/MimeRichTextC.h>
#include <hgl/CharC.h>

#include <Xm/Label.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include <Xm/Frame.h>
#include <Xm/Form.h>

#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>

/*---------------------------------------------------------------
 *  Main window constructor
 */

ReadPrefWinC::ReadPrefWinC(Widget par) : OptWinC(par, "readPrefWin")
{
   WArgList	args;
   Cardinal	wcount;
   Widget 	wlist[16];

//
// Create appForm hierarchy
//
// appForm
//    RowColC		fieldRC
//    
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_NONE);
   fieldRC = new RowColC(appForm, "fieldRC", ARGS);

//
// Set up 2 columns in fieldRC
//
   fieldRC->Defer(True);
   fieldRC->SetOrientation(RcROW_MAJOR);
   fieldRC->SetColCount(2);
   fieldRC->SetColAlignment(0, XmALIGNMENT_END);
   fieldRC->SetColAlignment(1, XmALIGNMENT_BEGINNING);
   fieldRC->SetColWidthAdjust(RcADJUST_NONE);
   fieldRC->SetColResize(0, False);
   fieldRC->SetColResize(1, True);

//
// Create fieldRC hierarchy
//
// fieldRC
//    Label		colLabel
//    Form		colForm
//    Label		bodyRowLabel
//    Form		rowForm
//    Label		viewLabel
//    Frame		viewFrame
//    Label		webLabel
//    TextField		webTF
//    Label		decryptLabel
//    TextField		decryptTF
//    Label		authLabel
//    TextField		authTF
//
   Widget colLabel     = XmCreateLabel  (*fieldRC, "colLabel",     0,0);
   Widget colForm      = XmCreateForm   (*fieldRC, "colForm",      0,0);
   Widget bodyRowLabel = XmCreateLabel  (*fieldRC, "bodyRowLabel", 0,0);
   Widget rowForm      = XmCreateForm   (*fieldRC, "rowForm",      0,0);
   Widget viewLabel    = XmCreateLabel  (*fieldRC, "viewLabel",    0,0);
   Widget viewFrame    = XmCreateFrame  (*fieldRC, "viewFrame",    0,0);
   Widget webLabel     = XmCreateLabel  (*fieldRC, "webLabel",     0,0);
          webTF        = CreateTextField(*fieldRC, "webTF",        0,0);
   Widget decryptLabel = XmCreateLabel  (*fieldRC, "decryptLabel", 0,0);
          decryptTF    = CreateTextField(*fieldRC, "decryptTF",    0,0);
   Widget authLabel    = XmCreateLabel  (*fieldRC, "authLabel",    0,0);
          authTF       = CreateTextField(*fieldRC, "authTF",       0,0);

//
// Create colForm widgets
//
// colForm
//    TextField		colTF
//    ToggleButton	wrapTB
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_NONE);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   wrapTB = XmCreateToggleButton(colForm, "wrapTB", ARGS);

   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_WIDGET, wrapTB);
   colTF = CreateTextField(colForm, "colTF", ARGS);
   XtManageChild(wrapTB);
   XtManageChild(colTF);

//
// Create rowForm widgets
//
// rowForm
//    TextField		bodyRowTF
//    Label		headRowLabel
//    TextField		headRowTF
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   bodyRowTF = CreateTextField(rowForm, "bodyRowTF", ARGS);

   args.LeftAttachment(XmATTACH_WIDGET, bodyRowTF);
   args.RightAttachment(XmATTACH_NONE);
   Widget headRowLabel = XmCreateLabel(rowForm, "headRowLabel", ARGS);

   args.LeftAttachment(XmATTACH_WIDGET, headRowLabel);
   args.RightAttachment(XmATTACH_FORM);
   headRowTF = CreateTextField(rowForm, "headRowTF", ARGS);

   XtManageChild(bodyRowTF);
   XtManageChild(headRowLabel);
   XtManageChild(headRowTF);

//
// Create viewFrame hierarchy
//
// viewFrame
//    RadioBox		viewRadio
//       ToggleButton	viewFlatTB
//       ToggleButton	viewOutlineTB
//       ToggleButton	viewNestedTB
//       ToggleButton	viewSourceTB
//
   args.Reset();
   args.Orientation(XmHORIZONTAL);
   args.Packing(XmPACK_TIGHT);
   Widget viewRadio = XmCreateRadioBox(viewFrame, "viewRadio", ARGS);

   viewFlatTB    = XmCreateToggleButton(viewRadio, "viewFlatTB",    0,0);
   viewOutlineTB = XmCreateToggleButton(viewRadio, "viewOutlineTB", 0,0);
   viewNestedTB  = XmCreateToggleButton(viewRadio, "viewNestedTB",  0,0);
   viewSourceTB  = XmCreateToggleButton(viewRadio, "viewSourceTB",  0,0);

   wcount = 0;
   wlist[wcount++] = viewFlatTB;
   wlist[wcount++] = viewOutlineTB;
//   wlist[wcount++] = viewNestedTB;
   wlist[wcount++] = viewSourceTB;
   XtManageChildren(wlist, wcount);       // viewRadio children

   XtManageChild(viewRadio);  // viewFrame children

   wcount = 0;
   wlist[wcount++] = colLabel;
   wlist[wcount++] = colForm;
   wlist[wcount++] = bodyRowLabel;
   wlist[wcount++] = rowForm;
   wlist[wcount++] = viewLabel;
   wlist[wcount++] = viewFrame;
   wlist[wcount++] = webLabel;
   wlist[wcount++] = webTF;
   wlist[wcount++] = decryptLabel;
   wlist[wcount++] = decryptTF;
   wlist[wcount++] = authLabel;
   wlist[wcount++] = authTF;
   fieldRC->SetChildren(wlist, wcount);	// fieldRC children

   XtManageChild(*fieldRC);

   fieldRC->Defer(False);

   HandleHelp();

} // End constructor

/*---------------------------------------------------------------
 *  Method to handle display
 */

void
ReadPrefWinC::Show(Widget parent)
{
   ReadPrefC	*prefs = ishApp->readPrefs;

//
// See if this widget corresponds to a particular reading window
//
   ReadWinC	*readWin = NULL;
   if ( parent != (Widget)*ishApp->mainWin ) {
      unsigned	count = ishApp->readWinList.size();
      for (int i=0; !readWin && i<count; i++) {
	 ReadWinC	*win = (ReadWinC*)*ishApp->readWinList[i];
	 if ( parent == (Widget)*win ) readWin = win;
      }
   }

//
// Initialize settings
//
   if ( readWin )
      XmToggleButtonSetState(wrapTB, readWin->Wrapping(), True);
   else
      XmToggleButtonSetState(wrapTB, prefs->wrap, True);

   StringC	tmpStr;
   if ( readWin ) tmpStr += readWin->ColumnCount();
   else		  tmpStr += prefs->visCols;
   XmTextFieldSetString(colTF, tmpStr);

   tmpStr.Clear();
   if ( readWin ) tmpStr += readWin->HeadRowCount();
   else		  tmpStr += prefs->visHeadRows;
   XmTextFieldSetString(headRowTF, tmpStr);

   tmpStr.Clear();
   if ( readWin ) tmpStr += readWin->BodyRowCount();
   else		  tmpStr += prefs->visBodyRows;
   XmTextFieldSetString(bodyRowTF, tmpStr);

   ReadViewTypeT	viewType;
   if ( readWin ) viewType = readWin->ViewType();
   else		  viewType = prefs->viewType;

   switch (viewType) {
      case (READ_VIEW_FLAT):
	 XmToggleButtonSetState(viewFlatTB, True, True);
	 break;
      case (READ_VIEW_OUTLINE):
	 XmToggleButtonSetState(viewOutlineTB, True, True);
	 break;
      case (READ_VIEW_CONTAINER):
	 XmToggleButtonSetState(viewNestedTB, True, True);
	 break;
      case (READ_VIEW_SOURCE):
	 XmToggleButtonSetState(viewSourceTB, True, True);
	 break;
   }

   XmTextFieldSetString(webTF,     prefs->webCmd);
   XmTextFieldSetString(decryptTF, prefs->decryptCmd);
   XmTextFieldSetString(authTF,    prefs->authCmd);

   OptWinC::Show(parent);

} // End Show

void
ReadPrefWinC::Show()
{
   Show(XtParent(this->shell));
}

/*---------------------------------------------------------------
 *  Method to apply settings
 */

Boolean
ReadPrefWinC::Apply()
{
   ReadPrefC	*prefs = ishApp->readPrefs;

   BusyCursor(True);

//
// Read values
//
   Boolean	wrap = XmToggleButtonGetState(wrapTB);

   char	*cs = XmTextFieldGetString(colTF);
   int	cols = 0;
   if ( strlen(cs) > 0 ) cols = atoi(cs);
   XtFree(cs);

   cs = XmTextFieldGetString(headRowTF);
   int	headRows = 0;
   if ( strlen(cs) > 0 ) headRows = atoi(cs);
   XtFree(cs);

   cs = XmTextFieldGetString(bodyRowTF);
   int	bodyRows = 0;
   if ( strlen(cs) > 0 ) bodyRows = atoi(cs);
   XtFree(cs);

   ReadViewTypeT	viewType = READ_VIEW_FLAT;
   if ( XmToggleButtonGetState(viewOutlineTB) )
      viewType = READ_VIEW_OUTLINE;
   else if ( XmToggleButtonGetState(viewNestedTB) )
      viewType = READ_VIEW_CONTAINER;
   else if ( XmToggleButtonGetState(viewSourceTB) )
      viewType = READ_VIEW_SOURCE;

//
// Loop through reading windows
//
   u_int	count = ishApp->readWinList.size();
   for (int i=0; i<count; i++) {

      ReadWinC	*readWin = (ReadWinC*)*ishApp->readWinList[i];
      readWin->SetWrap(wrap);
      readWin->SetSize(headRows, bodyRows, cols);
      readWin->SetViewType(viewType);
   }

   prefs->wrap        = wrap;
   prefs->visCols     = cols;
   prefs->visHeadRows = headRows;
   prefs->visBodyRows = bodyRows;
   prefs->viewType    = viewType;

//
// Read web command
//
   cs = XmTextFieldGetString(webTF);
   Boolean	webChanged = (prefs->webCmd != cs);
   prefs->webCmd = cs;
   XtFree(cs);

//
// Read decrypt command
//
   cs = XmTextFieldGetString(decryptTF);
   prefs->decryptCmd = cs;
   XtFree(cs);

//
// Read authentication command
//
   cs = XmTextFieldGetString(authTF);
   prefs->authCmd = cs;
   XtFree(cs);

   prefs->WriteDatabase();

//
// Write to file if necessary
//
   if ( applyAll ) prefs->WriteFile();

//
// If web command changed, update all rich text widgets
//
   if ( webChanged ) UpdateWebCommand(*halApp);

   BusyCursor(False);

   return True;

} // End Apply

/*---------------------------------------------------------------
 *  Method used to update the web browser command for all richtext widgets
 */

void
ReadPrefWinC::UpdateWebCommand(Widget w)
{
   if ( !w || !XtIsWidget(w) ) return;

   if ( debuglev > 1 ) cout <<"Checking " <<XtName(w) <<endl;

//
// If this is a richtext widget, get the object pointer from its user data
//
   CharC	wname = XtName(w);
   if ( wname.Equals("richTextArea") ) {

      MimeRichTextC	*rt;
      XtVaGetValues(w, XmNuserData, &rt, NULL);

      if ( rt ) rt->SetWebCommand(ishApp->readPrefs->webCmd);

   } // End if this is a richtext widget

//
// If this is a composite, loop through the children and check them.
//
   if ( XtIsComposite(w) ) {

      WidgetList	list;
      Cardinal		count;
      XtVaGetValues(w, XmNnumChildren, &count, XmNchildren, &list, NULL);

//
// Loop through children
//
      for (int i=0; i<count; i++)
	 UpdateWebCommand(list[i]);

   } // End if this is a composite widget

//
// If there are any popups associated with this widget, check them
//
   if ( w->core.num_popups > 0 ) {

      for (int i=0; i<w->core.num_popups; i++)
	 UpdateWebCommand(w->core.popup_list[i]);

   } // End if there are any popups
   
} // End UpdateWebCommand

