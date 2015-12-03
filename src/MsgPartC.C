/*
 *  $Id: MsgPartC.C,v 1.3 2001/01/03 09:57:02 evgeny Exp $
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
#include "MsgPartC.h"
#include "ParamC.h"
#include "HeaderC.h"
#include "HeaderValC.h"
#include "Mailcap.h"
#include "QuotedP.h"
#include "Base64.h"
#include "MsgC.h"
#include "FileMsgC.h"
#include "MhMsgC.h"
#include "FolderC.h"
#include "UnixFolderC.h"
#include "MimeEncode.h"

#include <hgl/HalAppC.h>
#include <hgl/SysErr.h>
#include <hgl/PtrListC.h>
#include <hgl/StringListC.h>
#include <hgl/CharC.h>
#include <hgl/RegexC.h>

#include <unistd.h>     
#include <errno.h>     
#include <sys/stat.h>     

extern int	debuglev;

#define BUFLEN	1024

/*---------------------------------------------------------------
 *  MsgPartC constructor
 */

MsgPartC::MsgPartC(MsgPartC *parPart)
{
   Init();

//
// These variables cannot be reset
//
   parentMsg = NULL;
   parent    = parPart;
   prev	     = NULL;
   next	     = NULL;
}

/*---------------------------------------------------------------
 *  Method to initialize variables
 */

void
MsgPartC::Init()
{
   conType	= CT_UNKNOWN;
   grpType	= GT_UNKNOWN;
   accType	= AT_INLINE;
   encType	= ET_NONE;
   defConType   = True;

   conParams	= NULL;
   accParams	= NULL;
   disParams	= NULL;

   offset       = 0;
   bytes        = -1;
   lines        = -1;

   headBytes    = -1;
   headLines    = -1;

   extOffset    = -1;
   extBytes     = -1;
   extLines     = -1;
   extBlank     = 0;

   bodyOffset   = -1;
   bodyBytes    = -1;
   bodyLines    = -1;

   headers      = NULL;
   extHeaders	= NULL;
   subject	= NULL;

   headScanned  = False;
   bodyScanned  = False;

   msgFile	= "";
   dataFile	= "";

   delMsgFile   = False;
   delDataFile  = False;

   childMsg     = NULL;
   child        = NULL;

} // End Init

/*---------------------------------------------------------------
 *  Method to delete variables
 */

void
MsgPartC::Delete()
{
   while ( child ) {
      MsgPartC	*next = child->next;
      delete child;
      child = next;
   }

   delete childMsg;

   delete conParams;
   delete accParams;
   delete disParams;

   if ( delMsgFile  ) unlink(msgFile);
   if ( delDataFile ) unlink(dataFile);

   delete subject;
   delete headers;
   delete extHeaders;

} // End Delete

/*---------------------------------------------------------------
 *  Method to reset variables
 */

void
MsgPartC::Reset()
{
   Delete();
   Init();
}

/*---------------------------------------------------------------
 *  MsgPartC destructor
 */

MsgPartC::~MsgPartC()
{
   Delete();
}

/*---------------------------------------------------------------
 *  Method to print
 */

void
MsgPartC::Print(ostream& strm) const
{
   if ( debuglev == 0 ) return;

   StringC	headStr;
   if ( IsMultipart() ) {
      if ( conStr.size() > 0 ) strm <<conStr  <<" ";
   }
   else {
      if ( partNum.size() > 0 ) strm <<partNum <<" ";
      if ( conStr.size() > 0 ) strm <<conStr  <<" ";
      headStr.Clear();
      GetHeaderValue("Content-Disposition", headStr);
      if ( headStr.size() > 0 ) strm <<headStr     <<" ";
      strm <<EncodingTypeStr(encType) <<" ";
      strm <<"(" <<lines <<" lines, " <<bytes <<" bytes @ "
		 <<offset <<")" <<endl;
   }

   headStr.Clear();
   GetDescription(headStr);
   if ( headStr.size() > 0 ) strm <<"\t" <<headStr <<endl;

   if ( debuglev > 1 ) {
      ParamC	*param;
      for ( param=conParams; param; param=param->next )
	 strm <<"   " <<*param;
      for ( param=accParams; param; param=param->next )
	 strm <<"   " <<*param;
      for ( param=disParams; param; param=param->next )
	 strm <<"   " <<*param;
   }

} // End Print

/*---------------------------------------------------------------
 *  Methods to query headers
 */

HeaderC*
MsgPartC::Header(CharC key) const
{
//
// Loop through external headers
//
   HeaderC	*head = extHeaders;
   while ( head ) {

      if ( head->key.Equals(key, IGNORE_CASE) )
	 return head;

      head = head->next;
   }

//
// Loop through regular headers
//
   head = headers;
   while ( head ) {

      if ( head->key.Equals(key, IGNORE_CASE) )
	 return head;

      head = head->next;
   }

   return NULL;

} // End Header

HeaderValC*
MsgPartC::HeaderValue(CharC key) const
{
//
// Look up header
//
   HeaderC	*head = Header(key);
   if ( head ) return head->value;

   return NULL;
}

void
MsgPartC::GetHeaderValue(CharC key, StringC& val) const
{
//
// Look up header
//
   HeaderC	*head = Header(key);
   if ( head ) head->GetValueText(val);
}

/*---------------------------------------------------------------
 *  Method to return a pointer to the requested parameter
 */

ParamC*
MsgPartC::Param(CharC key) const
{
//
// Check type, access and disposition parameters
//
   ParamC	*param = Param(key, conParams);
   if ( !param ) param = Param(key, accParams);
   if ( !param ) param = Param(key, disParams);

   return param;

} // End Param

/*---------------------------------------------------------------
 *  Method to return a pointer to the requested parameter
 */

ParamC*
MsgPartC::Param(CharC key, ParamC *list) const
{
//
// Loop through list
//
   ParamC	*param;
   for ( param=list; param; param=param->next )
      if ( param->key.Equals(key, IGNORE_CASE) )
	 return param;

   return NULL;

} // End Param

/*---------------------------------------------------------------
 *  Methods to add a parameter
 */

void
MsgPartC::AddConParam(CharC key, CharC val)
{
   if ( !conParams )
      conParams = new ParamC(key, val);
   else
      AddParam(key, val, conParams);
}

void
MsgPartC::AddAccParam(CharC key, CharC val)
{
   if ( !accParams )
      accParams = new ParamC(key, val);
   else
      AddParam(key, val, accParams);
}

void
MsgPartC::AddDisParam(CharC key, CharC val)
{
   if ( !disParams )
      disParams = new ParamC(key, val);
   else
      AddParam(key, val, disParams);
}

void
MsgPartC::AddParam(CharC key, CharC val, ParamC *params)
{
//
// See if this parameter is already defined
//
   ParamC	*param = Param(key, params);

//
// If it is, check the value.  Change it if necessary
//
   if ( param ) {
      if ( !param->val.Equals(val, IGNORE_CASE) )
	 param->SetValue(val);
   }

//
// Create a new parameter at the end of the list
//
   else {
      param = params;
      while ( param->next ) param = param->next;
      param->next = new ParamC(key, val);
   }

} // End AddParam

/*---------------------------------------------------------------
 *  Method to calculate part number
 */

void
MsgPartC::SetPartNumber(char *num)
{
   if ( IsMultipart() ) {

//
// See how many children we have
//
      int	partCount = ChildCount();
      if ( partCount == 1 )
	 child->SetPartNumber(num);

      else {

	 int		index = 1;
	 StringC	partStr;
	 MsgPartC	*part = child;
	 while ( part ) {

	    if ( parent ) {
	       partStr = num;
	       partStr += ".";
	    }
	    else
	       partStr.Clear();

	    partStr += index;
	    part->SetPartNumber(partStr);
	    index++;

	    part = part->next;
	 }
      }

   } // End if this is a multipart

   else {

      partNum = num;

//
// message/rfc822 can have a single child
//
      if ( child ) {
	 StringC	partStr = num;
	 if ( !child->IsMultipart() ) partStr += ".1";
	 child->SetPartNumber(partStr);
      }
   }

} // End SetPartNumber

/*---------------------------------------------------------------
 *  Method to calculate the maximum length of all part numbers
 */

int
MsgPartC::GetPartNumberMaxSize()
{
   if ( IsMultipart() || (Is822() && child) ) {

//
// Loop through children
//
      int	maxSize = 0;
      if ( !IsMultipart() ) maxSize = partNum.size();

      MsgPartC	*part;
      for (part=child; part; part=part->next) {
	 int	size = part->GetPartNumberMaxSize();
	 if ( size > maxSize ) maxSize = size;
      }

      return maxSize;
   }

   return partNum.size(); 

} // End GetPartNumberMaxSize

/*---------------------------------------------------------------
 *  Method to set the length of all part numbers
 */

void
MsgPartC::SetPartNumberSize(int size)
{
   if ( !IsMultipart() ) {
#if 0
      int	pad = size - partNum.size();
      int i=0; for (i=0; i<pad; i++) partNum += ' ';
#endif
      if ( child ) child->SetPartNumberSize(size);
   }

   else {
//
// Loop through children
//
      MsgPartC	*part;
      for (part=child; part; part=part->next)
	 part->SetPartNumberSize(size);
   }

} // End SetPartNumberSize

/*---------------------------------------------------------------
 *  Method to find a part matching the specified part number
 */

MsgPartC*
MsgPartC::FindPart(CharC num)
{
   if ( !IsMultipart() ) {
      CharC	tmp = partNum;
      tmp.Trim();
      if ( tmp == num ) return this;

      if ( child ) return child->FindPart(num);
   }

   else {
//
// Loop through children
//
      MsgPartC	*part;
      for (part=child; part; part=part->next) {
	 MsgPartC	*found = part->FindPart(num);
	 if ( found ) return found;
      }
   }

   return NULL;

} // End FindPart

/*---------------------------------------------------------------
 *  Method to update the part offset
 */

void
MsgPartC::Move(long headDelta, long bodyDelta)
{
   offset     = (u_int)((long)offset     + headDelta);
   bodyOffset = (u_int)((long)bodyOffset + bodyDelta);
   if ( IsExternal() ) extOffset = (u_int)((long)extOffset + bodyDelta);
   int	newHeadBytes = bodyOffset - offset - 1;
   if ( newHeadBytes > headBytes ) {
      headLines++;	// Assume a Status line was added
      headBytes = newHeadBytes;
   }

//
// Loop through children
//
   MsgPartC	*part;
   for (part=child; part; part=part->next)
      part->Move(headDelta, bodyDelta);
}

/*-----------------------------------------------------------------------
 * Method to return best alternative.  The best alternative for printing
 *    may be different than the best alternative for display.
 */

MsgPartC*
MsgPartC::BestAlternative(Boolean forPrinting) const
{
   if ( !IsAlternative() || !child ) return NULL;

   MsgPartC	*lastChild = NULL;
   MsgPartC	*cp        = child;

//
// Loop through children
//
   Boolean	done = False;
   while ( !done && cp ) {

//
// Check the recognized types
//
      if ( forPrinting ) {

//
// Can't print most external bodies
//
	 if ( cp->IsExternal() && !cp->IsLocal() )
	    done = True;

	 else if ( cp->IsOctet() ||
		   cp->IsJPEG()  ||
		   cp->IsGIF()   ||
		   cp->IsAudio() ||
		   cp->IsMPEG() )
	    done = True;

//
// If this is an unknown type, see if there's a mailcap entry
//
	 else if ( cp->IsUnknown() ) {
	    MailcapC	*mcap = MailcapEntry(cp);
	    done = (!mcap || mcap->print.size()==0);
	 }

      } // End if for printing

      else if ( cp->IsPlainText()	||
		cp->IsRichText()	||
		cp->IsEnriched()	||
		cp->IsMixed()		||
		cp->IsAlternative()	||
		cp->IsDigest()		||
		// cp->IsParallel()	||
	        cp->Is822()		||
	        // cp->IsPartial()	||
		cp->IsOctet()		||
		cp->IsPostScript()	||
		cp->IsJPEG()		||
		cp->IsGIF()		||
		cp->IsAudio()		||
		cp->IsMPEG() )
	 done = False;

//
// If this is an unknown type, see if there's a mailcap entry
//
      else if ( cp->conType == CT_UNKNOWN ) {
	 MailcapC	*mcap = MailcapEntry(cp);
	 done = (!mcap || mcap->present.size()==0);
      }

      else
	 done = True;

//
// Mark this one if it was ok
//
      if ( !done ) {
	 lastChild = cp;
	 cp = cp->next;
      }

   } // End for each child

//
// If there are no known types, use the last one
//
   if ( !lastChild ) {
      lastChild = child;
      while ( lastChild->next ) lastChild = lastChild->next;
   }

   return lastChild;

} // End BestAlternative


/*-----------------------------------------------------------------------
 * Method to return best text alternative for displaying inline.
 */

MsgPartC*
MsgPartC::BestTextAlternative() const
{
   if ( !IsAlternative() || !child ) return NULL;

   MsgPartC	*lastChild = NULL;
   MsgPartC	*cp        = child;

// Loop through children

   Boolean	done = False;
   while ( !done && cp ) {

// Check the recognized types

      if ( cp->IsInlineText() &&
	 (cp->IsPlainText() || cp->IsRichText() || cp->IsEnriched()) ) {

	 lastChild = cp;
	 cp = cp->next;

      } else {

	 done = True;
      }

   } // End for each child

// If there are no known types, use the first one

   if (!lastChild ) lastChild = child;

   return lastChild;

} // End BestTextAlternative

/*-----------------------------------------------------------------------
 *  Method to return a good label for the body part that can be called by
 *     a const object.
 */

void
MsgPartC::GetLabel(StringC& str) const
{
#if 0
   if ( partNum.size() > 0 ) {
      str = partNum;
      str += ' ';
   }
   else
#endif
      str.Clear();

//
// Try the description
//
   StringC	desc;
   GetHeaderValue("Content-Description", desc);
   if ( desc.size() > 0 && !desc.Equals("default", IGNORE_CASE) ) {
      str += desc;
      return;
   }

//
// Try the name parameter
//
   ParamC	*param = Param(NAME_S, accParams);
   if ( param ) {
      str += param->val;
      return;
   }

   param = Param(NAME_S, conParams);
   if ( param ) {
      str += param->val;
      return;
   }

//
// If this is a message/rfc822 type, look for a subject
//
   if ( Is822() ) {

      if ( subject ) {
	 subject->GetValueText(str);
	 return;
      }

      else if ( childMsg ) {
	 StringC	sub;
	 childMsg->GetSubjectText(sub);
	 str += sub;
	 return;
      }
   }

//
// Try the content-disposition header
//
   param = Param(FILENAME_S);
   if ( param ) {
      str += param->val;
      return;
   }

   param = Param(NAME_S, disParams);
   if ( param ) {
      str += param->val;
      return;
   }

//
// If the name is still blank, use the type.
//
   str += conStr;

   return;

} // End GetLabel

/*-----------------------------------------------------------------------
 *  Method to return a good label for the body part that can be called by
 *     a non-const object.
 */

void
MsgPartC::GetLabel(StringC& str)
{
#if 0
   if ( partNum.size() > 0 ) {
      str = partNum;
      str += ' ';
   }
   else
#endif
      str.Clear();

//
// Try the description
//
   StringC	desc;
   GetHeaderValue("Content-Description", desc);
   if ( desc.size() > 0 && !desc.Equals("default", IGNORE_CASE) ) {
      str += desc;
      return;
   }

//
// Try the name parameter
//
   ParamC	*param = Param(NAME_S, accParams);
   if ( param ) {
      str += param->val;
      return;
   }

   param = Param(NAME_S, conParams);
   if ( param ) {
      str += param->val;
      return;
   }

//
// If this is a message/rfc822 type, look for a subject
//
   if ( Is822() ) {

      if ( subject )
	 subject->GetValueText(str);

      else {
	 FileMsgC	*msg = ChildMsg();
	 StringC	sub;
	 msg->GetSubjectText(sub);
	 str += sub;
      }

      return;
   }

//
// Try the content-disposition header
//
   param = Param(FILENAME_S);
   if ( param ) {
      str += param->val;
      return;
   }

   param = Param(NAME_S, disParams);
   if ( param ) {
      str += param->val;
      return;
   }

//
// If the name is still blank, use the type.
//
   str += conStr;

   return;

} // End GetLabel

/*-----------------------------------------------------------------------
 *  Method to return a file name for this part
 */

void
MsgPartC::GetFileName(StringC& file) const
{
   if ( !IsMail() && dataFile.size() > 0 ) {
      file += dataFile;
      return;
   }

//
// Try the name parameter
//
   ParamC	*param = Param(NAME_S, accParams);
   if ( param ) {
      file += param->val;
      return;
   }

   param = Param(NAME_S, conParams);
   if ( param ) {
      file += param->val;
      return;
   }

//
// Try the content-disposition header
//
   param = Param(FILENAME_S);
   if ( param ) {
      file += param->val;
      return;
   }

   param = Param(NAME_S, disParams);
   if ( param ) {
      file += param->val;
      return;
   }

//
// Try the description
//
   StringC	desc;
   GetDescription(desc);
   if ( !desc.Equals("default", IGNORE_CASE) ) {
      file += desc;
      return;
   }

//
// If the name is still blank, use the message file.
//
   if ( msgFile.size() > 0 ) {
      file += msgFile;
      return;
   }

//
// If the name is still blank, punt.
//
   file += "Unknown";

   return;

} // End GetFileName

/*-----------------------------------------------------------------------
 *  Method to return value of content-description header
 */

void
MsgPartC::GetDescription(StringC& str) const
{
   HeaderValC	*val = HeaderValue("Content-Description");
   if ( val ) val->GetValueText(str);
}

/*-----------------------------------------------------------------------
 *  Method to return value of content-disposition header
 */

void
MsgPartC::GetDisposition(StringC& str) const
{
   HeaderValC	*val = HeaderValue("Content-Disposition");
   if ( val ) val->GetValueText(str);
}

/*-----------------------------------------------------------------------
 *  Method to return whether the content-disposition headers says "attachment"
 */

Boolean
MsgPartC::IsAttachment() const
{
   StringC	disp;
   GetHeaderValue("Content-Disposition", disp);

   return disp.StartsWith("attachment", IGNORE_CASE);
}

/*---------------------------------------------------------------
 *  Method to return the text as it appears in the mail message.
 */

Boolean
MsgPartC::GetText(StringC& text, FILE *fp, Boolean getHead, Boolean getExtHead,
		  Boolean getBody, Boolean restoreFroms) const
{
   if ( getHead && getBody ) getExtHead = True;
   else if ( !IsExternal() ) getExtHead = False;
   if ( !getBody ) restoreFroms = False;

   if ( msgFile.size() == 0 ) {
      if ( parentMsg && parentMsg->IsImap() )
	 return parentMsg->GetPartText(this, text, getHead, getExtHead,
				       getBody);
      else
	 return False;
   }

//
// Figure out how much to get
//
   int	off = 0;
   int	len = 0;
   if ( getHead && getBody ) {	// Assume getExtHead
      off = offset;
      len = bytes;
   }
   else if ( getHead && getExtHead ) {
      off = offset;
      len = headBytes+1+extBytes;
   }
   else if ( getExtHead && getBody ) {
      off = extOffset;
      len = extBytes+extBlank+bodyBytes;
   }
   else if ( getHead ) {
      off = offset;
      len = headBytes;
   }
   else if ( getExtHead ) {
      off = extOffset;
      len = extBytes;
   }
   else if ( getBody ) {
      off = bodyOffset;
      len = bodyBytes;
   }

//
// Perform the get
//
   return GetText(text, fp, off, len, restoreFroms);

} // End GetText

/*-------------------------------------------------------------------------
 * Return the text for the specified offset and length
 */

Boolean
MsgPartC::GetText(StringC& text, FILE *ifp, int off, int len,
		  Boolean restoreFroms) const
{
   Boolean	closeInput = (ifp == NULL);
   if ( !ifp ) {
      ifp = fopen(msgFile, "r");
      if ( !ifp ) {
	 StringC	errmsg("Could not open file: \"");
	 errmsg += msgFile;
	 errmsg += "\".\n";
	 errmsg += SystemErrorMessage(errno);
	 halApp->PopupMessage(errmsg);
	 return False;
      }
   }

   fseek(ifp, off, SEEK_SET);

   Boolean	error = False;
   if ( restoreFroms ) {

      RegexC	*fromPat = UnixFolderC::fromPat;
      int	remaining = len;
      char	buffer[BUFLEN];
      buffer[0] = 0;

      while ( remaining > 0 ) {

//
// Read a line from the source file
//
	 if ( !fgets(buffer, BUFLEN-1, ifp) ) {
	    StringC	errmsg("Could not read file: \"");
	    errmsg += msgFile;
	    errmsg += "\".\n";
	    errmsg += SystemErrorMessage(errno);
	    halApp->PopupMessage(errmsg);
	    if ( closeInput ) fclose(ifp);
	    return False;
	 }

//
// See if it needs restoring
//
	 if ( buffer[0] == '>' ) {
	    CharC	bufStr = buffer;
	    if ( bufStr.StartsWith(">From ") &&
		 (!fromPat || fromPat->match(bufStr, 1)) ) {
	       bufStr.CutBeg(1);
	    }
	    text += bufStr;
	 }
	 else
	    text += buffer;

	 remaining -= strlen(buffer);

      } // End while bytes remain to be copied

   } // End if Froms may need restoring

   else
      error = !text.AppendFile(ifp, len);

   if ( closeInput ) fclose(ifp);

   return !error;

} // End GetText

/*-----------------------------------------------------------------------
 * Method to return the contents of the data file associated with this part.
 */

Boolean
MsgPartC::GetData(StringC& text, FILE *fp)
{
//
// If it's external, read the part file.  It is an error if this file does
//    not already exist.
//
   if ( IsExternal() ) {

      if ( dataFile.size() == 0 ) return GetText(text, fp);

      if ( !text.AppendFile(dataFile) ) {

	 StringC	lab;
	 GetLabel(lab);

	 StringC	errmsg("Could not read file: ");
	 errmsg += dataFile;
	 errmsg += "\nfor part \"";
	 errmsg += lab;
	 errmsg += "\"\n";
	 errmsg += SystemErrorMessage(errno);
	 halApp->PopupMessage(errmsg);

	 return False;
      }

      return True;

   } // End if part is external

//
// If it's encoded, decode it and read the resulting file.
//
   if ( IsEncoded() ) {

      if ( dataFile.size() == 0 && !Decode(fp) )
	 return GetText(text, fp);

      if ( !text.AppendFile(dataFile) ) {

	 StringC	lab;
	 GetLabel(lab);

	 StringC	errmsg("Could not read decoded body file: ");
	 errmsg += dataFile;
	 errmsg += "\nfor part \"";
	 errmsg += lab;
	 errmsg += "\"\n";
	 errmsg += SystemErrorMessage(errno);
	 halApp->PopupMessage(errmsg);

	 return False;
      }

      return True;

   } // End if part is encoded

//
// If it's not encoded or external, return the body text
//
   return GetText(text, fp, False, False, True,
   			parentMsg && parentMsg->HasSafeFroms());

} // End GetData

/*-----------------------------------------------------------------------
 * Method to decode this body part to a file
 */

Boolean
MsgPartC::Decode(FILE *fp)
{
   if ( !IsEncoded() || dataFile.size() > 0 ) return True;

//
// Create an output file
//
   char	*ofile = tempnam(NULL, "dec.");
   dataFile    = ofile;
   if ( conType == CT_HTML ) {
      // Some brain-damaged MUAs generating HTML mails omit the encapsulating
      // <HTML></HTML> tags so we'll have to tell the browser to display this
      // as a valid HTML doc via the proper file extension.
      dataFile += ".html";
   }
   delDataFile = True;
   free(ofile);

//
// If there's no message file, we have to get the text.
//
   Boolean	error = False;
   if ( msgFile.size() == 0 ) {

      StringC	text;
      if ( bodyBytes > 0 ) text.reSize(bodyBytes);

      GetText(text, /*fp*/NULL, /*getHead*/False, /*getExtHead*/False,
			        /*getBody*/True, /*restoreFroms*/False);

      if ( IsBase64() ) {
	 MailcapC	*mcap = MailcapEntry(this);
	 error = !Text64ToFile(text, dataFile, mcap ? mcap->isText : False,
			       NULL);
      }

      else if ( IsQuoted() ) {
	 error = !TextQPToFile(text, dataFile, NULL);
      }
      else if ( IsUUencoded() ) {
	 error = !TextUUToFile(text, dataFile);
      }
      // If none of the above, silently ignore the encoding
      // and hope for the best.
   }

//
// Get the decoded text
//
   else {
      if ( IsBase64() ) {
	 MailcapC	*mcap = MailcapEntry(this);
	 error = !File64ToFile(msgFile, dataFile,
			       mcap ? mcap->isText : False,
			       fp, NULL, bodyOffset, bodyBytes);
      }
      else
	 error = !FileQPToFile(msgFile, dataFile,
			       fp, NULL, bodyOffset, bodyBytes);
   }

   return !error;

} // End Decode

/*---------------------------------------------------------------
 *  Method to read and process the part headers
 */

Boolean
MsgPartC::ScanHead(FILE *fp)
{
   if ( headScanned ) return True;

   Boolean	closeFile = (fp == NULL);
   if ( !fp ) {
      fp = fopen(msgFile, "r");
      if ( !fp ) {
	 StringC	errmsg("Could not open file: \"");
	 errmsg += msgFile;
	 errmsg += "\".\n";
	 errmsg += SystemErrorMessage(errno);
	 halApp->PopupMessage(errmsg);
	 return False;
      }
   }

//
// Move to start of headers
//
   fseek(fp, offset, SEEK_SET);

   headLines = 0;
   headBytes = 0;
   if ( headers != NULL ) {
	delete headers;
	headers = NULL;
   }

   char		line[BUFLEN];
   StringC	headStr;
   char		*getOK = fgets(line, BUFLEN-1, fp);
   Boolean	done   = strlen(line) <= 1;
   while ( getOK && !done ) {

      if ( debuglev > 2 ) cout <<"--> " <<line <<flush;

      headLines++;
      headBytes += strlen(line);

//
// Check for a continuation
//
      if ( isspace(line[0]) ) headStr += line;
      else {
	 if ( headStr.size() > 0 ) {

	   //
	   // KLUGE ALERT: remove spurrious ";;" in header, if any
	   //
	   int tmpSize = headStr.size();
	   headStr.Replace(";;",";");
	   headBytes -= headStr.size() - tmpSize;

	   AddHeader(headStr);
	 }
	 headStr = line;
      }

      getOK = fgets(line, BUFLEN-1, fp);
      done  = strlen(line) <= 1;

   } // End for each header line

   if ( debuglev > 2 && getOK ) cout <<"--> " <<line <<flush;

//
// See if we have a final header
//
   if ( headStr.size() > 0 ) AddHeader(headStr);

//
// If this is an external part, look for another set of headers
//
   if ( IsExternal() ) {

//
// Mark start of external headers
//
      extOffset = (int)ftell(fp);
      extLines  = 0;
      extBytes  = 0;
      extBlank  = 0;
      headStr.Clear();

//
// Look for more headers
//
      getOK = fgets(line, BUFLEN-1, fp);
      done  = strlen(line) <= 1;
      while ( getOK && !done ) {

	 if ( debuglev > 2 ) cout <<"--> " <<line <<flush;

	 extLines++;
	 extBytes += strlen(line);

//
// Check for a continuation
//
	 if ( isspace(line[0]) ) headStr += line;
	 else {
	    if ( headStr.size() > 0 ) AddExtHeader(headStr);
	    headStr = line;
	 }

	 getOK = fgets(line, BUFLEN-1, fp);
	 done  = strlen(line) <= 1;

      } // End for each header line

      if ( debuglev > 2 && getOK ) cout <<"--> " <<line <<flush;
      if ( getOK ) extBlank = 1;	// Got blank line

//
// See if we have a final header
//
      if ( headStr.size() > 0 ) AddExtHeader(headStr);

   } // End if part could have external headers

//
// Mark start of body
//
   bodyOffset = (int)ftell(fp);

//
// See if we found a content-type header
//
   defConType = (conStr.size() == 0);
   if ( defConType ) {
      if ( parent && parent->IsDigest() )
	 SetType("message/rfc822");
      else
	 SetType("text/plain");
   }

//
// If this is an rfc822 part, look for a subject header
//
   if ( Is822() ) {

      headStr.Clear();

//
// Look for more headers
//
      getOK = fgets(line, BUFLEN-1, fp);
      done  = strlen(line) <= 1;
      while ( getOK && !done ) {

//
// Check for a continuation
//
	 if ( isspace(line[0]) ) headStr += line;
	 else {
	    if ( headStr.size() > 0 &&
	       	 headStr.StartsWith("Subject:", IGNORE_CASE) )
	       SetSubject(headStr);
	    headStr = line;
	 }

	 getOK = fgets(line, BUFLEN-1, fp);
	 done  = strlen(line) <= 1;

      } // End for each header line

//
// See if we have a final header
//
      if ( headStr.StartsWith("Subject:", IGNORE_CASE) )
	 SetSubject(headStr);

//
// Move back to start of body
//
      fseek(fp, bodyOffset, SEEK_SET);

   } // End if looking for subject header

   if ( closeFile ) fclose(fp);

   headScanned = True;
   return True;

} // End ScanHead

/*-------------------------------------------------------------------------
 * Add a header to the list
 */

void
MsgPartC::AddHeader(CharC headStr)
{
   HeaderC	*newHead = new HeaderC(headStr);

//
// Add header at end of list
//
   if ( !headers )
      headers = newHead;
   else {
      HeaderC	*last = headers;
      while ( last->next ) last = last->next;
      last->next = newHead;
   }

   CheckHead(newHead);

} // End AddHeader

/*-------------------------------------------------------------------------
 * Add an external header to the list
 */

void
MsgPartC::AddExtHeader(CharC headStr)
{
   HeaderC	*newHead = new HeaderC(headStr);

//
// Add header at end of list
//
   if ( !extHeaders )
      extHeaders = newHead;
   else {
      HeaderC	*last = extHeaders;
      while ( last->next ) last = last->next;
      last->next = newHead;
   }

   CheckHead(newHead);

} // End AddExtHeader

/*---------------------------------------------------------------
 *  Method to look at a header for special info
 */

void
MsgPartC::CheckHead(HeaderC *head)
{
   CharC	key = head->key;
   if ( key.StartsWith("content-", IGNORE_CASE) )
      key.CutBeg(strlen("content-"));

   StringC	val;
   if ( key.Equals("type", IGNORE_CASE) ) {
      head->GetValueText(val);
      SetType(val);
   }

   else if ( key.Equals("disposition", IGNORE_CASE) ) {
      head->GetValueText(val);
      SetDisposition(val);
   }

   else if ( key.Equals("transfer-encoding", IGNORE_CASE) ) {
      head->GetValueText(val);
      SetEncoding(val);
   }

} // End CheckHead

/*---------------------------------------------------------------
 *  Method to set type
 */

void
MsgPartC::SetType(CharC str)
{
   conType = CT_UNKNOWN;
   grpType = GT_UNKNOWN;

   conStr.Clear();
   grpStr.Clear();
   subStr.Clear();

   delete conParams;
   conParams = NULL;

//
// Separate parameters
//
   CharC	paramStr;
   int		pos = str.PosOf(';');
   if ( pos > 0 ) {

      conStr = str(0,pos);
      conStr.Trim();

      paramStr = str(pos+1, str.Length());
      paramStr.Trim();
   }
   else
      conStr = str;

   conStr.toLower();

//
// Build list of parameters
//
   if ( paramStr.Length() > 0 ) conParams = new ParamC(paramStr);

//
// Separate group and subtype
//
   pos = conStr.PosOf('/');
   if ( pos > 0 ) {
      grpStr = conStr(0,     pos);
      subStr = conStr(pos+1, conStr.size());
   }
   else
      grpStr = conStr;

   conType = ContentType(conStr);
   grpType = GroupType(grpStr);

//
// If external message, look for access type
//
   if ( conType == CT_EXTERNAL ) {	// Can't use IsExternal() yet

      ParamC	*ap = Param(ACCESS_TYPE_S);
      if ( ap ) accType = AccessType(ap->val);
      else	accType = AT_LOCAL_FILE;

      delete accParams;
      accParams = conParams;
      conParams = NULL;

      conType = CT_UNKNOWN;
      grpType = GT_UNKNOWN;

      conStr.Clear();
      grpStr.Clear();
      subStr.Clear();

//
// Point to the local file if necessary
//
      if ( IsLocal() )
	 GetFileName(dataFile);

   } // End if external message

} // End SetType

/*---------------------------------------------------------------
 *  Method to set disposition
 */

void
MsgPartC::SetDisposition(CharC str)
{
//
// Separate parameters
//
   delete disParams;
   disParams = NULL;

   CharC	paramStr;
   CharC	dispStr;
   int		pos = str.PosOf(';');
   if ( pos > 0 ) {
      dispStr = str(0, pos);
      dispStr.Trim();

      paramStr = str(pos+1, str.Length());
      paramStr.Trim();
   }
   else {
      dispStr = str;
   }

//
// Build list of parameters
//
   if ( paramStr.Length() > 0 ) disParams = new ParamC(paramStr);

} // End SetDisposition

/*---------------------------------------------------------------
 *  Method to set encoding
 */

void
MsgPartC::SetEncoding(CharC str)
{
   encType = EncodingType(str);
}

/*---------------------------------------------------------------
 *  Method to set subject
 */

void
MsgPartC::SetSubject(CharC str)
{
   if ( subject ) delete subject;
   subject = new HeaderC(str);
}

/*---------------------------------------------------------------
 *  Method to scan this part from the offset to the next terminating line
 *     or EOF
 */

Boolean
MsgPartC::Scan(FILE *fp, int maxBytes, TermFn *terminatingLine,
	       char *nextBound, Boolean *gotLastBound, Boolean *prevLineBlank)
{
   if ( bodyScanned ) return True;

   Boolean	closeFile = (fp == NULL);
   if ( !fp ) {
      fp = fopen(msgFile, "r");
      if ( !fp ) {
	 StringC	errmsg("Could not open file: \"");
	 errmsg += msgFile;
	 errmsg += "\".\n";
	 errmsg += SystemErrorMessage(errno);
	 halApp->PopupMessage(errmsg);
	 return False;
      }
   }

//
// Scan the part headers
//
   if ( !ScanHead(fp) ) return False;

//
// Get the boundary
//
   StringC	boundStr;
   if ( IsMultipart() ) {

      ParamC	*curBound = Param(BOUNDARY_S);

      if ( !curBound ) {
	 StringC errmsg("Malformed MIME message.\n");
	 errmsg += "Missing boundary parameter";
	 HeaderC	*head = Header("Content-Description");
	 if ( head ) {
	    errmsg += " for part\n";
	    head->GetValueText(errmsg);
	 }

	 halApp->PopupMessage(errmsg);
	 SetType("text/plain");
      }

      boundStr = "--";
      if (curBound)
         boundStr += curBound->val;
      if ( debuglev > 2 ) cout <<"Multipart with boundary: " <<boundStr <<endl;

   } // End if this is a multipart

//
// Loop until we hit the max byte count or a terminating line.  Only check
//    for a terminating line if maxBytes < 0
//
   fseek(fp, bodyOffset, SEEK_SET);

   bodyLines = 0;
   bodyBytes = 0;

   if ( gotLastBound  ) *gotLastBound = False;

   char		line[BUFLEN];
   CharC	lineStr;
   Boolean	gotBound   = False;
   Boolean	prevBlank  = True;	// line between headers and body
   Boolean	checkBlank = True;
   Boolean	done       = False;
   char		*getOK     = fgets(line, BUFLEN-1, fp);
   while ( getOK && !done ) {

      if ( debuglev > 2 ) cout <<"--> " <<line <<flush;
      lineStr = line;

//
// Check for the end of this part
//
      if ( nextBound ) {

	 if ( lineStr.StartsWith(nextBound) ) {

	    lineStr.CutBeg(strlen(nextBound));

	    if ( lineStr == "--\n" ) {
	       lineStr.CutBeg(2);
	       if ( gotLastBound ) *gotLastBound = True;
	    }

	    if ( lineStr == "\n" ) done = True;
	    else		   lineStr = line;

//
// If we got a boundary, account for the fact the the newline we counted for
//    the previous line was really part of this boundary.
//
	    if ( done ) {

	       gotBound = True;

	       if      ( bodyBytes > 0 ) bodyBytes--;
	       else if ( extBlank  > 0 ) extBlank--;	// Lost blank line
		  					// after external head

	       if ( prevBlank && bodyLines > 0 ) bodyLines--;

	    } // End if we found the right boundary

	    else if ( maxBytes >= 0 )
	       done = (bodyBytes >= maxBytes);

	 } // End if got a boundary

      } // End if looking for a boundary

      else if ( maxBytes >= 0 )
	 done = (bodyBytes >= maxBytes);

      else if ( parentMsg && terminatingLine ) {
	 long	oldPos = ftell(fp);
	 done = (*terminatingLine)(parentMsg, line, prevBlank);
	 if ( done ) fseek(fp, oldPos, SEEK_SET);
      }

      if ( done ) continue;

//
// Add this line to the total
//
      bodyLines++;
      bodyBytes += lineStr.Length();

//
// See if we're looking for the first boundary
//
      if ( boundStr.size() > 0 ) {

	 if ( lineStr.StartsWith(boundStr) ) {

	    lineStr.CutBeg(boundStr.size());
	    if ( lineStr == "\n" ) {

//
// Add children until the last boundary is read
//
	       MsgPartC	*newPart;
	       MsgPartC	*prev = NULL;
	       Boolean	gotLast = False;
	       while ( !gotLast ) {

//
// Scan new part
//
		  newPart = new MsgPartC(this);
		  newPart->offset    = (int)ftell(fp);
		  newPart->msgFile   = msgFile;
		  newPart->parentMsg = parentMsg;

		  if ( !newPart->Scan(fp, maxBytes - bodyBytes,
				      terminatingLine, boundStr, &gotLast,
				      &prevBlank) )
		     return False;

		  checkBlank = False;	// Since we didn't read it here

//
// Add in child sizes
//
		  bodyLines += newPart->lines;
		  bodyBytes += newPart->bytes;

//
// Add in the size of the boundary that ended the child
//
		  bodyLines++;
		  bodyBytes += boundStr.size() + 2/*nl's before and after*/;

//
// Add in the dashes if this was the last boundary
//
		  if ( gotLast ) bodyBytes += 2;
//
// If the previous line was blank, count it here.
//
		  if ( prevBlank ) bodyLines++;

//
// Link in new part
//
		  if ( !child ) {
		     child = newPart;
		  }
		  else if ( prev ) {
		     prev->next = newPart;
		     newPart->prev = prev;
		  }

		  prev = newPart;

	       } // End while not last boundary
	    } // End if found first boundary

	    else if ( lineStr == "--\n" ) {	// No children???
	       if ( gotLastBound ) *gotLastBound = True;
	    }

	 } // End if found a boundary

      } // End if looking for first boundary

      if ( checkBlank ) prevBlank = (strlen(line) <= 1);
      getOK      = fgets(line, BUFLEN-1, fp);
      checkBlank = True;

   } // End for each line

   bodyScanned = True;

   if ( closeFile ) fclose(fp);

   if ( prevLineBlank  ) *prevLineBlank = prevBlank;

//
// See if we left a multipart hanging
//
   if ( nextBound && !gotBound ) {

      StringC errmsg("Malformed MIME message");
      if ( parentMsg ) {
	 errmsg += ' ';
	 errmsg += parentMsg->Number();
      }
      errmsg += ".\nCannot find closing boundary:\n";
      errmsg += nextBound;
      errmsg += "--";

      StringC	lab;
      GetDescription(lab);
      if ( lab.size() > 0 ) {
	 errmsg += "\nfor part:\n";
	 errmsg += lab;
      }

      halApp->PopupMessage(errmsg);
      return False;
   }

//
// Compute total lines and bytes
//
   ComputeSize();

   return True;

} // End Scan

/*-----------------------------------------------------------------------
 *  Method to count the total number of bytes and lines
 */

void
MsgPartC::ComputeSize()
{
   if ( headLines >= 0 && bodyLines >= 0 ) {
      lines = headLines + 1;
      if ( extLines > 0 ) lines += extLines + extBlank;
      lines += bodyLines;
   }

   if ( headBytes >= 0 && bodyBytes >= 0 ) {
      bytes = headBytes + 1;
      if ( extBytes > 0 ) bytes += extBytes + extBlank;
      bytes += bodyBytes;
   }
}

/*-----------------------------------------------------------------------
 *  Method to invalidate the body size
 */

void
MsgPartC::InvalidateSize()
{
   lines     = -1;
   bytes     = -1;
   bodyLines = -1;
   bodyBytes = -1;
}

/*-----------------------------------------------------------------------
 *  Method that returns True if the external body part needs to be
 *     retrieved
 */

Boolean
MsgPartC::NeedFetch()
{
   if ( !IsExternal() || IsLocal() ) return False;

//
// We have no way of knowing whether mail server attachments have arrived
//    so we always fetch.
//
   if ( IsMail() ) return True;

//
// If the attachment permissions are "read-write", we must get the file
//    again no matter what.
//
   ParamC	*param = Param("permission");
   if ( param && param->val.Equals("read-write", IGNORE_CASE) ) return True;

//
// If the file is not here, we need it.
//
   return (dataFile.size() == 0);

} // End NeedFetch

/*-----------------------------------------------------------------------
 *  Method that returns a message object for this part if the type is
 *     message/rfc822
 */

FileMsgC*
MsgPartC::ChildMsg()
{
   if ( !Is822() ) return NULL;

   if ( !childMsg ) {

      char	*name;
      int	childOff = 0;
      int	childLen = 0;

//
// Use the data file if present
//
      if ( dataFile.size() > 0 ) {
	 name = dataFile;
	 struct stat	stats;
	 if ( stat(dataFile, &stats) == 0 ) childLen = (int)stats.st_size;
      }

//
// Use the message file if present
//
      else if ( msgFile.size() > 0 ) {
	 name     = msgFile;
	 childOff = bodyOffset;
	 childLen = bodyBytes;
      }

//
// Create a data file
//
      else {

	 StringC	text;
	 if ( bodyBytes > 0 ) text.reSize(bodyBytes);

	 GetText(text, /*fp*/NULL, /*getHead*/False, /*getExtHead*/False,
				   /*getBody*/True, /*restoreFroms*/False);

	 char	*cs = tempnam(NULL, "msg.");
	 dataFile = cs;
	 free(cs);
	 delDataFile = True;

	 text.WriteFile(dataFile);

	 name     = dataFile;
	 childLen = text.size();
      }

      childMsg = new FileMsgC(name, childOff, childLen, False/*don't delete*/);
      childMsg->Body();	// To force scanning

   } // End if child message needed

   return childMsg;

} // End ChildMsg

/*---------------------------------------------------------------
 *  Method to determine if a mime tree contains only plain text parts
 */

Boolean
MsgPartC::PlainTextOnly() const
{
   if (  IsPlainText()  ) return True;
   if (  IsText()       ) return False;
   if ( !IsMultipart()  ) return True;	// Just a graphic.  Plain OK

   if ( IsAlternative() ) {
      MsgPartC	*altPart = BestAlternative();
      return altPart->PlainTextOnly();
   }

   else {
      MsgPartC	*part = child;
      while ( part ) {
	 if ( !part->PlainTextOnly() ) return False;
	 part = part->next;
      }
   }

   return True;

} // End PlainTextOnly

/*--------------------------------------------------------------------
 *  Method to count the children
 */
int
MsgPartC::ChildCount() const
{
   int		count = 0;
   MsgPartC	*part = child;
   while ( part ) {
      count++;
      part = part->next;
   }

   return count;
}

/*--------------------------------------------------------------------
 *  Method to determine how many non-multipart parts there are in this
 *    tree.
 */

int
MsgPartC::LeafCount() const
{
   int	count = 0;

//
// Loop through children
//
   MsgPartC	*part = child;
   while ( part ) {

      if ( part->IsMultipart() )
	 count += part->LeafCount();
      else
	 count++;

      part = part->next;
   }

   return count;

} // End LeafCount

/*---------------------------------------------------------------
 *  Method to copy the text as it appears in the mail message.
 */

Boolean
MsgPartC::CopyText(char *ofile, FILE *ofp, FILE *ifp, Boolean copyHead,
		   Boolean copyExtHead, Boolean copyBody, Boolean protectFroms,
		   Boolean restoreFroms, Boolean endWithNL) const
{
   if ( copyHead && copyBody ) copyExtHead = True;
   else if ( !IsExternal() ) copyExtHead = False;

   if ( !copyBody ) {
      protectFroms = False;
      restoreFroms = False;
   }

   if ( msgFile.size() == 0 ) {
      if ( parentMsg && parentMsg->IsImap() ) {
	 StringC	text;
	 return (parentMsg->GetPartText(this, text, copyHead, copyExtHead,
				        copyBody) && text.WriteFile(ofp));
      }
      else
	 return False;
   }

//
// Figure out how much to copy
//
   int	off = 0;
   int	len = 0;
   if ( copyHead && copyBody ) {	// Assume copyExtHead
      off = offset;
      len = bytes;
   }
   else if ( copyHead && copyExtHead ) {
      off = offset;
      len = headBytes+1+extBytes;
   }
   else if ( copyExtHead && copyBody ) {
      off = extOffset;
      len = extBytes+1+bodyBytes;
   }
   else if ( copyHead ) {
      off = offset;
      len = headBytes;
   }
   else if ( copyExtHead ) {
      off = extOffset;
      len = extBytes;
   }
   else if ( copyBody ) {
      off = bodyOffset;
      len = bodyBytes;
   }

//
// Perform the copy
//
   return CopyText(ofile, ofp, ifp, off, len,
   		   protectFroms, restoreFroms, endWithNL);

} // End CopyText

/*---------------------------------------------------------------
 *  Method to copy the text as it appears in the mail message.
 */

Boolean
MsgPartC::CopyText(char *ofile, FILE *ofp, FILE *ifp, int off, int len,
		   Boolean protectFroms, Boolean restoreFroms,
		   Boolean endWithNL) const
{
   if ( msgFile.size() == 0 ) return False;

   Boolean	closeInput = (ifp == NULL);
   if ( !ifp ) {
      ifp = fopen(msgFile, "r");
      if ( !ifp ) {
	 StringC	errmsg("Could not open file: \"");
	 errmsg += msgFile;
	 errmsg += "\".\n";
	 errmsg += SystemErrorMessage(errno);
	 halApp->PopupMessage(errmsg);
	 return False;
      }
   }

   fseek(ifp, off, SEEK_SET);

   int	remaining = len;

   char	buffer[BUFLEN];
   buffer[0] = 0;

   Boolean	error  = False;
   Boolean	lastNL = False;
   if ( protectFroms || restoreFroms ) {

      RegexC	*fromPat = UnixFolderC::fromPat;
      CharC	bufStr;

      while ( remaining > 0 ) {

//
// Read a line from the source file
//
	 if ( !fgets(buffer, BUFLEN-1, ifp) ) {
	    StringC	errmsg("Could not read file: \"");
	    errmsg += msgFile;
	    errmsg += "\".\n";
	    errmsg += SystemErrorMessage(errno);
	    halApp->PopupMessage(errmsg);
	    if ( closeInput ) fclose(ifp);
	    return False;
	 }

//
// See if it needs protecting
//
	 Boolean	error = False;
	 bufStr = buffer;
	 if ( protectFroms && buffer[0] == 'F' ) {
	    if ( bufStr.StartsWith("From ") &&
		 (!fromPat || fromPat->match(bufStr)) ) {
	       error = (fwrite(">", 1, 1, ofp) != 1);
	    }
	 }

//
// See if it needs restoring
//
	 else if ( restoreFroms && buffer[0] == '>' ) {
	    if ( bufStr.StartsWith(">From ") &&
		 (!fromPat || fromPat->match(bufStr, 1)) ) {
	       bufStr.CutBeg(1);
	    }
	 }

//
// Write bytes to destination file
//
	 if ( !error ) error = !bufStr.WriteFile(ofp);

	 if ( error ) {
	    StringC	errmsg("Could not write file: \"");
	    errmsg += ofile;
	    errmsg += "\".\n";
	    errmsg += SystemErrorMessage(errno);
	    halApp->PopupMessage(errmsg);
	    if ( closeInput ) fclose(ifp);
	    return False;
	 }

	 lastNL = bufStr.EndsWith('\n');
	 remaining -= strlen(buffer);

      } // End while bytes remain to be copied

   } // End if Froms may need protecting or restoring

   else {

      while ( remaining > 0 ) {

	 int	count = MIN(remaining, BUFLEN-1);

//
// Read bytes from source file
//
	 int	check = fread(buffer, 1, count, ifp);
	 if ( check <= 0 && ferror(ifp) ) {
	    StringC	errmsg("Could not read file: \"");
	    errmsg += msgFile;
	    errmsg += "\".\n";
	    errmsg += SystemErrorMessage(errno);
	    halApp->PopupMessage(errmsg);
	    if ( closeInput ) fclose(ifp);
	    return False;
	 }

//
// Write bytes to destination file
//
	 count = check;
	 buffer[count] = 0;
	 check = fwrite(buffer, 1, count, ofp);
	 if ( check != count ) {
	    StringC	errmsg("Could not write file: \"");
	    errmsg += ofile;
	    errmsg += "\".\n";
	    errmsg += SystemErrorMessage(errno);
	    halApp->PopupMessage(errmsg);
	    if ( closeInput ) fclose(ifp);
	    return False;
	 }

	 lastNL = (buffer[count-1] == '\n');
	 remaining -= count;
	 if (feof(ifp)) remaining = 0;

      } // End while bytes remain to be copied

   } // End if no From issues

//
// If the body didn't end with a newline, write one
//
   if ( !error && len > 0 && endWithNL && !lastNL )
      error = (fwrite("\n", 1, 1, ofp) != 1);

   if ( !error ) error = (fflush(ofp) != 0);
   if ( closeInput ) fclose(ifp);

   return !error;

} // End CopyText

/*---------------------------------------------------------------
 *  Method to copy the source for this part to a file
 */

Boolean
MsgPartC::WriteFile(char *ofile, FILE *ofp)
{
   Boolean	closeOutput = (ofp == NULL);
   if ( !ofp ) {
      ofp = fopen(ofile, "r");
      if ( !ofp ) {
	 StringC	errmsg("Could not open file: \"");
	 errmsg += ofile;
	 errmsg += "\".\n";
	 errmsg += SystemErrorMessage(errno);
	 halApp->PopupMessage(errmsg);
	 return False;
      }
   }

//
// If there's no message file, we have to get the text.
//
   FILE		*ifp = NULL;
   StringC	text;

   if ( msgFile.size() == 0 ) {

      if ( bodyBytes > 0 ) text.reSize(bodyBytes);
      GetText(text, /*fp*/NULL, /*getHead*/True, /*getExtHead*/True,
			        /*getBody*/True, /*restoreFroms*/False);
   }

   else {

       ifp = fopen(msgFile, "r");
       if ( !ifp ) {
	  StringC	errmsg("Could not open file: \"");
	  errmsg += msgFile;
	  errmsg += "\".\n";
	  errmsg += SystemErrorMessage(errno);
	  halApp->PopupMessage(errmsg);
	  if ( closeOutput ) fclose(ofp);
	  return False;
       }

       fseek(ifp, offset, SEEK_SET);
   }

   Boolean	error = False;

//
// Write the content-type header if it is not present in the source
//
   if ( defConType ) {
      StringC	headStr("Content-Type: ");
      headStr += conStr;
      headStr += '\n';
      error = !headStr.WriteFile(ofp);
   }

//
// Copy the source
//
   int	remaining = (ifp ? bytes : text.length());

   char	buffer[BUFLEN];
   buffer[0] = 0;

   while ( !error && remaining > 0 ) {

      int	count = MIN(remaining, BUFLEN-1);
      int	check;

//
// Read bytes from source file
//
      if ( ifp ) {
	  check = fread(buffer, 1, count, ifp);
	  if ( check <= 0 ) {
	     StringC	errmsg("Could not read file: \"");
	     errmsg += msgFile;
	     errmsg += "\".\n";
	     errmsg += SystemErrorMessage(errno);
	     halApp->PopupMessage(errmsg);
	     error = True;
	  }
      }
      else {
	  check = text.length();
      }
//
// Write bytes to destination file
//
      if ( !error ) {

	 count = check;

	 if ( ifp ) {
	     buffer[count] = 0;
	     check = fwrite(buffer, 1, count, ofp);
	 }
	 else {
	     check = fwrite((char*)text, 1, count, ofp);
	 }

	 if ( check != count ) {
	    StringC	errmsg("Could not write file: \"");
	    errmsg += ofile;
	    errmsg += "\".\n";
	    errmsg += SystemErrorMessage(errno);
	    halApp->PopupMessage(errmsg);
	    error = True;
	 }
      }

      remaining -= count;

   } // End while bytes remain to be copied

   if ( ifp ) fclose(ifp);
   if ( closeOutput ) fclose(ofp);

   return !error;

} // End WriteFile

/*---------------------------------------------------------------
 *  Method to create the data file if it doesn't exist.
 */

Boolean
MsgPartC::CreateDataFile()
{
   if ( dataFile.size() > 0 ) return True;

   if ( IsExternal() ) return False;

//
// Read the data
//
   StringC	bodyStr;
   if ( !GetData(bodyStr) ) return False;

//
// Reading the body may have created the data file
//
   if ( dataFile.size() > 0 ) return True;

//
// We must create the data file
//
   char	*cs = tempnam(NULL, "data.");
   dataFile    = cs;
   if ( conType == CT_HTML ) {
      // Some brain-damaged MUAs generating HTML mails omit the encapsulating
      // <HTML></HTML> tags so we'll have to tell the browser to display this
      // as a valid HTML doc via the proper file extension.
      dataFile += ".html";
   }
   delDataFile = True;
   free(cs);

   if ( !bodyStr.WriteFile(dataFile) ) {

      StringC	errmsg = "Could not write file: \"";
      errmsg += dataFile;
      errmsg += "\".\n";
      errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);

      unlink(dataFile);
      dataFile.Clear();
      delDataFile = False;

      return False;
   }

   return True;

} // End CreateDataFile
