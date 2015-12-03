/*
 *  $Id: UnixMsgC.C,v 1.2 2000/05/07 12:26:13 fnevgeny Exp $
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
#include "UnixMsgC.h"
#include "MsgPartC.h"
#include "UnixFolderC.h"
#include "HeaderValC.h"
#include "AddressC.h"

#include <hgl/RegexC.h>

#include <unistd.h>

#define MAXLINE	255

extern int	debuglev;

/*---------------------------------------------------------------
 *  Constructor
 */

UnixMsgC::UnixMsgC(UnixFolderC *uf, int num, u_int off, const char *frm)
: FilePartMsgC(uf, num, off)
{
   type = UNIX_MSG;

   fromLine = frm;
   fromLine.Trim();

   useContentLen   = False;
   contentLen      = 0;
   contentLenExtra = 0;

   ScanHead();

//
// See if we have a content-length header
//
   HeaderValC	*val = HeaderValue("Content-Length");
   if ( val && val->text ) {

      contentLen    = atoi(val->text);
      useContentLen = True;
      if ( debuglev > 0 ) cout <<"Checking Content-Length " <<contentLen <<endl;

//
// See if the content length is correct
//
      if ( !uf->OpenFile() ) return;

      if ( fseek(uf->fp, body->bodyOffset+contentLen, SEEK_SET) < 0 ) {
	 fseek(uf->fp, 0, SEEK_END);
	 long	size = ftell(uf->fp);
	 int	len  = (int)(size - (long)body->bodyOffset);
	 if ( debuglev > 0 )
	    cout <<"There are only " <<len <<" bytes available." <<endl;
	 int	diff = contentLen - len;
	 if ( diff < 0 ) diff = -diff;
	 if ( diff <= 100 ) contentLen = len;
      }

//
// Read until we hit a From line
//
      else {

	 char	line[MAXLINE+1];
	 contentLenExtra = 0;
	 while ( fgets(line, MAXLINE, uf->fp) && !uf->fromPat->match(line) )
	    contentLenExtra += strlen(line);

//
// See how much we had to read to get to the From line
//
	 if ( debuglev > 0 && contentLenExtra > 1 )
	    cout <<"Had to read " <<contentLenExtra
		 <<" bytes past stated content length." <<endl;
      }

      uf->CloseFile();

   } // End if there is a content length header

//
// If we don't have a from address, get it from the from line
//
   if ( !from && uf->fromPat->match(fromLine) ) {
      StringC	tmp = "From: ";
      tmp += fromLine.Range((*uf->fromPat)[1]);
      body->AddHeader(tmp);
   }

} // End constructor

/*---------------------------------------------------------------
 *  Destructor
 */

UnixMsgC::~UnixMsgC()
{
}

/*---------------------------------------------------------------
 *  Method to check if the given line should terminate a message.  It does
 *     if it's a From line and we don't have a content-length header or
 *     if we have reached the content-length.
 */

Boolean
UnixMsgC::TerminatingLine(UnixMsgC *msg, CharC line, Boolean prevBlank)
{
   UnixFolderC	*uf = (UnixFolderC*)msg->folder;

   Boolean	foundFrom = (prevBlank && line.StartsWith("From ") &&
			     UnixFolderC::fromPat->match(line));
   u_int	fromPos;
   if ( foundFrom ) // Get position relative to start of body
      fromPos = (u_int)ftell(uf->fp) - line.Length() - msg->BodyOffset();

   if ( msg->useContentLen ) {

//
// If we've hit a from line, see if it's closer to the stated content length
//    than the one after.  We located the one after in ScanHead.
//
      if ( foundFrom ) {
	 u_int	diff = msg->contentLen - fromPos;
	 if ( diff < msg->contentLenExtra ) {
	    msg->contentLen      = fromPos - 1;
	    msg->contentLenExtra = 0;
	 }
      }

//
// If we reached the content length without hitting a From line, we can just
//    ignore this content length
//
      else if ( msg->body->bodyBytes >= msg->contentLen ) {
	 msg->contentLen    = 0;	// Quit checking
	 msg->useContentLen = False;
      }

   } // End if using content length

   else if ( foundFrom ) {
      msg->contentLen      = fromPos - 1;
      msg->contentLenExtra = 0;
      msg->useContentLen   = True;
   }

   return foundFrom;

} // End TerminatingLine

void*
UnixMsgC::TerminatingLineFn()
{
   return (void*)TerminatingLine;
}

/*---------------------------------------------------------------
 *  Method to check for unprotected "From " lines in the message body
 */

void
UnixMsgC::CheckFroms(CharC text)
{
   if ( IsSet(MSG_FROMS_CHECKED) ) return;

   if ( debuglev > 0 )
      cout <<"Checking for From in body of message " <<number <<endl;

//
// We will only protect From lines that match the pattern
//
   RegexC	*fromPat = UnixFolderC::fromPat;
   if ( text.StartsWith("From ") && (!fromPat || fromPat->match(text)) )
      SetStatus(MSG_OPEN_FROMS, False);

   else {
      int	pos = text.PosOf("\nFrom ");
      if ( pos >= 0 && (!fromPat || fromPat->match(text, pos+2)) )
	 SetStatus(MSG_OPEN_FROMS, False);
   }

//
// We will unprotect all ">From " lines since those are the ones protected
//    by /bin/mail when mail is delivered
//
   if ( text.StartsWith(">From ") || text.Contains("\n>From ") )
      SetStatus(MSG_SAFE_FROMS, False);

   SetStatus(MSG_FROMS_CHECKED);

} // End CheckFroms

/*---------------------------------------------------------------
 *  Method to query for existence of unprotected Froms that can be called
 *     by const objects
 */

Boolean
UnixMsgC::HasOpenFroms() const
{
   if ( IsSet(MSG_FROMS_CHECKED) ) return IsSet(MSG_OPEN_FROMS);

//
// Get the body text
//
   UnixFolderC	*uf = (UnixFolderC*)folder;
   if ( !uf->OpenFile() ) return False;

   StringC	text;
   Boolean success = body->GetText(text, uf->fp, False, False, True, False);

   uf->CloseFile();

   if ( !success ) return False;

//
// Check the text.  We can't call CheckFroms because we are const and it
//    updates the status
//
   if ( debuglev > 0 )
      cout <<"Checking for From in body of message " <<number <<endl;

   RegexC	*fromPat = UnixFolderC::fromPat;
   if ( text.StartsWith("From ") && (!fromPat || fromPat->match(text)) )
      return True;

   else {
      int	pos = text.PosOf("\nFrom ");
      if ( pos >= 0 && (!fromPat || fromPat->match(text, pos+1)) )
	 return True;
   }

   return False;

} // End HasOpenFroms

/*---------------------------------------------------------------
 *  Method to query for existence of unprotected Froms that can be called
 *     by non-const objects
 */

Boolean
UnixMsgC::HasOpenFroms()
{
   if ( IsSet(MSG_FROMS_CHECKED) ) return IsSet(MSG_OPEN_FROMS);

//
// Get the body text
//
   UnixFolderC	*uf = (UnixFolderC*)folder;
   if ( !uf->OpenFile() ) return False;

   StringC	text;
   Boolean success = body->GetText(text, uf->fp, False, False, True, False);

   uf->CloseFile();

   if ( !success ) return False;

//
// Check the text
//
   CheckFroms(text);

   return IsSet(MSG_OPEN_FROMS);
}

/*---------------------------------------------------------------
 *  Methods to query for existence of protected Froms that can be called
 *     by const objects
 */

Boolean
UnixMsgC::HasSafeFroms() const
{
   if ( IsSet(MSG_FROMS_CHECKED) ) return IsSet(MSG_SAFE_FROMS);

//
// Get the body text
//
   UnixFolderC	*uf = (UnixFolderC*)folder;
   if ( !uf->OpenFile() ) return False;

   StringC	text;
   Boolean success = body->GetText(text, uf->fp, False, False, True, False);

   uf->CloseFile();

   if ( !success ) return False;

//
// Check the text.  We can't call CheckFroms because we are const and it
//    updates the status
//
   if ( debuglev > 0 )
      cout <<"Checking for >From in body of message " <<number <<endl;

   if ( text.StartsWith(">From ") || text.Contains("\n>From ") )
      return True;

   return False;

} // End HasSafeFroms

/*---------------------------------------------------------------
 *  Method to query for existence of protected Froms that can be called
 *     by non-const objects
 */

Boolean
UnixMsgC::HasSafeFroms()
{
   if ( IsSet(MSG_FROMS_CHECKED) ) return IsSet(MSG_SAFE_FROMS);

//
// Get the body text
//
   UnixFolderC	*uf = (UnixFolderC*)folder;
   if ( !uf->OpenFile() ) return False;

   StringC	text;
   Boolean success = body->GetText(text, uf->fp, False, False, True, False);

   uf->CloseFile();

   if ( !success ) return False;

//
// Check the text
//
   CheckFroms(text);

   return IsSet(MSG_SAFE_FROMS);
}

