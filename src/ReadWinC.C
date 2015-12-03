/*
 * $Id: ReadWinC.C,v 1.4 2000/06/28 11:41:48 evgeny Exp $
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

#include "ReadWinC.h"
#include "ReadWinP.h"
#include "IshAppC.h"
#include "ButtonMgrC.h"
#include "ReadButtPrefC.h"
#include "MsgC.h"
#include "AppPrefC.h"
#include "QuickMenu.h"
#include "LocalTextWinC.h"
#include "ReadIconC.h"

#include <hgl/WXmString.h>
#include <hgl/MimeRichTextC.h>

#include <Xm/ToggleB.h>

// Max length of the title string
#define MAX_TITLE_STR_LENGTH    80

/*---------------------------------------------------------------
 *  Main window constructor
 */

ReadWinC::ReadWinC(const char *name, Widget parent, ReadWinC *parWin)
 : HalTopLevelC(name, parent)
{
   msg          = NULL;
   buttMgr      = NULL;
   parentWin    = parWin;
   fullFunction = (parWin == NULL);

   priv = new ReadWinP(this);

   priv->BuildMenus();
   priv->BuildWidgets();
   priv->ReadResources();

//
// Create the custom button manager if necessary
//
   buttMgr = new ButtonMgrC(this, menuBar, buttonRC,
			    ishApp->readButtPrefs->buttonStr);

   priv->BuildRecentFolderMenu();
   ishApp->appPrefs->AddRecentChangeCallback(
			      (CallbackFn*)ReadWinP::RecentListChanged, priv);

} // End constructor

/*---------------------------------------------------------------
 *  Main window destructor
 */

ReadWinC::~ReadWinC()
{
   delete buttMgr;
   delete priv;
}

/*-----------------------------------------------------------------------
 *  Display window
 */

void
ReadWinC::Show()
{
   HalTopLevelC::Show();

//
// Update recent folder menu if necessary
//
   if ( ishApp->appPrefs->recentFolderTime > priv->recentMenuTime )
      priv->BuildRecentFolderMenu();
}

/*---------------------------------------------------------------
 *  Methods to set state of pushpin
 */

void
ReadWinC::Pin()
{
   if ( priv->pushPinTB ) XmToggleButtonSetState(priv->pushPinTB, True, True);
}

void
ReadWinC::Unpin()
{
   if ( priv->pushPinTB ) XmToggleButtonSetState(priv->pushPinTB, False, True);
}

/*---------------------------------------------------------------
 *  Method to check state of pushpin
 */

Boolean
ReadWinC::Pinned()
{
   return (priv->pushPinTB && XmToggleButtonGetState(priv->pushPinTB));
}

/*---------------------------------------------------------------
 *  Method to set displayed message number
 */

void
ReadWinC::SetMessageNumber(int num)
{
   ClearMessage();

//
// Place message number and subject in message label
//
   WXmString	ws;
   if ( msg ) {

      StringC	titleStr = priv->msgTitleStr;
      StringC	subStr; msg->GetSubjectText(subStr);
      StringC	numStr;

      if ( fullFunction ) numStr += num;

      titleStr.Replace("$NUM", numStr);
      titleStr.Replace("$SUB", subStr);
      
      if (titleStr.length() > MAX_TITLE_STR_LENGTH) {
         titleStr.reSize(MAX_TITLE_STR_LENGTH);
         titleStr += "...";
      }

      ws = (char *)titleStr;
   }

   else
      ws = (char *)priv->noMsgTitleStr;

//
// Display title
//
   XtVaSetValues(appForm, XmNresizePolicy, XmRESIZE_NONE, NULL);
   XtVaSetValues(priv->msgTitle, XmNlabelString, (XmString)ws, NULL);
   XtVaSetValues(appForm, XmNresizePolicy, XmRESIZE_ANY, NULL);

} // End SetMessageNumber

/*---------------------------------------------------------------
 *  Method to set displayed message
 */

void
ReadWinC::SetMessage(MsgC *m)
{
   BusyCursor(True);
   ClearMessage();

   priv->msgHeadText->Defer(True);
   priv->msgBodyText->Defer(True);

   if ( m != msg ) priv->Reset();

   msg = m;
   if ( msg ) {
      SetMessageNumber(msg->Number());
      priv->DisplayHeaders();
      priv->DisplayBody();
   }

   else {
      SetMessageNumber(0);
      priv->msgHeadText->Clear();
      priv->msgBodyText->Clear();
   }

   priv->msgHeadText->Defer(False);
   priv->msgBodyText->Defer(False);

//
// Enable buttons
//
   priv->EnableButtons();

   if ( msg ) priv->UpdateSaveFolder();

   BusyCursor(False);

} // End SetMessage(MsgC*)

/*---------------------------------------------------------------
 *  Method to redisplay the headers
 */

void
ReadWinC::RedisplayHeaders()
{
   if ( msg ) priv->DisplayHeaders();
}

/*---------------------------------------------------------------
 *  Method to check state of wrapping
 */

Boolean
ReadWinC::Wrapping()
{
   return XmToggleButtonGetState(priv->optWrapTB);
}

/*---------------------------------------------------------------
 *  Method to return column count
 */

int
ReadWinC::ColumnCount()
{
   return (priv->msgBodyText->ColumnCount());
}

/*---------------------------------------------------------------
 *  Method to return body row count
 */

int
ReadWinC::BodyRowCount()
{
   return (priv->msgBodyText->RowCount());
}

/*---------------------------------------------------------------
 *  Method to return header row count
 */

int
ReadWinC::HeadRowCount()
{
   return (priv->msgHeadText->RowCount());
}

/*---------------------------------------------------------------
 *  Method to return view type
 */

ReadViewTypeT
ReadWinC::ViewType()
{
   return priv->viewType;
}

/*---------------------------------------------------------------
 *  Method to set the number of columns displayed
 */

void
ReadWinC::SetSize(int headRows, int bodyRows, int cols)
{
   BusyCursor(True);

   if ( headRows != priv->msgHeadText->RowCount() )
      priv->msgHeadText->SetSize(headRows, priv->msgHeadText->ColumnCount());

   if ( bodyRows != priv->msgBodyText->RowCount() ||
	cols     != priv->msgBodyText->ColumnCount() )
      priv->msgBodyText->SetSize(bodyRows, cols, *this);

//
// Loop through the list of icons and update visible text windows and
//    message windows
//
   PtrListC&	list  = priv->msgBodyText->GraphicList();
   u_int	count = list.size();
   for (int i=0; i<count; i++) {
      ReadIconC	*icon = (ReadIconC*)*list[i];
      if ( icon->textWin ) icon->textWin->SetSize(bodyRows, cols);
      if ( icon->msgWin  ) icon->msgWin->SetSize(headRows, bodyRows, cols);
   }

   BusyCursor(False);

} // End SetSize

/*---------------------------------------------------------------
 *  Method to set whether text wraps at the margin.
 */

void
ReadWinC::SetWrap(Boolean val)
{
   BusyCursor(True);

   priv->msgBodyText->ResizeWidth(!val);

   if ( fullFunction )
      XmToggleButtonSetState(priv->optWrapTB, val, False);

//
// Loop through the list of icons and update visible text windows and
//    message windows
//
   PtrListC&	list  = priv->msgBodyText->GraphicList();
   u_int	count = list.size();
   for (int i=0; i<count; i++) {
      ReadIconC	*icon = (ReadIconC*)*list[i];
      if ( icon->textWin ) icon->textWin->SetWrap(val);
      if ( icon->msgWin  ) icon->msgWin->SetWrap(val);
   }

   BusyCursor(False);

} // End SetWrap

/*---------------------------------------------------------------
 *  Method to set the view type
 */

void
ReadWinC::SetViewType(ReadViewTypeT type)
{
   if ( priv->viewType == type ) return;

//
// Loop through the list of icons and close any displays since the icons
//    will be rebuilt.
//
   PtrListC&	list  = priv->msgBodyText->GraphicList();
   u_int	count = list.size();
   for (int i=0; i<count; i++) {
      ReadIconC	*icon = (ReadIconC*)*list[i];
      priv->HideIcon(icon);
   }

//
// Redisplay the message
//
   priv->viewType = type;
   switch (type) {
      case (READ_VIEW_FLAT):
	 XmToggleButtonSetState(priv->viewFlatTB, True, False);
	 break;
      case (READ_VIEW_OUTLINE):
	 XmToggleButtonSetState(priv->viewOutlineTB, True, False);
	 break;
      case (READ_VIEW_CONTAINER):
	 XmToggleButtonSetState(priv->viewNestedTB, True, False);
	 break;
      case (READ_VIEW_SOURCE):
	 XmToggleButtonSetState(priv->viewSourceTB, True, False);
	 break;
   }

   priv->DisplayBody();

} // End SetViewType

/*---------------------------------------------------------------
 *  Method to update display based on the fact that the folder directory
 *     has changed.
 */

void
ReadWinC::FolderDirChanged()
{
   UpdateQuickDir(priv->fileSaveQuickCB, ishApp->appPrefs->FolderDir());
}

/*---------------------------------------------------------------
 *  Method to update button sensitivities
 */

void
ReadWinC::EnableButtons()
{
   priv->EnableButtons();
}

