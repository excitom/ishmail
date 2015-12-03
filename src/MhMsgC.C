/*
 *  $Id: MhMsgC.C,v 1.2 2000/05/07 12:26:12 fnevgeny Exp $
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
#include "MhMsgC.h"
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

MhMsgC::MhMsgC(FolderC *fld, const char *name, int num, Boolean del)
: MsgC(fld)
{
   type       = MH_MSG;
   filename   = name;
   fp         = NULL;
   number     = num;
   deleteFile = del;
   openLevel  = 0;

   body->msgFile = name;

   ScanHead();
}

/*---------------------------------------------------------------
 *  Destructor
 */

MhMsgC::~MhMsgC()
{
   CloseFile(True/*force*/);
   if ( deleteFile && filename.size() > 0 ) unlink(filename);
}

/*---------------------------------------------------------------
 *  Method to open file that can be used by const objects
 */

FILE*
MhMsgC::OpenFile() const
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
MhMsgC::OpenFile()
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
MhMsgC::CloseFile(FILE *filep) const
{
   if ( !filep || filep == fp ) return;

   if ( debuglev > 1 ) cout <<"Closing message file " <<filename <<endl;
   fclose(filep);
}

/*---------------------------------------------------------------
 *  Method to close file that can be used by non-const objects
 */

void
MhMsgC::CloseFile(Boolean force)
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
MhMsgC::SetFile(const char *name, Boolean del)
{
   CloseFile(True/*force*/);
   if ( deleteFile && filename.size() > 0 ) unlink(filename);

   filename   = name;
   deleteFile = del;
}

/*---------------------------------------------------------------
 *  Method to return the header text that can be called by const objects
 */

Boolean
MhMsgC::GetHeaderText(StringC& text) const
{
   FILE	*filep = OpenFile();
   if ( !filep ) return False;

   Boolean	success = ReadHeaderText(filep, text);

   CloseFile(filep);

   return success;
}

/*---------------------------------------------------------------
 *  Method to return the header text that can be called by non-const objects
 */

Boolean
MhMsgC::GetHeaderText(StringC& text)
{
   if ( !OpenFile() ) return False;

   Boolean	success = ReadHeaderText(fp, text);

   CloseFile();

   return success;
}

/*---------------------------------------------------------------
 *  Methods to return the entire text for the specified part
 */

Boolean
MhMsgC::GetPartText(const MsgPartC *part, StringC& text, Boolean getHead,
		      Boolean getExtHead, Boolean getBody) const
{
   FILE	*filep = OpenFile();
   if ( !filep ) return False;

   Boolean success = part->GetText(text, filep, getHead, getExtHead, getBody);

   CloseFile(filep);

   return success;
}

Boolean
MhMsgC::GetPartText(const MsgPartC *part, StringC& text, Boolean getHead,
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
MhMsgC::GetFileData(MsgPartC *part, StringC& text) const
{
   FILE	*filep = OpenFile();
   if ( !filep ) return False;

   Boolean	success = part->GetData(text, filep);

   CloseFile(filep);

   return success;
}

Boolean
MhMsgC::GetFileData(MsgPartC *part, StringC& text)
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
MhMsgC::ScanHead()
{
   if ( !OpenFile() ) return;

   ScanHeadFile(fp);

//
// Calculate the length of the body
//
   fseek(fp, 0, SEEK_END);
   long	end  = ftell(fp);
   body->bodyBytes = (int)(end - body->offset);

   CloseFile();

} // End ScanHead

/*---------------------------------------------------------------
 *  Method to scan the file and determine the body line count
 */

void
MhMsgC::ScanBody()
{
   if ( !OpenFile() ) return;

   ScanBodyFile(fp);

   CloseFile();

} // End ScanBody

/*------------------------------------------------------------------------
 * Method to write the body to a file
 */

Boolean
MhMsgC::WriteBody(FILE *dstfp, Boolean addBlank, Boolean protectFroms)
{
   if ( !OpenFile() ) return False;

   Boolean	success = CopyBody(fp, dstfp, addBlank, protectFroms);

   CloseFile();

   return success;
}
