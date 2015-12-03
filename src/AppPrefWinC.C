/*
 *  $Id: AppPrefWinC.C,v 1.3 2000/06/29 10:53:29 evgeny Exp $
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
#include "AppPrefWinC.h"
#include "IshAppC.h"
#include "AppPrefC.h"
#include "AlertPrefC.h"
#include "MainWinC.h"

#include <hgl/WArgList.h>
#include <hgl/rsrc.h>
#include <hgl/RowColC.h>
#include <hgl/HalAppC.h>
#include <hgl/TextMisc.h>
#include <hgl/CharC.h>

#include <Xm/Form.h>
#include <Xm/ToggleB.h>
#include <Xm/Label.h>
#include <Xm/TextF.h>
#include <Xm/Scale.h>
#include <Xm/Frame.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>

/*---------------------------------------------------------------
 *  Callback to play the bell when the volume value changes
 */

static void
SoundBell(Widget, XtPointer, XmScaleCallbackStruct *scale)
{
//
// Read bell volume.  Displayed in range 0-10, stored in range -100 to 100
//
   XBell(halApp->display, (scale->value - 5) * 20);
}

/*---------------------------------------------------------------
 *  Main window constructor
 */

							  // historical name
AppPrefWinC::AppPrefWinC(Widget parent) : OptWinC(parent, "prefWin")
{
   WArgList	args;
   Widget 	wlist[26];

//
// Create appForm hierarchy
//
// appForm
//    RowColC		checkRC
//    Form		archiveForm
//    RowColC		fieldRC
//    

   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_NONE);
   checkRC = new RowColC(appForm, "checkRC", ARGS);

   args.TopAttachment(XmATTACH_WIDGET, *checkRC);
   Widget archiveForm = XmCreateForm(appForm, "archiveForm", ARGS);

   args.TopAttachment(XmATTACH_WIDGET, archiveForm);
   args.BottomAttachment(XmATTACH_FORM);
   args.BorderWidth(1);
   fieldRC = new RowColC(appForm, "fieldRC", ARGS);

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
// prefRC
//    Form		inBoxForm
//    Form		checkForm
//    ToggleButton	alertTB
//    ToggleButton	showNewTB
//    ToggleButton	windowPosTB
//    ToggleButton	quickHelpTB
//    ToggleButton	delSaveTB
//    ToggleButton	hideDelTB
//    ToggleButton	newUnreadTB
//    Form		recentForm
//
   Widget inBoxForm   = XmCreateForm        (*checkRC, "inBoxForm",     0,0);
   Widget checkForm   = XmCreateForm        (*checkRC, "checkForm",     0,0);
   alertTB            = XmCreateToggleButton(*checkRC, "alertTB",       0,0);
   showNewTB          = XmCreateToggleButton(*checkRC, "showNewTB",     0,0);
   windowPosTB        = XmCreateToggleButton(*checkRC, "windowPosTB",   0,0);
   quickHelpTB        = XmCreateToggleButton(*checkRC, "quickHelpTB",   0,0);
   delSaveTB          = XmCreateToggleButton(*checkRC, "delSaveTB",     0,0);
   hideDelTB          = XmCreateToggleButton(*checkRC, "hideDelTB",     0,0);
   newUnreadTB        = XmCreateToggleButton(*checkRC, "newUnreadTB",   0,0);
   Widget recentForm  = XmCreateForm        (*checkRC, "recentForm",    0,0);

//
// Create inBoxForm hierarchy
//
// inBoxForm
//    Label		inBoxLabel
//    TextField		inBoxTF
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   Widget inBoxLabel = XmCreateLabel(inBoxForm, "inBoxLabel", ARGS);

   args.LeftAttachment(XmATTACH_WIDGET, inBoxLabel);
   args.RightAttachment(XmATTACH_FORM);
   inBoxTF = CreateTextField(inBoxForm, "inBoxTF", ARGS);

   wlist[0] = inBoxLabel;
   wlist[1] = inBoxTF;
   XtManageChildren(wlist, 2);	// inBoxForm children

//
// Create checkForm hierarchy
//
// checkForm
//    Label		checkLabel
//    TextField		checkTF
//    Label		checkSecLabel
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   Widget checkLabel = XmCreateLabel(checkForm, "checkLabel", ARGS);

   args.LeftAttachment(XmATTACH_WIDGET, checkLabel);
   args.RightAttachment(XmATTACH_NONE);
   checkTF = CreateTextField(checkForm, "checkTF", ARGS);

   args.LeftAttachment(XmATTACH_WIDGET, checkTF);
   Widget checkSecLabel = XmCreateLabel(checkForm, "checkSecLabel", ARGS);

   wlist[0] = checkLabel;
   wlist[1] = checkTF;
   wlist[2] = checkSecLabel;
   XtManageChildren(wlist, 3);	// checkForm children

//
// Create recentForm hierarchy
//
// recentForm
//    Label		recentLabel1
//    TextField		recentTF
//    Label		recentLabel2
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   Widget recentLabel1 = XmCreateLabel(recentForm, "recentLabel1", ARGS);

   args.LeftAttachment(XmATTACH_NONE);
   args.RightAttachment(XmATTACH_FORM);
   Widget recentLabel2 = XmCreateLabel(recentForm, "recentLabel2", ARGS);

   args.LeftAttachment(XmATTACH_WIDGET, recentLabel1);
   args.RightAttachment(XmATTACH_WIDGET, recentLabel2);
   recentTF = CreateTextField(recentForm, "recentTF", ARGS);

   wlist[0] = recentLabel1;
   wlist[1] = recentTF;
   wlist[2] = recentLabel2;
   XtManageChildren(wlist, 3);	// recentForm children

   wlist[0] = inBoxForm;
   wlist[1] = checkForm;
   wlist[2] = alertTB;
   wlist[3] = showNewTB;
   wlist[4] = hideDelTB;
   wlist[5] = delSaveTB;
   wlist[6] = newUnreadTB;
   wlist[7] = windowPosTB;
   wlist[8] = quickHelpTB;
   wlist[9] = recentForm;
   checkRC->SetChildren(wlist, 10);

//
// Create archiveForm hierarchy
//
// archiveForm
//    Label		archiveTB
//    TextField		archiveTF
//    Label		archiveLabel
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   archiveTB = XmCreateToggleButton(archiveForm, "archiveTB", ARGS);

   args.LeftAttachment(XmATTACH_NONE);
   args.RightAttachment(XmATTACH_FORM);
   Widget archiveLabel = XmCreateLabel(archiveForm, "archiveLabel", ARGS);

   args.LeftAttachment(XmATTACH_WIDGET, archiveTB);
   args.RightAttachment(XmATTACH_WIDGET, archiveLabel);
   archiveTF = CreateTextField(archiveForm, "archiveTF", ARGS);

   wlist[0] = archiveTB;
   wlist[1] = archiveTF;
   wlist[2] = archiveLabel;
   XtManageChildren(wlist, 3);	// archiveForm children

//
// Create fieldRC hierarchy
//
// fieldRC
//    ToggleButton	imapTB
//    TextField		imapTF
//    ToggleButton	popTB
//    TextField		popTF
//    ToggleButton	popCmdLabel
//    TextField		popCmdTF
//    Label		folderLabel
//    TextField		folderTF
//    Label		saveLabel
//    Frame		saveFrame
//    Label		autoLabel
//    TextField		autoTF
//    Label		printLabel
//    TextField		printTF
//    Label		bellLabel
//    Scale		bellScale
//
   imapTB             = XmCreateToggleButton(*fieldRC, "imapTB",      0,0);
   imapTF             = CreateTextField     (*fieldRC, "imapTF",      0,0);
   popTB              = XmCreateToggleButton(*fieldRC, "popTB",       0,0);
   popTF              = CreateTextField     (*fieldRC, "popTF",       0,0);
   Widget popCmdLabel = XmCreateLabel       (*fieldRC, "popCmdLabel", 0,0);
   popCmdTF           = CreateTextField     (*fieldRC, "popCmdTF",    0,0);
   Widget folderLabel = XmCreateLabel       (*fieldRC, "folderLabel", 0,0);
   folderTF           = CreateTextField     (*fieldRC, "folderTF",    0,0);
   Widget saveLabel   = XmCreateLabel       (*fieldRC, "saveLabel",   0,0);
   Widget saveFrame   = XmCreateFrame       (*fieldRC, "saveFrame",   0,0);
   Widget autoLabel   = XmCreateLabel       (*fieldRC, "autoLabel",   0,0);
   autoTF             = CreateTextField     (*fieldRC, "autoTF",      0,0);
   Widget printLabel  = XmCreateLabel       (*fieldRC, "printLabel",  0,0);
   printTF            = CreateTextField     (*fieldRC, "printTF",     0,0);
   Widget bellLabel   = XmCreateLabel       (*fieldRC, "bellLabel",   0,0);

   args.Reset();
   args.Orientation(XmHORIZONTAL);
   args.Maximum(10);
   args.Minimum(0);
   args.ShowValue(True);
   bellScale = XmCreateScale(*fieldRC, "bellScale", ARGS);

   XtAddCallback(bellScale, XmNvalueChangedCallback, (XtCallbackProc)SoundBell,
   		 NULL);

//
// Create saveFrame hierarchy
//
// saveFrame
//    RowColumn		saveRC
//       Form		   saveFolderForm
//       Form		   savePatternForm
//
   args.Reset();
   args.Orientation(XmVERTICAL);
   args.Packing(XmPACK_TIGHT);
   Widget saveRC = XmCreateRowColumn(saveFrame, "saveRC", ARGS);

   Widget saveFolderForm  = XmCreateForm      (saveRC, "saveFolderForm",  0,0);
   Widget savePatternForm = XmCreateForm      (saveRC, "savePatternForm", 0,0);

//
// Create saveFolderForm hierarchy
//
// saveFolderForm
//    ToggleButton	saveFolderTB
//    TextField		saveFolderTF
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   args.IndicatorType(XmONE_OF_MANY);
   saveFolderTB = XmCreateToggleButton(saveFolderForm, "saveFolderTB", ARGS);

   XtAddCallback(saveFolderTB, XmNvalueChangedCallback,
   		 (XtCallbackProc)SaveChanged, this);

   args.LeftAttachment(XmATTACH_WIDGET, saveFolderTB);
   args.RightAttachment(XmATTACH_FORM);
   saveFolderTF = CreateTextField(saveFolderForm, "saveFolderTF", ARGS);

   XtAddCallback(saveFolderTF, XmNvalueChangedCallback,
   		 (XtCallbackProc)AutoSelectFolder, this);

   XtManageChild(saveFolderTB);
   XtManageChild(saveFolderTF);

//
// Create savePatternForm hierarchy
//
// savePatternForm
//    ToggleButton	savePatternTB
//    OptionMenu	savePatternOM
//    PulldownMenu	savePatternPD
//    Label		savePatternLabel
//    TextField		savePatternTF
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   args.IndicatorType(XmONE_OF_MANY);
   savePatternTB = XmCreateToggleButton(savePatternForm, "savePatternTB", ARGS);

   XtAddCallback(savePatternTB, XmNvalueChangedCallback,
   		 (XtCallbackProc)SaveChanged, this);

   Widget savePatternPD
		  = XmCreatePulldownMenu(savePatternForm, "savePatternPD", 0,0);

   args.LeftAttachment(XmATTACH_WIDGET, savePatternTB);
   args.SubMenuId(savePatternPD);
   savePatternOM = XmCreateOptionMenu(savePatternForm, "savePatternOM", ARGS);

   args.LeftAttachment(XmATTACH_WIDGET, savePatternOM);
   Widget savePatternLabel
   		  = XmCreateLabel(savePatternForm, "savePatternLabel", ARGS);
   XtManageChild(savePatternLabel);

   args.LeftAttachment(XmATTACH_WIDGET, savePatternLabel);
   args.RightAttachment(XmATTACH_FORM);
   savePatternTF = CreateTextField(savePatternForm, "savePatternTF", ARGS);

   XtAddCallback(savePatternTF, XmNvalueChangedCallback,
   		 (XtCallbackProc)AutoSelectPattern, this);

//
// Create savePatternPD hierarchy
//
// savePatternPD
//    PushButton	saveUserPB
//    PushButton	saveAddrPB
//    PushButton	savePatternPB
//    PushButton	savePatUserPB
//    PushButton	savePatAddrPB
//
   saveUserPB    = XmCreatePushButton(savePatternPD, "saveUserPB",    0,0);
   saveAddrPB    = XmCreatePushButton(savePatternPD, "saveAddrPB",    0,0);
   savePatternPB = XmCreatePushButton(savePatternPD, "savePatternPB", 0,0);
   savePatUserPB = XmCreatePushButton(savePatternPD, "savePatUserPB", 0,0);
   savePatAddrPB = XmCreatePushButton(savePatternPD, "savePatAddrPB", 0,0);

   XtAddCallback(saveUserPB, XmNactivateCallback,
   		 (XtCallbackProc)AutoSelectPattern, this);
   XtAddCallback(saveAddrPB, XmNactivateCallback,
   		 (XtCallbackProc)AutoSelectPattern, this);
   XtAddCallback(savePatternPB, XmNactivateCallback,
   		 (XtCallbackProc)AutoSelectPattern, this);
   XtAddCallback(savePatUserPB, XmNactivateCallback,
   		 (XtCallbackProc)AutoSelectPattern, this);
   XtAddCallback(savePatAddrPB, XmNactivateCallback,
   		 (XtCallbackProc)AutoSelectPattern, this);

   wlist[0] = saveUserPB;
   wlist[1] = saveAddrPB;
   wlist[2] = savePatternPB;
   wlist[3] = savePatUserPB;
   wlist[4] = savePatAddrPB;
   XtManageChildren(wlist, 5);

   XtManageChild(savePatternTB);
   XtManageChild(savePatternOM);
   XtManageChild(savePatternTF);

   XtManageChild(savePatternForm);
   XtManageChild(saveFolderForm);

   XtManageChild(saveRC);

   int	count = 0;
   wlist[count++] = imapTB;
   wlist[count++] = imapTF;
   wlist[count++] = popTB;
   wlist[count++] = popTF;
   wlist[count++] = popCmdLabel;
   wlist[count++] = popCmdTF;
   wlist[count++] = folderLabel;
   wlist[count++] = folderTF;
   wlist[count++] = saveLabel;
   wlist[count++] = saveFrame;
   wlist[count++] = autoLabel;
   wlist[count++] = autoTF;
   wlist[count++] = printLabel;
   wlist[count++] = printTF;
   wlist[count++] = bellLabel;
   wlist[count++] = bellScale;
   XtManageChildren(wlist, count);	// fieldRC children
   fieldRC->SetChildren(wlist, count);

   wlist[0] = *checkRC;
   wlist[1] = archiveForm;
   wlist[2] = *fieldRC;
   XtManageChildren(wlist, 3);	// appForm children

   checkRC->Defer(False);
   fieldRC->Defer(False);

   HandleHelp();

} // End constructor

/*---------------------------------------------------------------
 *  Main window destructor
 */

AppPrefWinC::~AppPrefWinC()
{
   delete checkRC;
   delete fieldRC;
}

/*---------------------------------------------------------------
 *  Callback to handle initial display
 */

void
AppPrefWinC::Show()
{
   AppPrefC	*prefs = ishApp->appPrefs;

//
// Initialize settings
//
   XmTextFieldSetString(inBoxTF,   prefs->inBox);
   StringC	tmpStr; tmpStr += prefs->checkInterval;
   XmTextFieldSetString(checkTF,   tmpStr);
   tmpStr.Clear();      tmpStr += prefs->recentFolderCount;
   XmTextFieldSetString(recentTF,  tmpStr);
   XmTextFieldSetString(archiveTF,   prefs->OrigArchiveFolder());
   XmTextFieldSetString(imapTF,      prefs->imapServer);
   XmTextFieldSetString(popTF,       prefs->popServer);
   XmTextFieldSetString(popCmdTF,    prefs->popclientCmd);
   XmTextFieldSetString(folderTF,    prefs->OrigFolderDir());
   XmTextFieldSetString(printTF,     prefs->printCmd);

   XmToggleButtonSetState(alertTB,       ishApp->alertPrefs->alertOn,	False);
   XmToggleButtonSetState(showNewTB,     prefs->scrollToNew,		True);
   XmToggleButtonSetState(delSaveTB,     prefs->deleteSaved,		False);
   XmToggleButtonSetState(hideDelTB,     prefs->hideDeleted,		False);
   XmToggleButtonSetState(quickHelpTB,   halApp->quickHelpEnabled,	False);
   XmToggleButtonSetState(windowPosTB,   prefs->rememberWindows,	False);
   XmToggleButtonSetState(newUnreadTB,   prefs->markNewAsUnread,	False);
   XmToggleButtonSetState(archiveTB,     prefs->archiveOnSave,		False);
   XmToggleButtonSetState(imapTB,        prefs->usingImap,		False);
   XmToggleButtonSetState(popTB,         prefs->usingPop,		False);

   if ( prefs->OrigSaveFile().size() )
      XmTextFieldSetString(saveFolderTF, prefs->OrigSaveFile());
   if ( prefs->OrigSaveDir().size() )
      XmTextFieldSetString(savePatternTF, prefs->OrigSaveDir());

   switch ( prefs->saveType ) {

      case (SAVE_TO_FOLDER):
         XmToggleButtonSetState(saveFolderTB, True, True);
	 break;
      case (SAVE_BY_USER):
         XmToggleButtonSetState(savePatternTB, True, True);
	 XtVaSetValues(savePatternOM, XmNmenuHistory, saveUserPB,    NULL);
	 break;
      case (SAVE_BY_ADDRESS):
         XmToggleButtonSetState(savePatternTB, True, True);
	 XtVaSetValues(savePatternOM, XmNmenuHistory, saveAddrPB,    NULL);
	 break;
      case (SAVE_BY_PATTERN):
         XmToggleButtonSetState(savePatternTB, True, True);
	 XtVaSetValues(savePatternOM, XmNmenuHistory, savePatternPB, NULL);
	 break;
      case (SAVE_BY_PATTERN_OR_USER):
         XmToggleButtonSetState(savePatternTB, True, True);
	 XtVaSetValues(savePatternOM, XmNmenuHistory, savePatUserPB, NULL);
	 break;
      case (SAVE_BY_PATTERN_OR_ADDRESS):
         XmToggleButtonSetState(savePatternTB, True, True);
	 XtVaSetValues(savePatternOM, XmNmenuHistory, savePatAddrPB, NULL);
	 break;
   }

   if ( prefs->OrigAutomountRoot().size() )
      XmTextFieldSetString(autoTF, prefs->OrigAutomountRoot());

//
// Display the bell volume.  It is stored in the range (-100, 100) and
//    display in the range (0, 10)
//
   XmScaleSetValue(bellScale, (prefs->bellVolume + 100) / 20);

   OptWinC::Show();

} // End Show

/*---------------------------------------------------------------
 *  Method to apply settings
 */

Boolean
AppPrefWinC::Apply()
{
//
// Check filenames
//
   char *cs = XmTextFieldGetString(inBoxTF);
   StringC	inBoxStr(cs);
   inBoxStr.Trim();
   XtFree(cs);

   if ( inBoxStr.size() <= 0 ) {
      set_invalid(inBoxTF, True, True);
      PopupMessage("Please enter an In-box folder name.");
      return False;
   }

   cs = XmTextFieldGetString(saveFolderTF);
   StringC	saveStr(cs);
   saveStr.Trim();
   XtFree(cs);

   if ( XmToggleButtonGetState(saveFolderTB) && saveStr.size() <= 0 ) {
      set_invalid(saveFolderTF, True, True);
      PopupMessage("Please enter a folder name for saved messages.");
      return False;
   }

   cs = XmTextFieldGetString(imapTF);
   StringC	imapStr(cs);
   imapStr.Trim();
   XtFree(cs);

   if ( XmToggleButtonGetState(imapTB) && imapStr.size() <= 0 ) {
      set_invalid(imapTF, True, True);
      PopupMessage("Please enter the name of your IMAP server.");
      return False;
   }
   else if ( imapStr.Contains('{') || imapStr.Contains('}') ||
      	     imapStr.Contains('/') ) {
      set_invalid(imapTF, True, True);
      PopupMessage("Please enter the server name only.");
      return False;
   }

   cs = XmTextFieldGetString(popTF);
   StringC	popStr(cs);
   popStr.Trim();
   XtFree(cs);

   cs = XmTextFieldGetString(popCmdTF);
   StringC	popCmdStr(cs);
   popCmdStr.Trim();
   XtFree(cs);

   if ( XmToggleButtonGetState(popTB) ) {
      if ( popStr.size() <= 0 ) {
	 set_invalid(popTF, True, True);
	 PopupMessage("Please enter the name of your POP server.");
	 return False;
      }
      else if ( popCmdStr.size() <= 0 ) {
	 set_invalid(popCmdTF, True, True);
	 PopupMessage("Please enter the path to your popclient command.");
	 return False;
      }
   }

   cs = XmTextFieldGetString(savePatternTF);
   StringC	saveDirStr(cs);
   saveDirStr.Trim();
   XtFree(cs);

   if ( XmToggleButtonGetState(savePatternTB) && saveDirStr.size() <= 0 ) {
      set_invalid(savePatternTF, True, True);
      PopupMessage("Please enter a directory name for saved messages.");
      return False;
   }

   cs = XmTextFieldGetString(archiveTF);
   StringC	archiveStr(cs);
   archiveStr.Trim();
   XtFree(cs);

   if ( XmToggleButtonGetState(archiveTB) && archiveStr.size() <= 0 ) {
      set_invalid(archiveTF, True, True);
      PopupMessage("Please enter a folder name for archived messages.");
      return False;
   }

   BusyCursor(True);

   AppPrefC	*prefs = ishApp->appPrefs;


   if (inBoxStr.compare(prefs->inBox)) {
      PopupMessage("New In-box setting will take effect only after restart.",
         XmDIALOG_INFORMATION);
   }

   prefs->inBox = inBoxStr;

   prefs->usingImap  = XmToggleButtonGetState(imapTB);
   prefs->imapServer = imapStr;

   prefs->usingPop     = XmToggleButtonGetState(popTB);
   prefs->popServer    = popStr;
   prefs->popclientCmd = popCmdStr;

//
// Set folder directory
//
   cs = XmTextFieldGetString(folderTF);
   if ( strlen(cs) > 0 ) prefs->SetFolderDir(cs);
   else {
      if ( prefs->usingImap ) 
         prefs->SetFolderDir(cs);		// OK to have null dir for IMAP
      else
	 prefs->SetFolderDir(ishApp->home);	// else set sensible default
   }
   XtFree(cs);

//
// Set default save file and directory
//
   prefs->SetSaveFile(saveStr);
   prefs->SetSaveDir(saveDirStr);

//
// Set save type
//
   if ( XmToggleButtonGetState(saveFolderTB) ) prefs->saveType = SAVE_TO_FOLDER;
   else {
      Widget	pb;
      XtVaGetValues(savePatternOM, XmNmenuHistory, &pb, NULL);
      if      ( pb == saveUserPB    ) prefs->saveType = SAVE_BY_USER;
      else if ( pb == saveAddrPB    ) prefs->saveType = SAVE_BY_ADDRESS;
      else if ( pb == savePatternPB ) prefs->saveType = SAVE_BY_PATTERN;
      else if ( pb == savePatUserPB ) prefs->saveType = SAVE_BY_PATTERN_OR_USER;
      else if ( pb == savePatAddrPB )
	 prefs->saveType = SAVE_BY_PATTERN_OR_ADDRESS;
   }

//
// Set automounter root prefix
//
   cs = XmTextFieldGetString(autoTF);
   prefs->SetAutomountRoot(cs);
   XtFree(cs);

//
// Read check interval
//
   cs = XmTextFieldGetString(checkTF);
   int	ci = atoi(cs);
   if ( ci != prefs->checkInterval ) {
      prefs->checkInterval = ci;
      if ( ci > 0 ) ishApp->CheckForNewMail();	// Start check again
   }
   XtFree(cs);

   ishApp->alertPrefs->alertOn = XmToggleButtonGetState(alertTB);
   prefs->deleteSaved          = XmToggleButtonGetState(delSaveTB);
   prefs->scrollToNew          = XmToggleButtonGetState(showNewTB);
   prefs->rememberWindows      = XmToggleButtonGetState(windowPosTB);
   prefs->markNewAsUnread      = XmToggleButtonGetState(newUnreadTB);
   prefs->archiveOnSave        = XmToggleButtonGetState(archiveTB);
   prefs->SetArchiveFolder(archiveStr);
   halApp->QuickHelp(XmToggleButtonGetState(quickHelpTB));

//
// Read recent folder count
//
   cs = XmTextFieldGetString(recentTF);
   ci = atoi(cs);
   if ( ci < 1 ) {
      ci = 10;
      StringC	ciStr;
      ciStr += ci;
      XmTextFieldSetString(recentTF, ciStr);
   }
   prefs->recentFolderCount = ci;
   XtFree(cs);

//
// Read print command
//
   cs = XmTextFieldGetString(printTF);
   prefs->printCmd = cs;
   XtFree(cs);

//
// Read bell volume.  Displayed in range 0-10, stored in range -100 to 100
//
   int	val;
   XmScaleGetValue(bellScale, &val);
   prefs->bellVolume = (val - 5) * 20;

//
// Set main window attributes
//
   Boolean	hideDel = XmToggleButtonGetState(hideDelTB);
   if ( hideDel != prefs->hideDeleted ) {
      prefs->hideDeleted = hideDel;
      ishApp->mainWin->HideDeletedChanged();
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
 *  Callback to implement radio button behavior in saveFrame
 */

void
AppPrefWinC::SaveChanged(Widget tb, AppPrefWinC* This,
		      XmToggleButtonCallbackStruct *data)
{
//
// Don't allow buttons to be turned off by the user
//
   if ( !data->set ) {
      XmToggleButtonSetState(tb, True, False);
      return;
   }

   if ( tb == This->saveFolderTB )
      XmToggleButtonSetState(This->savePatternTB, False, False);
   else
      XmToggleButtonSetState(This->saveFolderTB,  False, False);

} // End SaveChanged

/*---------------------------------------------------------------
 *  Callbacks to automatically select toggle buttons when field is
 *     changed or type is selected
 */

void
AppPrefWinC::AutoSelectFolder(Widget, AppPrefWinC* This, XtPointer)
{
   XmToggleButtonSetState(This->saveFolderTB, True, True);
}

void
AppPrefWinC::AutoSelectPattern(Widget, AppPrefWinC* This, XtPointer)
{
   XmToggleButtonSetState(This->savePatternTB, True, True);
}

