/*
 * $Id: Mailcap.C,v 1.4 2001/03/29 09:44:25 evgeny Exp $
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

#include "IshAppC.h"
#include "Mailcap.h"
#include "Misc.h"
#include "MsgPartC.h"
#include "MsgC.h"
#include "SafeSystem.h"
#include "ParamC.h"

#include <hgl/StrCase.h>
#include <hgl/CallbackC.h>
#include <hgl/RegexC.h>
#include <hgl/CharC.h>

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>	// For stat

MailcapFileC	*MailcapFiles   = NULL;
time_t		mailcapReadTime = 0;
static StringC	*mailcapPath    = NULL;
const char	*defMailcapPath =
	 "$HOME/.mailcap:/etc/mailcap:/usr/etc/mailcap:/usr/local/etc/mailcap";

extern int	debuglev;

/*---------------------------------------------------------------
 *  Method to return a pointer to the requested parameter
 */

static ParamC*
Param(CharC key, ParamC *list)
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

/*-----------------------------------------------------------------------
 *  Function to perform substitutions in mailcap commands
 */

StringC
BuildCommand(StringC cmdStr, CharC file, CharC enc, CharC type, ParamC *params,
	     CharC user, CharC pass)
{
//
// Make the following substitutions:
//
// %%       ==> %
// %s       ==> file name
// %e       ==> encoding string
// %t       ==> type string
// %{param} ==> parameter value
//
   static RegexC	*percPat = NULL;
   static RegexC	*parmPat = NULL;
   if ( !percPat ) percPat = new RegexC("%[%set{]");
   if ( !parmPat ) parmPat = new RegexC("%{\\([^}]+\\)}");
   Boolean	gotFile = False;

   int	pos = 0;
   while ( (pos=percPat->search(cmdStr, pos)) >= 0 ) {

      char	c = cmdStr[pos+1];
      RangeC	range = (*percPat)[0];

      switch (c) {

	 case '%':
	    cmdStr(range) = "%";
	    pos++;
	    break;

	 case 's':	// File name
	    if ( file.Length() > 0 ) {
	       cmdStr(range) = file;
	       pos += file.Length();
	       gotFile = True;
	    }
	    else
	       pos += range.length();

	    break;

	 case 'e':	// Encoding
	    if ( enc.Length() > 0 ) {
	       cmdStr(range) = enc;
	       pos += enc.Length();
	    }
	    else
	       pos += range.length();

	    break;

	 case 't':	// Type
	    if ( type.Length() > 0 ) {
	       cmdStr(range) = type;
	       pos += type.Length();
	    }
	    else
	       pos += range.length();

	    break;

	 case '{':

	    if ( parmPat->match(cmdStr, pos) ) {

//
// Look up the parameter
//
	       range = (*parmPat)[1];
	       CharC	key    = cmdStr(range);
	       ParamC	*param = Param(key, params);
	       StringC	val;

//
// If it wasn't found, check for the special cases of "ruser" and "rpwd"
//    used in the ftp command
//
	       if ( !param ) {

		  if ( user.Length() >0 && key.Equals("ruser", IGNORE_CASE) )
		     val = user;

		  else
		  if ( pass.Length()>0  && key.Equals("rpwd",  IGNORE_CASE) ) {

//
// Encode the password so it doesn't accidently show up in clear text. In
//    'tr' format, here is the encoding:
//
// tr '\050-\172' '\060-\172\050-\057'
//
		     val = pass;
		     char	*vp = val;
		     for (int i=0; i<val.size(); i++, vp++) {
			if ( *vp >= '\050' && *vp <= '\172' ) {
			   *vp += '\010';
			   if ( *vp > '\172' ) *vp = '\050' + (*vp - '\173');
			}
		     }

		  } // End if need password

	       } // End if parameter not found

	       else
		  val = param->val;

//
// Substitute the parameter value
//
	       //if ( val.size() > 0 ) {
		  range = (*parmPat)[0];
		  cmdStr(range) = val;
		  pos += val.size();
	       //}
	       //else
		  //pos += range.length();

	    } // End if parameter pattern found

	    break;

      } // End switch character

   } // End for each percent pattern

//
// If we didn't add the file name, we have to pipe it through stdin.
//
   if ( !gotFile ) {
      StringC	pipeStr = "cat ";
      pipeStr += file;
      pipeStr += " | ";
      cmdStr = pipeStr + cmdStr;
   }

   return cmdStr;

} // End BuildCommand

/*-----------------------------------------------------------------------
 *  Function to perform substitutions in mailcap commands
 */

StringC
BuildCommand(StringC cmdStr, MsgPartC *part, CharC file)
{
//
// Make the following substitutions:
//
// %%       ==> %
// %s       ==> file name
// %e       ==> encoding string
// %t       ==> type string
// %{param} ==> parameter value
//
   static RegexC	*percPat = NULL;
   static RegexC	*parmPat = NULL;
   if ( !percPat ) percPat = new RegexC("%[%set{]");
   if ( !parmPat ) parmPat = new RegexC("%{\\([^}]+\\)}");
   Boolean	gotFile = False;

   int	pos = 0;
   while ( (pos=percPat->search(cmdStr, pos)) >= 0 ) {

      char	c = cmdStr[pos+1];
      RangeC	range = (*percPat)[0];

      switch (c) {

	 case '%':
	    cmdStr(range) = "%";
	    pos++;
	    break;

	 case 's':	// File name
	    if ( file.Length() > 0 ) {
	       cmdStr(range) = file;
	       pos += file.Length();
	       gotFile = True;
	    }
	    else
	       pos += range.length();

	    break;

	 case 'e':	// Encoding
	 {
	    CharC	enc = EncodingTypeStr(part->encType);
	    if ( enc.Length() > 0 ) {
	       cmdStr(range) = enc;
	       pos += enc.Length();
	    }
	    else
	       pos += range.length();

	 } break;

	 case 't':	// Type
	 {
	    CharC	type(part->conStr);
	    if ( type.Length() > 0 ) {
	       cmdStr(range) = type;
	       pos += type.Length();
	    }
	    else
	       pos += range.length();

	 } break;

	 case '{':

	    if ( parmPat->match(cmdStr, pos) ) {

//
// Look up the parameter
//
	       range = (*parmPat)[1];
	       CharC	key    = cmdStr(range);
	       ParamC	*param = part->Param(key);
	       StringC	val;

//
// If it wasn't found, check for the special cases of "ruser" and "rpwd"
//    used in the ftp command
//
	       if ( !param ) {

		  if ( part->userName.size() > 0 &&
		       key.Equals("ruser", IGNORE_CASE) )
		     val = part->userName;

		  else
		  if ( part->userPass.size() > 0 &&
		       key.Equals("rpwd",  IGNORE_CASE) ) {

//
// Encode the password so it doesn't accidently show up in clear text. In
//    'tr' format, here is the encoding:
//
// tr '\050-\172' '\060-\172\050-\057'
//
		     val = part->userPass;
		     char	*vp = val;
		     for (int i=0; i<val.size(); i++, vp++) {
			if ( *vp >= '\050' && *vp <= '\172' ) {
			   *vp += '\010';
			   if ( *vp > '\172' ) *vp = '\050' + (*vp - '\173');
			}
		     }

		  } // End if need password

	       } // End if parameter not found

	       else
		  val = param->val;

//
// Substitute the parameter value
//
	       //if ( val.size() > 0 ) {
		  range = (*parmPat)[0];
		  cmdStr(range) = val;
		  pos += val.size();
	       //}
	       //else
		  //pos += range.length();

	    } // End if parameter pattern found

	    break;

      } // End switch character

   } // End for each percent pattern

//
// If we didn't add the file name, we have to pipe it through stdin.
//
   if ( !gotFile && file.Length() > 0 ) {
      StringC	pipeStr = "cat ";
      pipeStr += file;
      pipeStr += " | ";
      cmdStr = pipeStr + cmdStr;
   }

   return cmdStr;

} // End BuildCommand

/*-----------------------------------------------------------------------
 *  Method to run mailcap test command
 */

static int
TestMailcap(MailcapC *mc, MsgPartC *part)
{
   if ( mc->test.size() == 0 ) return True;

//
// See if we need a data file to pass to the command
//
   char		*file = NULL;
   StringC	cmd;

   if ( part ) {

//
// See if there's a %s and a data file
//
      if ( mc->test.Contains("%s") && part->CreateDataFile() )
	 file = part->dataFile;

      cmd = BuildCommand(mc->test, part, file);

   } // End if there was a body part passed in

   else {

      StringC	type = mc->conType;
      if ( mc->subType != "*" ) {
	 type += "/";
	 type += mc->subType;
      }

      cmd = BuildCommand(mc->test, NULL, NULL, type, NULL, NULL, NULL);

   } // End if there is no body part

//
// Run the command
//
   int	status = SafeSystem(cmd);
   if ( debuglev > 0 ) cout <<cmd <<" returns " <<status <<endl;

   return (status == 0);

} // TestMailcap

/*-----------------------------------------------------------------------
 *  Function to re-read mailcap files if necessary
 */

static void
UpdateMailcapFiles()
{
//
// See if any files have changed since we last read them
//
   u_int	offset = 0;
   CharC	file = mailcapPath->NextWord(offset, ':');
   while ( file.Length() > 0 ) {

//
// If a file has changed, re-read them all.
//
      StringC		tmp(file);
      struct stat	stats;
      if ( stat(tmp, &stats) == 0 && stats.st_mtime > mailcapReadTime ) {
	 ReadMailcap();
	 return;
      }

      offset = file.Addr() - (char*)(*mailcapPath) + file.Length();
      file = mailcapPath->NextWord(offset, ':');
   }

} // End UpdateMailcapFiles

/*-----------------------------------------------------------------------
 *  Functions to lookup mailcap entry
 */

MailcapC*
MailcapEntry(MsgPartC *part)
{
//
// Re-read mailcap files if necessary
//
   UpdateMailcapFiles();

//
// Look for matching type
//
   MailcapFileC	*file = MailcapFiles;
   while ( file ) {

//
// Loop through entries in this file
//
      u_int	count = file->entryList.size();
      for (int i=0; i<count; i++) {

	 MailcapC	*entry = file->entryList[i];

//
// See if type matches
//
	 if ( entry->conType.Equals(part->grpStr, IGNORE_CASE) ||
	      entry->conType == "*" ) {

//
// See if subtype matches
//
	    if ( entry->subType.Equals(part->subStr, IGNORE_CASE) ||
	         entry->subType == "*" ) {

//
// See if entry passes test
//
	       if ( TestMailcap(entry, part) )
		  return entry;

	    } // End if subtype matches

	 } // End if type matches

      } // End for each mailcap entry

      file = file->next;

   } // End for each mailcap file

   return NULL;

} // End MailcapEntry

MailcapC*
MailcapEntry(CharC con, CharC sub, MsgPartC *part)
{
//
// Re-read mailcap files if necessary
//
   UpdateMailcapFiles();

//
// Look for matching type
//
   MailcapFileC	*file = MailcapFiles;
   while ( file ) {

//
// Loop through entries in this file
//
      u_int	count = file->entryList.size();
      for (int i=0; i<count; i++) {

	 MailcapC	*entry = file->entryList[i];

//
// See if type matches
//
	 if ( entry->conType.Equals(con, IGNORE_CASE) ||
	      entry->conType == "*" ) {

//
// See if subtype matches
//
	    if ( entry->subType.Equals(sub, IGNORE_CASE) ||
	         entry->subType == "*" ) {

//
// See if entry passes test
//
	       if ( TestMailcap(entry, part) )
		  return entry;

	    } // End if subtype matches

	 } // End if type matches

      } // End for each mailcap entry

      file = file->next;

   } // End for each mailcap file

   return NULL;

} // End MailcapEntry

/*---------------------------------------------------------------------
 * Function to read the next real line from a mailcap file
 */

static Boolean
GetNextLine(CharC data, u_int *offset, StringC *line)
{
   line->Clear();

//
// Look for next non-commented line
//
   CharC	segment = data.NextWord(*offset, '\n');
   *offset = segment.Addr() - data.Addr() + segment.Length();

   while ( segment[0] == '#' ) {
      segment = data.NextWord(*offset, '\n');
      *offset = segment.Addr() - data.Addr() + segment.Length();
   }

   if ( segment.Length() == 0 ) return False;

//
// Remove escaped newlines
//
   *line = segment;
   line->Replace("\\\n", "");
   return True;

} // End GetNextLine

/*---------------------------------------------------------------------
 * Function to read a mailcap file and add its entries
 */

static void
ProcessMailcapFile(StringC file)
{
   if ( debuglev > 0 ) cout <<"Reading mailcap file: " <<file <<endl;

   StringC	data;
   if ( !data.ReadFile(file) ) return;

//
// Create a new file object
//
   MailcapFileC	*mcfile = new MailcapFileC(file);
   if ( MailcapFiles ) {
      MailcapFileC	*fileEntry = MailcapFiles;
      while ( fileEntry->next ) fileEntry = fileEntry->next;
      fileEntry->next = mcfile;
   }
   else
      MailcapFiles = mcfile;

   StringC	line;
   u_int	offset = 0;
   while ( GetNextLine(data, &offset, &line) ) {

      if ( debuglev > 0 ) cout <<"Found mailcap entry: " <<line <<endl;

//
// Create a new data structure
//
      MailcapC	*mcap = new MailcapC(line);
      mcfile->entryList.add(mcap);

   } // End for each mailcap entry

//
// Sort the entries in this file
//
   mcfile->Sort();

   if ( debuglev > 1 ) {
      cout <<"Sorted entries:" <<endl;
      u_int	count = mcfile->entryList.size();
      for (int i=0; i<count; i++) {
	 MailcapC	*entry = mcfile->entryList[i];
	 cout <<"\t" <<entry->fullType <<endl;
      }
   }

} // End ProcessMailcapFile

/*---------------------------------------------------------------------
 * Function to read mailcap files and build tree
 */

int
ReadMailcap()
{
   if ( !mailcapPath ) mailcapPath = new StringC;

   char		*cs = getenv("MAILCAPS");
   if ( cs ) {
      *mailcapPath = cs;
   } else {
      *mailcapPath = ishHome + "/lib/mailcap:" + defMailcapPath;
      mailcapPath->Replace("$HOME", ishApp->home);
   }

//
// Delete existing entries
//
   MailcapFileC	*mcfile = MailcapFiles;
   while ( mcfile ) {
      MailcapFileC	*next = mcfile->next;
      delete mcfile;
      mcfile = next;
   }
   MailcapFiles = NULL;

//
// Loop through files and process them
//
   u_int	offset = 0;
   CharC	file = mailcapPath->NextWord(offset, ':');
   StringC	data;
   while ( file.Length() > 0 ) {

      ProcessMailcapFile(file);

      offset = file.Addr() - (char*)(*mailcapPath) + file.Length();
      file = mailcapPath->NextWord(offset, ':');
   }

   mailcapReadTime = time(0);

   return 0;

} // End ReadMailcap

/*---------------------------------------------------------------------
 * Initialize mailcap stuff
 */

int
InitMailcap()
{
   MailcapFileC	*fileEntry = MailcapFiles;
   while ( fileEntry ) {
      MailcapFileC	*nextEntry = fileEntry->next;
      delete fileEntry;
      fileEntry = nextEntry;
   }
   MailcapFiles = NULL;

   return ReadMailcap();
}

/*---------------------------------------------------------------------
 * Constructor for mailcap file object
 */

MailcapFileC::MailcapFileC(const char *file)
{
   name = file;
   next = NULL;

   entryList.AllowDuplicates(FALSE);
   entryList.SetSorted(FALSE);
}

/*---------------------------------------------------------------------
 * Destructor for mailcap file object
 */

MailcapFileC::~MailcapFileC()
{
   u_int	count = entryList.size();
   for (int i=0; i<count; i++) {
      MailcapC	*entry = entryList[i];
      delete entry;
   }
}

/*---------------------------------------------------------------------
 * Method to sort mailcap entries
 */

static int
CompareEntries(const void *a, const void *b)
{
#define A_GREATER	 1
#define AB_EQUAL	 0
#define B_GREATER	-1

   MailcapC	*ma = *(MailcapC **)a;
   MailcapC	*mb = *(MailcapC **)b;
   int		result = AB_EQUAL;

//
// Check the content types
//
   if ( ma->conType.Equals("*") && !mb->conType.Equals("*") )
      result = A_GREATER;

   else if ( mb->conType.Equals("*") && !ma->conType.Equals("*") )
      result = B_GREATER;

   else if ( ma->conType == mb->conType ) {

//
// Check the sub types
//
      if ( ma->subType.Equals("*") && !mb->subType.Equals("*") )
	 result = A_GREATER;

      else if ( mb->subType.Equals("*") && !ma->subType.Equals("*") )
	 result = B_GREATER;

      else if ( ma->subType == mb->subType )
	 result = AB_EQUAL;

      else
	 result = ma->subType.compare(mb->subType);

   } // End if content types are equal

   else
      result = ma->conType.compare(mb->conType);

   return result;

} // End CompareEntries

void
MailcapFileC::Sort()
{
   entryList.sort(CompareEntries);
}

