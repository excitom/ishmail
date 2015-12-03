/*
 *  $Id: ReadWinP.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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
#ifndef _ReadWinP_h_
#define _ReadWinP_h_

#include "ReadPrefC.h"

#include <hgl/StringC.h>
#include <hgl/PtrListC.h>
#include <hgl/CallbackC.h>

#include <Xm/Xm.h>
#include <Xm/DragDrop.h>

class MimeRichTextC;
class PixmapC;
class MsgPartC;
class PrintWinC;
class PipeWinC;
class ReadIconC;
class LoginWinC;
class LocalTextWinC;
class ReadWinP;
class MailcapC;
class FileChooserWinC;
class SendWinC;

/*---------------------------------------------------------------
 *  Data for fetching external body parts
 */

typedef struct {

   ReadWinP	*win;
   ReadIconC	*icon;
   MsgPartC	*part;
   StringC	file;
   StringC	cmdStr;
   pid_t	pid;
   CallbackC	postFetchCall;

} FetchDataT;

/*---------------------------------------------------------------
 *  Data for externally displaying body parts
 */

typedef struct {

   ReadWinP	*win;
   ReadIconC	*icon;
   StringC	cmdStr;
   pid_t	pid;

} DisplayDataT;

/*---------------------------------------------------------------
 *  Data for printing body parts
 */

typedef struct {

   ReadWinP	*win;
   MsgPartC	*part;
   StringC	file;
   StringC	cmdStr;
   pid_t	pid;

} PrintDataT;

/*---------------------------------------------------------------
 *  Private data for reading window
 */

class ReadWinP {

public:

   friend class	ReadWinC;

   ReadWinC		*pub;	// Public data

//
// File menu widgets
//
   Widget		fileNextPB;
   Widget		fileNextUnreadPB;
   Widget		fileNextSenderPB;
   Widget		fileNextSubjectPB;
   Widget		filePrevPB;
   Widget		filePrevUnreadPB;
   Widget		filePrevSenderPB;
   Widget		filePrevSubjectPB;
   Widget		fileSavePB;
   Widget		fileSaveSelPB;
   Widget		fileSaveToPB;
   Widget		fileSaveToFilePB;
   Widget		fileSaveRecentCB;
   Widget		fileSaveRecentPD;
   Widget		fileSaveQuickCB;
   Widget		filePrintPB;
   Widget		filePipePB;
   Widget		fileMimePB;
   Widget		fileUndeletePB;
   Widget		fileDeletePB;
   Widget		fileDelClosePB;

//
// File menu callbacks
//
   static void		DoNext        (Widget, ReadWinP*, XtPointer);
   static void		DoNextUnread  (Widget, ReadWinP*, XtPointer);
   static void		DoNextSender  (Widget, ReadWinP*, XtPointer);
   static void		DoNextSubject (Widget, ReadWinP*, XtPointer);
   static void		DoPrev        (Widget, ReadWinP*, XtPointer);
   static void		DoPrevUnread  (Widget, ReadWinP*, XtPointer);
   static void		DoPrevSender  (Widget, ReadWinP*, XtPointer);
   static void		DoPrevSubject (Widget, ReadWinP*, XtPointer);
   static void		DoSave        (Widget, ReadWinP*, XtPointer);
   static void		DoSaveSel     (Widget, ReadWinP*, XtPointer);
   static void		DoSaveTo      (Widget, ReadWinP*, XtPointer);
   static void		DoSaveToFile  (Widget, ReadWinP*, XtPointer);
   static void		DoSaveToButton(Widget, ReadWinP*, XtPointer);
   static void		DoPrint       (Widget, ReadWinP*, XtPointer);
   static void		DoPipe        (Widget, ReadWinP*, XtPointer);
   static void		DoSunToMime   (Widget, ReadWinP*, XtPointer);
   static void		DoDecrypt     (Widget, ReadWinP*, XtPointer);
   static void		DoAuth        (Widget, ReadWinP*, XtPointer);
   static void		DoEdit	      (Widget, ReadWinP*, XtPointer);
   static void		DoUndelete    (Widget, ReadWinP*, XtPointer);
   static void		DoDelete      (Widget, ReadWinP*, XtPointer);
   static void		DoDelClose    (Widget, ReadWinP*, XtPointer);
   static void		DoClose       (Widget, ReadWinP*, XtPointer);
   static void		DoHide        (ReadWinC*, void*);
   static void          PrepareQuickMenu(Widget, ReadWinP*, XtPointer);

//
// Reply menu widgets
//
   Widget		replyCB;

//
// Reply menu callbacks
//
   static void		DoReply       (Widget, ReadWinP*, XtPointer);
   static void		DoReplyInc    (Widget, ReadWinP*, XtPointer);
   static void		DoReplyAll    (Widget, ReadWinP*, XtPointer);
   static void		DoReplyAllInc (Widget, ReadWinP*, XtPointer);
   static void		DoForward     (Widget, ReadWinP*, XtPointer);
   static void		DoForward822  (Widget, ReadWinP*, XtPointer);
   static void		DoResend      (Widget, ReadWinP*, XtPointer);

//
// Option menu widgets
//
   Widget		optHeadTB;
   Widget		optWrapTB;
   Widget		optViewCB;
   Widget		viewFlatTB;
   Widget		viewOutlineTB;
   Widget		viewNestedTB;
   Widget		viewSourceTB;

//
// Option menu callbacks
//
   static void		DoOptPref(Widget, ReadWinP*, XtPointer);
   static void		DoOptButt(Widget, ReadWinP*, XtPointer);
   static void		DoOptHead(Widget, ReadWinP*,
					  XmToggleButtonCallbackStruct*);
   static void		DoOptWrap(Widget, ReadWinP*,
					  XmToggleButtonCallbackStruct*);
   static void		DoOptView(Widget, ReadWinP*,
					  XmToggleButtonCallbackStruct*);

//
// Mime part popup menu widgets
//
   Widget		mimePU;
   Widget		mimePUShowPB;
   Widget		mimePUHidePB;
   Widget		mimePUSavePB;
   Widget		mimePUPrintPB;
   Widget		mimePUFetchPB;
   Widget		mimePULabel;

//
// Mime part popup menu callbacks
//
   static void		PostPartMenu(ReadIconC*, ReadWinP*);
   static void		DoShowPart (Widget, ReadWinP*, XtPointer);	// Menu
   static void		DoHidePart (Widget, ReadWinP*, XtPointer);
   static void		DoSavePart (Widget, ReadWinP*, XtPointer);
   static void		DoPrintPart(Widget, ReadWinP*, XtPointer);
   static void		DoFetchPart(Widget, ReadWinP*, XtPointer);

//
// Stuff for displaying parts
//
   ReadIconC		*popupIcon;

   static void		HandleDoubleClick(ReadIconC*, ReadWinP*);
   static void		FinishShowPart(ReadIconC*, ReadWinP*);
   static void		HideTextWin(LocalTextWinC*, ReadWinP*);
   static void		HideMsgWin(ReadWinC*, ReadWinP*);

   void			Show822(ReadIconC*);
   void			ShowText(ReadIconC*);
   void			ShowIcon(ReadIconC*);
   void			HideIcon(ReadIconC*);

//
// External display stuff
//
   PtrListC		displayDataList;
   static PtrListC	*displayPixmaps;

   void			LoadDisplayPixmaps(ReadIconC*);
   void			ShowExternal(ReadIconC*, MailcapC*);
   static void		DisplayDone(int, DisplayDataT*);
   DisplayDataT		*DisplayData(ReadIconC*);
   static void		CryptDone(int, DisplayDataT*);

//
// Stuff for when there's no mailcap entry for a particular part
//
   ReadIconC		*noMailcapIcon;

   void			NoMailcap(ReadIconC*);
   static void		NoMailcapCancel(Widget, ReadWinP*, XtPointer);
   static void		NoMailcapSave  (Widget, ReadWinP*, XtPointer);
   static void		NoMailcapText  (Widget, ReadWinP*, XtPointer);

//
// External retrieval stuff
//
   static PtrListC	*fetchDataList;
   static PtrListC	*fetchPixmaps;

   Boolean		OkToGet(MsgPartC*, char*);
   Boolean		OkToSend(MsgPartC*);
   void			FetchPart(ReadIconC*, const char*, CallbackFn*);
   static void		FetchDone(int, FetchDataT*);
   void			LoadFetchPixmaps(ReadIconC*);
   FetchDataT		*FetchData(MsgPartC*);

//
// Stuff for saving parts
//
   FileChooserWinC	*savePartWin;

   static void		FinishSavePart(StringListC*, ReadWinP*);
   static void		SaveAfterFetch(ReadIconC*, ReadWinP*);
   void			SavePart(ReadIconC*, const char*);

//
// Stuff for printing parts
//
   static void		FinishPrintPart(ReadIconC*, ReadWinP*);
   static void		PrintDone(int, PrintDataT*);

//
// Recent folder menu stuff
//
   time_t	recentMenuTime;

   static void	RecentListChanged(void*, ReadWinP*);
   void		BuildRecentFolderMenu();

//
// Stuff for editing message
//
   SendWinC	*editWin;

   static void	EditFinished(char*, ReadWinP*);

//
// Main window widgets
//
   Widget		msgTitle;
   Widget		pushPinTB;
   Widget		msgPanes;
   MimeRichTextC	*msgHeadText;
   MimeRichTextC	*msgBodyText;
   PixmapC		*readIcon;

//
// Window callbacks
//

//
// Drag and drop callbacks
//
   static void		MsgTextDrop(Widget, XtPointer,
					    XmDropProcCallbackStruct*);
   static void		MsgDragOver(Widget, XtPointer,
					    XmDragProcCallbackStruct*);

//
// Resources
//
   Pixel		asciiBg;
   Pixel		imageBg;
   Pixel		audioBg;
   Pixel		videoBg;
   Pixel		asciiFg;
   Pixel		imageFg;
   Pixel		audioFg;
   Pixel		videoFg;
   StringC		asciiShowStr;
   StringC		imageShowStr;
   StringC		audioShowStr;
   StringC		videoShowStr;
   StringC		asciiHideStr;
   StringC		imageHideStr;
   StringC		audioHideStr;
   StringC		videoHideStr;

//
// State
//
   ReadViewTypeT	viewType;

//
// Windows
//
   PipeWinC		*pipeWin;
   PrintWinC		*printWin;
   LoginWinC		*loginWin;
   PtrListC		textWinList;	// List of available text windows
   PtrListC		msgWinList;	// List of available msg windows

//
// Resources
//
   StringC		msgTitleStr;
   StringC		noMsgTitleStr;

   void			AddBodyPart(MsgPartC*, Boolean doNext=True);
   void			BuildFileMenu();
   void			BuildReplyMenu();
   void			BuildOptMenu();
   void			BuildMimePopupMenu();
   LocalTextWinC	*GetTextWindow();
   ReadWinC		*GetMsgWindow();
   ReadIconC		*IconWithPart(MsgPartC*);
   ReadIconC		*IconWithWin (ReadWinC*);

public:

// Methods

   ReadWinP(ReadWinC*);
   ~ReadWinP();

   void		BuildMenus();
   void		BuildWidgets();
   void		DisplayBody();
   void		DisplayHeaders();
   void		EnableButtons();
   void		ReadResources();
   void		Reset();
   void		UpdateSaveFolder();
   ReadIconC	*NextReadableIcon(ReadWinC*);
   ReadIconC	*PrevReadableIcon(ReadWinC*);
   void		ShowNext822(ReadWinC*);
   void		ShowPrev822(ReadWinC*);

};

#endif // _ReadWinP_h_
