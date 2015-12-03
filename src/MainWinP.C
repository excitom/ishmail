/*
 *  $Id: MainWinP.C,v 1.14 2001/07/28 18:26:03 evgeny Exp $
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
#include "MainWinP.h"
#include "MainWinC.h"
#include "IshAppC.h"
#include "QuickMenu.h"
#include "AppPrefC.h"
#include "AliasPrefC.h"
#include "SumPrefC.h"
#include "FolderPrefC.h"
#include "MsgItemC.h"
#include "MsgC.h"
#include "IconPrefC.h"
#include "FolderC.h"
#include "Fork.h"
#include "AlertPrefC.h"
#include "SaveMgrC.h"
#include "ConfPrefC.h"
#include "AutoFilePrefC.h"
#include "FileChooserWinC.h"
#include "MsgFindWinC.h"
#include "ReadWinC.h"
#include "PrintWinC.h"
#include "PipeWinC.h"
#include "MsgListC.h"
#include "ReadPrefC.h"

#include <hgl/WArgList.h>
#include <hgl/TBoxC.h>
#include <hgl/VBoxC.h>
#include <hgl/FieldViewC.h>
#include <hgl/PixmapC.h>
#include <hgl/rsrc.h>
#include <hgl/WXmString.h>
#include <hgl/RegexC.h>
#include <hgl/RangeC.h>
#include <hgl/SysErr.h>
#include <hgl/VItemC.h>
#include <hgl/System.h>
#include <hgl/WidgetListC.h>

#include <Xm/PushB.h>
#include <Xm/Separator.h>
#include <Xm/CascadeB.h>
#include <Xm/RowColumn.h>
#include <Xm/PanedW.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/MessageB.h>
#include <Xm/Protocols.h>
#include <Xm/AtomMgr.h>

#include <unistd.h>
#include <sys/wait.h>

/*---------------------------------------------------------------
 *  Constructor
 */

MainWinP::MainWinP(MainWinC *mw)
{
   pub      = mw;
   iconDict = new PixmapNameDictC;
   iconDict->AllowDuplicates(FALSE);

   delFolderList = new FolderListC;
   lastDelList   = new VItemListC;

   recentMenuTime  = 0;
   deferRecentMenu = False;

   saveQueryWin	   = NULL;
   newFolderWin	   = NULL;
   openSelectWin   = NULL;
   folderVBox	   = NULL;
   msgTBox	   = NULL;
   fieldView	   = NULL;
   msgFindWin	   = NULL;
   printWin        = NULL;
   pipeWin         = NULL;

} // End constructor

/*---------------------------------------------------------------
 *  Destructor
 */

MainWinP::~MainWinP()
{
   //delete commentWin;	Don't do this since it is part of ishApp->sendWinList
   delete printWin;
   delete pipeWin;
   delete newFolderWin;
   delete openSelectWin;
   delete msgFindWin;
   delete folderVBox;
   delete fieldView;
   delete msgTBox;

//
// Delete pixmaps
//
   u_int	count = iconDict->size();
   int i=0; for (i=0; i<count; i++) {
      PixmapC   *pm = (*iconDict)[i]->val;
      delete pm;
   }

   delete iconDict;
   delete noMailPM;
   delete newMailPM;
   delete readMailPM;
   delete unreadMailPM;

   delete delFolderList;
   delete lastDelList;

} // End destructor

/*---------------------------------------------------------------
 *  Method to build menu hierarchy
 */

void
MainWinP::BuildMenus()
{
   BuildFileMenu();
   BuildFolderMenu();
   BuildMsgMenu();
   BuildOptMenu();
   BuildHelpMenu();

//
// Set the font on the fileCB since it is created before the user's font
//    resources are set.
//
   XmFontList	fontList;
   XtVaGetValues(pub->helpCB, XmNfontList, &fontList, NULL);
   XtVaSetValues(pub->fileCB, XmNfontList,  fontList, NULL);

} // End BuildMenus

/*---------------------------------------------------------------
 *  Method to build file menu hierarchy
 */

void
MainWinP::BuildFileMenu()
{
   Widget	wlist[5];

//
// filePD
//    PushButton	filePrintPB
//    Separator		fileSep1
//    PushButton	fileSleepPB
//    PushButton	fileQuitPB
//    PushButton	fileExitPB
//
   filePrintPB          = XmCreatePushButton(pub->filePD, "filePrintPB",   0,0);
   Widget fileSep1      = XmCreateSeparator (pub->filePD, "fileSep1",      0,0);
   Widget fileSleepPB   = XmCreatePushButton(pub->filePD, "fileSleepPB",   0,0);
   Widget fileQuitPB    = XmCreatePushButton(pub->filePD, "fileQuitPB",    0,0);
   Widget fileExitPB    = XmCreatePushButton(pub->filePD, "fileExitPB",    0,0);

   wlist[0] = filePrintPB;
   wlist[1] = fileSep1;
   wlist[2] = fileSleepPB;
   wlist[3] = fileQuitPB;
   wlist[4] = fileExitPB;
   XtManageChildren(wlist, 5);

   AddActivate(filePrintPB,   DoMsgPrint,    this);
   AddActivate(fileSleepPB,   DoFileSleep,   this);
   AddActivate(fileQuitPB,    DoFileQuit,    this);
   AddActivate(fileExitPB,    DoFileExit,    this);

   halApp->AddExitCallback((CallbackFn *)Exit, (void *)this);

} // End BuildFileMenu

/*---------------------------------------------------------------
 *  Method to build folder menu hierarchy
 */

void
MainWinP::BuildFolderMenu()
{
   WArgList	args;
   Cardinal	wcount;
   Widget	wlist[16];

//
// Create cascade button and pulldown menu
//
   Widget	folderCB = XmCreateCascadeButton(pub->menuBar, "folderCB", 0,0);
   Widget	folderPD = XmCreatePulldownMenu (pub->menuBar, "folderPD", 0,0);
   XtVaSetValues(folderCB, XmNsubMenuId, folderPD, NULL);
   XtManageChild(folderCB);

//
// Create folderPD hierarchy
//
// folderPD
//    PushButton	folderNewPB
//    PushButton	folderOpenPB
//    CascadeButton	folderOpenRecentCB
//    PulldownMenu	folderOpenRecentPD
//    CascadeButton	folderOpenQuickCB
//    PulldownMenu	folderOpenQuickPD
//    CascadeButton	folderActCB
//    PulldownMenu	folderActPD
//       PushButton	   folderActSysPB
//       PushButton	   folderActSelPB
//    Separator		folderSep1
//    CascadeButton	folderSaveCB
//    PulldownMenu	folderSavePD
//       PushButton	   folderSaveCurPB
//       PushButton	   folderSaveSelPB
//       PushButton	   folderSaveAllPB
//    CascadeButton	folderReadCB
//    PulldownMenu	folderReadPD
//       PushButton	   folderReadCurPB
//       PushButton	   folderReadSelPB
//       PushButton	   folderReadAllPB
//    CascadeButton	folderCloseCB
//    PulldownMenu	folderClosePD
//       PushButton	   folderCloseCurPB
//       PushButton	   folderCloseSelPB
//       PushButton	   folderCloseAllPB
//    CascadeButton	folderDelCB
//    PulldownMenu	folderDelPD
//       PushButton	   folderDelCurPB
//       PushButton	   folderDelSelPB
//       PushButton	   folderDelAllPB
//    Separator		folderSep2
//    PushButton	folderSelPB
//    PushButton	folderDeselPB
//
   Widget folderNewPB  = XmCreatePushButton(folderPD, "folderNewPB",   0,0);
   Widget folderOpenPB = XmCreatePushButton(folderPD, "folderOpenPB",  0,0);
   folderOpenRecentCB =
      XmCreateCascadeButton(folderPD, "folderOpenRecentCB", 0,0);
   folderOpenRecentPD =
      XmCreatePulldownMenu (folderPD, "folderOpenRecentPD", 0,0);
   XtVaSetValues(folderOpenRecentCB, XmNsubMenuId, folderOpenRecentPD, NULL);

//
// Add a data structure and a pulldown menu for the quick function
//
   QuickInfoT	*cbData = new QuickInfoT;
   if ( (!ishApp->appPrefs->usingImap) ||
	(!ishApp->folderPrefs->UsingLocal()) )
       cbData->dir = ishApp->appPrefs->FolderDir();
   else
       cbData->dir = "";
   cbData->menu = XmCreatePulldownMenu(folderPD, "quickPD", 0,0);
   cbData->menuTime = 0;

   args.Reset();
   args.UserData(cbData);
   args.SubMenuId(cbData->menu);
   folderOpenQuickCB = XmCreateCascadeButton(folderPD,"folderOpenQuickCB",ARGS);

   Widget folderActCB   = XmCreateCascadeButton(folderPD, "folderActCB",   0,0);
   Widget folderSep1    = XmCreateSeparator    (folderPD, "folderSep1",    0,0);
   Widget folderSaveCB  = XmCreateCascadeButton(folderPD, "folderSaveCB",  0,0);
   Widget folderReadCB  = XmCreateCascadeButton(folderPD, "folderReadCB",  0,0);
   Widget folderCloseCB = XmCreateCascadeButton(folderPD, "folderCloseCB", 0,0);
   Widget folderDelCB   = XmCreateCascadeButton(folderPD, "folderDelCB",   0,0);
   Widget folderSep2    = XmCreateSeparator    (folderPD, "folderSep2",    0,0);
          folderSelPB   = XmCreatePushButton   (folderPD, "folderSelPB",   0,0);
          folderDeselPB = XmCreatePushButton   (folderPD, "folderDeselPB", 0,0);

   Widget folderActPD   = XmCreatePulldownMenu(folderPD, "folderActPD", 0,0);
   Widget folderSavePD  = XmCreatePulldownMenu(folderPD, "folderSavePD", 0,0);
   Widget folderReadPD  = XmCreatePulldownMenu(folderPD, "folderReadPD", 0,0);
   Widget folderClosePD = XmCreatePulldownMenu(folderPD, "folderClosePD", 0,0);
   Widget folderDelPD   = XmCreatePulldownMenu(folderPD, "folderDelPD", 0,0);

   XtVaSetValues(folderActCB,   XmNsubMenuId, folderActPD,   NULL);
   XtVaSetValues(folderSaveCB,  XmNsubMenuId, folderSavePD,  NULL);

   XtVaSetValues(folderReadCB,  XmNsubMenuId, folderReadPD,  NULL);
   XtVaSetValues(folderCloseCB, XmNsubMenuId, folderClosePD, NULL);
   XtVaSetValues(folderDelCB,   XmNsubMenuId, folderDelPD,   NULL);

   wcount = 0;
   wlist[wcount++] = folderNewPB;
   wlist[wcount++] = folderOpenPB;
   wlist[wcount++] = folderOpenRecentCB;
   wlist[wcount++] = folderOpenQuickCB;
   wlist[wcount++] = folderActCB;
   wlist[wcount++] = folderSep1;
   wlist[wcount++] = folderSaveCB;
   wlist[wcount++] = folderReadCB;
   wlist[wcount++] = folderCloseCB;
   wlist[wcount++] = folderDelCB;
   wlist[wcount++] = folderSep2;
   wlist[wcount++] = folderSelPB;
   wlist[wcount++] = folderDeselPB;
   XtManageChildren(wlist, wcount);	// folderPD children

//
// Build folderActPD
//
// PushButton	folderActSysPB
// PushButton	folderActSelPB
//
   folderActSysPB = XmCreatePushButton(folderActPD, "folderActSysPB", 0,0);
   folderActSelPB = XmCreatePushButton(folderActPD, "folderActSelPB", 0,0);

   wlist[0] = folderActSysPB;
   wlist[1] = folderActSelPB;
   XtManageChildren(wlist, 2);	// folderActPD children

//
// Build folderSavePD
//
// PushButton	folderSaveCurPB
// PushButton	folderSaveSelPB
// PushButton	folderSaveAllPB
//
   folderSaveCurPB = XmCreatePushButton(folderSavePD, "folderSavePB",    0,0);
   folderSaveSelPB = XmCreatePushButton(folderSavePD, "folderSaveSelPB", 0,0);
   folderSaveAllPB = XmCreatePushButton(folderSavePD, "folderSaveAllPB", 0,0);

   wlist[0] = folderSaveCurPB;
   wlist[1] = folderSaveSelPB;
   wlist[2] = folderSaveAllPB;
   XtManageChildren(wlist, 3);	// folderSavePD children

//
// Build folderReadPD
//
// PushButton	folderReadCurPB
// PushButton	folderReadSelPB
// PushButton	folderReadAllPB
//
   folderReadCurPB = XmCreatePushButton(folderReadPD, "folderReadCurPB", 0,0);
   folderReadSelPB = XmCreatePushButton(folderReadPD, "folderReadSelPB", 0,0);
   folderReadAllPB = XmCreatePushButton(folderReadPD, "folderReadAllPB", 0,0);

   wlist[0] = folderReadCurPB;
   wlist[1] = folderReadSelPB;
   wlist[2] = folderReadAllPB;
   XtManageChildren(wlist, 3);	// folderReadPD children

//
// Build folderClosePD
//
// PushButton	folderCloseCurPB
// PushButton	folderCloseSelPB
// PushButton	folderCloseAllPB
//
   folderCloseCurPB= XmCreatePushButton(folderClosePD, "folderCloseCurPB", 0,0);
   folderCloseSelPB= XmCreatePushButton(folderClosePD, "folderCloseSelPB", 0,0);
   folderCloseAllPB= XmCreatePushButton(folderClosePD, "folderCloseAllPB", 0,0);

   wlist[0] = folderCloseCurPB;
   wlist[1] = folderCloseSelPB;
   wlist[2] = folderCloseAllPB;
   XtManageChildren(wlist, 3);	// folderClosePD children

//
// Build folderDelPD
//
// PushButton	folderDelCurPB
// PushButton	folderDelSelPB
// PushButton	folderDelAllPB
//
   folderDelCurPB = XmCreatePushButton(folderDelPD, "folderDelCurPB", 0,0);
   folderDelSelPB = XmCreatePushButton(folderDelPD, "folderDelSelPB", 0,0);
   folderDelAllPB = XmCreatePushButton(folderDelPD, "folderDelAllPB", 0,0);

   wlist[0] = folderDelCurPB;
   wlist[1] = folderDelSelPB;
   wlist[2] = folderDelAllPB;
   XtManageChildren(wlist, 3);	// folderDelPD children

//
// Set initial sensitivities
//
   XtSetSensitive(folderOpenRecentCB, False);
   XtSetSensitive(folderActSysPB,     False);
   XtSetSensitive(folderActSelPB,     False);
   XtSetSensitive(folderSaveCurPB,    False);
   XtSetSensitive(folderSaveSelPB,    False);
   XtSetSensitive(folderSaveAllPB,    False);
   XtSetSensitive(folderReadSelPB,    False);
   XtSetSensitive(folderCloseCurPB,   False);
   XtSetSensitive(folderCloseSelPB,   False);
   XtSetSensitive(folderCloseAllPB,   False);
   XtSetSensitive(folderDelCurPB,     False);
   XtSetSensitive(folderDelSelPB,     False);
   XtSetSensitive(folderDelAllPB,     False);
   XtSetSensitive(folderSelPB,        False);
   XtSetSensitive(folderDeselPB,      False);

   //AddCascading(folderOpenRecentCB, PrepareRecentMenu,     this);
   AddCascading(folderOpenQuickCB,  PrepareOpenQuickMenu,  this);

   AddActivate(folderNewPB,      DoFolderNew,      this);
   AddActivate(folderOpenPB,     DoFolderOpen,     this);
   AddActivate(folderActSysPB,   DoFolderActSys,   this);
   AddActivate(folderActSelPB,   DoFolderActSel,   this);
   AddActivate(folderSaveCurPB,  DoFolderSaveCur,  this);
   AddActivate(folderSaveSelPB,  DoFolderSaveSel,  this);
   AddActivate(folderSaveAllPB,  DoFolderSaveAll,  this);
   AddActivate(folderReadCurPB,  DoFolderReadCur,  this);
   AddActivate(folderReadSelPB,  DoFolderReadSel,  this);
   AddActivate(folderReadAllPB,  DoFolderReadAll,  this);
   AddActivate(folderCloseCurPB, DoFolderCloseCur, this);
   AddActivate(folderCloseSelPB, DoFolderCloseSel, this);
   AddActivate(folderCloseAllPB, DoFolderCloseAll, this);
   AddActivate(folderDelCurPB,   DoFolderDelCur,   this);
   AddActivate(folderDelSelPB,   DoFolderDelSel,   this);
   AddActivate(folderDelAllPB,   DoFolderDelAll,   this);
   AddActivate(folderSelPB,      DoFolderSel,      this);
   AddActivate(folderDeselPB,    DoFolderDesel,    this);

} // End BuildFolderMenu

/*---------------------------------------------------------------
 *  Method to build message menu hierarchy
 */

void
MainWinP::BuildMsgMenu()
{
   WArgList	args;
   Cardinal	wcount;
   Widget	wlist[24];

//
// Create cascade button and pulldown menu
//
   Widget	msgCB = XmCreateCascadeButton(pub->menuBar, "msgCB", 0,0);
   Widget	msgPD = XmCreatePulldownMenu (pub->menuBar, "msgPD", 0,0);
   XtVaSetValues(msgCB, XmNsubMenuId, msgPD, NULL);
   XtManageChild(msgCB);

//
// Create msgPD hierarchy
//
// msgPD
//    PushButton	msgComposePB
//    CascadeButton	msgReplyCB
//    PulldownMenu	msgReplyPD
//    PushButton	msgForwardPB
//    PushButton	msgForward822PB
//    PushButton	msgResendPB
//    Separator		msgSep1
//    CascadeButton	msgReadCB
//    PulldownMenu	msgReadPD
//    CascadeButton	msgSaveCB
//    PulldownMenu	msgSavePD
//    PushButton	msgPrintPB
//    PushButton	msgPipePB
//    Separator		msgSep2
//    PushButton	msgDelPB
//    CascadeButton	msgUndelCB
//    PulldownMenu	msgUndelPD
//    CascadeButton	msgStatCB
//    PulldownMenu	msgStatPD
//    Separator		msgSep3
//    PushButton	msgSelPB
//    PushButton	msgDeselPB
//    PushButton	msgFindPB
//
          msgComposePB    = XmCreatePushButton   (msgPD, "msgComposePB",   0,0);
   Widget msgReplyCB      = XmCreateCascadeButton(msgPD, "msgReplyCB",     0,0);
          msgForwardPB    = XmCreatePushButton   (msgPD, "msgForwardPB",   0,0);
          msgForward822PB = XmCreatePushButton   (msgPD, "msgForward822PB",0,0);
          msgResendPB     = XmCreatePushButton   (msgPD, "msgResendPB",    0,0);
   Widget msgSep1         = XmCreateSeparator    (msgPD, "msgSep1",        0,0);
   Widget msgReadCB       = XmCreateCascadeButton(msgPD, "msgReadCB",      0,0);
   Widget msgSaveCB       = XmCreateCascadeButton(msgPD, "msgSaveCB",      0,0);
          msgPrintPB      = XmCreatePushButton   (msgPD, "msgPrintPB",     0,0);
          msgPipePB       = XmCreatePushButton   (msgPD, "msgPipePB",      0,0);
   Widget msgSep2         = XmCreateSeparator    (msgPD, "msgSep2",        0,0);
          msgDelPB        = XmCreatePushButton   (msgPD, "msgDelPB",       0,0);
   Widget msgUndelCB      = XmCreateCascadeButton(msgPD, "msgUndelCB",     0,0);
          msgStatCB       = XmCreateCascadeButton(msgPD, "msgStatCB",      0,0);
   Widget msgSep3         = XmCreateSeparator    (msgPD, "msgSep3",        0,0);
          msgSelPB        = XmCreatePushButton   (msgPD, "msgSelPB",       0,0);
          msgDeselPB      = XmCreatePushButton   (msgPD, "msgDeselPB",     0,0);
          msgFindPB       = XmCreatePushButton   (msgPD, "msgFindPB",      0,0);

   wcount = 0;
   wlist[wcount++] = msgComposePB;
   wlist[wcount++] = msgReplyCB;
   wlist[wcount++] = msgForwardPB;
   wlist[wcount++] = msgForward822PB;
   wlist[wcount++] = msgResendPB;
   wlist[wcount++] = msgSep1;
   wlist[wcount++] = msgReadCB;
   wlist[wcount++] = msgSaveCB;
   wlist[wcount++] = msgPrintPB;
   wlist[wcount++] = msgPipePB;
   wlist[wcount++] = msgSep2;
   wlist[wcount++] = msgDelPB;
   wlist[wcount++] = msgUndelCB;
   wlist[wcount++] = msgStatCB;
   wlist[wcount++] = msgSep3;
   wlist[wcount++] = msgSelPB;
   wlist[wcount++] = msgDeselPB;
   wlist[wcount++] = msgFindPB;
   XtManageChildren(wlist, wcount);	// msgPD children

//
// Create msgReplyPD hierarchy
//
// msgReplyPD
//    PushButton	   msgReplyPB
//    PushButton	   msgReplyIncPB
//    PushButton	   msgReplyAllPB
//    PushButton	   msgReplyAllIncPB
//
   Widget msgReplyPD = XmCreatePulldownMenu(msgPD, "msgReplyPD", 0,0);
   msgReplyPB        = XmCreatePushButton(msgReplyPD, "msgReplyPB",       0,0);
   msgReplyIncPB     = XmCreatePushButton(msgReplyPD, "msgReplyIncPB",    0,0);
   msgReplyAllPB     = XmCreatePushButton(msgReplyPD, "msgReplyAllPB",    0,0);
   msgReplyAllIncPB  = XmCreatePushButton(msgReplyPD, "msgReplyAllIncPB", 0,0);

   wlist[0] = msgReplyPB;
   wlist[1] = msgReplyIncPB;
   wlist[2] = msgReplyAllPB;
   wlist[3] = msgReplyAllIncPB;
   XtManageChildren(wlist, 4);	// msgReplyPD children

//
// Create msgReadPD hierarchy
//
// msgReadPD
//    PushButton	   msgReadPB
//    CascadeButton	   msgNextCB
//    PulldownMenu	   msgNextPD
//       PushButton	      msgNextPB
//       PushButton	      msgNextUnreadPB
//       PushButton	      msgNextSenderPB
//       PushButton	      msgNextSubjectPB
//    CascadeButton	   msgPrevCB
//    PulldownMenu	   msgPrevPD
//       PushButton	      msgPrevPB
//       PushButton	      msgPrevUnreadPB
//       PushButton	      msgPrevSenderPB
//       PushButton	      msgPrevSubjectPB
//
   Widget msgReadPD = XmCreatePulldownMenu(msgPD, "msgReadPD", 0,0);
   msgReadPB        = XmCreatePushButton(msgReadPD, "msgReadPB", 0,0);
   Widget msgNextCB = XmCreateCascadeButton(msgReadPD, "msgNextCB", 0,0);
   Widget msgPrevCB = XmCreateCascadeButton(msgReadPD, "msgPrevCB", 0,0);

   wlist[0] = msgReadPB;
   wlist[1] = msgNextCB;
   wlist[2] = msgPrevCB;
   XtManageChildren(wlist, 3);	// msgReadPD children

   Widget msgNextPD = XmCreatePulldownMenu(msgReadPD, "msgNextPD", 0,0);
   msgNextPB        = XmCreatePushButton(msgNextPD, "msgNextPB", 0,0);
   msgNextUnreadPB  = XmCreatePushButton(msgNextPD, "msgNextUnreadPB", 0,0);
   msgNextSenderPB  = XmCreatePushButton(msgNextPD, "msgNextSenderPB", 0,0);
   msgNextSubjectPB = XmCreatePushButton(msgNextPD, "msgNextSubjectPB", 0,0);

   wlist[0] = msgNextPB;
   wlist[1] = msgNextUnreadPB;
   wlist[2] = msgNextSenderPB;
   wlist[3] = msgNextSubjectPB;
   XtManageChildren(wlist, 4);	// msgNextPD children

   Widget msgPrevPD = XmCreatePulldownMenu(msgReadPD, "msgPrevPD", 0,0);
   msgPrevPB        = XmCreatePushButton(msgPrevPD, "msgPrevPB", 0,0);
   msgPrevUnreadPB  = XmCreatePushButton(msgPrevPD, "msgPrevUnreadPB", 0,0);
   msgPrevSenderPB  = XmCreatePushButton(msgPrevPD, "msgPrevSenderPB", 0,0);
   msgPrevSubjectPB = XmCreatePushButton(msgPrevPD, "msgPrevSubjectPB", 0,0);

   wlist[0] = msgPrevPB;
   wlist[1] = msgPrevUnreadPB;
   wlist[2] = msgPrevSenderPB;
   wlist[3] = msgPrevSubjectPB;
   XtManageChildren(wlist, 4);	// msgPrevPD children

//
// Create msgSavePD hierarchy
//
// msgSavePD
//    PushButton	   msgSavePB
//    PushButton	   msgSavePatPB
//    CascadeButton	   msgSaveRecentCB
//    CascadeButton	   msgSaveQuickCB
//    PushButton	   msgSaveSelPB
//    PushButton	   msgSaveToPB
//    PushButton	   msgSaveToFilePB
//
   Widget msgSavePD = XmCreatePulldownMenu(msgPD, "msgSavePD", 0,0);
   msgSavePB       = XmCreatePushButton   (msgSavePD, "msgSavePB",       0,0);
   msgSavePatPB    = XmCreatePushButton   (msgSavePD, "msgSavePatPB",    0,0);
   msgSaveRecentCB = XmCreateCascadeButton(msgSavePD, "msgSaveRecentCB", 0,0);
   msgSaveRecentPD = XmCreatePulldownMenu (msgSavePD, "msgSaveRecentPD", 0,0);

//
// Add a data structure and a pulldown menu for the quick function
//
   QuickInfoT	*cbData = new QuickInfoT;
   if ( (!ishApp->appPrefs->usingImap) ||
	(!ishApp->folderPrefs->UsingLocal()) )
       cbData->dir = ishApp->appPrefs->FolderDir();
   else
       cbData->dir = "";
   cbData->menu = XmCreatePulldownMenu(msgSavePD, "quickPD", 0,0);
   cbData->menuTime = 0;

   args.Reset();
   args.UserData(cbData);
   args.SubMenuId(cbData->menu);
   msgSaveQuickCB = XmCreateCascadeButton(msgSavePD, "msgSaveQuickCB", ARGS);

   msgSaveSelPB    = XmCreatePushButton(msgSavePD, "msgSaveSelPB",    0,0);
   msgSaveToPB     = XmCreatePushButton(msgSavePD, "msgSaveToPB",     0,0);
   msgSaveToFilePB = XmCreatePushButton(msgSavePD, "msgSaveToFilePB", 0,0);

   wcount = 0;
   wlist[wcount++] = msgSavePB;
   wlist[wcount++] = msgSavePatPB;
   wlist[wcount++] = msgSaveRecentCB;
   wlist[wcount++] = msgSaveQuickCB;
   wlist[wcount++] = msgSaveSelPB;
   wlist[wcount++] = msgSaveToPB;
   wlist[wcount++] = msgSaveToFilePB;
   XtManageChildren(wlist, wcount);	// msgSavePD children

//
// Create msgUndelPD hierarchy
//
// msgUndelPD
//    PushButton	   msgUndelLastPB
//    PushButton	   msgUndelSelPB
//    PushButton	   msgUndelListPB
//
   Widget msgUndelPD = XmCreatePulldownMenu(msgPD, "msgUndelPD", 0,0);
   msgUndelLastPB = XmCreatePushButton(msgUndelPD, "msgUndelLastPB", 0,0);
   msgUndelSelPB  = XmCreatePushButton(msgUndelPD, "msgUndelSelPB",  0,0);
   msgUndelListPB = XmCreatePushButton(msgUndelPD, "msgUndelListPB", 0,0);

   wlist[0] = msgUndelLastPB;
   wlist[1] = (ishApp->appPrefs->hideDeleted) ? msgUndelListPB : msgUndelSelPB;
   XtManageChildren(wlist, 2);	// msgUndelPD children

//
// Create msgStatPD hierarchy
//
// msgStatPD
//    PushButton	   msgMarkReadPB
//    PushButton	   msgMarkUnreadPB
//    PushButton	   msgMarkNewPB
//    CascadeButton	   msgStatSetCB
//    PulldownMenu	   msgStatSetPD
//       PushButton	      msgSetSavedPB
//       PushButton	      msgSetRepliedPB
//       PushButton	      msgSetForwardedPB
//       PushButton	      msgSetResentPB
//       PushButton	      msgSetPrintedPB
//       PushButton	      msgSetFilteredPB
//    CascadeButton	   msgStatClearCB
//    PulldownMenu	   msgStatClearPD
//       PushButton	      msgClearAllPB
//       PushButton	      msgClearSavedPB
//       PushButton	      msgClearRepliedPB
//       PushButton	      msgClearForwardedPB
//       PushButton	      msgClearResentPB
//       PushButton	      msgClearPrintedPB
//       PushButton	      msgClearFilteredPB
//
   Widget
      msgStatPD       = XmCreatePulldownMenu (msgPD,     "msgStatPD",      0,0),
      msgMarkReadPB   = XmCreatePushButton   (msgStatPD, "msgMarkReadPB",  0,0),
      msgMarkUnreadPB = XmCreatePushButton   (msgStatPD, "msgMarkUnreadPB",0,0),
      msgMarkNewPB    = XmCreatePushButton   (msgStatPD, "msgMarkNewPB",   0,0),
      msgStatSetCB    = XmCreateCascadeButton(msgStatPD, "msgStatSetCB",   0,0),
      msgStatClearCB  = XmCreateCascadeButton(msgStatPD, "msgStatClearCB", 0,0);

   wlist[0] = msgMarkReadPB;
   wlist[1] = msgMarkUnreadPB;
   wlist[2] = msgMarkNewPB;
   wlist[3] = msgStatSetCB;
   wlist[4] = msgStatClearCB;
   XtManageChildren(wlist, 5);	// msgStatPD children

   Widget
      msgStatSetPD    = XmCreatePulldownMenu(msgStatPD, "msgStatSetPD", 0,0),
      msgSetSavedPB   = XmCreatePushButton(msgStatSetPD, "msgSetSavedPB", 0,0),
      msgSetRepliedPB = XmCreatePushButton(msgStatSetPD, "msgSetRepliedPB",0,0),
      msgSetForwardedPB =
	 XmCreatePushButton(msgStatSetPD, "msgSetForwardedPB", 0,0),
      msgSetResentPB  = XmCreatePushButton(msgStatSetPD, "msgSetResentPB", 0,0),
      msgSetPrintedPB = XmCreatePushButton(msgStatSetPD, "msgSetPrintedPB",0,0),
      msgSetFilteredPB =
         XmCreatePushButton(msgStatSetPD, "msgSetFilteredPB", 0,0);

   wlist[0] = msgSetSavedPB;
   wlist[1] = msgSetRepliedPB;
   wlist[2] = msgSetForwardedPB;
   wlist[3] = msgSetResentPB;
   wlist[4] = msgSetPrintedPB;
   wlist[5] = msgSetFilteredPB;
   XtManageChildren(wlist, 6);	// msgStatSetPD children

   Widget
      msgStatClearPD = XmCreatePulldownMenu(msgStatPD, "msgStatClearPD", 0,0),
      msgClearAllPB = XmCreatePushButton(msgStatClearPD, "msgClearAllPB", 0,0),
      msgClearSavedPB =
         XmCreatePushButton(msgStatClearPD, "msgClearSavedPB", 0,0),
      msgClearRepliedPB =
         XmCreatePushButton(msgStatClearPD, "msgClearRepliedPB", 0,0),
      msgClearForwardedPB =
	 XmCreatePushButton(msgStatClearPD, "msgClearForwardedPB", 0,0),
      msgClearResentPB =
         XmCreatePushButton(msgStatClearPD, "msgClearResentPB", 0,0),
      msgClearPrintedPB =
         XmCreatePushButton(msgStatClearPD, "msgClearPrintedPB", 0,0),
      msgClearFilteredPB =
         XmCreatePushButton(msgStatClearPD, "msgClearFilteredPB", 0,0);

   wlist[0] = msgClearAllPB;
   wlist[1] = msgClearSavedPB;
   wlist[2] = msgClearRepliedPB;
   wlist[3] = msgClearForwardedPB;
   wlist[4] = msgClearResentPB;
   wlist[5] = msgClearPrintedPB;
   wlist[6] = msgClearFilteredPB;
   XtManageChildren(wlist, 7);	// msgStatClearPD children

//
// Attach pulldown menus
//
   XtVaSetValues(msgReplyCB,        XmNsubMenuId, msgReplyPD,        NULL);
   XtVaSetValues(msgReadCB,         XmNsubMenuId, msgReadPD,         NULL);
   XtVaSetValues(msgNextCB,         XmNsubMenuId, msgNextPD,         NULL);
   XtVaSetValues(msgPrevCB,         XmNsubMenuId, msgPrevPD,         NULL);
   XtVaSetValues(msgSaveCB,         XmNsubMenuId, msgSavePD,         NULL);
   XtVaSetValues(msgSaveRecentCB,   XmNsubMenuId, msgSaveRecentPD,   NULL);
   XtVaSetValues(msgUndelCB,        XmNsubMenuId, msgUndelPD,        NULL);
   XtVaSetValues(msgStatCB,         XmNsubMenuId, msgStatPD,         NULL);
   XtVaSetValues(msgStatSetCB,      XmNsubMenuId, msgStatSetPD,      NULL);
   XtVaSetValues(msgStatClearCB,    XmNsubMenuId, msgStatClearPD,    NULL);

//
// Set initial sensitivities
//
   XtSetSensitive(msgReplyPB,       False);
   XtSetSensitive(msgReplyAllPB,    False);
   XtSetSensitive(msgReplyIncPB,    False);
   XtSetSensitive(msgReplyAllIncPB, False);
   XtSetSensitive(msgForwardPB,     False);
   XtSetSensitive(msgForward822PB,  False);
   XtSetSensitive(msgResendPB,      False);
   XtSetSensitive(msgReadPB,        False);
   XtSetSensitive(msgNextPB,        False);
   XtSetSensitive(msgNextUnreadPB,  False);
   XtSetSensitive(msgNextSenderPB,  False);
   XtSetSensitive(msgNextSubjectPB, False);
   XtSetSensitive(msgPrevPB,        False);
   XtSetSensitive(msgPrevUnreadPB,  False);
   XtSetSensitive(msgPrevSenderPB,  False);
   XtSetSensitive(msgPrevSubjectPB, False);
   XtSetSensitive(msgSavePB,        False);
   XtSetSensitive(msgSavePatPB,     False);
   XtSetSensitive(msgSaveSelPB,     False);
   XtSetSensitive(msgSaveToPB,      False);
   XtSetSensitive(msgSaveToFilePB,  False);
   XtSetSensitive(msgSaveRecentCB,  False);
   XtSetSensitive(msgSaveQuickCB,   False);
   XtSetSensitive(msgPrintPB,       False);
   XtSetSensitive(msgPipePB,        False);
   XtSetSensitive(msgDelPB,         False);
   XtSetSensitive(msgUndelLastPB,   False);
   XtSetSensitive(msgUndelSelPB,    False);
   XtSetSensitive(msgUndelListPB,   False);
   XtSetSensitive(msgStatCB,        False);
   XtSetSensitive(msgSelPB,         False);
   XtSetSensitive(msgDeselPB,       False);
   XtSetSensitive(msgFindPB,        False);

   //AddCascading(msgSaveRecentCB, PrepareRecentMenu,     this);
   AddCascading(msgSaveQuickCB,  PrepareSaveQuickMenu,  this);

   AddActivate(msgComposePB,        DoMsgCompose,        this);
   AddActivate(msgReplyPB,          DoMsgReply,          this);
   AddActivate(msgReplyIncPB,       DoMsgReplyInc,       this);
   AddActivate(msgReplyAllPB,       DoMsgReplyAll,       this);
   AddActivate(msgReplyAllIncPB,    DoMsgReplyAllInc,    this);
   AddActivate(msgForwardPB,        DoMsgForward,        this);
   AddActivate(msgForward822PB,     DoMsgForward822,     this);
   AddActivate(msgResendPB,         DoMsgResend,         this);
   AddActivate(msgReadPB,           DoMsgRead,           this);
   AddActivate(msgNextPB,           DoMsgNext,           this);
   AddActivate(msgNextUnreadPB,     DoMsgNextUnread,     this);
   AddActivate(msgNextSenderPB,     DoMsgNextSender,     this);
   AddActivate(msgNextSubjectPB,    DoMsgNextSubject,    this);
   AddActivate(msgPrevPB,           DoMsgPrev,           this);
   AddActivate(msgPrevUnreadPB,     DoMsgPrevUnread,     this);
   AddActivate(msgPrevSenderPB,     DoMsgPrevSender,     this);
   AddActivate(msgPrevSubjectPB,    DoMsgPrevSubject,    this);
   AddActivate(msgSavePB,           DoMsgSave,           this);
   AddActivate(msgSavePatPB,        DoMsgSavePat,        this);
   AddActivate(msgSaveSelPB,        DoMsgSaveSel,        this);
   AddActivate(msgSaveToPB,         DoMsgSaveTo,         this);
   AddActivate(msgSaveToFilePB,     DoMsgSaveToFile,     this);
   AddActivate(msgPrintPB,          DoMsgPrint,          this);
   AddActivate(msgPipePB,           DoMsgPipe,           this);
   AddActivate(msgDelPB,            DoMsgDel,            this);
   AddActivate(msgUndelLastPB,      DoMsgUndelLast,      this);
   AddActivate(msgUndelSelPB,       DoMsgUndelSel,       this);
   AddActivate(msgUndelListPB,      DoMsgUndelList,      this);
   AddActivate(msgMarkReadPB,       DoMsgMarkRead,       this);
   AddActivate(msgMarkUnreadPB,     DoMsgMarkUnread,     this);
   AddActivate(msgMarkNewPB,        DoMsgMarkNew,        this);
   AddActivate(msgSetSavedPB,       DoMsgSetSaved,       this);
   AddActivate(msgSetRepliedPB,     DoMsgSetReplied,     this);
   AddActivate(msgSetForwardedPB,   DoMsgSetForwarded,   this);
   AddActivate(msgSetResentPB,      DoMsgSetResent,      this);
   AddActivate(msgSetPrintedPB,     DoMsgSetPrinted,     this);
   AddActivate(msgSetFilteredPB,    DoMsgSetFiltered,    this);
   AddActivate(msgClearAllPB,       DoMsgClearAll,       this);
   AddActivate(msgClearSavedPB,     DoMsgClearSaved,     this);
   AddActivate(msgClearRepliedPB,   DoMsgClearReplied,   this);
   AddActivate(msgClearForwardedPB, DoMsgClearForwarded, this);
   AddActivate(msgClearResentPB,    DoMsgClearResent,    this);
   AddActivate(msgClearPrintedPB,   DoMsgClearPrinted,   this);
   AddActivate(msgClearFilteredPB,  DoMsgClearFiltered,  this);
   AddActivate(msgSelPB,            DoMsgSel,            this);
   AddActivate(msgDeselPB,          DoMsgDesel,          this);
   AddActivate(msgFindPB,           DoMsgFind,           this);

} // End BuildMsgMenu

/*---------------------------------------------------------------
 *  Method to build option menu hierarchy
 */

void
MainWinP::BuildOptMenu()
{
   WArgList	args;
   Cardinal	wcount;
   Widget	wlist[24];

//
// Create cascade button and pulldown menu
//
   Widget	optCB = XmCreateCascadeButton(pub->menuBar, "optCB", 0,0);
   Widget	optPD = XmCreatePulldownMenu (pub->menuBar, "optPD", 0,0);
   XtVaSetValues(optCB, XmNsubMenuId, optPD, NULL);
   XtManageChild(optCB);

//
// Create optPD hierarchy
//
// optPD
//    PushButton	optPrefPB
//    PushButton	optReadPB
//    PushButton	optSendPB
//    PushButton	optMailPB
//    PushButton	optSigPB
//    PushButton	optReplyPB
//    Separator		optSep1
//    PushButton	optConfirmPB
//    PushButton	optHeadPB
//    PushButton	optAliasPB
//    PushButton	optGroupAliasPB
//    PushButton	optAlertPB
//    PushButton	optIconPB
//    PushButton	optSavePB
//    PushButton	optAutoFilePB
//    Separator		optSep2
//    PushButton	optFontPB
//    PushButton	optButtPB
//    PushButton	optSummPB
//    PushButton	optFolderPB
//    Separator		optSep3
//    PushButton	optSortPB
//
   Widget	optPrefPB    = XmCreatePushButton(optPD, "optPrefPB",    0,0);
   Widget	optReadPB    = XmCreatePushButton(optPD, "optReadPB",    0,0);
   Widget	optSendPB    = XmCreatePushButton(optPD, "optSendPB",    0,0);
   Widget	optMailPB    = XmCreatePushButton(optPD, "optMailPB",    0,0);
   Widget	optSigPB     = XmCreatePushButton(optPD, "optSigPB",     0,0);
   Widget	optReplyPB   = XmCreatePushButton(optPD, "optReplyPB",   0,0);
   Widget	optSep1      = XmCreateSeparator (optPD, "optSep1",      0,0);
   Widget	optConfirmPB = XmCreatePushButton(optPD, "optConfirmPB", 0,0);
   Widget	optHeadPB    = XmCreatePushButton(optPD, "optHeadPB",    0,0);
   Widget	optAliasPB   = XmCreatePushButton(optPD, "optAliasPB",   0,0);
   Widget	optGroupAliasPB = NULL;
   if ( access(ishApp->aliasPrefs->GroupMailrcFile(), R_OK|W_OK) == 0 )
      optGroupAliasPB        = XmCreatePushButton(optPD, "optGroupAliasPB",0,0);
   Widget	optAlertPB   = XmCreatePushButton(optPD, "optAlertPB",   0,0);
   Widget	optIconPB    = XmCreatePushButton(optPD, "optIconPB",    0,0);
   Widget	optSavePB    = XmCreatePushButton(optPD, "optSavePB",    0,0);
   Widget	optAutoFilePB= XmCreatePushButton(optPD, "optAutoFilePB",0,0);
   Widget	optSep2      = XmCreateSeparator (optPD, "optSep2",      0,0);
   Widget	optFontPB    = XmCreatePushButton(optPD, "optFontPB",    0,0);
   Widget	optButtPB    = XmCreatePushButton(optPD, "optButtPB",    0,0);
   Widget	optSummPB    = XmCreatePushButton(optPD, "optSummPB",    0,0);
   Widget	optFolderPB  = XmCreatePushButton(optPD, "optFolderPB",  0,0);
   Widget	optSep3      = XmCreateSeparator (optPD, "optSep3",      0,0);
   Widget	optSortPB    = XmCreatePushButton(optPD, "optSortPB",    0,0);

   wcount = 0;
   wlist[wcount++] = optPrefPB;
   wlist[wcount++] = optReadPB;
   wlist[wcount++] = optSendPB;
   wlist[wcount++] = optMailPB;
   wlist[wcount++] = optSigPB;
   wlist[wcount++] = optReplyPB;
   wlist[wcount++] = optSep1;
   wlist[wcount++] = optConfirmPB;
   wlist[wcount++] = optHeadPB;
   wlist[wcount++] = optAliasPB;
   if ( optGroupAliasPB ) wlist[wcount++] = optGroupAliasPB;
   wlist[wcount++] = optAlertPB;
   wlist[wcount++] = optIconPB;
   wlist[wcount++] = optSavePB;
   wlist[wcount++] = optAutoFilePB;
   wlist[wcount++] = optSep2;
   wlist[wcount++] = optFontPB;
   wlist[wcount++] = optButtPB;
   wlist[wcount++] = optSummPB;
   wlist[wcount++] = optFolderPB;
   wlist[wcount++] = optSep3;
   wlist[wcount++] = optSortPB;
   XtManageChildren(wlist, wcount);

   AddActivate(optPrefPB,     DoOptApp,		this);
   AddActivate(optReadPB,     DoOptRead,	this);
   AddActivate(optSendPB,     DoOptSend,	this);
   AddActivate(optMailPB,     DoOptMail,	this);
   AddActivate(optSigPB,      DoOptSig,		this);
   AddActivate(optReplyPB,    DoOptReply,	this);
   AddActivate(optConfirmPB,  DoOptConf,	this);
   AddActivate(optHeadPB,     DoOptHead,	this);
   AddActivate(optAliasPB,    DoOptAlias,	this);
   if ( optGroupAliasPB )
      AddActivate(optGroupAliasPB, DoOptGroup,	this);
   AddActivate(optAlertPB,    DoOptAlert,	this);
   AddActivate(optIconPB,     DoOptIcon,	this);
   AddActivate(optSavePB,     DoOptSave,	this);
   AddActivate(optAutoFilePB, DoOptAuto,	this);
   AddActivate(optFontPB,     DoOptFont,	this);
   AddActivate(optButtPB,     DoOptButt,	this);
   AddActivate(optSummPB,     DoOptSumm,	this);
   AddActivate(optFolderPB,   DoOptFolder,	this);
   AddActivate(optSortPB,     DoOptSort,	this);

} // End BuildOptMenu

/*---------------------------------------------------------------
 *  Method to build help menu hierarchy
 */

void
MainWinP::BuildHelpMenu()
{
   pub->AddHelpMenu();

//
// Add buttons for getting to the user's guide and sending comments
//
   if ( pub->helpPD) {
      Widget wlist[4];
      
      Widget helpSep2      = XmCreateSeparator (pub->helpPD, "helpSep2",    0,0);
      Widget helpGuidePB   = XmCreatePushButton(pub->helpPD, "helpGuidePB", 0,0);
      Widget helpSep3      = XmCreateSeparator (pub->helpPD, "helpSep3",    0,0);
      Widget helpCommentPB = XmCreatePushButton(pub->helpPD, "helpCommentPB", 0,0);
      
      wlist[0] = helpSep2;
      wlist[1] = helpGuidePB;
      wlist[2] = helpSep3;
      wlist[3] = helpCommentPB;
      XtManageChildren(wlist, 4);
   
      AddActivate(helpGuidePB, DoHelpGuide, this);
      AddActivate(helpCommentPB, DoHelpComment, this);
   }

} // End BuildHelpMenu

/*---------------------------------------------------------------
 *  Method to build the widgets in the main window
 */

void
MainWinP::BuildWidgets()
{
   WArgList	args;
   Widget	wlist[4];

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
   Widget	panedWin = XmCreatePanedWindow(pub->appForm, "panedWin", ARGS);

//
// Create panedWin hierarchy
//
// panedWin
//    Form	folderForm
//    TBoxC	msgTBox
//
   args.Reset();
   args.AllowResize(True);
   Widget	folderForm = XmCreateForm(panedWin, "folderForm", 0,0);

//
// Create folderForm hierarchy
//
// folderForm
//    Label		folderTitle
//    VBoxC		folderVBox
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   Widget	folderTitle = XmCreateLabel(folderForm, "folderTitle", ARGS);

   folderVBox = new VBoxC(folderForm, "folderBox");

   args.Reset();
   args.TopAttachment(XmATTACH_WIDGET, folderTitle);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   XtSetValues(*folderVBox, ARGS);

   folderVBox->DisablePopupMenu();
   folderVBox->HideStatus();
   folderVBox->SetSorted(ishApp->folderPrefs->SortFolders());

   folderVBox->AddSelectChangeCallback((CallbackFn *)ChangeFolderSelection,
   					this);
   folderVBox->AddDropCallback((CallbackFn *)FolderDrop, this);
   folderVBox->SetCompareFunction((CompareFn)FolderCompare);

//
// Register folder box as a drop site for message icons
//
   Atom impAtoms[1];
   impAtoms[0] = ishApp->msgAtom;
   folderVBox->DisableDrag();
   folderVBox->EnableDrop();
   folderVBox->SetDropAtoms(impAtoms, 1);

   wlist[0] = folderTitle;
   wlist[1] = *folderVBox;
   XtManageChildren(wlist, 2);	// folderForm children

//
// Create message task box
//
   msgTBox = new TBoxC(panedWin, "msgBox");
   msgVBox = &msgTBox->VBox();

#if 0
   args.Reset();
   args.AllowResize(True);
   XtSetValues(*msgTBox, ARGS);
#endif

   msgVBox->DisablePopupMenu();
   msgVBox->HideStatus();
   msgVBox->EnableDrag();
   msgVBox->DisableDrop();
   msgVBox->SetSorted(True);

   msgVBox->AddSelectChangeCallback((CallbackFn *)ChangeMsgSelection, this);
   msgVBox->SetCompareFunction((CompareFn)MsgItemC::MsgItemCompare);
   msgVBox->AddDragCallback((CallbackFn *)MsgDrag, this);

//
// Add field view to msgTBox
//
   fieldView = new FieldViewC(msgVBox);
   viewType = msgVBox->AddView(*fieldView);
   if ( ishApp->sumPrefs->showPixmaps) fieldView->ShowPixmaps();
   else				       fieldView->HidePixmaps();

   wlist[0] = folderForm;
   wlist[1] = *msgTBox;
   XtManageChildren(wlist, 2);	// panedWin children

   XtManageChild(panedWin);	// appForm children

   BuildFolderPopupMenu();
   BuildMsgPopupMenu();
   BuildRecentFolderMenus();
   ishApp->appPrefs->AddRecentChangeCallback((CallbackFn*)RecentListChanged,
   					     this);

   pub->ShowInfoMsg();
   pub->HandleHelp();

} // End BuildWidgets

/*---------------------------------------------------------------
 *  Method to build the widgets in the message popup menu
 */

void
MainWinP::BuildMsgPopupMenu()
{
   WArgList	args;
   Cardinal	wcount;
   Widget	wlist[16];

//
// Create msgPU hierarchy
//
// msgPU
//    Label		msgPULabel
//    Separator		msgPUSep1
//    PushButton	msgPUReadPB
//    CascadeButton	msgPUSaveCB
//    PulldownMenu	msgPUSavePD
//       PushButton	   msgPUSavePB
//       PushButton	   msgPUSavePatPB
//       PushButton	   msgPUSaveRecentCB
//       PushButton	   msgPUSaveQuickCB
//       PushButton	   msgPUSaveSelPB
//       PushButton	   msgPUSaveToPB
//       PushButton	   msgPUSaveToFilePB
//    PushButton	msgPUPrintPB
//    PushButton	msgPUPipePB
//    Separator		msgPUSep2
//    CascadeButton	msgPUReplyCB
//    PulldownMenu	msgPUReplyPD
//       PushButton	   msgPUReplyPB
//       PushButton	   msgPUReplyIncPB
//       PushButton	   msgPUReplyAllPB
//       PushButton	   msgPUReplyAllIncPB
//    PushButton	msgPUForwardPB
//    PushButton	msgPUForward822PB
//    PushButton	msgPUResendPB
//    Separator		msgPUSep3
//    PushButton	msgPUDelPB
//    PushButton	msgPUUndelPB
//    CascadeButton	msgPUStatCB
//
   msgPU = XmCreatePopupMenu(msgVBox->ClipWin(), "msgPU", 0,0);

   msgPULabel          = XmCreateLabel        (msgPU, "msgPULabel",        0,0);
   msgPUReadPB         = XmCreatePushButton   (msgPU, "msgPUReadPB",       0,0);
   Widget msgPUSaveCB  = XmCreateCascadeButton(msgPU, "msgPUSaveCB",       0,0);
   msgPUPrintPB        = XmCreatePushButton   (msgPU, "msgPUPrintPB",      0,0);
   msgPUPipePB         = XmCreatePushButton   (msgPU, "msgPUPipePB",       0,0);
   Widget msgPUSep1    = XmCreateSeparator    (msgPU, "msgPUSep1",         0,0);
   Widget msgPUReplyCB = XmCreateCascadeButton(msgPU, "msgPUReplyCB",      0,0);
   msgPUForwardPB      = XmCreatePushButton   (msgPU, "msgPUForwardPB",    0,0);
   msgPUForward822PB   = XmCreatePushButton   (msgPU, "msgPUForward822PB", 0,0);
   msgPUResendPB       = XmCreatePushButton   (msgPU, "msgPUResendPB",     0,0);
   Widget msgPUSep2    = XmCreateSeparator    (msgPU, "msgPUSep2",         0,0);
   msgPUDelPB          = XmCreatePushButton   (msgPU, "msgPUDelPB",        0,0);
   msgPUUndelPB        = XmCreatePushButton   (msgPU, "msgPUUndelPB",      0,0);
   Widget msgPUStatCB  = XmCreateCascadeButton(msgPU, "msgPUStatCB",       0,0);

   wcount = 0;
   wlist[wcount++] = msgPULabel;
   wlist[wcount++] = msgPUReadPB;
   wlist[wcount++] = msgPUSaveCB;
   wlist[wcount++] = msgPUPrintPB;
   wlist[wcount++] = msgPUPipePB;
   wlist[wcount++] = msgPUSep1;
   wlist[wcount++] = msgPUReplyCB;
   wlist[wcount++] = msgPUForwardPB;
   wlist[wcount++] = msgPUForward822PB;
   wlist[wcount++] = msgPUResendPB;
   wlist[wcount++] = msgPUSep2;
   wlist[wcount++] = msgPUDelPB;
   wlist[wcount++] = msgPUUndelPB;
   wlist[wcount++] = msgPUStatCB;
   XtManageChildren(wlist, wcount);

   XtSetSensitive(msgPUUndelPB, False);

   Widget msgPUReplyPD = XmCreatePulldownMenu(msgPU, "msgPUReplyPD", 0,0);
   msgPUReplyPB    = XmCreatePushButton(msgPUReplyPD, "msgPUReplyPB",      0,0);
   msgPUReplyIncPB = XmCreatePushButton(msgPUReplyPD, "msgPUReplyIncPB",   0,0);
   msgPUReplyAllPB = XmCreatePushButton(msgPUReplyPD, "msgPUReplyAllPB",   0,0);
   msgPUReplyAllIncPB
   		   = XmCreatePushButton(msgPUReplyPD, "msgPUReplyAllIncPB",0,0);

   wlist[0] = msgPUReplyPB;
   wlist[1] = msgPUReplyIncPB;
   wlist[2] = msgPUReplyAllPB;
   wlist[3] = msgPUReplyAllIncPB;
   XtManageChildren(wlist, 4);	// msgPUReplyPD children

   Widget msgPUSavePD = XmCreatePulldownMenu(msgPU, "msgPUSavePD", 0,0);
   msgPUSavePB       = XmCreatePushButton(msgPUSavePD, "msgPUSavePB",      0,0);
   msgPUSavePatPB    = XmCreatePushButton(msgPUSavePD, "msgPUSavePatPB",   0,0);
   msgPUSaveRecentCB =
      XmCreateCascadeButton(msgPUSavePD, "msgPUSaveRecentCB",0,0);
   msgPUSaveRecentPD =
      XmCreatePulldownMenu(msgPUSavePD, "msgPUSaveRecentPD", 0,0);

//
// Add a data structure and a pulldown menu for the quick function
//
   QuickInfoT	*cbData = new QuickInfoT;
   if ( (!ishApp->appPrefs->usingImap) ||
	(!ishApp->folderPrefs->UsingLocal()) )
       cbData->dir = ishApp->appPrefs->FolderDir();
   else
       cbData->dir = "";
   cbData->menu = XmCreatePulldownMenu(msgPUSavePD, "quickPD", 0,0);
   cbData->menuTime = 0;

   args.Reset();
   args.UserData(cbData);
   args.SubMenuId(cbData->menu);
   msgPUSaveQuickCB =
      XmCreateCascadeButton(msgPUSavePD, "msgPUSaveQuickCB", ARGS);

   msgPUSaveSelPB    = XmCreatePushButton(msgPUSavePD, "msgPUSaveSelPB",   0,0);
   msgPUSaveToPB     = XmCreatePushButton(msgPUSavePD, "msgPUSaveToPB",    0,0);
   msgPUSaveToFilePB = XmCreatePushButton(msgPUSavePD, "msgPUSaveToFilePB",0,0);

   wcount = 0;
   wlist[wcount++] = msgPUSavePB;
   wlist[wcount++] = msgPUSavePatPB;
   wlist[wcount++] = msgPUSaveRecentCB;
   wlist[wcount++] = msgPUSaveQuickCB;
   wlist[wcount++] = msgPUSaveSelPB;
   wlist[wcount++] = msgPUSaveToPB;
   wlist[wcount++] = msgPUSaveToFilePB;
   XtManageChildren(wlist, wcount);	// msgPUSavePD children

//
// Create msgPUStatPD hierarchy
//
// msgPUStatPD
//    PushButton	   msgPUMarkReadPB
//    PushButton	   msgPUMarkUnreadPB
//    PushButton	   msgPUMarkNewPB
//    CascadeButton	   msgPUStatSetCB
//    PulldownMenu	   msgPUStatSetPD
//       PushButton	      msgPUSetSavedPB
//       PushButton	      msgPUSetRepliedPB
//       PushButton	      msgPUSetForwardedPB
//       PushButton	      msgPUSetResentPB
//       PushButton	      msgPUSetPrintedPB
//       PushButton	      msgPUSetFilteredPB
//    CascadeButton	   msgPUStatClearCB
//    PulldownMenu	   msgPUStatClearPD
//       PushButton	      msgPUClearAllPB
//       PushButton	      msgPUClearSavedPB
//       PushButton	      msgPUClearRepliedPB
//       PushButton	      msgPUClearForwardedPB
//       PushButton	      msgPUClearResentPB
//       PushButton	      msgPUClearPrintedPB
//       PushButton	      msgPUClearFilteredPB
//
   Widget
      msgPUStatPD = XmCreatePulldownMenu(msgPU, "msgPUStatPD", 0,0),
      msgPUMarkReadPB = XmCreatePushButton(msgPUStatPD, "msgPUMarkReadPB", 0,0),
      msgPUMarkUnreadPB = 
         XmCreatePushButton(msgPUStatPD, "msgPUMarkUnreadPB",0,0),
      msgPUMarkNewPB = XmCreatePushButton(msgPUStatPD, "msgPUMarkNewPB", 0,0),
      msgPUStatSetCB = XmCreateCascadeButton(msgPUStatPD, "msgPUStatSetCB",0,0),
      msgPUStatClearCB =
         XmCreateCascadeButton(msgPUStatPD, "msgPUStatClearCB", 0,0);

   wlist[0] = msgPUMarkReadPB;
   wlist[1] = msgPUMarkUnreadPB;
   wlist[2] = msgPUMarkNewPB;
   wlist[3] = msgPUStatSetCB;
   wlist[4] = msgPUStatClearCB;
   XtManageChildren(wlist, 5);	// msgPUStatPD children

   Widget
      msgPUStatSetPD = XmCreatePulldownMenu(msgPUStatPD, "msgPUStatSetPD", 0,0),
      msgPUSetSavedPB =
         XmCreatePushButton(msgPUStatSetPD, "msgPUSetSavedPB", 0,0),
      msgPUSetRepliedPB =
         XmCreatePushButton(msgPUStatSetPD, "msgPUSetRepliedPB",0,0),
      msgPUSetForwardedPB =
	 XmCreatePushButton(msgPUStatSetPD, "msgPUSetForwardedPB", 0,0),
      msgPUSetResentPB =
         XmCreatePushButton(msgPUStatSetPD, "msgPUSetResentPB", 0,0),
      msgPUSetPrintedPB =
         XmCreatePushButton(msgPUStatSetPD, "msgPUSetPrintedPB",0,0),
      msgPUSetFilteredPB =
         XmCreatePushButton(msgPUStatSetPD, "msgPUSetFilteredPB", 0,0);

   wlist[0] = msgPUSetSavedPB;
   wlist[1] = msgPUSetRepliedPB;
   wlist[2] = msgPUSetForwardedPB;
   wlist[3] = msgPUSetResentPB;
   wlist[4] = msgPUSetPrintedPB;
   wlist[5] = msgPUSetFilteredPB;
   XtManageChildren(wlist, 6);	// msgPUStatSetPD children

   Widget
      msgPUStatClearPD =
         XmCreatePulldownMenu(msgPUStatPD, "msgPUStatClearPD", 0,0),
      msgPUClearAllPB =
         XmCreatePushButton(msgPUStatClearPD, "msgPUClearAllPB", 0,0),
      msgPUClearSavedPB =
         XmCreatePushButton(msgPUStatClearPD, "msgPUClearSavedPB", 0,0),
      msgPUClearRepliedPB =
         XmCreatePushButton(msgPUStatClearPD, "msgPUClearRepliedPB", 0,0),
      msgPUClearForwardedPB =
	 XmCreatePushButton(msgPUStatClearPD, "msgPUClearForwardedPB", 0,0),
      msgPUClearResentPB =
         XmCreatePushButton(msgPUStatClearPD, "msgPUClearResentPB", 0,0),
      msgPUClearPrintedPB =
         XmCreatePushButton(msgPUStatClearPD, "msgPUClearPrintedPB", 0,0),
      msgPUClearFilteredPB =
         XmCreatePushButton(msgPUStatClearPD, "msgPUClearFilteredPB", 0,0);

   wlist[0] = msgPUClearAllPB;
   wlist[1] = msgPUClearSavedPB;
   wlist[2] = msgPUClearRepliedPB;
   wlist[3] = msgPUClearForwardedPB;
   wlist[4] = msgPUClearResentPB;
   wlist[5] = msgPUClearPrintedPB;
   wlist[6] = msgPUClearFilteredPB;
   XtManageChildren(wlist, 7);	// msgPUStatClearPD children

   XtVaSetValues(msgPUSaveCB,       XmNsubMenuId, msgPUSavePD,       NULL);
   XtVaSetValues(msgPUSaveRecentCB, XmNsubMenuId, msgPUSaveRecentPD, NULL);
   XtVaSetValues(msgPUReplyCB,      XmNsubMenuId, msgPUReplyPD,      NULL);
   XtVaSetValues(msgPUStatCB,       XmNsubMenuId, msgPUStatPD,       NULL);
   XtVaSetValues(msgPUStatSetCB,    XmNsubMenuId, msgPUStatSetPD,    NULL);
   XtVaSetValues(msgPUStatClearCB,  XmNsubMenuId, msgPUStatClearPD,  NULL);

   //AddCascading(msgPUSaveRecentCB, PrepareRecentMenu,      this);
   AddCascading(msgPUSaveQuickCB,  PreparePUSaveQuickMenu, this);

   AddActivate(msgPUReadPB,           DoMsgPURead,           this);
   AddActivate(msgPUSavePB,           DoMsgPUSave,           this);
   AddActivate(msgPUSavePatPB,        DoMsgPUSavePat,        this);
   AddActivate(msgPUSaveSelPB,        DoMsgPUSaveSel,        this);
   AddActivate(msgPUSaveToPB,         DoMsgPUSaveTo,         this);
   AddActivate(msgPUSaveToFilePB,     DoMsgPUSaveToFile,     this);
   AddActivate(msgPUPrintPB,          DoMsgPUPrint,          this);
   AddActivate(msgPUPipePB,           DoMsgPUPipe,           this);
   AddActivate(msgPUReplyPB,          DoMsgPUReply,          this);
   AddActivate(msgPUReplyIncPB,       DoMsgPUReplyInc,       this);
   AddActivate(msgPUReplyAllPB,       DoMsgPUReplyAll,       this);
   AddActivate(msgPUReplyAllIncPB,    DoMsgPUReplyAllInc,    this);
   AddActivate(msgPUForwardPB,        DoMsgPUForward,        this);
   AddActivate(msgPUForward822PB,     DoMsgPUForward822,     this);
   AddActivate(msgPUResendPB,         DoMsgPUResend,         this);
   AddActivate(msgPUDelPB,            DoMsgPUDel,            this);
   AddActivate(msgPUUndelPB,          DoMsgPUUndel,          this);
   AddActivate(msgPUMarkReadPB,       DoMsgPUMarkRead,       this);
   AddActivate(msgPUMarkUnreadPB,     DoMsgPUMarkUnread,     this);
   AddActivate(msgPUMarkNewPB,        DoMsgPUMarkNew,        this);
   AddActivate(msgPUSetSavedPB,       DoMsgPUSetSaved,       this);
   AddActivate(msgPUSetRepliedPB,     DoMsgPUSetReplied,     this);
   AddActivate(msgPUSetForwardedPB,   DoMsgPUSetForwarded,   this);
   AddActivate(msgPUSetResentPB,      DoMsgPUSetResent,      this);
   AddActivate(msgPUSetPrintedPB,     DoMsgPUSetPrinted,     this);
   AddActivate(msgPUSetFilteredPB,    DoMsgPUSetFiltered,    this);
   AddActivate(msgPUClearAllPB,       DoMsgPUClearAll,       this);
   AddActivate(msgPUClearSavedPB,     DoMsgPUClearSaved,     this);
   AddActivate(msgPUClearRepliedPB,   DoMsgPUClearReplied,   this);
   AddActivate(msgPUClearForwardedPB, DoMsgPUClearForwarded, this);
   AddActivate(msgPUClearResentPB,    DoMsgPUClearResent,    this);
   AddActivate(msgPUClearPrintedPB,   DoMsgPUClearPrinted,   this);
   AddActivate(msgPUClearFilteredPB,  DoMsgPUClearFiltered,  this);

} // End BuildMsgPopupMenu

/*---------------------------------------------------------------
 *  Method to build the widgets in the folder popup menu
 */

void
MainWinP::BuildFolderPopupMenu()
{
   Widget	wlist[8];

//
// Create folderPU hierarchy
//
// folderPU
//    Label		folderPULabel
//    Separator		folderPUSep
//    PushButton	folderPUActPB
//    PushButton	folderPUSavePB
//    PushButton	folderPUReadPB
//    PushButton	folderPUClosePB
//    PushButton	folderPUDelPB
//
   folderPU = XmCreatePopupMenu(folderVBox->ClipWin(), "folderPU", 0,0);

   folderPULabel   = XmCreateLabel     (folderPU, "folderPULabel",   0,0);
   folderPUActPB   = XmCreatePushButton(folderPU, "folderPUActPB",   0,0);
   folderPUSavePB  = XmCreatePushButton(folderPU, "folderPUSavePB",  0,0);
   folderPUReadPB  = XmCreatePushButton(folderPU, "folderPUReadPB",  0,0);
   folderPUClosePB = XmCreatePushButton(folderPU, "folderPUClosePB", 0,0);
   folderPUDelPB   = XmCreatePushButton(folderPU, "folderPUDelPB",   0,0);

   wlist[0] = folderPULabel;
   wlist[1] = folderPUActPB;
   wlist[2] = folderPUSavePB;
   wlist[3] = folderPUReadPB;
   wlist[4] = folderPUClosePB;
   wlist[5] = folderPUDelPB;
   XtManageChildren(wlist, 6);

   XtSetSensitive(folderPUSavePB, False);
   XtSetSensitive(folderPUDelPB,  False);

   AddActivate(folderPUActPB,   DoFolderPUAct,   this);
   AddActivate(folderPUSavePB,  DoFolderPUSave,  this);
   AddActivate(folderPUReadPB,  DoFolderPURead,  this);
   AddActivate(folderPUClosePB, DoFolderPUClose, this);
   AddActivate(folderPUDelPB,   DoFolderPUDel,   this);

} // End BuildFolderPopupMenu

/*------------------------------------------------------------------------
 * Method to build the menus of recent folders.
 */

void
MainWinP::BuildRecentFolderMenus()
{
//
// Unmanage existing buttons
//
   WidgetList	wlist;
   Cardinal	wcount;
   XtVaGetValues(msgSaveRecentPD, XmNnumChildren, &wcount, XmNchildren, &wlist,
   		 NULL);
   //if ( wcount > 0 ) XtUnmanageChildren(wlist, wcount);

   WidgetList	pwlist;
   Cardinal	pwcount;
   XtVaGetValues(msgPUSaveRecentPD, XmNnumChildren, &pwcount, XmNchildren,
   		 &pwlist, NULL);
   //if ( pwcount > 0 ) XtUnmanageChildren(pwlist, pwcount);

   WidgetList	fwlist;
   Cardinal	fwcount;
   XtVaGetValues(folderOpenRecentPD, XmNnumChildren, &fwcount, XmNchildren,
   		 &fwlist, NULL);
   //if ( fwcount > 0 ) XtUnmanageChildren(fwlist, fwcount);

//
// Create new buttons
//
   WArgList	args;
   WXmString	wstr;
   u_int	count = ishApp->appPrefs->recentFolders.size();
   int i=0; for (i=0; i<count; i++) {

      StringC	*label = ishApp->appPrefs->recentFolders[i];
      wstr = (char*)*label;
      args.LabelString(wstr);

//
// Create a button in the menu bar message menu.  Re-use one if possible
//
      Widget	pb;
      if ( i < wcount ) {
	 pb = wlist[i];
	 XtSetValues(pb, ARGS);
      }
      else {
	 pb = XmCreatePushButton(msgSaveRecentPD, "recentPB", ARGS);
	 XtAddCallback(pb, XmNactivateCallback,
		       (XtCallbackProc)DoMsgSaveToButton, this);
      }

      XtManageChild(pb);

//
// Create a button in the message popup menu.  Re-use one if possible.
//
      if ( i < pwcount ) {
	 pb = pwlist[i];
	 XtSetValues(pb, ARGS);
      }
      else {
	 pb = XmCreatePushButton(msgPUSaveRecentPD, "recentPB", ARGS);
	 XtAddCallback(pb, XmNactivateCallback,
		       (XtCallbackProc)DoMsgPUSaveToButton, this);
      }

      XtManageChild(pb);

//
// Create a button in the folder menu.  Re-use one if possible.
//
      if ( i < fwcount ) {
	 pb = fwlist[i];
	 XtSetValues(pb, ARGS);
      }
      else {
	 pb = XmCreatePushButton(folderOpenRecentPD, "recentPB", ARGS);
	 XtAddCallback(pb, XmNactivateCallback,
		       (XtCallbackProc)DoFolderOpenButton, this);
      }

      XtManageChild(pb);

   } // End for each button to be added

   pub->EnableButtons();
   recentMenuTime = time(0);

} // End BuildRecentFolderMenus

/*---------------------------------------------------------------
 *  Method to read main window resources
 */

void
MainWinP::ReadResources()
{
//
// Read title strings
//
   systemTitleStr = get_string(*msgTBox, "systemTitleString", "In-Box");
   folderTitleStr = get_string(*msgTBox, "folderTitleString", "Folder: $NAME");
   msgCountStr    = get_string(*msgTBox, "msgCountString", "$COUNT messages");
   newMsgCountStr = get_string(*msgTBox, "newMsgCountString",
   					 "$COUNT new/unread");
   unrMsgCountStr = get_string(*msgTBox, "unreadMsgCountString",
   					 "$COUNT old/unread");
   savMsgCountStr = get_string(*msgTBox, "savedMsgCountString", "$COUNT saved");
   delMsgCountStr = get_string(*msgTBox, "delMsgCountString",
					 "$COUNT marked for deletion");

//
// Create icons.  Use the default colormap rather than the application colormap
//    since these icons are displayed in the window manager, not the app.
//    The application colormap may be different.
//
   if ( debuglev > 0 ) cout <<"Creating icons" NL;
   Screen	*scr = halApp->screen;
   iconFG = get_color(*halApp, "iconForeground", BlackPixelOfScreen(scr));
   iconBG = get_color(*halApp, "iconBackground", WhitePixelOfScreen(scr));

   StringC	pmName = get_string(*halApp, "newMailPixmap", "flagup");
   newMailPM = new PixmapC(pmName, iconFG, iconBG, iconBG, iconFG,
   			   scr, (Window)*halApp, XDefaultColormapOfScreen(scr));

   pmName = get_string(*halApp, "unreadMailPixmap", "flagdown");
   unreadMailPM = new PixmapC(pmName, iconFG, iconBG, iconBG, iconFG,
   			      scr, (Window)*halApp,
			      XDefaultColormapOfScreen(scr));

   pmName = get_string(*halApp, "readMailPixmap", "flagdown");
   readMailPM = new PixmapC(pmName, iconFG, iconBG, iconBG, iconFG,
   			    scr, (Window)*halApp,
			    XDefaultColormapOfScreen(scr));

   pmName = get_string(*halApp, "noNewMailPixmap", "flagdown");
   pmName = get_string(*halApp, "noMailPixmap", pmName);
   noMailPM = new PixmapC(pmName, iconFG, iconBG, iconBG, iconFG,
   			  scr, (Window)*halApp, XDefaultColormapOfScreen(scr));

} // End ReadResources

/*---------------------------------------------------------------
 *  Callback for map of main window.
 */

void
MainWinP::DoMap(void*, MainWinP *This)
{
   Boolean	wasSleeping = ishApp->sleeping;
   ishApp->sleeping = False;

//
// Activate current folder
//
   if ( wasSleeping && This->pub->curFolder ) This->pub->curFolder->Activate();

//
// Force a mail check if we were sleeping.
//
   if ( wasSleeping ) {

      ishApp->CheckForNewMail();

//
// See if there's any auto-filing to be done
//
      if ( ishApp->autoFilePrefs->autoFileOn )
	 This->AutoFileNewMail();
   }

//
// Check for the first time displayed.
//
   static Boolean	firstTime = True;
   if ( firstTime ) {

//
// Display field view
//
      XtVaSetValues(*This->msgTBox, XmNallowResize, True, NULL);
      This->msgVBox->ViewType(This->viewType);
      XtVaSetValues(*This->msgTBox, XmNallowResize, False, NULL);

//
// Set up folder view
//
      if ( ishApp->folderPrefs->ViewType() != FOLDER_VIEW_LARGE )
	 This->folderVBox->ViewType(1);

//
// Scroll to first new or unread message
//
      MsgItemC		*firstNew = NULL;
      MsgItemC		*lastNew  = NULL;
      VItemListC&	list      = This->msgVBox->VisItems();
      unsigned		count      = list.size();
      int i=0; for (i=0; i<count; i++) {

	 MsgItemC	*item = (MsgItemC*)list[i];
	 if ( !item->IsDeleted() && !item->IsRead() ) {
	    if ( !firstNew ) firstNew = item;
	    lastNew = item;
	 }

      } // End for each message

//
// Scroll to the last one, then the first one.  Do this to get as many new
//    messages on the screen as possible, while making sure the first new
//    one is visible
//
      if ( lastNew  ) {
	 This->fieldView->ScrollToItem(*lastNew);
	 This->fieldView->ScrollToItem(*firstNew);
      }

//
// Display the first item if there are no new ones
//
      if ( !lastNew && count > 0 ) This->fieldView->ScrollToItem(*list[0]);

      This->pub->UpdateFields();
      This->pub->UpdateTitle();
      This->pub->EnableButtons();

      firstTime = False;

   } // End if first time window mapped

//
// Update the recent folder menus if necessary
//
   if ( ishApp->appPrefs->recentFolderTime > This->recentMenuTime )
      This->BuildRecentFolderMenus();

} // End DoMap

/*------------------------------------------------------------------------
 * Method to return pixmap appropriate to new message
 */

PixmapC *
MainWinP::NewMailPixmap()
{
   if ( ishApp->sleeping ) return newMailPM;

//
// See if there is an icon rule for the newest message
//
   MsgC		*msg = ishApp->systemFolder->NewestMsg();
   if ( !msg ) return newMailPM;

   StringC	*name = msg->Match(ishApp->iconPrefs->iconRules);

//
// If a name was found, look up or create the pixmap
//
   if ( name ) {

//
// If the pixmap already exists, just return it.
//
      PixmapPtrT	*ptr = iconDict->definitionOf(*name);
      PixmapC		*pm  = ptr ? *ptr : (PixmapC*)NULL;
      if ( pm ) return pm;

//
// Create a new pixmap.  The icon pixmaps use the default colormap rather
//    than the application colormap since they're displayed in the window
//    manager.
//
      Screen	*scr = halApp->screen;
      pm = new PixmapC(*name, iconFG, iconBG, iconBG, iconFG, scr,
		       (Window)*halApp, XDefaultColormapOfScreen(scr));
      if ( !pm->reg ) {
	 delete pm;
	 return newMailPM;
      }
      else {
	 iconDict->add(*name, pm);
	 return pm;
      }

   } // End if an image name was found

//
// Return default pixmap if no match was found
//
   return newMailPM;

} // End NewMailPixmap

/*---------------------------------------------------------------
 *  Callback used when alert has completed
 */

typedef struct {
   StringC	cmd;
   char		*msgFile;
   char		*headFile;
   char		*bodyFile;
} AlertDataT;

static void
AlertDone(int status, AlertDataT *data)
{
   if ( status != 0 ) {

      if ( debuglev > 1 ) cout <<"alert status: " <<status <<endl;

      StringC msg = data->cmd;
      msg += " reported the following error:\n";

      if ( status < 0 )
	 msg += SystemErrorMessage(status);
      else {
         if ( WIFEXITED(status) ) {
	    status = WEXITSTATUS(status);
	    msg += SystemErrorMessage(status);
	 }
	 else {
	    msg += status;
	 }
      }

      XBell(ishApp->display, ishApp->appPrefs->bellVolume);
      ishApp->PopupMessage(msg);

   } // End if status non-zero

//
// Clean up
//
   if ( data->msgFile ) {
      unlink(data->msgFile);
      free(data->msgFile);
   }
   if ( data->headFile ) {
      unlink(data->headFile);
      free(data->headFile);
   }
   if ( data->bodyFile ) {
      unlink(data->bodyFile);
      free(data->bodyFile);
   }

   delete data;

} // End AlertDone

/*------------------------------------------------------------------------
 * Method to ring bell or execute alert command as appropriate
 */

void
MainWinP::NewMailAlert()
{
   if ( ishApp->sleeping || !ishApp->alertPrefs->alertOn ) return;

//
// See if there is a alert rule for the newest message
//
   MsgC		*msg = ishApp->systemFolder->NewestMsg();
   if ( !msg ) return;

   StringC	*cmd = msg->Match(ishApp->alertPrefs->alertRules);

//
// Execute command or ring bell
//
   if ( !cmd ) {
      XBell(ishApp->display, ishApp->appPrefs->bellVolume);
   }

   else {

//
// Look for %m, %h or %b in the command.
//    %m will be replaced with the name of a file containing the entire message.
//    %h will be replaced with the name of a file containing the headers only.
//    %b will be replaced with the name of a file containing the body only.
//    %% will be replaced with %
//
      AlertDataT	*data = new AlertDataT;
      data->cmd      = *cmd;
      data->msgFile  = NULL;
      data->headFile = NULL;
      data->bodyFile = NULL;

      static RegexC	*percPat = NULL;
      if ( !percPat ) percPat = new RegexC("%[%mhb]");

      int	pos = 0;
      while ( (pos=percPat->search(data->cmd, pos)) >= 0 ) {

	 char	c     = data->cmd[pos+1];
	 RangeC	range = (*percPat)[0];

	 switch (c) {

	    case '%':
	       data->cmd(range) = "%";
	       pos++;
	       break;

	    case 'm':      // Message file
	       if ( !data->msgFile ) {
		  data->msgFile = tempnam(NULL, "alrt.");
		  msg->WriteFile(data->msgFile, /*copyHead=*/True,
		  				/*allHead=*/True,
		  				/*statHead=*/True,
						/*addBlank=*/False,
						/*protectFroms=*/False);
	       }
	       data->cmd(range) = data->msgFile;
	       pos += strlen(data->msgFile);
	       break;

	    case 'h':      // Header file
	       if ( !data->headFile ) {
		  data->headFile = tempnam(NULL, "alrt.");
		  msg->WriteHeaders(data->headFile);
	       }
	       data->cmd(range) = data->headFile;
	       pos += strlen(data->headFile);
	       break;

	    case 'b':      // Body file
	       if ( !data->bodyFile ) {
		  data->bodyFile = tempnam(NULL, "alrt.");
		  msg->WriteBody(data->bodyFile, /*addBlank=*/False,
		  				 /*protectFroms=*/False);
	       }
	       data->cmd(range) = data->bodyFile;
	       pos += strlen(data->bodyFile);
	       break;

	 } // End switch percent character

      } // End while a percent pattern is found

//
// Run the command.  Add a callback so we can delete any temporary files
//    when the command is finished.
//
      CallbackC	doneCb((CallbackFn*)AlertDone, data);

      pid_t	status = ForkIt(data->cmd, &doneCb);
      if ( status < 0 ) AlertDone((int)status, data);

   } // End if there is a command

} // End NewMailAlert

/*---------------------------------------------------------------
 *  Callback routine to handle selection of a folder icon
 */

void
MainWinP::ChangeFolderSelection(VBoxC *vbox, MainWinP *This)
{
//
// If the system folder is selected, deselect it
//
   if ( vbox->SelItems().includes(ishApp->systemFolder->icon) )
      vbox->DeselectItem(*ishApp->systemFolder->icon, False);

   This->pub->EnableButtons();
}

/*------------------------------------------------------------------------
 * Method to compare two folders
 */

int
MainWinP::FolderCompare(const void *a, const void *b)
{
   VItemC	*ia = *(VItemC **)a;
   VItemC	*ib = *(VItemC **)b;
   FolderC	*fa = (FolderC*)ia->UserData();
   FolderC	*fb = (FolderC*)ib->UserData();

//
// The in-box always comes first
//
   if      ( fa->isInBox ) return -1;
   else if ( fb->isInBox ) return  1;
   else			   return fa->abbrev.compare(fb->abbrev);

} // End FolderCompare

/*-----------------------------------------------------------------------
 *  Handle drop event to folder box
 */

void
MainWinP::FolderDrop(void *arg, MainWinP *This)
{
   ViewDropDataT	*dropData = (ViewDropDataT*)arg;
   //cout << "Got drop into folder box" NL;

   XmDropProcCallbackStruct	*callData = dropData->procData;
   ViewDragDataT		*dragData;
   XtVaGetValues(callData->dragContext, XmNclientData, &dragData, NULL);
   //cout <<"Drag data is " << dragData NL;

   u_char	status = XmTRANSFER_SUCCESS;
   if ( callData->dropSiteStatus == XmVALID_DROP_SITE ) {

      This->pub->BusyCursor(True);
      This->pub->ClearMessage();

//
// Figure out which folder.  In-box is not applicable
//
      Boolean	found = False;
      FolderC	*folder;
      unsigned	count = ishApp->folderPrefs->OpenFolders().size();
      int i=0; for (i=0; !found && i<count; i++) {
	 folder = ishApp->folderPrefs->OpenFolders()[i];
	 found = (folder->icon == dropData->item);
      }

      if ( found ) {

	 //cout <<"   Folder is " << folder->Name() NL;

//
// Move or copy the items
//
	 switch ( callData->operation ) {

	    case (XmDROP_MOVE):
	       ishApp->saveMgr->SaveMsgsToFolder(dragData->itemList, folder,
			       This->pub->curFolder->writable/*delete after*/);
	       break;

	    case (XmDROP_COPY):
	       ishApp->saveMgr->SaveMsgsToFolder(dragData->itemList, folder,
						 False/*don't delete after*/);
	       break;

	 } // End switch operation type

      } // End if folder was found

      This->pub->BusyCursor(False);

//
// Drop must be acknowledged
//
      //cout << "Valid drop" NL;

   } // End if drop site is valid

   else {
      status = XmTRANSFER_FAILURE;
      //cout << "Invalid drop" NL;
   }

//
// Drop must be acknowledged
//
   WArgList	  args;
   args.TransferStatus(status);
   XmDropTransferStart(callData->dragContext, ARGS);

// Not on heap   delete dragData;

   //cout << "Finished with drop into folder box" NL;

} // End FolderDrop

/*---------------------------------------------------------------
 *  Callback to handle double-click on folder icon
 */

void
MainWinP::OpenFolder(VItemC *item, MainWinP *This)
{
   FolderC	*folder = (FolderC*)item->UserData();
   if ( !folder ) return;

   if ( folder->isInBox ) This->folderVBox->DeselectItem(*folder->icon);

//
// Double-click on current folder causes new mail check of the folder
//
   if ( folder == This->pub->curFolder ) {
      folder->NewMail();
      This->pub->UpdateTitle();
      return;
   }

   This->pub->BusyCursor(True);

//
// Close any unpinned reading windows
//
   unsigned	count = ishApp->readWinList.size();
   int i=0; for (i=0; i<count; i++) {
      ReadWinC	*readWin = (ReadWinC *)*ishApp->readWinList[i];
      if ( !readWin->Pinned() ) readWin->Hide();
   }

//
// Switch folders
//
   This->pub->curFolder->Deactivate();
   folder->Activate();

   if ( folder->active )
      This->pub->SetCurrentFolder(folder);
   else
      This->pub->ActivateSystemFolder();

   This->pub->BusyCursor(False);

} // End OpenFolder

/*---------------------------------------------------------------
 *  Callback to handle posting of folder popup menu
 */

void
MainWinP::PostFolderMenu(VItemC *item, MainWinP *This)
{
   FolderC	*folder = (FolderC*)item->UserData();
   if ( !folder ) return;

   This->pub->popupFolder = folder;

//
// See if this popup menu will apply to the selected items or the current
//    item.  It will apply to the selected items if there are more than one
//    and the current folder is also selected.
//
   VItemListC&	list = This->folderVBox->SelItems();
   This->pub->popupOnSelected = (list.size() > 1 && list.includes(item));

   StringC	title;
   if ( This->pub->popupOnSelected ) {

      XtSetSensitive(This->folderPUActPB, False);

//
// Allow the save operation if any of the folders has changed
//
      Boolean	changed  = False;
      Boolean	readOnly = False;
      unsigned	count = ishApp->folderPrefs->OpenFolders().size();
      int i=0; for (i=0; !changed && i<count; i++) {
	 FolderC	*fp = ishApp->folderPrefs->OpenFolders()[i];
	 if ( list.includes(fp->icon) ) {
	    if ( !changed  ) changed  =  fp->Changed();
	    if ( !readOnly ) readOnly = !fp->writable;
	 }
      }
      XtSetSensitive(This->folderPUSavePB, changed && !readOnly);

//
// Allow the delete operation if the folders are not read only
//
      XtSetSensitive(This->folderPUDelPB, !readOnly);

//
// Set the menu title
//
      title = "Selected folders";

   } // End if menu applies to more than one folder

   else {

      XtSetSensitive(This->folderPUActPB,  folder != This->pub->curFolder);
      XtSetSensitive(This->folderPUSavePB, folder->Changed()&&folder->writable);
      XtSetSensitive(This->folderPUClosePB, !folder->isInBox);
      XtSetSensitive(This->folderPUDelPB,   !folder->isInBox);

//
// Use the folder name as the menu title
//
      title = "Folder: " + item->Label();

//
// Select this folder
//
      This->folderVBox->SelectItemOnly(*item);

   } // End if menu applies to only one folder

//
// Display the menu title
//
   WXmString	wstr = (char*)title;
   XtVaSetValues(This->folderPULabel, XmNlabelString, (XmString)wstr, NULL);

} // End PostFolderMenu

/*---------------------------------------------------------------
 *  Callback routine to handle selection of a message summary line
 */

void
MainWinP::ChangeMsgSelection(VBoxC *vb, MainWinP *This)
{
   This->pub->EnableButtons();

//
// Get the name of the default save folder
//
   VItemListC&	list = vb->SelItems();
   MsgItemC	*item = NULL;
   if ( list.size() > 0 ) item = (MsgItemC*)list[0];

   ishApp->saveMgr->UpdateSaveFolder(item ? item->msg : (MsgC*)NULL);

//
// Display folder name on button
//
   StringC	label = ishApp->saveMgr->curSaveFolder;
   ishApp->AbbreviateFolderName(label);
   label = "To: " + label;
   WXmString	wstr = (char*)label;
   XtVaSetValues(This->msgSavePB, XmNlabelString, (XmString)wstr, NULL);

// Causes too much resizing
#if 0
//
// Update custom button if present
//
   Widget	custom = This->buttMgr->ButtonFor(This->msgSavePB);
   if ( custom )
      XtVaSetValues(custom, XmNlabelString, (XmString)wstr, NULL);
#endif

} // End ChangeMsgSelection

/*---------------------------------------------------------------
 *  Callback routine to handle drag out of message summary
 */

void
MainWinP::MsgDrag(void *arg, MainWinP *This)
{
   ViewDragDataT	*dd = (ViewDragDataT*)arg;
   //cout <<"Got drag of " <<This <<" with data " <<dd NL;

//
// Get message numbers from item list
//
   StringC	statMsg("Dragging message");
   unsigned	count = dd->itemList.size();
   if ( count > 1 ) statMsg += 's';
   statMsg += ' ';

   int i=0; for (i=0; i<count; i++) {
      if ( i>0 ) statMsg += ", ";
      MsgItemC	*item = (MsgItemC *)dd->itemList[i];
      statMsg += item->msg->Number();
   }
   This->pub->Message(statMsg);

//
// Create atom for exported target
//
   Atom expAtoms[1];
   expAtoms[0] = ishApp->msgAtom;

//
// Start the drag
//
   WArgList	args;
   args.Reset();
   args.ExportTargets(expAtoms);
   args.NumExportTargets(1);
   args.SourceCursorIcon(dd->icon);
   args.SourcePixmapIcon(dd->icon);
   args.ClientData((XtPointer)dd);
   Widget	dragContext = XmDragStart(dd->widget, dd->event, ARGS);

} // End MsgDrag

/*---------------------------------------------------------------
 *  Callback to handle double-click on message summary
 */

void
MainWinP::OpenMessage(MsgItemC *mi, MainWinP *This)
{
   This->pub->readingSelected = False;
   ishApp->DisplayMessage(mi->msg);

//
// Find the reading window that was used and force it to open
//
   unsigned	count = ishApp->readWinList.size();
   int i=0; for (i=0; i<count; i++) {
      ReadWinC	*readWin = (ReadWinC *)*ishApp->readWinList[i];
      if ( mi->msg == readWin->msg ) readWin->Show();
   }
}

/*---------------------------------------------------------------
 *  Callback to handle posting of message item popup menu
 */

void
MainWinP::PostMsgMenu(MsgItemC *item, MainWinP *This)
{
   This->pub->popupMsg = item->msg;

//
// See if this popup menu will apply to the selected items or the current
//    item.  It will apply to the selected items if there are more than one
//    and the current item is also selected.
//
   VItemListC&	list = This->msgVBox->SelItems();
   This->pub->popupOnSelected = (list.size() > 1 && list.includes(item));
   MsgItemC	*firstItem = NULL;
   if ( list.size() > 0 ) firstItem = (MsgItemC*)list[0];

//
// Allow these operations if there is only one item
//
   Boolean	replyOk = !This->pub->popupOnSelected;
   XtSetSensitive(This->msgPUReplyPB,       replyOk);
   XtSetSensitive(This->msgPUReplyAllPB,    replyOk);
   XtSetSensitive(This->msgPUReplyIncPB,    replyOk);
   XtSetSensitive(This->msgPUReplyAllIncPB, replyOk);
   XtSetSensitive(This->msgPUResendPB,      True);

//
// Allow Delete if there are any undeleted items
//
   Boolean	deleteOk = False;
   if ( This->pub->curFolder->writable ) {

      if ( This->pub->popupOnSelected ) {
	 unsigned	count = list.size();
	 for (int i=0; !deleteOk && i<count; i++) {
	    MsgItemC	*mi = (MsgItemC*)list[i];
	    deleteOk = !mi->IsDeleted();
	 }
      }
      else {
	 deleteOk = !item->IsDeleted();
      }

   } // End if folder not read-only

   XtSetSensitive(This->msgPUDelPB, deleteOk);

//
// Allow Undelete if there are any deleted items
//
   Boolean	undeleteOk = False;
   if ( This->pub->curFolder->writable ) {

      if ( This->pub->popupOnSelected ) {
	 unsigned	count = list.size();
	 for (int i=0; !undeleteOk && i<count; i++) {
	    MsgItemC	*mi = (MsgItemC*)list[i];
	    undeleteOk = mi->IsDeleted();
	 }
      }
      else {
	 undeleteOk = item->IsDeleted();
      }

   } // End if folder not read-only

   XtSetSensitive(This->msgPUUndelPB, undeleteOk);

//
// Use the message number as the menu title if there is only one
//
   StringC	title;
   if ( This->pub->popupOnSelected )
      title = "Selected messages";
   else {
      title = "Message: ";
      title += item->msg->Number();
   }

   WXmString	wstr = (char*)title;
   XtVaSetValues(This->msgPULabel, XmNlabelString, (XmString)wstr, NULL);

//
// Select this message if it's not selected and we're not popping up on
//    selected
//
   if ( !This->pub->popupOnSelected ) {
      This->msgVBox->SelectItemOnly(*item);
      firstItem = item;
   }

//
// Determine the default save folder
//
   ishApp->saveMgr->UpdateSaveFolder(firstItem ? firstItem->msg : (MsgC*)NULL);

//
// Display folder name on button
//
   StringC	label = ishApp->saveMgr->curSaveFolder;
   ishApp->AbbreviateFolderName(label);
   label = "To: " + label;
   wstr = (char*)label;
   XtVaSetValues(This->msgPUSavePB, XmNlabelString, (XmString)wstr, NULL);

   u_int	folderSelCount = This->folderVBox->SelItems().size();
   XtSetSensitive(This->msgPUSaveSelPB,    folderSelCount>0);
   XtSetSensitive(This->msgPUSaveRecentCB,
   		  ishApp->appPrefs->recentFolders.size()>0);

} // End PostMsgMenu

/*---------------------------------------------------------------
 *  Callback to handle change of recent folders list
 */

void
MainWinP::RecentListChanged(void*, MainWinP *This)
{
//
// Update the menus only if this window is displayed
//
   if ( !This->pub->IsShown() || This->deferRecentMenu ) return;

   This->BuildRecentFolderMenus();
}

/*---------------------------------------------------------------
 *  Method to query for folder save if necessary
 */

QueryAnswerT
MainWinP::SaveQuery(FolderListC *folders)
{
   if ( !ishApp->confPrefs->confirmSaveOnExit ) return QUERY_YES;

//
// If no list was passed, we're doing all folders
//
   Boolean	doSystem = (folders == NULL);
   FolderListC&	folderList = (folders ? *folders
				      : ishApp->folderPrefs->OpenFolders());

//
// See if any open folder has changed.
//
   int		numChanged = 0;
   Boolean	anyChanged = (doSystem && ishApp->systemFolder->writable &&
   					  ishApp->systemFolder->Changed());
   Boolean	allChanged = anyChanged;
   StringListC	nameList;
   if ( anyChanged ) {
      nameList.append("In-box");
      numChanged++;
   }

   unsigned	count = folderList.size();
   int i=0; for (i=0; i<count; i++) {

      FolderC	*folder = folderList[i];
      if ( folder->isInBox ) continue;

      Boolean	changed = (folder->writable && folder->Changed());

      anyChanged = (anyChanged || changed);
      allChanged = (allChanged && changed);

      if ( changed ) {
	 numChanged++;
	 nameList.append(folder->name);
      }
   }

   if ( !anyChanged ) return QUERY_NO;

   static QueryAnswerT	answer;

//
// Create the dialog if necessary
//
   if ( !saveQueryWin ) {

      pub->BusyCursor(True);

      WArgList	args;
      args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
      if ( halApp->questionPM ) args.SymbolPixmap(halApp->questionPM);
      Widget w = XmCreateQuestionDialog(*halApp, "saveQueryWin", ARGS);

      XtAddCallback(w, XmNokCallback, (XtCallbackProc)AnswerQuery,
		    (XtPointer)&answer);
      XtAddCallback(w, XmNcancelCallback, (XtCallbackProc)AnswerQuery,
		    (XtPointer)&answer);
      XtAddCallback(w, XmNhelpCallback, (XtCallbackProc)HalAppC::DoHelp,
      		    (char *) "helpcard");

      Widget noPB = XmCreatePushButton(w, "noPB", 0,0);
      XtManageChild(noPB);

      XtAddCallback(noPB, XmNactivateCallback, (XtCallbackProc)AnswerQuery,
		    (XtPointer)&answer);

//
// Trap window manager close function
//
      XmAddWMProtocolCallback(XtParent(w), halApp->delWinAtom,
			      (XtCallbackProc)WmClose,
			      (caddr_t)&answer);

      saveQueryWin = w;
      pub->BusyCursor(False);

   } // End if save query dialog not created

//
// If only one folder changed, ask about that one.  Otherwise, if all of
//    them changed, present the "all" message.  If not, create a list of
//    the changed ones.  And add the names to the message.
//
   StringC	msg;
   if ( numChanged == 1 ) {
      if ( doSystem && ishApp->systemFolder->Changed() )
	 msg = "Save changes to in-box?";
      else
	 msg = "Save changes to folder \"" + *nameList[0] + "\"?";
   }
   else {
      if ( allChanged ) {
	 msg = "Save changes to all folders?";
      }
      else {
	 msg = "Save changes to these folders?:\n";
	 count = nameList.size();
	 for (i=0; i<count; i++) msg += "\n" + *nameList[i];
      }
   }

   WXmString	wstr = (char *)msg;
   XtVaSetValues(saveQueryWin, XmNmessageString, (XmString)wstr, NULL);
   XtManageChild(saveQueryWin);
   XMapRaised(halApp->display, XtWindow(XtParent(saveQueryWin)));

//
// Simulate the main event loop and wait for the answer
//

   answer = QUERY_NONE;
   while ( answer == QUERY_NONE ) {
      XtAppProcessEvent(halApp->context, XtIMXEvent);
      XSync(halApp->display, False);
   }

   XtUnmanageChild(saveQueryWin);
   XSync(halApp->display, False);
   XmUpdateDisplay(saveQueryWin);

//
// If more than one changed and they answered no, should we ask about each
//    one individually?  Maybe later.
//
   return answer;

} // End SaveQuery

/*------------------------------------------------------------------------
 * Function to bring up the user's guide
 */

void
MainWinP::DoHelpGuide(Widget, MainWinP*, XtPointer)
{
   ishApp->BusyCursor(True);

//
// Get the URL for the user's guide
//
   StringC url = ishHome + "/doc/index.html";

//
// Add the URL to the web browser command
//
   StringC	cmd = ishApp->readPrefs->webCmd;
   cmd.Replace("%s", url);

//
// Run this command in the background
//
   if ( debuglev > 0 ) cout <<"Executing command: " <<cmd <<endl;
   if ( !cmd.EndsWith("&") ) cmd += " &";

   System(cmd);

   ishApp->BusyCursor(False);

} // End DoHelpGuide

/*------------------------------------------------------------------------
 * Function to allow user to send comments
 */

void
MainWinP::DoHelpComment(Widget, MainWinP*, XtPointer)
{
   ishApp->BusyCursor(True);

//
// Get the URL for the user's guide
//
   StringC url = "http://sourceforge.net/tracker/?group_id=5503";

//
// Add the URL to the web browser command
//
   StringC	cmd = ishApp->readPrefs->webCmd;
   cmd.Replace("%s", url);

//
// Run this command in the background
//
   if ( debuglev > 0 ) cout <<"Executing command: " <<cmd <<endl;
   if ( !cmd.EndsWith("&") ) cmd += " &";

   System(cmd);

   ishApp->BusyCursor(False);

} // End DoHelpComment

/*---------------------------------------------------------------
 *  Method to see if any of the new messages should be auto-filed
 */

void
MainWinP::AutoFileNewMail()
{
   if ( !ishApp->systemFolder ) return;
   
//
// Temporarily postpone changes to the recent folders menu
//
   deferRecentMenu = True;

   MsgListC&	msgList = *ishApp->systemFolder->msgList;
   StringC	statusMsg;
   u_int	count = msgList.size();
   int i=0; for (i=0; i<count; i++) {

      MsgC	*msg = msgList[i];
      if ( msg->IsDeleted() || msg->IsPartial() || msg->IsRead() )
	 continue;

//
// See if this message matches any auto-filing patterns
//
      StringC	*name = msg->Match(ishApp->autoFilePrefs->autoFileRules);
      if ( !name ) continue;

//
// Save the message
//
      FolderC *folder = ishApp->folderPrefs->GetFolder(*name,
						       True/*ok to create*/);
      if ( folder ) {

	 ishApp->saveMgr->SaveMsgToFolder(msg, folder, True/*delete*/);

	 statusMsg = "Message from %fromName saved to ";
	 statusMsg += folder->abbrev;
	 statusMsg += ": %subject";
	 msg->ReplaceHeaders(statusMsg);
      }

      else {
	 statusMsg = "Could not open folder ";
	 statusMsg += *name;
	 statusMsg += " for auto-filing.";
      }

      ishApp->Broadcast(statusMsg);

   } // End for each message in folder

   msgVBox->Refresh();

//
// Update recent folder menus now
//
   deferRecentMenu = False;

   if ( ishApp->appPrefs->recentFolderTime > recentMenuTime )
      BuildRecentFolderMenus();

} // End AutoFileNewMail

