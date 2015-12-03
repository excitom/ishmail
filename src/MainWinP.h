/*
 *  $Id: MainWinP.h,v 1.3 2001/07/28 18:26:03 evgeny Exp $
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
#ifndef _MainWinP_h_
#define _MainWinP_h_

#include "Query.h"

#include <hgl/StringC.h>

#include <X11/Intrinsic.h>

#include <time.h>

class MainWinC;
class TBoxC;
class VBoxC;
class FieldViewC;
class PixmapC;
class PixmapNameDictC;
class MsgC;
class MsgItemC;
class VItemC;
class VItemListC;
class FolderC;
class FolderListC;
class FileChooserWinC;
class StringListC;
class MsgFindWinC;
class PrintWinC;
class PipeWinC;
class SendWinC;

class MainWinP {

   friend class MainWinC;

   MainWinC	*pub;		// Public stuff

//
// File menu widgets
//
   Widget	filePrintPB;

//
// Folder menu widgets
//
   Widget	folderOpenRecentCB;
   Widget	folderOpenRecentPD;
   Widget	folderOpenQuickCB;
   Widget	folderSelPB;
   Widget	folderDeselPB;
   Widget	folderActSysPB;
   Widget	folderActSelPB;
   Widget	folderSaveCurPB;
   Widget	folderSaveSelPB;
   Widget	folderSaveAllPB;
   Widget	folderReadCurPB;
   Widget	folderReadSelPB;
   Widget	folderReadAllPB;
   Widget	folderCloseCurPB;
   Widget	folderCloseSelPB;
   Widget	folderCloseAllPB;
   Widget	folderDelCurPB;
   Widget	folderDelSelPB;
   Widget	folderDelAllPB;

//
// Message menu widgets
//
   Widget	msgComposePB;
   Widget	msgReplyPB;
   Widget	msgReplyIncPB;
   Widget	msgReplyAllPB;
   Widget	msgReplyAllIncPB;
   Widget	msgForwardPB;
   Widget	msgForward822PB;
   Widget	msgResendPB;
   Widget	msgSavePB;
   Widget	msgSavePatPB;
   Widget	msgSaveSelPB;
   Widget	msgSaveToPB;
   Widget	msgSaveToFilePB;
   Widget	msgSaveRecentCB;
   Widget	msgSaveRecentPD;
   Widget	msgSaveQuickCB;
   Widget	msgPrintPB;
   Widget	msgPipePB;
   Widget	msgReadPB;
   Widget	msgNextPB;
   Widget	msgNextUnreadPB;
   Widget	msgNextSenderPB;
   Widget	msgNextSubjectPB;
   Widget	msgPrevPB;
   Widget	msgPrevUnreadPB;
   Widget	msgPrevSenderPB;
   Widget	msgPrevSubjectPB;
   Widget	msgDelPB;
   Widget	msgUndelLastPB;
   Widget	msgUndelSelPB;
   Widget	msgUndelListPB;
   Widget	msgStatCB;
   Widget	msgSelPB;
   Widget	msgDeselPB;
   Widget	msgFindPB;

//
// Message popup menu widgets
//
   Widget	msgPU;
   Widget	msgPULabel;
   Widget	msgPUReadPB;
   Widget	msgPUSavePB;
   Widget	msgPUSavePatPB;
   Widget	msgPUSaveSelPB;
   Widget	msgPUSaveToPB;
   Widget	msgPUSaveToFilePB;
   Widget	msgPUSaveRecentCB;
   Widget	msgPUSaveRecentPD;
   Widget	msgPUSaveQuickCB;
   Widget	msgPUPrintPB;
   Widget	msgPUPipePB;
   Widget	msgPUReplyPB;
   Widget	msgPUReplyAllPB;
   Widget	msgPUReplyIncPB;
   Widget	msgPUReplyAllIncPB;
   Widget	msgPUForwardPB;
   Widget	msgPUForward822PB;
   Widget	msgPUResendPB;
   Widget	msgPUStatPB;
   Widget	msgPUDelPB;
   Widget	msgPUUndelPB;

//
// Folder popup menu widgets
//
   Widget	folderPU;
   Widget	folderPULabel;
   Widget	folderPUActPB;
   Widget	folderPUSavePB;
   Widget	folderPUReadPB;
   Widget	folderPUClosePB;
   Widget	folderPUDelPB;

//
// Main window widgets
//
   VBoxC	*folderVBox;
   TBoxC	*msgTBox;
   VBoxC	*msgVBox;
   FieldViewC	*fieldView;
   int		viewType;

//
// Private data
//
   StringC	systemTitleStr;
   StringC	folderTitleStr;
   StringC	msgCountStr;
   StringC	newMsgCountStr;
   StringC	unrMsgCountStr;
   StringC	savMsgCountStr;
   StringC	delMsgCountStr;

//
// Pixmaps
//
   Pixel		iconFG;
   Pixel		iconBG;
   PixmapC		*noMailPM;
   PixmapC		*newMailPM;
   PixmapC		*readMailPM;
   PixmapC		*unreadMailPM;
   PixmapNameDictC      *iconDict;       // name -> pixmap

//
// File menu callbacks
//
   static void	DoFileSleep  (Widget, MainWinP*, XtPointer);
   static void	DoFileQuit   (Widget, MainWinP*, XtPointer);
   static void	DoFileExit   (Widget, MainWinP*, XtPointer);
   static void	Exit         (void*,  MainWinP*);

//
// Folder menu callbacks
//
   static void	DoFolderNew          (Widget, MainWinP*, XtPointer);
   static void	DoFolderOpen         (Widget, MainWinP*, XtPointer);
   static void	DoFolderOpenButton   (Widget, MainWinP*, XtPointer);
   static void	DoFolderActSys       (Widget, MainWinP*, XtPointer);
   static void	DoFolderActSel       (Widget, MainWinP*, XtPointer);
   static void	DoFolderSaveCur      (Widget, MainWinP*, XtPointer);
   static void	DoFolderSaveSel      (Widget, MainWinP*, XtPointer);
   static void	DoFolderSaveAll      (Widget, MainWinP*, XtPointer);
   static void	DoFolderReadCur      (Widget, MainWinP*, XtPointer);
   static void	DoFolderReadSel      (Widget, MainWinP*, XtPointer);
   static void	DoFolderReadAll      (Widget, MainWinP*, XtPointer);
   static void	DoFolderCloseCur     (Widget, MainWinP*, XtPointer);
   static void	DoFolderCloseSel     (Widget, MainWinP*, XtPointer);
   static void	DoFolderCloseAll     (Widget, MainWinP*, XtPointer);
   static void	DoFolderDelCur       (Widget, MainWinP*, XtPointer);
   static void	DoFolderDelSel       (Widget, MainWinP*, XtPointer);
   static void	DoFolderDelAll       (Widget, MainWinP*, XtPointer);
   static void	DoFolderSel          (Widget, MainWinP*, XtPointer);
   static void	DoFolderDesel        (Widget, MainWinP*, XtPointer);
   static void	PrepareOpenQuickMenu (Widget, MainWinP*, XtPointer);

//
// Message menu callbacks
//
   static void	DoMsgCompose          (Widget, MainWinP*, XtPointer);
   static void	DoMsgReply            (Widget, MainWinP*, XtPointer);
   static void	DoMsgReplyAll         (Widget, MainWinP*, XtPointer);
   static void	DoMsgReplyAllInc      (Widget, MainWinP*, XtPointer);
   static void	DoMsgReplyInc         (Widget, MainWinP*, XtPointer);
   static void	DoMsgForward          (Widget, MainWinP*, XtPointer);
   static void	DoMsgForward822       (Widget, MainWinP*, XtPointer);
   static void	DoMsgResend           (Widget, MainWinP*, XtPointer);
   static void	DoMsgRead             (Widget, MainWinP*, XtPointer);
   static void	DoMsgNext             (Widget, MainWinP*, XtPointer);
   static void	DoMsgNextUnread       (Widget, MainWinP*, XtPointer);
   static void	DoMsgNextSender       (Widget, MainWinP*, XtPointer);
   static void	DoMsgNextSubject      (Widget, MainWinP*, XtPointer);
   static void	DoMsgPrev             (Widget, MainWinP*, XtPointer);
   static void	DoMsgPrevUnread       (Widget, MainWinP*, XtPointer);
   static void	DoMsgPrevSender       (Widget, MainWinP*, XtPointer);
   static void	DoMsgPrevSubject      (Widget, MainWinP*, XtPointer);
   static void	DoMsgSave             (Widget, MainWinP*, XtPointer);
   static void	DoMsgSavePat          (Widget, MainWinP*, XtPointer);
   static void	DoMsgSaveSel          (Widget, MainWinP*, XtPointer);
   static void	DoMsgSaveTo           (Widget, MainWinP*, XtPointer);
   static void	DoMsgSaveToFile       (Widget, MainWinP*, XtPointer);
   static void	DoMsgSaveToButton     (Widget, MainWinP*, XtPointer);
   static void	DoMsgPrint            (Widget, MainWinP*, XtPointer);
   static void	DoMsgPipe             (Widget, MainWinP*, XtPointer);
   static void	DoMsgDel              (Widget, MainWinP*, XtPointer);
   static void	DoMsgUndelLast        (Widget, MainWinP*, XtPointer);
   static void	DoMsgUndelSel         (Widget, MainWinP*, XtPointer);
   static void	DoMsgUndelList        (Widget, MainWinP*, XtPointer);
   static void	DoMsgMarkRead         (Widget, MainWinP*, XtPointer);
   static void	DoMsgMarkUnread       (Widget, MainWinP*, XtPointer);
   static void	DoMsgMarkNew          (Widget, MainWinP*, XtPointer);
   static void	DoMsgSetSaved         (Widget, MainWinP*, XtPointer);
   static void	DoMsgSetReplied       (Widget, MainWinP*, XtPointer);
   static void	DoMsgSetForwarded     (Widget, MainWinP*, XtPointer);
   static void	DoMsgSetResent        (Widget, MainWinP*, XtPointer);
   static void	DoMsgSetPrinted       (Widget, MainWinP*, XtPointer);
   static void	DoMsgSetFiltered      (Widget, MainWinP*, XtPointer);
   static void	DoMsgClearAll         (Widget, MainWinP*, XtPointer);
   static void	DoMsgClearSaved       (Widget, MainWinP*, XtPointer);
   static void	DoMsgClearReplied     (Widget, MainWinP*, XtPointer);
   static void	DoMsgClearForwarded   (Widget, MainWinP*, XtPointer);
   static void	DoMsgClearResent      (Widget, MainWinP*, XtPointer);
   static void	DoMsgClearPrinted     (Widget, MainWinP*, XtPointer);
   static void	DoMsgClearFiltered    (Widget, MainWinP*, XtPointer);
   static void	DoMsgSel              (Widget, MainWinP*, XtPointer);
   static void	DoMsgDesel            (Widget, MainWinP*, XtPointer);
   static void	DoMsgFind             (Widget, MainWinP*, XtPointer);
   static void	PrepareSaveQuickMenu  (Widget, MainWinP*, XtPointer);

//
// Option menu callbacks
//
   static void	DoOptApp   (Widget, MainWinP*, XtPointer);
   static void	DoOptRead  (Widget, MainWinP*, XtPointer);
   static void	DoOptSend  (Widget, MainWinP*, XtPointer);
   static void	DoOptMail  (Widget, MainWinP*, XtPointer);
   static void	DoOptSig   (Widget, MainWinP*, XtPointer);
   static void	DoOptReply (Widget, MainWinP*, XtPointer);
   static void	DoOptConf  (Widget, MainWinP*, XtPointer);
   static void	DoOptHead  (Widget, MainWinP*, XtPointer);
   static void	DoOptAlias (Widget, MainWinP*, XtPointer);
   static void	DoOptGroup (Widget, MainWinP*, XtPointer);
   static void	DoOptAlert (Widget, MainWinP*, XtPointer);
   static void	DoOptIcon  (Widget, MainWinP*, XtPointer);
   static void	DoOptSave  (Widget, MainWinP*, XtPointer);
   static void	DoOptAuto  (Widget, MainWinP*, XtPointer);
   static void	DoOptFont  (Widget, MainWinP*, XtPointer);
   static void	DoOptButt  (Widget, MainWinP*, XtPointer);
   static void	DoOptSumm  (Widget, MainWinP*, XtPointer);
   static void	DoOptFolder(Widget, MainWinP*, XtPointer);
   static void	DoOptSort  (Widget, MainWinP*, XtPointer);

//
// Help menu callbacks
//
   static void	DoHelpGuide(Widget, MainWinP*, XtPointer);
   static void	DoHelpComment(Widget, MainWinP*, XtPointer);

//
// Message popup menu callbacks
//
   static void	DoMsgPURead           (Widget, MainWinP*, XtPointer);
   static void	DoMsgPUSave           (Widget, MainWinP*, XtPointer);
   static void	DoMsgPUSavePat        (Widget, MainWinP*, XtPointer);
   static void	DoMsgPUSaveSel        (Widget, MainWinP*, XtPointer);
   static void	DoMsgPUSaveTo         (Widget, MainWinP*, XtPointer);
   static void	DoMsgPUSaveToFile     (Widget, MainWinP*, XtPointer);
   static void	DoMsgPUSaveToButton   (Widget, MainWinP*, XtPointer);
   static void	DoMsgPUPrint          (Widget, MainWinP*, XtPointer);
   static void	DoMsgPUPipe           (Widget, MainWinP*, XtPointer);
   static void	DoMsgPUReply          (Widget, MainWinP*, XtPointer);
   static void	DoMsgPUReplyAll       (Widget, MainWinP*, XtPointer);
   static void	DoMsgPUReplyAllInc    (Widget, MainWinP*, XtPointer);
   static void	DoMsgPUReplyInc       (Widget, MainWinP*, XtPointer);
   static void	DoMsgPUForward        (Widget, MainWinP*, XtPointer);
   static void	DoMsgPUForward822     (Widget, MainWinP*, XtPointer);
   static void	DoMsgPUResend         (Widget, MainWinP*, XtPointer);
   static void	DoMsgPUStat           (Widget, MainWinP*, XtPointer);
   static void	DoMsgPUDel            (Widget, MainWinP*, XtPointer);
   static void	DoMsgPUUndel          (Widget, MainWinP*, XtPointer);
   static void	DoMsgPUMarkRead       (Widget, MainWinP*, XtPointer);
   static void	DoMsgPUMarkUnread     (Widget, MainWinP*, XtPointer);
   static void	DoMsgPUMarkNew        (Widget, MainWinP*, XtPointer);
   static void	DoMsgPUSetSaved       (Widget, MainWinP*, XtPointer);
   static void	DoMsgPUSetReplied     (Widget, MainWinP*, XtPointer);
   static void	DoMsgPUSetForwarded   (Widget, MainWinP*, XtPointer);
   static void	DoMsgPUSetResent      (Widget, MainWinP*, XtPointer);
   static void	DoMsgPUSetPrinted     (Widget, MainWinP*, XtPointer);
   static void	DoMsgPUSetFiltered    (Widget, MainWinP*, XtPointer);
   static void	DoMsgPUClearAll       (Widget, MainWinP*, XtPointer);
   static void	DoMsgPUClearSaved     (Widget, MainWinP*, XtPointer);
   static void	DoMsgPUClearReplied   (Widget, MainWinP*, XtPointer);
   static void	DoMsgPUClearForwarded (Widget, MainWinP*, XtPointer);
   static void	DoMsgPUClearResent    (Widget, MainWinP*, XtPointer);
   static void	DoMsgPUClearPrinted   (Widget, MainWinP*, XtPointer);
   static void	DoMsgPUClearFiltered  (Widget, MainWinP*, XtPointer);
   static void	PreparePUSaveQuickMenu(Widget, MainWinP*, XtPointer);

//
// Folder popup menu callbacks
//
   static void	DoFolderPUAct         (Widget, MainWinP*, XtPointer);
   static void	DoFolderPUSave        (Widget, MainWinP*, XtPointer);
   static void	DoFolderPURead        (Widget, MainWinP*, XtPointer);
   static void	DoFolderPUClose       (Widget, MainWinP*, XtPointer);
   static void	DoFolderPUDel         (Widget, MainWinP*, XtPointer);

//
// Folder list callbacks
//
   static void	ChangeFolderSelection(VBoxC*, MainWinP*);
   static int	FolderCompare(const void*, const void*);
   static void	FolderDrop(void*, MainWinP*);
   static void	OpenFolder(VItemC*, MainWinP*);
   static void	PostFolderMenu(VItemC*, MainWinP*);

//
// Message list callbacks
//
   static void	ChangeMsgSelection(VBoxC*, MainWinP*);
   static void	MsgDrag(void*, MainWinP*);
   static void	OpenMessage(MsgItemC*, MainWinP*);
   static void	PostMsgMenu(MsgItemC*, MainWinP*);

//
// Miscellaneous callbacks
//
   static void	DoMap(void*, MainWinP*);

//
// Private methods
//
   void		BuildFileMenu();
   void		BuildFolderMenu();
   void		BuildFolderPopupMenu();
   void		BuildHelpMenu();
   void		BuildMsgMenu();
   void		BuildMsgPopupMenu();
   void		BuildOptMenu();

//
// Recent folder menu stuff
//
   time_t	recentMenuTime;
   Boolean	deferRecentMenu;

   static void	RecentListChanged(void*, MainWinP*);
   void		BuildRecentFolderMenus();

//
// Creating folders
//
   FileChooserWinC	*newFolderWin;
   Widget		newFolderTypeFrame;
   Widget		newUnixFolderTB;
   Widget		newMmdfFolderTB;
   Widget		newMhFolderTB;

   static void  	FinishFolderNew(StringListC*, MainWinP*);

//
// Saving and deleting folders
//

   FolderListC	*delFolderList;
   Widget	saveQueryWin;

   void		DeleteFolders();
   static void	FinishFolderDel(Widget, MainWinP*, XtPointer);
   void		SaveFolder(FolderC*);
   QueryAnswerT	SaveQuery(FolderListC *list=NULL);

//
// Saving and deleting messages
//
   VItemListC	*lastDelList;	// List of most recently deleted msgs

   void		UndeleteHidden();
   void		UndeleteVisible();

//
// Opening folders
//
   FileChooserWinC	*openSelectWin;
   Widget		openOnlyTB;
   Widget		openActTB;

   static void		VerifyOpenFiles(StringListC*, MainWinP*);
   static void		FinishOpen     (StringListC*, MainWinP*);

//
// Searching for messages
//
   MsgFindWinC	*msgFindWin;

//
// Printing and filtering
//
   PrintWinC	*printWin;
   PipeWinC	*pipeWin;

public:

//
// Methods
//
   MainWinP(MainWinC*);
   ~MainWinP();

   void		AutoFileNewMail();
   void		BuildMenus();
   void		BuildWidgets();
   void         NewMailAlert();
   PixmapC	*NewMailPixmap();
   void		ReadResources();
};

#endif // _MainWinP_h_
