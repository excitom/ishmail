/*
 *  $Id: MainWinC.h,v 1.2 2000/09/19 16:19:17 evgeny Exp $
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
#ifndef _MainWinC_h_
#define _MainWinC_h_

#include <hgl/HalMainWinC.h>

class MainWinP;
class FolderC;
class VBoxC;
class VItemC;
class VItemListC;
class MsgC;
class SortMgrC;
class ButtonMgrC;
class FieldViewC;

class MainWinC : public HalMainWinC {

   friend class MainWinP;

   MainWinP	*priv;		// Private stuff

public:

   FolderC	*curFolder;
   VItemListC	*curMsgList;     // List of messages being read
   Boolean	readingSelected;
   SortMgrC	*sortMgr;
   ButtonMgrC	*buttMgr;

//
// Popup menu data
//
   FolderC	*popupFolder;   // Popup menu for this one
   MsgC		*popupMsg;      // Popup menu for this one
   Boolean	popupOnSelected;// Apply popup function to selected items
   				// rather than popup item

//
// Methods
//
   MainWinC(Widget);
   ~MainWinC();

   void		ActivateSystemFolder();
   void		AddInitialFolder(const char*);
   Boolean	DeleteItems(VItemListC&);
   Boolean	DeleteMsg(MsgC*);
   void		EnableButtons();
   void		FolderDirChanged();
   VBoxC&	FolderVBox();
   FieldViewC&	FieldView();
   void		GetNewMail(FolderC*);
   void		HideDeletedChanged();
   VBoxC&	MsgVBox();
   void		Refresh();
   void		RegisterMsgIcon(VItemC*);
   void		SetCurrentFolder(FolderC*);
   void		ShowFolder(FolderC*);
   void		UpdateFields();
   void		UpdateTitle();
   Boolean	UndeleteItems(VItemListC&);
   Boolean	UndeleteMsg(MsgC*);

   MsgC		*NextReadable(MsgC*);
   MsgC		*NextUnread  (MsgC*);
   MsgC		*NextSender  (MsgC*);
   MsgC		*NextSubject (MsgC*);
   MsgC		*PrevReadable(MsgC*);
   MsgC		*PrevUnread  (MsgC*);
   MsgC		*PrevSender  (MsgC*);
   MsgC		*PrevSubject (MsgC*);
};

#endif // _MainWinC_h_
