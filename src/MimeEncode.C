/*
 *  $Id: MimeEncode.C,v 1.2 2000/05/07 12:26:12 fnevgeny Exp $
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

#include "MimeEncode.h"
#include "SafeSystem.h"
#include "FileMisc.h"

#include <hgl/StringC.h>
#include <hgl/CharC.h>

#include <unistd.h>

/*-----------------------------------------------------------------------
 * Function to uuencode a file and store the output in a second file
 */

Boolean
FileToFileUU(const char *ifile, const char *ofile, FILE *ofp)
{
   StringC	dst;
   if ( ofp ) {
      char	*cs = tempnam(NULL, "uu.");
      dst = cs;
      free(cs);
   }
   else
      dst = ofile;

   StringC	cmdStr("uuencode ");
   cmdStr += ifile;
   cmdStr += " ";
   cmdStr += BaseName(ifile);
   cmdStr += " >> ";
   cmdStr += dst;

   int	status = SafeSystem(cmdStr);
   if ( status != 0 ) return False;

   if ( ofp )
      return CopyFile(dst, (char*)ofile, False, False, NULL, ofp);

   return True;

} // End FileToFileUU

/*-----------------------------------------------------------------------
 * Function to uuencode a file and store the output in a text string
 */

Boolean
FileToTextUU(const char *ifile, StringC *dst)
{
//
// Encode to a file and read the file
//
   char	*ofile = tempnam(NULL, "uu.");

   Boolean success = (FileToFileUU(ifile, ofile) && dst->AppendFile(ofile));

   unlink(ofile);
   free(ofile);

   return success;

} // End FileToTextUU

/*-----------------------------------------------------------------------
 * Function to uuencode a text string and store the output in a file
 */

Boolean
TextToFileUU(CharC src, const char *ofile)
{
//
// Write to a file and encode the file
//
   char	*ifile = tempnam(NULL, "uu.");

   Boolean success = (src.WriteFile(ifile) && FileToFileUU(ifile, ofile));

   unlink(ifile);
   free(ifile);

   return success;

} // End TextToFileUU

/*-----------------------------------------------------------------------
 * Function to uuencode a text string and store the output in a text string
 */

Boolean
TextToTextUU(CharC src, StringC *dst)
{
//
// Write to a file, encode it, then read it back.
//
   char	*ifile = tempnam(NULL, "uu.");
   char	*ofile = tempnam(NULL, "uu.");

   Boolean success = (src.WriteFile(ifile) && FileToFileUU(ifile, ofile) &&
		      dst->AppendFile(ofile));

   unlink(ifile);
   unlink(ofile);
   free(ifile);
   free(ofile);

   return success;

} // End TextToTextUU

/*-----------------------------------------------------------------------
 * Function to uudecode a file and store the output in a second file
 */

Boolean
FileUUToFile(const char *ifile, const char *ofile)
{
//
// Map the file and try to read the output name from the begin line:
//
// "begin mode name"
//
   StringC	cmdStr("uudecode ");
   cmdStr += ifile;
   int	status = SafeSystem(cmdStr);
   return (status == 0);

} // End FileUUToFile

/*-----------------------------------------------------------------------
 * Function to uudecode a file and store the output in a text string
 */

Boolean
FileUUToText(const char *ifile, StringC *dst)
{
   return dst->AppendFile((char*)ifile);

} // End FileUUToText

/*-----------------------------------------------------------------------
 * Function to uudecode a text string and store the output in a file
 */

Boolean
TextUUToFile(CharC src, const char *ofile)
{
   return src.WriteFile((char*)ofile);

} // End TextUUToFile

/*-----------------------------------------------------------------------
 * Function to uudecode a text string and store the output in a text string
 */

Boolean
TextUUToText(CharC src, StringC *dst)
{
   *dst = src;
   return True;

} // End TextUUToText

/*-----------------------------------------------------------------------
 * Function to binhex encode a file and store the output in a second file
 */

Boolean
FileToFileBH(const char *ifile, const char *ofile, FILE *ofp)
{
   StringC	dst;
   if ( ofp ) {
      char	*cs = tempnam(NULL, "bh.");
      dst = cs;
      free(cs);
   }
   else
      dst = ofile;

   StringC	cmdStr("cat ");
   cmdStr += ifile;
   cmdStr += " >> ";
   cmdStr += dst;

   int	status = SafeSystem(cmdStr);
   if ( status != 0 ) return False;

   if ( ofp )
      return CopyFile(dst, (char*)ofile, False, False, NULL, ofp);

   return True;

} // End FileToFileBH

/*-----------------------------------------------------------------------
 * Function to binhex encode a file and store the output in a text string
 */

Boolean
FileToTextBH(const char *ifile, StringC *dst)
{
   return dst->AppendFile((char*)ifile);

} // End FileToTextBH

/*-----------------------------------------------------------------------
 * Function to binhex encode a text string and store the output in a file
 */

Boolean
TextToFileBH(CharC src, const char *ofile)
{
   return src.WriteFile((char*)ofile);

} // End TextToFileBH

/*-----------------------------------------------------------------------
 * Function to binhex encode a text string and store the output in a text string
 */

Boolean
TextToTextBH(CharC src, StringC *dst)
{
   *dst = src;
   return True;

} // End TextToTextBH

/*-----------------------------------------------------------------------
 * Function to binhex decode a file and store the output in a second file
 */

Boolean
FileBHToFile(const char *ifile, const char *ofile)
{
   StringC	cmdStr("cat ");
   cmdStr += ifile;
   cmdStr += " > ";
   cmdStr += ofile;
   int	status = SafeSystem(cmdStr);
   return (status == 0);

} // End FileBHToFile

/*-----------------------------------------------------------------------
 * Function to binhex decode a file and store the output in a text string
 */

Boolean
FileBHToText(const char *ifile, StringC *dst)
{
   return dst->AppendFile((char*)ifile);

} // End FileBHToText

/*-----------------------------------------------------------------------
 * Function to binhex decode a text string and store the output in a file
 */

Boolean
TextBHToFile(CharC src, const char *ofile)
{
   return src.WriteFile((char*)ofile);

} // End TextBHToFile

/*-----------------------------------------------------------------------
 * Function to binhex decode a text string and store the output in a text string
 */

Boolean
TextBHToText(CharC src, StringC *dst)
{
   *dst = src;
   return True;

} // End TextBHToText
