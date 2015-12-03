/*
 *  $Id: FileFolderC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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
#ifndef _FileFolderC_h_
#define _FileFolderC_h_

#include "FolderC.h"

#include <sys/stat.h>

class FileFolderC : public FolderC {

protected:

   int		openLevel;
   int		lockLevel;
   StringC	lockFile;
   struct stat	stats;
   Boolean	changedWhileSleeping;

   virtual Boolean	ScanNew() = 0;
   static Boolean	CreateLock(const char *file);
   Boolean		CreateSaveFile(u_long, StringC&, Boolean*);
   virtual Boolean	WriteMessage(MsgC*, FILE*) = 0;
   virtual Boolean	WriteMessage(StringListC&, char*, FILE*) = 0;

public:

   FILE		*fp;

   FileFolderC(const char*, FolderTypeT type, Boolean create=True);
   virtual ~FileFolderC();

   Boolean		Lock(Boolean);
   Boolean		OpenFile();
   void			CloseFile(Boolean force=False);

   Boolean		AddMessage(MsgC*);
   Boolean		AddMessage(StringListC&, char*);
   time_t               ModTime() { return stats.st_mtime; }
   Boolean		NewMail();
   Boolean		Save();
   virtual Boolean	Scan()    = 0;
};

#endif // _FileFolderC_h_
