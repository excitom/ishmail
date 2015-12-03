/*
 *  $Id: SendIconC.C,v 1.2 2000/05/07 12:26:12 fnevgeny Exp $
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
#include "SendIconC.h"
#include "SendWinC.h"
#include "IncludeWinC.h"
#include "MsgPartC.h"
#include "MsgC.h"
#include "MimeEncode.h"
#include "FileMisc.h"
#include "SendMisc.h"
#include "HeaderC.h"
#include "ParamC.h"
#include "Misc.h"
#include "Base64.h"
#include "QuotedP.h"

#include <hgl/rsrc.h>
#include <hgl/SysErr.h>

#include <unistd.h>	// For unlink
#include <errno.h>
#include <sys/stat.h>

/*---------------------------------------------------------------
 *  Constructor with file data from include window
 */

SendIconC::SendIconC(SendWinC *sw, IncludeWinC *iw)
: MimeIconC(iw->ContentTypeStr(), sw->BodyText())
{
   sendWin = sw;
   data    = NULL;

   Update(iw);

} // End constructor with file data

/*---------------------------------------------------------------
 *  Constructor with content type.
 */

SendIconC::SendIconC(SendWinC *sw, char *type, char *ifile, char *ofile)
: MimeIconC(type, sw->BodyText())
{
   sendWin = sw;

//
// Build a message part and open the message file
//
   FILE	*fp = CreatePart();
   if ( !fp ) return;

//
// Initialize data from type and file
//
   if ( !ofile ) ofile = ifile;
   Boolean	error = !BuildPartFile(fp, type, ifile, ofile);
   if ( !error ) {
      fseek(fp, 0, SEEK_SET);
      error = !data->ScanHead(fp);
   }

   if ( !error ) CalcBodySize(fp);

   fclose(fp);

   if ( !error ) InitIcon();

} // End constructor with content type

/*---------------------------------------------------------------
 *  Constructor with rfc822 message.
 */

SendIconC::SendIconC(SendWinC *sw, MsgC *msg)
: MimeIconC("message/rfc822", sw->BodyText())
{
   sendWin = sw;

//
// Build a message part and open the message file
//
   FILE	*fp = CreatePart();
   if ( !fp ) return;

//
// Initialize data from message
//
   Boolean	error = !BuildPartFile(fp, msg);
   if ( !error ) {
      fseek(fp, 0, SEEK_SET);
      error = !data->ScanHead(fp);
   }

   if ( !error ) CalcBodySize(fp);

   fclose(fp);

   if ( !error ) InitIcon();

} // End constructor with content type

/*---------------------------------------------------------------
 *  Constructor with a mime message part
 */

SendIconC::SendIconC(SendWinC *sw, MsgPartC *part)
: MimeIconC(part->conStr, sw->BodyText())
{
   sendWin = sw;

//
// Build a message part and open the message file
//
   FILE	*fp = CreatePart();
   if ( !fp ) return;

//
// Initialize data from the specified part
//
   Boolean	error = !BuildPartFile(fp, part);
   if ( !error ) {
      fseek(fp, 0, SEEK_SET);
      error = !data->ScanHead(fp);
   }

   if ( !error ) CalcBodySize(fp);

   fclose(fp);

   if ( !error ) InitIcon();

} // End constructor with mime part

/*---------------------------------------------------------------
 *  Method to create a new message part
 */

FILE*
SendIconC::CreatePart()
{
   data = new MsgPartC;

//
// Create temporary message part file
//
   char	*cs = tempnam(NULL, "part.");
   FILE	*fp = fopen(cs, "w+");
   if ( !fp ) {
      StringC	errmsg("Could not create file: \"");
      errmsg += cs;
      errmsg += "\"\n.";
      errmsg += SystemErrorMessage(errno);
      sendWin->PopupMessage(errmsg);
   }
   else {
      data->msgFile    = cs;
      data->delMsgFile = True;
   }

   free(cs);
   return fp;

} // End CreatePart

/*---------------------------------------------------------------
 *  Method to build a MIME message body for the information in the include
 *     window
 */

Boolean
SendIconC::BuildPartFile(FILE *fp, IncludeWinC *iw)
{
   StringC	headStr;
   StringC	tmpStr;
   StringC	name;
   iw->GetFileName(name);

//
// Write external-body header if necessary
//
   MimeAccessType	accType = iw->AccessType();
   if ( IsExternal(accType) ) {

      headStr = "Content-Type: message/external-body;\n\taccess-type=";

//
// Add the access information
//
      switch (accType) {

	 case (AT_LOCAL_FILE):
	    headStr += "\"local-file\"";
	    AddQuotedVal(headStr, ";\n\tname=", name);
	    break;

	 case (AT_ANON_FTP):
	 case (AT_FTP):
	 case (AT_TFTP):

	    if      ( accType == AT_TFTP ) headStr += "\"tftp\"";
	    else if ( accType == AT_FTP  ) headStr += "\"ftp\"";
	    else			   headStr += "\"anon-ftp\"";

	    AddQuotedVal(headStr, ";\n\tname=", name);

	    iw->GetFtpHost(tmpStr);
	    AddQuotedVal(headStr, ";\n\tsite=", tmpStr);

	    iw->GetFtpDir(tmpStr);
	    if ( tmpStr.size() > 0 )
	       AddQuotedVal(headStr, ";\n\tdirectory=", tmpStr);

	    tmpStr.Clear();
	    if ( accType == AT_TFTP ) iw->GetTftpMode(tmpStr);
	    else		      iw->GetFtpMode(tmpStr);
	    if ( tmpStr.size() > 0 )
	       AddQuotedVal(headStr, ";\n\tmode=", tmpStr);

	    break;

	 case (AT_MAIL_SERVER):

	    headStr += "\"mail-server\"";

	    iw->GetMailAddr(tmpStr);
	    AddQuotedVal(headStr, ";\n\tserver=", tmpStr);

	    iw->GetMailSubject(tmpStr);
	    if ( tmpStr.size() > 0 )
	       AddQuotedVal(headStr, ";\n\tsubject=", tmpStr);

	    break;

	 case (AT_INLINE):	// Not possible here
	 default:
	    break;

      } // End switch access type

//
// Add parameters common to all
//
      iw->GetExpiration(tmpStr);
      if ( tmpStr.size() > 0 )
	 AddQuotedVal(headStr, ";\n\texpiration=", tmpStr);

      iw->GetSize(tmpStr);
      if ( tmpStr.size() > 0 )
	 AddQuotedVal(headStr, ";\n\tsize=", tmpStr);

      if ( iw->IsReadWrite() )
	 headStr += ";\n\tpermission=\"read-write\"";

      if ( !headStr.EndsWith('\n') ) headStr += '\n';

//
// Add description header
//
      iw->GetDescription(tmpStr);
      if ( tmpStr.size() > 0 ) {
	 headStr += "Content-Description: ";
	 headStr += tmpStr;
	 if ( !headStr.EndsWith('\n') ) headStr += '\n';
      }

//
// Add a blank line
//
      while ( !headStr.EndsWith("\n\n") ) headStr += '\n';

   } // End if type is message/external-body

//
// Build the content-type header for this body part
// If we're uuencoding, force the type to "text/plain"
//
   headStr += "Content-Type: ";
   if ( iw->IsUUencode() ) headStr += "text/plain";
   else			   headStr += iw->ContentTypeStr();

   iw->GetCharset(tmpStr);
   if ( tmpStr.size() > 0 && !tmpStr.Equals("us-ascii", IGNORE_CASE) )
      AddQuotedVal(headStr, ";\n\tcharset=", tmpStr);

   iw->GetAppType(tmpStr);
   if ( tmpStr.size() > 0 ) AddQuotedVal(headStr, ";\n\ttype=", tmpStr);

   iw->GetAppPadding(tmpStr);
   if ( tmpStr.size()  > 0 ) AddQuotedVal(headStr, ";\n\tpadding=", tmpStr);

   iw->GetOtherParams(tmpStr);
   if ( tmpStr.size() > 0 ) {
      headStr += "; ";
      headStr += tmpStr;
   }

   if ( !headStr.EndsWith('\n') ) headStr += '\n';

//
// Add description header
//
   iw->GetDescription(tmpStr);
   if ( tmpStr.size() > 0 ) {
      headStr += "Content-Description: ";
      headStr += tmpStr;
      if ( !headStr.EndsWith('\n') ) headStr += '\n';
   }

//
// Add a content-id header
//
   if ( IsExternal(accType) ) {
      GenId(tmpStr);
      headStr += "Content-Id: ";
      headStr += '<';
      headStr += tmpStr;
      headStr += ">\n";
   }

//
// Add a content-disposition header
//
   headStr += "Content-Disposition: attachment";
   iw->GetOutputName(tmpStr);
   if ( tmpStr.size() > 0 ) {
      headStr += ";\n\tfilename=";
      headStr += tmpStr;
   }
   headStr += '\n';

//
// Add a content transfer encoding header
//
   MimeEncodingType	encType = iw->EncodingType();
   if ( !IsExternal(accType) &&
        (Is8Bit(encType) || IsBase64(encType) || IsQP(encType)) ) {

      headStr += "Content-Transfer-Encoding: ";
      headStr += EncodingTypeStr(encType);
      headStr += '\n';
   }

//
// Add a blank line
//
   while ( !headStr.EndsWith("\n\n") ) headStr += '\n';
   if ( !headStr.WriteFile(fp) ) {
      StringC	errmsg("Could not write file: \"");
      errmsg += data->msgFile;
      errmsg += "\".\n";
      errmsg += SystemErrorMessage(errno);
      sendWin->PopupMessage(errmsg);
      return False;
   }

//
// Add the body
//
   if ( IsMail(accType) ) {

      iw->GetMailBody(tmpStr);
      if ( !tmpStr.WriteFile(fp) ) {
	 StringC	errmsg("Could not write file: \"");
	 errmsg += data->msgFile;
	 errmsg += "\".\n";
	 errmsg += SystemErrorMessage(errno);
	 sendWin->PopupMessage(errmsg);
	 return False;
      }
   }

   else if ( !IsExternal(accType) ) {

//
// Encode the file if necessary
//
      if ( IsEncoded(encType) && !iw->AlreadyEncoded() ) {

	 tmpStr = "Encoding file ";
	 tmpStr += name;
	 sendWin->Message(tmpStr);

//
// Encode the original file and write the result to the output file
//
	 Boolean	success = True;
	 if      ( IsBase64(encType) )
	    success = FileToFile64(name, data->msgFile, iw->IsText(), NULL, fp);
	 else if ( IsQP(encType) )
	    success = FileToFileQP(name, data->msgFile, NULL, fp);
	 else if ( IsUU(encType) )
	    success = FileToFileUU(name, data->msgFile, fp);
	 else if ( IsBinHex(encType) )
	    success = FileToFileBH(name, data->msgFile, fp);

	 if ( !success ) {
	    StringC	errmsg("Could not encode file: ");
	    errmsg += name;
	    errmsg += "\n";
	    errmsg += SystemErrorMessage(errno);
	    sendWin->PopupMessage(errmsg);
	    return False;
	 }

	 data->dataFile    = name;
	 data->delDataFile = False;

      } // End if encoded

//
// Just copy the file
//
      else {

         if ( !CopyFile(name, data->msgFile, False, False, NULL, fp) )
	    return False;

      } // End if no encoding

   } // End if not external attachment

   return True;

} // End BuildPartFile

/*---------------------------------------------------------------
 *  Method to build a MIME message body for the specified type and file
 *     This will either be a multipart with no file or a text/plain with
 *     a file.
 */

Boolean
SendIconC::BuildPartFile(FILE *fp, char *type, char *ifile, char *ofile)
{
   StringC	headStr;

   data->dataFile = ifile;

//
// Build the content-type header for this body part
//
   headStr += "Content-Type: ";
   headStr += type;
   if ( headStr.Contains("multipart/") ) {
       StringC	bound;
       GenBoundary(bound);
       AddQuotedVal(headStr, ";\n boundary=", bound);
   }
   headStr += '\n';

//
// Add a content-disposition header
//
   if ( ofile ) {
      headStr += "Content-Disposition: attachment";
      headStr += ";\n\tfilename=";
      headStr += ofile;
      headStr += '\n';
   }

//
// Add a blank line
//
   while ( !headStr.EndsWith("\n\n") ) headStr += '\n';
   if ( !headStr.WriteFile(fp) ) {
      StringC	errmsg("Could not write file: \"");
      errmsg += data->msgFile;
      errmsg += "\".\n";
      errmsg += SystemErrorMessage(errno);
      sendWin->PopupMessage(errmsg);
      return False;
   }

//
// Add the body
//
   if ( ifile ) {
      if ( !CopyFile(ifile, data->msgFile, False, False, NULL, fp) )
	 return False;
   }

   return True;

} // End BuildPartFile

/*---------------------------------------------------------------
 *  Method to build a MIME message body for the specified mail message
 */

Boolean
SendIconC::BuildPartFile(FILE *fp, MsgC *msg)
{
//
// Build the content-type header for this body part
//
   StringC	headStr;
   headStr += "Content-Type: message/rfc822\n\n";
   if ( !headStr.WriteFile(fp) ) {
      StringC	errmsg("Could not write file: \"");
      errmsg += data->msgFile;
      errmsg += "\".\n";
      errmsg += SystemErrorMessage(errno);
      sendWin->PopupMessage(errmsg);
      return False;
   }

//
// Make a copy of the message
//
   if ( !msg->WriteFile(fp, /*copyHead=*/True, /*allHead=*/True,
      			/*statHead=*/False, /*addBlank=*/False,
			/*protectFroms=*/False) )
      return False;

//
// Extract the subject
//
   HeaderC	*sub = msg->Header("Subject");
   if ( sub ) data->SetSubject(sub->full);

   return True;

} // End BuildPartFile

/*---------------------------------------------------------------
 *  Method to build a MIME message body for the specified MIME tree
 */

Boolean
SendIconC::BuildPartFile(FILE *fp, MsgPartC *tree)
{
   return tree->WriteFile(data->msgFile, fp);
}

/*---------------------------------------------------------------
 *  Method to determine the number of body bytes for a message part
 */

void
SendIconC::CalcBodySize(FILE *fp)
{
   if ( fp ) {
      fseek(fp, 0, SEEK_END);
      data->bytes = (int)ftell(fp);
   }
   else {
      struct stat	stats;
      if ( stat(data->msgFile, &stats) == 0 )
	 data->bytes = (int)stats.st_size;
   }

   data->bodyBytes = data->bytes - data->headBytes - 1/*blank line*/;
   if ( data->extBytes > 0 ) data->bodyBytes -= (data->extBytes + 1);
}

/*---------------------------------------------------------------
 *  Method to update the variables using the values in an include window
 */

void
SendIconC::Update(IncludeWinC *iw)
{
//
// Delete the existing data and start over
//
   delete data;
   data = NULL;

//
// Build a message part and open the message file
//
   FILE	*fp = CreatePart();
   if ( !fp ) return;

//
// Initialize data from include window
//
   Boolean	error = !BuildPartFile(fp, iw);
   if ( !error ) {
      fseek(fp, 0, SEEK_SET);
      error = !data->ScanHead(fp);
   }

   if ( !error ) CalcBodySize(fp);

   fclose(fp);

   if ( !error ) InitIcon();

} // End Update

/*---------------------------------------------------------------
 *  Method to change the message file
 */

Boolean
SendIconC::SetSourceFile(char *file, Boolean del)
{
   data->Reset();

   data->msgFile    = file;
   data->delMsgFile = del;

   if ( !data->ScanHead() ) return False;

   CalcBodySize();

   InitIcon();

   return True;

} // End SetSourceFile

/*---------------------------------------------------------------
 *  Method to initialize the icon labels
 */

void
SendIconC::InitIcon()
{
   dropColor = get_color("SendIconC", data->conStr, "validDropColor",
   			  rt->TextArea(), "green");

   popupLabel.Clear();

//
// Create the popup menu label
//
   StringC	desc;
   data->GetDescription(desc);
   if ( desc.size() ) {
      popupLabel = "Desc: ";
      if ( desc[0] != '\"' ) popupLabel += "\"" + desc + "\"";
      else		     popupLabel += desc;
      popupLabel += "\n";
   }

   popupLabel += "Type: " + data->conStr;
   if ( data->IsExternal() ) popupLabel += " (external-body)";
   popupLabel += "\n";

   StringC	accStr;
   if ( data->IsExternal() ) {

      switch ( data->accType ) {

	 case (AT_LOCAL_FILE):	accStr = "local-file";	break;
	 case (AT_ANON_FTP):	accStr = "anon-ftp";	break;
	 case (AT_FTP):		accStr = "ftp";		break;
	 case (AT_TFTP):	accStr = "tftp";		break;
	 case (AT_MAIL_SERVER):	accStr = "mail-server";	break;

	 case (AT_INLINE):
	 default:
	    break;
      }

      if ( accStr.size() > 0 ) {
	 popupLabel += "access-type: ";
	 popupLabel += accStr;
	 popupLabel += "\n";
      }
   }

   if ( data->dataFile.size() > 0 ) {
      popupLabel += "name: ";
      popupLabel += data->dataFile;
      popupLabel += "\n";
   }

//
// Add access parameters
//
   if ( data->IsExternal() ) {

      ParamC	*param = data->accParams;
      while ( param ) {
	 if ( popupLabel.size() > 0 ) popupLabel += '\n';
	 popupLabel += param->full;
	 param = param->next;
      }
   }

//
// Remove last newline
//
   if ( popupLabel.EndsWith('\n') ) popupLabel.CutEnd(1);

//
// Create the icon label.
//
   StringC	label;
   if ( data->subject ) data->subject->GetValueText(label);

   if ( label.size() == 0 ) {
      if      ( desc.size()           ) label = desc;
      else if ( data->dataFile.size() ) label = data->dataFile;
      else				label = data->conStr;
   }

//
// Add external info
//
   if ( data->IsExternal() ) {

      accStr.Clear();
      switch ( data->accType ) {
	 case (AT_LOCAL_FILE):	accStr = "local";	break;
	 case (AT_ANON_FTP):	accStr = "anon-ftp";	break;
	 case (AT_FTP):		accStr = "ftp";		break;
	 case (AT_TFTP):	accStr = "tftp";	break;
	 case (AT_MAIL_SERVER):	accStr = "mail-server";	break;
	 case (AT_INLINE):				break;
      }

      if ( accStr.size() > 0 ) {
	 label += " (";
	 label += accStr;
	 label += " access)";
      }
   }

   SetLabel(label);

} // End InitIcon

/*---------------------------------------------------------------
 *  desctructor
 */

SendIconC::~SendIconC()
{
   delete data;
}

/*---------------------------------------------------------------
 *  Method to return a plain text description of this graphic
 */

Boolean
SendIconC::GetText(StringC& text)
{
//
// Substitute address for %a
//
   Boolean	error = False;
   if ( sendWin->DescTemplate().size() > 0 ) {

      StringC	addrStr;
      addrStr += (int)this;

      StringC	label = sendWin->DescTemplate();
      label.Replace("%a", addrStr);

      text += label;
   }

   else
      error = !GetPartText(data, text);

   return !error;

} // End GetText

/*---------------------------------------------------------------
 *  Method to return a plain text description of a message part
 */

Boolean
SendIconC::GetPartText(MsgPartC *part, StringC& text)
{
   Boolean	error = False;

//
// If this is a multipart, scan and process children
//
   if ( part->IsMultipart() ) {

      part->Scan(NULL, part->bodyBytes, NULL);
      MsgPartC	*child = part->child;
      while ( child && !error ) {
	 error = !GetPartText(child, text);
	 child = child->next;
      }
   }

//
// If this is a mail attachment or an inline message, text part or uuencoded
//    file, return that.
//
   else if ( part->IsMail() || (!part->IsExternal() &&
	     (part->IsText() || part->Is822() || part->IsUUencoded())) ) {

      error = !part->GetText(text);
   }

//
// If there is no suitable text representation, insert a message
//
   else {

      text += "\n\n(";

      if ( part->IsExternal() ) text += "Attached ";
      else			text += "Included ";

      switch ( part->conType ) {

	 case CT_PLAIN:		text += "Text";		break;
	 case CT_RICH:		text += "RichText";	break;
	 case CT_GIF:		text += "GIF Image";	break;
	 case CT_JPEG:		text += "JPEG Image";	break;
	 case CT_BASIC_AUDIO:	text += "U-LAW Audio";	break;
	 case CT_MPEG:		text += "MPEG Video";	break;
	 case CT_OCTET:		text += "Binary";	break;
	 case CT_POSTSCRIPT:	text += "PostScript";	break;
	 case CT_RFC822:	text += "Mail Message";	break;
	 case CT_UNKNOWN:	text +=  part->conStr;	break;

      } // End switch type

      StringC	desc;
      part->GetDescription(desc);
      if ( desc.size() > 0 || part->dataFile.size() > 0 ) {
	 text += " File: ";
	 if ( desc.size() > 0 ) text += desc;
	 else			text += part->dataFile;
      }

      text += ")\n";

   } // End if no suitable text representation is available

   return !error;

} // End GetPartText

/*---------------------------------------------------------------
 *  Method to build a MIME message body for this file
 */

Boolean
SendIconC::Write(FILE *fp, StringListC *contList)
{
//
// Copy any Content- headers
//
   HeaderC	*head = data->headers;
   StringC	headStr;
   Boolean	error = False;
   while ( head ) {

      if ( head->key.StartsWith("Content-", IGNORE_CASE) ) {
	 headStr = head->full;
	 headStr += '\n';
	 if ( contList ) error = !AddHeader(headStr, *contList);
	 else		 error = !headStr.WriteFile(fp);
	 if ( error ) return False;
      }

      head = head->next;
   }

   if ( !contList ) {
      headStr = "\n";
      if ( !headStr.WriteFile(fp) ) return False;
   }

//
// Copy the body
//
   return data->CopyText("", fp, NULL, /*copyHead*/False, /*copyExtHead*/True,
   			 /*copyBody*/True, /*protectFroms*/False,
			 /*restoreFroms*/False, /*endWithNL*/False);

} // End Write
