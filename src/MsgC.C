/*
 * $Id: MsgC.C,v 1.3 2000/09/19 16:42:02 evgeny Exp $
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

#include <config.h>

#include "MsgC.h"
#include "AddressC.h"
#include "HeaderC.h"
#include "HeaderValC.h"
#include "MsgPartC.h"
#include "FolderC.h"
#include "IshAppC.h"
#include "HeadPrefC.h"
#include "MsgItemC.h"
#include "MainWinC.h"
#include "RuleDictC.h"
#include "date.h"
#include "ReplyPrefC.h"
#include "TermFn.h"
#include "SendWinC.h"

#include <hgl/RegexC.h>
#include <hgl/StringC.h>
#include <hgl/CharC.h>
#include <hgl/PtrListC.h>
#include <hgl/StringListC.h>
#include <hgl/SysErr.h>

#include <unistd.h>
#include <errno.h>

RegexC  *MsgC::rePat = NULL;

extern int	debuglev;

/*---------------------------------------------------------------
 *  Constructor
 */

MsgC::MsgC(FolderC *fld)
{
   if ( !rePat ) rePat = new RegexC("^[ \t]*[Rr][Ee][[(]?[0-9]*[])]?:[ \t]*");

   status        = MSG_NEW;
   epochTime     = 0;
   folder	 = fld;
   from          = NULL;
   to            = NULL;
   cc            = NULL;
   replyTo       = NULL;
   returnPath    = NULL;
   sender        = NULL;
   thread        = NULL;
   indexOffset   = -1;
   number        = 0;
   headWritePos  = 0;
   bodyWritePos  = 0;
   icon		 = NULL;
   bodySizeKnown = False;

   body = new MsgPartC;
   body->parentMsg = this;

} // End constructor

/*---------------------------------------------------------------
 *  Destructor
 */

MsgC::~MsgC()
{
//
// Notify all composition windows
//
   u_int	count = ishApp->sendWinList.size();
   for (int i=0; i<count; i++) {
      SendWinC	*win = (SendWinC*)*ishApp->sendWinList[i];
      if ( win->IsShown() ) win->MessageDeleted(this);
   }

   delete from;
   delete to;
   delete cc;
   delete replyTo;
   delete returnPath;
   delete sender;
   delete thread;
   delete icon;
   delete body;
}

/*---------------------------------------------------------------
 *  Method to add a new header to the linked list and check it's
 *     attributes.
 */

void
MsgC::CheckHeader(HeaderC *head)
{
//
// Check for Content- headers
//

   if ( head->key.StartsWith("Content-",   IGNORE_CASE) ||
	head->key.StartsWith("X-Content-", IGNORE_CASE) ) {

//
// Look for sun attachments
//
      if ( head->key.Equals("content-type", IGNORE_CASE) ) {
	 CharC	full(head->full);
	 if ( full.Contains("X-Sun-Attachment", IGNORE_CASE) )
	    SetStatus(MSG_SUN_ATTACH);
	 else if ( full.Contains("message/partial", IGNORE_CASE) )
	    SetStatus(MSG_PARTIAL);
      }

   } // End if this is a Content- header

//
// Check for other special headers
//
   else switch (head->key.Length()) {

      case 2:
	 if ( head->key.Equals("To", IGNORE_CASE) )
	    AddAddress(head, &to);
	 else if ( head->key.Equals("Cc", IGNORE_CASE) )
	    AddAddress(head, &cc);
	 break;

      case 4:
	 if ( head->key.Equals("From", IGNORE_CASE) )
	    AddAddress(head, &from);
	 else if ( head->key.Equals("Date", IGNORE_CASE) ) {
	    StringC	tmp;
	    head->GetValueText(tmp);
	    epochTime = parsedate(tmp, NULL);
	 }
	 break;

      case 6:
	 if ( head->key.Equals("Sender", IGNORE_CASE) )
	    AddAddress(head, &sender);
	 else if ( head->key.Equals("Status", IGNORE_CASE) ) {
	    ClearStatus(MSG_NEW);
	    CharC	full(head->full);
	    if ( full.Contains('R') ) SetStatus(MSG_READ);
	    else		      ClearStatus(MSG_READ);
	 }
	 break;

      case 7:
	 if ( head->key.Equals("Subject", IGNORE_CASE) && !thread )
	    GetThread(head);
	 break;

      case 8:
	 if ( head->key.Equals("Reply-To", IGNORE_CASE) )
	    AddAddress(head, &replyTo);
	 break;

      case 10:
	 if ( head->key.Equals("Message-Id", IGNORE_CASE) )
	    head->GetValueText(msgId);
	 break;

      case 11:
	 if ( head->key.Equals("Return-Path", IGNORE_CASE) )
	    AddAddress(head, &returnPath);
	 break;

      case 12:
	 if ( head->key.Equals("Mime-Version", IGNORE_CASE) )
	    SetStatus(MSG_MIME);
	 break;

   } // End switch length

} // End CheckHeader

/*---------------------------------------------------------------
 *  Method to add a new header to the linked list and check it's
 *     attributes.
 */

void
MsgC::AddAddress(HeaderC *head, AddressC **addr)
{
   AddressC	*newAddr = new AddressC(head->value->full);
   if ( !*addr )
      *addr = newAddr;
   else {
      AddressC	*last = *addr;
      while ( last->next ) last = last->next;
      last->next = newAddr;
   }

} // End AddAddress

/*------------------------------------------------------------------------
 * Method to extract the thread string.  This is the subject without any
 *    Re's or (fwd)'s
 */

void
MsgC::GetThread(HeaderC *head)
{
//
// Remove Re's at the beginning
//
   StringC	threadStr;
   head->GetValueText(threadStr);
   Boolean	done = False;
   while ( !done ) {

      if ( rePat->match(threadStr) )
	 threadStr.CutBeg((*rePat)[0].length());
      else if ( threadStr.StartsWith("fwd:", IGNORE_CASE) )
	 threadStr.CutBeg(4);
      else if ( threadStr.StartsWith("(fwd)", IGNORE_CASE) )
	 threadStr.CutBeg(5);
      else if ( threadStr.EndsWith("(fwd)", IGNORE_CASE) )
	 threadStr.CutEnd(5);
      else
	 done = True;

      threadStr.Trim();
   }

   thread = new char[threadStr.size()+1];
   strcpy(thread, threadStr);

} // End GetThread

/*---------------------------------------------------------------
 *  Method to print message to stream.
 */

void
MsgC::Print(ostream& strm) const
{
   if ( body->headers && debuglev > 1 ) {
      strm <<"........................................" <<endl;
      HeaderC	*head = body->headers;
      while ( head ) {
	 strm <<*head;
	 head = head->next;
      }
      strm <<"........................................" <<endl;
   }

   if ( from ) strm <<"  From: " <<*from <<endl;
   if ( to   ) strm <<"    To: " <<*to <<endl;
   if ( cc   ) strm <<"    Cc: " <<*cc <<endl;
   if ( debuglev > 1 ) {
      if ( replyTo    ) strm <<"   Reply-To: " <<*replyTo <<endl;
      if ( returnPath ) strm <<"Return-Path: " <<*returnPath <<endl;
      if ( sender     ) strm <<"     Sender: " <<*sender <<endl;
      if ( thread     ) strm <<"     Thread: " <<thread <<endl;
   }
   HeaderValC	*val = Date();
   if ( val ) strm <<"  Date: " <<*val <<endl;

   strm <<"Status: ";
   if ( IsSet(MSG_NEW)	         ) strm <<"New ";
   if ( IsSet(MSG_READ)	         ) strm <<"Read ";
   if ( IsSet(MSG_DELETED)	 ) strm <<"Deleted ";
   if ( IsSet(MSG_SAVED)	 ) strm <<"Saved ";
   if ( IsSet(MSG_REPLIED)	 ) strm <<"Replied ";
   if ( IsSet(MSG_FORWARDED)	 ) strm <<"Forwarded ";
   if ( IsSet(MSG_RESENT)	 ) strm <<"Resent ";
   if ( IsSet(MSG_PRINTED)	 ) strm <<"Printed ";
   if ( IsSet(MSG_FILTERED)	 ) strm <<"Filtered ";
   if ( IsSet(MSG_FLAGGED)	 ) strm <<"Flagged ";
   if ( IsSet(MSG_MIME)	         ) strm <<"Mime ";
   if ( IsSet(MSG_SUN_ATTACH)    ) strm <<"Sun ";
   if ( IsSet(MSG_OPEN_FROMS)    ) strm <<"Open-froms ";
   if ( IsSet(MSG_SAFE_FROMS)    ) strm <<"Safe-froms ";
   if ( IsSet(MSG_FROMS_CHECKED) ) strm <<"Froms-checked ";
   if ( IsSet(MSG_VIEWED)	 ) strm <<"Viewed ";
   if ( IsSet(MSG_IN_USE)	 ) strm <<"In-use ";
   if ( IsSet(MSG_CHANGED)	 ) strm <<"Changed ";
   if ( IsSet(MSG_PARTIAL)	 ) strm <<"Partial ";
   if (!body->IsPlainText()	 ) strm <<"Not-plain ";
   if ( body->IsEncoded()	 ) strm <<"Encoded ";
   strm <<endl;
   strm <<"........................................" <<endl;
   strm <<*body;
   strm <<"........................................" <<endl;
}

/*------------------------------------------------------------------------
 * Method to set number
 */

void
MsgC::SetNumber(int num)
{
   number = num;
   if ( icon ) icon->UpdateStatus();
}

/*------------------------------------------------------------------------
 * Method to set status flags.  If VIEWED is being set, we will also set
 *    READ.
 */

void
MsgC::SetStatus(MsgStatusT val, Boolean write)
{
   if ( (status & (u_int)val) != 0 ) return;	// Already set

   status |= (u_int)val;

   if ( !folder ) return;

//
// Viewed implies read
//
   if ( val == MSG_VIEWED ) {

      if ( IsRead() )
	 write = False;
      else {
	 status |= (u_int) MSG_READ;
	 status &= (u_int)~MSG_NEW;
	 status |= (u_int) MSG_CHANGED;
	 folder->MsgStatusChanged(this);
      }
   }

   else {

      if      ( val == MSG_READ ) status &= (u_int)~MSG_NEW;
      else if ( val == MSG_NEW  ) status &= (u_int)~MSG_READ;

      if ( val == MSG_DELETED )
	 folder->MsgDeleted(this);
      else if ( val == MSG_NEW || val == MSG_READ )
	 folder->MsgStatusChanged(this);

      status |= (u_int)MSG_CHANGED;
      //folder->SetChanged(True);
   }

//
// Update index file
//
   if ( write && indexOffset >= 0 && folder && folder->writable )
      folder->UpdateIndexEntry(this);

//
// Update visible icon
//
   if ( icon ) icon->UpdateStatus();

} // End SetStatus

/*------------------------------------------------------------------------
 * Method to clear status flags
 */

void
MsgC::ClearStatus(MsgStatusT val, Boolean write)
{
   if ( (status & (u_int)val) == 0 ) return;	// Already clear

   status &= ~(u_int)val;

   if ( !folder ) return;

   if ( val != MSG_VIEWED ) {

      if ( val == MSG_DELETED )
	 folder->MsgUndeleted(this);
      else if ( val == MSG_NEW || val == MSG_READ )
	 folder->MsgStatusChanged(this);

      if ( val != MSG_CHANGED ) {
	 status |= (u_int)MSG_CHANGED;
	 //folder->SetChanged(True);
      }

//
// Update index file
//
      if ( write && indexOffset >= 0 && folder && folder->writable )
	 folder->UpdateIndexEntry(this);

   } // End if not clearing VIEWED

//
// Update visible icon
//
   if ( icon ) icon->UpdateStatus();

} // End ClearStatus

/*------------------------------------------------------------------------
 * Method to check status flags
 */

Boolean
MsgC::IsSet(MsgStatusT val) const
{
   return ((status & (u_int)val) != 0);
}

/*------------------------------------------------------------------------
 * Method to return the string representation of the status
 */

void
MsgC::GetStatusString(StringC& val) const
{
   val.Clear();

   if ( IsSet(MSG_VIEWED)	 ) val += '*';

   if ( IsSet(MSG_PARTIAL)	 ) val += 'm';
   else if ( IsSet(MSG_MIME)	 ) val += 'M';

   if ( IsSet(MSG_NEW)	         ) val += 'N';
   else if ( !IsSet(MSG_READ)    ) val += 'U';

   if ( IsSet(MSG_DELETED)	 ) val += 'D';
   if ( IsSet(MSG_SAVED)	 ) val += 'S';
   if ( IsSet(MSG_REPLIED)	 ) val += '>';
   if ( IsSet(MSG_FORWARDED)	 ) val += 'F';
   if ( IsSet(MSG_RESENT)	 ) val += 'B';
   if ( IsSet(MSG_PRINTED)	 ) val += 'P';
   if ( IsSet(MSG_FILTERED)	 ) val += '|';

} // End GetStatusString

/*------------------------------------------------------------------------
 * Method to build address structure for a header
 */

AddressC*
MsgC::AddressOf(const char *head) const
{
   HeaderC	*data = Header(head);
   if ( !data ) return NULL;

   AddressC	*addr = new AddressC(data->value->full);
   return addr;
}

/*------------------------------------------------------------------------
 * Method to return text of subject header
 */

void
MsgC::GetSubjectText(StringC& substr) const
{
   substr.Clear();
   HeaderValC	*sub = Subject();
   if ( sub  ) sub->GetValueText(substr);
   else        substr = "[No subject]";
}

/*---------------------------------------------------------------
 *  Header query
 */

HeaderC*	MsgC::Headers()		const { return body->headers; }
HeaderC*	MsgC::Header(CharC key) const { return body->Header(key); }
HeaderValC*	MsgC::HeaderValue(CharC key) const
					  { return body->HeaderValue(key); }

u_int	MsgC::HeadOffset() { return body->offset;    }
int	MsgC::HeadBytes()  { return body->headBytes; }
int	MsgC::HeadLines()  { return body->headLines; }

/*---------------------------------------------------------------
 *  Methods to return the size of the body
 */

int
MsgC::BodyBytes()
{
   //if ( !bodySizeKnown ) ScanBody();
   if ( !bodySizeKnown && body->bodyBytes < 0 ) ScanBody();
   return body->bodyBytes;
}

int
MsgC::BodyLines()
{
   //
   // It's so painfully slow to scan all messages just to count
   // the body line, the function is disabled.
   //
   if (folder->IsImap()) return 0;

   //if ( !bodySizeKnown ) ScanBody();
   if ( !bodySizeKnown && body->bodyLines < 0 ) ScanBody();
   return body->bodyLines;
}

/*---------------------------------------------------------------
 *  Method to return a pointer to the body structure
 */

MsgPartC*
MsgC::Body()
{
   if ( !body->bodyScanned ) ScanBody();
   return body;
}

/*---------------------------------------------------------------
 *  Method to return the offset to the start of the body
 */

u_int
MsgC::BodyOffset()
{
   return body->bodyOffset;
}

/*---------------------------------------------------------------
 *  Method to return a pointer to the specified body part
 */

MsgPartC*
MsgC::Part(CharC partNum)
{
   if ( !body->bodyScanned ) ScanBody();
   return body->FindPart(partNum);
}

/*---------------------------------------------------------------
 *  Method to replace any header variables in the given string with
 *     their value.
 */

void
MsgC::ReplaceHeaders(StringC& str)
{
//
// Loop through string, substituting for header variables.
//
   CharC	key;
   StringC	valStr;
   int		pos;
   u_int	off = 0;
   while ( (pos=str.PosOf('%', off)) >= 0 ) {

//
// Replace %% with %
//
      if ( str[pos+1] == '%' ) {
	 str(pos+1,0) = "";
	 off = pos+1;
      }

//
// Replace %header with value
//
      else {

//
// Look for next non-alphanum
//
	 int	len = 0;
	 char	c = str[pos+1];
	 while ( !isspace(c) && (isalnum(c) || c == '-') ) {
	    len++;
	    c = str[pos+1+len];
	 }

	 key = str.Range(pos+1, len);
	 valStr.Clear();

//
// Look up value for header
//
	 if ( key.Length() > 0 ) {

//
// Check for special cases
//
	    if      ( key.Equals("fromname", IGNORE_CASE) ) {
	       if ( from ) {
		  if ( from->name ) from->name->GetValueText(valStr);
		  else		    valStr = from->addr;
	       }
	       else
		  valStr = "Unknown";
	    }

	    else if ( key.Equals("fromaddr", IGNORE_CASE) ) {
	       if ( from ) valStr = from->addr;
	       else	   valStr = "Unknown";
	    }

//
// Look up value
//
	    else {
	       HeaderValC	*val = HeaderValue(key);
	       if ( val ) val->GetValueText(valStr);
	       else	  valStr = "Unknown";
	    }

//
// Escape '<'s since this string will be used in enriched text
//
            if ( valStr.Contains('<') ) {

	       u_int	loff = 0;
	       int	lpos = valStr.PosOf('<', loff);
	       while ( lpos >= 0 ) {
		  valStr(lpos,0) = "<";
		  loff = lpos + 2;
		  lpos = valStr.PosOf('<', loff);
	       }
	    }

//
// Substitute value for %key
//
	    if ( valStr.length() == 0 ) valStr = "Unknown";
	    str(pos, key.Length()+1) = valStr;
	    off = pos + valStr.length();
	 }
	 else
	    off = pos + 1;

      } // End if %header found

   } // End for each %

} // End ReplaceHeaders

/*---------------------------------------------------------------
 *  Method to scan the the headers from the offset to the next blank
 *     or terminating line
 */

void
MsgC::ScanHeadFile(FILE *fp)
{
   if ( !body->ScanHead(fp) ) {
      return;
   }

   if ( debuglev > 1 ) {
      StringC	tmp;
      GetSubjectText(tmp);
      cout <<"Found: " <<tmp <<endl;
   }

//
// Check for special headers
//
   HeaderC	*head = body->headers;
   while ( head ) {
      CheckHeader(head);
      head = head->next;
   }

   body->SetPartNumber("1");

//
// Don't mark plain text as MIME
//
   if ( IsSet(MSG_MIME) && body->IsPlainText() && !body->IsEncoded() &&
      	!body->IsAttachment() && !body->IsExternal() )
      ClearStatus(MSG_MIME);

   ClearStatus(MSG_CHANGED);

} // End ScanHeadFile

/*---------------------------------------------------------------
 *  The default terminating line function
 */

static Boolean
TerminatingLine(MsgC*, CharC, Boolean)
{
   return False;
}

void*
MsgC::TerminatingLineFn()
{
   return (void*)TerminatingLine;
}

/*---------------------------------------------------------------
 *  Method to scan the the body from the offset to the next terminating line
 *     or EOF
 */

void
MsgC::ScanBodyFile(FILE *fp)
{
   if ( debuglev > 1 )
      cout <<"Scanning body for message " <<number
	   <<" at offset " <<body->bodyOffset <<endl;

   if ( !body->Scan(fp, body->bodyBytes, (TermFn*)TerminatingLineFn()) )
      return;

//
// Assign default types to parts that have no type
//
   body->SetPartNumber("1");
   body->SetPartNumberSize(body->GetPartNumberMaxSize());

   if ( debuglev > 1 ) cout <<"Found " <<body->bodyLines <<" lines" <<endl;

   bodySizeKnown = True;

} // End ScanBodyFile

/*---------------------------------------------------------------
 *  Method to return the header text
 */

Boolean
MsgC::ReadHeaderText(FILE *fp, StringC& text) const
{
   return body->GetText(text, fp, True, False, False, False);
}

/*---------------------------------------------------------------
 *  Method to return the body text
 */

Boolean
MsgC::ReadBodyText(FILE *fp, StringC& text) const
{
   return body->GetText(text, fp);
}

/*------------------------------------------------------------------------
 * Method to write this message to the specified file
 */

Boolean
MsgC::WriteFile(const char *file, Boolean copyHead, Boolean allHead,
		Boolean statHead, Boolean addBlank, Boolean protectFroms)
{
   FILE	*fp;
   fp = fopen(file, "a");
   if ( !fp ) {
      StringC	errmsg("Could not open file: ");
      errmsg += file;
      errmsg += ".\n";
      errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
      return False;
   }

   Boolean success = WriteFile(fp, copyHead, allHead, statHead, addBlank,
			       protectFroms);
   fclose(fp);

   return success;

} // End WriteFile

/*------------------------------------------------------------------------
 * Method to write this message to an open file
 */

Boolean
MsgC::WriteFile(FILE *fp, Boolean copyHead, Boolean allHead, Boolean statHead,
			  Boolean addBlank, Boolean protectFroms)
{
   if ( copyHead && !WriteHeaders(fp, allHead, statHead) )
      return False;

   if ( !WriteBody(fp, addBlank, protectFroms) )
      return False;

   return True;
}

/*------------------------------------------------------------------------
 * Method to write the headers to the specified file
 */

Boolean
MsgC::WriteHeaders(const char *file, Boolean allHead, Boolean statHead)
{
   FILE	*fp;
   fp = fopen(file, "a");
   if ( !fp ) {
      StringC	errmsg("Could not open file: ");
      errmsg += file;
      errmsg += ".\n";
      errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
      return False;
   }

   Boolean	success = WriteHeaders(fp, allHead, statHead);
   fclose(fp);

   return success;

} // End WriteHeaders

/*------------------------------------------------------------------------
 * Method to write the headers to an open file
 */

Boolean
MsgC::WriteHeaders(FILE *fp, Boolean allHead, Boolean statHead)
{
   headWritePos = ftell(fp);

   Boolean	someWritten = False;
   CharC	nlStr("\n");

//
// Loop through headers
//
   HeaderC	*head = body->headers;
   while ( head ) {

//
// Write this header if necessary
//
      Boolean	writeIt = allHead || ishApp->headPrefs->HeaderShown(head->key);
      if ( writeIt ) {

//
// If this is the Content-Length header, make sure it is correct
//
	 if ( head->key.Equals("content-length", IGNORE_CASE) ) {

	    StringC	lenStr;
	    head->GetValueText(lenStr);
	    lenStr.Trim();
	    int		len = atoi(lenStr);
	    if ( len != BodyBytes() ) {

	       if ( debuglev > 0 ) {
		  cout <<"Fixing Content-Length header for message " <<number;
		  cout <<"." <<endl;
		  cout <<"Was " <<len <<", should be " <<BodyBytes() <<endl;
	       }

	       lenStr = "Content-Length: ";
	       lenStr += (int)BodyBytes();

	       if ( !lenStr.WriteFile(fp) || !nlStr.WriteFile(fp) )
		  return False;

	       writeIt = False;

	    } // End if content length header is wrong

	 } // End if this is content length header

//
// If this is the Status header, save it for later
//
	 else if ( head->key.Equals("status", IGNORE_CASE) )
	    writeIt = False;

	 if ( writeIt ) {
	    CharC	headStr = head->full;
	    if ( !headStr.WriteFile(fp) || !nlStr.WriteFile(fp) )
	       return False;
	    someWritten = True;
	 }

      } // End if header is to be written

      head = head->next;

   } // End for each header

//
// Write the status line
//
   if ( statHead && !IsNew() ) {

      StringC	statStr("Status: ");
      if ( IsRead() ) statStr += 'R';
      statStr += "O\n";

      if ( !statStr.WriteFile(fp) ) return False;
   }

//
// Add a blank line if we wrote anything
//
   if ( someWritten ) {
      if ( !nlStr.WriteFile(fp) ) return False;
   }

   if ( fflush(fp) != 0 ) return False;

   return True;

} // End WriteHeaders

/*------------------------------------------------------------------------
 * Method to write the body to the specified file
 */

Boolean
MsgC::WriteBody(const char *file, Boolean addBlank, Boolean protectFroms)
{
   if ( !bodySizeKnown ) ScanBody();

   FILE	*fp;
   fp = fopen(file, "a");
   if ( !fp ) {
      StringC	errmsg("Could not open file: ");
      errmsg += file;
      errmsg += ".\n";
      errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
      return False;
   }

   Boolean	success = WriteBody(fp, addBlank, protectFroms);
   fclose(fp);

   return success;

} // End WriteBody

/*------------------------------------------------------------------------
 * Method to copy the body text from one file to another
 */

Boolean
MsgC::CopyBody(FILE *srcfp, FILE *dstfp, Boolean addBlank, Boolean protectFroms)
{
   if ( !bodySizeKnown ) ScanBody();

   CharC	nlStr("\n");

   long	writePos = ftell(dstfp);

//
// See if we need to worry about converting "From " lines to ">From "
//
   if ( protectFroms && type != UNIX_MSG &&
      	(!IsSet(MSG_FROMS_CHECKED) || IsSet(MSG_OPEN_FROMS)) ) {
      if ( !body->CopyText("", dstfp, srcfp, /*copyHead*/False,
	 		   /*copyExtHead*/body->IsExternal(),
			   /*copyBody*/True, /*protectFroms*/True,
			   /*restoreFroms*/False, /*endWithNL*/True) )
	 return False;
   }

//
// Else we don't want Froms protected
//
   if ( !protectFroms && type == UNIX_MSG &&
	(!IsSet(MSG_FROMS_CHECKED) || IsSet(MSG_SAFE_FROMS)) ) {
      if ( !body->CopyText("", dstfp, srcfp, /*copyHead*/False,
	 		   /*copyExtHead*/body->IsExternal(),
			   /*copyBody*/True, /*protectFroms*/False,
			   /*restoreFroms*/True, /*endWithNL*/True) )
	 return False;
   }

//
// If we don't have any From issues, we can just copy the text as-is
//
   else if ( !body->CopyText("", dstfp, srcfp, /*copyHead*/False,
			     /*copyExtHead*/body->IsExternal(),
			     /*copyBody*/True, /*protectFroms*/False,
			     /*restoreFroms*/False, /*endWithNL*/True) )
      return False;

//
// See if we need to add a blank line
//
   if ( addBlank && body->bodyLines > 0 && !nlStr.WriteFile(dstfp) )
      return False;

   bodyWritePos = writePos;

   if ( fflush(dstfp) != 0 ) return False;

   return True;

} // End CopyBody

/*------------------------------------------------------------------------
 * Method to determine the size of this message
 */

u_long
MsgC::SpaceNeeded()
{
   u_long	space = 0;

//
// Loop through headers
//
   HeaderC	*head = body->headers;
   while ( head ) {

//
// Skip the status header.  We'll do it below
//
      if ( !head->key.Equals("status", IGNORE_CASE) )
	 space += strlen(head->full) + 1/*nl*/;

      head = head->next;

   } // End for each header

//
// Now count the status header
//
   if ( !IsNew() ) {

      StringC	statStr = "Status: ";
      if ( IsRead() ) statStr += 'R';
      statStr += 'O';

      space += statStr.size() + 1/*nl*/;
   }

//
// Add blank line, body size and any extra needed by derived class
//
   space += 1 + BodyBytes() + ExtraSpaceNeeded();

   return space;

} // End SpaceNeeded

/*------------------------------------------------------------------------
 * Method to update the head and body offsets and the head size.  The new
 *    offsets will be taken from the last write positions.
 */

void
MsgC::UpdateOffsets()
{
   long	headDelta = headWritePos - body->offset;
   long	bodyDelta = bodyWritePos - body->bodyOffset;
   if ( headDelta == 0 && bodyDelta == 0 ) return;

   body->Move(headDelta, bodyDelta);

} // End UpdateOffsets

#if 0
/*------------------------------------------------------------------------
 * Method to write the index line to the index file
 */

void
MsgC::WriteIndex(FILE *fp)
{
//
// Don't write out the VIEWED bit
//
   u_int	tmp = status & (u_int)~MSG_VIEWED;

   indexOffset = ftell(fp);
   fprintf(fp, MSG_INDEX_WFMT, tmp, BodyBytes(), BodyLines(), (char*)msgId);
}
#endif

/*------------------------------------------------------------------------
 * Method to write the index line to the index file
 */

Boolean
MsgC::ReadIndex(FILE *fp)
{
//
// Move to the supposed position
//
   if ( indexOffset < 0 || fseek(fp, indexOffset, SEEK_SET) != 0 ) {
      if ( debuglev > 0 )
	 cout <<"Bad index offset " <<indexOffset
	      <<" for message " <<number <<endl;
      return False;
   }

//
// Read a line.  We can't scan the file directly because sometimes the
//    message id is blank.
//
#define MAXLINE 255
   char		line[MAXLINE+1];
   if ( !fgets(line, MAXLINE, fp) )
      return False;

   char		buf[128]; buf[0] = 0;
   u_int	statVal;
   int		bytes;
   int		lines;
   sscanf(line, MSG_INDEX_RFMT, &statVal, &bytes, &lines, buf);

//
// Check the id
//
   if ( msgId != buf ) {
      if ( debuglev > 0 ) {
	 cout <<"Bad id in index file for message " <<number <<endl;
	 cout <<"Expecting \"" <<msgId <<"\"" <<endl;
	 cout <<"Got       \"" <<buf   <<"\"" <<endl;
      }
      return False;
   }

//
// Save the data
//
   status          = statVal;
   body->bodyBytes = bytes;
   body->bodyLines = lines;
   bodySizeKnown   = True;

   body->ComputeSize();

   return True;

} // End ReadIndex

/*------------------------------------------------------------------------
 * Method to create message list icon
 */

void
MsgC::CreateIcon()
{
   if ( icon ) return;
   icon = new MsgItemC(this);

   ishApp->mainWin->RegisterMsgIcon(icon);

} // End CreateIcon

/*------------------------------------------------------------------------
 * Method to look for a rule that matches this message in the given rule dict
 */

StringC*
MsgC::Match(const RuleDictC& dict)
{
//
// Loop through patterns and see if any matches
//
   u_int	count = dict.size();
   int i=0; for (i=0; i<count; i++) {

      RegexC	*regPat = &dict[i]->key;
      StringC	*strPat = &dict[i]->key;

//
// Loop through the headers
//
      HeaderC	*head = body->headers;
      CharC	full;
      while ( head ) {

	 full = head->full;
	 if ( regPat->search(full) >= 0 ||
	      full.Contains(*strPat, IGNORE_CASE) ) {
	    StringC	*rule = &dict[i]->val;
	    return rule;
	 }

	 head = head->next;
      }

   } // End for each pattern

   return NULL;

} // End Match

/*---------------------------------------------------------------
 *  Method to build the attribution line for this message
 */

StringC
MsgC::Attribution()
{
   StringC	str = ishApp->replyPrefs->attribStr;
   if ( str.size() > 0 ) ReplaceHeaders(str);
   return str;
}

/*---------------------------------------------------------------
 *  Method to build the "begin forward" line for this message
 */

StringC
MsgC::BeginForward()
{
   StringC	str = ishApp->replyPrefs->beginForwardStr;
   if ( str.size() > 0 ) ReplaceHeaders(str);
   return str;
}

/*---------------------------------------------------------------
 *  Method to build the "end forward" line for this message
 */

StringC
MsgC::EndForward()
{
   StringC	str = ishApp->replyPrefs->endForwardStr;
   if ( str.size() > 0 ) ReplaceHeaders(str);
   return str;
}

#if 0
/*---------------------------------------------------------------
 *  Method to return the visible headers
 */

StringC
MsgC::DisplayedHeaders()
{
   HeaderC	*head = Headers();
   StringC	headStr;
   while ( head ) {

      if ( prefs->HeaderShown(head->key) ) {
	 headStr += head->full;
	 headStr += "\n";
      }

      head = head->next;
   }

   return headStr;

} // End DisplayedHeaders
#endif

