/*
 * $Id: FileChooserWinC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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

#ifndef _FileChooserWinC_h_
#define _FileChooserWinC_h_

#include <hgl/HalDialogC.h>
#include <hgl/StringC.h>
#include <hgl/RegexC.h>
#include <hgl/CallbackC.h>
#include <hgl/StringListC.h>

#include <time.h>

class FileChooserWinC : public HalDialogC {

//
// Widgets
//
   Widget		panedWin;
   Widget		selectForm;
   Widget		listTB;
   Widget		listForm;
   Widget		dirForm;
   Widget		dirNameTF;
   Widget		dirList;
   Widget		fileForm;
   Widget		filterLabel;
   Widget		filterTF;
   Widget		fileListLabel;
   Widget		fileList;
   Widget		selectLabel;
   Widget		selectTextForm;
   Widget		selectTextSW;
   Widget		selectText;
   Widget		selectTF;
   Widget		imapForm;
   Widget		imapTB;
   Widget		imapTF;
   Widget		okPB;
   Widget		cancelPB;

//
// Data
//
   StringC		dirName;
   StringC		defaultDir;
   StringC		filterStr;
   RegexC		filterPat;
   Boolean		singleSelect;
   Boolean		showDirsInFileList;
   Boolean		showFilesInFileList;
   Boolean		acceptFileDoubleClick;
   StringListC		subdirNames;
   StringListC		fileNames;
   StringListC		filtNames;
   StringListC		selectNames;
   CallbackListC	verifyCalls;
   CallbackListC	okCalls;
   Boolean		localTextChange;
   Boolean		hideOk;
   Boolean		listVisible;
   time_t		lastTimeListed;
   Boolean		showImap;

   static XtActionsRec	actions[1];
//
// Callbacks
//
   static void DoChooseDir    (Widget, FileChooserWinC*, XmListCallbackStruct*);
   static void DoChooseFile   (Widget, FileChooserWinC*, XmListCallbackStruct*);
   static void DoClear        (Widget, FileChooserWinC*, XtPointer);
   static void DoFileSelection(Widget, FileChooserWinC*, XmListCallbackStruct*);
   static void DoFilter       (Widget, FileChooserWinC*, XtPointer);
   static void DoSelectDir    (Widget, FileChooserWinC*, XmListCallbackStruct*);
   static void DoSetDir       (Widget, FileChooserWinC*, XtPointer);
   static void DoOk           (Widget, FileChooserWinC*, XtPointer);
   static void DoPopup        (Widget, FileChooserWinC*, XtPointer);
   static void SelectTextChanged(Widget, FileChooserWinC*, XtPointer);
   static void SelectTFChanged(Widget, FileChooserWinC*, XtPointer);
   static void ToggleList     (Widget, FileChooserWinC*,
			       XmToggleButtonCallbackStruct*);
   static void HandleActivate (Widget, XKeyEvent*, String*, Cardinal*);
   static void AutoSelectImap (Widget, FileChooserWinC*, XtPointer);

//
// Private methods
//
   StringC		BaseName(StringC&);
   void			FilterFiles();
   Boolean		GetImapServerName(StringC&);
   void			ListDirectory(const char*, StringListC&, StringListC&);
   void			ListDirectoryImap(CharC, StringListC&, StringListC&);
   void			SetList(Widget, StringListC&);
   void			UpdateSelectText();

public:

//
// Constructor and destructor
//
   FileChooserWinC(Widget, char *name="fileChooserWin", ArgList argv=NULL,
		   Cardinal argc=0);
   ~FileChooserWinC();

//
// Register callbacks
//
   inline void	AddVerifyCallback(CallbackFn *fn, void *data) {
      AddCallback(verifyCalls, fn, data);
   }
   inline void	RemoveVerifyCallback(CallbackFn *fn, void *data) {
      RemoveCallback(verifyCalls, fn, data);
   }
   inline void	AddOkCallback(CallbackFn *fn, void *data) {
      AddCallback(okCalls, fn, data);
   }
   inline void	RemoveOkCallback(CallbackFn *fn, void *data) {
      RemoveCallback(okCalls, fn, data);
   }

//
// Public methods
//
   void			ClearSelection();
   void			ExpandSelectedNames();
   void			ExpandWildcards();
   void			HideImap();
   void			HideList();
   void			HideOk(Boolean val) { hideOk = val; }
   void			SetDefaultDir(const char*);
   void			SetDirectory(StringC);
   void			Show() { HalDialogC::Show(); }
   void			Show(Widget);
   void			ShowImap();
   void			ShowList();
   void			ShowDirsInFileList(Boolean);
   void			ShowFilesInFileList(Boolean);
   void			SingleSelect(Boolean);
   Boolean		UsingImap() const;
   MEMBER_QUERY(Widget,		CancelPB,	cancelPB);
      PTR_QUERY(StringC&,	Directory,	dirName);
   MEMBER_QUERY(Widget,		FileListLabel,	fileListLabel);
   MEMBER_QUERY(Widget,		FilterLabel,	filterLabel);
   MEMBER_QUERY(Widget,		OkPB,		okPB);
   MEMBER_QUERY(Widget,		PanedWin,	panedWin);
   MEMBER_QUERY(Widget,		SelectLabel,	selectLabel);
   MEMBER_QUERY(Widget,		SelectText,	selectText);
   MEMBER_QUERY(Widget,		SelectTF,	selectTF);
      PTR_QUERY(StringListC&,	Names,		selectNames);
};

#endif // _FileChooserWinC_h_
