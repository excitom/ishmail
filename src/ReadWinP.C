/*
 *  $Id: ReadWinP.C,v 1.5 2000/08/07 11:05:17 evgeny Exp $
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
#include "ReadWinP.h"
#include "ReadWinC.h"
#include "QuickMenu.h"
#include "IshAppC.h"
#include "AppPrefC.h"
#include "FolderPrefC.h"
#include "ReadPrefC.h"
#include "ReplyPrefC.h"
#include "SaveMgrC.h"
#include "HeaderC.h"
#include "HeaderValC.h"
#include "HeadPrefC.h"
#include "MsgC.h"
#include "MainWinC.h"
#include "FolderC.h"
#include "ButtonMgrC.h"
#include "MsgPartC.h"
#include "ReadIconC.h"
#include "Mailcap.h"
#include "ParamC.h"
#include "LocalTextWinC.h"
#include "PipeWinC.h"
#include "PrintWinC.h"
#include "LoginWinC.h"
#include "MsgItemC.h"
#include "FileChooserWinC.h"
#include "SortMgrC.h"
#include "SendWinC.h"
#include "reading.xpm"

#include <hgl/WArgList.h>
#include <hgl/rsrc.h>
#include <hgl/WXmString.h>
#include <hgl/MimeRichTextC.h>
#include <hgl/CharC.h>
#include <hgl/PixmapC.h>
#include <hgl/VBoxC.h>

#include <Xm/CascadeB.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>
#include <Xm/Separator.h>
#include <Xm/ToggleB.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/PanedW.h>
#include <Xm/AtomMgr.h>

#include <signal.h>	// For kill

#define CreatePB	XmCreatePushButton

PtrListC	*ReadWinP::displayPixmaps = NULL;
PtrListC	*ReadWinP::fetchDataList  = NULL;
PtrListC	*ReadWinP::fetchPixmaps   = NULL;

/*---------------------------------------------------------------
 *  Constructor
 */

ReadWinP::ReadWinP(ReadWinC *rw)
{
   pub = rw;

   msgHeadText    = NULL;
   msgBodyText    = NULL;
   readIcon       = NULL;
   viewType       = ishApp->readPrefs->viewType;
   recentMenuTime = 0;

   pipeWin	  = NULL;
   printWin	  = NULL;
   loginWin	  = NULL;
   savePartWin    = NULL;
   editWin	  = NULL;

   if ( !displayPixmaps ) {
      displayPixmaps = new PtrListC;
      fetchDataList  = new PtrListC;
      fetchPixmaps   = new PtrListC;
   }
}

/*---------------------------------------------------------------
 *  Destructor
 */

ReadWinP::~ReadWinP()
{
//
// Kill any display processes still running
//
   u_int	count = displayDataList.size();
   int	i;
   for (i=0; i<count; i++) {
      DisplayDataT	*data = (DisplayDataT*)*displayDataList[i];
      if ( kill(-data->pid, 0) == 0 ) kill(-data->pid, SIGKILL);
   }

//
// Delete local text windows
//
   count = textWinList.size();
   for (i=0; i<count; i++) {
      LocalTextWinC	*tw = (LocalTextWinC*)*textWinList[i];
      delete tw;
   }

//
// Delete local message windows
//
   count = msgWinList.size();
   for (i=0; i<count; i++) {
      ReadWinC	*rw = (ReadWinC*)*msgWinList[i];
      delete rw;
   }

   delete pipeWin;
   delete printWin;
   delete loginWin;
   delete savePartWin;
   delete editWin;

   delete msgHeadText;
   delete msgBodyText;
   delete readIcon;

   if ( ishApp->exiting ) {

//
// Kill any retrieval processes still running
//
      count = fetchDataList->size();
      for (i=count-1; i>=0; i--) {
	 FetchDataT	*data = (FetchDataT*)*(*fetchDataList)[i];
	 if ( kill(-data->pid, 0) == 0 ) kill(-data->pid, SIGKILL);
      }

      count = displayPixmaps->size();
      for (i=0; i<count; i++) {
	 PixmapC	*pm = (PixmapC*)*(*displayPixmaps)[i];
	 delete pm;
      }

      count = fetchPixmaps->size();
      for (i=0; i<count; i++) {
	 PixmapC	*pm = (PixmapC*)*(*fetchPixmaps)[i];
	 delete pm;
      }

   } // End if exiting

} // End destructor

/*---------------------------------------------------------------
 *  Method to build menu hierarchy
 */

void
ReadWinP::BuildMenus()
{
   pub->AddMenuBar();

   BuildFileMenu();
   BuildReplyMenu();
   BuildOptMenu();

   if ( pub->fullFunction ) pub->AddHelpMenu();
}

/*---------------------------------------------------------------
 *  Method to build file menu hierarchy
 */

void
ReadWinP::BuildFileMenu()
{
   Cardinal	wcount;
   Widget	wlist[16];
   WArgList	args;

//
// Create filePD hierarchy
//
// filePD
//    CascadeButton     fileNextCB
//    PulldownMenu      fileNextPD
//    CascadeButton     filePrevCB
//    PulldownMenu      filePrevPD
//    CascadeButton     fileSaveCB
//    PulldownMenu      fileSavePD
//    PushButton	filePrintPB
//    CascadeButton     fileFilterCB
//    PulldownMenu      fileFilterPD
//    Separator		fileSep1
//    PushButton	fileUndeletePB
//    PushButton	fileDeletePB
//    PushButton	fileDelClosePB
//    Separator		fileSep2
//    PushButton	fileClosePB
//
   Widget fileNextCB;
   Widget filePrevCB;

   if ( pub->fullFunction ) {
      fileNextCB = XmCreateCascadeButton(pub->filePD, "fileNextCB", 0,0);
      filePrevCB = XmCreateCascadeButton(pub->filePD, "filePrevCB", 0,0);
   }
   else {
      fileNextPB = XmCreatePushButton(pub->filePD, "fileNextPB", 0,0);
      filePrevPB = XmCreatePushButton(pub->filePD, "filePrevPB", 0,0);
   }

   Widget fileSaveCB = XmCreateCascadeButton(pub->filePD, "fileSaveCB",   0,0);
   filePrintPB       = XmCreatePushButton   (pub->filePD, "filePrintPB",  0,0);
   Widget fileFilterCB = XmCreateCascadeButton(pub->filePD, "fileFilterCB",0,0);
   Widget fileSep1   = XmCreateSeparator    (pub->filePD, "fileSep1",    0,0);

   Widget fileSep2;
   if ( pub->fullFunction ) {
      fileUndeletePB = XmCreatePushButton(pub->filePD, "fileUndeletePB", 0,0);
      fileDeletePB   = XmCreatePushButton(pub->filePD, "fileDeletePB",   0,0);
      fileDelClosePB = XmCreatePushButton(pub->filePD, "fileDelClosePB", 0,0);
      fileSep2       = XmCreateSeparator (pub->filePD, "fileSep2",       0,0);
   }

   Widget fileClosePB = XmCreatePushButton(pub->filePD, "fileClosePB",  0,0);

   wcount = 0;
   wlist[wcount++] = fileSaveCB;
   wlist[wcount++] = filePrintPB;
   wlist[wcount++] = fileFilterCB;
   wlist[wcount++] = fileSep1;
   if ( pub->fullFunction ) {
      wlist[wcount++] = fileNextCB;
      wlist[wcount++] = filePrevCB;
      wlist[wcount++] = fileUndeletePB;
      wlist[wcount++] = fileDeletePB;
      wlist[wcount++] = fileDelClosePB;
      wlist[wcount++] = fileSep2;
   }
   else {
      wlist[wcount++] = fileNextPB;
      wlist[wcount++] = filePrevPB;
   }
   wlist[wcount++] = fileClosePB;
   XtManageChildren(wlist, wcount);

//
// Create fileNextPD hierarchy
//
//    fileNextPD
//       PushButton	   fileNextPB
//       PushButton	   fileNextUnreadPB
//       PushButton	   fileNextSenderPB
//       PushButton	   fileNextSubjectPB
//
   if ( pub->fullFunction ) {

      Widget fileNextPD = XmCreatePulldownMenu (pub->filePD, "fileNextPD", 0,0);
      fileNextPB        = CreatePB(fileNextPD, "fileNextPB",        0,0);
      fileNextUnreadPB  = CreatePB(fileNextPD, "fileNextUnreadPB",  0,0);
      fileNextSenderPB  = CreatePB(fileNextPD, "fileNextSenderPB",  0,0);
      fileNextSubjectPB = CreatePB(fileNextPD, "fileNextSubjectPB", 0,0);

      wcount = 0;
      wlist[wcount++] = fileNextPB;
      wlist[wcount++] = fileNextUnreadPB;
      wlist[wcount++] = fileNextSenderPB;
      wlist[wcount++] = fileNextSubjectPB;
      XtManageChildren(wlist, wcount);	// fileNextPD children

      XtVaSetValues(fileNextCB, XmNsubMenuId, fileNextPD, NULL);
   }

//
// Create filePrevPD hierarchy
//
//    filePrevPD
//       PushButton	   filePrevPB
//       PushButton	   filePrevUnreadPB
//       PushButton	   filePrevSenderPB
//       PushButton	   filePrevSubjectPB
//
   if ( pub->fullFunction ) {

      Widget filePrevPD = XmCreatePulldownMenu (pub->filePD, "filePrevPD", 0,0);
      filePrevPB        = CreatePB(filePrevPD, "filePrevPB",        0,0);
      filePrevUnreadPB  = CreatePB(filePrevPD, "filePrevUnreadPB",  0,0);
      filePrevSenderPB  = CreatePB(filePrevPD, "filePrevSenderPB",  0,0);
      filePrevSubjectPB = CreatePB(filePrevPD, "filePrevSubjectPB", 0,0);

      wlist[0] = filePrevPB;
      wlist[1] = filePrevUnreadPB;
      wlist[2] = filePrevSenderPB;
      wlist[3] = filePrevSubjectPB;
      XtManageChildren(wlist, 4);	// filePrevPD children

      XtVaSetValues(filePrevCB, XmNsubMenuId, filePrevPD, NULL);
   }

//
// Create fileSavePD hierarchy
//
//    fileSavePD
//       PushButton        fileSavePB
//       CascadeButton     fileSaveRecentCB
//       CascadeButton     fileSaveQuickCB
//       PushButton        fileSaveSelPB
//       PushButton        fileSaveToPB
//       PushButton        fileSaveToFilePB
//
   Widget fileSavePD = XmCreatePulldownMenu(pub->filePD, "fileSavePD",  0,0);
   fileSavePB        = XmCreatePushButton(fileSavePD, "fileSavePB",    0,0);
   fileSaveRecentCB  =
      XmCreateCascadeButton(fileSavePD, "fileSaveRecentCB", 0,0);
   fileSaveRecentPD = XmCreatePulldownMenu(fileSavePD, "fileSaveRecentPD", 0,0);
   XtVaSetValues(fileSaveRecentCB, XmNsubMenuId, fileSaveRecentPD, NULL);

//
// Add a data structure and a pulldown menu for the quick function
//
   QuickInfoT	*cbData = new QuickInfoT;
   if ( (!ishApp->appPrefs->usingImap) ||
	(!ishApp->folderPrefs->UsingLocal()) )
       cbData->dir = ishApp->appPrefs->FolderDir();
   else
       cbData->dir = "";
   cbData->menu = XmCreatePulldownMenu(fileSavePD, "quickPD", 0,0);
   cbData->menuTime = 0;

   args.Reset();
   args.UserData(cbData);
   args.SubMenuId(cbData->menu);
   fileSaveQuickCB   =
      XmCreateCascadeButton(fileSavePD, "fileSaveQuickCB",  ARGS);

   fileSaveSelPB    = XmCreatePushButton(fileSavePD, "fileSaveSelPB",    0,0);
   fileSaveToPB     = XmCreatePushButton(fileSavePD, "fileSaveToPB",     0,0);
   fileSaveToFilePB = XmCreatePushButton(fileSavePD, "fileSaveToFilePB", 0,0);

   wcount = 0;
   wlist[wcount++] = fileSavePB;
   wlist[wcount++] = fileSaveRecentCB;
   wlist[wcount++] = fileSaveQuickCB;
   wlist[wcount++] = fileSaveSelPB;
   wlist[wcount++] = fileSaveToPB;
   wlist[wcount++] = fileSaveToFilePB;
   XtManageChildren(wlist, wcount);	// fileSavePD children

   XtVaSetValues(fileSaveCB, XmNsubMenuId, fileSavePD, NULL);

//
// Create fileFilterPD hierarchy
//
//    fileFilterPD
//       PushButton	fileMimePB
//       PushButton	fileDecryptPB
//       PushButton	fileAuthPB
//       PushButton	fileEditPB
//       PushButton	filePipePB
//
   Widget fileFilterPD = XmCreatePulldownMenu(pub->filePD, "fileFilterPD", 0,0);
   if ( pub->fullFunction )
      fileMimePB = XmCreatePushButton       (fileFilterPD, "fileMimePB",   0,0);
   Widget fileDecryptPB = XmCreatePushButton(fileFilterPD, "fileDecryptPB",0,0);
   Widget fileAuthPB    = XmCreatePushButton(fileFilterPD, "fileAuthPB",   0,0);
   Widget fileEditPB;
   if ( pub->fullFunction )
      fileEditPB = XmCreatePushButton       (fileFilterPD, "fileEditPB",   0,0);
   filePipePB           = XmCreatePushButton(fileFilterPD, "filePipePB",   0,0);

   wcount = 0;
   wlist[wcount++] = fileDecryptPB;
   wlist[wcount++] = fileAuthPB;
   wlist[wcount++] = filePipePB;
   if ( pub->fullFunction ) {
      wlist[wcount++] = fileMimePB;
      wlist[wcount++] = fileEditPB;
   }
   XtManageChildren(wlist, wcount);

   XtVaSetValues(fileFilterCB, XmNsubMenuId, fileFilterPD, NULL);

//
// Set initial sensitivities
//
   XtSetSensitive(fileSavePB,       False);
   XtSetSensitive(fileSaveSelPB,    False);
   XtSetSensitive(fileSaveToPB,     False);
   XtSetSensitive(fileSaveToFilePB, False);
   XtSetSensitive(fileSaveRecentCB, False);
   XtSetSensitive(fileSaveQuickCB,  False);
   XtSetSensitive(filePrintPB,      False);
   XtSetSensitive(filePipePB,       False);
   if ( pub->fullFunction ) {
      XtSetSensitive(fileMimePB,     False);
      XtSetSensitive(fileUndeletePB, False);
      XtSetSensitive(fileDeletePB,   False);
      XtSetSensitive(fileDelClosePB, False);
   }

//
// Add callbacks
//
   //AddCascading(fileSaveRecentCB, PrepareRecentMenu, this);
   AddCascading(fileSaveQuickCB,  PrepareQuickMenu,  this);

   AddActivate(fileNextPB, DoNext, this);
   AddActivate(filePrevPB, DoPrev, this);

   if ( pub->fullFunction ) {
      AddActivate(fileNextUnreadPB,  DoNextUnread,  this);
      AddActivate(fileNextSenderPB,  DoNextSender,  this);
      AddActivate(fileNextSubjectPB, DoNextSubject, this);
      AddActivate(filePrevUnreadPB,  DoPrevUnread,  this);
      AddActivate(filePrevSenderPB,  DoPrevSender,  this);
      AddActivate(filePrevSubjectPB, DoPrevSubject, this);
   }

   AddActivate(fileSavePB,       DoSave,       this);
   AddActivate(fileSaveSelPB,    DoSaveSel,    this);
   AddActivate(fileSaveToPB,     DoSaveTo,     this);
   AddActivate(fileSaveToFilePB, DoSaveToFile, this);

   AddActivate(filePrintPB,      DoPrint,      this);
   AddActivate(filePipePB,       DoPipe,       this);
   AddActivate(fileDecryptPB,	 DoDecrypt,    this);
   AddActivate(fileAuthPB,	 DoAuth,       this);

   if ( pub->fullFunction ) {
      AddActivate(fileMimePB,     DoSunToMime, this);
      AddActivate(fileEditPB,	  DoEdit,      this);
      AddActivate(fileUndeletePB, DoUndelete,  this);
      AddActivate(fileDeletePB,   DoDelete,    this);
      AddActivate(fileDelClosePB, DoDelClose,  this);
   }

   AddActivate(fileClosePB, DoClose, this);

   pub->AddHideCallback((CallbackFn*)DoHide, NULL);

} // End BuildFileMenu

/*---------------------------------------------------------------
 *  Method to build reply menu hierarchy
 */

void
ReadWinP::BuildReplyMenu()
{
   Cardinal	wcount;
   Widget	wlist[10];

//
// Create cascade button and pulldown menu
//
          replyCB = XmCreateCascadeButton(pub->menuBar, "replyCB", 0,0);
   Widget replyPD = XmCreatePulldownMenu (pub->menuBar, "replyPD", 0,0);
   XtVaSetValues(replyCB, XmNsubMenuId, replyPD, NULL);
   XtManageChild(replyCB);
   XtSetSensitive(replyCB, False);

//
// Create replyPD hierarchy
//
// replyPD
//    PushButton	replyPB
//    PushButton	replyIncPB
//    PushButton	replyAllPB
//    PushButton	replyAllIncPB
//    Separator		replySep1
//    PushButton	forwardPB
//    PushButton	forward822PB
//    PushButton	resendPB
//
   Widget replyPB       = XmCreatePushButton(replyPD, "replyPB",       0,0);
   Widget replyIncPB    = XmCreatePushButton(replyPD, "replyIncPB",    0,0);
   Widget replyAllPB    = XmCreatePushButton(replyPD, "replyAllPB",    0,0);
   Widget replyAllIncPB = XmCreatePushButton(replyPD, "replyAllIncPB", 0,0);
   Widget replySep1     = XmCreateSeparator (replyPD, "replySep1",     0,0);
   Widget forwardPB     = XmCreatePushButton(replyPD, "forwardPB",     0,0);
   Widget forward822PB  = XmCreatePushButton(replyPD, "forward822PB",  0,0);
   Widget resendPB      = XmCreatePushButton(replyPD, "resendPB",      0,0);

//
// Manage widgets
//
   wcount = 0;
   wlist[wcount++] = replyPB;
   wlist[wcount++] = replyIncPB;
   wlist[wcount++] = replyAllPB;
   wlist[wcount++] = replyAllIncPB;
   wlist[wcount++] = replySep1;
   wlist[wcount++] = forwardPB;
   wlist[wcount++] = forward822PB;
   wlist[wcount++] = resendPB;
   XtManageChildren(wlist, wcount);

//
// Add callbacks
//
   AddActivate(replyPB,       DoReply,       this);
   AddActivate(replyIncPB,    DoReplyInc,    this);
   AddActivate(replyAllPB,    DoReplyAll,    this);
   AddActivate(replyAllIncPB, DoReplyAllInc, this);
   AddActivate(forwardPB,     DoForward,     this);
   AddActivate(forward822PB,  DoForward822,  this);
   AddActivate(resendPB,      DoResend,      this);

} // End BuildReplyMenu

/*---------------------------------------------------------------
 *  Method to build the option menu hierarchy
 */

void
ReadWinP::BuildOptMenu()
{
   Cardinal	wcount;
   Widget	wlist[8];

//
// Create cascade button and pulldown menu
//
   Widget	optCB	= XmCreateCascadeButton(pub->menuBar, "optCB",   0,0);
   Widget	optPD	 = XmCreatePulldownMenu(pub->menuBar, "optPD",   0,0);
   XtVaSetValues(optCB, XmNsubMenuId, optPD, NULL);
   XtManageChild(optCB);

//
// Create optPD hierarchy
//
// optPD
//    PushButton	optPrefPB
//    PushButton	optButtPB
//    PushButton	optHeadTB
//    PushButton	optWrapTB
//    CascadeButton	optViewCB
//    PulldownMenu	optViewPD
//
   Widget	optPrefPB;
   Widget	optButtPB;
   if ( pub->fullFunction ) {
      optPrefPB = XmCreatePushButton(optPD, "optPrefPB", 0,0);
      optButtPB = XmCreatePushButton(optPD, "optButtPB", 0,0);
   }

   optHeadTB = XmCreateToggleButton (optPD, "optHeadTB", 0,0);
   optWrapTB = XmCreateToggleButton (optPD, "optWrapTB", 0,0);
   optViewCB = XmCreateCascadeButton(optPD, "optViewCB", 0,0);
   Widget	optViewPD = XmCreatePulldownMenu(optPD, "optViewPD", 0,0);
   XtVaSetValues(optCB,     XmNsubMenuId,     optPD, NULL);
   XtVaSetValues(optViewCB, XmNsubMenuId,     optViewPD, NULL);
   XtVaSetValues(optViewPD, XmNradioBehavior, True,  NULL);

   wcount = 0;
   if ( pub->fullFunction ) {
      wlist[wcount++] = optPrefPB;
      wlist[wcount++] = optButtPB;
   }
   wlist[wcount++] = optHeadTB;
   wlist[wcount++] = optWrapTB;
   wlist[wcount++] = optViewCB;
   XtManageChildren(wlist, wcount);

   if ( pub->fullFunction ) {
      AddActivate(optPrefPB, DoOptPref, this);
      AddActivate(optButtPB, DoOptButt, this);
   }
   AddValueChanged(optHeadTB, DoOptHead, this);
   AddValueChanged(optWrapTB, DoOptWrap, this);

//
// Create optViewPD hierarchy
//
// optViewPD
//    ToggleButton	viewFlatTB
//    ToggleButton	viewOutlineTB
//    ToggleButton	viewNestedTB
//    ToggleButton	viewSourceTB
//
   viewFlatTB    = XmCreateToggleButton(optViewPD, "viewFlatTB",    0,0);
   viewOutlineTB = XmCreateToggleButton(optViewPD, "viewOutlineTB", 0,0);
   viewNestedTB  = XmCreateToggleButton(optViewPD, "viewNestedTB",  0,0);
   viewSourceTB  = XmCreateToggleButton(optViewPD, "viewSourceTB",  0,0);

   wcount = 0;
   wlist[wcount++] = viewFlatTB;
   wlist[wcount++] = viewOutlineTB;
//   wlist[wcount++] = viewNestedTB;
   wlist[wcount++] = viewSourceTB;
   XtManageChildren(wlist, wcount);

   AddValueChanged(viewFlatTB,    DoOptView, this);
   AddValueChanged(viewOutlineTB, DoOptView, this);
   AddValueChanged(viewNestedTB,  DoOptView, this);
   AddValueChanged(viewSourceTB,  DoOptView, this);

   XtSetSensitive(optViewCB, False);

//
// Set initial values
//
   XmToggleButtonSetState(optWrapTB, ishApp->readPrefs->wrap, False);

   viewType = ishApp->readPrefs->viewType;
   switch (viewType) {
      case (READ_VIEW_FLAT):
	 XmToggleButtonSetState(viewFlatTB, True, False);
	 break;
      case (READ_VIEW_OUTLINE):
	 XmToggleButtonSetState(viewOutlineTB, True, False);
	 break;
      case (READ_VIEW_CONTAINER):
	 XmToggleButtonSetState(viewNestedTB, True, False);
	 break;
      case (READ_VIEW_SOURCE):
	 XmToggleButtonSetState(viewSourceTB, True, False);
	 break;
   }

} // End BuildOptMenu

/*---------------------------------------------------------------
 *  Method to build the reading window widgets
 */

void
ReadWinP::BuildWidgets()
{
   WArgList	args;
   Cardinal	wcount;
   Widget	wlist[8];

//
// Create appForm hierarchy
//
// appForm
//    Form		titleForm
//       Label		msgTitle
//       ToggleButton	pushPinTB
//    PanedWindow	msgPanes
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_NONE);
   Widget	titleForm = XmCreateForm(pub->appForm, "titleForm", ARGS);

   args.Reset();
   args.TopAttachment(XmATTACH_NONE);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   msgTitle = XmCreateLabel(titleForm, "msgTitle", ARGS);

   msgTitleStr   = get_string(msgTitle, "labelString",     "Message $NUM");
   noMsgTitleStr = get_string(msgTitle, "noMessageString", "No messages");

   WXmString	ws = (char *)noMsgTitleStr;
   XtVaSetValues(titleForm, XmNresizePolicy, XmRESIZE_NONE, NULL);
   XtVaSetValues(msgTitle,  XmNlabelString, (XmString)ws,   NULL);
   XtVaSetValues(titleForm, XmNresizePolicy, XmRESIZE_ANY,  NULL);
   XtManageChild(msgTitle);

   pushPinTB = NULL;
   if ( pub->fullFunction ) {

      args.Reset();
      args.TopAttachment(XmATTACH_NONE);
      args.LeftAttachment(XmATTACH_NONE);
      args.RightAttachment(XmATTACH_FORM);
      args.BottomAttachment(XmATTACH_FORM);
      pushPinTB = XmCreateToggleButton(titleForm, "pushPinTB", ARGS);
      XtManageChild(pushPinTB);

   } // End if fullFunction

   args.Reset();
   args.TopAttachment(XmATTACH_WIDGET, titleForm);
   args.BottomAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   msgPanes = XmCreatePanedWindow(pub->appForm, "msgPanes", ARGS);

//
// Create msgPanes hierarchy
//
// msgPanes
//    MimeRichTextC	msgHeadText
//    MimeRichTextC	msgBodyText
//
   args.Reset();
   args.UserData(this);
   args.Rows(ishApp->readPrefs->visHeadRows);
   args.Columns(ishApp->readPrefs->visCols);
   msgHeadText = new MimeRichTextC(msgPanes, "msgHeadText", ARGS);
   msgHeadText->ResizeWidth(!ishApp->readPrefs->wrap);
   msgHeadText->SetTextType(TT_PLAIN);
   XtManageChild(msgHeadText->MainWidget());

   args.Reset();
   args.UserData(this);
   args.Rows(ishApp->readPrefs->visBodyRows);
   args.Columns(ishApp->readPrefs->visCols);
   msgBodyText = new MimeRichTextC(msgPanes, "msgBodyText", ARGS);
   msgBodyText->ResizeWidth(!ishApp->readPrefs->wrap);
   msgBodyText->SetTextType(TT_PLAIN);
   msgBodyText->SetExcerptString(ishApp->replyPrefs->indentPrefix);
   XtManageChild(msgBodyText->MainWidget());

//
// Register text widgets as drop sites for message icons
//
   if ( pub->fullFunction ) {

      XmDropSiteUnregister(msgHeadText->TextArea());
      XmDropSiteUnregister(msgBodyText->TextArea());

      Atom impAtoms[1];
      impAtoms[0] = ishApp->msgAtom;

      args.Reset();
      args.ImportTargets(impAtoms);
      args.NumImportTargets(1);
      args.DragProc((XtCallbackProc)MsgDragOver);
      args.DropProc((XtCallbackProc)MsgTextDrop);
      XmDropSiteRegister(msgHeadText->TextArea(), ARGS);
      XmDropSiteRegister(msgBodyText->TextArea(), ARGS);
   }

   wcount = 0;
   wlist[wcount++] = titleForm;
   wlist[wcount++] = msgPanes;
   XtManageChildren(wlist, wcount);	// appForm children

   pub->AddButtonBox();

#if 0
   if ( !pub->fullFunction ) {
      Widget       okPB   = XmCreatePushButton(pub->buttonRC, "okPB",   0,0);
      Widget       helpPB = XmCreatePushButton(pub->buttonRC, "helpPB", 0,0);
      XtManageChild(okPB);
      XtManageChild(helpPB);
      AddActivate(okPB,   DoClose, this);
      AddActivate(helpPB, HalAppC::DoHelp, "helpcard");
   }
#endif

//
// Create pixmap for this window's icon
//
   readIcon = new PixmapC(reading_xpm, (Window)*halApp,
			  XDefaultColormapOfScreen(halApp->screen));
   if ( readIcon->reg )
      XtVaSetValues(*pub, XmNiconPixmap, readIcon->reg,
                          XmNiconMask, readIcon->mask,
                          NULL);

   BuildMimePopupMenu();

//
// Add message line
//
   pub->ShowInfoMsg();

   pub->HandleHelp();

#if 000
//
// Add callback to finish initialization
//
   XtAddCallback(*this, XmNpopupCallback, (XtCallbackProc)DoPopup,
		 (XtPointer)this);
#endif

} // End BuildWidgets

/*---------------------------------------------------------------
 *  Method to build the mime graphic popup menu hierarchy
 */

void
ReadWinP::BuildMimePopupMenu()
{
//
// Create mimePU hierarchy
//
// mimePU
//    PushButton        mimePUShowPB
//    PushButton        mimePUHidePB
//    PushButton        mimePUSavePB
//    PushButton        mimePUPrintPB
//    PushButton        mimePUFetchPB
//    Separator         mimePUSep
//    Label             mimePULabel
//
   mimePU = XmCreatePopupMenu(msgBodyText->TextArea(), "mimePU", 0,0);

   mimePUShowPB     = XmCreatePushButton(mimePU, "mimePUShowPB",  0,0);
   mimePUHidePB     = XmCreatePushButton(mimePU, "mimePUHidePB",  0,0);
   mimePUSavePB     = XmCreatePushButton(mimePU, "mimePUSavePB",  0,0);
   mimePUPrintPB    = XmCreatePushButton(mimePU, "mimePUPrintPB", 0,0);
   mimePUFetchPB    = XmCreatePushButton(mimePU, "mimePUFetchPB", 0,0);
   Widget mimePUSep = XmCreateSeparator (mimePU, "mimePUSep",     0,0);
   mimePULabel      = XmCreateLabel     (mimePU, "mimePULabel",   0,0);

   Cardinal	wcount = 0;
   Widget	wlist[7];
   wlist[wcount++] = mimePUShowPB;
   wlist[wcount++] = mimePUHidePB;
   wlist[wcount++] = mimePUSavePB;
   wlist[wcount++] = mimePUPrintPB;
   wlist[wcount++] = mimePUFetchPB;
   wlist[wcount++] = mimePUSep;
   wlist[wcount++] = mimePULabel;
   XtManageChildren(wlist, wcount);

   AddActivate(mimePUShowPB,  DoShowPart,  this);
   AddActivate(mimePUHidePB,  DoHidePart,  this);
   AddActivate(mimePUSavePB,  DoSavePart,  this);
   AddActivate(mimePUPrintPB, DoPrintPart, this);
   AddActivate(mimePUFetchPB, DoFetchPart, this);

} // End BuildMimePopupMenu

/*---------------------------------------------------------------
 *  Method to read the resources for this window
 */

void
ReadWinP::ReadResources()
{
   Pixel	fg = msgBodyText->Foreground();
   Pixel	bg = msgBodyText->Background();

   asciiBg      = get_color (*pub, "asciiBackground", bg);
   imageBg      = get_color (*pub, "imageBackground", bg);
   audioBg      = get_color (*pub, "audioBackground", bg);
   videoBg      = get_color (*pub, "videoBackground", bg);
   asciiFg      = get_color (*pub, "asciiForeground", fg);
   imageFg      = get_color (*pub, "imageForeground", fg);
   audioFg      = get_color (*pub, "audioForeground", fg);
   videoFg      = get_color (*pub, "videoForeground", fg);

   asciiShowStr = get_string(*pub, "asciiShowLabel", "Show");
   imageShowStr = get_string(*pub, "imageShowLabel", "Show");
   audioShowStr = get_string(*pub, "audioShowLabel", "Play");
   videoShowStr = get_string(*pub, "videoShowLabel", "Play");
   asciiHideStr = get_string(*pub, "asciiHideLabel", "Hide");
   imageHideStr = get_string(*pub, "imageHideLabel", "Hide");
   audioHideStr = get_string(*pub, "audioHideLabel", "Stop");
   videoHideStr = get_string(*pub, "videoHideLabel", "Stop");

} // End ReadResources

/*---------------------------------------------------------------
 *  Method to update the name of the default save folder for this message
 */

void
ReadWinP::UpdateSaveFolder()
{
//
// Get the name of the default save folder
//
   ishApp->saveMgr->UpdateSaveFolder(pub->msg);

//
// Display folder name on button
//
   StringC	label = ishApp->saveMgr->curSaveFolder;
   ishApp->AbbreviateFolderName(label);
   label = "To: " + label;
   WXmString	wstr = (char*)label;
   XtVaSetValues(fileSavePB, XmNlabelString, (XmString)wstr, NULL);

// This causes too much re-sizing of the custom buttons
#if 0
//
// Update custom button if present
//
   if ( pub->fullFunction ) {
      Widget	custom = buttMgr->ButtonFor(fileSavePB);
      if ( custom )
	 XtVaSetValues(custom, XmNlabelString, (XmString)wstr, NULL);
   }
#endif

} // End UpdateSaveFolder

/*---------------------------------------------------------------
 *  Method to set button sensitivities
 */

void
ReadWinP::EnableButtons()
{
   MainWinC	*mainWin = ishApp->mainWin;
   u_int	 folderSelCount = mainWin->FolderVBox().SelItems().size();
   MsgC		*msg = pub->msg;

   XtSetSensitive(replyCB,	    msg != NULL);
   XtSetSensitive(fileSavePB,       msg != NULL);
   XtSetSensitive(fileSaveSelPB,    msg != NULL && folderSelCount>0);
   XtSetSensitive(fileSaveToPB,     msg != NULL);
   XtSetSensitive(fileSaveToFilePB, msg != NULL);
   XtSetSensitive(fileSaveRecentCB, msg != NULL &&
				    ishApp->appPrefs->recentFolders.size()>0);
   XtSetSensitive(fileSaveQuickCB,  msg != NULL);
   XtSetSensitive(filePrintPB,      msg != NULL);
   XtSetSensitive(filePipePB,       msg != NULL);
   XtSetSensitive(optViewCB,        msg != NULL /*&& msg->IsMime()*/);

   if ( pub->fullFunction ) {

      XtSetSensitive(fileMimePB, msg != NULL && !msg->IsDeleted() &&
				 msg->IsSet(MSG_SUN_ATTACH));
      XtSetSensitive(fileUndeletePB,
		     msg != NULL &&  msg->IsDeleted() && msg->folder->writable);
      XtSetSensitive(fileDeletePB,
		     msg != NULL && !msg->IsDeleted() && msg->folder->writable);
      XtSetSensitive(fileDelClosePB,
		     msg != NULL && !msg->IsDeleted() && msg->folder->writable);

      XtSetSensitive(fileNextPB,        mainWin->NextReadable(msg) != NULL);
      XtSetSensitive(fileNextUnreadPB,  mainWin->NextUnread(msg)   != NULL);
      XtSetSensitive(fileNextSenderPB,  mainWin->NextSender(msg)   != NULL);
      XtSetSensitive(fileNextSubjectPB, mainWin->NextSubject(msg)  != NULL);
      XtSetSensitive(filePrevPB,        mainWin->PrevReadable(msg) != NULL);
      XtSetSensitive(filePrevUnreadPB,  mainWin->PrevUnread(msg)   != NULL);
      XtSetSensitive(filePrevSenderPB,  mainWin->PrevSender(msg)   != NULL);
      XtSetSensitive(filePrevSubjectPB, mainWin->PrevSubject(msg)  != NULL);

   } // End if window is full function

//
// In rfc822 windows, the next and prev buttons are sensitive if this part
//    is in a digest and there are parts before or after.
//
   else {
      XtSetSensitive(fileNextPB,
		     pub->parentWin->priv->NextReadableIcon(pub) != NULL);
      XtSetSensitive(filePrevPB,
		     pub->parentWin->priv->PrevReadableIcon(pub) != NULL);
   }

   pub->buttMgr->EnableButtons();

} // End EnableButtons

/*---------------------------------------------------------------
 *  Method to display the headers for the current message
 */

void
ReadWinP::DisplayHeaders()
{
   if ( !pub->msg ) {
      msgHeadText->Clear();
      return;
   }

   msgHeadText->Defer(True);
   msgHeadText->Clear();

//
// Loop through headers
//
   Boolean	all    = XmToggleButtonGetState(optHeadTB);
   HeaderC	*head  = pub->msg->Headers();
   Boolean	needNL = False;
   while ( head ) {

//
// See if this header should be displayed
//
      if ( all || ishApp->headPrefs->HeaderShown(head->key) ) {

	 if ( needNL ) msgHeadText->AddString("\n");

	 HeaderValC	*val = head->value;

//
// See if this header has a complex value
//
	 if ( val->charset || val->next ) {

	    msgHeadText->AddString(head->key);
	    msgHeadText->AddString(": ");

//
// Loop through value components
//
	    while ( val ) {

	       if ( val->charset ) msgHeadText->AddCommand(val->charset);
	       msgHeadText->AddString(val->text);
	       if ( val->charset ) msgHeadText->AddCommand(val->charset,
							   True/*negate*/);

	       val = val->next;

	    } // End for each value component

	 } // End if value is complex

	 else {
	    msgHeadText->AddString(head->full);
	 }

	 needNL = True;

      } // End if header is displayed

      head = head->next;

   } // End for each header

   msgHeadText->ScrollTop();
   msgHeadText->Defer(False);

} // End DisplayHeaders

/*---------------------------------------------------------------
 *  Method to display the body for the current message
 */

void
ReadWinP::DisplayBody()
{
   if ( !pub->msg ) {
      msgBodyText->Clear();
      return;
   }

   pub->BusyCursor(True);

   msgBodyText->Defer(True);
   msgBodyText->Clear();

//
// Tell the richtext widget whether to look for ">From " lines or not
//
   msgBodyText->CheckFroms(pub->msg->HasSafeFroms());

//
// Check for a MIME message
//
   MsgPartC	*body = pub->msg->Body();
   if ( (viewType == READ_VIEW_OUTLINE || viewType == READ_VIEW_FLAT) &&
	(pub->msg->IsMime() || body->IsEncoded()) ) {

      if ( viewType == READ_VIEW_OUTLINE )
	 msgBodyText->SetTextType(TT_PLAIN);

      msgBodyText->ForceFixed((viewType != READ_VIEW_OUTLINE) &&
			      body->PlainTextOnly());

      AddBodyPart(body);
   }
      
   else {
      StringC	bodyStr;
      pub->msg->GetBodyText(bodyStr);
      msgBodyText->ForceFixed(True);
      msgBodyText->SetTextType(TT_PLAIN);
      msgBodyText->SetString(bodyStr);
   }

   msgBodyText->ScrollTop();
   msgBodyText->Defer(False);

   pub->BusyCursor(False);

} // End DisplayBody

/*---------------------------------------------------------------
 *  Function to determine if a mime part can be displayed in-line
 */

static Boolean
DisplayInline(MsgPartC *part)
{
   if ( part->IsExternal() || part->Is822() || part->IsAttachment() )
      return False;
    
//
// We won't also display inline attachments with Content-Disposition: inline
// set BUT a filename given (a "feature" of Communicator's mail).
//
   ParamC	*param = part->Param("filename", part->disParams);
   if (param) {
      return False;
   }
    
//
// Look for a mailcap entry and a present command.  If there is none,
//   the front end can display text.
//
   MailcapC	*mcap = MailcapEntry(part);
   if ( !mcap || mcap->present.size() == 0 )
      return (part->IsPlainText() || part->IsRichText() ||
	      part->IsEnriched() || part->IsMultipart());

//
// If there is a mailcap entry, see if "Ishmail" is the present command
//
   return mcap->present.Equals("ishmail", IGNORE_CASE);

} // End DisplayInline

/*---------------------------------------------------------------
 *  Method to display a single part of a MIME message
 */

void
ReadWinP::AddBodyPart(MsgPartC *part, Boolean doNext)
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

   StringC	msgStr("Adding part: ");
   msgStr += label;
   pub->Message(msgStr);

#if 0
   if ( !label.EndsWith(part->typeStr) ) {
      label += " (";
      label += part->typeStr;
      label += ")";
   }
#endif

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

#if 0
   else
      label += "in-line";
#endif

   if ( viewType == READ_VIEW_OUTLINE ) {

      MailcapC	*mcap = NULL;
      if ( part->IsMultipart() ) mcap = MailcapEntry(part);
      if ( part->IsMultipart() && (!mcap || mcap->present.size() == 0) ) {
	 StringC	desc;
	 part->GetDescription(desc);
	 if ( desc.size() == 0 ) desc = part->conStr;
	 msgBodyText->AddStringPlain(desc);
	 msgBodyText->AddStringPlain("\n");
      }

      else {

//
// Add an icon
//
	 ReadIconC	*icon = new ReadIconC(part, msgBodyText);
	 icon->SetLabel(label);
	 icon->AddDoubleClickCallback((CallbackFn*)HandleDoubleClick, this);
	 icon->AddMenuCallback       ((CallbackFn*)PostPartMenu,      this);

	 msgBodyText->AddStringPlain(part->partNum);
	 msgBodyText->AddStringPlain(" ");
	 msgBodyText->AddGraphic(icon);
	 msgBodyText->AddStringPlain("\n");


//
// See if there is a fetch in progress
//
	 FetchDataT	*data = FetchData(part);
	 if ( data ) {
	    data->win  = this;
	    data->icon = icon;
	    icon->Highlight();
	    LoadFetchPixmaps(icon);
	    icon->Animate(fetchPixmaps);
	 }

      } // End if this is a leaf

      if ( part->child ) AddBodyPart(part->child);

   } // End if outline view

   else {

//
// Display this part as necessary
//
      Boolean	displayHere = DisplayInline(part);
      if ( displayHere ) {

         if ( part->IsText() ) {

//
// Display the description
//
	    StringC	desc;
	    part->GetDescription(desc);
	    if ( desc.size() > 0 ) {
	       msgBodyText->SetTextType(TT_PLAIN);
	       if ( !msgBodyText->IsEmpty() ) msgBodyText->AddString("\n\n");
	       msgBodyText->AddCommand("underline");
	       msgBodyText->AddString(desc);
	       msgBodyText->AddCommand("underline", True/*negate*/);
	       msgBodyText->AddString("\n\n");
	    }

//
// Set the text type
//
	    TextTypeT	ttype = TT_PLAIN;
	    if      ( part->IsRichText() ) ttype = TT_RICH;
	    else if ( part->IsEnriched() ) ttype = TT_ENRICHED;
	    msgBodyText->SetTextType(ttype);
	    if ( ttype == TT_PLAIN ) msgBodyText->AddCommand("fixed");

//
// See if we need an alternate character set
//
	    ParamC	*csParam = part->Param("charset");
	    if ( csParam ) msgBodyText->AddCommand(csParam->val);

//
// Add the text
//
	    StringC	body;
	    part->GetData(body);
	    msgBodyText->AddString(body);

//
// Restore the character set if we changed it
//
	    if ( csParam )
	       msgBodyText->AddCommand(csParam->val, True/*negate*/);

	    if ( ttype == TT_PLAIN )
	       msgBodyText->AddCommand("fixed", True/*negate*/);

	 } // End if this is a text part

	 else if ( part->IsMultipart() && part->child ) {

//
// If this is a multipart/alternative, find the last text child we can display
//
	    if ( part->IsAlternative() ) {

	       MsgPartC	*altPart = part->BestTextAlternative();
	       AddBodyPart(altPart, False/*Don't do next*/);

// Show the remaining parts as alternatives to the text part

	       if ( altPart->next ) {
		  msgBodyText->AddStringPlain("\n");
		  msgBodyText->AddCommand("underline");
		  msgBodyText->AddStringPlain("Alternatives");
		  msgBodyText->AddCommand("underline", True/*negate*/);
		  msgBodyText->AddStringPlain("\n");
		  AddBodyPart(altPart->next, True/*Show all alternatives*/);
		  msgBodyText->AddStringPlain("\n");
		  msgBodyText->AddCommand("underline");
		  msgBodyText->AddStringPlain("End Alternatives");
		  msgBodyText->AddCommand("underline", True/*negate*/);
		  msgBodyText->AddStringPlain("\n\n");
	       }

	    } else {
	       AddBodyPart(part->child);
	    }
	 }

      } // End if we're displaying this part

//
// Build panel label for others
//
      else {

//
// Add an icon
//
	 ReadIconC	*icon = new ReadIconC(part, msgBodyText);
	 icon->SetLabel(label);
	 icon->AddDoubleClickCallback((CallbackFn*)HandleDoubleClick, this);
	 icon->AddMenuCallback       ((CallbackFn*)PostPartMenu,      this);

	 msgBodyText->AddGraphic(icon);

//
// See if there is a fetch in progress
//
	 FetchDataT	*data = FetchData(part);
	 if ( data ) {
	    data->win  = this;
	    data->icon = icon;
	    icon->Highlight();
	    LoadFetchPixmaps(icon);
	    icon->Animate(fetchPixmaps);
	 }

//
// If this is a message in a digest, add a linefeed
//
	 if ( part->Is822() && part->parent && part->parent->IsDigest() )
	    msgBodyText->AddStringPlain("\n");

      } // End if we're not displaying this part

   } // End if not outline view

//
// Add next part if present
//
   if ( part->next && doNext ) AddBodyPart(part->next);

} // End AddBodyPart

/*-----------------------------------------------------------------------
 *  Handle drag over event
 */

void
ReadWinP::MsgDragOver(Widget w, XtPointer, XmDragProcCallbackStruct *dp)
{
   if ( dp->reason == XmCR_DROP_SITE_MOTION_MESSAGE ) {

      MimeRichTextC	*text;
      XtVaGetValues(w, XmNuserData, &text, NULL);

      ReadWinP	*This;
      XtVaGetValues(text->MainWidget(), XmNuserData, &This, NULL);

      if ( This->pub->Pinned() )
	 dp->dropSiteStatus = XmINVALID_DROP_SITE;
      else
	 dp->dropSiteStatus = XmVALID_DROP_SITE;

   } // End if this is a motion message

//   dp->animate = False;

} // End MsgDragOver

/*-----------------------------------------------------------------------
 *  Handle message drop event
 */

void
ReadWinP::MsgTextDrop(Widget w, XtPointer, XmDropProcCallbackStruct *dp)
{
   //cout << "Got drop into message text" NL;

//
// Get the message number and open it
//
   ViewDragDataT	*dd;
   XtVaGetValues(dp->dragContext, XmNclientData, &dd, NULL);
   //cout <<"Drag data is " << dd NL;

   MimeRichTextC	*text;
   XtVaGetValues(w, XmNuserData, &text, NULL);

   ReadWinP	*This;
   XtVaGetValues(text->MainWidget(), XmNuserData, &This, NULL);

   WArgList	  args;
   if ( dp->dropSiteStatus == XmVALID_DROP_SITE && !This->pub->Pinned() ) {

      MsgItemC	*item = (MsgItemC*)(dd->itemList[0]);
      //cout <<"Item is " << item NL;

      VBoxC&	vbox = ishApp->mainWin->MsgVBox();

      VItemListC&	selList = vbox.SelItems();
      if ( !This->pub->Pinned() && selList.size() == 1 &&
	   selList[0] == This->pub->msg->icon ) {
	 vbox.DeselectItem(*This->pub->msg->icon, False/*don't notify*/);
	 vbox.SelectItem(*item);
      }

      ishApp->DisplayMessage(item->msg, This->pub);
      vbox.Refresh();
      args.TransferStatus(XmTRANSFER_SUCCESS);
   }

   else
      args.TransferStatus(XmTRANSFER_FAILURE);

//
// Drop must be acknowledged
//
   XmDropTransferStart(dp->dragContext, ARGS);

//
// Delete drag data
//
   delete dd;

} // End MsgTextDrop

/*---------------------------------------------------------------
 *  Method to clean up child windows and processes
 */

void
ReadWinP::Reset()
{
   if ( editWin ) editWin->Close();

//
// Kill any display processes still running
//
   u_int	count = displayDataList.size();
   int	i;
   for (i=0; i<count; i++) {
      DisplayDataT	*data = (DisplayDataT*)*displayDataList[i];
      if ( kill(-data->pid, 0) == 0 ) kill(-data->pid, SIGKILL);
   }
   displayDataList.removeAll();

//
// Close local text windows
//
   count = textWinList.size();
   for (i=0; i<count; i++) {
      LocalTextWinC	*tw = (LocalTextWinC*)*textWinList[i];
      tw->Hide();
   }

//
// Close local message windows
//
   count = msgWinList.size();
   for (i=0; i<count; i++) {
      ReadWinC	*rw = (ReadWinC*)*msgWinList[i];
      rw->Hide();
   }

//
// Clear text
//
   msgHeadText->Clear();
   msgBodyText->Clear();

//
// Reset message
//
   if ( pub->fullFunction && pub->msg ) {

      pub->msg->ClearViewed();

//
// Update main window
//
      ishApp->mainWin->curMsgList->remove(pub->msg->icon);
      ishApp->mainWin->UpdateTitle();
      ishApp->mainWin->EnableButtons();

//
// Re-sort if threaded.  Status could have changed
//
      if ( ishApp->mainWin->curMsgList->size() == 0 ) {
	 SortMgrC	*sortMgr = ishApp->mainWin->curFolder->SortMgr();
	 if ( sortMgr->Threaded() && sortMgr->StatusKey() )
	    ishApp->mainWin->MsgVBox().Sort();
      }

   } // End if closing a full function window

//
// Reset pushPin
//
   if ( pushPinTB )
      XmToggleButtonSetState(pushPinTB, False, True);

} // End Reset

