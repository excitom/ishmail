/*
 *  $Id: CompPrefWinC.C,v 1.2 2000/05/07 12:26:11 fnevgeny Exp $
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
#include "CompPrefWinC.h"
#include "CompPrefC.h"
#include "IshAppC.h"
#include "SendWinC.h"
#include "MainWinC.h"

#include <hgl/WArgList.h>
#include <hgl/rsrc.h>
#include <hgl/RowColC.h>
#include <hgl/TextMisc.h>
#include <hgl/MimeRichTextC.h>

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
						       // historical name
CompPrefWinC::CompPrefWinC(Widget par) : OptWinC(par, "sendPrefWin")
{
   WArgList	args;
   Widget 	wlist[24];

//
// Create appForm hierarchy
//
// appForm
//    RowColC		sizeRC
//    ToggleButton	spaceTB
//    Form		maxFieldsForm
//    RowColC		fieldRC
//    
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_NONE);
   sizeRC = new RowColC(appForm, "sizeRC", ARGS);

   args.TopAttachment(XmATTACH_WIDGET, *sizeRC);
   spaceTB = XmCreateToggleButton(appForm, "spaceTB", ARGS);

   args.TopAttachment(XmATTACH_WIDGET, spaceTB);
   args.RightAttachment(XmATTACH_FORM);
   Widget maxFieldsForm = XmCreateForm(appForm, "maxFieldsForm", ARGS);

   args.TopAttachment(XmATTACH_WIDGET, maxFieldsForm);
   args.BottomAttachment(XmATTACH_FORM);
   fieldRC = new RowColC(appForm, "fieldRC", ARGS);

//
// Set up 2 columns in sizeRC
//
   sizeRC->Defer(True);
   sizeRC->SetOrientation(RcROW_MAJOR);
   sizeRC->SetColCount(2);

//
// Create sizeRC hierarchy
//
// sizeRC
//    Form		bodyColForm
//    Form		bodyRowForm
//    Form		headRowForm
//    ToggleButton	wrapTB
//    ToggleButton	showCcTB
//    ToggleButton	showBccTB
//    ToggleButton	showFccTB
//    ToggleButton	showOtherTB
//    ToggleButton	emacsTB
//    ToggleButton	deleteTB
//
   Widget bodyColForm = XmCreateForm        (*sizeRC, "bodyColForm",    0,0);
   Widget bodyRowForm = XmCreateForm        (*sizeRC, "bodyRowForm",    0,0);
   Widget headRowForm = XmCreateForm        (*sizeRC, "headRowForm",    0,0);
   wrapTB             = XmCreateToggleButton(*sizeRC, "wrapTB",         0,0);
   showCcTB           = XmCreateToggleButton(*sizeRC, "showCcTB",       0,0);
   showBccTB          = XmCreateToggleButton(*sizeRC, "showBccTB",      0,0);
   showFccTB          = XmCreateToggleButton(*sizeRC, "showFccTB",      0,0);
   showOtherTB        = XmCreateToggleButton(*sizeRC, "showOtherTB",    0,0);
   emacsTB            = XmCreateToggleButton(*sizeRC, "emacsTB",        0,0);
   deleteTB           = XmCreateToggleButton(*sizeRC, "deleteTB",       0,0);

//
// Create bodyColForm hierarchy
//
// bodyColForm
//    Label		bodyColLabel
//    TextField		bodyColTF
//
   args.Reset();
   args.LeftAttachment(XmATTACH_NONE);
   args.RightAttachment(XmATTACH_FORM);
   args.TopAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   bodyColTF = CreateTextField(bodyColForm, "bodyColTF", ARGS);

   args.RightAttachment(XmATTACH_WIDGET, bodyColTF);
   Widget bodyColLabel = XmCreateLabel(bodyColForm, "bodyColLabel", ARGS);

   wlist[0] = bodyColLabel;
   wlist[1] = bodyColTF;
   XtManageChildren(wlist, 2);	// bodyColForm children

//
// Create bodyRowForm hierarchy
//
// bodyRowForm
//    Label		bodyRowLabel
//    TextField		bodyRowTF
//
   args.Reset();
   args.LeftAttachment(XmATTACH_NONE);
   args.RightAttachment(XmATTACH_FORM);
   args.TopAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   bodyRowTF = CreateTextField(bodyRowForm, "bodyRowTF", ARGS);

   args.RightAttachment(XmATTACH_WIDGET, bodyRowTF);
   Widget bodyRowLabel = XmCreateLabel(bodyRowForm, "bodyRowLabel", ARGS);

   wlist[0] = bodyRowLabel;
   wlist[1] = bodyRowTF;
   XtManageChildren(wlist, 2);	// bodyRowForm children

//
// Create headRowForm hierarchy
//
// headRowForm
//    Label		headRowLabel
//    TextField		headRowTF
//
   args.Reset();
   args.LeftAttachment(XmATTACH_NONE);
   args.RightAttachment(XmATTACH_FORM);
   args.TopAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   headRowTF = CreateTextField(headRowForm, "headRowTF", ARGS);

   args.RightAttachment(XmATTACH_WIDGET, headRowTF);
   Widget headRowLabel = XmCreateLabel(headRowForm, "headRowLabel", ARGS);

   wlist[0] = headRowLabel;
   wlist[1] = headRowTF;
   XtManageChildren(wlist, 2);	// headRowForm children

//
// Add sizeRC children
//
   int	wcount = 0;
   wlist[wcount++] = bodyColForm;
   wlist[wcount++] = bodyRowForm;
   wlist[wcount++] = headRowForm;
   wlist[wcount++] = wrapTB;
   wlist[wcount++] = showCcTB;
   wlist[wcount++] = showBccTB;
   wlist[wcount++] = showFccTB;
   wlist[wcount++] = showOtherTB;
   wlist[wcount++] = emacsTB;
   wlist[wcount++] = deleteTB;
   sizeRC->SetChildren(wlist, wcount);

//
// Create maxFieldsForm hierarchy
//
// maxFieldsForm
//    Label			maxFieldsLabel1
//    Frame			maxFieldsFrame
//       RadioBox		   maxFieldsRadio
//          ToggleButton	      maxFields1TB
//          ToggleButton	      maxFields2TB
//          ToggleButton	      maxFields3TB
//          ToggleButton	      maxFields4TB
//          ToggleButton	      maxFields5TB
//    Label			maxFieldsLabel2
//    
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   Widget maxFieldsLabel1 = XmCreateLabel(maxFieldsForm, "maxFieldsLabel1",
   					  ARGS);

   args.LeftAttachment(XmATTACH_WIDGET, maxFieldsLabel1);
   Widget maxFieldsFrame  = XmCreateFrame(maxFieldsForm, "maxFieldsFrame",
   					  ARGS);

   args.LeftAttachment(XmATTACH_WIDGET, maxFieldsFrame);
   Widget maxFieldsLabel2 = XmCreateLabel(maxFieldsForm, "maxFieldsLabel2",
   					  ARGS);

   args.Reset();
   args.Orientation(XmHORIZONTAL);
   args.Packing(XmPACK_TIGHT);
   Widget maxFieldsRadio = XmCreateRadioBox(maxFieldsFrame, "maxFieldsRadio",
					    ARGS);

   maxFields1TB = XmCreateToggleButton(maxFieldsRadio, "maxFields1TB", 0,0);
   maxFields2TB = XmCreateToggleButton(maxFieldsRadio, "maxFields2TB", 0,0);
   maxFields3TB = XmCreateToggleButton(maxFieldsRadio, "maxFields3TB", 0,0);
   maxFields4TB = XmCreateToggleButton(maxFieldsRadio, "maxFields4TB", 0,0);
   maxFields5TB = XmCreateToggleButton(maxFieldsRadio, "maxFields5TB", 0,0);

   wlist[0] = maxFields1TB;
   wlist[1] = maxFields2TB;
   wlist[2] = maxFields3TB;
   wlist[3] = maxFields4TB;
   wlist[4] = maxFields5TB;
   XtManageChildren(wlist, 5);       // maxFieldsRadio children

   XtManageChild(maxFieldsRadio);  // maxFieldsFrame children

   wlist[0] = maxFieldsLabel1;
   wlist[1] = maxFieldsFrame;
   wlist[2] = maxFieldsLabel2;
   XtManageChildren(wlist, 3);       // maxFieldsForm children

//
// Set up 2 columns in fieldRC
//
   fieldRC->Defer(True);
   fieldRC->SetOrientation(RcROW_MAJOR);
   fieldRC->SetColCount(2);
   fieldRC->SetColAlignment(0, XmALIGNMENT_END);
   fieldRC->SetColAlignment(1, XmALIGNMENT_CENTER);
   fieldRC->SetColWidthAdjust(0, RcADJUST_NONE);
   fieldRC->SetColWidthAdjust(1, RcADJUST_ATTACH);
   fieldRC->SetColResize(0, False);
   fieldRC->SetColResize(1, True);
   fieldRC->SetUniformRows(False);

//
// Create fieldRC hierarchy
//
// fieldRC
//    Label		editorLabel
//    TextField		editorTF
//    Label		spellLabel
//    TextField		spellTF
//    Label		digSignLabel
//    TextField		digSignTF
//    Label		encryptLabel
//    TextField		encryptTF
//    Label		encryptSignLabel
//    TextField		encryptSignTF
//    Label		mimeDigSignLabel
//    TextField		mimeDigSignTF
//    Label		mimeEncryptLabel
//    TextField		mimeEncryptTF
//    Label		mimeEncryptSignLabel
//    TextField		mimeEncryptSignTF
//    ToggleButton	autoSaveTB
//    Form		autoSaveForm
//
   Widget editorLabel    = XmCreateLabel  (*fieldRC, "editorLabel",       0,0);
   editorTF              = CreateTextField(*fieldRC, "editorTF",          0,0);
   Widget spellLabel     = XmCreateLabel  (*fieldRC, "spellLabel",        0,0);
   spellTF               = CreateTextField(*fieldRC, "spellTF",           0,0);
   Widget digSignLabel   = XmCreateLabel  (*fieldRC, "digSignLabel",      0,0);
   digSignTF             = CreateTextField(*fieldRC, "digSignTF",         0,0);
   Widget encryptLabel   = XmCreateLabel  (*fieldRC, "encryptLabel",      0,0);
   encryptTF             = CreateTextField(*fieldRC, "encryptTF",         0,0);
   Widget encryptSignLabel = XmCreateLabel(*fieldRC, "encryptSignLabel",  0,0);
   encryptSignTF         = CreateTextField(*fieldRC, "encryptSignTF",     0,0);
   Widget mimeDigSignLabel = XmCreateLabel(*fieldRC, "mimeDigSignLabel",  0,0);
   mimeDigSignTF         = CreateTextField(*fieldRC, "mimeDigSignTF",     0,0);
   Widget mimeEncryptLabel = XmCreateLabel(*fieldRC, "mimeEncryptLabel",  0,0);
   mimeEncryptTF         = CreateTextField(*fieldRC, "mimeEncryptTF",     0,0);
   Widget mimeEncryptSignLabel = XmCreateLabel(*fieldRC, "mimeEncryptSignLabel",
   									  0,0);
   mimeEncryptSignTF     = CreateTextField(*fieldRC, "mimeEncryptSignTF", 0,0);
   autoSaveTB		 = XmCreateToggleButton(*fieldRC, "autoSaveTB",   0,0);
   Widget autoSaveForm   = XmCreateForm   (*fieldRC, "autoSaveForm",      0,0);

//
// Create autoSaveForm hierarchy
//
// autoSaveForm
//    TextField		autoSaveRateTF
//    Label		autoSaveDirLabel
//    TextField		autoSaveDirTF
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   autoSaveRateTF = CreateTextField(autoSaveForm, "autoSaveRateTF", ARGS);

   args.LeftAttachment(XmATTACH_WIDGET, autoSaveRateTF);
   Widget autoSaveDirLabel =
			   XmCreateLabel(autoSaveForm, "autoSaveDirLabel",ARGS);

   args.LeftAttachment(XmATTACH_WIDGET, autoSaveDirLabel);
   args.RightAttachment(XmATTACH_FORM);
   autoSaveDirTF = CreateTextField(autoSaveForm, "autoSaveDirTF", ARGS);

   wcount = 0;
   wlist[wcount++] = autoSaveRateTF;
   wlist[wcount++] = autoSaveDirLabel;
   wlist[wcount++] = autoSaveDirTF;
   XtManageChildren(wlist, wcount);	// autoSaveForm children

   wcount = 0;
   wlist[wcount++] = editorLabel;
   wlist[wcount++] = editorTF;
   wlist[wcount++] = spellLabel;
   wlist[wcount++] = spellTF;
   wlist[wcount++] = digSignLabel;
   wlist[wcount++] = digSignTF;
   wlist[wcount++] = encryptLabel;
   wlist[wcount++] = encryptTF;
   wlist[wcount++] = encryptSignLabel;
   wlist[wcount++] = encryptSignTF;
   wlist[wcount++] = mimeDigSignLabel;
   wlist[wcount++] = mimeDigSignTF;
   wlist[wcount++] = mimeEncryptLabel;
   wlist[wcount++] = mimeEncryptTF;
   wlist[wcount++] = mimeEncryptSignLabel;
   wlist[wcount++] = mimeEncryptSignTF;
   wlist[wcount++] = autoSaveTB;
   wlist[wcount++] = autoSaveForm;
   fieldRC->SetChildren(wlist, wcount);

   wcount = 0;
   wlist[wcount++] = *sizeRC;
   wlist[wcount++] = spaceTB;
   wlist[wcount++] = maxFieldsForm;
   wlist[wcount++] = *fieldRC;
   XtManageChildren(wlist, wcount);	// appForm children

   sizeRC->Defer(False);
   fieldRC->Defer(False);

   HandleHelp();

} // End constructor

/*---------------------------------------------------------------
 *  Destructor
 */

CompPrefWinC::~CompPrefWinC()
{
   delete fieldRC;
   delete sizeRC;
}

/*---------------------------------------------------------------
 *  Method to handle display
 */

void
CompPrefWinC::Show(Widget parent)
{
   CompPrefC	*prefs = ishApp->compPrefs;

//
// See if this widget corresponds to a particular composition window
//
   sendWin = NULL;
   if ( parent != (Widget)*ishApp->mainWin ) {
      unsigned	count = ishApp->sendWinList.size();
      for (int i=0; !sendWin && i<count; i++) {
	 SendWinC	*win = (SendWinC*)*ishApp->sendWinList[i];
	 if ( parent == (Widget)*win ) sendWin = win;
      }
   }

//
// Initialize settings
//
   StringC str;
   if ( sendWin ) str += sendWin->ColumnCount();
   else		  str += prefs->bodyCols;
   XmTextFieldSetString(bodyColTF, str);

   str.Clear();
   if ( sendWin ) str += sendWin->BodyRowCount();
   else		  str += prefs->bodyRows;
   XmTextFieldSetString(bodyRowTF, str);

   str.Clear();
   if ( sendWin ) str += sendWin->HeadRowCount();
   else		  str += prefs->headRows;
   XmTextFieldSetString(headRowTF, str);

   if ( sendWin ) {
      XmToggleButtonSetState(wrapTB, sendWin->Wrapping(), False);
      XmToggleButtonSetState(showCcTB,       sendWin->ccVis,	False);
      XmToggleButtonSetState(showBccTB,      sendWin->bccVis,	False);
      XmToggleButtonSetState(showFccTB,	     sendWin->fccVis,	False);
      XmToggleButtonSetState(showOtherTB,    sendWin->otherVis,	False);
   }
   else {
      XmToggleButtonSetState(wrapTB,	     prefs->wrap,     False);
      XmToggleButtonSetState(showCcTB,	     prefs->showCc,       False);
      XmToggleButtonSetState(showBccTB,	     prefs->showBcc,      False);
      XmToggleButtonSetState(showFccTB,	     prefs->showFcc,      False);
      XmToggleButtonSetState(showOtherTB,    prefs->showOther,    False);
   }

   XmToggleButtonSetState(emacsTB,  prefs->emacsMode,	  False);
   XmToggleButtonSetState(deleteTB, prefs->delMeansBs,	  False);
   XmToggleButtonSetState(spaceTB,  prefs->spaceEndsAddr, False);

   Widget	tb = maxFields1TB;
   switch (prefs->maxFieldsPerLine) {
      case 2:	tb = maxFields2TB;	break;
      case 3:	tb = maxFields3TB;	break;
      case 4:	tb = maxFields4TB;	break;
      case 5:	tb = maxFields5TB;	break;
   }
   XmToggleButtonSetState(tb, True, True);

   XmTextFieldSetString(editorTF,           prefs->editCmd);
   XmTextFieldSetString(spellTF,            prefs->spellCmd);
   XmTextFieldSetString(digSignTF,          prefs->digSignCmd);
   XmTextFieldSetString(encryptTF,          prefs->encryptCmd);
   XmTextFieldSetString(encryptSignTF,      prefs->encryptSignCmd);
   XmTextFieldSetString(mimeDigSignTF,      prefs->mimeDigSignCmd);
   XmTextFieldSetString(mimeEncryptTF,      prefs->mimeEncryptCmd);
   XmTextFieldSetString(mimeEncryptSignTF,  prefs->mimeEncryptSignCmd);

   XmToggleButtonSetState(autoSaveTB, prefs->autoSave, True);
   str.Clear();
   str += prefs->autoSaveRate;
   TextFieldSetString(autoSaveRateTF, str);
   TextFieldSetString(autoSaveDirTF,  prefs->OrigAutoSaveDir());

   OptWinC::Show(parent);

} // End Show

void
CompPrefWinC::Show()
{
   Show(XtParent(*this));
}

/*---------------------------------------------------------------
 *  Method to apply settings
 */

Boolean
CompPrefWinC::Apply()
{
   CompPrefC	*prefs = ishApp->compPrefs;

//
// Get number of body rows
//
   char	*cs = XmTextFieldGetString(bodyRowTF);
   int	bodyRows = 0;
   if ( strlen(cs) > 0 ) bodyRows = atoi(cs);
   XtFree(cs);

   if ( bodyRows <= 0 ) {
      set_invalid(bodyRowTF, True, True);
      PopupMessage("Number of rows must be greater than 0");
      return False;
   }

//
// Get number of body columns
//
   cs = XmTextFieldGetString(bodyColTF);
   int	bodyCols = 0;
   if ( strlen(cs) > 0 ) bodyCols = atoi(cs);
   XtFree(cs);

   if ( bodyCols <= 0 ) {
      set_invalid(bodyColTF, True, True);
      PopupMessage("Number of columns must be greater than 0");
      return False;
   }

//
// Get number of rows in header fields
//
   cs = XmTextFieldGetString(headRowTF);
   int	headRows = 0;
   if ( strlen(cs) > 0 ) headRows = atoi(cs);
   XtFree(cs);

   if ( headRows <= 0 ) {
      set_invalid(headRowTF, True, True);
      PopupMessage("Number of rows must be greater than 0");
      return False;
   }

//
// Get auto save rate and directory
//
   Boolean	autoSave = XmToggleButtonGetState(autoSaveTB);
   cs = XmTextFieldGetString(autoSaveRateTF);
   int	autoSaveRate = 0;
   if ( strlen(cs) > 0 ) autoSaveRate = atoi(cs);
   XtFree(cs);

   if ( autoSave && autoSaveRate <= 0 ) {
      set_invalid(autoSaveRateTF, True, True);
      PopupMessage("The number of keystrokes must be greater than 0");
      return False;
   }

   cs = XmTextFieldGetString(autoSaveDirTF);
   StringC	autoSaveDir = cs;
   XtFree(cs);
   autoSaveDir.Trim();

   if ( autoSave && autoSaveDir.size() == 0 ) {
      set_invalid(autoSaveDirTF, True, True);
      StringC	msg("Please enter the name of a directory");
      msg += "\nwhere compositions can be saved";
      PopupMessage(msg);
      return False;
   }

   BusyCursor(True);

   Boolean	doWrap       = XmToggleButtonGetState(wrapTB);
   Boolean	showCc       = XmToggleButtonGetState(showCcTB);
   Boolean	showBcc      = XmToggleButtonGetState(showBccTB);
   Boolean	showFcc      = XmToggleButtonGetState(showFccTB);
   Boolean	showOther    = XmToggleButtonGetState(showOtherTB);
   Boolean	emacsMode    = XmToggleButtonGetState(emacsTB);
   Boolean	delMeansBs   = XmToggleButtonGetState(deleteTB);

   int	maxFields = 1;
   if      ( XmToggleButtonGetState(maxFields2TB) ) maxFields = 2;
   else if ( XmToggleButtonGetState(maxFields3TB) ) maxFields = 3;
   else if ( XmToggleButtonGetState(maxFields4TB) ) maxFields = 4;
   else if ( XmToggleButtonGetState(maxFields5TB) ) maxFields = 5;

//
// Update all composition windows
//
   Boolean	wrapChanged     = False;
   Boolean	sizeChanged     = False;
   Boolean	headSizeChanged = False;
   Boolean	headVisChanged  = False;
   Boolean	keysChanged     = False;

   if ( sendWin ) {
      wrapChanged = (doWrap != sendWin->Wrapping());
      sizeChanged     = (bodyRows   != sendWin->BodyRowCount() ||
			 bodyCols   != sendWin->ColumnCount());
      headSizeChanged = (headRows   != sendWin->HeadRowCount() ||
			 maxFields  != sendWin->maxFieldsPerLine);
      headVisChanged  = (showCc     != sendWin->ccVis  ||
			 showBcc    != sendWin->bccVis ||
			 showFcc    != sendWin->fccVis ||
			 showOther  != sendWin->otherVis);
   }

   else {
      wrapChanged     = (doWrap     != prefs->wrap);
      sizeChanged     = (bodyRows   != prefs->bodyRows ||
			 bodyCols   != prefs->bodyCols);
      headSizeChanged = (headRows   != prefs->headRows ||
			 maxFields  != prefs->maxFieldsPerLine);
      headVisChanged  = (showCc     != prefs->showCc  ||
			 showBcc    != prefs->showBcc ||
			 showFcc    != prefs->showFcc ||
			 showOther  != prefs->showOther);
   }

   keysChanged = (emacsMode  != prefs->emacsMode ||
		  delMeansBs != prefs->delMeansBs);

   u_int	count = ishApp->sendWinList.size();
   int	i;
   for (i=0; i<count; i++) {
      SendWinC	*sendWin = (SendWinC*)*ishApp->sendWinList[i];
      if ( sizeChanged ) sendWin->SetSize(bodyRows, bodyCols);
      if ( wrapChanged ) sendWin->SetWrap(doWrap);
      if ( keysChanged ) sendWin->SetKeys(emacsMode, delMeansBs);
   }

   prefs->bodyRows         = bodyRows;
   prefs->bodyCols         = bodyCols;
   prefs->headRows         = headRows;
   prefs->wrap             = doWrap;
   prefs->showCc           = showCc;
   prefs->showBcc          = showBcc;
   prefs->showFcc          = showFcc;
   prefs->showOther        = showOther;
   prefs->emacsMode        = emacsMode;
   prefs->delMeansBs       = delMeansBs;
   prefs->spaceEndsAddr    = XmToggleButtonGetState(spaceTB);
   prefs->maxFieldsPerLine = maxFields;
//   if ( prefs->spaceEndsAddr ) prefs->stripComments = True;

   if ( headVisChanged || headSizeChanged ) {
      count = ishApp->sendWinList.size();
      for (i=0; i<count; i++) {
	 SendWinC	*sendWin = (SendWinC*)*ishApp->sendWinList[i];
	 if ( headVisChanged )
	    sendWin->UpdateVisibleFields();
	 else
	    sendWin->PlaceHeaderFields();
      }
   }

//
// Set editor command
//
   cs = XmTextFieldGetString(editorTF);
   prefs->editCmd = cs;
   XtFree(cs);

//
// Set spell command
//
   cs = XmTextFieldGetString(spellTF);
   prefs->spellCmd = cs;
   XtFree(cs);

//
// Set digital signature and encryption commands
//
   cs = XmTextFieldGetString(digSignTF);
   prefs->digSignCmd = cs;
   XtFree(cs);

   cs = XmTextFieldGetString(encryptTF);
   prefs->encryptCmd = cs;
   XtFree(cs);

   cs = XmTextFieldGetString(encryptSignTF);
   prefs->encryptSignCmd = cs;
   XtFree(cs);

   cs = XmTextFieldGetString(mimeDigSignTF);
   prefs->mimeDigSignCmd = cs;
   XtFree(cs);

   cs = XmTextFieldGetString(mimeEncryptTF);
   prefs->mimeEncryptCmd = cs;
   XtFree(cs);

   cs = XmTextFieldGetString(mimeEncryptSignTF);
   prefs->mimeEncryptSignCmd = cs;
   XtFree(cs);

//
// Set auto save parameters
//
   prefs->autoSave     = autoSave;
   prefs->autoSaveRate = autoSaveRate;
   prefs->SetAutoSaveDir(autoSaveDir);

   prefs->WriteDatabase();

//
// Write to file if necessary
//
   if ( applyAll ) prefs->WriteFile();

   BusyCursor(False);

   return True;

} // End Apply
