/*
 *  $Id: FilePartMsgC.C,v 1.2 2000/05/07 12:26:11 fnevgeny Exp $
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
#include "FilePartMsgC.h"
#include "MsgPartC.h"
#include "FileFolderC.h"
#include "HeaderValC.h"

#include <unistd.h>

#define MAXLINE	255

/*---------------------------------------------------------------
 *  Constructor
 */

FilePartMsgC::FilePartMsgC(FileFolderC *fld, int num, u_int msgOff) : MsgC(fld)
{
   number        = num;
   body->offset  = msgOff;
   body->msgFile = fld->name;
}

/*---------------------------------------------------------------
 *  Destructor
 */

FilePartMsgC::~FilePartMsgC()
{
}

/*---------------------------------------------------------------
 *  Method to return the header text that can be called by const objects
 */

Boolean
FilePartMsgC::GetHeaderText(StringC& text) const
{
   FileFolderC	*ff = (FileFolderC*)folder;

   if ( !ff->OpenFile() ) return False;

   Boolean	success = ReadHeaderText(ff->fp, text);
   ff->CloseFile();

   return success;
}

/*---------------------------------------------------------------
 *  Method to return the header text that can be called by non-const objects
 */

Boolean
FilePartMsgC::GetHeaderText(StringC& text)
{
   FileFolderC	*ff = (FileFolderC*)folder;

   if ( !ff->OpenFile() ) return False;

   Boolean	success = ReadHeaderText(ff->fp, text);
   ff->CloseFile();

   return success;
}

/*---------------------------------------------------------------
 *  Methods to return the body text for the specified part
 */

Boolean
FilePartMsgC::GetPartText(const MsgPartC *part, StringC& text, Boolean getHead,
			  Boolean getExtHead, Boolean getBody) const
{
   FileFolderC	*ff = (FileFolderC*)folder;
   if ( !ff->OpenFile() ) return False;

   Boolean	success = part->GetText(text, ff->fp, getHead, getExtHead,
					getBody, IsSet(MSG_SAFE_FROMS));

   ff->CloseFile();

   return success;
}

Boolean
FilePartMsgC::GetPartText(const MsgPartC *part, StringC& text, Boolean getHead,
			  Boolean getExtHead, Boolean getBody)
{
   FileFolderC	*ff = (FileFolderC*)folder;
   if ( !ff->OpenFile() ) return False;

   Boolean	success = part->GetText(text, ff->fp, getHead, getExtHead,
					getBody, IsSet(MSG_SAFE_FROMS));

   ff->CloseFile();

   return success;
}

/*---------------------------------------------------------------
 *  Methods to return the decoded contents of this body part
 */

Boolean
FilePartMsgC::GetFileData(MsgPartC *part, StringC& text) const
{
   FileFolderC	*ff = (FileFolderC*)folder;
   if ( !ff->OpenFile() ) return False;

   Boolean	success = part->GetData(text, ff->fp);

   ff->CloseFile();

   return success;
}

Boolean
FilePartMsgC::GetFileData(MsgPartC *part, StringC& text)
{
   FileFolderC	*ff = (FileFolderC*)folder;
   if ( !ff->OpenFile() ) return False;

   Boolean	success = part->GetData(text, ff->fp);

   ff->CloseFile();

   return success;
}

/*---------------------------------------------------------------
 *  Method to scan the the headers from the offset to the next blank
 *     or From line or until the content-length is reached
 */

void
FilePartMsgC::ScanHead()
{
   FileFolderC	*ff = (FileFolderC*)folder;

   if ( !ff->OpenFile() ) return;

   ScanHeadFile(ff->fp);

   ff->CloseFile();

} // End ScanHead

/*---------------------------------------------------------------
 *  Method to scan the the body from the offset to the next AAAA line
 */

void
FilePartMsgC::ScanBody()
{
   FileFolderC	*ff = (FileFolderC*)folder;

   if ( !ff->OpenFile() ) return;

   ScanBodyFile(ff->fp);

   ff->CloseFile();

//
// Remove final blank line from counts
//
   if ( type == UNIX_MSG ) {

      if ( body->bodyBytes > 0 ) {
	 body->bytes--;
	 body->bodyBytes--;
      }
      if ( body->bodyLines > 0 ) {
	 body->lines--;
	 body->bodyLines--;
      }
   }

} // End ScanBody

/*------------------------------------------------------------------------
 * Method to write the body to a file
 */

Boolean
FilePartMsgC::WriteBody(FILE *dstfp, Boolean addBlank, Boolean protectFroms)
{
   FileFolderC	*ff = (FileFolderC*)folder;

   if ( !ff->OpenFile() ) return False;

   Boolean	success = CopyBody(ff->fp, dstfp, addBlank, protectFroms);

   ff->CloseFile();

   return success;
}

