/*
 *  $Id: SigPrefC.C,v 1.2 2000/05/07 12:26:13 fnevgeny Exp $
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
#include "SigPrefC.h"
#include "ShellExp.h"
#include "SafeSystem.h"
#include "SigPrefWinC.h"

#include <hgl/rsrc.h>
#include <hgl/CharC.h>
#include <hgl/StringListC.h>
#include <hgl/SysErr.h>

#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

/*---------------------------------------------------------------
 *  Constructor
 */

SigPrefC::SigPrefC() : PrefC()
{
   prefWin = NULL;

//
// Read resources
//
   StringC	defSigFile = ishApp->home;
   defSigFile += "/.signature";
   orig.extPSigFile = get_string(*halApp, "signatureFile",	   defSigFile);
   orig.extESigFile = get_string(*halApp, "enrichedSignatureFile", defSigFile);
   orig.intPSigFile = get_string(*halApp, "internalSignatureFile", defSigFile);
   orig.intESigFile = get_string(*halApp, "internalEnrichedSignatureFile",
   				 defSigFile);

   appendSig = get_boolean(*halApp, "appendSignature",	False);
   addPrefix = get_boolean(*halApp, "addSignaturePrefix",	False);

   StringC	tmpStr = get_string (*halApp, "signatureType",	"plain");
   if      ( tmpStr.Equals("enriched", IGNORE_CASE) )
      type = ENRICHED_SIG;
   else if ( tmpStr.Equals("enriched_if_mime", IGNORE_CASE) )
      type = ENRICHED_MIME_SIG;
   else
      type = PLAIN_SIG;

//
// Get the signature file names
//
   extPSigFile = orig.extPSigFile;	ShellExpand(extPSigFile);
   char	*sp = strchr((char*)extPSigFile, ' ');
   if ( sp ) {
      int       pos = sp - (char*)extPSigFile;
      extPSigFile.Clear(pos);
   }

   extESigFile = orig.extESigFile;	ShellExpand(extESigFile);
   sp = strchr((char*)extESigFile, ' ');
   if ( sp ) {
      int       pos = sp - (char*)extESigFile;
      extESigFile.Clear(pos);
   }

   intPSigFile = orig.intPSigFile;	ShellExpand(intPSigFile);
   sp = strchr((char*)intPSigFile, ' ');
   if ( sp ) {
      int       pos = sp - (char*)intPSigFile;
      intPSigFile.Clear(pos);
   }

   intESigFile = orig.intESigFile;	ShellExpand(intESigFile);
   sp = strchr((char*)intESigFile, ' ');
   if ( sp ) {
      int       pos = sp - (char*)intESigFile;
      intESigFile.Clear(pos);
   }

//
// See if there's a signature resource to fall back on.
//
   signature = get_string(*halApp, "signature", "");

} // End constructor

/*---------------------------------------------------------------
 *  destructor
 */

SigPrefC::~SigPrefC()
{
   delete prefWin;
}

/*---------------------------------------------------------------
 *  Method to write resources to resource database
 */

Boolean
SigPrefC::WriteDatabase()
{
   Store("appendSignature",	appendSig);
   Store("addSignaturePrefix",	addPrefix);

   switch (type) {
      case (PLAIN_SIG):		Store("signatureType", "Plain");    break;
      case (ENRICHED_SIG):	Store("signatureType", "Enriched"); break;
      case (ENRICHED_MIME_SIG):	Store("signatureType", "Enriched_If_Mime");
      				break;
   }

   Store("signatureFile",			orig.extPSigFile);
   Store("enrichedSignatureFile",		orig.extESigFile);
   Store("internalSignatureFile",		orig.intPSigFile);
   Store("internalEnrichedSignatureFile",	orig.intESigFile);

   if ( signature.size() > 0 ) Store("signature", signature);

   return True;

} // End WriteDatabase

/*---------------------------------------------------------------
 *  Method to write resources to a file
 */

Boolean
SigPrefC::WriteFile()
{
   StringListC	lineList;
   ReadResFile(lineList);

   Update(lineList, "appendSignature",		appendSig);
   Update(lineList, "addSignaturePrefix",	addPrefix);

   switch (type) {
      case (PLAIN_SIG):
	 Update(lineList, "signatureType",	"Plain");
	 break;
      case (ENRICHED_SIG):
	 Update(lineList, "signatureType",	"Enriched");
	 break;
      case (ENRICHED_MIME_SIG):
	 Update(lineList, "signatureType",	"Enriched_If_Mime");
	 break;
   }

   Update(lineList,"signatureFile",			orig.extPSigFile);
   Update(lineList,"enrichedSignatureFile",		orig.extESigFile);
   Update(lineList,"internalSignatureFile",		orig.intPSigFile);
   Update(lineList,"internalEnrichedSignatureFile",	orig.intESigFile);

   if ( signature.size() > 0 ) Update(lineList, "signature", signature);

//
// Write the information back to the file
//
   return WriteResFile(lineList);

} // End WriteFile

/*------------------------------------------------------------------------
 * Method to return signature.  Reads file each time in case it changes.
 */

StringC
SigPrefC::Sig(StringC& sigFile)
{
   StringC	sigStr("");

//
// is there a signature file ?
//
   struct stat  st;
   if ( sigFile.size() > 0 && stat(sigFile, &st) == 0 ) {

      Boolean	error = False;

//
// See if the file is executable
//
      Boolean	isExec = ((st.st_mode&S_IEXEC) != 0);
      if ( isExec ) {

//
// Make sure it is owned by this user
//
	 uid_t	uid = getuid();
	 if ( st.st_uid != uid ) {
	    StringC msg;
	    msg = "Your signature file: \"";
	    msg += sigFile;
	    msg += "\"\nis executable but is not owned by you.";
	    msg += "\nFor security reasons it will not be executed.";
	    halApp->PopupMessage(msg);
	    error = True;
	 }

//
// Make sure it is not world or group writable
//
	 Boolean	worldWrite = ((st.st_mode&S_IWOTH) != 0);
	 if ( !error && worldWrite ) {
	    StringC msg;
	    msg = "Your signature file: \"";
	    msg += sigFile;
	    msg = "\"\nis executable but is world writable.";
	    msg += "\nFor security reasons it will not be executed.";
	    halApp->PopupMessage(msg);
	    error = True;
	 }

	 Boolean	groupWrite = ((st.st_mode&S_IWGRP) != 0);
	 if ( !error && groupWrite ) {
	    StringC msg;
	    msg = "Your signature file: \"";
	    msg += sigFile;
	    msg = "\"\nis executable but is group writable.";
	    msg += "\nFor security reasons it will not be executed.";
	    halApp->PopupMessage(msg);
	    error = True;
	 }

//
// Create a temporary file to hold output from signature script
//
	 char	*tf = NULL;
	 if ( !error ) {

	    tf = tempnam(NULL, "sig.");
	    if ( !tf ) {
	       int	err = errno;
	       StringC errmsg;
	       errmsg = "I could not create a temp file to capture your signature output.\n";
	       errmsg += SystemErrorMessage(err);
	       halApp->PopupMessage(errmsg);
	       error = True;
	    }
	 }

//
// execute the signature file script
//
	 if ( !error ) {

	    StringC tmpFile(tf);
	    free(tf);
	    StringC	sigCommand = sigFile + " > " + tmpFile;
	    int rc = SafeSystem(sigCommand);
	    if ( rc != 0 ) {
	       StringC	errmsg("I could not execute your signature file: \"");
	       errmsg += sigFile;
	       errmsg += "\"\n";
	       StringC	output;
	       if ( output.ReadFile(tmpFile) && output.size() > 0 )
		  errmsg += output;
	       else
		  errmsg += SystemErrorMessage(rc);
	       halApp->PopupMessage(errmsg);
	       error = True;
	    }

	    else if ( !sigStr.ReadFile(tmpFile) ) {
	       int	err = errno;
	       StringC errmsg;
	       errmsg = "I could not read the output from your signature file: \"";
	       errmsg += sigFile;
	       errmsg += "\"\n";
	       errmsg += SystemErrorMessage(err);
	       halApp->PopupMessage(errmsg, XmDIALOG_WARNING);
	       error = True;
	    }

	    unlink(tmpFile);	// Remove the temp file
	 }

      } // End if signature file is executable

//
// If the file is not executable, try to read it
//
      else if ( !sigStr.ReadFile(sigFile) ) {
	 int	err = errno;
	 StringC errmsg;
	 errmsg = "I could not read your signature file: \"";
	 errmsg += sigFile;
	 errmsg += "\"\n";
	 errmsg += SystemErrorMessage(err);
	 halApp->PopupMessage(errmsg, XmDIALOG_WARNING);
      }

   } // End if signature file exists

//
// no signature file, so return signature string, if any
//
   else
      sigStr = signature;

   return sigStr;

} // End Sig

/*------------------------------------------------------------------------
 * Method to edit the preferences
 */

void
SigPrefC::Edit(Widget parent)
{
   halApp->BusyCursor(True);

   if ( !prefWin ) prefWin = new SigPrefWinC(parent);
   prefWin->Show(parent);

   halApp->BusyCursor(False);
}

/*------------------------------------------------------------------------
 * Method to set external plain signature file name
 */

void
SigPrefC::SetExtPSigFile(const char *name)
{
   if ( orig.extPSigFile == name ) return;

   extPSigFile = orig.extPSigFile = name;
   ShellExpand(extPSigFile);
}

/*------------------------------------------------------------------------
 * Method to set external enriched signature file name
 */

void
SigPrefC::SetExtESigFile(const char *name)
{
   if ( orig.extESigFile == name ) return;

   extESigFile = orig.extESigFile = name;
   ShellExpand(extESigFile);
}

/*------------------------------------------------------------------------
 * Method to set internal plain signature file name
 */

void
SigPrefC::SetIntPSigFile(const char *name)
{
   if ( orig.intPSigFile == name ) return;

   intPSigFile = orig.intPSigFile = name;
   ShellExpand(intPSigFile);
}

/*------------------------------------------------------------------------
 * Method to set internal enriched signature file name
 */

void
SigPrefC::SetIntESigFile(const char *name)
{
   if ( orig.intESigFile == name ) return;

   intESigFile = orig.intESigFile = name;
   ShellExpand(intESigFile);
}

