/*
 *  $Id: MimeTypes.h,v 1.2 2001/01/03 09:54:49 evgeny Exp $
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
#ifndef _MimeTypes_h_
#define _MimeTypes_h_

#include <hgl/CharC.h>
#include <X11/Intrinsic.h>

//
// Registered MIME content types
//
   enum MimeGroupType {
      GT_TEXT,
      GT_MULTIPART,
      GT_MESSAGE,
      GT_APPLICATION,
      GT_IMAGE,
      GT_AUDIO,
      GT_VIDEO,
      GT_UNKNOWN
   };

//
// Shorthand
//
   inline Boolean IsApp(MimeGroupType g)	{ return g == GT_APPLICATION; }
   inline Boolean IsAudio(MimeGroupType g)	{ return g == GT_AUDIO; }
   inline Boolean IsImage(MimeGroupType g)	{ return g == GT_IMAGE; }
   inline Boolean IsMessage(MimeGroupType g)	{ return g == GT_MESSAGE; }
   inline Boolean IsMultipart(MimeGroupType g)	{ return g == GT_MULTIPART; }
   inline Boolean IsText(MimeGroupType g)	{ return g == GT_TEXT; }
   inline Boolean IsUnknown(MimeGroupType g)	{ return g == GT_UNKNOWN; }
   inline Boolean IsVideo(MimeGroupType g)	{ return g == GT_VIDEO; }

//
// Registered MIME subtypes
//
   enum MimeContentType {
      CT_PLAIN,		// text/plain
      CT_RICH,		// text/richtext
      CT_ENRICHED,	// text/enriched
      CT_HTML,		// text/html
      CT_MIXED,		// multipart/mixed
      CT_ALTERNATIVE,	// multipart/alternative
      CT_DIGEST,	// multipart/digest
      CT_PARALLEL,	// multipart/parallel
      CT_SIGNED,	// multipart/signed
      CT_ENCRYPTED,	// multipart/encrypted
      CT_RFC822,	// message/rfc822
      CT_PARTIAL,	// message/partial
      CT_EXTERNAL,	// message/external-body
      CT_OCTET,		// application/octet-stream
      CT_POSTSCRIPT,	// application/postscript
      CT_PGP_SIG,	// application/pgp-signature
      CT_PGP_ENC,	// application/pgp-encrypted
      CT_GIF,		// image/gif
      CT_JPEG,		// image/jpeg
      CT_BASIC_AUDIO,	// audio/basic
      CT_MPEG,		// video/mpeg
      CT_UNKNOWN
   };

//
// Shorthand
//
   inline Boolean Is822(MimeContentType c)	 { return c == CT_RFC822; }
   inline Boolean IsAlt(MimeContentType c)	 { return c == CT_ALTERNATIVE; }
   inline Boolean IsBasic(MimeContentType c)	 { return c == CT_BASIC_AUDIO; }
   inline Boolean IsOctet(MimeContentType c)	 { return c == CT_OCTET; }
   inline Boolean IsDigest(MimeContentType c)	 { return c == CT_DIGEST; }
   inline Boolean IsEncrypted(MimeContentType c) { return c == CT_ENCRYPTED; }
   inline Boolean IsEnriched(MimeContentType c)	 { return c == CT_ENRICHED; }
   inline Boolean IsGIF(MimeContentType c)	 { return c == CT_GIF; }
   inline Boolean IsGif(MimeContentType c)	 { return c == CT_GIF; }
   inline Boolean IsJPEG(MimeContentType c)	 { return c == CT_JPEG; }
   inline Boolean IsJpeg(MimeContentType c)	 { return c == CT_JPEG; }
   inline Boolean IsMixed(MimeContentType c)	 { return c == CT_MIXED; }
   inline Boolean IsMPEG(MimeContentType c)	 { return c == CT_MPEG; }
   inline Boolean IsMpeg(MimeContentType c)	 { return c == CT_MPEG; }
   inline Boolean IsParallel(MimeContentType c)	 { return c == CT_PARALLEL; }
   inline Boolean IsPartial(MimeContentType c)	 { return c == CT_PARTIAL; }
   inline Boolean IsPgpSig(MimeContentType c)	 { return c == CT_PGP_SIG; }
   inline Boolean IsPgpEnc(MimeContentType c)	 { return c == CT_PGP_ENC; }
   inline Boolean IsPlain(MimeContentType c)	 { return c == CT_PLAIN; }
   inline Boolean IsPlainText(MimeContentType c) { return c == CT_PLAIN; }
   inline Boolean IsPostScript(MimeContentType c){ return c == CT_POSTSCRIPT; }
   inline Boolean IsRich(MimeContentType c)	 { return c == CT_RICH; }
   inline Boolean IsRichText(MimeContentType c)	 { return c == CT_RICH; }
   inline Boolean IsSigned(MimeContentType c)	 { return c == CT_SIGNED; }
   inline Boolean IsUnknown(MimeContentType c)	 { return c == CT_UNKNOWN; }

//
// Registered MIME encoding methods
//
   enum MimeEncodingType {
      ET_NONE,
      ET_7BIT,
      ET_8BIT,
      ET_BASE_64,
      ET_BINARY,
      ET_BINHEX,
      ET_QP,
      ET_UUENCODE,
      ET_UNKNOWN
   };

//
// Shorthand
//
   inline Boolean Is7Bit(MimeEncodingType e)	{ return e == ET_7BIT; }
   inline Boolean Is8Bit(MimeEncodingType e)	{ return e == ET_8BIT; }
   inline Boolean IsBase64(MimeEncodingType e)	{ return e == ET_BASE_64; }
   inline Boolean IsBinary(MimeEncodingType e)	{ return e == ET_BINARY; }
   inline Boolean IsBinHex(MimeEncodingType e)	{ return e == ET_BINHEX; }
   inline Boolean IsQP(MimeEncodingType e)	{ return e == ET_QP;}
   inline Boolean IsQuoted(MimeEncodingType e)	{ return e == ET_QP;}
   inline Boolean IsUUencode(MimeEncodingType e){ return e == ET_UUENCODE; }
   inline Boolean IsUU(MimeEncodingType e)      { return e == ET_UUENCODE; }
   inline Boolean IsEncoded(MimeEncodingType e)
      { return (IsBase64(e) || IsQP(e) || IsUU(e) || IsBinHex(e)); }

//
// Registered MIME access methods for external body parts
//
   enum MimeAccessType {
      AT_INLINE,		// If part not external
      AT_LOCAL_FILE,
      AT_ANON_FTP,
      AT_FTP,
      AT_TFTP,
      AT_MAIL_SERVER
   };

//
// Shorthand
//
   inline Boolean IsAnon(MimeAccessType a)	{ return a == AT_ANON_FTP; }
   inline Boolean IsExternal(MimeAccessType a)	{ return a != AT_INLINE; }
   inline Boolean IsFTP(MimeAccessType a)	{ return a == AT_FTP; }
   inline Boolean IsFtp(MimeAccessType a)	{ return a == AT_FTP; }
   inline Boolean IsInline(MimeAccessType a)	{ return a == AT_INLINE; }
   inline Boolean IsLocal(MimeAccessType a)	{ return a == AT_LOCAL_FILE; }
   inline Boolean IsMail(MimeAccessType a)	{ return a == AT_MAIL_SERVER; }
   inline Boolean IsTFTP(MimeAccessType a)	{ return a == AT_TFTP; }
   inline Boolean IsTftp(MimeAccessType a)	{ return a == AT_TFTP; }

//
// String constants
//
#define ACCESS_TYPE_S		"access-type"
#define ALTERNATIVE_S		"alternative"
#define ANON_FTP_S		"anon-ftp"
#define APPLICATION_S		"application"
#define AUDIO_S			"audio"
#define BASE64_S		"base64"
#define BASIC_S			"basic"
#define BINARY_S		"binary"
#define BINHEX_S		"binhex"
#define BOUNDARY_S		"boundary"
#define DIGEST_S		"digest"
#define EIGHT_BIT_S		"8bit"
#define ENRICHED_S		"enriched"
#define EXTERNAL_BODY_S		"external-body"
#define FILENAME_S		"filename"
#define FTP_S			"ftp"
#define GIF_S			"gif"
#define IMAGE_S			"image"
#define JPEG_S			"jpeg"
#define LOCAL_FILE_S		"local-file"
#define MAIL_SERVER_S		"mail-server"
#define MESSAGE_S		"message"
#define MIXED_S			"mixed"
#define MPEG_S			"mpeg"
#define MULTIPART_S		"multipart"
#define NAME_S			"name"
#define OCTET_STREAM_S		"octet-stream"
#define PARALLEL_S		"parallel"
#define PARTIAL_S		"partial"
#define PLAIN_S			"plain"
#define POSTSCRIPT_S		"postscript"
#define QUOTED_PRINTABLE_S	"quoted-printable"
#define RFC822_S		"rfc822"
#define RICHTEXT_S		"richtext"
#define SEVEN_BIT_S		"7bit"
#define	TEXT_S			"text"
#define TFTP_S			"tftp"
#define UNKNOWN_S		"x-unknown"
#define UUENCODE_S		"x-uuencode"
#define VIDEO_S			"video"

//
// Functions
//
   extern MimeAccessType	AccessType  (CharC);
   extern MimeContentType	ContentType (CharC);
   extern MimeEncodingType	EncodingType(CharC);
   extern MimeGroupType		GroupType   (CharC);
   extern CharC			AccessTypeStr  (MimeAccessType);
   extern CharC			ContentTypeStr (MimeContentType);
   extern CharC			EncodingTypeStr(MimeEncodingType);
   extern CharC			GroupTypeStr   (MimeGroupType);

#endif // _MimeTypes_h_
