/*
 *  $Id: FileMsgC.C,v 1.2 2000/05/07 12:26:11 fnevgeny Exp $
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

#include <config.h>
#include "FileMsgC.h"
#include "MsgPartC.h"

#include <hgl/StringC.h>
#include <hgl/SysErr.h>
#include <hgl/HalAppC.h>

#include <errno.h>
#include <unistd.h>

extern int	debuglev;

/*---------------------------------------------------------------
 *  Constructor
 */

FileMsgC::FileMsgC(const char *name, int offset, int bytes, Boolean del,
		  FolderC *f)
: MsgC(f)
{
   type          = FILE_MSG;
   filename      = name;
   deleteFile    = del;
   openLevel     = 0;
   fp            = NULL;

   body->msgFile = name;
   body->offset  = offset;
   body->bytes   = bytes;

   ScanHead();
}

/*---------------------------------------------------------------
 *  Destructor
 */

FileMsgC::~FileMsgC()
{
   CloseFile(True/*force*/);
   if ( deleteFile && filename.size() > 0 ) unlink(filename);
}

/*---------------------------------------------------------------
 *  Method to open file that can be used by const objects
 */

FILE*
FileMsgC::OpenFile() const
{
   if ( fp ) return fp;

//
// Open the file
//
   if ( debuglev > 1 ) cout <<"Opening message file " <<filename <<endl;
   FILE	*filep = fopen(filename, "r");
   if ( !filep ) {
      StringC   errmsg("Could not open file: ");
      errmsg += filename;
      errmsg += "\n" + SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
   }

   return filep;
}

/*---------------------------------------------------------------
 *  Method to open file that can be used by non-const objects
 */

Boolean
FileMsgC::OpenFile()
{
   openLevel++;
   if ( fp ) return True;

//
// Open the file
//
   if ( debuglev > 1 ) cout <<"Opening message file " <<filename <<endl;
   fp = fopen(filename, "r");
   if ( !fp ) {
      StringC   errmsg("Could not open file: ");
      errmsg += filename;
      errmsg += "\n" + SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
      openLevel = 0;
      return False;
   }

   return True;
}

/*---------------------------------------------------------------
 *  Method to close file that can be used by const objects
 */

void
FileMsgC::CloseFile(FILE *filep) const
{
   if ( !filep || filep == fp ) return;

   if ( debuglev > 1 ) cout <<"Closing message file " <<filename <<endl;
   fclose(filep);
}

/*---------------------------------------------------------------
 *  Method to close file that can be used by non-const objects
 */

void
FileMsgC::CloseFile(Boolean force)
{
   if ( !fp ) return;

   if ( force ) openLevel = 0;
   else		openLevel--;
   if ( openLevel > 0 ) return;
   if ( openLevel < 0 ) openLevel = 0;

   if ( debuglev > 1 ) cout <<"Closing message file " <<filename <<endl;
   fclose(fp);
   fp = NULL;
}

/*---------------------------------------------------------------
 *  Method to change the file name
 */

void
FileMsgC::SetFile(const char *name, Boolean del)
{
   CloseFile(True/*force*/);
   if ( deleteFile && filename.size() > 0 ) unlink(filename);

   filename   = name;
   deleteFile = del;
}

/*---------------------------------------------------------------
 *  Methods to return the header text
 */

Boolean
FileMsgC::GetHeaderText(StringC& text) const
{
   FILE	*filep = OpenFile();
   if ( !filep ) return False;

   Boolean	success = ReadHeaderText(filep, text);

   CloseFile(filep);

   return success;
}

Boolean
FileMsgC::GetHeaderText(StringC& text)
{
   if ( !OpenFile() ) return False;

   Boolean success = ReadHeaderText(fp, text);

   CloseFile();

   return success;
}

/*---------------------------------------------------------------
 *  Methods to return the entire text for the specified part
 */

Boolean
FileMsgC::GetPartText(const MsgPartC *part, StringC& text, Boolean getHead,
		      Boolean getExtHead, Boolean getBody) const
{
   FILE	*filep = OpenFile();
   if ( !filep ) return False;

   Boolean success = part->GetText(text, filep, getHead, getExtHead, getBody);

   CloseFile(filep);

   return success;
}

Boolean
FileMsgC::GetPartText(const MsgPartC *part, StringC& text, Boolean getHead,
		      Boolean getExtHead, Boolean getBody)
{
   if ( !OpenFile() ) return False;

   Boolean success = part->GetText(text, fp, getHead, getExtHead, getBody);

   CloseFile();

   return success;
}

/*---------------------------------------------------------------
 *  Methods to return the decoded contents of this body part
 */

Boolean
FileMsgC::GetFileData(MsgPartC *part, StringC& text) const
{
   FILE	*filep = OpenFile();
   if ( !filep ) return False;

   Boolean	success = part->GetData(text, filep);

   CloseFile(filep);

   return success;
}

Boolean
FileMsgC::GetFileData(MsgPartC *part, StringC& text)
{
   if ( !OpenFile() ) return False;

   Boolean	success = part->GetData(text, fp);

   CloseFile();

   return success;
}

/*---------------------------------------------------------------
 *  Method to scan the file and determine the header sizes and body byte count
 */

void
FileMsgC::ScanHead()
{
   if ( !OpenFile() ) return;

   ScanHeadFile(fp);

//
// Calculate the length of the body
//
   if ( body->bytes >= 0 ) {

      body->bodyBytes = body->bytes - body->headBytes - 1;
      if ( body->bodyBytes < 0 ) body->bodyBytes = 0;

      body->bytes = body->headBytes + 1;
      if ( body->extBytes > 0 ) body->bytes += (body->extBytes + 1);
      body->bytes += body->bodyBytes;
   }

   CloseFile();

} // End ScanHead

/*---------------------------------------------------------------
 *  Method to scan the file and determine the body line count
 */

void
FileMsgC::ScanBody()
{
   if ( !OpenFile() ) return;

   ScanBodyFile(fp);

   CloseFile();

} // End ScanBody

/*------------------------------------------------------------------------
 * Method to write the body to a file
 */

Boolean
FileMsgC::WriteBody(FILE *dstfp, Boolean addBlank, Boolean protectFroms)
{
   if ( !OpenFile() ) return False;

   Boolean	success = CopyBody(fp, dstfp, addBlank, protectFroms);

   CloseFile();

   return success;
}
