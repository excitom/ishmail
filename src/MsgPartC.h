/*
 *  $Id: MsgPartC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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
#ifndef _MsgPartC_h_
#define _MsgPartC_h_

#include "MimeTypes.h"
#include "TermFn.h"

#include <hgl/StringC.h>

#include <X11/Intrinsic.h>

class ParamC;
class CharC;
class HeaderC;
class HeaderValC;
class MsgC;
class FileMsgC;

class MsgPartC {

   FileMsgC	*childMsg;	// If type is message/rfc822

   void		Init();
   void		Delete();
   void		CheckHead(HeaderC*);
   Boolean	Decode(FILE*);
   Boolean	GetText(StringC&, FILE*, int, int, Boolean) const;
   Boolean	CopyText(char*, FILE*, FILE*, int, int, Boolean, Boolean,
   			 Boolean) const;

public:

//
// Type information
//
   MimeContentType	conType;
   MimeGroupType	grpType;
   MimeAccessType	accType;
   MimeEncodingType	encType;

   StringC		conStr;		// type/subtype
   StringC		grpStr;		// type
   StringC		subStr;		// subtype

   ParamC		*conParams;	// Content-Type parameters
   ParamC		*accParams;	// External-Body parameters
   ParamC		*disParams;	// Content-Disposition parameters

   Boolean		defConType;	// If no Content-Type header

//
// Message source information
//
   StringC		msgFile;
   Boolean		delMsgFile;	// If it can be deleted when we're done

//
// Part totals
//
   int			offset;
   int			bytes;
   int			lines;

//
// Header totals (headOffset == offset)
//
   int			headBytes;
   int			headLines;

//
// Totals for any external headers
//
   int			extOffset;
   int			extBytes;
   int			extLines;
   int			extBlank;	// May or may not be there

//
// Totals for source body
//
   int			bodyOffset;
   int			bodyBytes;
   int			bodyLines;

   Boolean		headScanned;
   Boolean		bodyScanned;

//
// Lists of headers and external headers
//
   HeaderC		*headers;
   HeaderC		*extHeaders;		// In message/external-body
   HeaderC		*subject;		// In message/rfc822

//
// External file or decoded data file
//
   StringC		dataFile;	// Real file (external or decoded)
   Boolean		delDataFile;	// If it can be deleted when we're done

//
// Structure information
//
   MsgPartC		*child;		// If multipart
   MsgPartC		*parent;	// If in multipart
   MsgPartC		*prev;		// If in multipart
   MsgPartC		*next;		// If in multipart

//
// Data and methods used only when displaying a part
//
   MsgC			*parentMsg;
   StringC		userName;	// For FTP
   StringC		userPass;	// For FTP
   StringC		partNum;

   Boolean		NeedFetch();
   FileMsgC		*ChildMsg();

//
// Public methods
//
   MsgPartC(MsgPartC *par=NULL);
   ~MsgPartC();

   void			Reset();

//
// Write part to a file
//
   Boolean		WriteFile(char *name, FILE *fp=NULL);

//
// Parse body part
//
   Boolean		 Scan(FILE *fp, int maxBytes, TermFn*,
			      char *nextBound=NULL, Boolean *gotLast=NULL,
			      Boolean *prevBlank=NULL);
   Boolean		 ScanHead(FILE *fp=NULL);
   void			 Print(ostream& strm) const;
   int			 GetPartNumberMaxSize();
   void			 Move(long, long);             // Change offset
   void			 SetPartNumber(char*);
   void			 SetPartNumberSize(int);
   void			 SetType(CharC);
   void			 SetDisposition(CharC);
   void			 SetEncoding(CharC);
   void			 SetSubject(CharC);
   MsgPartC		*BestAlternative(Boolean forPrinting=False) const;
   MsgPartC		*BestTextAlternative() const;
   MsgPartC		*FindPart(CharC);
   void			 AddHeader(CharC);
   void			 AddExtHeader(CharC);
   void			 ComputeSize();
   void			 InvalidateSize();

//
// Return contents (both encoded and decoded forms)
//
   void			 GetFileName(StringC&) const;
   Boolean	 	 CreateDataFile();
   void			 GetDescription(StringC&) const;
   void			 GetDisposition(StringC&) const;
   Boolean	 	 GetData(StringC&, FILE *fp=NULL);
   Boolean	 	 GetText(StringC&, FILE *fp=NULL,
				 Boolean getHead=False,
				 Boolean getExtHead=False,
				 Boolean getBody=True,
				 Boolean restoreFroms=False) const;
   void			 GetLabel(StringC&) const;
   void			 GetLabel(StringC&);
   Boolean	 	 CopyText(char *ofile, FILE *ofp, FILE *ifp=NULL,
				  Boolean getHead=False,
				  Boolean getExtHead=False,
				  Boolean getBody=True,
				  Boolean protectFroms=False,
				  Boolean restoreFroms=False,
				  Boolean endWithNL=False) const;

//
// Look for specific parameters
//
   void			 AddParam(CharC key, CharC val, ParamC*);
   void			 AddConParam(CharC key, CharC val);
   void			 AddAccParam(CharC key, CharC val);
   void			 AddDisParam(CharC key, CharC val);
   ParamC		*Param(CharC) const;
   ParamC		*Param(CharC, ParamC *list) const;

//
// Look for specific headers
//
   HeaderC		*Header        (CharC) const;
   HeaderValC		*HeaderValue   (CharC) const;
   void			 GetHeaderValue(CharC, StringC&) const;

//
// Shorthand
//
   inline Boolean	Is7Bit()	const { return ::Is7Bit(encType); }
   inline Boolean	Is8Bit()	const { return ::Is8Bit(encType); }
   inline Boolean	IsBase64()	const { return ::IsBase64(encType); }
   inline Boolean	IsBinary()	const { return ::IsBinary(encType); }
   inline Boolean	IsQuoted()	const { return ::IsQuoted(encType); }
   inline Boolean	IsUUencoded()	const { return ::IsUU(encType); }
   inline Boolean	IsEncoded()	const { return ::IsEncoded(encType); }

   inline Boolean	IsAnon()	const { return ::IsAnon(accType); }
   inline Boolean	IsExternal()	const { return ::IsExternal(accType); }
   inline Boolean	IsFTP()		const { return ::IsFTP(accType); }
   inline Boolean	IsInline()	const { return ::IsInline(accType); }
   inline Boolean	IsLocal()	const { return ::IsLocal(accType); }
   inline Boolean	IsMail()	const { return ::IsMail(accType); }
   inline Boolean	IsTFTP()	const { return ::IsTFTP(accType); }

   inline Boolean	IsApp()		const { return ::IsApp(grpType); }
   inline Boolean	IsAudio()	const { return ::IsAudio(grpType); }
   inline Boolean	IsImage()	const { return ::IsImage(grpType); }
   inline Boolean	IsMessage()	const { return ::IsMessage(grpType); }
   inline Boolean	IsMultipart()	const { return ::IsMultipart(grpType); }
   inline Boolean	IsText()	const { return ::IsText(grpType); }
   inline Boolean	IsVideo()	const { return ::IsVideo(grpType); }

   inline Boolean	Is822()		const { return ::Is822(conType); }
   inline Boolean	IsAlternative()	const { return ::IsAlt(conType); }
   inline Boolean	IsBasicAudio()	const { return ::IsBasic(conType); }
   inline Boolean	IsDigest()	const { return ::IsDigest(conType); }
   inline Boolean	IsEncrypted()	const { return ::IsEncrypted(conType); }
   inline Boolean	IsEnriched()	const { return ::IsEnriched(conType); }
   inline Boolean	IsGIF()		const { return ::IsGIF(conType); }
   inline Boolean	IsJPEG()	const { return ::IsJPEG(conType); }
   inline Boolean	IsMixed()	const { return ::IsMixed(conType); }
   inline Boolean	IsMPEG()	const { return ::IsMPEG(conType); }
   inline Boolean	IsOctet()	const { return ::IsOctet(conType); }
   inline Boolean	IsPartial()	const { return ::IsPartial(conType); }
   inline Boolean	IsParallel()	const { return ::IsParallel(conType); }
   inline Boolean	IsPgpSig()	const { return ::IsPgpSig(conType); }
   inline Boolean	IsPgpEnc()	const { return ::IsPgpEnc(conType); }
   inline Boolean	IsPlainText()	const { return ::IsPlain(conType); }
   inline Boolean	IsPostScript()	const { return ::IsPostScript(conType);}
   inline Boolean	IsRichText()	const { return ::IsRich(conType); }
   inline Boolean	IsSigned()	const { return ::IsSigned(conType); }
   inline Boolean	IsUnknown()	const { return ::IsUnknown(conType); }

   Boolean		IsAttachment()	const;
   inline Boolean	IsInlineText()	const
      { return (IsText() && !IsExternal() && !IsAttachment()); }

   int			ChildCount() const;     // Number of direct children
   int			LeafCount() const;      // Number of descendents
   Boolean		PlainTextOnly() const;
};

//
// Method to allow printing
//

inline ostream&
operator<<(ostream& strm, const MsgPartC& mp)
{
   mp.Print(strm);
   return strm;
}

#endif // _MsgPartC_h_
