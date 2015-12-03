/*
 * $Id: FolderC.h,v 1.2 2000/04/27 14:39:58 fnevgeny Exp $
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

#ifndef _FolderC_h_
#define _FolderC_h_

#include <hgl/StringC.h>

#include <X11/Intrinsic.h>

#include <sys/stat.h>

enum FolderTypeT {
   UNKNOWN_FOLDER,
   MH_FOLDER,
   UNIX_FOLDER,
   MMDF_FOLDER,
   IMAP_FOLDER
};

class MsgC;
class MsgListC;
class MsgItemC;
class VItemC;
class VItemListC;
class SortMgrC;
class PartialMsgC;
class PartialMsgDictC;
class StringListC;

typedef Boolean		 StatusFn(u_int);

class FolderC {

private:

   Boolean		changed;	// Must be changed through call

protected:

   StringC		indexFile;
   struct stat		istats;
   int			indexOpenLevel;
   u_int		indexSum;	// Union of all message status values
   time_t		indexSumTime;	// Time value was calculated

   int			lastVScroll;	// Value when deactivated
   VItemListC		*lastSelList;	// Items selected when deactivated
   VItemListC		*msgItemList;
   VItemListC		*delItemList;
   SortMgrC		*sortMgr;
   static PartialMsgDictC	*partialMsgDict;

   void			CreateIcons();
   void			CreateIndex();
   Boolean		IndexSumValid();
   void			ReadIndex();
   void		 	Reset();
   void			ScanIndex();
   void			UpdateIndex();
   void			InsertInThread(MsgItemC*);

   static void		ResetScrollBar(FolderC*, XtIntervalId*); 

public:

   StringC		name;
   StringC		abbrev;
   FolderTypeT		type;
   MsgListC		*msgList;
   FILE			*indexfp;
   VItemC		*icon;

   Boolean		isInBox;
   Boolean		writable;
   Boolean		scanned;
   Boolean		opened;
   Boolean		active;
   Boolean		delFiles;

   inline int	compare(const FolderC& f) const { return name.compare(f.name); }

   inline int	operator==(const FolderC& f) const { return name == f.name; }
   inline int	operator!=(const FolderC& f) const { return name != f.name; }
   inline int	operator< (const FolderC& f) const { return (compare(f) < 0); }
   inline int	operator> (const FolderC& f) const { return (compare(f) > 0); }

   FolderC(const char*, FolderTypeT type, Boolean create=True);
   virtual ~FolderC();

   Boolean		 IsImap()	const { return (type == IMAP_FOLDER); }
   Boolean		 IsMh()		const { return (type == MH_FOLDER); }
   Boolean		 IsMmdf()	const { return (type == MMDF_FOLDER); }
   Boolean		 IsUnix()	const { return (type == UNIX_FOLDER); }
   VItemListC&		 DelItemList()	{ return *delItemList; }
   VItemListC&		 MsgItemList()	{ return *msgItemList; }

   void			 Activate();
   long			 AddIndexEntry(u_int, int, int, char*);
   Boolean		 AddPartial(MsgC*);
   Boolean		 Changed();
   void			 CloseIndex(Boolean force=False);
   void			 CreateIcon();
   void			 Deactivate();
   void			 DeleteAllMessages();
   int			 DeletedMsgCount()	const;
   Boolean		 HasMessagesWithStatus(StatusFn*);
   Boolean		 HasDeletedMessages();
   Boolean		 HasNewMessages();
   Boolean		 HasUnreadMessages();
   Boolean		 IndexValid();
   void			 ItemsDeleted(VItemListC&);
   void			 ItemsUndeleted(VItemListC&);
   void			 MsgDeleted(MsgC*);
   void			 MsgUndeleted(MsgC*);
   void			 MsgStatusChanged(MsgC*);
   int			 MsgCount()		const;
   MsgC			*MsgWithIndex(u_int)	const;
   MsgC			*MsgWithNumber(int)	const;
   MsgC			*NewestMsg()		const;
   int			 NewMsgCount()		const;
   Boolean		 OpenIndex();
   void			 Print(ostream&)	const;
   void			 ReplaceMsg(MsgC*, MsgC*);
   Boolean		 Rescan();
   int			 SavedMsgCount()	const;
   void			 SetChanged(Boolean);
   void			 SetSortMgr(SortMgrC*);
   void			 Sort(CharC);
   SortMgrC		*SortMgr()		const;
   int			 UnreadMsgCount()	const;
   void			 UpdateFields();
   void			 UpdateIcon();
   Boolean		 UpdateIndexEntry(MsgC*);

   virtual Boolean	 Lock(Boolean) { return True; }
   virtual Boolean       AddMessage(MsgC*) = 0;
   virtual Boolean       AddMessage(StringListC&, char*) = 0;
   virtual time_t	 ModTime() { return 0; }
   virtual Boolean	 NewMail() = 0;
   virtual Boolean	 Save()    = 0;
   virtual Boolean	 Scan()    = 0;
};

//
// Method to allow <<
//
inline ostream& operator<<(ostream& strm, const FolderC& f)
{
   f.Print(strm);
   return strm;
}
#endif // _FolderC_h_
