/*
 *  $Id: Base64.C,v 1.5 2000/08/17 14:58:45 evgeny Exp $
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
#include "Base64.h"

#include <hgl/StringC.h>
#include <hgl/CharC.h>
#include <hgl/SysErr.h>
#include <hgl/HalAppC.h>

#include <unistd.h>
#include <errno.h>

/*
 * Copyright (c) 1991 Bell Communications Research, Inc. (Bellcore)
 *
 * Permission to use, copy, modify, and distribute this material
 * for any purpose and without fee is hereby granted, provided
 * that the above copyright notice and this permission notice
 * appear in all copies, and that the name of Bellcore not be
 * used in advertising or publicity pertaining to this
 * material without the specific, prior written permission
 * of an authorized representative of Bellcore.  BELLCORE
 * MAKES NO REPRESENTATIONS ABOUT THE ACCURACY OR SUITABILITY
 * OF THIS MATERIAL FOR ANY PURPOSE.  IT IS PROVIDED "AS IS",
 * WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES.
 */

#define	LF_CHAR	(char)10
#define CR_CHAR	(char)13

/*
 * When text is encoded, LF is converted to CRLF.
 * When text is decoded, CRLF is converted to LF.
 * When non-text is encoded or decoded, no conversion is done.
 */

static Boolean	IsText    = False;
static Boolean	CRpending = False;

//*************************************************************************

//
// Lookup table for encoding
//
static char	basis_64[] =
   "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/*-------------------------------------------------------------------------
 * Encode 3 ints and write 4 int result to a file
 *
 * i1 = aabbccdd
 * i2 = eeffgghh
 * i3 = iijjkkll
 *
 * o1 = ..aabbcc
 * o2 = ..ddeeff
 * o3 = ..gghhii
 * o4 = ..jjkkll
 */

inline Boolean
Encode64(u_int buffer[3], FILE *fp)
{
   u_int	o1 =  (buffer[0] >> 2);
   u_int	o2 = ((buffer[0] & 0x03) << 4) | ((buffer[1] & 0xf0) >> 4);
   u_int	o3 = ((buffer[1] & 0x0f) << 2) | ((buffer[2] & 0xC0) >> 6);
   u_int	o4 =  (buffer[2] & 0x3f);

#if 0
   if ( debuglev > 1 )
      cout <<"("
	   <<hex(buffer[0],2)	<<" "
	   <<hex(buffer[1],2)	<<" "
	   <<hex(buffer[2],2)	<<") ("
	   <<hex(o1,2)		<<" "
	   <<hex(o2,2)		<<" "
	   <<hex(o3,2)		<<" "
	   <<hex(o4,2)		<<") ("
	   <<hex(basis_64[o1],2)	<<" "
	   <<hex(basis_64[o2],2)	<<" "
	   <<hex(basis_64[o3],2)	<<" "
	   <<hex(basis_64[o4],2)	<<")" <<endl;
#endif

   return ( putc(basis_64[o1], fp) != EOF &&
	    putc(basis_64[o2], fp) != EOF &&
	    putc(basis_64[o3], fp) != EOF &&
	    putc(basis_64[o4], fp) != EOF );

} // End Encode64

/*-------------------------------------------------------------------------
 * Encode 1 or 2 ints and write 4 int result to a file.
 */

inline Boolean
Encode64(u_int *buffer, int bufsize, FILE *fp)
{
//
// Buffer size equals 1 or 2
//
   if ( bufsize == 1 ) buffer[1] = 0;

   int	o1 =  (buffer[0] >> 2);
   int	o2 = ((buffer[0] & 0x03) << 4) | ((buffer[1] & 0xf0) >> 4);
   int	o3 =  (buffer[1] & 0x0f) << 2;

//
// Write first 2 words
//
   Boolean	error;
   error = (putc(basis_64[o1], fp) == EOF ||
	    putc(basis_64[o2], fp) == EOF);

//
// Write 3rd word if necessary or padding
//
   if ( !error ) {
      if ( bufsize == 2 ) error = (putc(basis_64[o3], fp) == EOF);
      else		  error = (putc('=',          fp) == EOF);
   }

//
// Write final padding
//
   if ( !error ) error = (putc('=', fp) == EOF);
   return !error;

} // End Encode64

/*-------------------------------------------------------------------------
 * Encode 3 ints and add 4 int result to a string
 *
 * i1 = aabbccdd
 * i2 = eeffgghh
 * i3 = iijjkkll
 *
 * o1 = ..aabbcc
 * o2 = ..ddeeff
 * o3 = ..gghhii
 * o4 = ..jjkkll
 */

inline void
Encode64(u_int buffer[3], StringC *str)
{
   u_int	o1 =  (buffer[0] >> 2);
   u_int	o2 = ((buffer[0] & 0x03) << 4) | ((buffer[1] & 0xf0) >> 4);
   u_int	o3 = ((buffer[1] & 0x0f) << 2) | ((buffer[2] & 0xC0) >> 6);
   u_int	o4 =  (buffer[2] & 0x3f);

#if 0
   if ( debuglev > 1 )
      cout <<"("
	   <<hex(buffer[0],2)	<<" "
	   <<hex(buffer[1],2)	<<" "
	   <<hex(buffer[2],2)	<<") ("
	   <<hex(o1,2)		<<" "
	   <<hex(o2,2)		<<" "
	   <<hex(o3,2)		<<" "
	   <<hex(o4,2)		<<") ("
	   <<hex(basis_64[o1],2)	<<" "
	   <<hex(basis_64[o2],2)	<<" "
	   <<hex(basis_64[o3],2)	<<" "
	   <<hex(basis_64[o4],2)	<<")" <<endl;
#endif

   *str += basis_64[o1];
   *str += basis_64[o2];
   *str += basis_64[o3];
   *str += basis_64[o4];

} // End Encode64

/*-------------------------------------------------------------------------
 * Encode 1 or 2 ints and add 4 int result to a string.
 */

inline void
Encode64(u_int *buffer, int bufsize, StringC *str)
{
//
// Buffer size equals 1 or 2
//
   if ( bufsize == 1 ) buffer[1] = 0;

   int	o1 =  (buffer[0] >> 2);
   int	o2 = ((buffer[0] & 0x03) << 4) | ((buffer[1] & 0xf0) >> 4);
   int	o3 =  (buffer[1] & 0x0f) << 2;

//
// Write first 2 words
//
   *str += basis_64[o1];
   *str += basis_64[o2];

//
// Write 3rd word if necessary or padding
//
   if ( bufsize == 2 ) *str += basis_64[o3];
   else		       *str += '=';

//
// Write final padding
//
   *str += '=';

} // End Encode64

/*-------------------------------------------------------------------------
 * Function to encode a character array and place the base-64 encoded output
 *    in a string
 */

Boolean
TextToText64(CharC istr, StringC *ostr, Boolean isText, Boolean breakLines)
{
   Boolean	LFpending   = False;

   u_int	buffer[3];
   int		bufsize  = 0;
   int		linesize = 0;
   for (int i=0; i<istr.Length(); i++) {

//
// Add this character to the buffer.  If this is text, convert LF to CRLF
//
      if ( isText ) {

	 if ( LFpending ) {
	    LFpending = False;
	    buffer[bufsize] = LF_CHAR;
	    i--;	// Read current character again later
	 }
	 else {
	    buffer[bufsize] = (u_char)istr[i];
	    if ( buffer[bufsize] == '\n' ) {
	       LFpending = True;
	       buffer[bufsize] = CR_CHAR;
	    }
	 }
      }

      else
	 buffer[bufsize] = (u_char)istr[i];

      bufsize++;

//
// Process the buffer if it is full
//
      if ( bufsize == 3 ) {

	 Encode64(buffer, ostr);

//
// Add a newline if necessary
//
	 linesize += 4;
	 if ( breakLines && linesize > 71 ) {
	    *ostr += '\n';
            linesize = 0;
	 }

	 bufsize = 0;

      } // End if buffer is full

   } // End for each input character

//
// Write out any partial buffer.
//
   if ( bufsize > 0 ) {

      Encode64(buffer, bufsize, ostr);
      linesize += 4;

   } // End if there is a partial buffer

//
// Add a final newline if necessary
//
   if ( breakLines && linesize > 0 )
      *ostr += '\n';

   return True;

} // End TextToText64

/*-------------------------------------------------------------------------
 * Function to encode a file and place the base-64 encoded
 *    output in a file
 */

Boolean
FileToFile64(const char *ifile, const char *ofile, Boolean isText,
	     FILE *ifp, FILE *ofp, u_int offset, u_int length)
{
   Boolean	LFpending   = False;
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
	 errmsg += "\" for Base64 encoding.\n";
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
      errmsg += "\" for Base64 encoding.\n";
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
	 errmsg += "\" for Base64 encoding.\n";
	 errmsg += SystemErrorMessage(errno);
	 halApp->PopupMessage(errmsg);
	 if ( closeInput ) fclose(ifp);
	 return False;
      }

   } // End if output file is not open

//
// Loop through each input character
//
   Boolean	error = False;
   u_int	buffer[3];
   int		bufsize  = 0;
   int		linesize = 0;
   int		c;
   while ( !error && length>0 && (c=getc(ifp)) != EOF ) {

      length--;

//
// Add this character to the buffer.  If this is text, convert LF to CRLF
//
      if ( isText ) {

	 if ( LFpending ) {
	    LFpending = False;
	    buffer[bufsize] = LF_CHAR;
	    ungetc(c, ifp);	// Read current character again later
	    length++;		// Reset length left to read
	 }
	 else {
	    buffer[bufsize] = (u_char)c;
	    if ( buffer[bufsize] == '\n' ) {
	       LFpending = True;
	       buffer[bufsize] = CR_CHAR;
	    }
	 }
      }

      else
	 buffer[bufsize] = (u_char)c;

      bufsize++;

//
// Process the buffer if it is full
//
      if ( bufsize == 3 ) {

	 error = !Encode64(buffer, ofp);

//
// Add a newline if necessary
//
	 linesize += 4;
	 if ( !error && linesize > 71 ) {
	    error = (putc('\n', ofp) == EOF);
            linesize = 0;
	 }

	 bufsize = 0;

      } // End if buffer is full

   } // End for each input character

//
// Handle trailing newline in text mode
//
   if (LFpending) {
      buffer[bufsize] = LF_CHAR;
      bufsize++;
   }

//
// Write out any partial buffer.
//
   if ( !error && bufsize > 0 ) {

      error = !Encode64(buffer, bufsize, ofp);
      linesize += 4;

   } // End if there is a partial buffer

//
// Add a final newline if necessary
//
   if ( !error && linesize > 0 )
      error = (putc('\n', ofp) == EOF);

   if ( error ) {
      StringC	errmsg("Could not write file \"");
      errmsg += ofile;
      errmsg += "\" for Base64 encoding.\n";
      errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
   }

   else if ( c == EOF && length > 0 ) {
      StringC	errmsg("Unexpected end of file \"");
      errmsg += ifile;
      errmsg += "\" while Base64 encoding.\n";
      errmsg += "Expected ";
      errmsg += (int)length;
      errmsg += " more byte";
      if ( length > 1 ) errmsg += 's';
      errmsg += '.';
      halApp->PopupMessage(errmsg);
      error = True;
   }

   if ( closeInput  ) fclose(ifp);
   if ( closeOutput ) fclose(ofp);

   return !error;

} // End FileToFile64

/*-------------------------------------------------------------------------
 * Function to encode a character array and place the base-64 encoded output
 *    in a file
 */

Boolean
TextToFile64(CharC istr, const char *ofile, Boolean isText, Boolean breakLines,
	     FILE *ofp)
{
   Boolean	LFpending   = False;
   Boolean	closeOutput = (ofp == NULL);

   if ( !ofp ) {

//
// Create an output file
//
      ofp = fopen(ofile, "w+");
      if ( !ofp ) {
	 StringC	errmsg("Could not create file \"");
	 errmsg += ofile;
	 errmsg += "\" for Base64 encoding.\n";
	 errmsg += SystemErrorMessage(errno);
	 halApp->PopupMessage(errmsg);
	 return False;
      }

   } // End if output file is not open

   Boolean	error = False;
   u_int	buffer[3];
   int		bufsize  = 0;
   int		linesize = 0;
   for (int i=0; !error && i<istr.Length(); i++) {

//
// Add this character to the buffer.  If this is text, convert LF to CRLF
//
      if ( isText ) {

	 if ( LFpending ) {
	    LFpending = False;
	    buffer[bufsize] = LF_CHAR;
	    i--;	// Read current character again later
	 }
	 else {
	    buffer[bufsize] = (u_char)istr[i];
	    if ( buffer[bufsize] == '\n' ) {
	       LFpending = True;
	       buffer[bufsize] = CR_CHAR;
	    }
	 }
      }

      else
	 buffer[bufsize] = (u_char)istr[i];

      bufsize++;

//
// Process the buffer if it is full
//
      if ( bufsize == 3 ) {

	 error = !Encode64(buffer, ofp);

//
// Add a newline if necessary
//
	 linesize += 4;
	 if ( !error && breakLines && linesize > 71 ) {
	    error = (putc('\n', ofp) == EOF);
            linesize = 0;
	 }

	 bufsize = 0;

      } // End if buffer is full

   } // End for each input character

//
// Handle trailing newline in text mode
//
   if (LFpending) {
      buffer[bufsize] = LF_CHAR;
      bufsize++;
   }

//
// Write out any partial buffer.
//
   if ( !error && bufsize > 0 ) {

      error = !Encode64(buffer, bufsize, ofp);
      linesize += 4;

   } // End if there is a partial buffer

//
// Add a final newline if necessary
//
   if ( !error && breakLines && linesize > 0 )
      error = (putc('\n', ofp) == EOF);

   if ( error ) {
      StringC	errmsg("Could not write file \"");
      errmsg += ofile;
      errmsg += "\" for Base64 encoding.\n";
      errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
   }

   if ( closeOutput ) fclose(ofp);
   return !error;

} // End TextToFile64

/*-------------------------------------------------------------------------
 * Function to encode a file and place the base-64 encoded
 *    output in a text string
 */

Boolean
FileToText64(const char *ifile, StringC *ostr, Boolean isText, FILE *ifp,
	     u_int offset, u_int length)
{
   Boolean	LFpending   = False;
   Boolean	closeInput  = (ifp == NULL);

   if ( !ifp ) {

//
// Open input file
//
      ifp = fopen(ifile, "r");
      if ( !ifp ) {
	 StringC	errmsg("Could not open file \"");
	 errmsg += ifile;
	 errmsg += "\" for Base64 encoding.\n";
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
      errmsg += "\" for Base64 encoding.\n";
      errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
      if ( closeInput ) fclose(ifp);
      return False;
   }

//
// Loop through each input character
//
   u_int	buffer[3];
   int		bufsize  = 0;
   int		linesize = 0;
   int		c;
   while ( length>0 && (c=getc(ifp)) != EOF ) {

      length--;

//
// Add this character to the buffer.  If this is text, convert LF to CRLF
//
      if ( isText ) {

	 if ( LFpending ) {
	    LFpending = False;
	    buffer[bufsize] = LF_CHAR;
	    ungetc(c, ifp);	// Read current character again later
	    length++;		// Reset count of characters to read
	 }
	 else {
	    buffer[bufsize] = (u_char)c;
	    if ( buffer[bufsize] == '\n' ) {
	       LFpending = True;
	       buffer[bufsize] = CR_CHAR;
	    }
	 }
      }

      else
	 buffer[bufsize] = (u_char)c;

      bufsize++;

//
// Process the buffer if it is full
//
      if ( bufsize == 3 ) {

	 Encode64(buffer, ostr);

//
// Add a newline if necessary
//
	 linesize += 4;
	 if ( linesize > 71 ) {
	    *ostr += '\n';
            linesize = 0;
	 }

	 bufsize = 0;

      } // End if buffer is full

   } // End for each input character

//
// Handle trailing newline in text mode
//
   if (LFpending) {
      buffer[bufsize] = LF_CHAR;
      bufsize++;
   }

//
// Write out any partial buffer.
//
   if ( bufsize > 0 ) {

      Encode64(buffer, bufsize, ostr);
      linesize += 4;

   } // End if there is a partial buffer

//
// Add a final newline if necessary
//
   if ( linesize > 0 )
      *ostr += '\n';

   if ( closeInput  ) fclose(ifp);

   if ( c == EOF && length > 0 ) {
      StringC	errmsg("Unexpected end of file \"");
      errmsg += ifile;
      errmsg += "\" while Base64 encoding.\n";
      errmsg += "Expected ";
      errmsg += (int)length;
      errmsg += " more byte";
      if ( length > 1 ) errmsg += 's';
      errmsg += '.';
      halApp->PopupMessage(errmsg);
      return False;
   }

   return True;

} // End FileToText64

/*-------------------------------------------------------------------------
 * Lookup table for base-64 decoding
 */

static unsigned char index_64[128] = {
    255,255,255,255, 255,255,255,255, 255,255,255,255, 255,255,255,255,
    255,255,255,255, 255,255,255,255, 255,255,255,255, 255,255,255,255,
    255,255,255,255, 255,255,255,255, 255,255,255,62, 255,255,255,63,
    52,53,54,55, 56,57,58,59, 60,61,255,255, 255,255,255,255,
    255, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14,
    15,16,17,18, 19,20,21,22, 23,24,25,255, 255,255,255,255,
    255,26,27,28, 29,30,31,32, 33,34,35,36, 37,38,39,40,
    41,42,43,44, 45,46,47,48, 49,50,51,255, 255,255,255,255
};

//
// Base-64 decoding macro
//
#define CHAR_64(C)	((((u_int)(C)) > 127) ? 255 : index_64[C])

/*-------------------------------------------------------------------------
 * Function to output a decoded character to a file with conversion of
 *    CRLF to CR for text.
 */

inline Boolean
Output64(char c, FILE *fp)
{
   int	status = 0;

   if ( IsText ) {

      if ( CRpending ) {

	 if ( c == LF_CHAR ) {
	    status = putc('\n', fp);
	    CRpending = False;
	 }
	 else {
	    status = putc(CR_CHAR, fp);
	    if ( status != EOF && c != CR_CHAR ) {
	       status = putc(c, fp);
	       CRpending = False;
	    }
	 }
      }

      else if ( c == CR_CHAR )
	 CRpending = True;

      else
	 status = putc(c, fp);

   } // End if we need newline conversion

   else
      status = putc(c, fp);

   return (status != EOF);

} // End Output64

/*-------------------------------------------------------------------------
 * Function to add a decoded character to a string with conversion of
 *    CRLF to CR for text.
 */

inline void
Output64(char c, StringC *str)
{
   int	status = 0;

   if ( IsText ) {

      if ( CRpending ) {

	 if ( c == LF_CHAR ) {
	    *str += '\n';
	    CRpending = False;
	 }
	 else {
	    *str += (char)CR_CHAR;
	    if ( status != EOF && c != CR_CHAR ) {
	       *str += c;
	       CRpending = False;
	    }
	 }
      }

      else if ( c == CR_CHAR )
	 CRpending = True;

      else
	 *str += c;

   } // End if we need newline conversion

   else
      *str += c;

} // End Output64

/*-------------------------------------------------------------------------
 * Function to convert 4 encoded ints into 3 decoded ints and write the
 *    result to a file.
 *
 * i1 = ..aabbcc
 * i2 = ..ddeeff
 * i3 = ..gghhii
 * i4 = ..jjkkll
 *
 * o1 = aabbccdd
 * o2 = eeffgghh
 * o3 = iijjkkll
 */

inline Boolean
Decode64(char buffer[4], FILE *fp, Boolean *done)
{
//
// '=' marks the end of the data
//
   if ( buffer[0] == '=' || buffer[1] == '=' ) {
      *done = True;
      return True;
   }

//
// Decode the buffer
//
   u_int	i1 = CHAR_64(buffer[0]);
   u_int	i2 = CHAR_64(buffer[1]);
   u_int	i3 = CHAR_64(buffer[2]);
   u_int	i4 = CHAR_64(buffer[3]);

//
// Take the lower 6 bits of each input byte and shift them together
//
   u_int	o1 = ((i1 & 0x3f) << 2) | ((i2 & 0x30) >> 4);
   u_int	o2 = ((i2 & 0x0f) << 4) | ((i3 & 0x3c) >> 2);
   u_int	o3 = ((i3 & 0x03) << 6) | ((i4 & 0x3f));

   if ( !Output64(o1, fp) ) {
      *done  = True;
      return False;
   }

//
// Check for end
//
   if ( buffer[2] == '=' ) {
      *done = True;
      return True;
   }

   if ( !Output64(o2, fp) ) {
      *done = True;
      return False;
   }

//
// Check for end
//
   if ( buffer[3] == '=' ) {
      *done = True;
      return True;
   }

   if ( !Output64(o3, fp) ) {
      *done = True;
      return False;
   }

   return True;

} // End Decode64

/*-------------------------------------------------------------------------
 * Function to convert 4 encoded ints into 3 decoded ints and add the
 *    result to a string.
 */

inline void
Decode64(char buffer[4], StringC *str, Boolean *done)
{
//
// '=' marks the end of the data
//
   if ( buffer[0] == '=' || buffer[1] == '=' ) {
      *done = True;
      return;
   }

//
// Decode the buffer
//
   u_int	i1 = CHAR_64(buffer[0]);
   u_int	i2 = CHAR_64(buffer[1]);
   u_int	i3 = CHAR_64(buffer[2]);
   u_int	i4 = CHAR_64(buffer[3]);

//
// Take the lower 6 bits of each input byte and shift them together
//
   u_int	o1 = ((i1 & 0x3f) << 2) | ((i2 & 0x30) >> 4);
   u_int	o2 = ((i2 & 0x0f) << 4) | ((i3 & 0x3c) >> 2);
   u_int	o3 = ((i3 & 0x03) << 6) | ((i4 & 0x3f));

   Output64(o1, str);

//
// Check for end
//
   if ( buffer[2] == '=' ) {
      *done = True;
      return;
   }

   Output64(o2, str);

//
// Check for end
//
   if ( buffer[3] == '=' ) {
      *done = True;
      return;
   }

   Output64(o3, str);

} // End Decode64

/*-------------------------------------------------------------------------
 * Function to decode a base-64 encoded character array and place the decoded
 *    output in a string
 */

Boolean
Text64ToText(CharC istr, StringC *ostr, Boolean isText)
{
   IsText    = isText;
   CRpending = False;

   istr.Trim();		// skip leading and trailing blank lines

//
// Loop through data 4 chars at a time
//
   char		buffer[4];
   int		bufsize = 0;
   Boolean	done  = False;
   for (int i=0; !done && i<istr.Length(); i++) {

//
// Skip whitespace characters
//
      if ( isspace(istr[i]) || istr[i] == '\n' )
	 continue;

//
// Add this character to the buffer
//
      buffer[bufsize++] = istr[i];

//
// Process the buffer if it is full
//
      if ( bufsize == 4 ) {
	 Decode64(buffer, ostr, &done);
	 bufsize = 0;
      }

   } // End for each input character

   return True;

} // End Text64ToText

/*-------------------------------------------------------------------------
 * Function to decode a base-64 encoded file and place the decoded
 *    output in a file
 */

Boolean
File64ToFile(const char *ifile, const char *ofile, Boolean isText,
	     FILE *ifp, FILE *ofp, u_int offset, u_int length)
{
   IsText    = isText;
   CRpending = False;

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
	 errmsg += "\" for Base64 decoding.\n";
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
      errmsg += "\" for Base64 decoding.\n";
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
	 errmsg += "\" for Base64 encoding.\n";
	 errmsg += SystemErrorMessage(errno);
	 halApp->PopupMessage(errmsg);
	 if ( closeInput ) fclose(ifp);
	 return False;
      }
   }

//
// Loop through data 4 chars at a time
//
   char		buffer[4];
   int		bufsize = 0;
   Boolean	done  = False;
   Boolean	error = False;
   int		c;
   while ( !done && length>0 && (c=getc(ifp)) != EOF ) {

      length--;

//
// Skip whitespace characters
//
      if ( isspace(c) || c == '\n' )
	 continue;

//
// Add this character to the buffer
//
      buffer[bufsize++] = (char)c;

//
// Process the buffer if it is full
//
      if ( bufsize == 4 ) {
	 error = !Decode64(buffer, ofp, &done);
	 bufsize = 0;
      }

   } // End for each input character

   if ( error ) {
      StringC	errmsg("Could not write file \"");
      errmsg += ofile;
      errmsg += "\" for Base64 encoding.\n";
      errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
   }

   else if ( c == EOF && length > 0 ) {
      StringC	errmsg("Unexpected end of file \"");
      errmsg += ifile;
      errmsg += "\" while Base64 decoding.\n";
      errmsg += "Expected ";
      errmsg += (int)length;
      errmsg += " more byte";
      if ( length > 1 ) errmsg += 's';
      errmsg += '.';
      halApp->PopupMessage(errmsg);
      error = True;
   }

   if ( closeInput  ) fclose(ifp);
   if ( closeOutput ) fclose(ofp);

   return !error;

} // End File64ToFile

/*-------------------------------------------------------------------------
 * Function to decode a base-64 encoded character array and place the decoded
 *    output in a file
 */

Boolean
Text64ToFile(CharC istr, const char *ofile, Boolean isText, FILE *ofp)
{
   IsText    = isText;
   CRpending = False;

   Boolean	closeOutput = (ofp == NULL);

   if ( !ofp ) {

//
// Create an output file
//
      ofp = fopen(ofile, "w+");
      if ( !ofp ) {
	 StringC	errmsg("Could not create file \"");
	 errmsg += ofile;
	 errmsg += "\" for Base64 encoding.\n";
	 errmsg += SystemErrorMessage(errno);
	 halApp->PopupMessage(errmsg);
	 return False;
      }
   }

   istr.Trim();		// skip leading and trailing blank lines

//
// Loop through data 4 chars at a time
//
   char		buffer[4];
   int		bufsize = 0;
   Boolean	done  = False;
   Boolean	error = False;
   for (int i=0; !done && i<istr.Length(); i++) {

//
// Skip whitespace characters
//
      if ( isspace(istr[i]) || istr[i] == '\n' )
	 continue;

//
// Add this character to the buffer
//
      buffer[bufsize++] = istr[i];

//
// Process the buffer if it is full
//
      if ( bufsize == 4 ) {
	 error = !Decode64(buffer, ofp, &done);
	 bufsize = 0;
      }

   } // End for each input character

   if ( error ) {
      StringC	errmsg("Could not write file \"");
      errmsg += ofile;
      errmsg += "\" for Base64 encoding.\n";
      errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
   }

   if ( closeOutput ) fclose(ofp);

   return !error;

} // End Text64ToFile

/*-------------------------------------------------------------------------
 * Function to decode a base-64 encoded file and place the decoded
 *    output in a text string
 */

Boolean
File64ToText(const char *ifile, StringC *ostr, Boolean isText, FILE *ifp,
	     u_int offset, u_int length)
{
   IsText    = isText;
   CRpending = False;

   Boolean	closeInput  = (ifp == NULL);

   if ( !ifp ) {

//
// Open input file
//
      ifp = fopen(ifile, "r");
      if ( !ifp ) {
	 StringC	errmsg("Could not open file \"");
	 errmsg += ifile;
	 errmsg += "\" for Base64 decoding.\n";
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
      errmsg += "\" for Base64 decoding.\n";
      errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
      if ( closeInput ) fclose(ifp);
      return False;
   }

//
// Loop through data 4 chars at a time
//
   char		buffer[4];
   int		bufsize = 0;
   Boolean	done  = False;
   int		c;
   while ( length>0 && (c=getc(ifp)) != EOF ) {

      length--;

//
// Skip whitespace characters
//
      if ( isspace(c) || c == '\n' )
	 continue;

//
// Add this character to the buffer
//
      buffer[bufsize++] = (char)c;

//
// Process the buffer if it is full
//
      if ( bufsize == 4 ) {
	 Decode64(buffer, ostr, &done);
	 bufsize = 0;
      }

   } // End for each input character

   if ( closeInput  ) fclose(ifp);

   if ( c == EOF && length > 0 ) {
      StringC	errmsg("Unexpected end of file \"");
      errmsg += ifile;
      errmsg += "\" while Base64 decoding.\n";
      errmsg += "Expected ";
      errmsg += (int)length;
      errmsg += " more byte";
      if ( length > 1 ) errmsg += 's';
      errmsg += '.';
      halApp->PopupMessage(errmsg);
      return False;
   }

   return True;

} // End File64ToText

/*-----------------------------------------------------------------------
 * Function to encode the given string using RFC1522 B encoding and
 *    store the result in a string
 */

#define MAX_1522_WORD_LENGTH	75

Boolean
TextToText1522B(CharC istr, CharC charset, StringC *ostr)
{
//
// Encode the input
//
   StringC	result;
   if ( !TextToText64(istr, &result, True/*is text*/, False/*no line break*/) )
      return False;

//
// If the result is too long, we may have to split it.
//
   int		max = MAX_1522_WORD_LENGTH - charset.Length() - 7;
   CharC	remaining = result;
   while ( remaining.Length() > max ) {

//
// Add this word
//
      *ostr += "=?";
      *ostr += charset;
      *ostr += "?B?";
      *ostr += remaining(0, max);
      *ostr += "?= "; // An extra space is required between split words

//
// Remove this word from the remaining
//
      remaining.CutBeg(max);

   } // End for each output word required

//
// Add whatever's left
//
   *ostr += "=?";
   *ostr += charset;
   *ostr += "?B?";
   *ostr += remaining;
   *ostr += "?=";

   return True;

} // End TextToText1522B
