/*
 *  $Id: SendWinP.h,v 1.3 2001/07/28 18:26:03 evgeny Exp $
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
#ifndef _SendWinP_h_
#define _SendWinP_h_

#include "MsgListC.h"
#include "MimeTypes.h"
#include "MailPrefC.h"

#include <hgl/StringC.h>
#include <hgl/StringListC.h>
#include <hgl/PtrListC.h>
#include <hgl/CallbackC.h>

#include <Xm/DragDrop.h>

#include <sys/types.h>

class MimeRichTextC;
class RowColC;
class ButtonMgrC;
class PixmapC;
class AddressC;
class HeaderValC;
class PickAliasWinC;
class IncludeWinC;
class SendWinC;
class SendIconC;
class FileChooserWinC;
class WidgetListC;

/*---------------------------------------------------------------
 *  Data about one of the two header panels
 */

typedef struct {

   Widget		headForm;
   Widget		toForm;
   Widget		subForm;
   Widget		ccForm;
   Widget		bccForm;
   Widget		fccForm;
   Widget		otherForm;
   Widget		toLabel;
   Widget		subLabel;
   Widget		ccLabel;
   Widget		bccLabel;
   Widget		fccLabel;
   Widget		otherLabel;
   MimeRichTextC	*toText;
   MimeRichTextC	*subText;
   MimeRichTextC	*ccText;
   MimeRichTextC	*bccText;
   MimeRichTextC	*fccText;
   MimeRichTextC	*otherText;
   Widget		toAliasPB;
   Widget		subDummyPB;	// Place holder
   Widget		ccAliasPB;
   Widget		bccAliasPB;
   Widget		fccDummyPB;	// Place holder
   Widget		otherDummyPB;	// Place holder

} HeaderPaneT;

//=======================================================================

class SendWinP {

   friend class SendWinC;

   SendWinC		*pub;	// Public part

//
// File menu widgets
//
   Widget		fileEditPB;
   Widget		fileSpellPB;
   Widget		fileIncCB;
   Widget		fileAddCB;
   Widget		fileSavePB;
   Widget		fileLoadPB;
   Widget		fileClearPB;

//
// File menu callbacks
//
   static void		DoEdit          (Widget, SendWinP*, XtPointer);
   static void		DoSpell         (Widget, SendWinP*, XtPointer);
   static void		DoSaveFile      (Widget, SendWinP*, XtPointer);
   static void		DoLoadFile      (Widget, SendWinP*, XtPointer);
   static void		DoClear         (Widget, SendWinP*, XtPointer);
   static void		DoClose         (Widget, SendWinP*, XtPointer);
   static void		DoCheckNow      (Widget, SendWinP*, XtPointer);
   static void		DoSend          (Widget, SendWinP*, XtPointer);
   static void		DoSendKeep      (Widget, SendWinP*, XtPointer);
   static void		DoSendClose     (Widget, SendWinP*, XtPointer);
   static void		DoCancel        (Widget, SendWinP*, XtPointer);
   static void		DoIncludeText   (Widget, SendWinP*, XtPointer);
   static void		DoIncludeFile   (Widget, SendWinP*, XtPointer);
   static void		DoIncludeMsg    (Widget, SendWinP*, XtPointer);
   static void		DoIncludeExtPSig(Widget, SendWinP*, XtPointer);
   static void		DoIncludeExtESig(Widget, SendWinP*, XtPointer);
   static void		DoIncludeIntPSig(Widget, SendWinP*, XtPointer);
   static void		DoIncludeIntESig(Widget, SendWinP*, XtPointer);
   static void		DoAddAlt        (Widget, SendWinP*, XtPointer);
   static void		DoAddDigest     (Widget, SendWinP*, XtPointer);
   static void		DoAddMixed      (Widget, SendWinP*, XtPointer);
   static void		DoAddParallel   (Widget, SendWinP*, XtPointer);

//
// Edit menu widgets
//
   Widget		editCB;
   Widget		editPD;
   Widget		editUndeletePB;
   Widget		editPlainPB;
   Widget		editBoldPB;
   Widget		editItalicPB;
   Widget		editFixedPB;
   Widget		editUnderPB;
   Widget		editBiggerPB;
   Widget		editSmallerPB;
   Widget		editColorCB;
   Widget		editLeftPB;
   Widget		editRightPB;
   Widget		editBothPB;
   Widget		editCenterPB;
   Widget		editNoFillPB;
   Widget		editExcMorePB;
   Widget		editExcLessPB;
   Widget		editLeftInPB;
   Widget		editLeftOutPB;
   Widget		editRightInPB;
   Widget		editRightOutPB;

//
// Edit menu callbacks
//
   static void		DoUndelete       (Widget, SendWinP*, XtPointer);
   static void		DoPlain          (Widget, SendWinP*, XtPointer);
   static void		DoBold           (Widget, SendWinP*, XtPointer);
   static void		DoItalic         (Widget, SendWinP*, XtPointer);
   static void		DoFixed          (Widget, SendWinP*, XtPointer);
   static void		DoUnderline      (Widget, SendWinP*, XtPointer);
   static void		DoBigger         (Widget, SendWinP*, XtPointer);
   static void		DoSmaller        (Widget, SendWinP*, XtPointer);
   static void		DoFlushLeft      (Widget, SendWinP*, XtPointer);
   static void		DoFlushRight     (Widget, SendWinP*, XtPointer);
   static void		DoFlushBoth      (Widget, SendWinP*, XtPointer);
   static void		DoCenter         (Widget, SendWinP*, XtPointer);
   static void		DoNoFill         (Widget, SendWinP*, XtPointer);
   static void		DoExcerptMore    (Widget, SendWinP*, XtPointer);
   static void		DoExcerptLess    (Widget, SendWinP*, XtPointer);
   static void		DoLeftMarginIn   (Widget, SendWinP*, XtPointer);
   static void		DoLeftMarginOut  (Widget, SendWinP*, XtPointer);
   static void		DoRightMarginIn  (Widget, SendWinP*, XtPointer);
   static void		DoRightMarginOut (Widget, SendWinP*, XtPointer);

//
// Color menu callbacks
//
   static void		DoColorRed    (Widget, SendWinP*, XtPointer);
   static void		DoColorGreen  (Widget, SendWinP*, XtPointer);
   static void		DoColorBlue   (Widget, SendWinP*, XtPointer);
   static void		DoColorYellow (Widget, SendWinP*, XtPointer);
   static void		DoColorMagenta(Widget, SendWinP*, XtPointer);
   static void		DoColorCyan   (Widget, SendWinP*, XtPointer);
   static void		DoColorBlack  (Widget, SendWinP*, XtPointer);
   static void		DoColorWhite  (Widget, SendWinP*, XtPointer);
   static void		DoColorOther  (Widget, SendWinP*, XtPointer);
   static void		DoColorNone   (Widget, SendWinP*, XtPointer);

//
// Option menu widgets
//
   Widget		optCcTB;
   Widget		optBccTB;
   Widget		optFccTB;
   Widget		optOtherTB;
   Widget		optWrapTB;
   Widget		optCheckAddrTB;
   Widget		optMsgTypeCB;
   Widget		optMsgPlainTB;
   Widget		optMsgMimeTB;
   Widget		optMsgAltTB;
   Widget		optTextTypeCB;
   Widget		optTextPlainTB;
   Widget		optTextRichTB;

//
// Option menu callbacks
//
   static void		DoWinPrefs  (Widget, SendWinP*, XtPointer);
   static void		DoMailPrefs (Widget, SendWinP*, XtPointer);
   static void		DoReplyPrefs(Widget, SendWinP*, XtPointer);
   static void		DoSigPrefs  (Widget, SendWinP*, XtPointer);
   static void		DoButtons   (Widget, SendWinP*, XtPointer);
   static void		ToggleCc    (Widget, SendWinP*,
   					     XmToggleButtonCallbackStruct*);
   static void		ToggleBcc   (Widget, SendWinP*,
   					     XmToggleButtonCallbackStruct*);
   static void		ToggleFcc   (Widget, SendWinP*,
   					     XmToggleButtonCallbackStruct*);
   static void		ToggleOther (Widget, SendWinP*,
   					     XmToggleButtonCallbackStruct*);
   static void		ToggleWrap  (Widget, SendWinP*,
   					     XmToggleButtonCallbackStruct*);
   static void		ToggleMime  (Widget, SendWinP*,
					     XmToggleButtonCallbackStruct*);
   static void		ToggleMsgPlain (Widget, SendWinP*,
					        XmToggleButtonCallbackStruct*);
   static void		ToggleTextPlain(Widget, SendWinP*,
					        XmToggleButtonCallbackStruct*);

//
// Mime popup menu widgets
//
   Widget		mimePU;
   Widget		mimePULabel;
   Widget		mimePUEditPB;
   Widget		mimePUContPB;
   Widget		mimePUTypeCB;

//
// Mime popup menu callbacks
//
   static void		DoEditPart    (Widget, SendWinP*, XtPointer);
   static void		DoEditContents(Widget, SendWinP*, XtPointer);
   static void		DoDeletePart  (Widget, SendWinP*, XtPointer);
   static void		DoModAlt      (Widget, SendWinP*, XtPointer);
   static void		DoModDigest   (Widget, SendWinP*, XtPointer);
   static void		DoModMixed    (Widget, SendWinP*, XtPointer);
   static void		DoModParallel (Widget, SendWinP*, XtPointer);

//
// Main window widgets
//
   Widget		appPanes;
   HeaderPaneT		*oldHeadPane;
   HeaderPaneT		*newHeadPane;
   HeaderPaneT		*curHeadPane;
   MimeRichTextC	*headText;
   Widget		bodyForm;
   MimeRichTextC	*bodyText;
   Widget		bodyStateTF;
   PixmapC		*compIcon;
   MimeRichTextC	*toText;
   MimeRichTextC	*ccText;
   MimeRichTextC	*bccText;
   MimeRichTextC	*fccText;
   MimeRichTextC	*subText;
   MimeRichTextC	*otherText;
   RowColC		*optRC;
   Widget		optMimeTB;
   Widget		optAddSigTB;
   Widget		optDigSignTB;
   Widget		optEncryptTB;

//
// Main window callbacks
//
   static void		DoPopup         (Widget, SendWinP*, XtPointer);
   static void		TextChanged     (MimeRichTextC*, SendWinP*);
   static void		BodyStateChanged(MimeRichTextC*, SendWinP*);

//
// Header field stuff
//
   int			toRow;
   int			ccRow;
   int			bccRow;
   int			fccRow;
   int			otherRow;
   StringC		toStr;
   StringC		ccStr;
   StringC		fccStr;
   StringListC		fccList;
   StringC		otherStr;
   StringListC		recipientList;

   Boolean		AddRecipients(StringC&);
   Boolean		BuildHeaders(StringListC&, StringListC&);
   Boolean		ProcessHeaderFields();
   void			RemoveFiles(StringC&);
   void			RemoveQuotes(StringC&);

//
// Drag and drop stuff
//
   static Atom		graphicAtom;

   static void	HandleDragOver(Widget, XtPointer, XmDragProcCallbackStruct*);
   static void	HandleDrop    (Widget, XtPointer, XmDropProcCallbackStruct*);

   void		HandleGraphicDrop(Widget, XmDropProcCallbackStruct*);
   void		HandleMessageDrop(Widget, XmDropProcCallbackStruct*);

//
// Action procs
//
   static XtActionsRec	actions[2];
   static void	HandleExpandAliases  (Widget, XKeyEvent*, String*, Cardinal*);
   static void	HandleExpandSelection(Widget, XKeyEvent*, String*, Cardinal*);

//
// Clearing
//
   Widget		clearWin;

   static void		FinishClear(Widget, SendWinP*, XtPointer);

//
// Replying, Forwarding and resending
//
   MsgListC		msgList;
   StringListC		resendFileList;

   void			ForwardMsgEncap(MsgC*);
   void			ForwardMsgInline(MsgC*);

//
// External editor and spell checker
//
   pid_t		edit_pid;
   pid_t		spell_pid;
   Boolean		filterEnriched;

   pid_t		FilterBody(StringC&, CallbackFn*);
   void			ReadFilter(CharC);

   static void		EditFinished (char*, SendWinP*);
   static void		SpellFinished(char*, SendWinP*);
   static void		FileModified (char*, SendWinP*);

//
// Building message and sending
//
   Boolean		externalSig;
   StringC		sigStr;
   StringC		esigStr;

   void			BuildSig(Boolean);
   Boolean		CopyFilePart(FILE*, FILE*, u_int);
   Boolean		NullBodyOk();
   Boolean		NullSubjectOk();
   Boolean		Send(Boolean closing=False);
   Boolean		OkToSendPlain(OutgoingMailTypeT*);
   Boolean		OkToSendPlain(MimeContentType*);
   Boolean		OkToSend8Bit(OutgoingMailTypeT*);
   Boolean		OkToSendTo(CharC);
   Boolean		OkToSplitPlain(OutgoingMailTypeT*);
   Boolean		PerformFcc(StringListC&, char*, CharC);
   Boolean		SendMessage(char*, StringListC&, OutgoingMailTypeT);
   Boolean		SendSplitMessage(char*, StringListC);
   Boolean		WriteBody(char*, StringListC&, StringListC&,
   				  OutgoingMailTypeT*, Boolean saving=False);
   Boolean		WriteMimeBody(FILE*, StringListC&, StringListC&,
   				      OutgoingMailTypeT, MimeContentType,
				      Boolean);
   Boolean		WriteMultipartBody(FILE*, CharC, MimeContentType,
					   CharC*, Boolean);
   Boolean		WritePlainBody(FILE*, StringListC&, StringListC&,
				       Boolean, Boolean);
   Boolean		WriteTextPart(FILE*, MimeContentType, StringC&,
				      CharC*, StringListC*);
   Boolean		WriteCryptBody(char*, StringC&, StringListC&,
   				     OutgoingMailTypeT, Boolean, Boolean);

//
// Checking addresses
//
   Boolean		checkDone;	// Used in CheckAddresses
   StringC		checkOutput;
   int			checkStatus;
   pid_t		checkPid;

   Boolean		CheckAddresses(Boolean reportSuccess=False);
   static void		CheckDone(int, SendWinP*);

//
// Picking aliases
//
   PickAliasWinC	*pickAliasWin;

   static void		PickToAlias (Widget, SendWinP*, XtPointer);
   static void		PickCcAlias (Widget, SendWinP*, XtPointer);
   static void		PickBccAlias(Widget, SendWinP*, XtPointer);
   static void		AddToAlias  (PickAliasWinC*, SendWinP*);
   static void		AddCcAlias  (PickAliasWinC*, SendWinP*);
   static void		AddBccAlias (PickAliasWinC*, SendWinP*);

//
// Including messages
//
   Widget		incMsgWin;
   Widget		incMsgAsTextTB;
   Widget		incMsgAsIconTB;

   static void		IncludeMsgOk    (Widget, SendWinP*, XtPointer);
   static void		IncludeMsgCancel(Widget, SendWinP*, XtPointer);

//
// Including files
//
   FileChooserWinC	*incFileWin;
   FileChooserWinC	*incTextWin;
   Widget		incTextAsTextTB;
   Widget		incTextAsIconTB;

   static void		FinishIncludeFile(StringListC*, SendWinP*);
   static void		FinishIncludeText(StringListC*, SendWinP*);
   static void		IncludeFileOk    (IncludeWinC*, SendWinP*);
   static void		IncludeFileHide  (IncludeWinC*, SendWinP*);

//
// Editing attachments
//
   SendIconC		*popupIcon;
   SendIconC		*modIcon;
   SendIconC		*editIcon;	// message/rfc822 part being edited
   Boolean		modifying;
   IncludeWinC		*fileDataWin;
   PtrListC		editWinList;
   static PtrListC	*editPixmaps;

   MsgC			*editMsg;	// message being edited
   Boolean		editMsgText;	// No special processing required
   CallbackC		editDoneCall;	// Called when finished and changed

   SendWinC		*GetEditWin();
   void			Edit(SendIconC*);
   void			LoadEditPixmaps(SendIconC*);
   void			UpdateEditButtons();

   static void		OpenPart    (SendIconC*, SendWinP*);
   static void		PostPartMenu(SendIconC*, SendWinP*);

//
// Deleting attachments
//
   Widget		deleteWin;

   static void		FinishDelete(Widget, SendWinP*, XtPointer);

//
// Saving to a file
//
   FileChooserWinC	*saveFileWin;
   FileChooserWinC	*loadFileWin;

   static void		FinishSaveFile(StringListC*, SendWinP*);
   static void		FinishLoadFile(StringListC*, SendWinP*);
   Boolean		OkToSaveTo(StringC&);
   Boolean		Save(char *file=NULL);

//
// Auto-saving
//
   int			keystrokeCount;	// For auto-save
   StringC		autoSaveFile;
   XtIntervalId		autoSaveTimer;

   static void		CheckAutoSave(SendWinP*, XtIntervalId*);

//
// Private data
//
   ButtonMgrC		*buttMgr;
   MimeContentType	containerType;
   StringC		descTemplate;	// Used by SendIconC for descriptions
   static PtrListC	*winList;	// List of all composition windows
   					//    (including edit windows)

//
// Private methods
//
   void			AddContainer(MimeContentType);
   void			BuildEditMenu();
   void			BuildFileMenu();
   void			BuildMimePopupMenu();
   void			BuildNewHeadPane();
   void			BuildOldHeadPane();
   void			BuildOptMenu();
   void			InitHeaderPane(HeaderPaneT*);
   Boolean		OkToClose();
   void			Place2HeaderFields(WidgetListC&);
   void			Place3HeaderFields(WidgetListC&);
   void			Place4HeaderFields(WidgetListC&);
   void			Place5HeaderFields(WidgetListC&);
   void			Place6HeaderFields(WidgetListC&);

public:

   SendWinP(SendWinC*);
   ~SendWinP();

   void			AddBodyPart(MsgPartC*, Boolean doChildren=True);
   void			AddBodyTree(MsgPartC*);
   void			BuildMenus();
   void			BuildWidgets();
   void			DisplayBody(MsgC*, Boolean forceText=False);
   void			PlaceHeaderFields();
   void			SetField(MimeRichTextC*, AddressC*);
   void			SetResendMode(Boolean);
   void			UpdateFcc();

#if 000
//
// Widgets
//
   Widget		optSettingsTB;

   Widget		optForm;

   RowColC		*optRC;

   Boolean		ccVis;
   Boolean		bccVis;
   Boolean		fccVis;
   Boolean		otherVis;
   int			maxFieldsPerLine;

   static ButtonWinC	*buttWin;

   FileGraphicC		*dragOverPart;

   StringC		bodyStr;
   StringC		plainStr;
   StringC		headStr;
   StringListC		headList;
   Boolean		selfInducedMod; // For text fields
   Boolean		editOnly;	// True if this is a secondary window
   Boolean		changed;	// True if some editing done since last
   					//   send
   Boolean		sendingPlain;	// Send message as plain text

//
// Resources
//
   Pixel		asciiBg;
   Pixel		asciiFg;
   Pixel		audioBg;
   Pixel		audioFg;
   Pixel		imageBg;
   Pixel		imageFg;
   Pixel		multiBg;
   Pixel		multiFg;
   Pixel		videoBg;
   Pixel		videoFg;

//
// Callbacks
//




   static void		HandleIconInput(Widget, SendWinC*,
   					XmDrawingAreaCallbackStruct*);
   static void		HandleResize  (Widget, SendWinP*, XEvent*, Boolean*);
   static void		IconFocusChange(IconC*, SendWinC*);
   static void		ToggleSettings(Widget, SendWinC*,
				       XmToggleButtonCallbackStruct*);
   static void		ReplacePart   (Widget, SendWinC*, XtPointer);


//
// Private methods
//
   void			CompressTree(MimePartC*);
   void			DecodeHeader(StringC&);
   void			FoldHeader(StringC&);
   void			IncludeSingFile();
   void			IncludeMultFiles(StringListC&);
   Boolean		SendFile(char*, Boolean wait=False);
   Boolean		StoreMessage(const char*);
   Boolean		Write(char *file=NULL);
#endif
};

#endif // _SendWinP_h_
