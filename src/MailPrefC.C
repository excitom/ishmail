/*
 *  $Id: MailPrefC.C,v 1.4 2001/06/21 10:59:30 evgeny Exp $
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
#include "IshAppC.h"
#include "MailPrefC.h"
#include "AppPrefC.h"
#include "ShellExp.h"
#include "Misc.h"
#include "MailPrefWinC.h"

#include <hgl/rsrc.h>
#include <hgl/StringC.h>
#include <hgl/StringListC.h>

/*---------------------------------------------------------------
 *  Constructor
 */

MailPrefC::MailPrefC() : PrefC()
{
   prefWin = NULL;

//
// Read resources
//
   orig.deadFile     = get_string(*halApp, "deadLetterFile",	"+dead.letter");
   orig.sendmailCmd  = get_string(*halApp, "sendmailCmd", "/usr/sbin/sendmail");
   orig.fccFolder    = get_string(*halApp, "outgoingFile",	"+outgoing");
   orig.fccFolder    = get_string(*halApp, "fccFolder",		orig.fccFolder);
   orig.fccFolderDir = get_string(*halApp, "fccFolderDir",
				  ishApp->appPrefs->OrigFolderDir());

   split	     = get_boolean(*halApp, "composeSplitEnabled",False);
   splitSize     = get_int    (*halApp, "composeSplitSize",	100000);
   verifyAddresses   = get_boolean(*halApp, "verifyAddresses",	False);
   saveOnInterrupt   = get_boolean(*halApp, "saveOnInterrupt",	True);
   
//
// From: header; use "username <userAtDomain>" as fallback
//
   StringC fromHeader_fb(ishApp->userName);
   fromHeader_fb += " <";
   fromHeader_fb += ishApp->userAtDomain;
   fromHeader_fb += ">";
   fromHeader      = get_string (*halApp, "fromHeader", fromHeader_fb);
   
   otherHeaders      = get_string (*halApp, "additionalHeaders");
   charset       = get_string (*halApp, "composeCharset",    "us-ascii");

//
// Outgoing message settings
//
   StringC	fccStr = get_string(*halApp, "fccType");
   if      ( fccStr.Equals("folder",  IGNORE_CASE) )
      fccType = FCC_TO_FOLDER;
   else if ( fccStr.Equals("user",    IGNORE_CASE) )
      fccType = FCC_BY_USER;
   else if ( fccStr.Equals("address", IGNORE_CASE) )
      fccType = FCC_BY_ADDRESS;
   else if ( fccStr.Equals("year",    IGNORE_CASE) )
      fccType = FCC_BY_YEAR;
   else if ( fccStr.Equals("month",   IGNORE_CASE) )
      fccType = FCC_BY_MONTH;
   else if ( fccStr.Equals("week",    IGNORE_CASE) )
      fccType = FCC_BY_WEEK;
   else if ( fccStr.Equals("day",     IGNORE_CASE) )
      fccType = FCC_BY_DAY;
   else if ( fccStr.Equals("none",    IGNORE_CASE) )
      fccType = FCC_NONE;

   else {
      Boolean	copyOut = get_boolean(*halApp, "copyOutgoing",	False);
      if ( copyOut ) fccType = FCC_TO_FOLDER;
      else	     fccType = FCC_NONE;
   }

//
// Read list of aliases that require confirmation.
//
   StringC	tmpStr = get_string(*halApp, "confirmMailTo");
   ExtractList(tmpStr, confirmAddrList);
   confirmAddrs = get_boolean(*halApp, "confirmMailToEnabled", True);

   tmpStr = get_string(*halApp, "composeBodyEncoding", "quoted-printable");
   if ( tmpStr.Equals("8-bit", IGNORE_CASE) ) bodyEncType = ET_8BIT;
   else					      bodyEncType = ET_QP;

   tmpStr = get_string(*halApp, "composeHeaderEncoding", "Q");
   if      ( tmpStr.StartsWith('Q', IGNORE_CASE) ) headEncType = ET_QP;
   else if ( tmpStr.StartsWith('B', IGNORE_CASE) ) headEncType = ET_BASE_64;
   else					           headEncType = ET_NONE;

   tmpStr = get_string(*halApp, "outgoingMailType",	"plain");
   if ( tmpStr.EndsWith("plain", IGNORE_CASE) )
      mailType = MAIL_PLAIN;
   else if ( tmpStr.Equals("alternative", IGNORE_CASE) )
      mailType = MAIL_ALT;
   else
      mailType = MAIL_MIME;

   tmpStr = get_string(*halApp, "outgoingTextType",	"enriched");
   if ( tmpStr.EndsWith("plain", IGNORE_CASE) )
      textType = CT_PLAIN;
   else
      textType = CT_ENRICHED;

   sendmailCmd  = orig.sendmailCmd;	ShellExpand(sendmailCmd);
   deadFile     = orig.deadFile;	ishApp->ExpandFolderName(deadFile);
   fccFolder    = orig.fccFolder;	ishApp->ExpandFolderName(fccFolder);
   fccFolderDir = orig.fccFolderDir;	ishApp->ExpandFolderName(fccFolderDir);

} // End constructor

/*---------------------------------------------------------------
 *  destructor
 */

MailPrefC::~MailPrefC()
{
   delete prefWin;
}

/*---------------------------------------------------------------
 *  Method to write resources to resource database
 */

Boolean
MailPrefC::WriteDatabase()
{
   switch (fccType) {
      case (FCC_TO_FOLDER):	Store("fccType", "Folder");	break;
      case (FCC_BY_USER):	Store("fccType", "User");	break;
      case (FCC_BY_ADDRESS):	Store("fccType", "Address");	break;
      case (FCC_BY_YEAR):	Store("fccType", "Year");	break;
      case (FCC_BY_MONTH):	Store("fccType", "Month");	break;
      case (FCC_BY_WEEK):	Store("fccType", "Week");	break;
      case (FCC_BY_DAY):	Store("fccType", "Day");	break;
      case (FCC_NONE):
      default:			Store("fccType", "None");	break;
   }

   Store("fccFolder",		orig.fccFolder);
   Store("fccFolderDir",	orig.fccFolderDir);
   Store("saveOnInterrupt",	saveOnInterrupt);
   Store("deadLetterFile",	orig.deadFile);
   Store("sendmailCmd",		orig.sendmailCmd);
   Store("verifyAddresses",	verifyAddresses);
   Store("fromHeader",	        fromHeader);
   Store("additionalHeaders",	otherHeaders);
   Store("composeSplitEnabled",	split);
   if ( splitSize > 0 ) Store("composeSplitSize",	splitSize);
   Store("composeCharset",	charset);

   switch ( bodyEncType ) {
      case (ET_8BIT): Store("composeBodyEncoding", "8-bit");		break;
      default:	      Store("composeBodyEncoding", "quoted-printable");	break;
   }

   switch ( headEncType ) {
      case (ET_QP):	 Store("composeHeaderEncoding",	"Q");	 break;
      case (ET_BASE_64): Store("composeHeaderEncoding",	"B");	 break;
      default:		 Store("composeHeaderEncoding",	"None"); break;
   }

   switch ( mailType ) {
      case (MAIL_PLAIN): Store("outgoingMailType", "Plain");	   break;
      case (MAIL_MIME):  Store("outgoingMailType", "Mime");	   break;
      case (MAIL_ALT):   Store("outgoingMailType", "Alternative"); break;
   }

   switch ( textType ) {
      case (CT_PLAIN):	     Store("outgoingTextType", "Plain");	break;
      case (CT_ENRICHED):    Store("outgoingTextType", "Enriched");	break;
   }

   Store("confirmMailTo",		confirmAddrList);
   Store("confirmMailToEnabled",	confirmAddrs);

   return True;

} // End WriteDatabase

/*---------------------------------------------------------------
 *  Method to write resources to a file
 */

Boolean
MailPrefC::WriteFile()
{
   StringListC	lineList;
   ReadResFile(lineList);

   switch (fccType) {
      case (FCC_TO_FOLDER):	Update(lineList, "fccType", "Folder");	break;
      case (FCC_BY_USER):	Update(lineList, "fccType", "User");	break;
      case (FCC_BY_ADDRESS):	Update(lineList, "fccType", "Address");	break;
      case (FCC_BY_YEAR):	Update(lineList, "fccType", "Year");	break;
      case (FCC_BY_MONTH):	Update(lineList, "fccType", "Month");	break;
      case (FCC_BY_WEEK):	Update(lineList, "fccType", "Week");	break;
      case (FCC_BY_DAY):	Update(lineList, "fccType", "Day");	break;
      case (FCC_NONE):
      default:			Update(lineList, "fccType", "None");	break;
   }

   Update(lineList, "fccFolder",		orig.fccFolder);
   Update(lineList, "fccFolderDir",		orig.fccFolderDir);
   Update(lineList, "deadLetterFile",		orig.deadFile);
   Update(lineList, "sendmailCmd",		orig.sendmailCmd);
   Update(lineList, "saveOnInterrupt",		saveOnInterrupt);
   Update(lineList, "verifyAddresses",		verifyAddresses);
   Update(lineList, "fromHeader",	        fromHeader);
   Update(lineList, "additionalHeaders",	otherHeaders);

   Update(lineList, "composeCharset",		charset);
   Update(lineList, "composeSplitEnabled",	split);
   if ( splitSize > 0 ) Update(lineList, "composeSplitSize", splitSize);

   switch ( bodyEncType ) {
      case (ET_8BIT):
	 Update(lineList, "composeBodyEncoding",	"8-bit");
	 break;
      default:
	 Update(lineList, "composeBodyEncoding",	"quoted-printable");
	 break;
   }

   switch ( headEncType ) {
      case (ET_QP):
	 Update(lineList, "composeHeaderEncoding",	"Q");
	 break;
      case (ET_BASE_64):
	 Update(lineList, "composeHeaderEncoding",	"B");
	 break;
      default:
	 Update(lineList, "composeHeaderEncoding",	"None");
	 break;
   }

   switch ( mailType ) {
      case (MAIL_PLAIN):
	 Update(lineList, "outgoingMailType",	"Plain");
	 break;
      case (MAIL_MIME):
	 Update(lineList, "outgoingMailType",	"Mime");
	 break;
      case (MAIL_ALT):
	 Update(lineList, "outgoingMailType",	"Alternative");
	 break;
   }

   switch ( textType ) {
      case (CT_PLAIN):
	 Update(lineList, "outgoingTextType",	"Plain");
	 break;
      case (CT_ENRICHED):
	 Update(lineList, "outgoingTextType",	"Enriched");
	 break;
   }

   Update(lineList, "confirmMailTo",		confirmAddrList);
   Update(lineList, "confirmMailToEnabled",	confirmAddrs);

//
// Write the information back to the file
//
   return WriteResFile(lineList);

} // End WriteFile

/*------------------------------------------------------------------------
 * Method to edit the preferences
 */

void
MailPrefC::Edit(Widget parent)
{
   halApp->BusyCursor(True);

   if ( !prefWin ) prefWin = new MailPrefWinC(parent);
   prefWin->Show(parent);

   halApp->BusyCursor(False);
}

/*------------------------------------------------------------------------
 * Method to set dead letter file name
 */

void
MailPrefC::SetDeadFile(const char *name)
{
   if ( orig.deadFile == name ) return;

   deadFile = orig.deadFile = name;
   ishApp->ExpandFolderName(deadFile);
}

/*------------------------------------------------------------------------
 * Method to set the Fcc folder name
 */

void
MailPrefC::SetFccFolder(const char *name)
{
   if ( orig.fccFolder == name ) return;

   fccFolder = orig.fccFolder = name;
   ishApp->ExpandFolderName(fccFolder);
}

/*------------------------------------------------------------------------
 * Method to set the Fcc directory name
 */

void
MailPrefC::SetFccFolderDir(const char *name)
{
   if ( orig.fccFolderDir == name ) return;

   fccFolderDir = orig.fccFolderDir = name;
   ishApp->ExpandFolderName(fccFolderDir);
}

/*------------------------------------------------------------------------
 * Method to set the command used to send mail
 */

void
MailPrefC::SetSendmailCmd(const char *name)
{
   if ( orig.sendmailCmd == name ) return;

   sendmailCmd = orig.sendmailCmd = name;
   ShellExpand(sendmailCmd);

   char	*sp = strchr((char*)sendmailCmd, ' ');
   if ( sp ) {
      int	pos = sp - (char*)sendmailCmd;
      sendmailCmd.Clear(pos);
   }

} // End SetSendmailCmd

