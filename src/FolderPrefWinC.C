/*
 * $Id: FolderPrefWinC.C,v 1.2 2000/05/07 12:26:11 fnevgeny Exp $
 *
 * Copyright (c) 1994 HAL Computer Systems International, Ltd.
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

#include "FolderPrefWinC.h"
#include "FolderPrefC.h"
#include "IshAppC.h"
#include "ConfPrefC.h"
#include "Misc.h"
#include "AppPrefC.h"

#include <hgl/WArgList.h>
#include <hgl/rsrc.h>
#include <hgl/MimeRichTextC.h>
#include <hgl/CharC.h>

#include <Xm/RowColumn.h>
#include <Xm/Label.h>
#include <Xm/Frame.h>
#include <Xm/ToggleB.h>
#include <Xm/Form.h>

extern int	debug1, debug2;

/*---------------------------------------------------------------
 *  Constructor
 */

FolderPrefWinC::FolderPrefWinC(Widget par) : OptWinC(par, "folderPrefWin")
{
   WArgList	args;
   Widget	wlist[6];
   Cardinal	wcount;

//
// Create appForm hierarchy
//
// appForm
//    RowColumn		viewRC
//    RowColumn		typeRC
//    Form		initForm
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_NONE);
   args.Orientation(XmVERTICAL);
   args.Packing(XmPACK_TIGHT);
   Widget viewRC = XmCreateRowColumn(appForm, "viewRC", ARGS);

   args.TopAttachment(XmATTACH_WIDGET, viewRC);
   Widget typeRC = XmCreateRowColumn(appForm, "typeRC", ARGS);

   args.TopAttachment(XmATTACH_WIDGET, typeRC);
   args.BottomAttachment(XmATTACH_FORM);
   Widget initForm = XmCreateForm(appForm, "initForm", ARGS);

//
// Create viewRC hierarchy
//
// viewRC
//    Form		viewForm
//    ToggleButton	sortTB
//    ToggleButton	statusTB
//
   Widget viewForm = XmCreateForm        (viewRC, "viewForm", 0,0);
   sortTB          = XmCreateToggleButton(viewRC, "sortTB",   0,0);
   statusTB        = XmCreateToggleButton(viewRC, "statusTB", 0,0);

//
// Create viewForm hierarchy
//
// viewForm
//    Label		viewLabel
//    Frame		viewFrame
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   Widget viewLabel = XmCreateLabel(viewForm, "viewLabel", ARGS);

   args.LeftAttachment(XmATTACH_WIDGET, viewLabel);
   args.RightAttachment(XmATTACH_FORM);
   Widget viewFrame = XmCreateFrame(viewForm, "viewFrame", ARGS);

//
// Create viewFrame hierarchy
//
//   viewFrame
//	RadioBox	viewRadio
//	   ToggleButton	viewLargeTB
//	   ToggleButton	viewSmallTB
//	   ToggleButton	viewNameTB
//
   args.Reset();
   args.Orientation(XmHORIZONTAL);
   args.Packing(XmPACK_TIGHT);
   Widget viewRadio = XmCreateRadioBox(viewFrame, "viewRadio", ARGS);

   viewLargeTB = XmCreateToggleButton(viewRadio, "viewLargeTB", 0,0);
   viewSmallTB = XmCreateToggleButton(viewRadio, "viewSmallTB", 0,0);
   viewNameTB  = XmCreateToggleButton(viewRadio, "viewNameTB",  0,0);

   wlist[0] = viewLargeTB;
   wlist[1] = viewSmallTB;
   wlist[2] = viewNameTB;
   XtManageChildren(wlist, 3);	// viewRadio children
   XtManageChild(viewRadio);	// viewFrame children

   wlist[0] = viewLabel;
   wlist[1] = viewFrame;
   XtManageChildren(wlist, 2);	// viewForm children

   wlist[0] = viewForm;
   wlist[1] = sortTB;
   wlist[2] = statusTB;
   XtManageChildren(wlist, 3);	// viewRC children

//
// Create typeRC hierarchy
//
// typeRC
//    Form		useTypeForm
//    Form		defTypeForm
//    ToggleButton	confirmTypeTB
//
   Widget useTypeForm = XmCreateForm	    (typeRC, "useTypeForm",   0,0);
   Widget defTypeForm = XmCreateForm	    (typeRC, "typeForm",      0,0);
   confirmTypeTB      = XmCreateToggleButton(typeRC, "confirmTypeTB", 0,0);

//
// Create useTypeForm hierarchy
//
// useTypeForm
//    Label		useTypeLabel
//    Frame		useTypeFrame
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   Widget useTypeLabel = XmCreateLabel(useTypeForm, "useTypeLabel", ARGS);

   args.LeftAttachment(XmATTACH_WIDGET, useTypeLabel);
   args.RightAttachment(XmATTACH_FORM);
   Widget useTypeFrame = XmCreateFrame(useTypeForm, "useTypeFrame", ARGS);

//
// Create useTypeFrame hierarchy
//
//   useTypeFrame
//	RowColumn	useTypeRC
//	   ToggleButton	useUnixTB
//	   ToggleButton	useMhTB
//	   ToggleButton	useMmdfTB
//	   ToggleButton	useNoneTB
//
   args.Reset();
   args.Orientation(XmHORIZONTAL);
   args.Packing(XmPACK_TIGHT);
   Widget useTypeRC = XmCreateRowColumn(useTypeFrame, "useTypeRC", ARGS);

   useUnixTB = XmCreateToggleButton(useTypeRC, "useUnixTB", 0,0);
   useMhTB   = XmCreateToggleButton(useTypeRC, "useMhTB",   0,0);
   useMmdfTB = XmCreateToggleButton(useTypeRC, "useMmdfTB", 0,0);
   useNoneTB = XmCreateToggleButton(useTypeRC, "useNoneTB", 0,0);

   wcount = 0;
   wlist[wcount++] = useUnixTB;
   wlist[wcount++] = useMhTB;
   wlist[wcount++] = useMmdfTB;
   if ( ishApp->appPrefs->usingImap )
      wlist[wcount++] = useNoneTB;
   XtManageChildren(wlist, wcount);	// useTypeRC children
   XtManageChild(useTypeRC);	// useTypeFrame children

   XtAddCallback(useUnixTB, XmNvalueChangedCallback,
   		 (XtCallbackProc)DoToggleLocalType, this);
   XtAddCallback(useMhTB,   XmNvalueChangedCallback,
   		 (XtCallbackProc)DoToggleLocalType, this);
   XtAddCallback(useMmdfTB, XmNvalueChangedCallback,
   		 (XtCallbackProc)DoToggleLocalType, this);
   XtAddCallback(useNoneTB, XmNvalueChangedCallback,
   		 (XtCallbackProc)DoToggleLocalType, this);

   wlist[0] = useTypeLabel;
   wlist[1] = useTypeFrame;
   XtManageChildren(wlist, 2);	// useTypeForm children

//
// Create defTypeForm hierarchy
//
// defTypeForm
//    Label		defTypeLabel
//    Frame		defTypeFrame
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   Widget defTypeLabel = XmCreateLabel(defTypeForm, "typeLabel", ARGS);

   args.LeftAttachment(XmATTACH_WIDGET, defTypeLabel);
   args.RightAttachment(XmATTACH_FORM);
   Widget defTypeFrame = XmCreateFrame(defTypeForm, "typeFrame", ARGS);

//
// Create defTypeFrame hierarchy
//
//   defTypeFrame
//	RadioBox	defTypeRadio
//	   ToggleButton	defImapTB
//	   ToggleButton	defUnixTB
//	   ToggleButton	defMhTB
//	   ToggleButton	defMmdfTB
//
   args.Reset();
   args.Orientation(XmHORIZONTAL);
   args.Packing(XmPACK_TIGHT);
   Widget defTypeRadio = XmCreateRadioBox(defTypeFrame, "typeRadio", ARGS);

   defImapTB = XmCreateToggleButton(defTypeRadio, "typeImapTB", 0,0);
   defUnixTB = XmCreateToggleButton(defTypeRadio, "typeUnixTB", 0,0);
   defMhTB   = XmCreateToggleButton(defTypeRadio, "typeMhTB",   0,0);
   defMmdfTB = XmCreateToggleButton(defTypeRadio, "typeMmdfTB", 0,0);

   wcount = 0;
   if ( ishApp->appPrefs->usingImap )
      wlist[wcount++] = defImapTB;
   wlist[wcount++] = defUnixTB;
   wlist[wcount++] = defMhTB;
   wlist[wcount++] = defMmdfTB;
   XtManageChildren(wlist, wcount);	// defTypeRadio children
   XtManageChild(defTypeRadio);	// defTypeFrame children

   wlist[0] = defTypeLabel;
   wlist[1] = defTypeFrame;
   XtManageChildren(wlist, 2);	// defTypeForm children

   wlist[0] = useTypeForm;
   wlist[1] = defTypeForm;
   wlist[2] = confirmTypeTB;
   XtManageChildren(wlist, 3);	// typeRC children

//
// Create initForm hierarchy
//
// initForm
//    Label		initLabel
//    MimeRichTextC	initText
//    ToggleButton	rememberTB
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_NONE);
   Widget initLabel = XmCreateLabel(initForm, "initLabel", ARGS);

   args.TopAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   rememberTB = XmCreateToggleButton(initForm, "rememberTB", ARGS);

   args.EditMode(XmMULTI_LINE_EDIT);
   args.TopAttachment(XmATTACH_WIDGET, initLabel);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_WIDGET, rememberTB);
   initText = new MimeRichTextC(initForm, "initText",  ARGS);
   initText->SetTextType(TT_PLAIN);
   XtManageChild(initText->MainWidget());

   wlist[0] = initLabel;
   wlist[1] = rememberTB;
   XtManageChildren(wlist, 2);	// initForm children

   wlist[0] = viewRC;
   wlist[1] = typeRC;
   wlist[2] = initForm;
   XtManageChildren(wlist, 3);  // appForm children

   HandleHelp();

} // End constructor

/*---------------------------------------------------------------
 *  Destructor
 */

FolderPrefWinC::~FolderPrefWinC()
{
   delete initText;
}

/*---------------------------------------------------------------
 *  Method to display window settings
 */

void
FolderPrefWinC::Show()
{
   FolderPrefC	*prefs = ishApp->folderPrefs;

   if ( ishApp->appPrefs->usingImap ) {
      XtManageChild(useNoneTB);
      XtManageChild(defImapTB);
   }
   else {
      XtUnmanageChild(useNoneTB);
      XtUnmanageChild(defImapTB);
   }

   XmToggleButtonSetState(sortTB,	 prefs->SortFolders(),		True);
   XmToggleButtonSetState(statusTB,	 prefs->showStatus,		True);
   XmToggleButtonSetState(confirmTypeTB, ishApp->confPrefs->confirmFolderType,	
   					 True);
   XmToggleButtonSetState(useUnixTB,	 prefs->usingUnix,		True);
   XmToggleButtonSetState(useMhTB,	 prefs->usingMh,		True);
   XmToggleButtonSetState(useMmdfTB,	 prefs->usingMmdf,		True);
   XmToggleButtonSetState(useNoneTB,	!prefs->UsingLocal(),		True);
   XmToggleButtonSetState(rememberTB,	 prefs->rememberFolders,	True);

   switch (prefs->ViewType()) {
      case(FOLDER_VIEW_LARGE):
         XmToggleButtonSetState(viewLargeTB, True, True);
	 break;
      case(FOLDER_VIEW_SMALL):
         XmToggleButtonSetState(viewSmallTB, True, True);
	 break;
      case(FOLDER_VIEW_NAME):
         XmToggleButtonSetState(viewNameTB, True, True);
	 break;
   }

   switch (prefs->defFolderType) {
      case(UNIX_FOLDER):
         XmToggleButtonSetState(defUnixTB, True, True);
	 break;
      case(MH_FOLDER):
         XmToggleButtonSetState(defMhTB, True, True);
	 break;
      case(MMDF_FOLDER):
         XmToggleButtonSetState(defMmdfTB, True, True);
	 break;
      case(IMAP_FOLDER):
         XmToggleButtonSetState(defImapTB, True, True);
	 break;
      case(UNKNOWN_FOLDER):
	 break;
   }

//
// Display the list of initial folders.  Replace spaces with newlines
//
   StringListC	list;
   ExtractList(prefs->OrigInitialFolders(), list);
   u_int	count = list.size();
   initText->Defer(True);
   initText->Clear();
   for (int i=0; i<count; i++) {
      if ( i>0 ) initText->AddString("\n");
      initText->AddString(*list[i]);
   }
   initText->Defer(False);

   OptWinC::Show();

} // End Show

/*---------------------------------------------------------------
 *  Method to apply settings
 */

Boolean
FolderPrefWinC::Apply()
{
   FolderPrefC	*prefs = ishApp->folderPrefs;

   BusyCursor(True);

   if ( XmToggleButtonGetState(viewLargeTB) )
      prefs->SetViewType(FOLDER_VIEW_LARGE);
   else if ( XmToggleButtonGetState(viewSmallTB) )
      prefs->SetViewType(FOLDER_VIEW_SMALL);
   else
      prefs->SetViewType(FOLDER_VIEW_NAME);

   prefs->SetSorted(XmToggleButtonGetState(sortTB));
   prefs->showStatus = XmToggleButtonGetState(statusTB);

   if ( XmToggleButtonGetState(defMhTB) )
      prefs->defFolderType = MH_FOLDER;
   else if ( XmToggleButtonGetState(defMmdfTB) )
      prefs->defFolderType = MMDF_FOLDER;
   else if ( XmToggleButtonGetState(defImapTB) )
      prefs->defFolderType = IMAP_FOLDER;
   else
      prefs->defFolderType = UNIX_FOLDER;

   ishApp->confPrefs->confirmFolderType = XmToggleButtonGetState(confirmTypeTB);
   prefs->usingMh   = XmToggleButtonGetState(useMhTB);
   prefs->usingMmdf = XmToggleButtonGetState(useMmdfTB);
   prefs->usingUnix = XmToggleButtonGetState(useUnixTB);
   prefs->rememberFolders = XmToggleButtonGetState(rememberTB);

//
// Get string and replace newlines with spaces
//
   StringC	tmp;
   initText->GetString(tmp, TT_PLAIN);
   char	*cs = tmp;
   while ( *cs ) {
      if ( *cs == '\n' ) *cs = ' ';
      cs++;
   }
   prefs->SetInitialFolders(tmp);

//
// Write to resource database
//
   prefs->WriteDatabase();

//
// Write to file if necessary
//
   if ( applyAll ) prefs->WriteFile();

//
// If we are showing the status, update the folder icons
//
   if ( prefs->showStatus ) {
      FolderListC&	list = prefs->OpenFolders();
      u_int		count = list.size();
      for (int i=0; i<count; i++) {
	 FolderC	*folder = list[i];
	 if ( !folder->scanned ) {
//	    folder->Scan();
	    folder->UpdateIcon();
	 }
      }
   }

   BusyCursor(False);

   return True;

} // End Apply

/*---------------------------------------------------------------
 *  Callback to handle selection of local folder type
 */

void
FolderPrefWinC::DoToggleLocalType(Widget tb, FolderPrefWinC *This,
				  XmToggleButtonCallbackStruct *data)
{
   if ( tb == This->useNoneTB ) {
      if ( data->set ) {
	 XmToggleButtonSetState(This->useUnixTB, False, False);
	 XmToggleButtonSetState(This->useMhTB,   False, False);
	 XmToggleButtonSetState(This->useMmdfTB, False, False);
      }
   }

   else {
      if ( data->set )
	 XmToggleButtonSetState(This->useNoneTB, False, False);
      else if ( !XmToggleButtonGetState(This->useUnixTB) &&
		!XmToggleButtonGetState(This->useMhTB)   &&
		!XmToggleButtonGetState(This->useMmdfTB) )
	 XmToggleButtonSetState(This->useNoneTB, True, False);
   }

} // End DoToggleLocalType
