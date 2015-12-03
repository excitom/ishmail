/*
 *  $Id: SaveMgrC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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
#ifndef _SaveMgrC_h_
#define _SaveMgrC_h_

#include "Query.h"

#include <hgl/StringC.h>

class VItemListC;
class MsgC;
class FolderC;
class StringListC;
class FileChooserWinC;

class SaveMgrC {

   FileChooserWinC	*saveFolderWin;
   Widget		folderOpFrame;
   Widget		folderMoveTB;
   Widget		folderCopyTB;

   FileChooserWinC	*saveFileWin;
   Widget		fileOpFrame;
   Widget		fileMoveTB;
   Widget		fileCopyTB;
   Widget		headAllTB;
   Widget		headDispTB;
   Widget		headNoneTB;
   Widget		saveToFileQueryWin;

   MsgC			*saveMsg;
   VItemListC		*saveItemList;

   void			BuildSaveFileWin  (Widget);
   void			BuildSaveFolderWin(Widget);
   QueryAnswerT		SaveToFileQuery(const char*);
   void  		VerifySaveList(StringListC&, FileChooserWinC*);

   static void		FinishSaveFile   (StringListC*, SaveMgrC*);
   static void		FinishSaveFolder (StringListC*, SaveMgrC*);
   static void		VerifySaveFiles  (StringListC*, SaveMgrC*);
   static void		VerifySaveFolders(StringListC*, SaveMgrC*);


public:

//
// Public data
//
   StringC	curSaveFolder;

//
// Public methods
//
   SaveMgrC();
   ~SaveMgrC();

   void		SaveMsgToFolder(MsgC *src, FolderC *dst,     Boolean delAfter);
   void		SaveMsgToFolder(MsgC *src, const char *name, Boolean delAfter);
   void		SaveMsgToFolder(MsgC *src, Widget parent,    Boolean moveOk);
   void		SaveMsgToFile  (MsgC *src, Widget parent,    Boolean moveOk);
   void		SaveMsgToFile  (MsgC *src, const char *name,
				Boolean delAfter, Boolean copyHdrs,
				Boolean allHdrs);

   void		SaveMsgsToFolder(VItemListC& msgList, FolderC *dst,
   				 Boolean delAfter);
   void		SaveMsgsToFolder(VItemListC& msgList, const char *name,
   				 Boolean delAfter);
   void		SaveMsgsToFolder(VItemListC& msgList, Widget parent,
   				 Boolean moveOk);
   void		SaveMsgsToFile  (VItemListC& msgList, Widget parent,
   				 Boolean moveOk);
   void		SaveMsgsToFile  (VItemListC& msgList, const char *name,
				 Boolean delAfter, Boolean copyHdrs,
				 Boolean allHdrs);

   void		UpdateSaveFolder(MsgC*);
};

#endif // _SaveMgrC_h_
