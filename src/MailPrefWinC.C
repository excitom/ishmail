/*
 *  $Id: MailPrefWinC.C,v 1.5 2000/06/19 13:32:18 evgeny Exp $
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

#include "MailPrefWinC.h"
#include "MailPrefC.h"
#include "IshAppC.h"
#include "FileMisc.h"
#include "Misc.h"
#include "SendWinC.h"
#include "MainWinC.h"

#include <hgl/WArgList.h>
#include <hgl/rsrc.h>
#include <hgl/RowColC.h>
#include <hgl/TextMisc.h>

#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>
#include <Xm/Frame.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/CascadeB.h>

#include <signal.h>
#include <errno.h>
#include <unistd.h>

/*---------------------------------------------------------------
 *  Main window constructor
 */

MailPrefWinC::MailPrefWinC(Widget par) : OptWinC(par, "mailPrefWin")
{
   WArgList	args;
   Widget 	wlist[24];

//
// Create appForm hierarchy
//
// appForm
//    RowColC		fieldRC
//    Form		splitForm
//    RowColC		checkRC
//    
   args.Reset();
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.TopAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_NONE);
   fieldRC = new RowColC(appForm, "fieldRC", ARGS);

   args.TopAttachment(XmATTACH_WIDGET, *fieldRC);
   Widget splitForm = XmCreateForm(appForm, "splitForm", ARGS);

   args.TopAttachment(XmATTACH_WIDGET, splitForm);
   args.BottomAttachment(XmATTACH_FORM);
   checkRC = new RowColC(appForm, "checkRC", ARGS);

//
// Set up 2 columns in checkRC
//
   checkRC->Defer(True);
   checkRC->SetOrientation(RcROW_MAJOR);
   checkRC->SetColCount(2);
   checkRC->SetColAlignment(XmALIGNMENT_BEGINNING);
   checkRC->SetColWidthAdjust(RcADJUST_NONE);
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
//    ToggleButton	checkAddrTB
//
   checkAddrTB = XmCreateToggleButton(*checkRC, "checkAddrTB", 0,0);

   wlist[0] = checkAddrTB;
   checkRC->SetChildren(wlist, 1);

//
// Create splitForm hierarchy
//
// splitForm
//    ToggleButton	splitTB
//    TextField		splitTF
//    Label		splitLabel
//
   args.Reset();
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.TopAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   splitTB = XmCreateToggleButton(splitForm, "splitTB", ARGS);

   args.LeftAttachment(XmATTACH_WIDGET, splitTB);
   splitTF = CreateTextField(splitForm, "splitTF", ARGS);

   args.LeftAttachment(XmATTACH_WIDGET, splitTF);
   Widget splitLabel = XmCreateLabel(splitForm, "splitLabel", ARGS);

   wlist[0] = splitTB;
   wlist[1] = splitTF;
   wlist[2] = splitLabel;
   XtManageChildren(wlist, 3);	// splitForm children

//
// Create fieldRC hierarchy
//
// fieldRC
//    Label		fromHeadLabel
//    TextField		fromHeadTF
//    Label		mailTypeLabel
//    Frame		mailTypeFrame
//    Label		textTypeLabel
//    Frame		textTypeFrame
//    Label		charLabel
//    Form		charForm
//    Label		bodyEncLabel
//    Frame		bodyEncFrame
//    Label		headEncLabel
//    Frame		headEncFrame
//    Label		fccLabel
//    Frame		fccFrame
//    ToggleButton	confirmAddrTB
//    Text		confirmAddrText
//    Label		otherHeadLabel
//    Text		otherHeadText
//    Label		sendmailLabel
//    TextField		sendmailTF
//    ToggleButton	deadTB
//    TextField		deadTF
//
   Widget fromHeadLabel  = XmCreateLabel    (*fieldRC, "fromHeadLabel",   0,0);
   fromHeadTF            = CreateTextField  (*fieldRC, "fromHeadTF",      0,0);
   Widget mailTypeLabel  = XmCreateLabel    (*fieldRC, "mailTypeLabel",   0,0);
   Widget mailTypeFrame  = XmCreateFrame    (*fieldRC, "mailTypeFrame",   0,0);
   Widget textTypeLabel  = XmCreateLabel    (*fieldRC, "textTypeLabel",   0,0);
   Widget textTypeFrame  = XmCreateFrame    (*fieldRC, "textTypeFrame",   0,0);
   Widget charLabel      = XmCreateLabel    (*fieldRC, "charLabel",       0,0);
   Widget charForm       = XmCreateForm     (*fieldRC, "charForm",        0,0);
   Widget bodyEncLabel   = XmCreateLabel    (*fieldRC, "bodyEncLabel",    0,0);
   Widget bodyEncFrame   = XmCreateFrame    (*fieldRC, "bodyEncFrame",    0,0);
   Widget headEncLabel   = XmCreateLabel    (*fieldRC, "headEncLabel",    0,0);
   Widget headEncFrame   = XmCreateFrame    (*fieldRC, "headEncFrame",    0,0);
   Widget fccLabel       = XmCreateLabel    (*fieldRC, "fccLabel",        0,0);
   Widget fccFrame       = XmCreateFrame    (*fieldRC, "fccFrame",        0,0);
   confirmAddrTB      = XmCreateToggleButton(*fieldRC, "confirmAddrTB",   0,0);
   args.Reset();
   args.EditMode(XmMULTI_LINE_EDIT);
   confirmAddrText       = CreateText(*fieldRC, "confirmAddrText", ARGS);
   Widget otherHeadLabel = XmCreateLabel    (*fieldRC, "otherHeadLabel",  0,0);
   args.Reset();
   args.EditMode(XmMULTI_LINE_EDIT);
   otherHeadText         = CreateText(*fieldRC, "otherHeadText",   ARGS);
   Widget sendmailLabel  = XmCreateLabel    (*fieldRC, "sendmailLabel",   0,0);
   sendmailTF            = CreateTextField(*fieldRC, "sendmailTF",      0,0);
   deadTB             = XmCreateToggleButton(*fieldRC, "deadTB",          0,0);
   deadTF             = CreateTextField   (*fieldRC, "deadTF",          0,0);

//
// Create mailTypeFrame hierarchy
//
// mailTypeFrame
//    RadioBox		mailTypeRadio
//       ToggleButton      mailPlainTB
//       ToggleButton      mailMimeTB
//       ToggleButton      mailAltTB
//
   args.Reset();
   args.Packing(XmPACK_TIGHT);
   args.Orientation(XmHORIZONTAL);
   Widget mailTypeRadio = XmCreateRadioBox(mailTypeFrame, "mailTypeRadio",ARGS);

   mailPlainTB    = XmCreateToggleButton(mailTypeRadio, "mailPlainTB",    0,0);
   mailMimeTB     = XmCreateToggleButton(mailTypeRadio, "mailMimeTB",     0,0);
   mailAltTB      = XmCreateToggleButton(mailTypeRadio, "mailAltTB",      0,0);

   wlist[0] = mailPlainTB;
   wlist[1] = mailMimeTB;
   wlist[2] = mailAltTB;
   XtManageChildren(wlist, 3);       // mailTypeRadio children

   XtManageChild(mailTypeRadio);  // mailTypeFrame children

//
// Create textTypeFrame hierarchy
//
// textTypeFrame
//    RadioBox		textTypeRadio
//       ToggleButton      textPlainTB
//       ToggleButton      textRichTB
//
   args.Reset();
   args.Packing(XmPACK_TIGHT);
   args.Orientation(XmHORIZONTAL);
   Widget textTypeRadio = XmCreateRadioBox(textTypeFrame, "textTypeRadio",ARGS);

   textPlainTB = XmCreateToggleButton(textTypeRadio, "textPlainTB", 0,0);
   textRichTB  = XmCreateToggleButton(textTypeRadio, "textRichTB",  0,0);

   wlist[0] = textPlainTB;
   wlist[1] = textRichTB;
   XtManageChildren(wlist, 2);       // textTypeRadio children

   XtManageChild(textTypeRadio);  // textTypeFrame children

//
// Create charForm hierarchy
//
// charForm
//    TextField		charTF
//    MenuBar		charMB
//       CascadeButton	charCB
//       PulldownMenu	charPD
//          PushButton	charAsciiPB
//          PushButton	charIso1PB
//          PushButton	charIso2PB
//          PushButton	charIso3PB
//          PushButton	charIso4PB
//          PushButton	charIso5PB
//          PushButton	charIso6PB
//          PushButton	charIso7PB
//          PushButton	charIso8PB
//          PushButton	charIso9PB
//          PushButton	charIso13PB
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_NONE);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   Widget charMB = XmCreateMenuBar(charForm, "charMB", ARGS);

   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_WIDGET, charMB);
   args.Value("us-ascii");
   charTF = CreateTextField(charForm, "charTF", ARGS);

   Widget charPD = XmCreatePulldownMenu(charMB, "charPD", 0,0);

   charAsciiPB = XmCreatePushButton(charPD, "us-ascii",   0,0);
   charIso1PB  = XmCreatePushButton(charPD, "iso-8859-1", 0,0);
   charIso2PB  = XmCreatePushButton(charPD, "iso-8859-2", 0,0);
   charIso3PB  = XmCreatePushButton(charPD, "iso-8859-3", 0,0);
   charIso4PB  = XmCreatePushButton(charPD, "iso-8859-4", 0,0);
   charIso5PB  = XmCreatePushButton(charPD, "iso-8859-5", 0,0);
   charIso6PB  = XmCreatePushButton(charPD, "iso-8859-6", 0,0);
   charIso7PB  = XmCreatePushButton(charPD, "iso-8859-7", 0,0);
   charIso8PB  = XmCreatePushButton(charPD, "iso-8859-8", 0,0);
   charIso9PB  = XmCreatePushButton(charPD, "iso-8859-9", 0,0);
   charIso13PB  = XmCreatePushButton(charPD, "iso-8859-13", 0,0);

   XtAddCallback(charAsciiPB, XmNactivateCallback, (XtCallbackProc)DoCharset,
   		 this);
   XtAddCallback(charIso1PB,  XmNactivateCallback, (XtCallbackProc)DoCharset,
   		 this);
   XtAddCallback(charIso2PB,  XmNactivateCallback, (XtCallbackProc)DoCharset,
   		 this);
   XtAddCallback(charIso3PB,  XmNactivateCallback, (XtCallbackProc)DoCharset,
   		 this);
   XtAddCallback(charIso4PB,  XmNactivateCallback, (XtCallbackProc)DoCharset,
   		 this);
   XtAddCallback(charIso5PB,  XmNactivateCallback, (XtCallbackProc)DoCharset,
   		 this);
   XtAddCallback(charIso6PB,  XmNactivateCallback, (XtCallbackProc)DoCharset,
   		 this);
   XtAddCallback(charIso7PB,  XmNactivateCallback, (XtCallbackProc)DoCharset,
   		 this);
   XtAddCallback(charIso8PB,  XmNactivateCallback, (XtCallbackProc)DoCharset,
   		 this);
   XtAddCallback(charIso9PB,  XmNactivateCallback, (XtCallbackProc)DoCharset,
   		 this);
   XtAddCallback(charIso13PB,  XmNactivateCallback, (XtCallbackProc)DoCharset,
   		 this);

   wlist[0] = charAsciiPB;
   wlist[1] = charIso1PB;
   wlist[2] = charIso2PB;
   wlist[3] = charIso3PB;
   wlist[4] = charIso4PB;
   wlist[5] = charIso5PB;
   wlist[6] = charIso6PB;
   wlist[7] = charIso7PB;
   wlist[8] = charIso8PB;
   wlist[9] = charIso9PB;
   wlist[10] = charIso13PB;
   XtManageChildren(wlist, 11);		// charPD children

   args.Reset();
   args.SubMenuId(charPD);
   Widget charCB = XmCreateCascadeButton(charMB, "charCB", ARGS);
   XtManageChild(charCB);

   wlist[0] = charMB;
   wlist[1] = charTF;
   XtManageChildren(wlist, 2);	// charForm children

//
// Create fccFrame hierarchy
//
// fccFrame
//    RowColumn		fccRC
//       ToggleButton	   noFccTB
//       Form		   fccFolderForm
//       Form		   fccPatternForm
//
   args.Reset();
   args.Orientation(XmVERTICAL);
   args.Packing(XmPACK_TIGHT);
   Widget fccRC = XmCreateRowColumn(fccFrame, "fccRC", ARGS);

   args.Reset();
   args.IndicatorType(XmONE_OF_MANY);
   noFccTB = XmCreateToggleButton(fccRC, "noFccTB", ARGS);
   XtManageChild(noFccTB);

   XtAddCallback(noFccTB, XmNvalueChangedCallback, (XtCallbackProc)FccChanged,
   		 this);

   Widget fccFolderForm  = XmCreateForm(fccRC, "fccFolderForm",  0,0);
   Widget fccPatternForm = XmCreateForm(fccRC, "fccPatternForm", 0,0);

//
// Create fccFolderForm hierarchy
//
// fccFolderForm
//    ToggleButton	fccFolderTB
//    TextField		fccFolderTF
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   args.IndicatorType(XmONE_OF_MANY);
   fccFolderTB = XmCreateToggleButton(fccFolderForm, "fccFolderTB", ARGS);
   XtManageChild(fccFolderTB);

   XtAddCallback(fccFolderTB, XmNvalueChangedCallback,
   		 (XtCallbackProc)FccChanged, this);

   args.LeftAttachment(XmATTACH_WIDGET, fccFolderTB);
   args.RightAttachment(XmATTACH_FORM);
   fccFolderTF = CreateTextField(fccFolderForm, "fccFolderTF", ARGS);
   XtManageChild(fccFolderTF);
   XtManageChild(fccFolderForm);

   XtAddCallback(fccFolderTF, XmNvalueChangedCallback,
   		 (XtCallbackProc)AutoSelectFolder, this);

//
// Create fccPatternForm hierarchy
//
// fccPatternForm
//    ToggleButton	fccPatternTB
//    OptionMenu	fccPatternOM
//    PulldownMenu	fccPatternPD
//    Label		fccPatternLabel
//    TextField		fccPatternTF
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   args.IndicatorType(XmONE_OF_MANY);
   fccPatternTB = XmCreateToggleButton(fccPatternForm, "fccPatternTB", ARGS);
   XtManageChild(fccPatternTB);

   XtAddCallback(fccPatternTB, XmNvalueChangedCallback,
   		 (XtCallbackProc)FccChanged, this);

   Widget fccPatternPD
		  = XmCreatePulldownMenu(fccPatternForm, "fccPatternPD", 0,0);

   args.LeftAttachment(XmATTACH_WIDGET, fccPatternTB);
   args.SubMenuId(fccPatternPD);
   fccPatternOM = XmCreateOptionMenu(fccPatternForm, "fccPatternOM", ARGS);
   XtManageChild(fccPatternOM);

   args.LeftAttachment(XmATTACH_WIDGET, fccPatternOM);
   Widget fccPatternLabel
   		  = XmCreateLabel(fccPatternForm, "fccPatternLabel", ARGS);
   XtManageChild(fccPatternLabel);

   args.LeftAttachment(XmATTACH_WIDGET, fccPatternLabel);
   args.RightAttachment(XmATTACH_FORM);
   fccPatternTF = CreateTextField(fccPatternForm, "fccPatternTF", ARGS);
   XtManageChild(fccPatternTF);
   XtManageChild(fccPatternForm);

   XtAddCallback(fccPatternTF, XmNvalueChangedCallback,
   		 (XtCallbackProc)AutoSelectPattern, this);

//
// Create fccPatternPD hierarchy
//
// fccPatternPD
//    PushButton	fccUserPB
//    PushButton	fccAddrPB
//    PushButton	fccYearPB
//    PushButton	fccMonthPB
//    PushButton	fccWeekPB
//    PushButton	fccDayPB
//
   fccUserPB  = XmCreatePushButton(fccPatternPD, "fccUserPB",  0,0);
   fccAddrPB  = XmCreatePushButton(fccPatternPD, "fccAddrPB",  0,0);
   fccYearPB  = XmCreatePushButton(fccPatternPD, "fccYearPB",  0,0);
   fccMonthPB = XmCreatePushButton(fccPatternPD, "fccMonthPB", 0,0);
   fccWeekPB  = XmCreatePushButton(fccPatternPD, "fccWeekPB",  0,0);
   fccDayPB   = XmCreatePushButton(fccPatternPD, "fccDayPB",   0,0);

   XtAddCallback(fccUserPB, XmNactivateCallback,
   		 (XtCallbackProc)AutoSelectPattern, this);
   XtAddCallback(fccAddrPB, XmNactivateCallback,
   		 (XtCallbackProc)AutoSelectPattern, this);
   XtAddCallback(fccYearPB, XmNactivateCallback,
   		 (XtCallbackProc)AutoSelectPattern, this);
   XtAddCallback(fccMonthPB, XmNactivateCallback,
   		 (XtCallbackProc)AutoSelectPattern, this);
   XtAddCallback(fccWeekPB, XmNactivateCallback,
   		 (XtCallbackProc)AutoSelectPattern, this);
   XtAddCallback(fccDayPB, XmNactivateCallback,
   		 (XtCallbackProc)AutoSelectPattern, this);

   wlist[0] = fccUserPB;
   wlist[1] = fccAddrPB;
   wlist[2] = fccYearPB;
   wlist[3] = fccMonthPB;
   wlist[4] = fccWeekPB;
   wlist[5] = fccDayPB;
   XtManageChildren(wlist, 6);

   XtManageChild(fccRC);

//
// Create bodyEncFrame hierarchy
//
// bodyEncFrame
//    RadioBox		bodyEncRadio
//       ToggleButton      bodyEnc8bitTB
//       ToggleButton      bodyEncQpTB
//
   args.Reset();
   args.Orientation(XmHORIZONTAL);
   args.Packing(XmPACK_TIGHT);
   Widget bodyEncRadio = XmCreateRadioBox(bodyEncFrame, "bodyEncRadio",ARGS);

   bodyEnc8bitTB = XmCreateToggleButton(bodyEncRadio, "bodyEnc8bitTB", 0,0);
   bodyEncQpTB   = XmCreateToggleButton(bodyEncRadio, "bodyEncQpTB",   0,0);

   wlist[0] = bodyEnc8bitTB;
   wlist[1] = bodyEncQpTB;
   XtManageChildren(wlist, 2);       // bodyEncRadio children

   XtManageChild(bodyEncRadio);  // bodyEncFrame children

//
// Create headEncFrame hierarchy
//
// headEncFrame
//    RadioBox		headEncRadio
//       ToggleButton      headEncNoneTB
//       ToggleButton      headEncQTB
//       ToggleButton      headEncBTB
//
   args.Reset();
   args.Orientation(XmHORIZONTAL);
   args.Packing(XmPACK_TIGHT);
   Widget headEncRadio = XmCreateRadioBox(headEncFrame, "headEncRadio",ARGS);

   headEncNoneTB = XmCreateToggleButton(headEncRadio, "headEncNoneTB", 0,0);
   headEncQTB    = XmCreateToggleButton(headEncRadio, "headEncQTB",    0,0);
   headEncBTB    = XmCreateToggleButton(headEncRadio, "headEncBTB",    0,0);

   wlist[0] = headEncNoneTB;
   wlist[1] = headEncQTB;
   wlist[2] = headEncBTB;
   XtManageChildren(wlist, 3);       // headEncRadio children

   XtManageChild(headEncRadio);  // headEncFrame children

   checkRC->Defer(False);

   wlist[0] = *fieldRC;
   wlist[1] = splitForm;
   wlist[2] = *checkRC;
   XtManageChildren(wlist, 3);	// appForm children

   HandleHelp();

   XtAddCallback(shell, XmNpopupCallback, (XtCallbackProc)DoPopup,
                 (XtPointer)this);

} // End constructor

/*---------------------------------------------------------------
 *  Destructor
 */

MailPrefWinC::~MailPrefWinC()
{
   delete fieldRC;
   delete checkRC;
}

/*---------------------------------------------------------------
 *  Callback routine to handle initial display.
 */

void
MailPrefWinC::DoPopup(Widget, MailPrefWinC *This, XtPointer)
{
   XtRemoveCallback(This->shell, XmNpopupCallback, (XtCallbackProc)DoPopup,
		    (XtPointer)This);

//
// Finish initializing fieldRC
//
   WidgetList	wlist;
   Cardinal	wcount;
   XtVaGetValues(*This->fieldRC, XmNchildren, &wlist, XmNnumChildren, &wcount,
   		 NULL);
   This->fieldRC->SetChildren(wlist, wcount);

   This->fieldRC->SetRowResize(2, True);	// confirmAddr row
   This->fieldRC->SetRowResize(3, True);	// otherHead row
   This->fieldRC->SetRowHeightAdjust(2, RcADJUST_ATTACH);
   This->fieldRC->SetRowHeightAdjust(3, RcADJUST_ATTACH);

   This->fieldRC->Defer(False);
}

/*---------------------------------------------------------------
 *  Method to handle display
 */

void
MailPrefWinC::Show(Widget parent)
{
   MailPrefC	*prefs = ishApp->mailPrefs;

//
// See if this widget corresponds to a particular composition window
//
   SendWinC	*sendWin = NULL;
   if ( parent != (Widget)*ishApp->mainWin ) {
      u_int	count = ishApp->sendWinList.size();
      for (int i=0; !sendWin && i<count; i++) {
	 SendWinC	*win = (SendWinC*)*ishApp->sendWinList[i];
	 if ( parent == (Widget)*win ) sendWin = win;
      }
   }

//
// Initialize settings
//
   if ( sendWin )
      XmToggleButtonSetState(checkAddrTB, sendWin->CheckingAddresses(), False);
   else
      XmToggleButtonSetState(checkAddrTB, prefs->verifyAddresses, False);

   StringC	str;
   str += prefs->splitSize;
   XmTextFieldSetString(splitTF, str);
   XmToggleButtonSetState(splitTB, prefs->split, False);

   Widget	hist = NULL;
   Widget	tb   = NULL;
   switch ( prefs->fccType ) {

      case (FCC_TO_FOLDER):	tb = fccFolderTB;			break;
      case (FCC_BY_USER):	tb = fccPatternTB; hist = fccUserPB;	break;
      case (FCC_BY_ADDRESS):	tb = fccPatternTB; hist = fccAddrPB;	break;
      case (FCC_BY_YEAR):	tb = fccPatternTB; hist = fccYearPB;	break;
      case (FCC_BY_MONTH):	tb = fccPatternTB; hist = fccMonthPB;	break;
      case (FCC_BY_WEEK):	tb = fccPatternTB; hist = fccWeekPB;	break; 
      case (FCC_BY_DAY):	tb = fccPatternTB; hist = fccDayPB;	break; 
      case (FCC_NONE):
      default:			tb = noFccTB;				break;

   } // End switch fccType

   XmTextFieldSetString(fccFolderTF,  prefs->OrigFccFolder());
   XmTextFieldSetString(fccPatternTF, prefs->OrigFccFolderDir());
   if ( hist ) XtVaSetValues(fccPatternOM, XmNmenuHistory, hist, NULL);
   XmToggleButtonSetState(tb, True, True);

   XmTextFieldSetString(fromHeadTF, prefs->fromHeader);

   if ( sendWin ) {

      switch ( sendWin->MailType() ) {
	 case (MAIL_PLAIN):
	    XmToggleButtonSetState(mailPlainTB, True, True);
	    break;
	 case (MAIL_MIME):
	    XmToggleButtonSetState(mailMimeTB, True, True);
	    break;
	 case (MAIL_ALT):
	    XmToggleButtonSetState(mailAltTB, True, True);
	    break;
      }

      switch ( sendWin->TextType() ) {
	 case (CT_PLAIN):
	    XmToggleButtonSetState(textPlainTB, True, True);
	    break;
	 case (CT_ENRICHED):
	    XmToggleButtonSetState(textRichTB, True, True);
	    break;
      }
   }

   else {

      switch ( prefs->mailType ) {
	 case (MAIL_PLAIN):
	    XmToggleButtonSetState(mailPlainTB, True, True);
	    break;
	 case (MAIL_MIME):
	    XmToggleButtonSetState(mailMimeTB, True, True);
	    break;
	 case (MAIL_ALT):
	    XmToggleButtonSetState(mailAltTB, True, True);
	    break;
      }

      switch ( prefs->textType ) {
	 case (CT_PLAIN):
	    XmToggleButtonSetState(textPlainTB, True, True);
	    break;
	 case (CT_ENRICHED):
	    XmToggleButtonSetState(textRichTB, True, True);
	    break;
      }

   }

   XmTextFieldSetString(charTF, prefs->charset);

   switch ( prefs->bodyEncType ) {
      case (ET_8BIT):
	 XmToggleButtonSetState(bodyEnc8bitTB, True, True);
	 break;
      default:
	 XmToggleButtonSetState(bodyEncQpTB, True, True);
	 break;
   }

   switch ( prefs->headEncType ) {
      case (ET_QP):
	 XmToggleButtonSetState(headEncQTB, True, True);
	 break;
      case (ET_BASE_64):
	 XmToggleButtonSetState(headEncBTB, True, True);
	 break;
      default:
	 XmToggleButtonSetState(headEncNoneTB, True, True);
	 break;
   }

   XmToggleButtonSetState(deadTB, prefs->saveOnInterrupt, False);
   XmTextFieldSetString  (deadTF, prefs->OrigDeadFile());

   XmTextFieldSetString(sendmailTF, prefs->OrigSendmailCmd());

   XmToggleButtonSetState(confirmAddrTB, prefs->confirmAddrs, False);
   TextSetList(confirmAddrText, prefs->confirmAddrList);
   TextSetString(otherHeadText, prefs->otherHeaders);

   OptWinC::Show(parent);

} // End Show

void
MailPrefWinC::Show()
{
   Show(XtParent(*this));
}

/*---------------------------------------------------------------
 *  Method to apply settings
 */

Boolean
MailPrefWinC::Apply()
{
   MailPrefC	*prefs = ishApp->mailPrefs;

   char	*cs;

//
// Get split data
//
   Boolean	doSplit = XmToggleButtonGetState(splitTB);
   int		splitSize = 100000;
   cs = XmTextFieldGetString(splitTF);
   if ( strlen(cs) > 0 ) splitSize = atoi(cs);
   XtFree(cs);

//
// Get split size
//
   if ( doSplit && splitSize < 10000 ) {
      set_invalid(splitTF, True, True);
      PopupMessage("Minimum split size is 10000");
      return False;
   }

   Boolean	doDead = XmToggleButtonGetState(deadTB);
   cs = XmTextFieldGetString(deadTF);
   StringC	deadStr(cs);
   XtFree(cs);

   if ( doDead && deadStr.size() <= 0 ) {
      set_invalid(deadTF, True, True);
      PopupMessage("Please enter a file name for interrupted messages.");
      return False;
   }

//
// Get fcc type
//
   StringC	fccStr, origFccStr;
   if ( XmToggleButtonGetState(fccFolderTB) ) {

      cs = XmTextFieldGetString(fccFolderTF);
      origFccStr = cs;
      XtFree(cs);

      if ( origFccStr.size() == 0 ) {
	 set_invalid(fccFolderTF, True, True);
	 PopupMessage("Please enter a folder name.");
	 return False;
      }

      fccStr = origFccStr;
      ishApp->ExpandFolderName(fccStr);
   }

   else if ( XmToggleButtonGetState(fccPatternTB) ) {

      cs = XmTextFieldGetString(fccPatternTF);
      origFccStr = cs;
      XtFree(cs);

      if ( origFccStr.size() == 0 ) {
	 set_invalid(fccPatternTF, True, True);
	 PopupMessage("Please enter a directory name.");
	 return False;
      }

      fccStr = origFccStr;
      ishApp->ExpandFolderName(fccStr);

//
// Check the status of the specified name
//
      if ( access(fccStr, W_OK) != 0 ) {

//
// Create the directory if it doesn't exist
//
	 if ( errno == ENOENT ) {

	    StringC	errStr;
	    if ( !MakeDir(fccStr) ) {
	       set_invalid(fccPatternTF, True, True);
	       return False;
	    }

	 } // End if name does not exist

//
// Make sure this is a directory
//
	 else if ( !IsDir(fccStr) ) {

	    set_invalid(fccPatternTF, True, True);
	    fccStr += " exists but is not a directory.";
	    PopupMessage(fccStr);
	    return False;

	 } // End if name not a directory

//
// The directory is not writable
//
	 else {
	    set_invalid(fccPatternTF, True, True);
	    fccStr += " exists but is not writable.";
	    PopupMessage(fccStr);
	    return False;
	 }

      } // End if name not writable

//
// Make sure this is a directory
//
      else if ( !IsDir(fccStr) ) {

	 set_invalid(fccPatternTF, True, True);
	 fccStr += " exists but is not a directory.";
	 PopupMessage(fccStr);
	 return False;

      } // End if name not a directory

   } // End if using pattern

   BusyCursor(True);

//
// Set the "From:" header
//
   cs = XmTextFieldGetString(fromHeadTF);
   prefs->fromHeader = cs;
   XtFree(cs);

   prefs->verifyAddresses = XmToggleButtonGetState(checkAddrTB);
   u_int	count = ishApp->sendWinList.size();
   int	i;
   for (i=0; i<count; i++) {
      SendWinC  *sendWin = (SendWinC*)*ishApp->sendWinList[i];
      sendWin->SetAddressChecking(prefs->verifyAddresses);
   }

   prefs->split     = doSplit;
   prefs->splitSize = splitSize;

   if ( XmToggleButtonGetState(fccFolderTB) ) {
      prefs->fccType        = FCC_TO_FOLDER;
      prefs->SetFccFolder(origFccStr);
   }

   else if ( XmToggleButtonGetState(fccPatternTB) ) {

      Widget	hist;
      XtVaGetValues(fccPatternOM, XmNmenuHistory, &hist, NULL);

      if      ( hist == fccUserPB  ) prefs->fccType = FCC_BY_USER;
      else if ( hist == fccAddrPB  ) prefs->fccType = FCC_BY_ADDRESS;
      else if ( hist == fccYearPB  ) prefs->fccType = FCC_BY_YEAR;
      else if ( hist == fccMonthPB ) prefs->fccType = FCC_BY_MONTH;
      else if ( hist == fccWeekPB  ) prefs->fccType = FCC_BY_WEEK;
      else			     prefs->fccType = FCC_BY_DAY;

      prefs->SetFccFolderDir(origFccStr);
   }

   else {
      prefs->fccType = FCC_NONE;
   }

   if ( XmToggleButtonGetState(mailPlainTB) )
      prefs->mailType = MAIL_PLAIN;
   else if ( XmToggleButtonGetState(mailAltTB) )
      prefs->mailType = MAIL_ALT;
   else
      prefs->mailType = MAIL_MIME;

   if ( XmToggleButtonGetState(textPlainTB) )
      prefs->textType = CT_PLAIN;
   else
      prefs->textType = CT_ENRICHED;

   count = ishApp->sendWinList.size();
   for (i=0; i<count; i++) {

      SendWinC	*sendWin = (SendWinC*)*ishApp->sendWinList[i];
      sendWin->UpdateFcc();
      sendWin->SetMailType(prefs->mailType);
      sendWin->SetTextType(prefs->textType);
   }

//
// Set default character set
//
   cs = XmTextFieldGetString(charTF);
   if ( strlen(cs) > 0 ) prefs->charset = cs;
   else			 prefs->charset = "us-ascii";
   XtFree(cs);

//
// Set default 8-bit encoding
//
   if ( XmToggleButtonGetState(bodyEnc8bitTB) )
      prefs->bodyEncType = ET_8BIT;
   else
      prefs->bodyEncType = ET_QP;

   if ( XmToggleButtonGetState(headEncQTB) )
      prefs->headEncType = ET_QP;
   else if ( XmToggleButtonGetState(headEncBTB) )
      prefs->headEncType = ET_BASE_64;
   else
      prefs->headEncType = ET_NONE;

//
// Set dead letter file
//
   prefs->saveOnInterrupt = doDead;
   prefs->SetDeadFile(deadStr);

//
// Set sendmail command
//
   cs = XmTextFieldGetString(sendmailTF);
   prefs->SetSendmailCmd(cs);
   XtFree(cs);

//
// Set confirmation addresses
//
   prefs->confirmAddrs = XmToggleButtonGetState(confirmAddrTB);
   prefs->confirmAddrList.removeAll();
   StringC	tmp;
   tmp = XmTextGetString(confirmAddrText);
   ExtractList(tmp, prefs->confirmAddrList);

//
// Set other headers
//
   prefs->otherHeaders.Clear();
   cs = XmTextGetString(otherHeadText);
   prefs->otherHeaders = cs;
   XtFree(cs);

   prefs->WriteDatabase();

//
// Write to file if necessary
//
   if ( applyAll ) prefs->WriteFile();

   BusyCursor(False);

   return True;

} // End Apply

/*---------------------------------------------------------------
 *  Callback to implement radio button behavior in fccFrame
 */

void
MailPrefWinC::FccChanged(Widget tb, MailPrefWinC* This,
			 XmToggleButtonCallbackStruct *data)
{
//
// Don't allow buttons to be turned off by the user
//
   if ( !data->set ) {
      XmToggleButtonSetState(tb, True, False);
      return;
   }

   if ( tb == This->noFccTB ) {
      XmToggleButtonSetState(This->fccFolderTB,  False, False);
      XmToggleButtonSetState(This->fccPatternTB, False, False);
   }
   else if ( tb == This->fccFolderTB ) {
      XmToggleButtonSetState(This->noFccTB,      False, False);
      XmToggleButtonSetState(This->fccPatternTB, False, False);
   }
   else {
      XmToggleButtonSetState(This->noFccTB,      False, False);
      XmToggleButtonSetState(This->fccFolderTB,  False, False);
   }

} // End FccChanged

/*---------------------------------------------------------------
 *  Callbacks to automatically select toggle buttons when field is
 *     changed or type is selected
 */

void
MailPrefWinC::AutoSelectFolder(Widget, MailPrefWinC* This, XtPointer)
{
   XmToggleButtonSetState(This->fccFolderTB, True, True);
}

void
MailPrefWinC::AutoSelectPattern(Widget, MailPrefWinC* This, XtPointer)
{
   XmToggleButtonSetState(This->fccPatternTB, True, True);
}

/*---------------------------------------------------------------
 *  Callback to handle press of charset button
 */

void
MailPrefWinC::DoCharset(Widget w, MailPrefWinC *This, XtPointer)
{
   char	*cs = "us-ascii";

        if ( w == This->charIso1PB ) cs = "iso-8859-1";
   else if ( w == This->charIso2PB ) cs = "iso-8859-2";
   else if ( w == This->charIso3PB ) cs = "iso-8859-3";
   else if ( w == This->charIso4PB ) cs = "iso-8859-4";
   else if ( w == This->charIso5PB ) cs = "iso-8859-5";
   else if ( w == This->charIso6PB ) cs = "iso-8859-6";
   else if ( w == This->charIso7PB ) cs = "iso-8859-7";
   else if ( w == This->charIso8PB ) cs = "iso-8859-8";
   else if ( w == This->charIso9PB ) cs = "iso-8859-9";
   else if ( w == This->charIso13PB ) cs = "iso-8859-13";

   XmTextFieldSetString(This->charTF, cs);

} // End DoCharset

