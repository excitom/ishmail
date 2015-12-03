/*
 *  $Id: FileMisc.C,v 1.4 2000/05/07 12:26:11 fnevgeny Exp $
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
#include "AppPrefC.h"
#include "Misc.h"
#include "UnixFolderC.h"
#include "FolderPrefC.h"

#include <hgl/CharC.h>
#include <hgl/StringC.h>
#include <hgl/SysErr.h>
#include <hgl/MemMap.h>
#include <hgl/StringDictC.h>
#include <hgl/RegexC.h>

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <time.h>

extern int	debuglev;

/*-----------------------------------------------------------------------
 *  Function to return the last component of a pathname
 */

CharC
BaseName(CharC path)
{
   u_int	offset = 0;
   if ( path.EndsWith('/') ) offset++;

   int	pos = path.Length() - 1 - offset;
   pos = path.RevPosOf('/', (u_int)pos);
   if ( pos >= 0 )
      return path(pos+1, path.Length() - pos - offset - 1);
   else
      return path(0, path.Length() - offset);

} // End BaseName

/*-----------------------------------------------------------------------
 *  Function to return the directory component of a pathname
 */

CharC
DirName(CharC path)
{
   int	pos = path.RevPosOf('/');
   if ( pos >= 0 ) return path(0, pos);
   else		   return ".";
}

/*------------------------------------------------------------------------
 * Function to return the full pathname for a file
 */

StringC
FullPathname(StringC string)
{
   int	asize = ishApp->appPrefs->AutomountRoot().size();

   char	result[MAXPATHLEN+1];
   if ( realpath(string, result) == NULL )
      return string;

   CharC	res(result);
   if ( asize > 0 && res.StartsWith(ishApp->appPrefs->AutomountRoot()) )
      return &result[asize];
   else
      return result;
}

/*---------------------------------------------------------------
 *  Function to determine if the given name is a directory.
 */

Boolean
IsDir(char* name)
{
   struct stat  st;
   return (stat(name, &st) == 0 && S_ISDIR(st.st_mode));
}

/*------------------------------------------------------------------------
 * Function to create the requested directory, plus any directories leading
 *    up to it.
 */

Boolean
MakeDir(StringC path)
{
   if ( IsDir(path) ) return True;

//
// If the parent directory does not exist, try to create it.  Then try to
//    create the directory.
//
   StringC	dir = DirName(path);
   Boolean	parentExists = (access(dir, F_OK) == 0);
   if ( !parentExists ) {
      if ( !MakeDir(dir) ) return False;
   }

   if ( debuglev > 0 ) cout <<"Creating directory: " <<path <<endl;
   if ( mkdir(path, 0700) != 0 ) {
      StringC	errmsg;
      int	err = errno;
      errmsg = "Could not create directory: ";
      errmsg += path;
      errmsg += ".\n" + SystemErrorMessage(err);
      halApp->PopupMessage(errmsg);
      return False;
   }

   return True;

} // MakeDir

/*------------------------------------------------------------------------
 * Function to removed the requested directory and its descendents
 */

Boolean
RemoveDir(StringC path)
{
//
// Open the directory
//
   DIR	*dirp = opendir(path);
   if ( !dirp ) {
      int	err = errno;
      StringC	errmsg = "Could not open directory: ";
      errmsg += path;
      errmsg += ":\n\n";
      errmsg += SystemErrorMessage(err);
      halApp->PopupMessage(errmsg);
      return False;
   }

//
// Read the directory entries
//
   struct dirent	*dp;
   StringC		child;
   while ( (dp=readdir(dirp)) != NULL ) {

      if ( strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0 )
	 continue;

      child = path;
      child += "/";
      child += dp->d_name;

      if ( IsDir(child) ) {
	 if ( !RemoveDir(child) ) return False;
      }

      else if ( unlink(child) != 0 ) {
	 int	err = errno;
	 StringC	errmsg = "Could not remove file: ";
	 errmsg += child;
	 errmsg += ":\n\n";
	 errmsg += SystemErrorMessage(err);
	 halApp->PopupMessage(errmsg);
	 return False;
      }

   } // End for each directory entry

   closedir(dirp);

//
// Finally, delete this (now empty) directory
//
   if ( rmdir(path) != 0 ) {
      int	err = errno;
      StringC	errmsg = "Could not remove directory: ";
      errmsg += path;
      errmsg += ":\n\n";
      errmsg += SystemErrorMessage(err);
      halApp->PopupMessage(errmsg);
      return False;
   }

   return True;

} // End RemoveDir

/*------------------------------------------------------------------------
 * Function to create the requested file, plus any directories leading
 *    up to it.
 */

Boolean
MakeFile(StringC path)
{
   if ( access(path, F_OK) == 0 ) return True;

//
// Create the directory, then the file.
//
   StringC	dir = DirName(path);
   if ( !MakeDir(dir) ) return False;

   if ( debuglev > 0 ) cout <<"Creating file: " <<path <<endl;
   int	fd = open(path, O_CREAT|O_TRUNC, ishApp->folderPrefs->folderFileMask);
   if ( fd > 0 )
      close(fd);

   else {
      StringC	errmsg;
      int	err = errno;
      errmsg = "Could not create file: ";
      errmsg += path;
      errmsg += ".\n" + SystemErrorMessage(err);
      halApp->PopupMessage(errmsg);
      return False;
   }

   return True;

} // End MakeFile

/*---------------------------------------------------------------------
 * Function to read the next real line from a mime types file
 */

static Boolean
GetNextLine(CharC data, u_int *offset, StringC *line)
{
   line->Clear();

   CharC	segment = data.NextWord(*offset, '\n');
   while ( segment.Length() > 0 ) {

//
// Compute next offset
//
      *offset = segment.Addr() - data.Addr() + segment.Length();

//
// Ignore comments
//
      if ( segment[0] != '#' ) {

//
// Check for continuation
//
	 Boolean cont = segment.EndsWith('\\');
	 if ( cont ) segment.CutEnd(1);

	 *line += segment;

//
// If this line doesn't have a continuation at the end, we're done
//
	 if ( !cont ) return True;

      } // End if we got a line

//
// Look for next line segment
//
      segment = data.NextWord(*offset, '\n');

   } // End for each line segment

   return False;

} // End GetNextLine

/*---------------------------------------------------------------
 *  Function to build a dictionary of suffixes->mime-types from the
 *     mime.types file
 */

static StringDictC	*mimeTypeDict    = NULL;
static time_t		mimeTypeReadTime = 0;

static void
UpdateMimeTypes()
{
//
// Create the dictionary if necessary
//
   if ( !mimeTypeDict ) mimeTypeDict = new StringDictC;

//
// Get the name of the mime types file
//
   char	*cs = getenv("MIMETYPES");
   StringC	file;
   if ( cs ) {
      file = cs;
   } else {
      file = ishHome + "/lib/" + "mime.types";
   }

//
// Get the time on the mime types file
//
   struct stat	stats;
   if ( stat(file, &stats ) != 0 ) return;

   if ( stats.st_mtime <= mimeTypeReadTime ) return;

//
// The file needs to be read
//
   MappedFileC	*mf = MapFile(file);
   if ( !mf ) return;

   mimeTypeDict->removeAll();

   StringC	line;
   StringC	type;
   StringC	suffix;
   u_int	offset = 0;
   while ( GetNextLine(mf->data, &offset, &line) ) {
	     
      if ( debuglev > 1 ) cout <<"Found mime-types entry: " <<line <<endl;

//
// Loop through tokens
//
      type.Clear();
      u_int	loff  = 0;
      CharC	token = line.NextWord(loff);
      while ( token.Length() > 0 ) {

         loff = token.Addr() - (char*)line + token.Length();
	 token.Trim();

//
// The first token is the type
//
	 if ( type.size() == 0 )
	    type = token;

//
// Each additional token is a suffix
//
	 else {
	    suffix = token;
	    mimeTypeDict->add(token, type);
	 }

	 token = line.NextWord(loff);

      } // End for each token

   } // End for each entry

   UnmapFile(mf);
   mimeTypeReadTime = time(0);

} // End UpdateMimeTypes

/*---------------------------------------------------------------
 *  Method to return reference to type dictionary
 */

StringDictC&
MimeTypeDict()
{
   UpdateMimeTypes();
   return *mimeTypeDict;
}

/*---------------------------------------------------------------
 *  Function to return the best guess about the type of a file
 */

void
GetFileType(CharC name, StringC& type)
{
   type.Clear();

//
// See if there is a suffix
//
   int	pos = name.RevPosOf('.');
   if ( pos >= 0 ) {

      name.CutBeg(pos+1);

      if ( name.Equals("gif", IGNORE_CASE) )
	 type = "image/gif";

      else if ( name.Equals("jpg",  IGNORE_CASE) ||
		name.Equals("jpeg", IGNORE_CASE) )
	 type = "image/jpeg";

      else if ( name.Equals("au", IGNORE_CASE) )
	 type = "audio/basic";

      else if ( name.Equals("mpg",  IGNORE_CASE) ||
		name.Equals("mpeg", IGNORE_CASE) )
	 type = "video/mpeg";

      else if ( name.Equals("ps", IGNORE_CASE) )
	 type = "application/postscript";

      else if ( name.Equals("o") || name.Equals("a") )
	 type = "application/octet-stream";

      else if ( name.Equals("c",  IGNORE_CASE) ||
	 	name.Equals("cc", IGNORE_CASE) ||
	 	name.Equals("h",  IGNORE_CASE) )
	 type = "text/plain";

//
// If we don't have a standard suffix, try the mime.types file
//
      else {

	 UpdateMimeTypes();

	 StringC	suffix = name;
	 StringC	*val = mimeTypeDict->definitionOf(suffix);
	 if ( val ) type = *val;
      }

   } // End if the name contains a suffix

   if ( type.size() > 0 ) return;

//
// If we get to this point, the name didn't have a suffix or we didn't
//    recognize it
//
   StringC	tmpStr = name;
   MappedFileC	*mf = MapFile(tmpStr);
   if ( mf ) {

      if ( Contains8Bit(mf->data) ) {
	 type = "application/octet-stream";
      }

      else if ( mf->data.StartsWith("From ") ||
		mf->data.Contains("\nTo:",      IGNORE_CASE) ||
		mf->data.Contains("\nFrom:",    IGNORE_CASE) ||
		mf->data.Contains("\nSubject:", IGNORE_CASE) ) {
	 type = "message/rfc822";
      }

      else if ( mf->data.Contains("<nl>", IGNORE_CASE) ||
		mf->data.Contains("<lt>", IGNORE_CASE) ) {
	 type = "text/richtext";
      }

      else if ( IsEnriched(mf->data) ) {
	 type = "text/enriched";
      }

      UnmapFile(mf);

   } // End if file mapped

   if ( type.size() == 0 ) type = "text/plain";

} // End GetFileType

/*---------------------------------------------------------------
 *  Function to return the first suffix found for a given type
 */

void
GetTypeSuffix(CharC type, StringC& suffix)
{
   suffix.Clear();

   UpdateMimeTypes();

   u_int	count = mimeTypeDict->size();
   for (int i=0; i<count; i++) {

      StringC&	val = mimeTypeDict->valOf(i);
      if ( val.Equals(type, IGNORE_CASE) ) {
	 suffix = mimeTypeDict->keyOf(i);
	 return;
      }
   }

} // End GetTypeSuffix

/*---------------------------------------------------------------
 *  Method to write a list of message headers to a file
 */

Boolean
WriteHeaders(StringListC& headList, char *dstFile, Boolean addBlank,
	     FILE *dstFp)
{
   Boolean	closeFile = (dstFp == NULL);

//
// Create the header file
//
   if ( !dstFp ) {

      dstFp = fopen(dstFile, "a");
      if ( !dstFp ) {
	 StringC	errmsg("Could not create file \"");
	 errmsg += dstFile;
	 errmsg += "\".\n";
	 errmsg += SystemErrorMessage(errno);
	 halApp->PopupMessage(errmsg);
	 return False;
      }
   }

//
// Loop through the list
//
   u_int	count = headList.size();
   for (int i=0; i<count; i++) {

      StringC	*headStr = headList[i];
      if ( !headStr->WriteFile(dstFp) ) {
	 StringC	errmsg("Could not write to file \"");
	 errmsg += dstFile;
	 errmsg += "\".\n";
	 errmsg += SystemErrorMessage(errno);
	 halApp->PopupMessage(errmsg);
	 if ( closeFile ) fclose(dstFp);
	 return False;
      }
   }

//
// Add blank line if necessary
//
   if ( addBlank ) {
      CharC	nl("\n");
      nl.WriteFile(dstFp);
   }

   if ( closeFile ) fclose(dstFp);
   return True;

} // End WriteHeaders

/*---------------------------------------------------------------
 *  Method to copy text from one file to another
 */

Boolean
CopyFile(char *srcFile, char *dstFile, Boolean addBlank, Boolean protectFroms,
	  FILE *srcfp, FILE *dstfp)
{
   Boolean	closeSrc = (srcfp == NULL);
   Boolean	closeDst = (dstfp == NULL);

   if ( !srcfp ) {

//
// Open input file
//
      srcfp = fopen(srcFile, "r");
      if ( !srcfp ) {
	 StringC	errmsg("Could not open file \"");
	 errmsg += srcFile;
	 errmsg += "\"\n";
	 errmsg += SystemErrorMessage(errno);
	 halApp->PopupMessage(errmsg);
	 return False;
      }

   } // End if input file is not open

   if ( !dstfp ) {

//
// Create an output file
//
      dstfp = fopen(dstFile, "a+");
      if ( !dstfp ) {
	 StringC	errmsg("Could not open file \"");
	 errmsg += dstFile;
	 errmsg += "\"\n";
	 errmsg += SystemErrorMessage(errno);
	 halApp->PopupMessage(errmsg);
	 if ( closeSrc ) fclose(srcfp);
	 return False;
      }

   } // End if output file is not open

#define BUFLEN	1024

   char	buffer[BUFLEN];
   buffer[0] = 0;

   Boolean	error = False;
   CharC	bufStr;
   CharC	gtStr(">");

//
// See if we have to worry about Froms
//
   if ( protectFroms ) {

      RegexC	*fromPat = UnixFolderC::fromPat;

      while ( !error && fgets(buffer, BUFLEN-1, srcfp) ) {

//
// See if it needs protecting
//
	 bufStr = buffer;
	 if ( bufStr.StartsWith("From ") &&
	      (!fromPat || fromPat->match(bufStr)) )
	    error = !gtStr.WriteFile(dstfp);

//
// Write bytes to destination file
//
	 if ( !error ) error = !bufStr.WriteFile(dstfp);

      } // End while bytes remain to be copied

   } // End Froms need to be protected

//
// If we don't have any From issues, we can just copy the text as-is
//
   else {

//
// Read bytes from source file
//
      int	count = fread(buffer, 1, BUFLEN-1, srcfp);
      while ( !error && count > 0 ) {

//
// Write bytes to destination file
//
	 buffer[count] = 0;
	 bufStr = buffer;
	 error = !bufStr.WriteFile(dstfp);

//
// Read another block
//
	 if ( !error ) count = fread(buffer, 1, BUFLEN-1, srcfp);

      } // End while more input

   } // End if we're not concerned with Froms

//
// See if we need to add a blank line
//
   if ( !error && addBlank ) {
      CharC	nlStr("\n");
      error = !nlStr.WriteFile(dstfp);
   }

   if ( dstFile && error ) {
      StringC	errmsg("Could not write file \"");
      errmsg += dstFile;
      errmsg += "\"\n";
      errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
   }

   if ( !error ) error = (fflush(dstfp) != 0);
   if ( closeSrc ) fclose(srcfp);
   if ( closeDst ) fclose(dstfp);

   return !error;

} // End CopyFile

