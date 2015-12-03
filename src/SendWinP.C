/*
 *  $Id: SendWinP.C,v 1.9 2001/07/28 18:26:03 evgeny Exp $
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

#include "IshAppC.h"

#include "SendWinP.h"
#include "SendWinC.h"
#include "CompPrefC.h"
#include "MailPrefC.h"
#include "SendButtPrefC.h"
#include "ReplyPrefC.h"
#include "ButtonMgrC.h"
#include "AddressC.h"
#include "AliasPrefC.h"
#include "MsgItemC.h"
#include "AddrMisc.h"
#include "HeaderValC.h"
#include "Misc.h"
#include "HeaderC.h"
#include "HeadPrefC.h"
#include "Query.h"
#include "ConfPrefC.h"
#include "PickAliasWinC.h"
#include "IncludeWinC.h"
#include "MsgPartC.h"
#include "ParamC.h"
#include "SendIconC.h"
#include "FileChooserWinC.h"
#include "FileMisc.h"
#include "composition.xpm"

#include <hgl/WArgList.h>
#include <hgl/WXmString.h>
#include <hgl/RowColC.h>
#include <hgl/rsrc.h>
#include <hgl/MimeRichTextC.h>
#include <hgl/TextMisc.h>
#include <hgl/PixmapC.h>
#include <hgl/VBoxC.h>

#include <Xm/PushB.h>
#include <Xm/CascadeB.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/ToggleB.h>
#include <Xm/PanedW.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/TextF.h>
#include <Xm/Form.h>
#include <Xm/AtomMgr.h>
#include <Xm/MessageB.h>
#include <Xm/Protocols.h>

#ifndef HAVE_XMREMOVEFROMPOSTFROMLIST_DECL
extern "C" void XmRemoveFromPostFromList(Widget menu, Widget post_from_menu);
#endif

#include <unistd.h>

PtrListC	*SendWinP::winList     = NULL;
PtrListC	*SendWinP::editPixmaps = NULL;
Atom		 SendWinP::graphicAtom = (Atom)NULL;
XtActionsRec     SendWinP::actions[2]  = {
   "SendWinC-expand-aliases",   (XtActionProc)HandleExpandAliases,
   "SendWinC-expand-selection", (XtActionProc)HandleExpandSelection,
};

/*---------------------------------------------------------------
 *  Composition window private constructor
 */

SendWinP::SendWinP(SendWinC *sw)
{
   pub = sw;

   if ( !editPixmaps ) editPixmaps = new PtrListC;

//
// Add this object to the list of all windows
//
   if ( !winList ) winList = new PtrListC;
   void	*tmp = (void*)this;
   winList->add(tmp);

   oldHeadPane     = NULL;
   newHeadPane     = NULL;
   curHeadPane     = NULL;
   headText        = NULL;
   bodyText        = NULL;
   optCcTB         = NULL;
   optBccTB        = NULL;
   optFccTB        = NULL;
   optOtherTB      = NULL;
   optMimeTB       = NULL;
   optAddSigTB     = NULL;
   optDigSignTB    = NULL;
   optEncryptTB    = NULL;
   optCheckAddrTB  = NULL;
   optMsgAltTB     = NULL;
   optMsgMimeTB    = NULL;
   optMsgPlainTB   = NULL;
   optTextRichTB   = NULL;
   optTextPlainTB  = NULL;

   clearWin        = NULL;
   deleteWin       = NULL;
   incMsgWin       = NULL;
   incTextWin      = NULL;
   incFileWin      = NULL;
   pickAliasWin    = NULL;
   saveFileWin     = NULL;
   loadFileWin     = NULL;
   fileDataWin	   = NULL;

   compIcon        = NULL;
   popupIcon       = NULL;
   modIcon         = NULL;
   editIcon	   = NULL;
   editMsg	   = NULL;

   containerType   = CT_MIXED;
   edit_pid        = 0;
   spell_pid       = 0;
   modifying       = False;
   keystrokeCount  = 0;
   editMsgText     = False;
   autoSaveTimer   = (XtIntervalId)NULL;
}

/*---------------------------------------------------------------
 *  Composition window private destuctor
 */

SendWinP::~SendWinP()
{
//
// Remove any auto-save timer
//
   if ( halApp->xRunning && autoSaveTimer ) XtRemoveTimeOut(autoSaveTimer);

   delete pickAliasWin;
   delete fileDataWin;
   delete incTextWin;
   delete incFileWin;
   delete saveFileWin;
   delete loadFileWin;

   delete headText;
   delete bodyText;
   delete compIcon;
   delete optRC;

   if ( oldHeadPane ) {
      HeaderPaneT	*p = oldHeadPane;
      delete p->toText;
      delete p->ccText;
      delete p->bccText;
      delete p->fccText;
      delete p->subText;
      delete p->otherText;
      delete p;
   }

   if ( newHeadPane ) {
      HeaderPaneT	*p = newHeadPane;
      delete p->toText;
      delete p->ccText;
      delete p->bccText;
      delete p->fccText;
      delete p->subText;
      delete p->otherText;
      delete p;
   }

   if ( ishApp->exiting ) {
      u_int	count = editPixmaps->size();
      for (int i=0; i<count; i++) {
	 PixmapC	*pm = (PixmapC*)*(*editPixmaps)[i];
	 delete pm;
      }
   }

//
// Remove this object from the list of all windows
//
   void	*tmp = (void*)this;
   winList->remove(tmp);

} // End Destructor

/*---------------------------------------------------------------
 *  Method to build menus
 */

void
SendWinP::BuildMenus()
{
   pub->AddMenuBar();

   BuildFileMenu();
   BuildEditMenu();
   BuildOptMenu();

   if ( !pub->IsEditOnly() ) pub->AddHelpMenu();
}

/*---------------------------------------------------------------
 *  Method to build file menu hierarchy
 */

void
SendWinP::BuildFileMenu()
{
   Cardinal	wcount;
   Widget	wlist[16];
   WArgList	args;

//
// Create filePD hierarchy
//
// filePD
//    PushButton	fileEditPB
//    PushButton	fileSpellPB
//    CascadeButton	fileIncCB
//    CascadeButton	fileAddCB
//    PushButton	fileCheckPB
//    PushButton	fileSavePB
//    PushButton	fileLoadPB
//    Separator		fileSep1
//    PushButton	fileSendKeepPB
//    PushButton	fileSendPB
//    PushButton	fileSendClosePB
//    Separator		fileSep2
//    PushButton	fileClearPB
//    PushButton	fileClosePB
//    PushButton	fileCancelPB
//
   fileEditPB    = XmCreatePushButton   (pub->filePD, "fileEditPB",  0,0);
   fileSpellPB   = XmCreatePushButton   (pub->filePD, "fileSpellPB", 0,0);
   fileIncCB     = XmCreateCascadeButton(pub->filePD, "fileIncCB",   0,0);
   fileAddCB     = XmCreateCascadeButton(pub->filePD, "fileAddCB",   0,0);

   Widget fileCheckPB;
   if ( !pub->IsEditOnly() )
      fileCheckPB = XmCreatePushButton(pub->filePD, "fileCheckPB", 0,0);

   fileSavePB          = XmCreatePushButton(pub->filePD, "fileSavePB", 0,0);
   fileLoadPB          = XmCreatePushButton(pub->filePD, "fileLoadPB", 0,0);
   Widget fileSep1     = XmCreateSeparator (pub->filePD, "fileSep1",   0,0);

   Widget fileSendKeepPB;
   Widget fileSendPB;
   Widget fileSendClosePB;
   Widget fileSep2;
   if ( !pub->IsEditOnly() ) {
      fileSendKeepPB = XmCreatePushButton(pub->filePD, "fileSendKeepPB",  0,0);
      fileSendPB     = XmCreatePushButton(pub->filePD, "fileSendPB",      0,0);
      fileSendClosePB= XmCreatePushButton(pub->filePD, "fileSendClosePB", 0,0);
      fileSep2       = XmCreateSeparator (pub->filePD, "fileSep2",        0,0);
   }

   fileClearPB        = XmCreatePushButton(pub->filePD, "fileClearPB",     0,0);
   Widget fileClosePB = XmCreatePushButton(pub->filePD, "fileClosePB",     0,0);

   Widget fileCancelPB;
   if ( pub->IsEditOnly() )
      fileCancelPB = XmCreatePushButton(pub->filePD, "fileCancelPB", 0,0);

   wcount = 0;
   wlist[wcount++] = fileEditPB;
   wlist[wcount++] = fileSpellPB;
   wlist[wcount++] = fileIncCB;
   wlist[wcount++] = fileAddCB;
   wlist[wcount++] = fileSavePB;
   wlist[wcount++] = fileLoadPB;
   wlist[wcount++] = fileSep1;
   wlist[wcount++] = fileClearPB;
   wlist[wcount++] = fileClosePB;

   if ( pub->IsEditOnly() )
      wlist[wcount++] = fileCancelPB;
   else {
      wlist[wcount++] = fileCheckPB;
      wlist[wcount++] = fileSendKeepPB;
      wlist[wcount++] = fileSendPB;
      wlist[wcount++] = fileSendClosePB;
      wlist[wcount++] = fileSep2;
   }

   XtManageChildren(wlist, wcount);

   AddActivate(fileEditPB,		DoEdit,		this);
   AddActivate(fileSpellPB,		DoSpell,	this);
   AddActivate(fileSavePB,		DoSaveFile,	this);
   AddActivate(fileLoadPB,		DoLoadFile,	this);
   AddActivate(fileClearPB,		DoClear,	this);
   AddActivate(fileClosePB,		DoClose,	this);

   if ( !pub->IsEditOnly() ) {
      AddActivate(fileCheckPB,		DoCheckNow,	this);
      AddActivate(fileSendPB,		DoSend,		this);
      AddActivate(fileSendKeepPB,	DoSendKeep,	this);
      AddActivate(fileSendClosePB,	DoSendClose,	this);
   }
   else
      AddActivate(fileCancelPB,		DoCancel,	this);

//
// Create fileIncPD hierarchy
//
// fileIncPD
//    PushButton	fileIncTextPB
//    PushButton	fileIncFilePB
//    PushButton	fileIncMsgPB
//    CascadeButton	fileIncSigCB
//    PulldownMenu	fileIncSigPD
//       PushButton	fileIncExtPSigPB
//       PushButton	fileIncExtESigPB
//       PushButton	fileIncIntPSigPB
//       PushButton	fileIncIntESigPB
//
   Widget fileIncPD = XmCreatePulldownMenu(pub->filePD, "fileIncPD", 0,0);
   XtVaSetValues(fileIncCB, XmNsubMenuId, fileIncPD, NULL);

   Widget fileIncTextPB = XmCreatePushButton   (fileIncPD, "fileIncTextPB",0,0);
   Widget fileIncFilePB = XmCreatePushButton   (fileIncPD, "fileIncFilePB",0,0);
   Widget fileIncMsgPB  = XmCreatePushButton   (fileIncPD, "fileIncMsgPB", 0,0);
   Widget fileIncSigCB  = XmCreateCascadeButton(fileIncPD, "fileIncSigCB", 0,0);

   wcount = 0;
   wlist[wcount++] = fileIncTextPB;
   wlist[wcount++] = fileIncFilePB;
   wlist[wcount++] = fileIncMsgPB;
   wlist[wcount++] = fileIncSigCB;
   XtManageChildren(wlist, wcount);

   AddActivate(fileIncTextPB,  DoIncludeText, this);
   AddActivate(fileIncFilePB,  DoIncludeFile, this);
   AddActivate(fileIncMsgPB,   DoIncludeMsg,  this);

   Widget fileIncSigPD  = XmCreatePulldownMenu (fileIncPD, "fileIncSigPD", 0,0);
   XtVaSetValues(fileIncSigCB, XmNsubMenuId, fileIncSigPD, NULL);

   Widget fileIncExtPSigPB =
		     XmCreatePushButton(fileIncSigPD, "fileIncExtPSigPB", 0,0);
   Widget fileIncExtESigPB =
		     XmCreatePushButton(fileIncSigPD, "fileIncExtESigPB", 0,0);
   Widget fileIncIntPSigPB =
		     XmCreatePushButton(fileIncSigPD, "fileIncIntPSigPB", 0,0);
   Widget fileIncIntESigPB =
		     XmCreatePushButton(fileIncSigPD, "fileIncIntESigPB", 0,0);

   wcount = 0;
   wlist[wcount++] = fileIncExtPSigPB;
   wlist[wcount++] = fileIncExtESigPB;
   wlist[wcount++] = fileIncIntPSigPB;
   wlist[wcount++] = fileIncIntESigPB;
   XtManageChildren(wlist, wcount);

   AddActivate(fileIncExtPSigPB,   DoIncludeExtPSig, this);
   AddActivate(fileIncExtESigPB,   DoIncludeExtESig, this);
   AddActivate(fileIncIntPSigPB,   DoIncludeIntPSig, this);
   AddActivate(fileIncIntESigPB,   DoIncludeIntESig, this);

//
// Create fileAddPD hierarchy
//
// fileAddPD
//    PushButton	fileAddMixPB
//    PushButton	fileAddDigPB
//    PushButton	fileAddAltPB
//    PushButton	fileAddParPB
//
   Widget fileAddPD = XmCreatePulldownMenu(pub->filePD, "fileAddPD", 0,0);
   XtVaSetValues(fileAddCB, XmNsubMenuId, fileAddPD, NULL);

   Widget fileAddMixPB = XmCreatePushButton(fileAddPD, "fileAddMixPB", 0,0);
   Widget fileAddDigPB = XmCreatePushButton(fileAddPD, "fileAddDigPB", 0,0);
   Widget fileAddAltPB = XmCreatePushButton(fileAddPD, "fileAddAltPB", 0,0);
   Widget fileAddParPB = XmCreatePushButton(fileAddPD, "fileAddParPB", 0,0);

   wlist[0] = fileAddMixPB;
   wlist[1] = fileAddDigPB;
   wlist[2] = fileAddAltPB;
   wlist[3] = fileAddParPB;
   XtManageChildren(wlist, 4);

   AddActivate(fileAddMixPB, DoAddMixed,    this);
   AddActivate(fileAddDigPB, DoAddDigest,   this);
   AddActivate(fileAddAltPB, DoAddAlt,      this);
   AddActivate(fileAddParPB, DoAddParallel, this);

} // End BuildFileMenu

/*---------------------------------------------------------------
 *  Method to build edit menu hierarchy
 */

void
SendWinP::BuildEditMenu()
{
   Cardinal	wcount;
   Widget	wlist[24];
   WArgList	args;

   editCB = XmCreateCascadeButton(pub->menuBar, "editCB", 0,0);
   editPD = XmCreatePulldownMenu (pub->menuBar, "editPD", 0,0);
   XtVaSetValues(editCB, XmNsubMenuId, editPD, NULL);
   XtManageChild(editCB);

//
// Create editPD hierarchy
//
// editPD
//    PushButton	editUndeletePB
//    Separator		editSep1
//    PushButton	editPlainPB
//    PushButton	editBoldPB
//    PushButton	editItalicPB
//    PushButton	editFixedPB
//    PushButton	editUnderPB
//    Separator		editSep2
//    PushButton	editBiggerPB
//    PushButton	editSmallerPB
//    CascadeButton	editColorCB
//    Separator		editSep3
//    PushButton	editLeftPB
//    PushButton	editRightPB
//    PushButton	editBothPB
//    PushButton	editCenterPB
//    PushButton	editNoFillPB
//    Separator		editSep4
//    PushButton	editExcerptMorePB
//    PushButton	editExcerptLessPB
//    PushButton	editLeftInPB
//    PushButton	editLeftOutPB
//    PushButton	editRightInPB
//    PushButton	editRightOutPB
//
   editUndeletePB  = XmCreatePushButton(editPD, "editUndeletePB",       0,0);
   Widget editSep1 = XmCreateSeparator (editPD, "editSep1",             0,0);
   editPlainPB     = XmCreatePushButton(editPD, "editPlainPB",          0,0);
   editBoldPB      = XmCreatePushButton(editPD, "editBoldPB",           0,0);
   editItalicPB    = XmCreatePushButton(editPD, "editItalicPB",         0,0);
   editFixedPB     = XmCreatePushButton(editPD, "editFixedPB",          0,0);
   editUnderPB     = XmCreatePushButton(editPD, "editUnderPB",          0,0);
   Widget editSep2 = XmCreateSeparator (editPD, "editSep2",             0,0);
   editBiggerPB    = XmCreatePushButton(editPD, "editBiggerPB",         0,0);
   editSmallerPB   = XmCreatePushButton(editPD, "editSmallerPB",        0,0);
   editColorCB     = XmCreateCascadeButton(editPD, "editColorCB",       0,0);
   Widget editSep3 = XmCreateSeparator (editPD, "editSep3",             0,0);
   editLeftPB      = XmCreatePushButton(editPD, "editFlushLeftPB",      0,0);
   editRightPB     = XmCreatePushButton(editPD, "editFlushRightPB",     0,0);
   editBothPB      = XmCreatePushButton(editPD, "editFlushBothPB",      0,0);
   editCenterPB    = XmCreatePushButton(editPD, "editCenterPB",         0,0);
   editNoFillPB    = XmCreatePushButton(editPD, "editNoFillPB",         0,0);
   Widget editSep4 = XmCreateSeparator (editPD, "editSep4",             0,0);
   editExcMorePB   = XmCreatePushButton(editPD, "editExcerptMorePB",    0,0);
   editExcLessPB   = XmCreatePushButton(editPD, "editExcerptLessPB",    0,0);
   editLeftInPB    = XmCreatePushButton(editPD, "editLeftMarginInPB",   0,0);
   editLeftOutPB   = XmCreatePushButton(editPD, "editLeftMarginOutPB",  0,0);
   editRightInPB   = XmCreatePushButton(editPD, "editRightMarginInPB",  0,0);
   editRightOutPB  = XmCreatePushButton(editPD, "editRightMarginOutPB", 0,0);

   wcount = 0;
   wlist[wcount++] = editUndeletePB;
   wlist[wcount++] = editSep1;
   wlist[wcount++] = editPlainPB;
   wlist[wcount++] = editBoldPB;
   wlist[wcount++] = editItalicPB;
   wlist[wcount++] = editFixedPB;
   wlist[wcount++] = editUnderPB;
   wlist[wcount++] = editSep2;
   wlist[wcount++] = editBiggerPB;
   wlist[wcount++] = editSmallerPB;
   wlist[wcount++] = editColorCB;
   wlist[wcount++] = editSep3;
   wlist[wcount++] = editLeftPB;
   wlist[wcount++] = editRightPB;
   wlist[wcount++] = editBothPB;
   wlist[wcount++] = editCenterPB;
   wlist[wcount++] = editNoFillPB;
   wlist[wcount++] = editSep4;
   wlist[wcount++] = editExcMorePB;
   wlist[wcount++] = editExcLessPB;
   wlist[wcount++] = editLeftInPB;
   wlist[wcount++] = editLeftOutPB;
   wlist[wcount++] = editRightInPB;
   wlist[wcount++] = editRightOutPB;
   XtManageChildren(wlist, wcount);	// editPD children

   AddActivate(editUndeletePB,		DoUndelete,		this);
   AddActivate(editPlainPB,		DoPlain,		this);
   AddActivate(editBoldPB,		DoBold,			this);
   AddActivate(editItalicPB,		DoItalic,		this);
   AddActivate(editFixedPB,		DoFixed,		this);
   AddActivate(editUnderPB,		DoUnderline,		this);
   AddActivate(editBiggerPB,		DoBigger,		this);
   AddActivate(editSmallerPB,		DoSmaller,		this);
   AddActivate(editLeftPB,		DoFlushLeft,		this);
   AddActivate(editRightPB,		DoFlushRight,		this);
   AddActivate(editBothPB,		DoFlushBoth,		this);
   AddActivate(editCenterPB,		DoCenter,		this);
   AddActivate(editNoFillPB,		DoNoFill,		this);
   AddActivate(editExcMorePB,		DoExcerptMore,		this);
   AddActivate(editExcLessPB,		DoExcerptLess,		this);
   AddActivate(editLeftInPB,		DoLeftMarginIn,		this);
   AddActivate(editLeftOutPB,		DoLeftMarginOut,	this);
   AddActivate(editRightInPB,		DoRightMarginIn,	this);
   AddActivate(editRightOutPB,		DoRightMarginOut,	this);

//
// Display accelerators depending on edit mode
//
   Boolean	emacs = get_boolean(*pub, "emacsMode", False);
   if ( emacs ) {

      WXmString	wstr;
//      XtVaSetValues(editCB, XmNmnemonic, (KeySym)XK_d, NULL);
      wstr = "Ctrl + Y";
      XtVaSetValues(editUndeletePB, XmNacceleratorText, (XmString)wstr,0);
      wstr = "Ctrl + 0";
      XtVaSetValues(editPlainPB,    XmNacceleratorText, (XmString)wstr, NULL);
      wstr = "Ctrl + 1";
      XtVaSetValues(editBoldPB,     XmNacceleratorText, (XmString)wstr, NULL);
      wstr = "Ctrl + 2";
      XtVaSetValues(editItalicPB,   XmNacceleratorText, (XmString)wstr, NULL);
      wstr = "Ctrl + 3";
      XtVaSetValues(editFixedPB,    XmNacceleratorText, (XmString)wstr, NULL);
      wstr = "Ctrl + -";
      XtVaSetValues(editUnderPB,    XmNacceleratorText, (XmString)wstr, NULL);
      wstr = "Ctrl + 4";
      XtVaSetValues(editBiggerPB,   XmNacceleratorText, (XmString)wstr, NULL);
      wstr = "Meta + 4";
      XtVaSetValues(editSmallerPB,  XmNacceleratorText, (XmString)wstr, NULL);
      wstr = "Ctrl + 5";
      XtVaSetValues(editLeftPB,     XmNacceleratorText, (XmString)wstr, NULL);
      wstr = "Meta + 5";
      XtVaSetValues(editRightPB,    XmNacceleratorText, (XmString)wstr, NULL);
      wstr = "Ctrl + =";
      XtVaSetValues(editCenterPB,   XmNacceleratorText, (XmString)wstr, NULL);
      wstr = "Ctrl + 6";
      XtVaSetValues(editLeftInPB,   XmNacceleratorText, (XmString)wstr, NULL);
      wstr = "Meta + 6";
      XtVaSetValues(editLeftOutPB,  XmNacceleratorText, (XmString)wstr, NULL);
      wstr = "Ctrl + 7";
      XtVaSetValues(editRightInPB,  XmNacceleratorText, (XmString)wstr, NULL);
      wstr = "Meta + 7";
      XtVaSetValues(editRightOutPB, XmNacceleratorText, (XmString)wstr, NULL);
      wstr = "Ctrl + 8";
      XtVaSetValues(editExcMorePB,  XmNacceleratorText, (XmString)wstr, NULL);
      wstr = "Meta + 8";
      XtVaSetValues(editExcLessPB,  XmNacceleratorText, (XmString)wstr, NULL);

   } // End if editing in Emacs mode

   else {

      WXmString	wstr;
//      XtVaSetValues(editCB, XmNmnemonic, (KeySym)XK_E, NULL);
      wstr = "Ctrl + Z";
      XtVaSetValues(editUndeletePB, XmNacceleratorText, (XmString)wstr,0);
      wstr = "Ctrl + P";
      XtVaSetValues(editPlainPB,    XmNacceleratorText, (XmString)wstr, NULL);
      wstr = "Ctrl + B";
      XtVaSetValues(editBoldPB,     XmNacceleratorText, (XmString)wstr, NULL);
      wstr = "Ctrl + I";
      XtVaSetValues(editItalicPB,   XmNacceleratorText, (XmString)wstr, NULL);
      wstr = "Ctrl + F";
      XtVaSetValues(editFixedPB,    XmNacceleratorText, (XmString)wstr, NULL);
      wstr = "Ctrl + -";
      XtVaSetValues(editUnderPB,    XmNacceleratorText, (XmString)wstr, NULL);
      wstr = "Ctrl + .";
      XtVaSetValues(editBiggerPB,   XmNacceleratorText, (XmString)wstr, NULL);
      wstr = "Ctrl + ,";
      XtVaSetValues(editSmallerPB,  XmNacceleratorText, (XmString)wstr, NULL);
      wstr = "Ctrl + [";
      XtVaSetValues(editLeftPB,     XmNacceleratorText, (XmString)wstr, NULL);
      wstr = "Ctrl + ]";
      XtVaSetValues(editRightPB,    XmNacceleratorText, (XmString)wstr, NULL);
      wstr = "Ctrl + =";
      XtVaSetValues(editCenterPB,   XmNacceleratorText, (XmString)wstr, NULL);
      wstr = "Ctrl + Shift + .";
      XtVaSetValues(editLeftInPB,   XmNacceleratorText, (XmString)wstr, NULL);
      wstr = "Ctrl + Shift + ,";
      XtVaSetValues(editLeftOutPB,  XmNacceleratorText, (XmString)wstr, NULL);
      wstr = "Meta + .";
      XtVaSetValues(editExcMorePB,  XmNacceleratorText, (XmString)wstr, NULL);
      wstr = "Meta + ,";
      XtVaSetValues(editExcLessPB,  XmNacceleratorText, (XmString)wstr, NULL);

   } // End if editing in Motif mode

//
// Create editColorPD hierarchy
//
// editColorPD
//    PushButton	colorRedPB
//    PushButton	colorGreenPB
//    PushButton	colorBluePB
//    PushButton	colorYellowPB
//    PushButton	colorMagentaPB
//    PushButton	colorCyanPB
//    PushButton	colorBlackPB
//    PushButton	colorWhitePB
//    PushButton	colorOtherPB
//
   Widget	editColorPD = XmCreatePulldownMenu(editPD, "editColorPD", 0,0);
   XtVaSetValues(editColorCB, XmNsubMenuId, editColorPD, NULL);

   Widget colorRedPB     = XmCreatePushButton(editColorPD,"colorRedPB",    0,0);
   Widget colorGreenPB   = XmCreatePushButton(editColorPD,"colorGreenPB",  0,0);
   Widget colorBluePB    = XmCreatePushButton(editColorPD,"colorBluePB",   0,0);
   Widget colorYellowPB  = XmCreatePushButton(editColorPD,"colorYellowPB", 0,0);
   Widget colorMagentaPB = XmCreatePushButton(editColorPD,"colorMagentaPB",0,0);
   Widget colorCyanPB    = XmCreatePushButton(editColorPD,"colorCyanPB",   0,0);
   Widget colorBlackPB   = XmCreatePushButton(editColorPD,"colorBlackPB",  0,0);
   Widget colorWhitePB   = XmCreatePushButton(editColorPD,"colorWhitePB",  0,0);
   Widget colorOtherPB   = XmCreatePushButton(editColorPD,"colorOtherPB",  0,0);
   Widget colorSep1      = XmCreateSeparator (editColorPD,"colorSep1",     0,0);
   Widget colorNonePB    = XmCreatePushButton(editColorPD,"colorNonePB",   0,0);

   wcount = 0;
   wlist[wcount++] = colorRedPB;
   wlist[wcount++] = colorGreenPB;
   wlist[wcount++] = colorBluePB;
   wlist[wcount++] = colorYellowPB;
   wlist[wcount++] = colorMagentaPB;
   wlist[wcount++] = colorCyanPB;
   wlist[wcount++] = colorBlackPB;
   wlist[wcount++] = colorWhitePB;
//   wlist[wcount++] = colorOtherPB;
   wlist[wcount++] = colorSep1;
   wlist[wcount++] = colorNonePB;
   XtManageChildren(wlist, wcount);	// editColorPD children

   AddActivate(colorRedPB,		DoColorRed,		this);
   AddActivate(colorGreenPB,		DoColorGreen,		this);
   AddActivate(colorBluePB,		DoColorBlue,		this);
   AddActivate(colorYellowPB,		DoColorYellow,		this);
   AddActivate(colorMagentaPB,		DoColorMagenta,		this);
   AddActivate(colorCyanPB,		DoColorCyan,		this);
   AddActivate(colorBlackPB,		DoColorBlack,		this);
   AddActivate(colorWhitePB,		DoColorWhite,		this);
   AddActivate(colorOtherPB,		DoColorOther,		this);
   AddActivate(colorNonePB,		DoColorNone,		this);

} // End BuildEditMenu

/*---------------------------------------------------------------
 *  Method to build option menu hierarchy
 */

void
SendWinP::BuildOptMenu()
{
   Cardinal	wcount;
   Widget	wlist[24];
   WArgList	args;

   Widget	optCB = XmCreateCascadeButton(pub->menuBar, "optCB", 0,0);
   Widget	optPD = XmCreatePulldownMenu (pub->menuBar, "optPD", 0,0);
   XtVaSetValues(optCB, XmNsubMenuId, optPD, NULL);
   XtManageChild(optCB);

//
// Create optPD hierarchy
//
// optPD
//    PushButton	optPrefPB
//    PushButton	optMailPB
//    PushButton	optReplyPB
//    PushButton	optSigPB
//    PushButton	optButtPB
//    Separator		optSep1
//    ToggleButton	optCcTB
//    ToggleButton	optBccTB
//    ToggleButton	optFccTB
//    ToggleButton	optOtherTB
#if 0
//    ToggleButton	optWrapTB
//    ToggleButton	optAddSigTB
//    ToggleButton	optDigSignTB
//    ToggleButton	optEncryptTB
#endif
//    ToggleButton	optCheckAddrTB
//    CascadeButton	optMsgTypeCB
//    CascadeButton	optTextTypeCB
//
   wcount = 0;
   if ( !pub->IsEditOnly() ) {

      Widget optPrefPB  = XmCreatePushButton   (optPD, "optPrefPB",      0,0);
      Widget optMailPB  = XmCreatePushButton   (optPD, "optMailPB",      0,0);
      Widget optReplyPB = XmCreatePushButton   (optPD, "optReplyPB",     0,0);
      Widget optSigPB   = XmCreatePushButton   (optPD, "optSigPB",       0,0);
      Widget optButtPB  = XmCreatePushButton   (optPD, "optButtPB",      0,0);
      Widget optSep1    = XmCreateSeparator    (optPD, "optSep1",        0,0);

      optCcTB		= XmCreateToggleButton (optPD, "optCcTB",        0,0);
      optBccTB		= XmCreateToggleButton (optPD, "optBccTB",       0,0);
      optFccTB		= XmCreateToggleButton (optPD, "optFccTB",       0,0);
      optOtherTB	= XmCreateToggleButton (optPD, "optOtherTB",     0,0);

      wlist[wcount++] = optPrefPB;
      wlist[wcount++] = optMailPB;
      wlist[wcount++] = optSigPB;
      wlist[wcount++] = optReplyPB;
      wlist[wcount++] = optButtPB;
      wlist[wcount++] = optSep1;
      wlist[wcount++] = optCcTB;
      wlist[wcount++] = optBccTB;
      wlist[wcount++] = optFccTB;
      wlist[wcount++] = optOtherTB;
      XtManageChildren(wlist, wcount);

      AddActivate    (optPrefPB,  DoWinPrefs,   this);
      AddActivate    (optMailPB,  DoMailPrefs,  this);
      AddActivate    (optReplyPB, DoReplyPrefs, this);
      AddActivate    (optSigPB,   DoSigPrefs,   this);
      AddActivate    (optButtPB,  DoButtons,    this);
      AddValueChanged(optCcTB,    ToggleCc,     this);
      AddValueChanged(optBccTB,   ToggleBcc,    this);
      AddValueChanged(optFccTB,   ToggleFcc,    this);
      AddValueChanged(optOtherTB, ToggleOther,  this);

      XmToggleButtonSetState(optCcTB,    ishApp->compPrefs->showCc,    False);
      XmToggleButtonSetState(optBccTB,   ishApp->compPrefs->showBcc,   False);
      XmToggleButtonSetState(optFccTB,   ishApp->compPrefs->showFcc,   False);
      XmToggleButtonSetState(optOtherTB, ishApp->compPrefs->showOther, False);
   }

   if ( pub->IsEditOnly() ) {
      optWrapTB		= XmCreateToggleButton (optPD, "optWrapTB",      0,0);
      XtManageChild(optWrapTB);
      AddValueChanged(optWrapTB, ToggleWrap, this);
   }

   if ( !pub->IsEditOnly() ) {
   
#if 0
      optAddSigTB    = XmCreateToggleButton (optPD, "optAddSigTB",    0,0);
      optDigSignTB   = XmCreateToggleButton (optPD, "optDigSignTB",   0,0);
      optEncryptTB   = XmCreateToggleButton (optPD, "optEncryptTB",   0,0);
#endif
      optCheckAddrTB = XmCreateToggleButton (optPD, "optCheckAddrTB", 0,0);
      optMsgTypeCB   = XmCreateCascadeButton(optPD, "optMsgTypeCB",   0,0);
      optTextTypeCB  = XmCreateCascadeButton(optPD, "optTextTypeCB",  0,0);

      wcount = 0;
#if 0
      wlist[wcount++] = optAddSigTB;
      wlist[wcount++] = optDigSignTB;
      wlist[wcount++] = optEncryptTB;
#endif
      wlist[wcount++] = optCheckAddrTB;
      wlist[wcount++] = optMsgTypeCB;
      wlist[wcount++] = optTextTypeCB;
      XtManageChildren(wlist, wcount);

//
// Create optMsgTypePD hierarchy
//
// optMsgTypePD
//    ToggleButton	optMsgPlainTB
//    ToggleButton	optMsgMimeTB
//    ToggleButton	optMsgAltTB
//
      Widget	optMsgTypePD = XmCreatePulldownMenu(optPD, "optMsgTypePD", 0,0);
      XtVaSetValues(optMsgTypeCB, XmNsubMenuId, optMsgTypePD, NULL);
      XtVaSetValues(optMsgTypePD, XmNradioBehavior, True, NULL);

      optMsgPlainTB = XmCreateToggleButton(optMsgTypePD, "optMsgPlainTB", 0,0);
      optMsgMimeTB  = XmCreateToggleButton(optMsgTypePD, "optMsgMimeTB",  0,0);
      optMsgAltTB   = XmCreateToggleButton(optMsgTypePD, "optMsgAltTB",   0,0);

      wcount = 0;
      wlist[wcount++] = optMsgPlainTB;
      wlist[wcount++] = optMsgMimeTB;
      wlist[wcount++] = optMsgAltTB;
      XtManageChildren(wlist, wcount);

      AddValueChanged(optMsgPlainTB, ToggleMsgPlain, this);

//
// Create optTextTypePD hierarchy
//
// optTextTypePD
//    ToggleButton	optTextPlainTB
//    ToggleButton	optTextRichTB
//
      Widget optTextTypePD = XmCreatePulldownMenu(optPD, "optTextTypePD", 0,0);
      XtVaSetValues(optTextTypeCB, XmNsubMenuId, optTextTypePD, NULL);
      XtVaSetValues(optTextTypePD, XmNradioBehavior, True, NULL);

      optTextPlainTB = XmCreateToggleButton(optTextTypePD,"optTextPlainTB",0,0);
      optTextRichTB  = XmCreateToggleButton(optTextTypePD,"optTextRichTB", 0,0);

      wcount = 0;
      wlist[wcount++] = optTextPlainTB;
      wlist[wcount++] = optTextRichTB;
      XtManageChildren(wlist, wcount);

      AddValueChanged(optTextPlainTB, ToggleTextPlain, this);

   } // End if not edit-only

} // End BuildOptMenu

/*---------------------------------------------------------------
 *  Method to build widgets
 */

void
SendWinP::BuildWidgets()
{
   WArgList	args;

//
// Create appForm hierarchy
//
// appForm
//    PanedWin		appPanes

   args.Reset();
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.TopAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   appPanes = XmCreatePanedWindow(pub->appForm, "appPanes", ARGS);

//
// Create appPanes hierarchy
//
// appPanes
//    Form		oldHeadPane.headForm	(Used if main window)
//    Form		newHeadPane.headForm	(Used if main window)
//    MimeRichTextC	headText		(Used if edit window)
//    Form		bodyForm
//    RowCol		optRC			(Used if main window)
//

//
// Create header fields
//
   int	row = 0;
   toRow    = row++;
   ccRow    = row++;
   bccRow   = row++;
   fccRow   = row++;
   otherRow = row++;

   if ( !pub->IsEditOnly() ) {

      oldHeadPane = new HeaderPaneT;
      newHeadPane = new HeaderPaneT;
      InitHeaderPane(oldHeadPane);
      InitHeaderPane(newHeadPane);

      args.Reset();
      args.AllowResize(True);
      oldHeadPane->headForm = XmCreateForm(appPanes, "headForm", ARGS);
      newHeadPane->headForm = XmCreateForm(appPanes, "headForm", ARGS);

      BuildOldHeadPane();
      BuildNewHeadPane();
      PlaceHeaderFields();

   } // End if header form needed

//
// Create header text if necessary
//
   if ( pub->IsEditOnly() ) {

      args.Reset();
      args.AllowResize(True);
      headText = new MimeRichTextC(appPanes, "headerText", ARGS);
      headText->SetEditable(True);
      headText->ResizeWidth(False);

      headText->AddTextChangeCallback((CallbackFn*)TextChanged, this);
   }

//
// Create bodyForm hierarchy
//
// bodyForm
//    Form		bodyTitleForm
//       Label		bodyTitle
//       Text		bodyStateTF
//    ScrolledWindow	bodyWin
//
   args.Reset();
   args.AllowResize(True);
   bodyForm = XmCreateForm(appPanes, "bodyForm", ARGS);

   args.Reset();
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.TopAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_NONE);
   Widget	bodyTitleForm = XmCreateForm(bodyForm, "bodyTitleForm", ARGS);

   args.Reset();
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.TopAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   Widget	bodyTitle = XmCreateLabel(bodyTitleForm, "bodyTitle", ARGS);

   args.Reset();
   args.LeftAttachment(XmATTACH_WIDGET, bodyTitle);
   args.TopAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   args.ShadowThickness(0);
   args.Editable(False);
   args.CursorPositionVisible(False);
   args.TraversalOn(False);
   args.NavigationType(XmNONE);
   bodyStateTF = CreateTextField(bodyTitleForm, "bodyStateTF", ARGS);

   args.Reset();
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.TopAttachment(XmATTACH_WIDGET, bodyTitleForm);
   args.BottomAttachment(XmATTACH_FORM);
   args.UserData(this);
   args.Rows(ishApp->compPrefs->bodyRows);
   args.Columns(ishApp->compPrefs->bodyCols);
   bodyText = new MimeRichTextC(bodyForm, "bodyText", ARGS);
   bodyText->SetEditable(True);
   bodyText->ResizeWidth(!ishApp->compPrefs->wrap);
   bodyText->SetTextType(TT_ENRICHED);
   bodyText->SetExcerptString(ishApp->replyPrefs->indentPrefix);
   bodyText->ForceFixed(ishApp->mailPrefs->mailType == MAIL_PLAIN ||
			ishApp->mailPrefs->textType == CT_PLAIN);

   bodyText->AddStateChangeCallback((CallbackFn*)BodyStateChanged, this);
   bodyText->AddTextChangeCallback ((CallbackFn*)TextChanged,      this);

   XtManageChild(bodyTitle);
   XtManageChild(bodyStateTF);
   XtManageChild(bodyTitleForm);
   XtManageChild(bodyText->MainWidget());

//
// Allow message drops here
//
   XmDropSiteUnregister(bodyText->TextArea());

   if ( !graphicAtom )
      graphicAtom = XmInternAtom(halApp->display, "IshmailGraphic", False);

   Atom impAtoms[2];
   impAtoms[0] = ishApp->msgAtom;
   impAtoms[1] = graphicAtom;

   args.Reset();
   args.ImportTargets(impAtoms);
   args.NumImportTargets(2);
   args.DragProc((XtCallbackProc)HandleDragOver);
   args.DropProc((XtCallbackProc)HandleDrop);
   XmDropSiteRegister(bodyText->TextArea(), ARGS);

   XtManageChild(bodyForm);
   if ( headText ) XtManageChild(headText->MainWidget());

//
// Create optRC if necessary
//
   optRC = NULL;
   if ( !pub->IsEditOnly() ) {

      args.Reset();
      args.AllowResize(True);
      optRC = new RowColC(appPanes, "optRC", ARGS);

//
// Set up optRC for 1 row with equal sized columns
//
      optRC->Defer(True);
      optRC->SetOrientation(RcCOL_MAJOR);
      optRC->SetRowCount(1);
      optRC->SetColAlignment(XmALIGNMENT_BEGINNING);
      optRC->SetColWidthAdjust(RcADJUST_NONE);
      optRC->SetColResize(True);
      //optRC->SetUniformCols(True);
      optRC->SetUniformCols(False);

//
// Create optRC hierarchy
//
// optRC
//    ToggleButton	optMimeTB
//    ToggleButton	optWrapTB
//    ToggleButton	optAddSigTB
//    ToggleButton	optDigSignTB
//    ToggleButton	optEncryptTB
//
      optMimeTB    = XmCreateToggleButton(*optRC, "optMimeTB",    0,0);
      optWrapTB	   = XmCreateToggleButton(*optRC, "optWrapTB",    0,0);
      optAddSigTB  = XmCreateToggleButton(*optRC, "optAddSigTB",  0,0);
      optDigSignTB = XmCreateToggleButton(*optRC, "optDigSignTB", 0,0);
      optEncryptTB = XmCreateToggleButton(*optRC, "optEncryptTB", 0,0);

      Widget	wlist[5];
      Cardinal	wcount = 0;
      wlist[wcount++] = optMimeTB;
      wlist[wcount++] = optWrapTB;
      wlist[wcount++] = optAddSigTB;
      wlist[wcount++] = optDigSignTB;
      wlist[wcount++] = optEncryptTB;
      optRC->SetChildren(wlist, wcount);
      optRC->Defer(False);

      AddValueChanged(optMimeTB, ToggleMime, this);
      AddValueChanged(optWrapTB, ToggleWrap, this);

      XtManageChild(*optRC);

   } // End if need options panel

   XtManageChild(appPanes);	// appForm children

//
// Add other stuff
//
   BuildMimePopupMenu();
   pub->AddButtonBox();
   pub->ShowInfoMsg();	// Show this hidden widget

   pub->HandleHelp();

//
// Add actions if necessary
//
   static Boolean	actionsAdded = False;
   if ( !actionsAdded ) {
      XtAppAddActions(halApp->context, actions, XtNumber(actions));
      actionsAdded = True;
   }

//
// Create the button preferences object
//
   StringC	buttonStr = ishApp->sendButtPrefs->buttonStr;
   if ( pub->IsEditOnly() ) buttonStr += " fileCancelPB";	// Force this on
   buttMgr = new ButtonMgrC(pub, pub->menuBar, pub->buttonRC, buttonStr);

//
// Create pixmap for this window's icon
//
   compIcon = new PixmapC(composition_xpm, (Window)*halApp,
			  XDefaultColormapOfScreen(halApp->screen));

   if ( compIcon->reg )
      XtVaSetValues(*pub, XmNiconPixmap, compIcon->reg,
                          XmNiconMask, compIcon->mask,
                          NULL);

} // End BuildWidgets

/*---------------------------------------------------------------
 *  Routine to initialize header pane structure
 */

void
SendWinP::InitHeaderPane(HeaderPaneT *pane)
{
   pane->headForm	= NULL;
   pane->toForm		= NULL;
   pane->ccForm		= NULL;
   pane->bccForm	= NULL;
   pane->fccForm	= NULL;
   pane->otherForm	= NULL;
   pane->toLabel	= NULL;
   pane->ccLabel	= NULL;
   pane->bccLabel	= NULL;
   pane->fccLabel	= NULL;
   pane->otherLabel	= NULL;
   pane->toText		= NULL;
   pane->ccText		= NULL;
   pane->bccText	= NULL;
   pane->fccText	= NULL;
   pane->otherText	= NULL;
   pane->toAliasPB	= NULL;
   pane->ccAliasPB	= NULL;
   pane->bccAliasPB	= NULL;

} // End InitHeaderPane

/*---------------------------------------------------------------
 *  Routine to create the old-style header pane
 */

void
SendWinP::BuildOldHeadPane()
{
   WArgList	args;
   Widget	wlist[6];
   Cardinal	wcount;

   HeaderPaneT	*p = oldHeadPane;

//
// Create headForm hierarchy
//
// headForm
//    Form	labelForm
//    Form	textForm
//    Form	buttonForm
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   Widget labelForm  = XmCreateForm(p->headForm, "labelForm",  ARGS);

   args.LeftAttachment(XmATTACH_NONE);
   args.RightAttachment(XmATTACH_FORM);
   Widget buttonForm = XmCreateForm(p->headForm, "buttonForm", ARGS);

   args.LeftAttachment(XmATTACH_WIDGET, labelForm);
   args.RightAttachment(XmATTACH_WIDGET, buttonForm);
   Widget textForm   = XmCreateForm(p->headForm, "textForm",   ARGS);

//
// Create labelForm hierarchy
//
// labelForm
//    Label		toLabel
//    Label		subLabel
//    Label		ccLabel
//    Label		bccLabel
//    Label		fccLabel
//    Label		otherLabel
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_NONE);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_NONE);
   p->toLabel    = XmCreateLabel(labelForm, "toLabel",      ARGS);
   p->subLabel   = XmCreateLabel(labelForm, "subjectLabel", ARGS);
   p->ccLabel    = XmCreateLabel(labelForm, "ccLabel",      ARGS);
   p->bccLabel   = XmCreateLabel(labelForm, "bccLabel",     ARGS);
   p->fccLabel   = XmCreateLabel(labelForm, "fccLabel",     ARGS);
   p->otherLabel = XmCreateLabel(labelForm, "otherLabel",   ARGS);

   wcount = 0;
   wlist[wcount++] = p->toLabel;
   wlist[wcount++] = p->subLabel;
   wlist[wcount++] = p->ccLabel;
   wlist[wcount++] = p->bccLabel;
   wlist[wcount++] = p->fccLabel;
   wlist[wcount++] = p->otherLabel;
   XtManageChildren(wlist, wcount);

//
// Create textForm hierarchy
//
// textForm
//    MimeRichTextC	toText
//    MimeRichTextC	subText
//    MimeRichTextC	ccText
//    MimeRichTextC	bccText
//    MimeRichTextC	fccText
//    MimeRichTextC	otherText
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_NONE);
   p->toText    = new MimeRichTextC(textForm, "toText",      ARGS);
   p->subText   = new MimeRichTextC(textForm, "subjectText", ARGS);
   p->ccText    = new MimeRichTextC(textForm, "ccText",      ARGS);
   p->bccText   = new MimeRichTextC(textForm, "bccText",     ARGS);
   p->fccText   = new MimeRichTextC(textForm, "fccText",     ARGS);
   p->otherText = new MimeRichTextC(textForm, "otherText",   ARGS);

   p->toText->Defer(True);
   p->toText->SetEditable(True);
   p->toText->ResizeWidth(True);
   p->toText->Defer(False);

   p->subText->Defer(True);
   p->subText->SetEditable(True);
   p->subText->ResizeWidth(True);
   p->subText->Defer(False);

   p->ccText->Defer(True);
   p->ccText->SetEditable(True);
   p->ccText->ResizeWidth(True);
   p->ccText->Defer(False);

   p->bccText->Defer(True);
   p->bccText->SetEditable(True);
   p->bccText->ResizeWidth(True);
   p->bccText->Defer(False);

   p->fccText->Defer(True);
   p->fccText->SetEditable(True);
   p->fccText->ResizeWidth(False);
   p->fccText->Defer(False);

   p->otherText->Defer(True);
   p->otherText->SetEditable(True);
   p->otherText->ResizeWidth(True);
   p->otherText->Defer(False);

//
// Add callbacks to look for changes
//
   p->toText->AddTextChangeCallback   ((CallbackFn*)TextChanged, this);
   p->subText->AddTextChangeCallback  ((CallbackFn*)TextChanged, this);
   p->ccText->AddTextChangeCallback   ((CallbackFn*)TextChanged, this);
   p->bccText->AddTextChangeCallback  ((CallbackFn*)TextChanged, this);
   p->otherText->AddTextChangeCallback((CallbackFn*)TextChanged, this);
   p->fccText->AddTextChangeCallback  ((CallbackFn*)TextChanged, this);

   wcount = 0;
   wlist[wcount++] = p->toText->MainWidget();
   wlist[wcount++] = p->subText->MainWidget();
   wlist[wcount++] = p->ccText->MainWidget();
   wlist[wcount++] = p->bccText->MainWidget();
   wlist[wcount++] = p->fccText->MainWidget();
   wlist[wcount++] = p->otherText->MainWidget();
   XtManageChildren(wlist, wcount);

//
// Create buttonForm hierarchy
//
// buttonForm
//    PushButton	   toAliasPB
//    PushButton	   ccAliasPB
//    PushButton	   bccAliasPB
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_NONE);
   p->toAliasPB    = XmCreatePushButton(buttonForm,  "toAliasPB",   ARGS);
   p->subDummyPB   = XmCreatePushButton(buttonForm, "subAliasPB",   ARGS);
   p->ccAliasPB    = XmCreatePushButton(buttonForm,  "ccAliasPB",   ARGS);
   p->bccAliasPB   = XmCreatePushButton(buttonForm, "bccAliasPB",   ARGS);
   p->fccDummyPB   = XmCreatePushButton(buttonForm, "fccAliasPB",   ARGS);
   p->otherDummyPB = XmCreatePushButton(buttonForm, "otherAliasPB", ARGS);

//
// These buttons are present to simplify the layout code
//
   args.Reset();
   args.MappedWhenManaged(False);
   XtSetValues(p->subDummyPB, ARGS);
   XtSetValues(p->fccDummyPB, ARGS);
   XtSetValues(p->otherDummyPB, ARGS);

//
// Add callbacks to edit aliases
//
   AddActivate(p->toAliasPB,  PickToAlias,  this);
   AddActivate(p->ccAliasPB,  PickCcAlias,  this);
   AddActivate(p->bccAliasPB, PickBccAlias, this);

   wcount = 0;
   wlist[wcount++] = p->toAliasPB;
   wlist[wcount++] = p->ccAliasPB;
   wlist[wcount++] = p->bccAliasPB;
   XtManageChildren(wlist, wcount);

   wcount = 0;
   wlist[wcount++] = labelForm;
   wlist[wcount++] = textForm;
   wlist[wcount++] = buttonForm;
   XtManageChildren(wlist, wcount);

} // End BuildOldHeadPane

/*---------------------------------------------------------------
 *  Routine to create the new-style header pane
 */

void
SendWinP::BuildNewHeadPane()
{
   WArgList	args;

   HeaderPaneT	*p = newHeadPane;

//
// Create headForm hierarchy
//
// headForm
//   Form		toForm
//   Form		subForm
//   Form		ccForm
//   Form		bccForm
//   Form		fccForm
//   Form		otherForm
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_NONE);
   p->toForm    = XmCreateForm(p->headForm, "toForm",      ARGS);
   p->subForm   = XmCreateForm(p->headForm, "subjectForm", ARGS);
   p->ccForm    = XmCreateForm(p->headForm, "ccForm",      ARGS);
   p->bccForm   = XmCreateForm(p->headForm, "bccForm",     ARGS);
   p->fccForm   = XmCreateForm(p->headForm, "fccForm",     ARGS);
   p->otherForm = XmCreateForm(p->headForm, "otherForm",   ARGS);

//
// Create ??Form hierarchy
//
// ??Form
//    Form		??TopForm
//       Label		  ??Label
//       PushButton	  ??AliasPB
//    MimeRichTextC	??Text
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_NONE);
   Widget toTopForm    = XmCreateForm(p->toForm,    "toTopForm",    ARGS);
   Widget subTopForm   = XmCreateForm(p->subForm,   "subTopForm",   ARGS);
   Widget ccTopForm    = XmCreateForm(p->ccForm,    "ccTopForm",    ARGS);
   Widget bccTopForm   = XmCreateForm(p->bccForm,   "bccTopForm",   ARGS);
   Widget fccTopForm   = XmCreateForm(p->fccForm,   "fccTopForm",   ARGS);
   Widget otherTopForm = XmCreateForm(p->otherForm, "otherTopForm", ARGS);

   args.Reset();
   args.TopAttachment(XmATTACH_NONE);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   p->toLabel    = XmCreateLabel(toTopForm,    "toLabel",      ARGS);
   p->subLabel   = XmCreateLabel(subTopForm,   "subjectLabel", ARGS);
   p->ccLabel    = XmCreateLabel(ccTopForm,    "ccLabel",      ARGS);
   p->bccLabel   = XmCreateLabel(bccTopForm,   "bccLabel",     ARGS);
   p->fccLabel   = XmCreateLabel(fccTopForm,   "fccLabel",     ARGS);
   p->otherLabel = XmCreateLabel(otherTopForm, "otherLabel",   ARGS);

   args.LeftAttachment(XmATTACH_NONE);
   args.RightAttachment(XmATTACH_FORM);
   p->toAliasPB  = XmCreatePushButton(toTopForm,    "toAliasPB",    ARGS);
   p->ccAliasPB  = XmCreatePushButton(ccTopForm,    "ccAliasPB",    ARGS);
   p->bccAliasPB = XmCreatePushButton(bccTopForm,   "bccAliasPB",   ARGS);

//
// Create these buttons to simplify the layout code
//
   args.MappedWhenManaged(False);
   p->subDummyPB   = XmCreatePushButton(subTopForm,   "subAliasPB",   ARGS);
   p->fccDummyPB   = XmCreatePushButton(fccTopForm,   "fccAliasPB",   ARGS);
   p->otherDummyPB = XmCreatePushButton(otherTopForm, "otherAliasPB", ARGS);

   XtManageChild(p->toLabel);
   XtManageChild(p->toAliasPB);
   XtManageChild(toTopForm);
   XtManageChild(p->subLabel);
   XtManageChild(p->subDummyPB);
   XtManageChild(subTopForm);
   XtManageChild(p->ccLabel);
   XtManageChild(p->ccAliasPB);
   XtManageChild(ccTopForm);
   XtManageChild(p->bccLabel);
   XtManageChild(p->bccAliasPB);
   XtManageChild(bccTopForm);
   XtManageChild(p->fccLabel);
   XtManageChild(p->fccDummyPB);
   XtManageChild(fccTopForm);
   XtManageChild(p->otherLabel);
   XtManageChild(p->otherDummyPB);
   XtManageChild(otherTopForm);

   args.Reset();
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);

   args.TopAttachment(XmATTACH_WIDGET, toTopForm);
   p->toText    = new MimeRichTextC(p->toForm, "toText", ARGS);
   p->toText->Defer(True);
   p->toText->SetEditable(True);
   p->toText->ResizeWidth(True);
//   p->toText->SetString("_");
   p->toText->Defer(False);

   args.TopAttachment(XmATTACH_WIDGET, subTopForm);
   p->subText    = new MimeRichTextC(p->subForm, "subjectText", ARGS);
   p->subText->Defer(True);
   p->subText->SetEditable(True);
   p->subText->ResizeWidth(True);
//   p->subText->SetString("_");
   p->subText->Defer(False);

   args.TopAttachment(XmATTACH_WIDGET, ccTopForm);
   p->ccText    = new MimeRichTextC(p->ccForm, "ccText", ARGS);
   p->ccText->Defer(True);
   p->ccText->SetEditable(True);
   p->ccText->ResizeWidth(True);
//   p->ccText->SetString("_");
   p->ccText->Defer(False);

   args.TopAttachment(XmATTACH_WIDGET, bccTopForm);
   p->bccText    = new MimeRichTextC(p->bccForm, "bccText", ARGS);
   p->bccText->Defer(True);
   p->bccText->SetEditable(True);
   p->bccText->ResizeWidth(True);
//   p->bccText->SetString("_");
   p->bccText->Defer(False);

   args.TopAttachment(XmATTACH_WIDGET, fccTopForm);
   p->fccText    = new MimeRichTextC(p->fccForm, "fccText", ARGS);
   p->fccText->Defer(True);
   p->fccText->SetEditable(True);
   p->fccText->ResizeWidth(True);
//   p->fccText->SetString("_");
   p->fccText->Defer(False);

   args.TopAttachment(XmATTACH_WIDGET, otherTopForm);
   p->otherText    = new MimeRichTextC(p->otherForm, "otherText", ARGS);
   p->otherText->Defer(True);
   p->otherText->SetEditable(True);
   p->otherText->ResizeWidth(True);
//   p->otherText->SetString("_");
   p->otherText->Defer(False);

   XtManageChild(p->toText->MainWidget());
   XtManageChild(p->subText->MainWidget());
   XtManageChild(p->ccText->MainWidget());
   XtManageChild(p->bccText->MainWidget());
   XtManageChild(p->fccText->MainWidget());
   XtManageChild(p->otherText->MainWidget());

//
// Add callbacks to look for changes to fields
//
   p->toText->AddTextChangeCallback   ((CallbackFn*)TextChanged, this);
   p->subText->AddTextChangeCallback  ((CallbackFn*)TextChanged, this);
   p->ccText->AddTextChangeCallback   ((CallbackFn*)TextChanged, this);
   p->bccText->AddTextChangeCallback  ((CallbackFn*)TextChanged, this);
   p->otherText->AddTextChangeCallback((CallbackFn*)TextChanged, this);
   p->fccText->AddTextChangeCallback  ((CallbackFn*)TextChanged, this);

//
// Add callbacks to edit aliases
//
   AddActivate(p->toAliasPB,  PickToAlias,  this);
   AddActivate(p->ccAliasPB,  PickCcAlias,  this);
   AddActivate(p->bccAliasPB, PickBccAlias, this);

} // End BuildNewHeadPane

/*---------------------------------------------------------------
 *  Routine to position the header fields
 */

void
SendWinP::PlaceHeaderFields()
{
   u_int	visCount = 2;	// To and Subject
   if ( pub->ccVis    ) visCount++;
   if ( pub->bccVis   ) visCount++;
   if ( pub->fccVis   ) visCount++;
   if ( pub->otherVis ) visCount++;

//
// See which pane we need
//
   HeaderPaneT	*pane = (ishApp->compPrefs->maxFieldsPerLine <= 1)
		      ? oldHeadPane : newHeadPane;

//
// See if there are any changes
//
   Boolean rowsChanged =
      (pane->toText->RowCount() != ishApp->compPrefs->headRows);

   Boolean changed = (rowsChanged || pane != curHeadPane ||
		pub->maxFieldsPerLine != ishApp->compPrefs->maxFieldsPerLine);

   pub->maxFieldsPerLine = ishApp->compPrefs->maxFieldsPerLine;

//
// Save the width of the pane.  We'll need to restore it later
//
   Dimension	paneWd = 0;
   if ( curHeadPane ) {
      XtVaGetValues(curHeadPane->headForm, XmNwidth, &paneWd, NULL);
      XtUnmanageChild(curHeadPane->headForm);
   }

//
// If we've got only one per line, turn on the visible rows
//
   WArgList	args;

   if ( pub->maxFieldsPerLine <= 1 ) {

//
// Build lists of the visible and hidden fields
//
      WidgetListC	visLabList, hidLabList;
      WidgetListC	visTexList, hidTexList;
      WidgetListC	visButList, hidButList;

      visLabList.add(pane->toLabel);
      visTexList.add(pane->toText->MainWidget());
      visButList.add(pane->toAliasPB);

      visLabList.add(pane->subLabel);
      visTexList.add(pane->subText->MainWidget());
      visButList.add(pane->subDummyPB);

      if ( pub->ccVis ) {
	 visLabList.add(pane->ccLabel);
	 visTexList.add(pane->ccText->MainWidget());
	 visButList.add(pane->ccAliasPB);
      }
      else {
	 hidLabList.add(pane->ccLabel);
	 hidTexList.add(pane->ccText->MainWidget());
	 hidButList.add(pane->ccAliasPB);
      }
      if ( pub->bccVis ) {
	 visLabList.add(pane->bccLabel);
	 visTexList.add(pane->bccText->MainWidget());
	 visButList.add(pane->bccAliasPB);
      }
      else {
	 hidLabList.add(pane->bccLabel);
	 hidTexList.add(pane->bccText->MainWidget());
	 hidButList.add(pane->bccAliasPB);
      }
      if ( pub->fccVis ) {
	 visLabList.add(pane->fccLabel);
	 visTexList.add(pane->fccText->MainWidget());
	 visButList.add(pane->fccDummyPB);
      }
      else {
	 hidLabList.add(pane->fccLabel);
	 hidTexList.add(pane->fccText->MainWidget());
	 hidButList.add(pane->fccDummyPB);
      }
      if ( pub->otherVis ) {
	 visLabList.add(pane->otherLabel);
	 visTexList.add(pane->otherText->MainWidget());
	 visButList.add(pane->otherDummyPB);
      }
      else {
	 hidLabList.add(pane->otherLabel);
	 hidTexList.add(pane->otherText->MainWidget());
	 hidButList.add(pane->otherDummyPB);
      }

      visCount = visLabList.size();
      int	i;
      for (i=0; !changed && i<visCount; i++) {
	 Widget	w = *visLabList[i];
	 if ( !XtIsManaged(w) ) changed = True;
      }

      u_int	hidCount = hidLabList.size();
      for (i=0; !changed && i<hidCount; i++) {
	 Widget	w = *hidLabList[i];
	 if ( XtIsManaged(w) ) changed = True;
      }

      if ( !changed ) return;

//
// Unmanage all children
//
      if ( hidLabList.size() > 0 ) {
	 XtUnmanageChildren(hidLabList.start(), hidLabList.size());
	 XtUnmanageChildren(hidTexList.start(), hidTexList.size());
	 if ( hidButList.size() > 0 )
	    XtUnmanageChildren(hidButList.start(), hidButList.size());
      }
      if ( visLabList.size() > 0 ) {
	 XtUnmanageChildren(visLabList.start(), visLabList.size());
	 XtUnmanageChildren(visTexList.start(), visTexList.size());
	 if ( visButList.size() > 0 )
	    XtUnmanageChildren(visButList.start(), visButList.size());
      }

#if 0
//
// Set position attachments
//
      Widget	lw1 = pane->toLabel;
      Widget	tw1 = pane->toText->MainWidget();
      Widget	bw1 = pane->toAliasPB;
      Widget	lw2 = pane->subLabel;
      Widget	tw2 = pane->subText->MainWidget();
      Widget	bw2 = pane->subDummyPB;

//
// Get widget ids of parent forms
//
      Widget	labForm = XtParent(lw1);
      Widget	texForm = XtParent(tw1);
      Widget	butForm = XtParent(bw1);
#else
      Widget	labForm = XtParent(pane->toLabel);
      Widget	texForm = XtParent(pane->toText->MainWidget());
      Widget	butForm = XtParent(pane->toAliasPB);
#endif

      args.FractionBase(visCount);
      XtSetValues(labForm, ARGS);
      XtSetValues(texForm, ARGS);
      XtSetValues(butForm, ARGS);
      args.Reset();

#if 0
      if ( visCount == 2 ) {

	 args.TopAttachment(XmATTACH_FORM);
	 args.BottomAttachment(XmATTACH_POSITION, 1);
	 XtSetValues(tw1, ARGS);

	 args.BottomAttachment(XmATTACH_NONE);
	 XtSetValues(lw1, ARGS);
	 XtSetValues(bw1, ARGS);

	 args.TopAttachment(XmATTACH_POSITION, 1);
	 args.BottomAttachment(XmATTACH_FORM);
	 XtSetValues(tw2, ARGS);

	 args.BottomAttachment(XmATTACH_NONE);
	 XtSetValues(lw2, ARGS);
	 XtSetValues(bw2, ARGS);
      }

      else if ( visCount == 3 ) {

	 Widget	lw3 = *visLabList[2];
	 Widget	tw3 = *visTexList[2];
	 Widget	bw3 = *visButList[2];

	 args.TopAttachment(XmATTACH_FORM);
	 args.BottomAttachment(XmATTACH_POSITION, 1);
	 XtSetValues(tw1, ARGS);

	 args.BottomAttachment(XmATTACH_NONE);
	 XtSetValues(lw1, ARGS);
	 XtSetValues(bw1, ARGS);

	 args.TopAttachment(XmATTACH_POSITION, 1);
	 args.BottomAttachment(XmATTACH_POSITION, 2);
	 XtSetValues(tw2, ARGS);

	 args.BottomAttachment(XmATTACH_NONE);
	 XtSetValues(lw2, ARGS);
	 XtSetValues(bw2, ARGS);

	 args.TopAttachment(XmATTACH_POSITION, 2);
	 args.BottomAttachment(XmATTACH_FORM);
	 XtSetValues(tw3, ARGS);

	 args.BottomAttachment(XmATTACH_NONE);
	 XtSetValues(lw3, ARGS);
	 XtSetValues(bw3, ARGS);
      }

      else if ( visCount == 4 ) {

	 Widget	lw3 = *visLabList[2];
	 Widget	tw3 = *visTexList[2];
	 Widget	bw3 = *visButList[2];
	 Widget	lw4 = *visLabList[3];
	 Widget	tw4 = *visTexList[3];
	 Widget	bw4 = *visButList[3];

	 args.TopAttachment(XmATTACH_FORM);
	 args.BottomAttachment(XmATTACH_POSITION, 1);
	 XtSetValues(tw1, ARGS);

	 args.BottomAttachment(XmATTACH_NONE);
	 XtSetValues(lw1, ARGS);
	 XtSetValues(bw1, ARGS);

	 args.TopAttachment(XmATTACH_POSITION, 1);
	 args.BottomAttachment(XmATTACH_POSITION, 2);
	 XtSetValues(tw2, ARGS);

	 args.BottomAttachment(XmATTACH_NONE);
	 XtSetValues(lw2, ARGS);
	 XtSetValues(bw2, ARGS);

	 args.TopAttachment(XmATTACH_POSITION, 2);
	 args.BottomAttachment(XmATTACH_POSITION, 3);
	 XtSetValues(tw3, ARGS);

	 args.BottomAttachment(XmATTACH_NONE);
	 XtSetValues(lw3, ARGS);
	 XtSetValues(bw3, ARGS);

	 args.TopAttachment(XmATTACH_POSITION, 3);
	 args.BottomAttachment(XmATTACH_FORM);
	 XtSetValues(tw4, ARGS);

	 args.BottomAttachment(XmATTACH_NONE);
	 XtSetValues(lw4, ARGS);
	 XtSetValues(bw4, ARGS);
      }

      else if ( visCount == 5 ) {

	 Widget	lw3 = *visLabList[2];
	 Widget	tw3 = *visTexList[2];
	 Widget	bw3 = *visButList[2];
	 Widget	lw4 = *visLabList[3];
	 Widget	tw4 = *visTexList[3];
	 Widget	bw4 = *visButList[3];
	 Widget	lw5 = *visLabList[4];
	 Widget	tw5 = *visTexList[4];
	 Widget	bw5 = *visButList[4];

	 args.TopAttachment(XmATTACH_FORM);
	 args.BottomAttachment(XmATTACH_POSITION, 1);
	 XtSetValues(tw1, ARGS);

	 args.BottomAttachment(XmATTACH_NONE);
	 XtSetValues(lw1, ARGS);
	 XtSetValues(bw1, ARGS);

	 args.TopAttachment(XmATTACH_POSITION, 1);
	 args.BottomAttachment(XmATTACH_POSITION, 2);
	 XtSetValues(tw2, ARGS);

	 args.BottomAttachment(XmATTACH_NONE);
	 XtSetValues(lw2, ARGS);
	 XtSetValues(bw2, ARGS);

	 args.TopAttachment(XmATTACH_POSITION, 2);
	 args.BottomAttachment(XmATTACH_POSITION, 3);
	 XtSetValues(tw3, ARGS);

	 args.BottomAttachment(XmATTACH_NONE);
	 XtSetValues(lw3, ARGS);
	 XtSetValues(bw3, ARGS);

	 args.TopAttachment(XmATTACH_POSITION, 3);
	 args.BottomAttachment(XmATTACH_POSITION, 4);
	 XtSetValues(tw4, ARGS);

	 args.BottomAttachment(XmATTACH_NONE);
	 XtSetValues(lw4, ARGS);
	 XtSetValues(bw4, ARGS);

	 args.TopAttachment(XmATTACH_POSITION, 4);
	 args.BottomAttachment(XmATTACH_FORM);
	 XtSetValues(tw5, ARGS);

	 args.BottomAttachment(XmATTACH_NONE);
	 XtSetValues(lw5, ARGS);
	 XtSetValues(bw5, ARGS);
      }

      else {	// All six visible

	 Widget	lw3 = *visLabList[2];
	 Widget	tw3 = *visTexList[2];
	 Widget	bw3 = *visButList[2];
	 Widget	lw4 = *visLabList[3];
	 Widget	tw4 = *visTexList[3];
	 Widget	bw4 = *visButList[3];
	 Widget	lw5 = *visLabList[4];
	 Widget	tw5 = *visTexList[4];
	 Widget	bw5 = *visButList[4];
	 Widget	lw6 = *visLabList[5];
	 Widget	tw6 = *visTexList[5];
	 Widget	bw6 = *visButList[5];

	 args.TopAttachment(XmATTACH_FORM);
	 args.BottomAttachment(XmATTACH_POSITION, 1);
	 XtSetValues(tw1, ARGS);

	 args.BottomAttachment(XmATTACH_NONE);
	 XtSetValues(lw1, ARGS);
	 XtSetValues(bw1, ARGS);

	 args.TopAttachment(XmATTACH_POSITION, 1);
	 args.BottomAttachment(XmATTACH_POSITION, 2);
	 XtSetValues(tw2, ARGS);

	 args.BottomAttachment(XmATTACH_NONE);
	 XtSetValues(lw2, ARGS);
	 XtSetValues(bw2, ARGS);

	 args.TopAttachment(XmATTACH_POSITION, 2);
	 args.BottomAttachment(XmATTACH_POSITION, 3);
	 XtSetValues(tw3, ARGS);

	 args.BottomAttachment(XmATTACH_NONE);
	 XtSetValues(lw3, ARGS);
	 XtSetValues(bw3, ARGS);

	 args.TopAttachment(XmATTACH_POSITION, 3);
	 args.BottomAttachment(XmATTACH_POSITION, 4);
	 XtSetValues(tw4, ARGS);

	 args.BottomAttachment(XmATTACH_NONE);
	 XtSetValues(lw4, ARGS);
	 XtSetValues(bw4, ARGS);

	 args.TopAttachment(XmATTACH_POSITION, 4);
	 args.BottomAttachment(XmATTACH_POSITION, 5);
	 XtSetValues(tw5, ARGS);

	 args.BottomAttachment(XmATTACH_NONE);
	 XtSetValues(lw5, ARGS);
	 XtSetValues(bw5, ARGS);

	 args.TopAttachment(XmATTACH_POSITION, 5);
	 args.BottomAttachment(XmATTACH_FORM);
	 XtSetValues(tw6, ARGS);

	 args.BottomAttachment(XmATTACH_NONE);
	 XtSetValues(lw6, ARGS);
	 XtSetValues(bw6, ARGS);
      }
#else
      args.Reset();
      for (i=0; i<visCount; i++) {

	 if ( i == 0	      ) args.TopAttachment(XmATTACH_FORM);
	 else			args.TopAttachment(XmATTACH_POSITION, i);
	 if ( i == visCount-1 ) args.BottomAttachment(XmATTACH_FORM);
	 else			args.BottomAttachment(XmATTACH_POSITION, i+1);
	 XtSetValues(*visTexList[i], ARGS);

	 args.BottomAttachment(XmATTACH_NONE);
	 XtSetValues(*visLabList[i], ARGS);
	 XtSetValues(*visButList[i], ARGS);
      }
#endif

//
// Manage the visible children
//
      if ( visLabList.size() > 0 ) {
	 XtManageChildren(visLabList.start(), visLabList.size());
	 XtManageChildren(visTexList.start(), visTexList.size());
	 if ( visButList.size() > 0 )
	    XtManageChildren(visButList.start(), visButList.size());
      }

   } // End if we can only have one header field per line

//
// If we can have more than one header per line, lay out the fields
//
   else {

//
// Build lists of the visible and hidden fields
//
      WidgetListC	visList, hidList;
      visList.add(pane->toForm);
      visList.add(pane->subForm);

      if ( pub->ccVis    ) visList.add(pane->ccForm);
      else	           hidList.add(pane->ccForm);
      if ( pub->bccVis   ) visList.add(pane->bccForm);
      else	           hidList.add(pane->bccForm);
      if ( pub->fccVis   ) visList.add(pane->fccForm);
      else	           hidList.add(pane->fccForm);
      if ( pub->otherVis ) visList.add(pane->otherForm);
      else	           hidList.add(pane->otherForm);

      visCount = visList.size();
      int	i;
      for (i=0; !changed && i<visCount; i++) {
	 Widget	w = *visList[i];
	 if ( !XtIsManaged(w) ) changed = True;
      }

      u_int	hidCount = hidList.size();
      for (i=0; !changed && i<hidCount; i++) {
	 Widget	w = *hidList[i];
	 if ( XtIsManaged(w) ) changed = True;
      }

      if ( !changed ) return;

//
// Unmanage all children
//
      if ( hidList.size() > 0 )
	 XtUnmanageChildren(hidList.start(), hidList.size());
      if ( visList.size() > 0 )
	 XtUnmanageChildren(visList.start(), visList.size());

      switch (visCount) {
         case 2:  Place2HeaderFields(visList); break;
	 case 3:  Place3HeaderFields(visList); break;
	 case 4:  Place4HeaderFields(visList); break;
	 case 5:  Place5HeaderFields(visList); break;
	 default: Place6HeaderFields(visList); break;
      }

//
// Manage the visible children
//
      XtManageChildren(visList.start(), visList.size());

   } // End if we can have more that one header field per line

//
// Restore the width of the form
//
   if ( paneWd > 0 )
      XtVaSetValues(pane->headForm, XmNwidth, paneWd, NULL);

   XtManageChild(pane->headForm);

//
// Update the number of rows
//
   if ( rowsChanged ) {
      int	rows = ishApp->compPrefs->headRows;
      pane->toText->SetSize (rows, pane->toText->ColumnCount());
      pane->subText->SetSize(rows, pane->subText->ColumnCount());
      if ( pub->ccVis )
	 pane->ccText->SetSize(rows, pane->ccText->ColumnCount());
      if ( pub->bccVis )
	 pane->bccText->SetSize(rows, pane->bccText->ColumnCount());
      if ( pub->fccVis )
	 pane->fccText->SetSize(rows, pane->fccText->ColumnCount());
      if ( pub->otherVis )
	 pane->otherText->SetSize(rows, pane->otherText->ColumnCount());
   }

//
// Copy text if we've switched panes
//
   if ( curHeadPane && pane != curHeadPane ) {

      StringC	tmpStr;
      HeaderPaneT	*c = curHeadPane;

      c->toText->GetString(tmpStr);
      pane->toText->SetString(tmpStr);

      tmpStr.Clear();
      c->subText->GetString(tmpStr);
      pane->subText->SetString(tmpStr);

      if ( pub->ccVis ) {
	 tmpStr.Clear();
	 c->ccText->GetString(tmpStr);
	 pane->ccText->SetString(tmpStr);
      }

      if ( pub->bccVis ) {
	 tmpStr.Clear();
	 c->bccText->GetString(tmpStr);
	 pane->bccText->SetString(tmpStr);
      }

      if ( pub->fccVis ) {
	 tmpStr.Clear();
	 c->fccText->GetString(tmpStr);
	 pane->fccText->SetString(tmpStr);
      }

      if ( pub->otherVis ) {
	 tmpStr.Clear();
	 c->otherText->GetString(tmpStr);
	 pane->otherText->SetString(tmpStr);
      }

   } // End if text needs to be copied

   curHeadPane = pane;
   toText      = pane->toText;
   subText     = pane->subText;
   ccText      = pane->ccText;
   bccText     = pane->bccText;
   fccText     = pane->fccText;
   otherText   = pane->otherText;

} // End PlaceHeaderFields

/*---------------------------------------------------------------
 *  Routine to position 2 header fields
 */

void
SendWinP::Place2HeaderFields(WidgetListC& visList)
{
//
// If two fields are visible, make:
//    1 row  of 2 or
//
   Widget	w1 = *visList[0];
   Widget	w2 = *visList[1];
   WArgList	args;

   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_POSITION, 49);
   args.BottomAttachment(XmATTACH_FORM);
   XtSetValues(w1, ARGS);

   args.LeftAttachment(XmATTACH_POSITION, 51);
   args.RightAttachment(XmATTACH_FORM);
   XtSetValues(w2, ARGS);

} // End Place2HeaderFields

/*---------------------------------------------------------------
 *  Routine to position 3 header fields
 */

void
SendWinP::Place3HeaderFields(WidgetListC& visList)
{
//
// If three fields are visible, make:
//    1 row  of 3 or
//    1 row  of 2 and 1 row of 1
//
   Widget	w1 = *visList[0];
   Widget	w2 = *visList[1];
   Widget	w3 = *visList[2];
   WArgList	args;

   if ( pub->maxFieldsPerLine == 2 ) {

      args.TopAttachment(XmATTACH_FORM);
      args.LeftAttachment(XmATTACH_FORM);
      args.RightAttachment(XmATTACH_POSITION, 49);
      args.BottomAttachment(XmATTACH_POSITION, 49);
      XtSetValues(w1, ARGS);

      args.LeftAttachment(XmATTACH_POSITION, 51);
      args.RightAttachment(XmATTACH_FORM);
      XtSetValues(w2, ARGS);

      args.TopAttachment(XmATTACH_POSITION, 51);
      args.LeftAttachment(XmATTACH_FORM);
      args.RightAttachment(XmATTACH_FORM);
      args.BottomAttachment(XmATTACH_FORM);
      XtSetValues(w3, ARGS);
   }

   else {
      args.TopAttachment(XmATTACH_FORM);
      args.LeftAttachment(XmATTACH_FORM);
      args.RightAttachment(XmATTACH_POSITION, 33);
      args.BottomAttachment(XmATTACH_FORM);
      XtSetValues(w1, ARGS);

      args.LeftAttachment(XmATTACH_POSITION, 34);
      args.RightAttachment(XmATTACH_POSITION, 66);
      XtSetValues(w2, ARGS);

      args.LeftAttachment(XmATTACH_POSITION, 67);
      args.RightAttachment(XmATTACH_FORM);
      XtSetValues(w3, ARGS);
   }

} // End Place3HeaderFields

/*---------------------------------------------------------------
 *  Routine to position the header fields
 */

void
SendWinP::Place4HeaderFields(WidgetListC& visList)
{
//
// If four fields are visible, make:
//    1 row  of 4 or
//    2 rows of 2 or
//
   Widget	w1 = *visList[0];
   Widget	w2 = *visList[1];
   Widget	w3 = *visList[2];
   Widget	w4 = *visList[3];
   WArgList	args;

   if ( pub->maxFieldsPerLine < 4 ) {

      args.TopAttachment(XmATTACH_FORM);
      args.LeftAttachment(XmATTACH_FORM);
      args.RightAttachment(XmATTACH_POSITION, 49);
      args.BottomAttachment(XmATTACH_POSITION, 49);
      XtSetValues(w1, ARGS);

      args.LeftAttachment(XmATTACH_POSITION, 51);
      args.RightAttachment(XmATTACH_FORM);
      XtSetValues(w2, ARGS);

      args.TopAttachment(XmATTACH_POSITION, 51);
      args.LeftAttachment(XmATTACH_FORM);
      args.RightAttachment(XmATTACH_POSITION, 49);
      args.BottomAttachment(XmATTACH_FORM);
      XtSetValues(w3, ARGS);

      args.LeftAttachment(XmATTACH_POSITION, 51);
      args.RightAttachment(XmATTACH_FORM);
      XtSetValues(w4, ARGS);
   }

   else {

      args.TopAttachment(XmATTACH_FORM);
      args.LeftAttachment(XmATTACH_FORM);
      args.RightAttachment(XmATTACH_POSITION, 24);
      args.BottomAttachment(XmATTACH_FORM);
      XtSetValues(w1, ARGS);

      args.LeftAttachment(XmATTACH_POSITION, 26);
      args.RightAttachment(XmATTACH_POSITION, 49);
      XtSetValues(w2, ARGS);

      args.LeftAttachment(XmATTACH_POSITION, 51);
      args.RightAttachment(XmATTACH_POSITION, 74);
      XtSetValues(w3, ARGS);

      args.LeftAttachment(XmATTACH_POSITION, 76);
      args.RightAttachment(XmATTACH_FORM);
      XtSetValues(w4, ARGS);
   }

} // End Place4HeaderFields

/*---------------------------------------------------------------
 *  Routine to position the header fields
 */

void
SendWinP::Place5HeaderFields(WidgetListC& visList)
{
//
// If five fields are visible, make:
//    1 row  of 5 or
//    1 row  of 3 and 1 row of 2 or
//    2 rows of 2 and 1 row of 1 or
//
   Widget	w1 = *visList[0];
   Widget	w2 = *visList[1];
   Widget	w3 = *visList[2];
   Widget	w4 = *visList[3];
   Widget	w5 = *visList[4];
   WArgList	args;

   if ( pub->maxFieldsPerLine == 2 ) {

      args.TopAttachment(XmATTACH_FORM);
      args.LeftAttachment(XmATTACH_FORM);
      args.RightAttachment(XmATTACH_POSITION, 49);
      args.BottomAttachment(XmATTACH_POSITION, 33);
      XtSetValues(w1, ARGS);

      args.LeftAttachment(XmATTACH_POSITION, 51);
      args.RightAttachment(XmATTACH_FORM);
      XtSetValues(w2, ARGS);

      args.TopAttachment(XmATTACH_POSITION, 34);
      args.LeftAttachment(XmATTACH_FORM);
      args.RightAttachment(XmATTACH_POSITION, 49);
      args.BottomAttachment(XmATTACH_POSITION, 66);
      XtSetValues(w3, ARGS);

      args.LeftAttachment(XmATTACH_POSITION, 51);
      args.RightAttachment(XmATTACH_FORM);
      XtSetValues(w4, ARGS);

      args.TopAttachment(XmATTACH_POSITION, 67);
      args.LeftAttachment(XmATTACH_FORM);
      args.RightAttachment(XmATTACH_FORM);
      args.BottomAttachment(XmATTACH_FORM);
      XtSetValues(w5, ARGS);
   }

   else if ( pub->maxFieldsPerLine < 5 ) {

      args.TopAttachment(XmATTACH_FORM);
      args.LeftAttachment(XmATTACH_FORM);
      args.RightAttachment(XmATTACH_POSITION, 33);
      args.BottomAttachment(XmATTACH_POSITION, 49);
      XtSetValues(w1, ARGS);

      args.LeftAttachment(XmATTACH_POSITION, 34);
      args.RightAttachment(XmATTACH_POSITION, 66);
      XtSetValues(w2, ARGS);

      args.LeftAttachment(XmATTACH_POSITION, 67);
      args.RightAttachment(XmATTACH_FORM);
      XtSetValues(w3, ARGS);

      args.TopAttachment(XmATTACH_POSITION, 51);
      args.LeftAttachment(XmATTACH_FORM);
      args.RightAttachment(XmATTACH_POSITION, 49);
      args.BottomAttachment(XmATTACH_FORM);
      XtSetValues(w4, ARGS);

      args.LeftAttachment(XmATTACH_POSITION, 51);
      args.RightAttachment(XmATTACH_FORM);
      XtSetValues(w5, ARGS);
   }

   else {

      args.TopAttachment(XmATTACH_FORM);
      args.LeftAttachment(XmATTACH_FORM);
      args.RightAttachment(XmATTACH_POSITION, 20);
      args.BottomAttachment(XmATTACH_FORM);
      XtSetValues(w1, ARGS);

      args.LeftAttachment(XmATTACH_POSITION, 21);
      args.RightAttachment(XmATTACH_POSITION, 40);
      XtSetValues(w2, ARGS);

      args.LeftAttachment(XmATTACH_POSITION, 41);
      args.RightAttachment(XmATTACH_POSITION, 60);
      XtSetValues(w3, ARGS);

      args.LeftAttachment(XmATTACH_POSITION, 61);
      args.RightAttachment(XmATTACH_POSITION, 80);
      XtSetValues(w4, ARGS);

      args.LeftAttachment(XmATTACH_POSITION, 81);
      args.RightAttachment(XmATTACH_FORM);
      XtSetValues(w5, ARGS);
   }

} // End Place5HeaderFields

/*---------------------------------------------------------------
 *  Routine to position the header fields
 */

void
SendWinP::Place6HeaderFields(WidgetListC& visList)
{
//
// If six fields are visible, make:
//    2 rows of 3 or
//    3 rows of 2
//
   Widget	w1 = *visList[0];
   Widget	w2 = *visList[1];
   Widget	w3 = *visList[2];
   Widget	w4 = *visList[3];
   Widget	w5 = *visList[4];
   Widget	w6 = *visList[5];
   WArgList	args;

   if ( pub->maxFieldsPerLine == 2 ) {	// 3 rows of 2

      args.TopAttachment(XmATTACH_FORM);
      args.LeftAttachment(XmATTACH_FORM);
      args.RightAttachment(XmATTACH_POSITION, 49);
      args.BottomAttachment(XmATTACH_POSITION, 33);
      XtSetValues(w1, ARGS);

      args.LeftAttachment(XmATTACH_POSITION, 51);
      args.RightAttachment(XmATTACH_FORM);
      XtSetValues(w2, ARGS);

      args.TopAttachment(XmATTACH_POSITION, 34);
      args.LeftAttachment(XmATTACH_FORM);
      args.RightAttachment(XmATTACH_POSITION, 49);
      args.BottomAttachment(XmATTACH_POSITION, 66);
      XtSetValues(w3, ARGS);

      args.LeftAttachment(XmATTACH_POSITION, 51);
      args.RightAttachment(XmATTACH_FORM);
      XtSetValues(w4, ARGS);

      args.TopAttachment(XmATTACH_POSITION, 67);
      args.LeftAttachment(XmATTACH_FORM);
      args.RightAttachment(XmATTACH_POSITION, 49);
      args.BottomAttachment(XmATTACH_FORM);
      XtSetValues(w5, ARGS);

      args.LeftAttachment(XmATTACH_POSITION, 51);
      args.RightAttachment(XmATTACH_FORM);
      XtSetValues(w6, ARGS);
   }

   else {	// 2 rows of 3

      args.TopAttachment(XmATTACH_FORM);
      args.LeftAttachment(XmATTACH_FORM);
      args.RightAttachment(XmATTACH_POSITION, 33);
      args.BottomAttachment(XmATTACH_POSITION, 49);
      XtSetValues(w1, ARGS);

      args.LeftAttachment(XmATTACH_POSITION, 34);
      args.RightAttachment(XmATTACH_POSITION, 66);
      XtSetValues(w2, ARGS);

      args.LeftAttachment(XmATTACH_POSITION, 67);
      args.RightAttachment(XmATTACH_FORM);
      XtSetValues(w3, ARGS);

      args.TopAttachment(XmATTACH_POSITION, 51);
      args.LeftAttachment(XmATTACH_FORM);
      args.RightAttachment(XmATTACH_POSITION, 33);
      args.BottomAttachment(XmATTACH_FORM);
      XtSetValues(w4, ARGS);

      args.LeftAttachment(XmATTACH_POSITION, 34);
      args.RightAttachment(XmATTACH_POSITION, 66);
      XtSetValues(w5, ARGS);

      args.LeftAttachment(XmATTACH_POSITION, 67);
      args.RightAttachment(XmATTACH_FORM);
      XtSetValues(w6, ARGS);
   }

} // End Place6HeaderFields

/*---------------------------------------------------------------
 *  Method to popup menu for attachments
 */

void
SendWinP::BuildMimePopupMenu()
{
   Widget	wlist[16];
   WArgList	args;

//
// Create mimePU hierarchy
//
// mimePU
//    PushButton        mimePUEditPB
//    PushButton        mimePUContPB
//    PushButton        mimePUTypeCB
//    PushButton        mimePUDelPB
//    Separator         mimePUSep
//    Label             mimePULabel
//
   mimePU = XmCreatePopupMenu(bodyText->TextArea(), "mimePU", 0,0);

   mimePUEditPB        = XmCreatePushButton   (mimePU, "mimePUEditPB", 0,0);
   mimePUTypeCB        = XmCreateCascadeButton(mimePU, "mimePUTypeCB", 0,0);
   mimePUContPB        = XmCreatePushButton   (mimePU, "mimePUContPB", 0,0);
   Widget mimePUDelPB  = XmCreatePushButton   (mimePU, "mimePUDelPB",  0,0);
   Widget mimePUSep    = XmCreateSeparator    (mimePU, "mimePUSep",    0,0);
   mimePULabel         = XmCreateLabel        (mimePU, "mimePULabel",  0,0);

   wlist[0] = mimePUEditPB;
   wlist[1] = mimePUTypeCB;
   wlist[2] = mimePUContPB;
   wlist[3] = mimePUDelPB;
   wlist[4] = mimePUSep;
   wlist[5] = mimePULabel;
   XtManageChildren(wlist, 6);

   AddActivate(mimePUEditPB, DoEditPart,     this);
   AddActivate(mimePUContPB, DoEditContents, this);
   AddActivate(mimePUDelPB,  DoDeletePart,   this);

//
// Create mimePUTypePD hierarchy
//
// mimePUTypePD
//    PushButton	mimePUTypeMixPB
//    PushButton	mimePUTypeDigPB
//    PushButton	mimePUTypeAltPB
//    PushButton	mimePUTypeParPB
//
   Widget mimePUTypePD = XmCreatePulldownMenu (mimePU, "mimePUTypePD", 0,0);
   XtVaSetValues(mimePUTypeCB, XmNsubMenuId, mimePUTypePD, NULL);

   Widget mimePUTypeMixPB = XmCreatePushButton(mimePUTypePD, "mimePUTypeMixPB",
					       0,0);
   Widget mimePUTypeDigPB = XmCreatePushButton(mimePUTypePD, "mimePUTypeDigPB",
					       0,0);
   Widget mimePUTypeAltPB = XmCreatePushButton(mimePUTypePD, "mimePUTypeAltPB",
					       0,0);
   Widget mimePUTypeParPB = XmCreatePushButton(mimePUTypePD, "mimePUTypeParPB",
					       0,0);

   wlist[0] = mimePUTypeMixPB;
   wlist[1] = mimePUTypeDigPB;
   wlist[2] = mimePUTypeAltPB;
   wlist[3] = mimePUTypeParPB;
   XtManageChildren(wlist, 4);	// mimePUTypePD children

   AddActivate(mimePUTypeMixPB, DoModMixed,    this);
   AddActivate(mimePUTypeDigPB, DoModDigest,   this);
   AddActivate(mimePUTypeAltPB, DoModAlt,      this);
   AddActivate(mimePUTypeParPB, DoModParallel, this);

#if 0
   // FIXME: Problems using this with Linux Motif 2.0
   XmRemoveFromPostFromList(mimePU, bodyText->TextArea());
#endif

} // End BuildMimePopupMenu

/*-----------------------------------------------------------------------
 *  Handle initial display
 */

void
SendWinP::DoPopup(Widget, SendWinP *This, XtPointer)
{
//
// Remove this callback
//
   XtRemoveCallback(*This->pub, XmNpopupCallback, (XtCallbackProc)DoPopup,
		    (XtPointer)This);

} // End DoPopup

/*---------------------------------------------------------------
 *  Routine to initialize the contents of the Fcc field
 */

void
SendWinP::UpdateFcc()
{
   fccStr.Clear();
   fccText->Clear();

   if ( ishApp->mailPrefs->fccType == FCC_TO_FOLDER ) {
      fccStr = ishApp->mailPrefs->FccFolder();
      ishApp->AbbreviateFolderName(fccStr);
      fccText->SetString(fccStr);
   }

   else if ( ishApp->mailPrefs->fccType == FCC_BY_USER ||
      	     ishApp->mailPrefs->fccType == FCC_BY_ADDRESS ) {

      StringC		folderDir = ishApp->mailPrefs->FccFolderDir();
      ishApp->AbbreviateFolderName(folderDir);

//
// Get the addresses from the to field
//
      StringC	toStr;
      toText->GetString(toStr, TT_PLAIN);
      toStr.Trim();
      if ( toStr.size() == 0 ) return;

//
// Loop through the addresses and extract the appropriate parts
//
      AddressC	addrs(toStr);

      AddressC	*addr = &addrs;
      while ( addr ) {

	 if ( fccStr.size() > 0 ) fccStr += ", ";
	 fccStr += folderDir;
	 if ( folderDir != "+" ) fccStr += '/';

//
// Add the appropriate part of the address
//
	 if ( ishApp->mailPrefs->fccType == FCC_BY_USER )
	    fccStr += addr->mailbox;
	 else
	    fccStr += addr->addr;

	 addr = addr->next;

      } // End for each address

      fccText->SetString(fccStr);

   } // End if saving by user or address

   else if ( ishApp->mailPrefs->fccType != FCC_NONE ) {

//
// Get the current time
//
      char	buf[16];
      int	buflen = 16;
      time_t	now    = time(0);
      struct tm	*local = localtime(&now);

//
// Convert the time to a string based on the fcc type
//
      char	*fmt;
      if      ( ishApp->mailPrefs->fccType == FCC_BY_MONTH )
	 fmt = "month.%Y.%m";
      else if ( ishApp->mailPrefs->fccType == FCC_BY_WEEK  )
	 fmt = "week.%Y.%W";
      else if ( ishApp->mailPrefs->fccType == FCC_BY_DAY   )
	 fmt = "day.%Y.%m.%d";
      else
	 fmt = "year.%Y";

      if ( strftime(buf, buflen, fmt, local) ) {

	 fccStr = ishApp->mailPrefs->FccFolderDir();
	 ishApp->AbbreviateFolderName(fccStr);
	 fccStr += '/';
	 fccStr += buf;
	 fccText->SetString(fccStr);
      }

   } // End if saving by date/time

} // End UpdateFcc

/*---------------------------------------------------------------
 *  Callback to handle change of markup state in text body
 */

void
SendWinP::BodyStateChanged(MimeRichTextC *rt, SendWinP *This)
{
   StringC      str = rt->CursorStateString();
   XmTextFieldSetString(This->bodyStateTF, str);
}

/*---------------------------------------------------------------
 *  Callback to handle header text modification.  We're just setting the
 *     changed flag.
 */

void
SendWinP::TextChanged(MimeRichTextC *rt, SendWinP *This)
{
   This->pub->changed = True;

//
// See if commas need to be inserted automatically
//
   if ( ishApp->compPrefs->spaceEndsAddr &&
        (rt == This->toText  || rt == This->ccText ||
	 rt == This->bccText || rt == This->fccText) ) {

      StringC	str;
      rt->GetString(str, TT_PLAIN);
      if ( str.EndsWith(' ') || str.EndsWith('\t') ) {

	 Boolean	needComma = False;
	 const char	*cs = str.RevSearch(',');
	 if ( cs ) {
	    cs++;
	    while ( isspace(*cs) ) cs++;
	    if ( *cs != 0 ) needComma = True;
	 }
	 else
	    needComma = True;

	 if ( needComma ) {

	    int	pos = str.size() - 1;
	    while ( pos >= 0 && isspace(str[pos]) ) pos--;
	    str(pos+1,0) = ",";

	    rt->SetString(str);
	    rt->MoveCursor(rt->TextArea(), 9999, 9999); // End of text
	 }
      }
   }

   if ( rt == This->toText )
      This->UpdateFcc();
   else if ( rt == This->bodyText ) {
      XtSetSensitive(This->editUndeletePB, This->bodyText->UndeleteOk());
      This->buttMgr->SensitivityChanged(This->editUndeletePB);
   }

//
// See if an auto-save is needed
//
   This->keystrokeCount++;

} // End TextChanged

/*---------------------------------------------------------------
 *  Timer proc to reset vertical scroll bar to its previous position
 */

void
SendWinP::CheckAutoSave(SendWinP *This, XtIntervalId*)
{
   This->autoSaveTimer = (XtIntervalId)NULL;

   if ( !This->pub->IsShown() || !ishApp->compPrefs->autoSave ||
				  ishApp->compPrefs->autoSaveRate <= 0 )
      return;

//
// See if a save is necessary
//
   if ( This->keystrokeCount > 0 && !This->edit_pid && !This->spell_pid ) {

//
// Remove any previous file
//
      if ( This->autoSaveFile.size() > 0 ) unlink(This->autoSaveFile);

//
// Make sure the auto save file name is still valid
//
      if ( !This->autoSaveFile.StartsWith(ishApp->compPrefs->AutoSaveDir()) ) {
	 char	*cs = tempnam(ishApp->compPrefs->AutoSaveDir(), "comp.");
	 This->autoSaveFile = cs;
	 free(cs);
      }

//
// Just in case
//
      MakeDir(ishApp->compPrefs->AutoSaveDir());

//
// Save the message
//
      if ( This->Save(This->autoSaveFile) )
	 This->pub->ClearMessage();

      This->keystrokeCount = 0;

   } // End if auto-save is needed

//
// Start another timer
//
   XtAppAddTimeOut(ishApp->context, ishApp->compPrefs->autoSaveRate*1000/*ms*/,
		   (XtTimerCallbackProc)CheckAutoSave, (XtPointer)This);

} // End CheckAutoSave

/*---------------------------------------------------------------
 *  Callbacks to handle alias expansion request
 */

void
SendWinP::HandleExpandAliases(Widget w, XKeyEvent*, String*, Cardinal*)
{
   MimeRichTextC	*rt;
   XtVaGetValues(w, XmNuserData, &rt, NULL);

   StringC	addrStr;
   rt->GetString(addrStr, TT_PLAIN);

   StringC	expStr = ishApp->aliasPrefs->ExpandAddress(addrStr, 1);
   if ( expStr != addrStr ) rt->SetString(expStr);
}

void
SendWinP::HandleExpandSelection(Widget w, XKeyEvent*, String*, Cardinal*)
{
#if 0
   char	*cs = XmTextFieldGetSelection(w);
   if ( !cs ) return;

   StringC	addrStr(cs);
   XtFree(cs);

   SendWinC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);

   StringC	expStr = ishApp->aliasPrefs->ExpandAddress(addrStr, 1);
   if ( expStr != addrStr ) {
      XmTextPosition	left, right;
      This->priv->selfInducedMod = True;
      XmTextFieldGetSelectionPosition(w, &left, &right);
      XmTextFieldReplace(w, left, right, expStr);
      This->priv->selfInducedMod = False;
   }
#endif
}

/*-----------------------------------------------------------------------
 *  Handle drag over event
 */

void
SendWinP::HandleDragOver(Widget w, XtPointer, XmDragProcCallbackStruct *dp)
{
   if ( dp->reason == XmCR_DROP_SITE_MOTION_MESSAGE ) {

      dp->dropSiteStatus = XmVALID_DROP_SITE;

#if 0
//
// Get the window pointer
//
      TextPartC	*part;
      XtVaGetValues(w, XmNuserData, &part, NULL);
      SendWinC	*This = part->sendWin;
      if ( debuglev > 1 ) cout <<"Got drag-over in SendWinC: " <<This <<endl;


//
// See what the cursor is over
//
      Position	rootX, rootY;
      XtTranslateCoords(w, dp->x, dp->y, &rootX, &rootY);

      BodyPartC *overPart = This->priv->rootContPart->PartContaining(rootX, rootY);
      if ( overPart != This->priv->dragOverPart ) {

	 if ( debuglev > 1 ) {
	    cout <<"drag-over-part is " <<This->priv->dragOverPart <<endl;
	    cout <<"     over-part is " <<overPart <<endl;
	 }

	 if ( This->priv->dragOverPart )
	    This->priv->dragOverPart->HighlightDrop(False);

	 if ( overPart ) 
	    overPart->HighlightDrop(True);

	 This->priv->dragOverPart = overPart;
      }
#endif

   } // End if drop site motion

   dp->animate = True;

} // End HandleDragOver

/*-----------------------------------------------------------------------
 *  Handle drop event
 */

void
SendWinP::HandleDrop(Widget w, XtPointer, XmDropProcCallbackStruct *dp)
{
   if ( debuglev > 0 )
      cout <<"Got drop with status " <<dp->dropSiteStatus <<endl;

//
// See if this is a valid drop
//
   unsigned char	status = XmTRANSFER_FAILURE;
   if ( dp->dropSiteStatus == XmVALID_DROP_SITE ) {

//
// Get the window pointer
//
      SendWinP	*This = NULL;
      u_int	count = winList->size();
      for (int i=0; !This && i<count; i++) {
	 SendWinP	*win = (SendWinP *)*(*winList)[i];
	 if ( w == win->bodyText->TextArea() )
	    This = win;
      }

      if ( This ) {

#if 0
	 if ( This->dragOverPart )
	    This->dragOverPart->HighlightDrop(False);
#endif

//
// See what was dropped
//
	 Atom		*expAtoms;
	 Cardinal	atomCount;
	 XtVaGetValues(dp->dragContext, XmNexportTargets,    &expAtoms,
					XmNnumExportTargets, &atomCount, NULL);
	 if ( atomCount > 0 ) {

	    if ( expAtoms[0] == graphicAtom ) {
	       if ( debuglev > 0 ) cout <<"Got drop of graphic" <<endl;
	       This->HandleGraphicDrop(w, dp);
	       status = XmTRANSFER_SUCCESS;
	    }

	    else if ( expAtoms[0] == ishApp->msgAtom ) {
	       if ( debuglev > 0 ) cout <<"Got drop of message" <<endl;
	       This->HandleMessageDrop(w, dp);
	       status = XmTRANSFER_SUCCESS;
	    }

	    else if ( debuglev > 0 ) cout <<"Got drop of unknown" <<endl;

	 } // End if there are dropped atoms

      } // End if SendWinC could be found

   } // End if this is a valid drop site

//
// Drop must be acknowledged
//
   WArgList	args;
   args.TransferStatus(status);
   XmDropTransferStart(dp->dragContext, ARGS);

} // End HandleDrop

/*-----------------------------------------------------------------------
 *  Handle message drop event
 */

void
SendWinP::HandleMessageDrop(Widget w, XmDropProcCallbackStruct *dp)
{
//
// Get the drag data
//
   ViewDragDataT	*dd;
   XtVaGetValues(dp->dragContext, XmNclientData, &dd, NULL);

//
// Move the insertion cursor to the position of the drop
//
   bodyText->MoveCursor(w, dp->x, dp->y);

//
// See if this is one part or several
//
   if ( dd->itemList.size() > 1 ) {

//
// Ask about using a digest?
//

//
// Create a graphic for each message
//
      unsigned	count = dd->itemList.size();
      for (int i=0; i<count; i++) {

	 if ( i > 0 ) bodyText->InsertString("\n");

	 MsgItemC	*item = (MsgItemC*)(dd->itemList[i]);
	 MsgC		*msg = item->msg;

	 SendIconC	*icon = new SendIconC(pub, msg);
	 icon->AddDoubleClickCallback((CallbackFn*)OpenPart,     this);
	 icon->AddMenuCallback       ((CallbackFn*)PostPartMenu, this);
	 bodyText->InsertGraphic(icon);

      } // End for each dropped message

   } // End if more than one message was dropped

   else {

      MsgItemC	*item = (MsgItemC*)(dd->itemList[0]);
      MsgC	*msg = item->msg;

//
// Add a graphic for the message
//
      SendIconC	*icon = new SendIconC(pub, msg);
      icon->AddDoubleClickCallback((CallbackFn*)OpenPart,     this);
      icon->AddMenuCallback       ((CallbackFn*)PostPartMenu, this);
      bodyText->InsertGraphic(icon);

   } // End if a single message was dropped

//
// Delete drag data
//
   delete dd;

   return;

} // End HandleMessageDrop

/*-----------------------------------------------------------------------
 *  Handle body part drop event
 */

void
SendWinP::HandleGraphicDrop(Widget, XmDropProcCallbackStruct *dp)
{
#if 0
//
// Get the part being dragged
//
   BodyPartC	*dragPart;
   XtVaGetValues(dp->dragContext, XmNclientData, &dragPart, NULL);

//
// Translate the event location to root coords and see which part contains
//    that point
//
   Position	rootX, rootY;
   XtTranslateCoords(w, dp->x, dp->y, &rootX, &rootY);

   BodyPartC	*dropPart = rootContPart->PartContaining(rootX, rootY);
   if ( dropPart == dragPart || (dropPart && dropPart->next == dragPart) )
      return;

//
// Find the source container
//
   ContPartC	*dragContPart = rootContPart->ParentOf(dragPart);

//
// Find the destination container
//
   ContPartC	*dropContPart = NULL;
   if ( dropPart ) dropContPart = rootContPart->ParentOf(dropPart);
   if ( !dropContPart ) dropContPart = rootContPart;

//
// Unhook the part from the source container
//
   if ( dragContPart )
      dragContPart->Unlink(dragPart);

//
// If the destination container isn't the same as the source, we need to delete
//    the widgets so they'll get created again.
//
   if ( dropContPart != dragContPart )
      dragPart->DeleteWidgets();

//
// Add the drag part after the drop part.  If there is no drop part, add it as
//    the last child of the root container.

   if ( dropPart && dropPart != rootPart ) {
      dropPart->SetNext(dragPart);
   }

   else if ( rootContPart->child ) {
      MimePartC	*lastChild = rootContPart->child;
      while ( lastChild->next ) lastChild = lastChild->next;
      lastChild->SetNext(dragPart);
   }

   else {
      rootContPart->child = dragPart;
   }

//
// Create any new widgets
//
   rootContPart->CreateWidgets(altBodyWin);
   rootContPart->ManageWidgets();

   if ( debug1 ) {
      cout <<"Tree after HandleBodyPartDrop:" <<endl;
      rootContPart->PrintTree(1);
   }
#endif

   return;

} // End HandleGraphicDrop

/*-----------------------------------------------------------------------
 *  Method to load an address header field
 */

void
SendWinP::SetField(MimeRichTextC *field, AddressC *addr)
{
//
// Use a list so that duplicate addresses can be easily removed.
//
   StringC	text;
   StringListC	list;
   list.AllowDuplicates(FALSE);

   Boolean wrap = True;
   while ( addr ) {

      text.Clear();

//
// See if the name is to be displayed
//
      Boolean remove = (ishApp->replyPrefs->removeMe &&
      			IsMyAddress(addr->addr));
      if ( !remove ) {

//
// See if comments are to be displayed
//
	 HeaderValC	*cmt = addr->name;
	 if ( ishApp->replyPrefs->stripComments || !cmt )
	    text = addr->addr;

//
// See if there is an alternate character set.  If there is, this header was
//    RFC1522 encoded.
//
	 else if ( cmt->charset ) {

//
// If the alternate charset is compatible with the text field, display the
//    decoded value.  If the charset is not compatible, display the encoded
//    value.
//
	    if ( CharsetOk(cmt->charset, toText->Charset()) ) {
	       cmt->GetValueText(text);
	       text += " <";
	       text += addr->addr;
	       text += ">";
	    }
	    else
	       text = addr->full;

	 } // End if there is an alternate charset

	 else
	    text = addr->full;

//
// If the string contains whitespace, we won't wrap
//
	 text.Trim();
	 if ( text.size() > 0 ) {
	    list.add(text);
	    if ( text.Contains(' ') || text.Contains('\t') ) wrap = False;
	 }

      } // End if address is to be displayed

      addr = addr->next;

   } // End for each address

//
// If there are no comments, we'll wrap 
//
   u_int	count = list.size();
   if ( count == 0 ) wrap = False;
   wrap = wrap || ishApp->replyPrefs->stripComments;

//
// Build a single string from the list.  If we're wrapping or have just a
//    single row, add the entries with commas between.  Otherwise, add them
//    with newlines between
//
   text.Clear();
   int	rows = field->RowCount();
   for (int i=0; i<count; i++) {
      if ( text.size() > 0 ) {
	 if ( wrap || rows == 1 ) text += ", ";
	 else			  text += '\n';
      }
      text += *list[i];
   }

//
// Update the field
//
   field->Defer(True);
   field->Clear();
   field->ResizeWidth(!wrap);
   field->SetString(text);
   field->Defer(False);

//
// Update visibility of optional fields
//
   if ( field == ccText )
      XmToggleButtonSetState(optCcTB,
			     text.size()>0 || ishApp->compPrefs->showCc, True);
   else if ( field == bccText )
      XmToggleButtonSetState(optBccTB,
			     text.size()>0 || ishApp->compPrefs->showBcc, True);

} // End SetField

/*---------------------------------------------------------------
 *  Method to display the text for a non-encapsulated, forwarded message
 */

void
SendWinP::ForwardMsgInline(MsgC *msg)
{
   bodyText->Defer(True);
   bodyText->SetTextType(TT_PLAIN);

//
// Add begin forward string
//
   StringC	forStr = msg->BeginForward();
   bodyText->AddStringEnriched(forStr);
   if ( !forStr.EndsWith('\n') ) bodyText->AddStringPlain("\n");
   bodyText->AddStringPlain("\n");

//
// Add desired headers
//
   if ( !ishApp->replyPrefs->forwardNoHeaders ) {

      Boolean	all    = ishApp->replyPrefs->forwardAllHeaders;
      HeaderC	*head  = msg->Headers();
      Boolean	needNL = False;
      while ( head ) {

//
// See if this header should be displayed
//
	 if ( all || ishApp->headPrefs->HeaderShown(head->key) ) {

	    if ( needNL ) bodyText->AddString("\n");

	    HeaderValC	*val = head->value;

//
// See if this header has a complex value
//
	    if ( val->charset || val->next ) {

	       bodyText->AddString(head->key);
	       bodyText->AddString(": ");

//
// Loop through value components
//
	       while ( val ) {

		  if ( val->charset ) bodyText->AddCommand(val->charset);
		  bodyText->AddString(val->text);
		  if ( val->charset ) bodyText->AddCommand(val->charset,
							   True/*negate*/);

		  val = val->next;

	       } // End for each value component

	    } // End if value is complex

	    else
	       bodyText->AddString(head->full);

	    needNL = True;

	 } // End if header is displayed

	 head = head->next;

      } // End for each header

//
// Follow headers with a blank line
//
      if ( needNL ) bodyText->AddStringPlain("\n\n");

   } // End if headers are displayed

//
// Add body
//
   bodyText->CheckFroms(msg->HasSafeFroms());

   if ( msg->IsMime() ) {
      MsgPartC	*tree = msg->Body();
      AddBodyTree(tree);
      bodyText->AddStringPlain("\n\n");
   }
   
   else {

      StringC	body;
      msg->GetBodyText(body);
      bodyText->AddStringPlain(body);
      bodyText->CheckFroms(False);

//
// Make sure there's a blank line
//
      if      ( body.EndsWith("\n\n") ) ;
      else if ( body.EndsWith("\n")   ) bodyText->AddStringPlain("\n");
      else				bodyText->AddStringPlain("\n\n");

   } // End if message is not MIME

//
// Add end forward string
//
   forStr = msg->EndForward();
   bodyText->AddStringEnriched(forStr);

   bodyText->Defer(False);

} // End ForwardMsgInline

/*---------------------------------------------------------------
 *  Method to display the graphic for an encapsulated message
 */

void
SendWinP::ForwardMsgEncap(MsgC *msg)
{
   bodyText->Defer(True);

//
// Add begin forward string
//
   StringC	forStr = msg->BeginForward();
   bodyText->AddStringEnriched(forStr);
   if ( !forStr.EndsWith('\n') ) bodyText->AddStringPlain("\n");

//
// Add a graphic
//
   SendIconC	*icon = new SendIconC(pub, msg);
   icon->AddDoubleClickCallback((CallbackFn*)OpenPart,     this);
   icon->AddMenuCallback       ((CallbackFn*)PostPartMenu, this);
   bodyText->AddGraphic(icon);
   bodyText->AddStringPlain("\n");

//
// Add end forward string
//
   forStr = msg->EndForward();
   bodyText->AddStringEnriched(forStr);

   bodyText->Defer(False);

} // End ForwardMsgEncap

/*---------------------------------------------------------------
 *  Method to turn on/off widgets that aren't needed for resend
 */

void
SendWinP::SetResendMode(Boolean on)
{
//
// See if it's already set
//
   if ( on != XtIsSensitive(fileEditPB) ) return;

   XtSetSensitive(fileEditPB,	!on);
   XtSetSensitive(fileSpellPB,	!on);
   XtSetSensitive(fileIncCB,	!on);
   XtSetSensitive(fileAddCB,	!on);
   XtSetSensitive(fileSavePB,	!on);
   XtSetSensitive(fileLoadPB,	!on);
   XtSetSensitive(fileClearPB,	!on);
   XtSetSensitive(editCB,	!on);
   XtSetSensitive(optMimeTB,	!on);
   XtSetSensitive(optWrapTB,	!on);
   XtSetSensitive(optAddSigTB,	!on);
   XtSetSensitive(optDigSignTB,	!on);
   XtSetSensitive(optEncryptTB,	!on);
   XtSetSensitive(optMsgTypeCB,	!on);
   XtSetSensitive(optTextTypeCB,!on);

   subText->SetEditable(!on);

   if ( on ) XtUnmanageChild(bodyForm);
   else	     XtManageChild(bodyForm);

   buttMgr->EnableButtons();

   StringC	title;
   if ( on ) title = get_string(*pub, "resendTitle", "Resend Window");
   else	     title = get_string(*pub, XmNtitle, "Composition Window");
   XtVaSetValues(*pub, XmNtitle, (char*)title, XmNiconName, (char*)title, NULL);

} // End SetResendMode

/*---------------------------------------------------------------
 *  Method to ask user if changed message is ok to close
 */

Boolean
SendWinP::OkToClose()
{
   if ( !ishApp->confPrefs->confirmCloseSend || !halApp->xRunning ||
      	!pub->Changed() )
      return True;

   static QueryAnswerT	answer;
   static Widget	dialog = NULL;

//
// Create the dialog if necessary
//
   if ( !dialog ) {

      pub->BusyCursor(True);

      WArgList	args;
      args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
      if ( halApp->questionPM ) args.SymbolPixmap(halApp->questionPM);
      dialog = XmCreateQuestionDialog(*pub, "confirmCloseWin", ARGS);

      XtAddCallback(dialog, XmNokCallback, (XtCallbackProc)AnswerQuery,
		    (XtPointer)&answer);
      XtAddCallback(dialog, XmNcancelCallback,
		    (XtCallbackProc)AnswerQuery, (XtPointer)&answer);
      XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));

//
// Trap window manager close function
//
      XmAddWMProtocolCallback(XtParent(dialog), halApp->delWinAtom,
			      (XtCallbackProc)WmClose,
			      (caddr_t)&answer);

      pub->BusyCursor(False);

   } // End if query dialog not created

//
// Show this window and the dialog
//
   pub->HalTopLevelC::Show();
   PopupOver(dialog, *pub);

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

   return (answer == QUERY_YES);

} // End OkToClose

/*---------------------------------------------------------------
 *  Callbacks to select aliases from a list
 */

void
SendWinP::PickToAlias(Widget, SendWinP *This, XtPointer)
{
   if ( !This->pickAliasWin ) {
      This->pub->BusyCursor(True);
      This->pickAliasWin = new PickAliasWinC(*This->pub);
      This->pub->BusyCursor(False);
   }

   This->pickAliasWin->SetApplyCallback((CallbackFn*)AddToAlias, This);
   This->pickAliasWin->Show("To");

} // End PickToAlias

void
SendWinP::PickCcAlias(Widget, SendWinP *This, XtPointer)
{
   if ( !This->pickAliasWin ) {
      This->pub->BusyCursor(True);
      This->pickAliasWin = new PickAliasWinC(*This->pub);
      This->pub->BusyCursor(False);
   }

   This->pickAliasWin->SetApplyCallback((CallbackFn*)AddCcAlias, This);
   This->pickAliasWin->Show("Cc");

} // End PickCcAlias

void
SendWinP::PickBccAlias(Widget, SendWinP *This, XtPointer)
{
   if ( !This->pickAliasWin ) {
      This->pub->BusyCursor(True);
      This->pickAliasWin = new PickAliasWinC(*This->pub);
      This->pub->BusyCursor(False);
   }

   This->pickAliasWin->SetApplyCallback((CallbackFn*)AddBccAlias, This);
   This->pickAliasWin->Show("Bcc");

} // End PickBccAlias

/*---------------------------------------------------------------
 *  Callbacks to add selected aliases to text fields
 */

void
SendWinP::AddToAlias(PickAliasWinC *paw, SendWinP *This)
{
   StringC	toStr;
   This->toText->GetString(toStr, TT_PLAIN);
   toStr.Trim();

   const StringListC&	aliasList = paw->AliasList();
   u_int	count = aliasList.size();
   for (int i=0; i<count; i++) {
      StringC	*alias = aliasList[i];
      if ( toStr.size() > 0 && !toStr.EndsWith(',') ) toStr += ", ";
      toStr += *alias;
   }

   This->toText->SetString(toStr);

} // End AddToAlias

void
SendWinP::AddCcAlias(PickAliasWinC *paw, SendWinP *This)
{
   StringC	ccStr;
   This->ccText->GetString(ccStr, TT_PLAIN);
   ccStr.Trim();

   const StringListC&	aliasList = paw->AliasList();
   u_int	count = aliasList.size();
   for (int i=0; i<count; i++) {
      StringC	*alias = aliasList[i];
      if ( ccStr.size() > 0 && !ccStr.EndsWith(',') ) ccStr += ", ";
      ccStr += *alias;
   }

   This->ccText->SetString(ccStr);

} // End AddCcAlias

void
SendWinP::AddBccAlias(PickAliasWinC *paw, SendWinP *This)
{
   StringC	bccStr;
   This->bccText->GetString(bccStr, TT_PLAIN);
   bccStr.Trim();

   const StringListC&	aliasList = paw->AliasList();
   u_int	count = aliasList.size();
   for (int i=0; i<count; i++) {
      StringC	*alias = aliasList[i];
      if ( bccStr.size() > 0 && !bccStr.EndsWith(',') ) bccStr += ", ";
      bccStr += *alias;
   }

   This->bccText->SetString(bccStr);

} // End AddBccAlias

/*---------------------------------------------------------------
 *  Method to display the body for a given message
 */

void
SendWinP::DisplayBody(MsgC *msg, Boolean forceText)
{
   pub->BusyCursor(True);

   bodyText->Defer(True);
   bodyText->Clear();

//
// Tell the richtext widget whether to look for ">From " lines or not
//
   bodyText->CheckFroms(msg->HasSafeFroms());

//
// Check for a MIME message
//
   if ( msg->IsMime() && !forceText ) {
      MsgPartC	*tree = msg->Body();
      AddBodyTree(tree);
   }
      
   else {
      StringC	body;
      msg->GetBodyText(body);
      bodyText->ForceFixed(True);
      bodyText->SetTextType(TT_PLAIN);
      bodyText->SetString(body);
   }

   bodyText->ScrollTop();
   bodyText->Defer(False);

   pub->BusyCursor(False);

} // End DisplayBody

/*---------------------------------------------------------------
 *  Method to display a MIME message tree
 */

void
SendWinP::AddBodyTree(MsgPartC *tree)
{
   tree->Scan(NULL, tree->bodyBytes, NULL);

   Boolean	plain;
   if ( optMimeTB ) plain = (!XmToggleButtonGetState(optMimeTB) ||
			      XmToggleButtonGetState(optTextPlainTB));
   else			plain = (ishApp->mailPrefs->mailType == MAIL_PLAIN ||
				 ishApp->mailPrefs->textType == CT_PLAIN);
   bodyText->ForceFixed(plain || tree->PlainTextOnly());

//
// If the top-level is a multipart alternative with two children
//    and the first part is text/plain and the second part is text/enriched
//    text/richtext, display the second part only.
//
   if ( tree->IsAlternative() && tree->ChildCount() == 2 ) {

      MsgPartC	*child = tree->child;
      MsgPartC	*next  = child->next;
      if ( child->IsPlainText() &&
	   (next->IsEnriched() || next->IsRichText()) ) {
	 AddBodyPart(next);
	 return;
      }
   }

//
// If this is a multipart, display the children
//
   if ( tree->IsMultipart() ) {

      containerType = tree->conType;

      MsgPartC	*child = tree->child;
      
      if ( Query("Include attachments in the reply?", pub->topLevel,
                 /*cancel*/False) == QUERY_YES) {
         while ( child ) {
	    AddBodyPart(child, /*doChildren=*/False);
	    child = child->next;
         }
      } else {
         AddBodyPart(child, /*doChildren=*/False);
      }
   }

   else {
      AddBodyPart(tree);
   }

} // End AddBodyTree

/*---------------------------------------------------------------
 *  Method to display a single part of a MIME message
 */

void
SendWinP::AddBodyPart(MsgPartC *part, Boolean doChildren)
{
   if ( !part ) {
      if ( debuglev > 0 ) cout <<"Body part is NULL" <<endl;
      return;
   }

   if ( debuglev > 0 ) {
      cout <<"Msg part " <<part <<": " <<endl;
      cout <<*part <<endl;
   }

   StringC	label;
   part->GetLabel(label);

   StringC	msgStr("Adding part");
   msgStr += label;
   pub->Message(label);

   if ( part->IsExternal() ) {
      label += " <";
      switch (part->accType) {
	 case AT_LOCAL_FILE:	label += "local";	break;
	 case AT_ANON_FTP:	label += "anon FTP";	break;
	 case AT_FTP:		label += "FTP";		break;
	 case AT_TFTP:		label += "TFTP";	break;
	 case AT_MAIL_SERVER:	label += "mail server";	break;
	 case AT_INLINE:	label += "in-line";	break;
      }
      label += ">";
   }

//
// Display this part as necessary
//
   Boolean	isText = (part->IsText() && !part->IsExternal() &&
      					    !part->IsAttachment());
   if ( isText ) {

//
// Set the text type
//
      TextTypeT	ttype = TT_PLAIN;
      if      ( part->IsRichText() ) ttype = TT_RICH;
      else if ( part->IsEnriched() ) ttype = TT_ENRICHED;
      bodyText->SetTextType(ttype);

//
// See if we need an alternate character set
//
      ParamC	*csParam = part->Param("charset");
      if ( csParam ) bodyText->AddCommand(csParam->val);

//
// Add the text
//
      StringC	body;
      part->GetData(body);
      bodyText->AddString(body);

//
// Restore the character set if we changed it
//
      if ( csParam ) bodyText->AddCommand(csParam->val, True/*negate*/);

   } // End if this is a text part

//
// See if this is a multipart
//
   else if ( part->IsMultipart() ) {

//
// Return if there are no children.
//
      int	childCount = part->ChildCount();
      if ( childCount == 0 ) return;

//
// If there is only one child or this is a multipart/mixed, process the child
//    instead of the container.
//
      if ( childCount == 1 )
	 AddBodyPart(part->child, doChildren);

//
// If we're processing children, add them
//
      else if ( doChildren ) {

	 MsgPartC	*child = part->child;
	 while ( child ) {
	    AddBodyPart(child, False/*Don't follow any more children*/);
	    child = child->next;
	 }
      }

//
// If we're not processing children, just add a graphic for this container
//
      else {
	 SendIconC	*icon = new SendIconC(pub, part);
	 icon->AddDoubleClickCallback((CallbackFn*)OpenPart,     this);
	 icon->AddMenuCallback       ((CallbackFn*)PostPartMenu, this);
	 bodyText->AddGraphic(icon);
      }

   } // End if this is a multipart

//
// Build panel label for others
//
   else {

      SendIconC	*icon = new SendIconC(pub, part);
      icon->AddDoubleClickCallback((CallbackFn*)OpenPart,     this);
      icon->AddMenuCallback       ((CallbackFn*)PostPartMenu, this);
      bodyText->AddGraphic(icon);

//
// If this is a message in a digest, add a linefeed
//
      if ( part->Is822() && part->parent && part->parent->IsDigest() )
	 bodyText->AddStringPlain("\n");

   } // End if not inline text or multipart

} // End AddBodyPart

/*---------------------------------------------------------------
 *  Method to set sensitivities for buttons that depend on what we
 *     are editing
 */

void
SendWinP::UpdateEditButtons()
{
   XtSetSensitive(fileIncCB,		!editMsgText);
   XtSetSensitive(fileAddCB,		!editMsgText);
   XtSetSensitive(fileSavePB,		!editMsgText);
   XtSetSensitive(fileLoadPB,		!editMsgText);

   XtSetSensitive(editPlainPB,		!editMsgText);
   XtSetSensitive(editBoldPB,		!editMsgText);
   XtSetSensitive(editItalicPB,		!editMsgText);
   XtSetSensitive(editFixedPB,		!editMsgText);
   XtSetSensitive(editUnderPB,		!editMsgText);
   XtSetSensitive(editBiggerPB,		!editMsgText);
   XtSetSensitive(editSmallerPB,	!editMsgText);
   XtSetSensitive(editColorCB,		!editMsgText);
   XtSetSensitive(editLeftPB,		!editMsgText);
   XtSetSensitive(editRightPB,		!editMsgText);
   XtSetSensitive(editBothPB,		!editMsgText);
   XtSetSensitive(editCenterPB,		!editMsgText);
   XtSetSensitive(editNoFillPB,		!editMsgText);
   XtSetSensitive(editExcMorePB,	!editMsgText);
   XtSetSensitive(editExcLessPB,	!editMsgText);
   XtSetSensitive(editLeftInPB,		!editMsgText);
   XtSetSensitive(editLeftOutPB,	!editMsgText);
   XtSetSensitive(editRightInPB,	!editMsgText);
   XtSetSensitive(editRightOutPB,	!editMsgText);

   buttMgr->EnableButtons();

} // End EditUpdateButtons
