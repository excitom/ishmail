/*
 *  $Id: ImapMsgC.C,v 1.6 2000/12/31 14:36:02 evgeny Exp $
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
#include "ImapMsgC.h"
#include "ImapFolderC.h"
#include "ImapServerC.h"
#include "MsgPartC.h"
#include "IshAppC.h"
#include "MsgItemC.h"
#include "HeaderC.h"

#include <hgl/StringC.h>
#include <hgl/SysErr.h>

#include <errno.h>
#include <unistd.h>

/*---------------------------------------------------------------
 *  Constructor
 */

ImapMsgC::ImapMsgC(ImapFolderC *fld, ImapServerC *serv, int num) : MsgC(fld)
{
   type   = IMAP_MSG;

   server = serv;
   number = num;
   folder = fld;
   ScanHead();
}

/*---------------------------------------------------------------
 *  Destructor
 */

ImapMsgC::~ImapMsgC()
{
}

/*---------------------------------------------------------------
 *  Methods to return the header text
 */

Boolean
ImapMsgC::GetHeaderText(StringC& str) const
{
   char		*text;
   char		*flags;
   StringC	response;

   if ( !server->FetchHdrs(number, folder, &body->bytes, &text, &flags, response) )
         return False;

   delete flags;

//
// Remove the last blank line if present
//
   str += text;
   if ( str.EndsWith("\n\n") ) str.CutEnd(1);
   delete text;

   return True;

} // End GetHeaderText

Boolean
ImapMsgC::GetHeaderText(StringC& str)
{
   char		*text;
   char		*flags;
   StringC	response;

   if ( !server->FetchHdrs(number, folder, &body->bytes, &text, &flags, response) )
         return False;

   if ( flags ) ParseFlags(flags);
   delete flags;

//
// Remove the last blank line if present
//
   str += text;
   if ( str.EndsWith("\n\n") ) str.CutEnd(1);
   delete text;

   return True;

} // End GetHeaderText

/*---------------------------------------------------------------
 *  Methods to return the body text
 */

Boolean
ImapMsgC::GetBodyText(StringC& str) const
{
   char		*text;
   char		*flags;
   StringC	response;
   if ( !server->Fetch(number, folder, "RFC822.TEXT", &text, &flags, response) )
      return False;

   delete flags;

   str += text;
   delete text;

   return True;

} // End GetBodyText

Boolean
ImapMsgC::GetBodyText(StringC& str)
{
   char		*text = NULL;
   char		*flags = NULL;
   StringC	response;
   if ( !server->Fetch(number, folder, "RFC822.TEXT", &text, &flags, response) )
      return False;

//
// Remove closing parenthesis from the FETCH results
//
   // XXX - FIXME for some reason, text can be NULL here
   if (!text) {
       cerr << "ERROR!!  text is NULL at " << __FILE__ << ":" << __LINE__ <<
		   "\n";
       return False;
   }

   int pos = strlen(text) - 1;
   if (text[pos] == '\n' &&
       text[pos-1] == ')' &&
       text[pos-2] == '\n' )  text[pos-1] = '\0';

   if ( strlen(text) != body->bodyBytes ) {

      if ( debuglev > 0 && body->bodyBytes >= 0 )
	 cout <<"We thought body was " <<body->bodyBytes <<" bytes but we got "
	      <<strlen(text) <<" bytes" <<endl;

      CharC	textStr(text);
      body->bodyBytes = textStr.Length();
      body->bodyLines = textStr.NumberOf('\n');
      body->ComputeSize();
   }

   if ( flags ) ParseFlags(flags);
   delete flags;

   str += text;
   delete text;

   return True;

} // End GetBodyText

/*---------------------------------------------------------------
 *  Methods to return the body text for the specified part
 */

Boolean
ImapMsgC::GetPartText(const MsgPartC *part, StringC& str, Boolean getHead,
		      Boolean getExtHead, Boolean getBody) const
{
   char		*text  = NULL;
   char		*flags = NULL;
   StringC	response;
   StringC	cmd;

//
// Get the headers if necessary
//
   if ( getHead ) {

      cmd = "BODY[";
      cmd += part->partNum;
      cmd += ".0]";
      if ( !server->Fetch(number, folder, cmd, &text, &flags, response) )
	 return False;

      str += text;
      str += '\n';
      delete text;
      delete flags;
   }

   text  = NULL;
   flags = NULL;
   cmd = "BODY[";
   cmd += part->partNum;
   cmd += "]";
   if ( !server->Fetch(number, folder, cmd, &text, &flags, response) )
      return False;

//
// Remove closing parenthesis from the FETCH results
//
   int pos = strlen(text) - 1;
   if (text[pos] == '\n' &&
       text[pos-1] == ')' &&
       text[pos-2] == '\n' )  text[pos-1] = '\0';

//
// See if the external headers need to be stripped or extracted
//
   CharC	textStr = text;
   if ( part->IsExternal() && (!getExtHead || !getBody) ) {

      int	blankPos = textStr.PosOf("\n\n");
      if ( getExtHead ) {
	 if ( blankPos >= 0 ) textStr.CutEnd(textStr.Length()-blankPos+1);
      }
      else {	// getBody
	 if ( blankPos >= 0 ) textStr.CutBeg(blankPos+2);
      }
   }

   str += textStr;
   delete text;

   delete flags;

   return True;

} // End GetPartText

/*---------------------------------------------------------------
 *  Method to return the body text for the specified part that can
 *     be called by non-const objects
 */

Boolean
ImapMsgC::GetPartText(const MsgPartC *part, StringC& str, Boolean getHead,
		      Boolean getExtHead, Boolean getBody)
{
   char		*text  = NULL;
   char		*flags = NULL;
   StringC	response;
   StringC	cmd;

//
// Get the headers if necessary
//
   if ( getHead ) {

      cmd = "BODY[";
      cmd += part->partNum;
      cmd += ".0]";
      if ( !server->Fetch(number, folder, cmd, &text, &flags, response) )
	 return False;

      str += text;
      str += '\n';
      delete text;

      if ( flags ) ParseFlags(flags);
      delete flags;

   } // End if headers requested

   text  = NULL;
   flags = NULL;
   const StringC&	num = part->partNum;
   cmd = "BODY[";
   cmd += num;
   cmd += "]";
   if ( !server->Fetch(number, folder, cmd, &text, &flags, response) )
      return False;

//
// Double check body size
//
   CharC	bodyStr  = text;
   int		blankPos = bodyStr.PosOf("\n\n");
   if ( blankPos >= 0 ) bodyStr.CutBeg(blankPos+2);

   if ( bodyStr.Length() != part->bodyBytes ) {

      if ( debuglev > 0 && part->bodyBytes >= 0 )
	 cout <<"We thought part was " <<part->bodyBytes <<" bytes but we got "
	      <<bodyStr.Length() <<" bytes" <<endl;

      MsgPartC	*tmp = (MsgPartC*)part;
      tmp->bodyBytes = bodyStr.Length();
      tmp->bodyLines = bodyStr.NumberOf('\n');
      tmp->ComputeSize();
   }

//
// See if the external headers need to be stripped or extracted
//
   CharC	textStr = text;
   if ( part->IsExternal() && (!getExtHead || !getBody) ) {

      if ( getExtHead ) {
	 blankPos = textStr.PosOf("\n\n");
	 if ( blankPos >= 0 ) textStr.CutEnd(textStr.Length()-blankPos+1);
      }
      else {	// getBody
	 textStr = bodyStr;
      }
   }

   if ( flags ) ParseFlags(flags);
   delete flags;

   str += textStr;
   delete text;

   return True;

} // End GetPartText

/*---------------------------------------------------------------
 *  Methods to return the decoded contents of this body part
 */

Boolean
ImapMsgC::GetFileData(MsgPartC *part, StringC& text) const
{
   return part->GetData(text);
}

Boolean
ImapMsgC::GetFileData(MsgPartC *part, StringC& text)
{
   return part->GetData(text);
}

/*---------------------------------------------------------------
 *  Method to scan the file and determine the header sizes and body byte count
 */

void
ImapMsgC::ScanHead()
{
   StringC	headers;
   if ( !GetHeaderText(headers) ) return;

//
// Read the headers
//
   body->headLines = 0;
   body->headBytes = headers.size();

   StringC	headStr;
   u_int	offset = 0;
   int		nlPos = headers.PosOf('\n', offset);
   while ( nlPos >= 0 ) {

      body->headLines++;

      int	len = nlPos - offset + 1;

//
// Check for a continuation
//
      if ( isspace(headers[offset]) ) headStr += headers(offset, len);
      else {
	 if ( headStr.size() > 0 ) body->AddHeader(headStr);
	 headStr = headers(offset, len);
      }

      offset = nlPos+1;
      nlPos = headers.PosOf('\n', offset);

   } // End for each header line

//
// We don't have to read another line because the header text is guaranteed
//    to end with a newline
//

//
// See if we have a final header
//
   if ( headStr.size() > 0 ) body->AddHeader(headStr);

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
// If this is an external part, read the external headers
//
   if ( body->IsExternal() )
      GetExternalInfo(body);

//
// Get body size
//
   body->bodyBytes = body->bytes;
   if ( body->headBytes > 0 ) body->bodyBytes -= (body->headBytes + 1);
   if ( body->extBytes  > 0 ) body->bodyBytes -= (body->extBytes  + 1);

//
// See if we found a content-type header
//
   body->defConType = (body->conStr.size() == 0);
   if ( body->defConType ) {
      if ( body->parent && body->parent->IsDigest() )
	 body->SetType("message/rfc822");
      else
	 body->SetType("text/plain");
   }

//
// Don't mark plain text as MIME
//
   if ( IsMime() && body->IsPlainText() &&
        !body->IsEncoded() && !body->IsAttachment() && !body->IsExternal() )
      ClearStatus(MSG_MIME, False);

   ClearStatus(MSG_CHANGED, False);

   body->headScanned = True;

} // End ScanHead

/*---------------------------------------------------------------
 *  Method to scan the file and determine the body structure
 */

void
ImapMsgC::ScanBody()
{
   if ( debuglev > 1 ) cout <<"Scanning body in message " <<number <<endl;

   char		*text;
   char		*flags;
   StringC	response;

//
// If the top-level part is an external body, we already know the structure
//    We'll just count the number of lines in the body.
//
   if ( body->IsExternal() ) {

//
// Get body text
//
      if ( !server->Fetch(number, folder, "RFC822.TEXT", &text, &flags, response) )
	 return;

//
// Remove everything up to and including the first blank line
//
      CharC	bodyStr  = text;
      int	blankPos = bodyStr.PosOf("\n\n");
      if ( blankPos >= 0 ) bodyStr.CutBeg(blankPos+2);

      body->bodyLines = bodyStr.NumberOf('\n');

   } // End if body is external

//
// We need the structure
//
   else {

      if ( !server->Fetch(number, folder, "BODY", &text, &flags, response) )
	 return;

      if ( flags ) ParseFlags(flags);
      delete flags;

      body->bodyLines = 0;

      ScanPart(text, body);
      delete text;

      body->SetPartNumber("1");
      body->SetPartNumberSize(body->GetPartNumberMaxSize());

//
// Now we need to traverse the tree and get more information about any
//    external parts.  This has to be done after the part numbers are
//    calculated.
//
      GetExternalInfo(body);

   } // End if top-level part is not external

   body->bodyScanned = True;
   bodySizeKnown     = True;

} // End ScanBody

/*---------------------------------------------------------------
 *  Method to parse a body string
 *
 * body          : multipart-body or msg-body or other-body
 * multipart-body: (part) (part) ... (part) type
 * message-body  : type subtype (params) id {len} desc enc bytes (envelope) (body) lines
 * other-body    : type subtype (params) {len} id {len} desc enc bytes
 * params	 : (key {len} val) (key {len} val) ... (key {len} val)
 * envelope	 : {len} date {len} subj (addr:from) (addr:sender) (addr:reply-to) (addr:to) (addr:cc) (addr:bcc) {len} in-reply-to {len} message-id
 * addr		 : name route mailbox host
 *
 */

char*
ImapMsgC::ScanPart(char *data, MsgPartC *part)
{
//
// Skip initial whitespace
//
   char	*cp = data;
   while ( isspace(*cp) ) cp++;

   if ( *cp != '(' ) {
      cerr <<"Syntax error.  Expecting body part: "<<cp <<endl;
      return cp;
   }
   cp++;

//
// Scan to corresponding right paren
//
   StringC	headStr;
   Boolean	done = False;
   while ( !done && *cp ) {

      while ( isspace(*cp) ) cp++;

//
// See if this part is a multipart
//
      if ( *cp == '(' ) {

	 if ( !part->IsMultipart() ) part->SetType("multipart/mixed");

	 MsgPartC	*newPart = new MsgPartC(part);
	 newPart->parentMsg = this;
	 if ( !part->child ) {
	    if ( debuglev > 1 )
	       cout <<"Creating first child in multipart" <<endl;
	    part->child = newPart;
	 }
	 else {
	    if ( debuglev > 1 )
	       cout <<"Creating another child in multipart" <<endl;
	    MsgPartC	*lastPart = part->child;
	    while ( lastPart->next ) lastPart = lastPart->next;
	    lastPart->next = newPart;
	    newPart->prev = lastPart;
	 }
	 cp = ScanPart(cp, newPart);
      }

//
// See if this is the end of this part
//
      else if ( *cp == ')' ) {
	 done = True;
	 cp++;
      }

//
// If we're in a multipart, this must be the type
//
      else if ( part->IsMultipart() ) {
	 CharC	text;
	 cp = ScanText(cp, text);
	 StringC	type = "multipart/";
	 type += text;
	 part->SetType(type);
      }

//
// Scan the info for this part
//
      else {

	 CharC	group;
	 CharC	subtype;
	 cp = ScanText(cp, group);
	 cp = ScanText(cp, subtype);

	 StringC	type;
	 if ( group.Length() > 0 ) type = group;
	 else			   type = "text";
	 type += '/';
	 type.toLower();

	 if ( subtype.Length() > 0 )		 type += subtype;
	 else if ( type.Equals("text/") )	 type += "plain";
	 else if ( type.Equals("image/") )	 type += "gif";
	 else if ( type.Equals("audio/") )	 type += "basic";
	 else if ( type.Equals("video/") )	 type += "mpeg";
	 else if ( type.Equals("message/") )	 type += "rfc822";
	 else if ( type.Equals("application/") ) type += "octet-stream";
	 else					 type += "x-unknown";
	 type.toLower();

	 cp = ScanParams(cp, type);

	 part->SetType(type);

	 CharC	id;
	 cp = ScanText(cp, id);
	 if ( id.Length() > 0 ) {
	    headStr = "Content-Id: ";
	    headStr += id;
	    part->AddHeader(headStr);
	 }

	 CharC	desc;
	 cp = ScanText(cp, desc);
	 if ( desc.Length() > 0 ) {
	    headStr = "Content-Description: ";
	    headStr += desc;
	    part->AddHeader(headStr);
	 }

	 CharC	enc;
	 cp = ScanText(cp, enc);
	 if ( enc.Length() > 0 ) {
	    headStr = "Content-Transfer-Encoding: ";
	    headStr += enc;
	    part->AddHeader(headStr);
	 }

	 CharC	bytes;
	 cp = ScanNum(cp, bytes);
	 StringC	byteStr = bytes;
	 part->bodyBytes = atoi(byteStr);
	 if ( part->headBytes >= 0 )
	    part->bytes = part->bodyBytes + part->headBytes + 1;

//
// For encapsulated messages, we will scan the body ourself later.  For now,
//    create a temporary child to simplify scanning the input.
//
	 if ( part->Is822() ) {
	    cp = ScanEnvelope(cp, part);
	    if ( debuglev > 1 )
	       cout <<"Creating child in message/rfc822" <<endl;
	    MsgPartC	*tmpPart = new MsgPartC(part);
	    tmpPart->parentMsg = this;
	    cp = ScanPart(cp, tmpPart);
	    delete tmpPart;
	 }

	 while ( isspace(*cp) ) cp++;	// Skip whitespace

	 if ( isdigit(*cp) ) {		// See if line count is present
	    CharC	lines;
	    cp = ScanNum(cp, lines);
	    StringC	lineStr = lines;
	    part->bodyLines = atoi(lineStr);
	    if ( part->headLines >= 0 )
	       part->lines = part->bodyLines + part->headLines + 1;
	 }
      }

   } // End while not done

   return cp;

} // End ScanPart

/*---------------------------------------------------------------
 *  Method to parse a text string.  We will either have:
 *
 *  {len} text
 *  "text with possible whitespace"
 *  text-with-no-whitespace
 */

char*
ImapMsgC::ScanText(char *data, CharC& text)
{
//
// Skip initial whitespace
//
   char	*cp = data;
   while ( isspace(*cp) ) cp++;

//
// See if the length is specified
//
   int	len = 0;
   char	*start;
   if ( *cp == '{' ) {

      char	*lenp = ++cp;
      while ( *cp != '}' ) cp++;
      *cp = 0;		// Temporary
      len = atoi(lenp);
      *cp = '}';	// Restore
      cp++;		// Skip right brace

      start = cp;
      cp += len;	// Advance pointer
   }

//
// Look for the next non-escaped quote
//
   else if ( *cp == '"' ) {
      start = ++cp;
      Boolean	escape = False;
      while ( escape || *cp != '"' ) {
	 if ( *cp == '\\' && !escape ) escape = True;
	 else			       escape = False;
	 cp++;
      }
      len = cp - start;
      cp++;	// Advance pointer
   }

//
// Scan to the next whitespace or right paren character
//
   else {
      start = cp;
      while ( !isspace(*cp) && *cp != ')' ) cp++;
      len = cp - start;
   }

   text.Set(start, len);

   if ( debuglev > 2 ) cout <<"Found text [" <<text <<"]" <<endl;

   if ( text.Equals("NIL", IGNORE_CASE) ) text.Set(start, 0);

   return cp;

} // End ScanText

/*---------------------------------------------------------------
 *  Method to parse a number string.
 *
 */

char*
ImapMsgC::ScanNum(char *data, CharC& text)
{
//
// Skip initial whitespace
//
   char	*cp = data;
   while ( isspace(*cp) ) cp++;

//
// Scan to the next non-numeric character
//
   char	*start = cp;
   while ( isdigit(*cp) ) cp++;
   int	len = cp - start;

   text.Set(start, len);

   if ( debuglev > 2 ) cout <<"Found number [" <<text <<"]" <<endl;
   return cp;

} // End ScanNum

/*---------------------------------------------------------------
 *  Method to parse a list of MIME parameters.
 *
 */

char*
ImapMsgC::ScanParams(char *data, StringC& typeStr)
{
   if ( debuglev > 2 ) cout <<"Scanning parameters" <<endl;

//
// Skip initial whitespace
//
   char	*cp = data;
   while ( isspace(*cp) ) cp++;

   if ( *cp != '(' ) {

      CharC	nil(cp, 3);
      if ( nil.Equals("NIL", IGNORE_CASE) )
	 cp += 3;
      else
	 cerr <<"Syntax error.  Expecting parameter list: "<<cp <<endl;
      return cp;
   }
   cp++;

//
// Scan to corresponding right paren
//
   Boolean	done = False;
   while ( !done && *cp ) {

      while ( isspace(*cp) ) cp++;

//
// See if this is the end of this part
//
      if ( *cp == ')' ) {
	 done = True;
	 cp++;
      }

//
// Read the next parameter
//
      else {
	 CharC	key;
	 CharC	val;
	 cp = ScanText(cp, key);
	 cp = ScanText(cp, val);
	 if ( debuglev > 2 )
	    cout <<"Found param [" <<key <<"=" <<val <<"]" <<endl;
#if 0
	 if ( part->IsExternal() ) part->AddAccParam(key, val);
	 else			   part->AddConParam(key, val);
#else
	 typeStr += "; ";
	 typeStr += key;
	 typeStr += '=';
	 Boolean	addQuotes = !val.StartsWith('"');
	 if ( addQuotes ) typeStr += '"';
	 typeStr += val;
	 if ( addQuotes ) typeStr += '"';
#endif
      }

   } // End While not done

#if 0
   if ( part->IsLocal() )
      part->GetFileName(part->dataFile);
#endif

   return cp;

} // End ScanParams

/*---------------------------------------------------------------
 *  Method to parse an envelope string
 *
 * envelope	 : {len} date {len} subj (addr:from) (addr:sender) (addr:reply-to) (addr:to) (addr:cc) (addr:bcc) {len} in-reply-to {len} message-id
 */

char*
ImapMsgC::ScanEnvelope(char *data, MsgPartC *part)
{
   if ( debuglev > 2 ) cout <<"Scanning envelope" <<endl;

//
// Skip initial whitespace
//
   char	*cp = data;
   while ( isspace(*cp) ) cp++;

   if ( *cp != '(' ) {

      CharC	nil(cp, 3);
      if ( nil.Equals("NIL", IGNORE_CASE) )
	 cp += 3;
      else
	 cerr <<"Syntax error.  Expecting envelope: "<<cp <<endl;
      return cp;
   }
   cp++;

   CharC	date;
   cp = ScanText(cp, date);

   CharC	subj;
   cp = ScanText(cp, subj);
   if ( subj.Length() > 0 ) {
      StringC	subStr("Subject: ");
      subStr += subj;
      part->SetSubject(subStr);
   }

   cp = ScanAddressList(cp, part);	// from
   cp = ScanAddressList(cp, part);	// sender
   cp = ScanAddressList(cp, part);	// reply-to
   cp = ScanAddressList(cp, part);	// to
   cp = ScanAddressList(cp, part);	// cc
   cp = ScanAddressList(cp, part);	// bcc

   CharC	reply;
   cp = ScanText(cp, reply);	// In-Reply-To

   CharC	id;
   cp = ScanText(cp, id);	// Message-Id

//
// Look for closing right paren
//
   while ( *cp && *cp != ')' ) cp++;
   if ( *cp == ')' ) cp++;

   return cp;

} // End ScanEnvelope

/*---------------------------------------------------------------
 *  Method to parse an address list.  Either:
 *
 *  NIL
 *   or
 *  ((addr) (addr) ... (addr))
 */

char*
ImapMsgC::ScanAddressList(char *data, MsgPartC *part)
{
   if ( debuglev > 2 ) cout <<"Scanning address list" <<endl;

//
// Skip initial whitespace
//
   char	*cp = data;
   while ( isspace(*cp) ) cp++;

   if ( *cp != '(' ) {

      CharC	nil(cp, 3);
      if ( nil.Equals("NIL", IGNORE_CASE) )
	 cp += 3;
      else
	 cerr <<"Syntax error.  Expecting address list: "<<cp <<endl;
      return cp;
   }
   cp++;

//
// Scan to corresponding right paren
//
   Boolean	done = False;
   while ( !done && *cp ) {

      while ( isspace(*cp) ) cp++;

//
// See if this is another address
//
      if ( *cp == '(' ) {
	 cp = ScanAddress(cp, part);
      }

//
// See if this is the end of this part
//
      else if ( *cp == ')' ) {
	 done = True;
	 cp++;
      }
   }

   return cp;

} // End ScanAddressList

/*---------------------------------------------------------------
 *  Method to parse an address.
 *
 *  addr: name route mailbox host
 */

char*
ImapMsgC::ScanAddress(char *data, MsgPartC*)
{
   if ( debuglev > 2 ) cout <<"Scanning address" <<endl;

//
// Skip initial whitespace
//
   char	*cp = data;
   while ( isspace(*cp) ) cp++;

   if ( *cp != '(' ) {

      CharC	nil(cp, 3);
      if ( nil.Equals("NIL", IGNORE_CASE) )
	 cp += 3;
      else
	 cerr <<"Syntax error.  Expecting address: "<<cp <<endl;
      return cp;
   }
   cp++;

   CharC	tmp;
   cp = ScanText(cp, tmp);	// Name
   cp = ScanText(cp, tmp);	// Route
   cp = ScanText(cp, tmp);	// Mailbox
   cp = ScanText(cp, tmp);	// host

//
// Look for closing right paren
//
   while ( *cp && *cp != ')' ) cp++;
   if ( *cp == ')' ) cp++;

   return cp;

} // End ScanAddress

/*---------------------------------------------------------------
 *  Method to parse a flag string
 */

void
ImapMsgC::ParseFlags(CharC flagStr)
{
   if ( flagStr.Contains("\\seen", IGNORE_CASE) ) {
      ClearStatus(MSG_NEW, False);
      SetStatus(MSG_READ, False);
   }
   else if ( flagStr.Contains("\\recent", IGNORE_CASE) ) {
      ClearStatus(MSG_READ, False);
      SetStatus(MSG_NEW, False);
   }
   if ( flagStr.Contains("\\deleted", IGNORE_CASE) )
      SetStatus(MSG_DELETED, False);
   if ( flagStr.Contains("\\answered", IGNORE_CASE) )
      SetStatus(MSG_REPLIED, False);
   if ( flagStr.Contains("\\flagged", IGNORE_CASE) )
      SetStatus(MSG_FLAGGED, False);
   if ( flagStr.Contains("\\saved", IGNORE_CASE) )
      SetStatus(MSG_SAVED, False);
   if ( flagStr.Contains("\\forwarded", IGNORE_CASE) )
      SetStatus(MSG_FORWARDED, False);
   if ( flagStr.Contains("\\resent", IGNORE_CASE) )
      SetStatus(MSG_RESENT, False);
   if ( flagStr.Contains("\\printed", IGNORE_CASE) )
      SetStatus(MSG_PRINTED, False);
   if ( flagStr.Contains("\\filtered", IGNORE_CASE) )
      SetStatus(MSG_FILTERED, False);

} // End ParseFlags

/*------------------------------------------------------------------------
 * Method to get the external headers for a body part
 */

void
ImapMsgC::GetExternalInfo(MsgPartC *part)
{
//
// If this is a multipart, loop through children
//
   if ( part->IsMultipart() ) {

      MsgPartC	*child = part->child;
      while ( child ) {
	 GetExternalInfo(child);
	 child = child->next;
      }
   }

//
// If this is an external part, get the body text and read the external headers
//
   else if ( part->IsExternal() ) {

//
// Get body text
//
      char	*text;
      char	*flags;
      StringC	response;
      StringC	cmd = "BODY["; cmd += part->partNum; cmd += "]";
      if ( !server->Fetch(number, folder, cmd, &text, &flags, response) )
	 return;

//
// Remove everything after the first blank line
//
      StringC	headers  = text;
      int	blankPos = headers.PosOf("\n\n");
      if ( blankPos >= 0 ) headers.Clear(blankPos+1);
      if ( !headers.EndsWith('\n') ) headers += '\n';

      part->extLines = 0;
      part->extBytes = headers.size();

//
// Loop through external headers
//
      StringC	headStr;
      u_int	offset = 0;
      int	nlPos  = headers.PosOf('\n', offset);
      while ( nlPos >= 0 ) {

	 part->extLines++;

	 int	len = nlPos - offset + 1;

//
// Check for a continuation
//
	 if ( isspace(headers[offset]) ) headStr += headers(offset, len);
	 else {
	    if ( headStr.size() > 0 ) part->AddExtHeader(headStr);
	    headStr = headers(offset, len);
	 }

	 offset = nlPos+1;
	 nlPos = headers.PosOf('\n', offset);

      } // End for each header line

//
// See if we have a final header
//
      if ( headStr.size() > 0 ) part->AddExtHeader(headStr);

      delete text;
      delete flags;

   } // End if part is external

} // End GetExternalInfo

/*------------------------------------------------------------------------
 * Method to set status flags
 */

void
ImapMsgC::SetStatus(MsgStatusT val, Boolean write)
{
   if ( (status & (u_int)val) != 0 ) return;   // Already set

   status |= (u_int)val;

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

      if      ( val == MSG_READ   ) status &= ~MSG_NEW;
      else if ( val == MSG_NEW    ) status &= ~MSG_READ;

      if ( val == MSG_DELETED )
	 folder->MsgDeleted(this);
      else if ( val == MSG_NEW || val == MSG_READ )
	 folder->MsgStatusChanged(this);

      status |= (u_int) MSG_CHANGED;
      //folder->SetChanged(True);

//
// Only these flags need to be written
//
      write = write && (val & (MSG_NEW|MSG_READ|MSG_REPLIED|MSG_DELETED));
   }

//
// Update server if necessary
//
   if ( write )
      server->SetFlags(number, status);

//
// Update visible icon
//
   if ( icon ) icon->UpdateStatus();

} // End SetStatus

/*------------------------------------------------------------------------
 * Method to clear status flags
 */

void
ImapMsgC::ClearStatus(MsgStatusT val, Boolean write)
{
   if ( (status & (u_int)val) == 0 ) return;   // Already clear

   status &= ~(u_int)val;
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
// Only these flags need to be written
//
      write = write && (val & (MSG_NEW|MSG_READ|MSG_REPLIED|MSG_DELETED));
      if ( write )
	 server->SetFlags(number, status);
   }

//
// Update visible icon
//
   if ( icon ) icon->UpdateStatus();

} // End ClearStatus

/*------------------------------------------------------------------------
 * Method to write the body to a file
 */

Boolean
ImapMsgC::WriteBody(FILE *dstfp, Boolean addBlank, Boolean /*protectFroms*/)
{
   bodyWritePos = ftell(dstfp);

   StringC	textStr;
   if ( !GetBodyText(textStr) ) return False;

   Boolean	success = textStr.WriteFile(dstfp);

   if ( success && addBlank ) {
      CharC	nlStr("\n");
      success = nlStr.WriteFile(dstfp);
   }

   return success;
}
