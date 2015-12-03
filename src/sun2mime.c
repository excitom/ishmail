/*
 * $Id: sun2mime.c,v 1.3 2000/06/07 16:10:01 evgeny Exp $
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

/*
 * Name: sun2mime
 *
 * Function: Convert a mail message containing Sun mail attachments to a
 *	MIME message. Input is file name, offset of start of message, 
 *	number of lines in the message, and output file name.
 *
 *	Return code 0 = successful translation, 1 = something was wrong.
 *
 * Tom Lang
 * 8/94
 */

#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define XSUNATT "Content-Type: X-sun-attachment"
#define XSUNDT "X-Sun-Data-Type: "
#define XSUNDD "X-Sun-Data-Description: "
#define XSUNDN "X-Sun-Data-Name: "
#define XSUNEN "X-Sun-Encoding-Info: "
#define XSUNCL "X-Sun-Content-Lines: "
#define XSUNCLEN "X-Sun-Content-Length: "
#define XSUNBDY "----------"
#define XLINES "X-Lines: "

#define CONTENTLENGTH "Content-Length: "
#define CONTENTLINES "Content-Lines: "

/*#define MIMEBDY "--bODY.pART.bOUNDARY"*/
static char	*mimebdy;

#ifndef SEEK_SET
#define SEEK_SET 0
#endif

/* Data type codes */
#define T_UNK	0
#define T_TEXT	1
#define T_PS	2
#define T_XBM	3
#define T_XPM	4
#define T_AUDIO	5
#define T_GIF	6
#define T_FRAME	7

void convert();
void convertPart();
void convertText( int );
void convertEncoded( int );
void uu2base64();
void setContentType( int, char *, int );
void uuDecode( char * );
int  To64( FILE *, FILE *, int ); 
void copyPart( FILE * );
void copyLines( FILE * );
void findBegin( char * );

/*
 * global variables 
 */
extern int debug1, debug2;
static struct {
	int contentLength;
	int contentLines;
	int bannerLoc;
	int inconsistent;	/* flag: inconsistent content-length or lines */
	FILE *origFile;
	FILE *inFile;
	FILE *outFile;
	char line[2048];
} g;

/**************************************************************************/
int
sun2mime( char *in, int msgOffset, int bytes, char *out, char *bound )
{
	int inCount, outCount;
	char tmp[256];
	struct stat stats;

	mimebdy = bound;

	g.inconsistent = 0;
	if(debug1 || debug2) {
		fprintf(stderr, "sun2mime called, offset = %i, length = %i\n", msgOffset, bytes);
	}

	/*
	 * open the file to be converted
	 */
	if ((g.origFile = fopen( in, "r")) == NULL) {
		fprintf(stderr, "sun2mime: Message file: %s ", in);
		perror("Can't open for reading");
		return(1);
	}
	/*
	 * If file size was not passed in, figure it out now.
	 */
	if (bytes <= 0) {
		if (stat(in, &stats) == -1) {
			fprintf(stderr, "sun2mime: Message file: %s ", in);
			perror("Can't stat to find size");
			return(1);
		}
		bytes = stats.st_size;
	}
	/*
	 * Find the start of the message
	 * (not necessarily the beginning of the file if this is a 
	 * Unix-style folder)
	 */
	fseek( g.origFile, msgOffset, SEEK_SET);
	if(debug2) {
		fgets(g.line, sizeof(g.line), g.origFile);
		fprintf(stderr, "First line:\n%s\n",g.line);
		fseek(g.origFile, msgOffset+bytes, SEEK_SET);
		if((fgets(g.line, sizeof(g.line), g.origFile)) != NULL) {
			fprintf(stderr, "First line of next msg:\n%s\n",g.line);
		} else {
			fprintf(stderr, "No data past end\n");
		}
		fseek( g.origFile, msgOffset, SEEK_SET);
	}
	/*
	 * open a temporary file to contain a copy of the input message.
	 * note: this may be redundant, if the input message is the only
	 * message in the file...
	 */
	tmpnam( tmp );
	if((g.inFile = fopen( tmp, "w" )) == NULL) {
		perror("sun2mime: can't create temp file ");
		fclose(g.origFile);
		return(1);
	}
	/*
	 * extract the message from the original input file
	 */
	while(bytes > 0) {
		inCount = (bytes>sizeof(g.line)) ? sizeof(g.line) : bytes;
		inCount = fread(g.line, 1, inCount, g.origFile);
		bytes -= inCount;
		if(bytes < 0) {
			fprintf(stderr, "sun2mime: WARNING: input file length not consistent\n");
		}
		outCount = fwrite(g.line, 1, inCount, g.inFile);
		if(outCount != inCount) {
			fprintf(stderr, "sun2mime: WARNING: problem extracting msg\n");
		}
	}
	fclose(g.origFile);
	fclose(g.inFile);
	if ((g.inFile = fopen( tmp, "r")) == NULL) {
		fprintf(stderr, "sun2mime: Message file: %s ", in);
		perror("Can't open for reading");
		return(1);
	}
	/*
	 * open a temporary file to contain the converted message
	 * note: the name for this file was passed in from the caller
	 */
	if((g.outFile = fopen( out, "w" )) == NULL) {
		perror("sun2mime: can't create temp file ");
		fclose(g.inFile);
		return(1);
	}
	
	/*
	 * read through the mail message headers. copy headers to output
	 * until Sun-Attachment found or end of headers.
	 */
	if(debug2) {
		fprintf(stderr, "Looking for a Sun attachment\n");
	}
	while((fgets(g.line, sizeof(g.line), g.inFile)) != NULL) {
		/*
		 * look for blank line, indicating end of mail headers
		 */
		if( strlen(g.line) == 1 ) {
			fputc('\n', g.outFile);
			break;
		}
		/*
		 * look for "Content-Type: X-Sun-Attachment" mail header
		 * if found, call conversion routine, which will process
		 * the rest of the message.
		 */
		if( strncasecmp( g.line, XSUNATT, sizeof(XSUNATT)-1 ) == 0 ) {
			convert();
			break;
		}
		/*
		 * copy header line to output, unless it's Content-Length,
		 * Content-Lines, or X-Lines - this would be confusing...
		 */
		if((strncasecmp( g.line, CONTENTLENGTH, sizeof(CONTENTLENGTH)-1 ) != 0) && 
		   (strncasecmp( g.line, CONTENTLINES, sizeof(CONTENTLINES)-1 ) != 0) &&
		   (strncasecmp( g.line, XLINES, sizeof(XLINES)-1 ) != 0)) {
			fputs(g.line, g.outFile);
		}
	}
	/*
	 * If previous loop terminated because end of headers reached
	 * without finding that the message has Sun attachments, this
	 * loop will copy the remainder of the message. Else, there
	 * won't be anything left to copy.
	 *
	 * note: this function shouldn't be called unless there are
	 * Sun attachments, so there shouldn't be anything left...
	 */
	while((fgets(g.line, sizeof(g.line), g.inFile)) != NULL) {
		fputs(g.line, g.outFile);
	}

	/*
	 * close files
	 */
	fclose(g.inFile);
	unlink(tmp);	/* delete temporary file */
	fclose(g.outFile);
	return(0);
}
/*
 * subroutine: convert Sun attachment format to MIME format.
 */
void
convert() {
	/*
	 * output standard headers
	 */
	if(debug1 || debug2) {
		fprintf(stderr, "Found a message with one or more Sun attachments\n");
	}
	fputs("Mime-Version: 1.0\n", g.outFile);
	fputs("Content-Type: multipart/mixed;\n\tboundary=\"", g.outFile);
	fputs(mimebdy, g.outFile);
	fputs("\"\n", g.outFile);

	/*
	 * copy rest of the mail message headers
	 */
	while((fgets(g.line, sizeof(g.line), g.inFile)) != NULL) {
		fputs(g.line, g.outFile);
		/*
		 * look for blank line, indicating end of mail headers
		 */
		if( strlen(g.line) == 1 ) {
			break;
		}
	}
	/*
	 * output informative message
	 */
	fputs("--", g.outFile);
	fputs(mimebdy, g.outFile);
	fputc('\n', g.outFile);
	fputs("Content-Type: text/plain\n", g.outFile);
	fputs("Content-Description: conversion information\n", g.outFile);
	fputc('\n', g.outFile);
	fputs("******************************************************************************\n", g.outFile);
	fputs("*** This message was converted from Sun mailtool format to MIME by Ishmail ***\n", g.outFile);
	g.bannerLoc = ftell(g.outFile);
	fputs("******************************************************************************\n", g.outFile);
	fputc('\n', g.outFile);

	/*
	 * look for body parts, convert each one
	 */
	while((fgets(g.line, sizeof(g.line), g.inFile)) != NULL) {
		/*
		 * look for body part boundary
		 */
		if( strncasecmp( g.line, XSUNBDY, sizeof(XSUNBDY)-1 ) == 0 ) {
			fputs("--", g.outFile);
			fputs(mimebdy, g.outFile);
			fputc('\n', g.outFile);
			if(debug1 || debug2) {
				fprintf(stderr, "Found a body part to convert\n");
			}
			convertPart();
		}
		/*
		 * we don't expect to find extraneous lines between body
		 * parts, if the content length is correct, but if there
		 * are any - just copy them
		 */
		else
			fputs(g.line, g.outFile);
	}
	/*
	 * output final body part boundary and trailing blank line
	 */
	fputs("--", g.outFile);
	fputs(mimebdy, g.outFile);
	fputs("--\n\n", g.outFile);
	/*
	 * if inconsistent or missing content-length/lines info,
	 * update the banner with a warning
	 */
	if (g.inconsistent) {
		fseek(g.outFile, g.bannerLoc, SEEK_SET);
		fputs("*** NOTE: Inconsistent content-length info makes the conversion suspect... ***\n",g.outFile);
	}
}

#define DESC_SIZE 256
#define NAME_SIZE 256
#define ENCODE_SIZE 128
/*
 * subroutine: Determine body part type and process it.
 */
void
convertPart()
{
	int type;
	char *p;
	int isUU = 0;
	char desc[DESC_SIZE] = "";
	char name[NAME_SIZE] = "";
	char encoding[ENCODE_SIZE] = "";

	/*
	 * look through body part headers for:
	 *	data type
	 *	transfer encoding method
	 *	body part description
	 *	... others silently ignored
	 */
		if(debug2) {
			fprintf(stderr, "Looking for body part headers...\n");
		}
	while((fgets(g.line, sizeof(g.line), g.inFile)) != NULL) {

		/*
		 * end of headers ?
		 */
		if( strlen(g.line) == 1 ) {
			if(debug2) {
				fprintf(stderr, "End of body part headers.\n");
			}
			break;
		}	

		/*
		 * data type ?
		 */
		if( strncasecmp( g.line, XSUNDT, sizeof(XSUNDT)-1 ) == 0 ) {
			p = g.line + sizeof(XSUNDT)-1;

			if( strncasecmp( p, "text", sizeof("text")-1 ) == 0 ) {
				type = T_TEXT;
			} else
			if( strncasecmp( p, "postscript-file", sizeof("postscript-file")-1 ) == 0 ) {
				type = T_PS;
#if 0
			} else
			if( strncasecmp( p, "c-file", sizeof("c-file")-1 ) == 0 ) {
				type  = T_TEXT;
#endif
			} else
			if( strncasecmp( p, "xbm-file", sizeof("xbm-file")-1 ) == 0 ) {
				type = T_XBM;
			} else
			if( strncasecmp( p, "xpm-file", sizeof("xpm-file")-1 ) == 0 ) {
				type = T_XPM;
			} else
			if( strncasecmp( p, "audio-file", sizeof("audio-file")-1 ) == 0 ) {
				type = T_AUDIO;
			} else
			if( strncasecmp( p, "gif-file", sizeof("gif-file")-1 ) == 0 ) {
				type = T_GIF;
			} else
			if( strncasecmp( p, "framemaker-document", sizeof("framemaker-document")-1 ) == 0 ) {
				type = T_FRAME;
			}
			else {
				type = T_UNK;
			}
			if(debug2) {
				fprintf(stderr, "Data type header found, type = %i\n", type);
			}
			continue;
		}

		/*
		 * data description ?
		 */
		if( strncasecmp( g.line, XSUNDD, sizeof(XSUNDD)-1) == 0 ) {
			p = g.line + sizeof(XSUNDD)-1;
			/*
			 * truncate description, if necessary
			 */
			strncat( desc, p, DESC_SIZE );
			p = strrchr( desc, '\n' );
			if(p != NULL) *p = '\0';
			if(debug2) {
				fprintf(stderr, "Description: %s\n",desc);
			}
			continue;
		}
		/*
		 * data name ?
		 */
		if( strncasecmp( g.line, XSUNDN, sizeof(XSUNDN)-1) == 0 ) {
			p = g.line + sizeof(XSUNDN)-1;
			strncat( name, p, NAME_SIZE );
			p = strrchr( name, '\n' );
			if(p != NULL) *p = '\0';
			if(debug2) {
				fprintf(stderr, "Data name: %s\n",name);
			}
			continue;
		}
		/*
		 * transfer encoding ?
		 */
		if( strncasecmp( g.line, XSUNEN, sizeof(XSUNEN)-1) == 0 ) {
			p = g.line + sizeof(XSUNEN)-1;
			isUU = parseEncode( p, encoding );
			if(debug2) {
				fprintf(stderr, "Transfer encoding: %s uuencode = %i\n",p, isUU);
			}
		}
		/*
		 * content length ?
		 */
		if( strncasecmp( g.line, XSUNCLEN, sizeof(XSUNCLEN)-1) == 0 ) {
			p = g.line + sizeof(XSUNCLEN)-1;
			g.contentLength = atoi(p);
			if(debug2) {
				fprintf(stderr, "Content length: %i\n", g.contentLength);
			}
		}
		/*
		 * content lines ?
		 */
		if( strncasecmp( g.line, XSUNCL, sizeof(XSUNCL)-1) == 0 ) {
			p = g.line + sizeof(XSUNCL)-1;
			g.contentLines = atoi(p);
			if(debug2) {
				fprintf(stderr, "Content lines: %i\n", g.contentLines);
			}
		}
		/*
		 * we don't expect to find extraneous headers,
		 * but if there are any - just copy them
		 */
		else
			fputs(g.line, g.outFile);

	} /* End - look through headers for this body part */
	 
	/*
	 * Before processing the remainder of the body part,
	 * see if it needs to be uudecoded (then base64 encoded)
	 */
	if( isUU ) {
		findBegin( &name[0] );
	} 

	/*
	 * output content type header, based on input type code and
	 * input transfer encoding. 
	 */
	setContentType( type, name, isUU );

	/*
	 * output description and encoding headers, followed by blank line
	 */
	fprintf( g.outFile, "Content-Description: %s\n", desc );
	if( strlen(encoding) > (int)0 )
	   fprintf( g.outFile, "Content-Transfer-Encoding: %s\n", encoding );
	fputc('\n', g.outFile);

	/*
	 * if uuencoded body part, decode and re-encode as base64
	 */
	if ( isUU ) {
		uu2base64();
	}
	/*
	 * If not uuencoded, just pass the data through. It must just be
	 * some form of ASCII text.
	 */
	else {
		copyPart( g.outFile );
	}
	/*
	 * body part processed, end it with a blank 
	 * line and return.
	 */
	fputc('\n', g.outFile);
}
/*
 * subroutine: parse the transfer encoding line, look for uuencode
 *	return 1 is uuencoded, 0 otherwise
 *	also, the string pointed to by "encoding" is appended to.
 *
 * input:		updated string:			return val:
 * uuencode		base64				1
 * <any>, uuencode	base64				1
 * <other>		<input string>			0
 *
 * The middle case is only seen with audio files, where <any> is 
 * "adpcm-compress". The third case should never be seen, but if it is the
 * data will be passed through unchanged.
 */
int
parseEncode( char *line, char *encoding )
{
	char *p;

	/*
	 * uuencode ?
	 */
	if( strncasecmp( line, "uuencode", sizeof("uuencode")-1 ) == 0 ) {
		/*
		 * indicate what data will be after conversion
		 */
		strcpy( encoding, "base64");
		return(1);
	}
	/*
	 * two attributes, comma separated ?
	 */
	p = strchr( line, (int)',' );
	if( p == NULL ) {
		strncpy( encoding, line, ENCODE_SIZE );
		p = strrchr( encoding, '\n' );
		if (p != NULL) *p = '\0';
		return(0);
	}
	/*
	 * <any>, uuencode ?
	 */
	p++;			/* skip over the comma */
	if( *p == ' ' ) p++;	/* skip over blank, if any */
	if( strncasecmp( p, "uuencode", sizeof("uuencode")-1 ) == 0 ) {
		/*
		 * indicate what data will be after conversion
		 */
		strcpy( encoding, "base64");
		return(1);
	}
	strncpy( encoding, line, ENCODE_SIZE );
	p = strrchr( encoding, '\n' );
	if (p != NULL) *p = '\0';
	return(0);
}
/*
 * subroutine: Figure out what to put in the Content-Type header
 *	This is based partly on the input type parameter. If the type is
 *	just "binary", try to figure out if it's really a JPEG or MPEG.
 *	If just "default", see if it's maybe XWD or XBM. If it's "default"
 *	and not encoded, it's probably just ASCII text.
 */
void
setContentType( int type, char *fn, int isUU  )
{
	int  isText = 0;
	char *p;
	char cType[256] = "";

	switch( type ) {
	case T_TEXT:
		strcpy( cType, "text/plain");
		break;
	case T_PS:
		strcpy( cType, "application/postscript");
		break;
	case T_XBM:
		strcpy( cType, "image/x-xbm");
		break;
	case T_XPM:
		strcpy( cType, "image/x-xpm");
		break;
	case T_AUDIO:
		strcpy( cType, "audio/x-adpcm-compress" );
		break;
	case T_GIF:
		strcpy( cType, "image/gif" );
		break;
	case T_FRAME:
		strcpy( cType, "application/x-frame" );
		break;
	case T_UNK:
	default:
		if(isUU) {
		   /*
		    * Look at the name of the encoded file. If it has a
		    * recognizable suffix, trust it to be accurate...
		    */
		   p = strrchr( fn, '.' );
		   if( p != NULL ) {
			p++;
			if((strncasecmp(p, "jpeg", sizeof("jpeg")-1) == 0) || 
			   (strncasecmp(p, "jpg", sizeof("jpg")-1) == 0)) {
				strcpy( cType, "image/jpeg" );
			} else
			if((strncasecmp(p, "mpeg", sizeof("mpeg")-1) == 0) || 
			   (strncasecmp(p, "mpg", sizeof("mpg")-1) == 0)) {
				strcpy( cType, "image/mpeg" );
			} else
			if (strncasecmp(p, "xbm", sizeof("xbm")-1) == 0) {
				strcpy( cType, "image/x-xbm" );
			} else
			if (strncasecmp(p, "xwd", sizeof("xwd")-1) == 0) {
				strcpy( cType, "image/x-xwd" );
		/*
		 * Can't figure out what it is, but it's encoded...
		 * better just call it "binary".
		 */
			} else {
				strcpy( cType, "application/octet-stream" );
			}
		   }
		   else {
			strcpy( cType, "application/octet-stream" );
		   }
		}
		/*
		 * Can't figure out what it is, but it's not encoded...
		 * must be ASCII text. Examples include "c-file","h-file", etc.
		 */
		else {
			isText = 1;
			strcpy( cType, "text/plain");
		}
			
		break;
	}
	/*
	 * output the Content-Type header
	 */
	if(isText) {
	    fprintf(g.outFile, "Content-Type: %s\n", cType);
	    fprintf(g.outFile,
	    	    "Content-Disposition: attachment; filename=%s\n", fn);
	}
	else {
	   fprintf(g.outFile, "Content-Type: %s; name=\"%s\"\n", cType, fn);
	}
}
/*
 * subroutine: uudecode the body part and re-encode as base64
 *	note: it is assumed that the input stream is positioned to the
 *	first line past the "begin" header line.
 */
void
uu2base64()
{
	FILE *tmpFile;
	char tn[256];

	/*
	 * create a temporary file name to hold the output of "uudecode".
	 */
	tmpnam( tn );

	/*
	 * uudecode the input into the temporary file.
	 */
	uuDecode( tn );

	/*
	 * re-encode the file in "base 64" and write it to standard output
	 */
	if( (tmpFile = fopen( tn, "r" )) == NULL )
		perror("Couldn't open the temp file");
	else {
		To64( tmpFile, g.outFile, 0 );
		fclose(tmpFile);
	}

	/*
	 * remove the temporary file
	 */
	unlink (tn);
}
/*
 * subroutine: pipe uuencoded data into the uudecode command
 */
void
uuDecode( char *tn )
{
	int pfd[2];
	FILE *pout;
	int pid;

	/*
	 * open a pipe to send data to "uudecode"
	 */
	if(pipe(pfd) == -1) {
		perror("Can't create pipe");
		return;
	}
	
	/*
	 * create a new process
	 */
	if((pid = fork()) == -1) {
		perror("Can't fork");
		return;
	}

	/*
	 * if this is the child process, close one of the pipes. close
	 * stdin and dup the file descriptor for the other pipe,
	 * thus connecting stdin to that pipe. then, exec "uudecode".
	 */
	if(pid == 0) {

	/*
	 * Set the effective group id to the real group id.  We don't want to
	 *    run any child processes with the setgid permissions on.
	 */
#ifdef HAVE_SETEGID
		setegid(getgid());
#else
		gid_t	gid = getgid();
		setresgid(gid, gid, gid);
#endif
		close(pfd[1]);
		fclose(stdin);
		dup(pfd[0]);
		execl( "/usr/bin/uudecode", "uudecode", (char *)0 );
		perror("exec failed");
		return;
	}
	/*
	 * back in the parent process, close the other end of the pipe
	 */ 
	close(pfd[0]);

	/*
	 * get a file stream descriptor for the pipe
	 */
	pout = (FILE*)fdopen( pfd[1], "w" );
	if( pout == NULL ) {
		perror("Couldn't fdopen the pipe");
		return;
	}

	/*
	 * write the uuencode header line, substituting the temp file name
	 * for the output file name
	 */
	fprintf( pout, "begin 644 %s\n", tn );

	/*
	 * copy the body part
	 */
        copyPart(pout);
	fclose(pout);
	close(pfd[1]);

	/*
	 * wait for uudecode to complete
	 */
	wait(&pid);
}
/*
 * copy a body part
 *
 * input: output file descriptor
 * static vars: contentLines, contentLength, line, infile
 * returns: nothing
 */
void
copyPart( FILE *outFile )
{
   int bdySeen = 0;
   int bdyLoc;
   char *buff;
   char *srch;
   int readSize;
   /*
    * body part includes a content-lines header
    */
   if(debug1 || debug2) {
   	fprintf(stderr, "Copying a body part, len= %i, lines= %i\n",g.contentLength,g.contentLines);
   }
   if ( g.contentLines ) {
	/*
	 * copy while lines left in the message, and in the body part
	 */
	while( g.contentLines ) {
		g.contentLines--;
		if ((fgets(g.line, sizeof(g.line), g.inFile)) == NULL) {
			if(debug2) {
				fprintf(stderr, "Ran out of lines...\n");
			}
			g.inconsistent = 1;
			return;
		}
		if( strncasecmp( g.line, XSUNBDY, sizeof(XSUNBDY)-1 ) == 0 ) {
			if(debug2) {
				fprintf(stderr, "Saw a boundary while copying...\n");
			}
			bdySeen = 1;
			bdyLoc = ftell(g.inFile) - strlen(g.line);
		}
		fputs(g.line, outFile);
	}
	/*
	 * peek ahead - next line should be a boundary, or end-of-file
	 * blank lines are skipped
	 */
	while ((fgets(g.line, sizeof(g.line), g.inFile)) != NULL) {
		/*
		 * next line is a boundary, content-lines was right for a change!
		 * put back the line and return
		 */
		if( strncasecmp( g.line, XSUNBDY, sizeof(XSUNBDY)-1 ) == 0 ) {
			if(debug2) {
				fprintf(stderr, "Content-lines was correct!\n");
			}
			fseek( g.inFile, 0-strlen(g.line), SEEK_CUR);
			return;
		}
		else if (strlen(g.line) == 1 ) {
			if(debug2) {
				fprintf(stderr, "Skipping blank line at end of part.\n");
			}
			continue;
		}
		else {
			g.inconsistent = 1;
			/*
			 * already seen a boundary (or something like it)?
			 */
			if (bdySeen) {
				fseek( g.inFile, bdyLoc, SEEK_SET );
				return;
			}
			/*
			 * not yet at a boundary, keep copying until one is hit
			 */
			else {
				if(debug2) {
					fprintf(stderr, "Seeking next boundary\n");
				}
				copyLines( outFile );
			}
		}
	}
	if(debug2) {
		fprintf(stderr, "End of part is also end of message\n");
	}
   } /* End - copy using content-lines */
   /*
    * body part includes a content-length header
    */
   else if ( g.contentLength ) {
	/*
	 * allocate a buffer to hold the body part
	 */
	buff = (char *)malloc( g.contentLength );
	if ( buff == NULL ) {
		if(debug1 || debug2) {
			fprintf(stderr, "malloc failed, size = %i, copy line by line instead.\n", g.contentLength);
		}
		copyLines( outFile );
		return;
	}
	readSize = fread(buff, 1, g.contentLength, g.inFile);
	fwrite(buff, 1, readSize, outFile);
	free(buff);

	/*
	 * peek ahead - next line should be a boundary, or end-of-file
	 * blank lines are skipped
	 */
	while ((fgets(g.line, sizeof(g.line), g.inFile)) != NULL) {
		/*
		 * next line is a boundary, content-length was right for a change!
		 * put back the line and return
		 */
		if( strncasecmp( g.line, XSUNBDY, sizeof(XSUNBDY)-1 ) == 0 ) {
			if(debug2) {
				fprintf(stderr, "Content-length was correct!\n");
			}
			fseek( g.inFile, 0-strlen(g.line), SEEK_CUR);
			return;
		}
		else if (strlen(g.line) == 1 ) {
			if(debug2) {
				fprintf(stderr, "Skipping blank line at end of part.\n");
			}
			continue;
		}
		else {
			g.inconsistent = 1;
			/*
			 * already seen a boundary (or something like it)?
			 */
			srch = buff;
			while((srch = strchr(srch, '-')) != NULL) {
				if( strncasecmp( srch, XSUNBDY, sizeof(XSUNBDY)-1 ) == 0 ) {
					if(debug2) {
						fprintf(stderr, "Saw a boundary while copying...\n");
					}
					bdySeen = 1;
					bdyLoc = ftell(g.inFile) - strlen(g.line);
					break;
				}
				srch++;
			}
			if (bdySeen) {
				bdyLoc = ftell(g.inFile) - strlen(g.line);
				return;
			}
			
			/*
			 * not yet at a boundary, keep copying until one is hit
			 */
			else {
				if(debug2) {
					fprintf(stderr, "Seeking next boundary\n");
				}
				copyLines( outFile );
			}
		}
	}
	if(debug2) {
		fprintf(stderr, "End of part is also end of message\n");
	}
   } /* End - copy using content-length */
   /*
    * body part has neither content-lines nor content-length header
    */
   else {
	g.inconsistent = 1;
	if(debug2) {
		fprintf(stderr, "Copying a part with neither content-len nor lines\n");
	}
	copyLines( outFile );
   }
}
/*
 * copy lines of a body part without regard to content-length or
 * content-lines
 */
void
copyLines ( FILE *outFile )
{
	while((fgets(g.line, sizeof(g.line), g.inFile)) != NULL) {
		if( strncasecmp( g.line, XSUNBDY, sizeof(XSUNBDY)-1 ) == 0 ) {
			if(debug2) {
				fprintf(stderr, "Copying line by line, hit a boundary\n");
			}
			fseek( g.inFile, 0-strlen(g.line), SEEK_CUR);
			return;
		}
		fputs(g.line, outFile);
	}
	if(debug2) {
		fprintf(stderr, "Copying line by line, ran out of lines\n");
	}
}
/*
 * find the "begin" line in the uuencoded data, and
 * extract the file name. format of the line is:
 * 	begin <mode> <file name>
 */
void
findBegin( char *name )
{
	char *p;
	while((fgets(g.line, sizeof(g.line), g.inFile)) != NULL) {
		/*
		 * bail out if boundary reached - shouldn't happen
		 */
		if( strncasecmp( g.line, XSUNBDY, sizeof(XSUNBDY)-1 ) == 0 ) {
			fseek( g.inFile, 0-strlen(g.line), SEEK_CUR);
			break;
		}
		/*
		 * decrement content length/lines
		 */
		if (g.contentLength) {
			g.contentLength -= strlen(g.line);
			if(debug2) {
				if (g.contentLength<0) fprintf(stderr,"NEGATIVE content length!\n");
			}
			if (g.contentLength < 0) g.contentLength = 0;
		}
		if (g.contentLines) {
			g.contentLines--;
			if(debug2) {
				if (g.contentLines<0) fprintf(stderr,"NEGATIVE content lines!\n");
			}
			if (g.contentLines < 0) g.contentLines = 0;
		}
		if( strncasecmp(g.line, "begin ", sizeof("begin ")-1 ) == 0 ) {
			p = strchr(g.line, (int)' ');
			if(p == NULL) break;
			p = strchr(++p, (int)' ');
			if(p == NULL) break;
			strncpy(name, ++p, NAME_SIZE);
			p = strrchr(name, '\n');
			if(p != NULL) *p = '\0';
			if(debug2) {
				fprintf(stderr,"BEGIN line found, name = %s\n",name);
			}
			break;
		}
	}
} /* end findBegin */

/*
 * base64 conversion
 */

/*
Copyright (c) 1991 Bell Communications Research, Inc. (Bellcore)

Permission to use, copy, modify, and distribute this material 
for any purpose and without fee is hereby granted, provided 
that the above copyright notice and this permission notice 
appear in all copies, and that the name of Bellcore not be 
used in advertising or publicity pertaining to this 
material without the specific, prior written permission 
of an authorized representative of Bellcore.  BELLCORE 
MAKES NO REPRESENTATIONS ABOUT THE ACCURACY OR SUITABILITY 
OF THIS MATERIAL FOR ANY PURPOSE.  IT IS PROVIDED "AS IS", 
WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES.
*/

static int	InNewline = 0;

/* the following gets a character, but fakes it properly into two chars if there's a newline character */

int
nextcharin(FILE *infile, int PortableNewlines)
{
    int c;

    if (!PortableNewlines) return(getc(infile));
    if (InNewline) {
        InNewline = 0;
        return(10); /* LF */
    }
    c = getc(infile);
    if (c == '\n') {
        InNewline = 1;
        return(13); /* CR */
    }
    return(c);
}


int
output64chunk(int c1, int c2, int c3, int pads, FILE *outfile)
{
   static char	basis_64[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
   int	status = 0;

   status = putc(basis_64[c1>>2], outfile);
   if ( status == EOF ) return 0;

   status = putc(basis_64[((c1 & 0x3)<< 4) | ((c2 & 0xF0) >> 4)], outfile);
   if ( status == EOF ) return 0;

   if (pads == 2) {

      status = putc('=', outfile);
      if ( status == EOF ) return 0;

      status = putc('=', outfile);
      if ( status == EOF ) return 0;
   }
   else if (pads) {

      status = putc(basis_64[((c2 & 0xF) << 2) | ((c3 & 0xC0) >>6)], outfile);
      if ( status == EOF ) return 0;

      status = putc('=', outfile);
      if ( status == EOF ) return 0;
   }
   else {

      status = putc(basis_64[((c2 & 0xF) << 2) | ((c3 & 0xC0) >>6)], outfile);
      if ( status == EOF ) return 0;

      status = putc(basis_64[c3 & 0x3F], outfile);
      if ( status == EOF ) return 0;
   }

   return 1;
}

int
To64(FILE *infile, FILE *outfile, int PortableNewlines) 
{
    int c1, c2, c3, ct=0;
    int	error = 0;
    InNewline = 0; /* always reset it */

    while ( !error && (c1 = nextcharin(infile, PortableNewlines)) != EOF) {

        c2 = nextcharin(infile, PortableNewlines);
        if (c2 == EOF) {
            error = !output64chunk(c1, 0, 0, 2, outfile);
        } else {
            c3 = nextcharin(infile, PortableNewlines);
            if (c3 == EOF) {
                error = !output64chunk(c1, c2, 0, 1, outfile);
            } else {
                error = !output64chunk(c1, c2, c3, 0, outfile);
            }
        }
        ct += 4;
        if (ct > 71) {
            error = (putc('\n', outfile) == EOF);
            ct = 0;
        }
    }

    if (!error && ct) error = (putc('\n', outfile) == EOF);
    error = (fflush(outfile) == EOF);

    return !error;
}
