/*
 * $Id: MhFolderC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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

#ifndef _MhFolderC_h_
#define _MhFolderC_h_

#include "FolderC.h"
#include <hgl/IntListC.h>

#include <time.h>
#include <sys/stat.h>

class MhMsgC;
class StringC;

class MhFolderC : public FolderC {

   time_t	lastLoadTime;
   time_t	lastDirTime;
   IntListC	dirList;
   struct stat	stats;
   Boolean	changedWhileSleeping;

   void		GetNextMsgFile(StringC&);
   Boolean	ReadDirectory();
   Boolean	Save(MhMsgC*);

public:

   MhFolderC(const char*, Boolean create=True);
   ~MhFolderC();

   Boolean		AddMessage(MsgC*);
   Boolean		AddMessage(StringListC&, char*);
   time_t		ModTime() { return stats.st_mtime; }
   Boolean		NewMail();
   Boolean		Save();
   Boolean		Scan();
};

#endif // _MhFolderC_h_
