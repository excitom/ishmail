/*
 * $Id: FileChooserWinC.C,v 1.3 2000/07/24 11:39:52 evgeny Exp $
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

#include <config.h>

#include "FileChooserWinC.h"
#include "FileMisc.h"
#include "ShellExp.h"
#include "IshAppC.h"
#include "AppPrefC.h"
#include "ImapServerC.h"
#include "ImapMisc.h"
#include "FolderPrefC.h"

#include <hgl/WArgList.h>
#include <hgl/HalAppC.h>
#include <hgl/WXmString.h>
#include <hgl/rsrc.h>
#include <hgl/RegexC.h>
#include <hgl/TextMisc.h>
#include <hgl/CharC.h>

#include <Xm/PanedW.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/TextF.h>
#include <Xm/List.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/Text.h>
#include <Xm/ScrolledW.h>
#include <Xm/Separator.h>

#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

XtActionsRec	FileChooserWinC::actions[1] = {
   "FileChooserWinC-activate", (XtActionProc)FileChooserWinC::HandleActivate,
};

/*---------------------------------------------------------------
 * Function to convert shell wildcard patterns into regex wildcard patterns
 */

static StringC
ShellToRegex(StringC str)
{
//
// Look at each character in string
//
   unsigned	count = str.size();
   int i=0; for (i=0; i<count; i++) {

      char	c = str[i];
      if ( c == '*' ) {
	 if ( i == 0 || str[i-1] != '\\' ) {
	    str(i,0) = ".";
	    count++;
	    i++;
	 }
      }
      else if ( c == '?' ) {
	 if ( i == 0 || str[i-1] != '\\' ) {
	    str(i,1) = ".";
	 }
      }
      else if ( c == '.' ) {
	 str(i,0) = "\\";
	 count++;
	 i++;
      }
   }

   str += "$";
   return str;

} // End ShellToRegex

/*-----------------------------------------------------------------------
 *  FileChooserWinC constructor
 */

FileChooserWinC::FileChooserWinC(Widget parent, char* name, ArgList argv,
				 Cardinal argc)
 : HalDialogC(name, parent, argv, argc)
{
   WArgList	args;
   Widget	wlist[4];
   char		*cl = "FileChooserC";

   localTextChange = False;
   listVisible     = False;
   lastTimeListed  = 0;
   defaultDir      = "";
   showImap = get_boolean(cl, *this, "showImapField",
				     ishApp->appPrefs->usingImap);

   subdirNames.AllowDuplicates(FALSE);
   subdirNames.SetSorted(FALSE);
   fileNames.AllowDuplicates(FALSE);
   fileNames.SetSorted(FALSE);
   filtNames.AllowDuplicates(FALSE);
   filtNames.SetSorted(FALSE);
   selectNames.AllowDuplicates(FALSE);
   selectNames.SetSorted(TRUE);
   selectNames.AutoShrink(FALSE);

//
// Create the appForm children
//
// appForm
//    PanedWindow	panedWin
//
   args.Reset();
   args.MarginHeight(0);
   args.MarginWidth(0);
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   panedWin = XmCreatePanedWindow(appForm, "FileChooserC", ARGS);

//
// Create the panedWin children
//
// panedWin
//    Form	selectForm
//    Form	listForm
//
   args.Reset();
   args.AllowResize(True);
   selectForm = XmCreateForm(panedWin, "selectForm", ARGS);
   listForm   = XmCreateForm(panedWin, "listForm",   ARGS);

//
// Create the selectForm children
//
// selectForm
//    Label		selectLabel
//    Form		selectTextForm
//    ToggleButton	listTB
//    Form		imapForm
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_NONE);
   selectLabel = XmCreateLabel(selectForm, "selectLabel", ARGS);
   XtManageChild(selectLabel);

   args.Reset();
   args.TopAttachment(XmATTACH_WIDGET, selectLabel);
   args.LeftAttachment(XmATTACH_NONE);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_NONE);
   listTB = XmCreateToggleButton(selectForm, "listTB", ARGS);
   XtManageChild(listTB);

   XtAddCallback(listTB, XmNvalueChangedCallback,
		 (XtCallbackProc)ToggleList, (XtPointer)this);

   args.Reset();
   args.TopAttachment(XmATTACH_NONE);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   imapForm = XmCreateForm(selectForm, "imapForm", ARGS);

   args.Reset();
   args.TopAttachment(XmATTACH_WIDGET, selectLabel);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_WIDGET, listTB);
   if ( showImap )
      args.BottomAttachment(XmATTACH_WIDGET, imapForm);
   else
      args.BottomAttachment(XmATTACH_FORM);
   selectTextForm = XmCreateForm(selectForm, "selectTextForm", ARGS);

//
// Create the selectTextForm widgets
//
// selectTextForm
//    Text	selectText -or-
//    TextField	selectTF
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   args.ScrollingPolicy(XmAUTOMATIC);
   args.ScrollBarDisplayPolicy(XmAS_NEEDED);  //TEST
   args.VisualPolicy(XmCONSTANT);             //TEST
   args.EditMode(XmMULTI_LINE_EDIT);
   args.UserData(this);
   selectText = CreateScrolledText(selectTextForm, "selectText", ARGS);
   selectTextSW = XtParent(selectText);
   XtManageChild(selectText);

   XtAddCallback(selectText, XmNvalueChangedCallback,
		 (XtCallbackProc)SelectTextChanged, (XtPointer)this);

   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_NONE);
   args.UserData(this);
   selectTF = CreateTextField(selectTextForm, "selectTF", ARGS);

   XtAddCallback(selectTF, XmNvalueChangedCallback,
		 (XtCallbackProc)SelectTFChanged, (XtPointer)this);
   XtAddCallback(selectTF, XmNactivateCallback,
		 (XtCallbackProc)DoOk, (XtPointer)this);

   XtManageChild(selectTF);
   XtVaSetValues(selectForm, XmNinitialFocus, selectTF, NULL);
   XtManageChild(selectTextForm);

//
// Add special activation translation
//
   static Boolean	actionsAdded = False;
   if ( !actionsAdded ) {
      XtAppAddActions(halApp->context, actions, XtNumber(actions));
      actionsAdded = True;
   }

//
// Create the imapForm children
//
// imapForm
//    ToggleButton	imapTB
//    TextField		imapTF
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   imapTB = XmCreateToggleButton(imapForm, "imapTB", ARGS);
   XtManageChild(imapTB);

   args.LeftAttachment(XmATTACH_WIDGET, imapTB);
   args.RightAttachment(XmATTACH_FORM);
   imapTF = CreateTextField(imapForm, "imapTF", ARGS);
   XtManageChild(imapTF);

   XtAddCallback(imapTF, XmNvalueChangedCallback,
		 (XtCallbackProc)AutoSelectImap, this);

   if ( showImap ) {
      TextFieldSetString(imapTF, ishApp->appPrefs->imapServer);
      XmToggleButtonSetState(imapTB,
			     ishApp->folderPrefs->defFolderType == IMAP_FOLDER,
			     False);
      XtManageChild(imapForm);
   }

//
// Create the listForm children
//
// listForm
//    Form	dirForm
//    Separator	dirFileSep
//    Form	fileForm
//
   args.Reset();
   args.LeftAttachment(XmATTACH_FORM);
   args.TopAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   dirForm = XmCreateForm(listForm, "dirForm", ARGS);

   args.Reset();
   args.LeftAttachment(XmATTACH_WIDGET, dirForm);
   args.TopAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   args.Orientation(XmVERTICAL);
   Widget dirFileSep = XmCreateSeparator(listForm, "dirFileSep", ARGS);

   args.Reset();
   args.LeftAttachment(XmATTACH_WIDGET, dirFileSep);
   args.TopAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   fileForm = XmCreateForm(listForm, "fileForm", ARGS);

//
// Create the dirForm children
//
// dirForm
//    Label		dirNameLabel
//    Form		dirNameForm
//    Label		dirListLabel
//    ScrolledList	dirList
//
   args.Reset();
   args.LeftAttachment(XmATTACH_FORM);
   args.TopAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_NONE);
   Widget	dirNameLabel = XmCreateLabel(dirForm, "dirNameLabel", ARGS);

   args.Reset();
   args.LeftAttachment(XmATTACH_FORM);
   args.TopAttachment(XmATTACH_WIDGET, dirNameLabel);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_NONE);
   Widget	dirNameForm = XmCreateForm(dirForm, "dirNameForm", ARGS);

   args.Reset();
   args.LeftAttachment(XmATTACH_FORM);
   args.TopAttachment(XmATTACH_WIDGET, dirNameForm);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_NONE);
   Widget	dirListLabel = XmCreateLabel(dirForm, "dirListLabel", ARGS);

   args.Reset();
   args.LeftAttachment(XmATTACH_FORM);
   args.TopAttachment(XmATTACH_WIDGET, dirListLabel);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   args.SelectionPolicy(XmSINGLE_SELECT);
   args.ListSizePolicy(XmRESIZE_IF_POSSIBLE);
   dirList = XmCreateScrolledList(dirForm, "dirList", ARGS);
   XtManageChild(dirList);

   XtAddCallback(dirList, XmNdefaultActionCallback, (XtCallbackProc)DoChooseDir,
		 (XtPointer)this);
   XtAddCallback(dirList, XmNsingleSelectionCallback,
		 (XtCallbackProc)DoSelectDir, (XtPointer)this);

//
// Create the dirNameForm children
//
// dirNameForm
//    TextField		dirNameTF
//    PushButton	dirNamePB
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_NONE);
   args.RightAttachment(XmATTACH_FORM);
   Widget	dirNamePB = XmCreatePushButton(dirNameForm, "dirNamePB", ARGS);

   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_WIDGET, dirNamePB);
   dirNameTF = CreateTextField(dirNameForm, "dirNameTF", ARGS);

   XtAddCallback(dirNamePB, XmNactivateCallback, (XtCallbackProc)DoSetDir,
		 (XtPointer)this);
   XtAddCallback(dirNameTF, XmNactivateCallback, (XtCallbackProc)DoSetDir,
		 (XtPointer)this);

   wlist[0] = dirNameTF;
   wlist[1] = dirNamePB;
   XtManageChildren(wlist, 2);	// dirNameForm children

   wlist[0] = dirNameLabel;
   wlist[1] = dirNameForm;
   wlist[2] = dirListLabel;
   XtManageChildren(wlist, 3);	// dirForm children

//
// Create the fileForm children
//
// fileForm
//    Label		filterLabel
//    Form		filterForm
//    Label		fileListLabel
//    ScrolledList	fileList
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_NONE);
   filterLabel = XmCreateLabel(fileForm, "filterLabel", ARGS);

   args.Reset();
   args.LeftAttachment(XmATTACH_FORM);
   args.TopAttachment(XmATTACH_WIDGET, filterLabel);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_NONE);
   Widget	filterForm = XmCreateForm(fileForm, "filterForm", ARGS);

   args.Reset();
   args.LeftAttachment(XmATTACH_FORM);
   args.TopAttachment(XmATTACH_WIDGET, filterForm);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_NONE);
   fileListLabel = XmCreateLabel(fileForm, "fileListLabel", ARGS);

   args.Reset();
   args.LeftAttachment(XmATTACH_FORM);
   args.TopAttachment(XmATTACH_WIDGET, fileListLabel);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   args.ListSizePolicy(XmRESIZE_IF_POSSIBLE);
   fileList = XmCreateScrolledList(fileForm, "fileList", ARGS);
   XtManageChild(fileList);

   XtAddCallback(fileList, XmNdefaultActionCallback,
		 (XtCallbackProc)DoChooseFile, (XtPointer)this);
   XtAddCallback(fileList, XmNbrowseSelectionCallback,
		 (XtCallbackProc)DoFileSelection, (XtPointer)this);
   XtAddCallback(fileList, XmNextendedSelectionCallback,
		 (XtCallbackProc)DoFileSelection, (XtPointer)this);
   XtAddCallback(fileList, XmNmultipleSelectionCallback,
		 (XtCallbackProc)DoFileSelection, (XtPointer)this);
   XtAddCallback(fileList, XmNsingleSelectionCallback,
		 (XtCallbackProc)DoFileSelection, (XtPointer)this);

//
// Create the filterForm children
//
// filterForm
//    TextField		filterTF
//    PushButton	filterPB
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_NONE);
   args.RightAttachment(XmATTACH_FORM);
   Widget	filterPB = XmCreatePushButton(filterForm, "filterPB", ARGS);

   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_WIDGET, filterPB);
   filterTF = CreateTextField(filterForm, "filterTF", ARGS);

   XtAddCallback(filterPB, XmNactivateCallback, (XtCallbackProc)DoFilter,
		 (XtPointer)this);
   XtAddCallback(filterTF, XmNactivateCallback, (XtCallbackProc)DoFilter,
		 (XtPointer)this);

   wlist[0] = filterTF;
   wlist[1] = filterPB;
   XtManageChildren(wlist, 2);	// filterForm children

   wlist[0] = filterLabel;
   wlist[1] = filterForm;
   wlist[2] = fileListLabel;
   XtManageChildren(wlist, 3);	// fileForm children

   wlist[0] = dirForm;
   wlist[1] = dirFileSep;
   wlist[2] = fileForm;
   XtManageChildren(wlist, 3);	// listForm children

   wlist[0] = selectForm;
//   wlist[1] = listForm;
   XtManageChildren(wlist, 1);	// panedWin children

   XtManageChild(panedWin);

//
// Create buttonRC hierarchy
//
//   buttonRC
//	PushButton	okPB
//	PushButton	clearPB
//	PushButton	cancelPB
//	PushButton	helpPB
//
   AddButtonBox();
   okPB           = XmCreatePushButton(buttonRC, "okPB",     0,0);
   Widget clearPB = XmCreatePushButton(buttonRC, "clearPB",  0,0);
   cancelPB       = XmCreatePushButton(buttonRC, "cancelPB", 0,0);
   Widget helpPB  = XmCreatePushButton(buttonRC, "helpPB",   0,0);

   XtAddCallback(okPB, XmNactivateCallback, (XtCallbackProc)DoOk,
   		 (XtPointer)this);
   XtAddCallback(clearPB, XmNactivateCallback, (XtCallbackProc)DoClear,
   		 (XtPointer)this);
   XtAddCallback(cancelPB, XmNactivateCallback, (XtCallbackProc)DoHide,
   		 (XtPointer)this);
   XtAddCallback(helpPB, XmNactivateCallback, (XtCallbackProc)HalAppC::DoHelp,
   		 (XtPointer)"helpcard");

   wlist[0] = okPB;
   wlist[1] = clearPB;
   wlist[2] = cancelPB;
   wlist[3] = helpPB;
   XtManageChildren(wlist, 4);	// buttonRC children

   XtSetSensitive(okPB, False);	// Wait until some are selected

   ShowInfoMsg();
   HandleHelp();

//
// Read resources
//
   unsigned char	spol;
   XtVaGetValues(fileList, XmNselectionPolicy, &spol, NULL);
   singleSelect = get_boolean(cl, *this, "singleSelect", spol==XmSINGLE_SELECT);
   if ( singleSelect && spol != XmSINGLE_SELECT )
      XtVaSetValues(fileList, XmNselectionPolicy, XmSINGLE_SELECT, NULL);

   showDirsInFileList    = get_boolean(cl, *this, "showDirsInFileList",  False);
   showFilesInFileList   = get_boolean(cl, *this, "showFilesInFileList", True);
   acceptFileDoubleClick = get_boolean(cl, *this, "acceptFileDoubleClick",True);

   dirName   = get_string(cl, *this, "directory", ".");
   if ( !IsDir(dirName) ) dirName = ".";

   filterStr = get_string(cl, *this, "filter",    "*");
   XmTextFieldSetString(filterTF, filterStr);

   if ( singleSelect ) {

      XtManageChild(selectTF);
      XtUnmanageChild(selectTextSW);
      XtVaSetValues(selectForm, XmNinitialFocus, selectTF, NULL);

//
// Set special titles for single selection
//
      StringC	str = get_string(cl, *this, "singleTitle", "Select File");
      if ( str.size() > 0 )
	 XtVaSetValues(*this, XmNtitle, (char*)str, XmNiconName, (char*)str, 0);

      str = get_string(cl, selectLabel, "singleString", "Selected File");
      if ( str.size() > 0 ) {
	 WXmString	wstr = (char*)str;
	 XtVaSetValues(selectLabel, XmNlabelString, (XmString)wstr, NULL);
      }

   } else {

      XtManageChild(selectTextSW);
      XtUnmanageChild(selectTF);
      XtVaSetValues(selectForm, XmNinitialFocus, selectText, NULL);
   }

   XtAddCallback(*this, XmNpopupCallback, (XtCallbackProc)DoPopup,
		 (XtPointer)this);

} // End constructor

/*-----------------------------------------------------------------------
 *  Destructor
 */

FileChooserWinC::~FileChooserWinC()
{
   DeleteCallbacks(verifyCalls);
   DeleteCallbacks(okCalls);
}

/*-----------------------------------------------------------------------
 *  Handle initial display
 */

void
FileChooserWinC::DoPopup(Widget, FileChooserWinC *This, XtPointer)
{
   if ( This->singleSelect ) {
//
// Fix size of selectForm since this is single selection mode
//
      Dimension	ht;
      XtVaGetValues(This->selectForm, XmNheight, &ht, NULL);
      XtVaSetValues(This->selectForm, XmNpaneMinimum, ht, XmNpaneMaximum, ht,0);
   }

   if ( XmToggleButtonGetState(This->listTB) )
      This->ShowList();
}

/*-----------------------------------------------------------------------
 *  Display list portion
 */

void
FileChooserWinC::ShowList()
{
   if ( listVisible ) return;

   XtManageChild(listForm);
   XmToggleButtonSetState(listTB, True, False);
   listVisible = True;
   SetDirectory(dirName);
}

/*-----------------------------------------------------------------------
 *  Display directories in file list
 */

void
FileChooserWinC::ShowDirsInFileList(Boolean val)
{
   if ( showDirsInFileList == val ) return;

   showDirsInFileList = val;
   if ( !listVisible ) return;

   SetDirectory(dirName);
}

/*-----------------------------------------------------------------------
 *  Display files in file list
 */

void
FileChooserWinC::ShowFilesInFileList(Boolean val)
{
   if ( showFilesInFileList == val ) return;

   showFilesInFileList = val;
   if ( !listVisible ) return;

   SetDirectory(dirName);
}

/*-----------------------------------------------------------------------
 *  Removed list portion
 */

void
FileChooserWinC::HideList()
{
   if ( !listVisible ) return;

   XtUnmanageChild(listForm);
   XmToggleButtonSetState(listTB, False, False);
   listVisible = False;

//
// Re-read the names in the selected text field.  They could possibly have
//    been expanded to full pathnames internally.
//
   if ( singleSelect ) SelectTFChanged(NULL, this, NULL);
   else		       SelectTextChanged(NULL, this, NULL);
}

/*-----------------------------------------------------------------------
 *  Method to clear current selection
 */

void
FileChooserWinC::ClearSelection()
{
   XmListDeselectAllItems(fileList);
   XmTextSetString(selectText, "");
   XmTextSetString(selectTF, "");
}

/*-----------------------------------------------------------------------
 *  Display dialog
 */

void
FileChooserWinC::Show(Widget parent)
{
   WArgList	args;
   if ( showImap ) {
      XtManageChild(imapForm);
      args.BottomAttachment(XmATTACH_WIDGET, imapForm);
   }

   else {
      XtUnmanageChild(imapForm);
      args.BottomAttachment(XmATTACH_FORM);
   }
   XtSetValues(selectTextForm, ARGS);

   HalDialogC::Show(parent);

//
// Update the directory listing if it has changed since the last time we
// listed it.
//
   if ( listVisible && dirName.size() > 0 ) {
      struct stat	statbuf;
      if ( stat(dirName, &statbuf) == 0 && statbuf.st_mtime > lastTimeListed )
	 SetDirectory(dirName);
   }

} // End Show

/*-----------------------------------------------------------------------
 *  Show the contents of the given directory
 */

void
FileChooserWinC::SetDefaultDir(const char *dir)
{
   defaultDir = dir;
}

/*-----------------------------------------------------------------------
 *  Show the contents of the given directory
 */

void
FileChooserWinC::SetDirectory(StringC dir)
{
   if ( debuglev > 0 ) cout <<"Setting directory to \"" <<dir <<"\"" <<endl;

   if ( UsingImap() ) {
      dirName = dir;
   }

   else {

      ShellExpand(dir);
      if ( debuglev > 0 )
	 cout <<"Shell expanded name is \"" <<dir <<"\"" <<endl;

      char	*sp = strchr((char *)dir, ' ');
      if ( sp ) {
	 int	pos = sp - (char*)dir;
	 dir.Clear(pos);
      }
      dir = FullPathname(dir);
      if ( debuglev > 0 ) cout <<"Full pathname is \"" <<dir <<"\"" <<endl;

      if ( !IsDir(dir) ) return;

      dirName = dir;
      if ( dirName.size() == 0 )	     dirName = ".";
      else if ( dirName.size() > 1 && dirName.EndsWith("/") )
	 dirName.Clear(dirName.size()-1);
   }

   if ( debuglev > 0 ) cout <<"Final name is \"" <<dirName <<"\"" <<endl;
   XmTextFieldSetString(dirNameTF, dirName);
   XmTextFieldShowPosition(dirNameTF, XmTextFieldGetLastPosition(dirNameTF));

   if ( !listVisible ) return;

   BusyCursor(True);

//
// Get the directory entries
//
   ListDirectory(dirName, fileNames, subdirNames);

//
// Allow caller to preview lists and adjust if desired
//
   Message("Sorting names...");
   CallCallbacks(verifyCalls, &fileNames);
   CallCallbacks(verifyCalls, &subdirNames);

//
// Display names
//
   subdirNames.sort();
   SetList(dirList, subdirNames);

//
// Apply the file filter to the file list
//
   Message("Applying filter...");
   FilterFiles();

//
// Re-read the names in the selected text field.  If there are any names
//    still there, the full pathnames need to be updated.
//
   if ( singleSelect ) SelectTFChanged(NULL, this, NULL);
   else		       SelectTextChanged(NULL, this, NULL);

   ClearMessage();
   BusyCursor(False);

} // End SetDirectory

/*-----------------------------------------------------------------------
 *  Method to get list of files in specified directory
 */

void
FileChooserWinC::ListDirectory(const char *dir, StringListC& files,
						StringListC& dirs)
{
   if ( UsingImap() ) {
      ListDirectoryImap(dir, files, dirs);
      return;
   }

   files.removeAll();
   dirs.removeAll();

   DIR	*dirp = opendir(dir);
   if ( !dirp ) {
      StringC	msg("Cannot read directory: ");
      msg += dir;
      PopupMessage(msg);
      return;
   }

   StringC	path(dir);
   if ( !path.EndsWith("/") ) path += "/";

   int		prefixlen = path.size();
   StringC	name;
   char		*baseMsg = "Reading files: ";
   StringC	msg;
   int		count = 0;

   struct dirent	*dp;
   while ( (dp=readdir(dirp)) != NULL) {

      if ( count % 10 == 0 ) {
	 msg = baseMsg, msg += count;
	 Message(msg);
      }

      name = dp->d_name;
      if ( name == "." ) continue;

      if ( name == ".." ) {
	 dirs.add(name);
      }

      else {

	 path += name;

	 if ( IsDir(path) ) {

	    dirs.add(name);

	    if ( showDirsInFileList ) {
	       name += "/";
	       files.add(name);
	    }
	 }
	 else if ( showFilesInFileList )
	    files.add(name);

	 path.Clear(prefixlen);
      }

      count++;
   }

   ClearMessage();
   closedir(dirp);

   struct stat	statbuf;
   if ( stat(dir, &statbuf) == 0 )
      lastTimeListed = statbuf.st_mtime;
   else
      lastTimeListed = time(0);

} // End ListDirectory

/*-----------------------------------------------------------------------
 *  Method to open a connection to the IMAP server
 */

Boolean
FileChooserWinC::GetImapServerName(StringC& name)
{
//
// Get the name of the IMAP server
//
   char	*cs = XmTextFieldGetString(imapTF);
   name = cs;
   name.Trim();

   if ( name.size() == 0 ) {
      set_invalid(imapTF, True, True);
      StringC	errmsg("Please enter the name of an IMAP server.");
      PopupMessage(errmsg);
      return False;
   }

   return True;

} // End GetImapServerName

/*-----------------------------------------------------------------------
 *  Method to get list of files in specified directory on current IMAP server
 */

void
FileChooserWinC::ListDirectoryImap(CharC dir, StringListC& files,
					      StringListC& dirs)
{
   files.removeAll();
   dirs.removeAll();

   StringC	serverName;
   if ( !GetImapServerName(serverName) ) return;

   ImapServerC	*server = FindImapServer(serverName);

//
// Get file listing
//
   StringC	pat = dir;
   if ( pat.size() > 0 && !pat.EndsWith('/') ) pat += '/';
   pat += '*';

   dir = pat;
   dir.CutEnd(1);

   StringListC	output;
   if ( !server->ListMailboxes(pat, files, output) )
      return;

//
// Remove directory name from beginning of files
//
   dir = pat;
   dir.CutEnd(1);

   u_int	count = files.size();
   int i=0; for (i=0; i<count; i++) {

      StringC	*name = files[i];
      if ( name->StartsWith(dir) ) name->CutBeg(dir.Length());

      if ( *name == "." ) {
	 files.remove(i);
	 i--;
	 count--;
	 continue;
      }

      if ( *name == ".." ) {
	 dirs.add(*name);
	 files.remove(i);
	 i--;
	 count--;
	 continue;
      }

   } // End for each entry

   return;

} // End ListDirectoryImap

/*-----------------------------------------------------------------------
 *  Method to return the last component of a pathname
 */

StringC
FileChooserWinC::BaseName(StringC& path)
{
   static RegexC*	basePat = NULL;
   if ( !basePat ) basePat = new RegexC(".*/\\([^/]+\\)/?$");

   if ( basePat->search(path) >= 0 ) {
	return path((*basePat)[1]);
   } else {
      return path;
   }

} // End BaseName

/*-----------------------------------------------------------------------
 *  Method to apply filter to file names
 */

void
FileChooserWinC::FilterFiles()
{
   filtNames.removeAll();

   filterPat = ShellToRegex(filterStr);

   unsigned		count = fileNames.size();
   int i=0; for (i=0; i<count; i++) {
      StringC*	name = fileNames[i];
      if ( filterPat.match(*name) ) filtNames.add(*name);
   }

   filtNames.sort();
   SetList(fileList, filtNames);

} // End FilterFiles

/*-----------------------------------------------------------------------
 *  Method to set an XmList using a StringList
 */

void
FileChooserWinC::SetList(Widget wlist, StringListC& slist)
{
   int		count = slist.size();
   XmString	*strList = new XmString[count];
   int i=0; for (i=0; i<count; i++) {
      char	*cs = *slist[i];
      strList[i] = XmStringCreateLtoR(cs, XmFONTLIST_DEFAULT_TAG);
   }

   Boolean	resize;
   XtVaGetValues(*this, XtNallowShellResize, &resize, NULL);
   XtVaSetValues(*this, XtNallowShellResize, False, NULL);
   XtVaSetValues(wlist, XmNitems, strList, XmNitemCount, count, NULL);
   XtVaSetValues(*this, XtNallowShellResize, resize, NULL);

   for (i=0; i<count; i++) XmStringFree(strList[i]);
   delete strList;
}

/*-----------------------------------------------------------------------
 *  Method to display names in select list
 */

void
FileChooserWinC::UpdateSelectText()
{
   localTextChange = True;

   if ( singleSelect ) {

      if ( selectNames.size() > 0 ) {
	 XmTextFieldSetString(selectTF, *selectNames[0]);
	 XtSetSensitive(okPB, True);
      } else {
	 XmTextFieldSetString(selectTF, "");
	 XtSetSensitive(okPB, False);
      }

   } else {

      XmTextReplace(selectText, 0, XmTextGetLastPosition(selectText), "");
      unsigned	count = selectNames.size();
      int i=0; for (i=0; i<count; i++) {
	 StringC*		name = selectNames[i];
	 XmTextPosition	end = XmTextGetLastPosition(selectText);

	 if ( i != 0 ) {
	    XmTextInsert(selectText, end, "\n");
	    end++;
	 }

	 XmTextInsert(selectText, end, *name);

      } // End for each name

      XtSetSensitive(okPB, count>0);
   }

   localTextChange = False;

} // End UpdateSelectText

/*---------------------------------------------------------------
 *  Callback to handle user typing in select text list
 */

void
FileChooserWinC::SelectTextChanged(Widget, FileChooserWinC *This, XtPointer)
{
   if ( This->localTextChange ) return;

//
// Loop through the text and extract the names
//
   This->selectNames.removeAll(); 
   char		*cs = XmTextGetString(This->selectText);
   CharC	text(cs);
   u_int	offset = 0;
   static StringC	*tmp = NULL;
   if ( !tmp ) tmp = new StringC;

   if ( text.Length() > 0 ) {

      CharC	word = text.NextWord(offset);
      while ( word.Length() > 0 ) {
	 *tmp = word;
	 This->selectNames.add(*tmp);
	 offset = word.Addr() - text.Addr() + word.Length();
	 word = text.NextWord(offset);
      }

      XtSetSensitive(This->okPB, This->selectNames.size()>0);

   } // End if any text present

   else {
      XtSetSensitive(This->okPB, False);
   }

   XtFree(cs);

} // End SelectTextChanged

/*---------------------------------------------------------------
 *  Callback to handle user typing in select text field
 */

void
FileChooserWinC::SelectTFChanged(Widget, FileChooserWinC *This, XtPointer)
{
   if ( This->localTextChange ) return;

//
// Get the name
//
   This->selectNames.removeAll(); 
   char		*cs = XmTextFieldGetString(This->selectTF);
   StringC	*str = NULL;
   if ( !str ) str = new StringC;
   *str = cs;
   XtFree(cs);
   str->Trim();

   if ( str->size() > 0 ) {
      This->selectNames.add(*str);
      XtSetSensitive(This->okPB, True);
   } else {
      XtSetSensitive(This->okPB, False);
   }

} // End SelectTFChanged

/*-----------------------------------------------------------------------
 *  Handle double-click on directory name
 */

void
FileChooserWinC::DoChooseDir(Widget, FileChooserWinC *This,
			     XmListCallbackStruct *cb)
{
   WXmString	wstr(cb->item);
   char		*cs = (char *)wstr;
   StringC	dir = This->dirName + "/" + cs;
   XtFree(cs);

   This->SetDirectory(dir);
}

/*-----------------------------------------------------------------------
 *  Handle double-click on file name
 */

void
FileChooserWinC::DoChooseFile(Widget, FileChooserWinC *This,
			      XmListCallbackStruct *cb)
{
   This->selectNames.removeAll();
   This->selectNames.add(*This->filtNames[cb->item_position-1]);
   This->UpdateSelectText();

   if ( This->acceptFileDoubleClick )
      DoOk(NULL, This, NULL);
}

/*-----------------------------------------------------------------------
 *  Handle selection-change in file list
 */

void
FileChooserWinC::DoFileSelection(Widget w, FileChooserWinC *This,
				 XmListCallbackStruct *cb)
{
   This->selectNames.removeAll();
   if ( cb->reason == XmCR_SINGLE_SELECT || cb->reason == XmCR_BROWSE_SELECT ) {
      if ( XmListPosSelected(w, cb->item_position) )
	 This->selectNames.add(*This->filtNames[cb->item_position-1]);
   } else {
      int i=0; for (i=0; i<cb->selected_item_count; i++) {
	 int	pos = cb->selected_item_positions[i];
	 This->selectNames.add(*This->filtNames[pos-1]);
      }
   }
   This->UpdateSelectText();

} // End DoFileSelection

/*-----------------------------------------------------------------------
 *  Handle press of filter button or CR in filter text field
 */

void
FileChooserWinC::DoFilter(Widget, FileChooserWinC *This, XtPointer)
{
//
// Get the pattern filter string from the pattern textfield.
//
   char*	cs = XmTextFieldGetString(This->filterTF);
   This->filterStr = cs;
   XtFree(cs);

//
// See if the directory name has changed
//
   cs = XmTextFieldGetString(This->dirNameTF);
   if ( This->dirName != cs ) DoSetDir(NULL, This, NULL);
   else			      This->FilterFiles();
   XtFree(cs);

} // End DoFilter

/*-----------------------------------------------------------------------
 *  Handle single-click on directory name
 */

void
FileChooserWinC::DoSelectDir(Widget, FileChooserWinC *This,
			     XmListCallbackStruct *cb)
{
   XmListDeselectPos(This->dirList, cb->item_position);
}

/*-----------------------------------------------------------------------
 *  Handle CR in directory name field
 */

void
FileChooserWinC::DoSetDir(Widget, FileChooserWinC *This, XtPointer)
{
   char*	cs = XmTextFieldGetString(This->dirNameTF);
   StringC	dir(cs);
   XtFree(cs);

   if ( !This->UsingImap() ) {

      ShellExpand(dir);
      char	*sp = strchr((char *)dir, ' ');
      if ( sp ) {
	 int	pos = sp - (char*)dir;
	 dir.Clear(pos);
      }
      dir = FullPathname(dir);

      if ( !IsDir(dir) ) {
	 set_invalid(This->dirNameTF, True, True);
	 StringC	msg(dir);
	 msg += " is not a directory.";
	 This->PopupMessage(msg);
	 return;
      }
   }

   This->SetDirectory(dir);

} // End DoSetDir

/*-----------------------------------------------------------------------
 *  Handle press of ok button
 */

void
FileChooserWinC::DoOk(Widget, FileChooserWinC *This, XtPointer)
{
   halApp->BusyCursor(True);

   This->ExpandSelectedNames();

   This->hideOk = True;
   CallCallbacks(This->okCalls, &This->selectNames);
   if ( This->hideOk ) This->Hide();

   halApp->BusyCursor(False);

} // End DoOk

/*-----------------------------------------------------------------------
 *  Handle press of clear button
 */

void
FileChooserWinC::DoClear(Widget, FileChooserWinC *This, XtPointer)
{
   if ( This->singleSelect )
      XmTextFieldSetString(This->selectTF, "");
   else
      XmTextSetString(This->selectText, "");
}

/*-----------------------------------------------------------------------
 *  Method to expand names in selected text field
 */

void
FileChooserWinC::ExpandSelectedNames()
{
   Boolean	imapOk = showImap;

//
// Turn any relative names into full pathnames
//
   u_int	count = selectNames.size();
   int i=0; for (i=0; i<count; i++) {

      StringC	*name = selectNames[i];

//
// If the name is a full pathname or an IMAP folder leave it alone
//
      if ( *name == "."		   ||
	   *name == ".."	   ||
	   name->StartsWith("/")   ||
	   name->StartsWith("./")  ||
	   name->StartsWith("../") ||
	   name->StartsWith("$")   ||
	   name->StartsWith("~")   ||
	   IsImapName(*name) )
	 ;

      else if ( name->StartsWith("+") || name->StartsWith("=") ) {
   	 if ( (!ishApp->appPrefs->usingImap) ||
	      (!ishApp->folderPrefs->UsingLocal()) )
	     (*name)(0,1) = ishApp->appPrefs->FolderDir() + "/";
	 else
	     (*name)(0,1) = "";
      }

//
// If the list is visible, grab the directory name from there
//
      else if ( listVisible ) {
         if ( dirName.size()  == 0 ) {
            if ( !ishApp->appPrefs->usingImap )
               *name = "/" + *name;
            // else using IMAP, folder name doesn't need a leading "/"
         } else {
	    *name = dirName + "/" + *name;
         }
      }

//                                                
// If there is a default directory, use that.
// (unless using an IMAP server)
//
      else if ( defaultDir.size() > 0 && !ishApp->appPrefs->usingImap ) 
	 *name = defaultDir + "/" + *name;

   } // End for each name

//
// See if we need to add the IMAP server name
//
   if ( UsingImap() ) {

      StringC	serverName;
      if ( !GetImapServerName(serverName) ) return;

      if ( serverName != ishApp->appPrefs->imapServer ||
	   ishApp->folderPrefs->UsingLocal() ) {

//
// Add server name to beginning of each name
//
	 count = selectNames.size();
	 for (i=0; i<count; i++) {
	    StringC	*name = selectNames[i];
	    if ( !name->StartsWith('{') )
	       *name = "{" + serverName + "}" + *name;
	 }

      } // End if server name is needed

   } // End if using IMAP server

//
// Expand any shell variables and wildcards
//
   selectNames.SetSorted(False);
   ExpandList(selectNames);
   selectNames.SetSorted(True);

//
// This may not be necessary
//
#if 0
   if ( !UsingImap() ) {

//
// Finally, get the full pathnames
//
      count = selectNames.size();
      for (i=0; i<count; i++) {

	 StringC	*name = selectNames[i];

//
// If the name is a relative pathname, get the real one
//
	 if ( !name->StartsWith('/') )
	    *name = FullPathname(*name);

      } // End for each name

   } // End If not using the IMAP server
#endif

} // End ExpandSelectedNames

/*-----------------------------------------------------------------------
 *  Handle toggle of list display button
 */

void
FileChooserWinC::ToggleList(Widget, FileChooserWinC *This,
			    XmToggleButtonCallbackStruct *tb)
{
   if ( tb->set ) {

      This->BusyCursor(True);
      This->ShowList();
#if 0
      This->SetDirectory(This->dirName);
#endif
      This->BusyCursor(False);

   } else {

      This->HideList();
   }

} // End ToggleList

/*-----------------------------------------------------------------------
 *  Turn single selection on or off
 */

void
FileChooserWinC::SingleSelect(Boolean val)
{
   if ( singleSelect == val ) return;

   StringC	titleStr = get_string("FileChooserC", *this, "title");
   StringC	labelStr = get_string(selectLabel, "labelString");

   if ( val ) {

      XtVaSetValues(fileList, XmNselectionPolicy, XmSINGLE_SELECT, NULL);

      XtManageChild(selectTF);
      XtUnmanageChild(selectTextSW);

//
// Fix size of selectForm since this is single selection mode
//
      Dimension	ht;
      XtVaGetValues(selectForm, XmNheight, &ht, NULL);
      if ( ht > 0 )
	 XtVaSetValues(selectForm, XmNpaneMinimum, ht, XmNpaneMaximum, ht, 0);

//
// Set titles for single selection
//
      StringC	str = get_string("FileChooserC", *this, "singleTitle", titleStr);
      XtVaSetValues(*this, XmNtitle, (char*)str, XmNiconName, (char*)str, 0);

      str = get_string(selectLabel, "singleString", labelStr);
      WXmString	wstr = (char*)str;
      XtVaSetValues(selectLabel, XmNlabelString, (XmString)wstr, NULL);

   } // End if turning on single select mode

   else {

      XtVaSetValues(fileList, XmNselectionPolicy, XmEXTENDED_SELECT, NULL);
      XtManageChild(selectTextSW);
      XtUnmanageChild(selectTF);
      XtVaSetValues(selectForm, XmNpaneMinimum, 1, XmNpaneMaximum, 1000, 0);

//
// Set titles for multiple selection
//
      XtVaSetValues(*this, XmNtitle,    (char*)titleStr,
			   XmNiconName, (char*)titleStr, NULL);

      WXmString	wstr = (char*)labelStr;
      XtVaSetValues(selectLabel, XmNlabelString, (XmString)wstr, NULL);

   } // End if turning off single select mode

   singleSelect = val;

} // End SingleSelect

/*---------------------------------------------------------------
 *  Action callback to handle keyboard activation
 */

void
FileChooserWinC::HandleActivate(Widget w, XKeyEvent*, String*, Cardinal*)
{
   FileChooserWinC	*This;
   XtVaGetValues(w, XmNuserData, &This, NULL);

   DoOk(NULL, This, NULL);
}

/*---------------------------------------------------------------
 *  Callback to automatically select the IMAP toggle button when the
 *     server field is changed
 */

void
FileChooserWinC::AutoSelectImap(Widget, FileChooserWinC *This, XtPointer)
{
   XmToggleButtonSetState(This->imapTB, True, True);
}

/*---------------------------------------------------------------
 *  Method to determine if we're using the imap server
 */

Boolean
FileChooserWinC::UsingImap() const
{
   return (showImap && XmToggleButtonGetState(imapTB));
}

/*-----------------------------------------------------------------------
 *  Show/Hide IMAP server name entry field
 */

void
FileChooserWinC::ShowImap()
{
   if ( showImap ) return;

   showImap = True;

   XtManageChild(imapForm);

   WArgList	args;
   args.BottomAttachment(XmATTACH_WIDGET, imapForm);
   XtSetValues(selectTextForm, ARGS);
}

void
FileChooserWinC::HideImap()
{
   if ( !showImap ) return;

   showImap = False;

   XtUnmanageChild(imapForm);

   WArgList	args;
   args.BottomAttachment(XmATTACH_FORM);
   XtSetValues(selectTextForm, ARGS);
}
