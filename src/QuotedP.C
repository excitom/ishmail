/*
 *  $Id: QuotedP.C,v 1.3 2000/05/31 15:26:56 evgeny Exp $
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
#include "QuotedP.h"

#include <hgl/StringC.h>
#include <hgl/CharC.h>
#include <hgl/SysErr.h>
#include <hgl/HalAppC.h>

#include <unistd.h>
#include <errno.h>

/*-------------------------------------------------------------------------
 * Lookup table for QP encoding
 */

static char	basis_qp[] = "0123456789ABCDEF";

/*-----------------------------------------------------------------------
 * Encode any characters (< 32) or (> 127) and the '=' character.
 *    Don't encode newline or tab.
 *    Also encode any period on a line by itself.
 */

inline Boolean
NeedsQP(u_int c, int lineSize)
{
   return ( (c < 32 && (c != '\n' && c != '\t')) ||
	    (c == '=') ||
	    (c >= 127) ||
	    (lineSize == 0 && c == '.') );
}

/*-----------------------------------------------------------------------
 * Encode the character and write the output to the specified file
 */

inline Boolean
EncodeQP(u_int c, FILE *fp)
{
   return ( putc('=',               fp) != EOF &&
	    putc(basis_qp[c >> 4 ], fp) != EOF &&
	    putc(basis_qp[c & 0xf], fp) != EOF );
}

/*-----------------------------------------------------------------------
 * Write a properly encoded newline character to the specified file
 */

inline Boolean
EncodeNewlineQP(u_int prevc, FILE *fp)
{
   Boolean	error = False;

   if ( prevc == ' ' || prevc == '\t' )
      error = (putc('=', fp) == EOF || putc('\n', fp) == EOF);

   if ( !error ) error = (putc('\n', fp) == EOF);

   return !error;
}

/*-----------------------------------------------------------------------
 * Encode the character and add the output to the specified string
 */

inline void
EncodeQP(u_int c, StringC *ostr)
{
   *ostr += '=';
   *ostr += basis_qp[c >> 4 ];
   *ostr += basis_qp[c & 0xf];
}

/*-----------------------------------------------------------------------
 * Add a properly encoded newline character to the specified string
 */

inline void
EncodeNewlineQP(u_int prevc, StringC *ostr)
{
   if ( prevc == ' ' || prevc == '\t' ) *ostr += "=\n\n";
   else					*ostr += '\n';
}

/*-----------------------------------------------------------------------
 * Function to encode the given string using quoted-printable encoding and
 *    store the result in a string
 */

Boolean
TextToTextQP(CharC istr, StringC *ostr, Boolean breakLines)
{
//
// Loop through each character and check for wierd ones
//
   int		lineSize = 0;
   u_int	prevc = 255;
   for (u_int i=0; i<istr.Length(); i++) {

      u_int	c = (u_char)istr[i];
      if ( NeedsQP(c, lineSize) ) {

	 EncodeQP(c, ostr);
	 lineSize += 3;
	 prevc = 'A';	/* close enough */

      } // End if character needs to be encoded

//
// If this is a newline character
//
      else if ( c == '\n' ) {

	 EncodeNewlineQP(prevc, ostr);
	 lineSize = 0;
	 prevc = '\n';

      } // End if this is a newline

//
// Look for 'From ' at the beginning of a line
// HORRIBLE but clever hack suggested by MTR for sendmail-avoidance
//
      else if ( prevc == '\n' && istr.StartsWith("From ", i) ) {
	 *ostr += "=46rom ";
	 lineSize += 7;
	 i += 4;	// One more will be added at bottom of loop
      }

//
// Otherwise this is an ordinary character
//
      else {
	 *ostr += (char)c;
	 lineSize++;
	 prevc = c;
      }

//
// Wrap this line if necessary
//
      if ( lineSize > 72 ) {
	 if ( breakLines ) {
	    *ostr += "=\n";
	    prevc = '\n';
	 }
	 lineSize = 0;
      }

   } // End for each character

//
// Add a final newline
//
   if ( lineSize > 0 && breakLines )
      *ostr += "=\n";

   return True;

} // End TextToTextQP

/*-------------------------------------------------------------------------
 * Function to encode a stream of bytes from an open file and place the
 *    output in a file
 */

Boolean
FileToFileQP(const char *ifile, const char *ofile, FILE *ifp, FILE *ofp,
	     u_int offset, u_int length)
{
   Boolean	closeInput  = (ifp == NULL);
   Boolean	closeOutput = (ofp == NULL);

   if ( !ifp ) {

//
// Open input file
//
      ifp = fopen(ifile, "r");
      if ( !ifp ) {
	 StringC	errmsg("Could not open file \"");
	 errmsg += ifile;
	 errmsg += "\" for QP encoding.\n";
	 errmsg += SystemErrorMessage(errno);
	 halApp->PopupMessage(errmsg);
	 return False;
      }

//
// Initialize length
//
      if ( length == 0 ) {
	 fseek(ifp, 0, SEEK_END);
	 length = (u_int)ftell(ifp);
      }

   } // End if input file is not open

//
// Move to offset in input file
//
   if ( fseek(ifp, offset, SEEK_SET) != 0 ) {
      StringC	errmsg("Could not seek to offset ");
      errmsg += (int)offset;
      errmsg += " in input file ";
      errmsg += ifile;
      errmsg += "\" for QP encoding.\n";
      errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
      if ( closeInput ) fclose(ifp);
      return False;
   }

   if ( !ofp ) {

//
// Create an output file
//
      ofp = fopen(ofile, "w+");
      if ( !ofp ) {
	 StringC	errmsg("Could not create file \"");
	 errmsg += ofile;
	 errmsg += "\" for QP encoding.\n";
	 errmsg += SystemErrorMessage(errno);
	 halApp->PopupMessage(errmsg);
	 if ( closeInput ) fclose(ifp);
	 return False;
      }

   } // End if output file is not open

//
// Loop through each character and check for wierd ones
//
   int		lineSize = 0;
   u_int	prevc = 255;
   int		c;
   Boolean	error = False;
   while ( !error && length>0 && (c=getc(ifp)) != EOF ) {

      length--;

//
// Encode normal QP stuff plus 'F' at the beginning of a line.  This is to
//    hide possible "From "s
// HORRIBLE but clever hack suggested by MTR for sendmail-avoidance
//
      if ( NeedsQP((u_int)c, lineSize) || prevc == '\n' && c == 'F' ) {

	 error = !EncodeQP((u_int)c, ofp);
	 lineSize += 3;
	 prevc = 'A';	/* close enough */

      } // End if character needs to be encoded

//
// If this is a newline character
//
      else if ( c == '\n' ) {

	 error = !EncodeNewlineQP(prevc, ofp);
	 lineSize = 0;
	 prevc = '\n';

      } // End if this is a newline

//
// Otherwise this is an ordinary character
//
      else {
	 error = (putc((char)c, ofp) == EOF);
	 lineSize++;
	 prevc = (u_int)c;
      }

//
// Wrap this line if necessary
//
      if ( !error && lineSize > 72 ) {
	 error = (putc('=', ofp) == EOF || putc('\n', ofp) == EOF);
	 prevc = '\n';
	 lineSize = 0;
      }

   } // End for each character

//
// Add a final newline
//
   if ( !error && lineSize > 0 )
      error = (putc('=', ofp) == EOF || putc('\n', ofp) == EOF);

   if ( error ) {
      StringC	errmsg("Could not write file \"");
      errmsg += ofile;
      errmsg += "\" for QP encoding.\n";
      errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
   }

   else if ( c == EOF && length > 0 ) {
      StringC	errmsg("Unexpected end of file \"");
      errmsg += ifile;
      errmsg += "\" while QP encoding.\n";
      errmsg += "Expected ";
      errmsg += (int)length;
      errmsg += " more bytes. ";
      halApp->PopupMessage(errmsg);
   }

   if ( closeInput  ) fclose(ifp);
   if ( closeOutput ) fclose(ofp);

   return !error;

} // End FileToFileQP

/*-----------------------------------------------------------------------
 * Function to encode the given string using quoted-printable encoding and
 *    store the result in a file
 */

Boolean
TextToFileQP(CharC istr, const char *ofile, FILE *ofp)
{
   Boolean	closeOutput = (ofp == NULL);

   if ( !ofp ) {

//
// Create an output file
//
      ofp = fopen(ofile, "w+");
      if ( !ofp ) {
	 StringC	errmsg("Could not create file \"");
	 errmsg += ofile;
	 errmsg += "\" for QP encoding.\n";
	 errmsg += SystemErrorMessage(errno);
	 halApp->PopupMessage(errmsg);
	 return False;
      }
   }

//
// Loop through each character and check for wierd ones
//
   int		lineSize = 0;
   u_int	prevc = 255;
   Boolean	error = False;
   for (u_int i=0; !error && i<istr.Length(); i++) {

      u_int	c = (u_char)istr[i];
      if ( NeedsQP(c, lineSize) ) {

	 error = !EncodeQP(c, ofp);
	 lineSize += 3;
	 prevc = 'A';	/* close enough */

      } // End if character needs to be encoded

//
// If this is a newline character
//
      else if ( c == '\n' ) {

	 error = !EncodeNewlineQP(prevc, ofp);
	 lineSize = 0;
	 prevc = '\n';

      } // End if this is a newline

//
// Look for 'From ' at the beginning of a line
// HORRIBLE but clever hack suggested by MTR for sendmail-avoidance
//
      else if ( prevc == '\n' && istr.StartsWith("From ", i) ) {
	 error = (fwrite("=46rom ", 1, 7, ofp) != 7);
	 lineSize += 7;
	 i += 4;	// One more will be added at bottom of loop
      }

//
// Otherwise this is an ordinary character
//
      else {
	 error = (putc(c, ofp) == EOF);
	 lineSize++;
	 prevc = c;
      }

//
// Wrap this line if necessary
//
      if ( !error && lineSize > 72 ) {
	 error = (putc('=', ofp) == EOF || putc('\n', ofp) == EOF);
	 prevc = '\n';
	 lineSize = 0;
      }

   } // End for each character

//
// Add a final newline
//
   if ( !error && lineSize > 0 )
      error = (putc('=', ofp) == EOF || putc('\n', ofp) == EOF);

   if ( error ) {
      StringC	errmsg("Could not write file \"");
      errmsg += ofile;
      errmsg += "\" for QP encoding.\n";
      errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
   }

   if ( closeOutput ) fclose(ofp);
   return !error;

} // End TextToFileQP

/*-------------------------------------------------------------------------
 * Function to encode a file and place the quoted-printable encoded
 *    output in a text string
 */

Boolean
FileToTextQP(const char *ifile, StringC *ostr, Boolean breakLines,
	     FILE *ifp, u_int offset, u_int length)
{
   Boolean	closeInput  = (ifp == NULL);

   if ( !ifp ) {

//
// Open input file
//
      ifp = fopen(ifile, "r");
      if ( !ifp ) {
	 StringC	errmsg("Could not open file \"");
	 errmsg += ifile;
	 errmsg += "\" for QP encoding.\n";
	 errmsg += SystemErrorMessage(errno);
	 halApp->PopupMessage(errmsg);
	 return False;
      }

//
// Initialize length
//
      if ( length == 0 ) {
	 fseek(ifp, 0, SEEK_END);
	 length = (u_int)ftell(ifp);
      }

   } // End if input file is not open

//
// Move to offset in input file
//
   if ( fseek(ifp, offset, SEEK_SET) != 0 ) {
      StringC	errmsg("Could not seek to offset ");
      errmsg += (int)offset;
      errmsg += " in input file ";
      errmsg += ifile;
      errmsg += "\" for QP encoding.\n";
      errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
      if ( closeInput ) fclose(ifp);
      return False;
   }

//
// Loop through each character and check for wierd ones
//
   int		lineSize = 0;
   u_int	prevc = 255;
   int		c;
   while ( length>0 && (c=getc(ifp)) != EOF ) {

      length--;

//
// Encode normal QP stuff plus 'F' at the beginning of a line.  This is to
//    hide possible "From "s
// HORRIBLE but clever hack suggested by MTR for sendmail-avoidance
//
      if ( NeedsQP((u_int)c, lineSize) || prevc == '\n' && c == 'F' ) {

	 EncodeQP((u_int)c, ostr);
	 lineSize += 3;
	 prevc = 'A';	/* close enough */

      } // End if character needs to be encoded

//
// If this is a newline character
//
      else if ( c == '\n' ) {

	 EncodeNewlineQP(prevc, ostr);
	 lineSize = 0;
	 prevc = '\n';

      } // End if this is a newline

//
// Otherwise this is an ordinary character
//
      else {
	 *ostr += (char)c;
	 lineSize++;
	 prevc = (u_int)c;
      }

//
// Wrap this line if necessary
//
      if ( lineSize > 72 ) {
	 if ( breakLines ) {
	    *ostr += "=\n";
	    prevc = '\n';
	 }
	 lineSize = 0;
      }

   } // End for each character

//
// Add a final newline
//
   if ( lineSize > 0 && breakLines )
      *ostr += "=\n";

   if ( closeInput ) fclose(ifp);

   if ( c == EOF && length > 0 ) {
      StringC	errmsg("Unexpected end of file \"");
      errmsg += ifile;
      errmsg += "\" while QP encoding.\n";
      errmsg += "Expected ";
      errmsg += (int)length;
      errmsg += " more bytes. ";
      halApp->PopupMessage(errmsg);
      return False;
   }

   return True;

} // End FileToTextQP

/*-----------------------------------------------------------------------
 * Function to encode the given string using RFC1522 Q encoding and
 *    store the result in a string in the following format:
 *
 *  "=?charset?Q?encoded-text?="
 */

//
// The maximum length of the entire encoded string is 75 characters.
//
#define MAX_1522_WORD_LENGTH	75

Boolean
TextToText1522Q(CharC istr, CharC charset, StringC *ostr)
{
//
// Encode the input
//
   StringC	result;
   if ( !TextToTextQP(istr, &result, False/*don't break lines*/) )
      return False;

//
// Convert spaces to underscores
//
   char	*cs = result;
   while ( *cs ) {
      if ( *cs == ' ' ) *cs = '_';
      cs++;
   }

//
// If the result is too long, we may have to split it.
//
   int		max = MAX_1522_WORD_LENGTH - charset.Length() - 7;
   CharC	remaining = result;
   while ( remaining.Length() > max ) {

//
// Take as many characters as possible.  If there is an equals sign within
//   2 characters before the max character, back up.
//
      CharC	word = remaining(0, max);
      if      ( word[max-1] == '=' ) word.CutEnd(1);
      else if ( word[max-2] == '=' ) word.CutEnd(2);

//
// Add this word
//
      *ostr += "=?";
      *ostr += charset;
      *ostr += "?Q?";
      *ostr += word;
      *ostr += "?= "; // An extra space is required between split words

//
// Remove this word from the remaining
//
      remaining.CutBeg(word.Length());

   } // End for each output word required

//
// Add whatever's left
//
   *ostr += "=?";
   *ostr += charset;
   *ostr += "?Q?";
   *ostr += remaining;
   *ostr += "?=";

   return True;

} // End TextToText1522Q

/*-------------------------------------------------------------------------
 * Lookup table for QP decoding
 */
static unsigned char index_qp[128] = {
    255,255,255,255, 255,255,255,255, 255,255,255,255, 255,255,255,255,
    255,255,255,255, 255,255,255,255, 255,255,255,255, 255,255,255,255,
    255,255,255,255, 255,255,255,255, 255,255,255,255, 255,255,255,255,
     0, 1, 2, 3,  4, 5, 6, 7,  8, 9,255,255, 255,255,255,255,
    255,10,11,12, 13,14,15,255, 255,255,255,255, 255,255,255,255,
    255,255,255,255, 255,255,255,255, 255,255,255,255, 255,255,255,255,
    255,10,11,12, 13,14,15,255, 255,255,255,255, 255,255,255,255,
    255,255,255,255, 255,255,255,255, 255,255,255,255, 255,255,255,255
};
//
// QP decoding macro
//
#define CHAR_QP(C)	((((u_int)(C)) > 127) ? 255 : index_qp[(C)])

/*-------------------------------------------------------------------------
 * Decode a quoted-printable character and write the result to a file
 */

inline Boolean
DecodeQP(char i1, char i2, FILE *fp)
{
   u_int	o1 = CHAR_QP(i1);
   u_int	o2 = CHAR_QP(i2);
   u_int	o3 = (o1<<4 | o2);
   return (putc(o3, fp) != EOF);
}

/*-------------------------------------------------------------------------
 * Decode a quoted-printable character and add the result to a string
 */

inline void
DecodeQP(char i1, char i2, StringC *ostr)
{
   u_int	o1 = CHAR_QP(i1);
   u_int	o2 = CHAR_QP(i2);
   u_int	o3 = (o1<<4 | o2);
   *ostr += (char)o3;
}

/*-------------------------------------------------------------------------
 * Function to decode a quoted-printable encoded character array and place
 *    the decoded output in another string
 */

Boolean
TextQPToText(CharC istr, StringC *ostr)
{
//
// Look for =
//
   int	pos = istr.PosOf('=');
   if ( pos < 0 ) {
      *ostr += istr;
      return True;
   }

//
// Something needs to be converted
//
   u_int	offset = 0;
   CharC	range;
   while ( pos >= 0 ) {

//
// Write data up to this point
//
      u_int	len = pos - offset;
      if ( len > 0 )
	 *ostr += istr(offset, pos-offset);

//
// Translate characters
//
      pos++;
      if ( istr[pos] == '\n') {
	 pos++;
      }

      else {

//
// Make sure these are valid hex digits
//
	 char	i1 = istr[pos];
	 char	i2 = istr[pos+1];

	 if ( isxdigit(i1) && isxdigit(i2) ) {
	    DecodeQP(i1, i2, ostr);
	    pos += 2;
	 }

//
// If these aren't valid hex characters, write the equals and keep looking
//
	 else
	    *ostr += '=';

      } // End if equals followed by non-newline

      offset = pos;
      pos = istr.PosOf('=', offset);

   } // End for each =

//
// Write data up to this point
//
   u_int	len = istr.Length() - offset;
   if ( len > 0 )
      *ostr += istr(offset, istr.Length()-offset);

   return True;

} // End TextQPToText

/*-------------------------------------------------------------------------
 * Function to decode a quoted-printable encoded file and place the decoded
 *    output in a file
 */

Boolean
FileQPToFile(const char *ifile, const char *ofile, FILE *ifp, FILE *ofp,
	     u_int offset, u_int length)
{
   Boolean	closeInput  = (ifp == NULL);
   Boolean	closeOutput = (ofp == NULL);

   if ( !ifp ) {

//
// Open input file
//
      ifp = fopen(ifile, "r");
      if ( !ifp ) {
	 StringC	errmsg("Could not open file \"");
	 errmsg += ifile;
	 errmsg += "\" for QP decoding.\n";
	 errmsg += SystemErrorMessage(errno);
	 halApp->PopupMessage(errmsg);
	 return False;
      }

//
// Initialize length
//
      if ( length == 0 ) {
	 fseek(ifp, 0, SEEK_END);
	 length = (u_int)ftell(ifp);
      }

   } // End if input file is not open

//
// Move to offset in input file
//
   if ( fseek(ifp, offset, SEEK_SET) != 0 ) {
      StringC	errmsg("Could not seek to offset ");
      errmsg += (int)offset;
      errmsg += " in input file ";
      errmsg += ifile;
      errmsg += "\" for QP decoding.\n";
      errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
      if ( closeInput ) fclose(ifp);
      return False;
   }

   if ( !ofp ) {

//
// Create an output file
//
      ofp = fopen(ofile, "w+");
      if ( !ofp ) {
	 StringC	errmsg("Could not create file \"");
	 errmsg += ofile;
	 errmsg += "\" for QP decoding.\n";
	 errmsg += SystemErrorMessage(errno);
	 halApp->PopupMessage(errmsg);
	 if ( closeInput ) fclose(ifp);
	 return False;
      }
   }

//
// Loop through each character and check for encoded ones
//
   int		c;
   Boolean	ierror = False;
   Boolean	oerror = False;
   while ( !ierror && !oerror && length>0 && (c=getc(ifp)) != EOF ) {

      length--;

//
// Look for encoding
//
      if ( c == '=' && length>0 ) {

	 int	i1 = getc(ifp);
	 ierror = (i1 == EOF);
	 if ( !ierror ) length--;

//
// Ignore escaped newline
//
	 if ( !ierror && i1 != '\n' ) {

	    if ( length > 0 ) {

	       int	i2 = getc(ifp);
	       ierror = (i2 == EOF);
	       if ( !ierror ) {

		  length--;


//
// Make sure these are valid hex digits
//
		  if ( isxdigit(i1) && isxdigit(i2) )
		     oerror = !DecodeQP(i1, i2, ofp);

//
// If these aren't valid hex characters, put them back, write the equals and
//    keep looking
//
		  else {
		     ungetc(i2, ifp);
		     ungetc(i1, ifp);
		     length += 2;
		     oerror = (putc('=', ofp) == EOF);
		  }

	       } // End if character read

	    } // End if more data

	    else
	       oerror = (putc((char)i1, ofp) == EOF);

	 } // End if equals followed by non-newline

      } // End if equals

      else
	 oerror = (putc((char)c, ofp) == EOF);

   } // End for each input character

   if ( ierror ) {
      StringC	errmsg("Unexpected end of file \"");
      errmsg += ifile;
      errmsg += "\" while QP decoding.\n";
      errmsg += "Expected ";
      errmsg += (int)length;
      errmsg += " more bytes. ";
      halApp->PopupMessage(errmsg);
   }

   else if ( oerror ) {
      StringC	errmsg("Could not write file \"");
      errmsg += ofile;
      errmsg += "\" for QP decoding.\n";
      errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
   }

   if ( closeInput  ) fclose(ifp);
   if ( closeOutput ) fclose(ofp);
   return (!ierror && !oerror);

} // End FileQPToFile

/*-------------------------------------------------------------------------
 * Function to decode a quoted-printable encoded file and place the decoded
 *    output in a text string
 */

Boolean
FileQPToText(const char *ifile, StringC *ostr, FILE *ifp, u_int offset,
	     u_int length)
{
   Boolean	closeInput  = (ifp == NULL);

   if ( !ifp ) {

//
// Open input file
//
      ifp = fopen(ifile, "r");
      if ( !ifp ) {
	 StringC	errmsg("Could not open file \"");
	 errmsg += ifile;
	 errmsg += "\" for QP decoding.\n";
	 errmsg += SystemErrorMessage(errno);
	 halApp->PopupMessage(errmsg);
	 return False;
      }

//
// Initialize length
//
      if ( length == 0 ) {
	 fseek(ifp, 0, SEEK_END);
	 length = (u_int)ftell(ifp);
      }

   } // End if input file is not open

//
// Move to offset in input file
//
   if ( fseek(ifp, offset, SEEK_SET) != 0 ) {
      StringC	errmsg("Could not seek to offset ");
      errmsg += (int)offset;
      errmsg += " in input file ";
      errmsg += ifile;
      errmsg += "\" for QP decoding.\n";
      errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
      if ( closeInput ) fclose(ifp);
      return False;
   }

//
// Loop through each character and check for encoded ones
//
   int		c;
   Boolean	ierror = False;
   while ( !ierror && length>0 && (c=getc(ifp)) != EOF ) {

      length--;

//
// Look for encoding
//
      if ( c == '=' && length>0 ) {

	 int	i1 = getc(ifp);
	 ierror = (i1 == EOF);
	 if ( !ierror ) length--;

//
// Ignore escaped newline
//
	 if ( !ierror && i1 != '\n' ) {

	    if ( length > 0 ) {

	       int	i2 = getc(ifp);
	       ierror = (i2 == EOF);
	       if ( !ierror ) {

		  length--;

//
// Make sure these are valid hex digits
//
		  if ( isxdigit(i1) && isxdigit(i2) )
		     DecodeQP(i1, i2, ostr);

//
// If these aren't valid hex characters, put them back, write the equals and
//    keep looking
//
		  else {
		     ungetc(i2, ifp);
		     ungetc(i1, ifp);
		     length += 2;
		     *ostr += '=';
		  }

	       } // End if character read

	    } // End if more data

	    else
	       *ostr += (char)i1;

	 } // End if equals followed by non-newline

      } // End if equals

      else
	 *ostr += (char)c;

   } // End for each input character

   if ( ierror ) {
      StringC	errmsg("Unexpected end of file \"");
      errmsg += ifile;
      errmsg += "\" while QP decoding.\n";
      errmsg += "Expected ";
      errmsg += (int)length;
      errmsg += " more bytes. ";
      halApp->PopupMessage(errmsg);
   }

   if ( closeInput  ) fclose(ifp);
   return !ierror;

} // End FileQPToText

/*-------------------------------------------------------------------------
 * Function to decode a quoted-printable encoded character array and place
 *    the decoded output in a file
 */

Boolean
TextQPToFile(CharC istr, const char *ofile, FILE *ofp)
{
   Boolean	closeOutput = (ofp == NULL);

   if ( !ofp ) {

//
// Create an output file
//
      ofp = fopen(ofile, "w+");
      if ( !ofp ) {
	 StringC	errmsg("Could not create file \"");
	 errmsg += ofile;
	 errmsg += "\" for QP decoding.\n";
	 errmsg += SystemErrorMessage(errno);
	 halApp->PopupMessage(errmsg);
	 return False;
      }
   }

//
// Look for =
//
   Boolean	error = False;
   int	pos = istr.PosOf('=');
   if ( pos < 0 ) {
      error = !istr.WriteFile(ofp);
      if ( error ) {
	 StringC	errmsg("Could not write file \"");
	 errmsg += ofile;
	 errmsg += "\" for QP decoding.\n";
	 errmsg += SystemErrorMessage(errno);
	 halApp->PopupMessage(errmsg);
      }
      if ( closeOutput ) fclose(ofp);
      return !error;
   }

//
// Something needs to be converted
//
   u_int	offset = 0;
   CharC	range;
   while ( !error && pos >= 0 ) {

//
// Write data up to this point
//
      u_int	len = pos - offset;
      if ( len > 0 ) {
	 range = istr(offset, pos-offset);
	 error = !range.WriteFile(ofp);
	 if ( error ) continue;
      }

//
// Translate characters
//
      pos++;
      if ( istr[pos] == '\n') {
	 pos++;
      }

      else {

//
// Make sure these are valid hex digits
//
	 char	i1 = istr[pos];
	 char	i2 = istr[pos+1];

	 if ( isxdigit(i1) && isxdigit(i2) ) {
	    error = !DecodeQP(i1, i2, ofp);
	    pos += 2;
	 }

//
// If these aren't valid hex characters, write the equals and keep looking
//
	 else {
	    error = (putc('=', ofp) == EOF);
	 }

      } // End if equals followed by non-newline

      offset = pos;
      pos = istr.PosOf('=', offset);

   } // End for each =

//
// Write data up to this point
//
   u_int	len = istr.Length() - offset;
   if ( !error && len > 0 ) {
      range = istr(offset, istr.Length()-offset);
      error = !range.WriteFile(ofp);
   }

   if ( error ) {
      StringC	errmsg("Could not write file \"");
      errmsg += ofile;
      errmsg += "\" for QP decoding.\n";
      errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
   }

   if ( closeOutput ) fclose(ofp);
   return !error;

} // End TextQPToFile

/*-------------------------------------------------------------------------
 * Function to decode an RFC1522 Q encoded character array and place
 *    the decoded output in another string
 */

Boolean
Text1522QToText(CharC istr, StringC *ostr)
{
//
// Look for _ and =
//
   u_int	offset = 0;
   CharC	range;

   int		epos = istr.PosOf('=');
   int		upos = istr.PosOf('_');
   int		pos;
   if ( epos >= 0 && upos >= 0 ) pos = MIN(epos, upos);
   else if ( epos >= 0 )	 pos = epos;
   else if ( upos >= 0 )	 pos = upos;
   else				 pos = -1;

   while ( pos >= 0 ) {

//
// Copy istr up to this point
//
      u_int	len = pos - offset;
      if ( len > 0 ) {
	 range = istr(offset, len);
	 *ostr += range;
      }

//
// Convert an underscore to hex 20
//
      if ( istr[pos] == '_' ) {
	 pos++;
	 *ostr += (char)0x20;
      }

      else {

//
// Translate characters
//
	 pos++;
	 if ( istr[pos] == '\n')
	    pos++;

	 else {

//
// Make sure these are valid hex digits
//
	    char	i1 = istr[pos];
	    char	i2 = istr[pos+1];

	    if ( isxdigit(i1) && isxdigit(i2) ) {
	       DecodeQP(i1, i2, ostr);
	       pos += 2;
	    }

//
// If these aren't valid hex characters, write the equals and keep looking
//
	    else
	       *ostr += '=';

	 } // End if equals followed by non-newline

      } // End if equals

      offset = pos;
      epos = istr.PosOf('=', offset);
      upos = istr.PosOf('_', offset);
      if ( epos >= 0 && upos >= 0 ) pos = MIN(epos, upos);
      else if ( epos >= 0 )	    pos = epos;
      else if ( upos >= 0 )	    pos = upos;
      else			    pos = -1;

   } // End for each =

//
// Copy istr up to this point
//
   u_int	len = istr.Length() - offset;
   if ( len > 0 ) {
      range = istr(offset, len);
      *ostr += range;
   }

   return True;

} // End Text1522QToText

