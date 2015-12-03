/*
 *  $Id: MimeTypes.C,v 1.3 2001/01/03 09:54:49 evgeny Exp $
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
#include "MimeTypes.h"

MimeAccessType
AccessType(CharC type)
{
   if ( type.Equals(LOCAL_FILE_S,  IGNORE_CASE) ||
        type.Equals("afs",	   IGNORE_CASE) ) return AT_LOCAL_FILE;
   if ( type.Equals(ANON_FTP_S,    IGNORE_CASE) ) return AT_ANON_FTP;
   if ( type.Equals(FTP_S,         IGNORE_CASE) ) return AT_FTP;
   if ( type.Equals(TFTP_S,        IGNORE_CASE) ) return AT_TFTP;
   if ( type.Equals(MAIL_SERVER_S, IGNORE_CASE) ) return AT_MAIL_SERVER;

   return AT_INLINE;

} // End AccessType

MimeContentType
ContentType(CharC type)
{
   if ( type.StartsWith("text/", IGNORE_CASE) ) {
      if ( type.Equals("text/plain",    IGNORE_CASE) ) return CT_PLAIN;
      if ( type.Equals("text/enriched", IGNORE_CASE) ) return CT_ENRICHED;
      if ( type.Equals("text/richtext", IGNORE_CASE) ) return CT_RICH;
      if ( type.Equals("text/html", 	IGNORE_CASE) ) return CT_HTML;
      return CT_UNKNOWN;
   }

   if ( type.StartsWith("application/", IGNORE_CASE) ) {
      if ( type.Equals("application/octet-stream", IGNORE_CASE) )
	 return CT_OCTET;
      if ( type.Equals("application/postscript",   IGNORE_CASE) )
	 return CT_POSTSCRIPT;
      if ( type.Equals("application/pgp-signature",   IGNORE_CASE) )
	 return CT_PGP_SIG;
      if ( type.Equals("application/pgp-encrypted",   IGNORE_CASE) )
	 return CT_PGP_ENC;
      return CT_UNKNOWN;
   }

   if ( type.StartsWith("image/", IGNORE_CASE) ) {
      if ( type.Equals("image/gif",  IGNORE_CASE) ) return CT_GIF;
      if ( type.Equals("image/jpeg", IGNORE_CASE) ) return CT_JPEG;
      return CT_UNKNOWN;
   }

   if ( type.StartsWith("audio/", IGNORE_CASE) ) {
      if ( type.Equals("audio/basic", IGNORE_CASE) ) return CT_BASIC_AUDIO;
      return CT_UNKNOWN;
   }

   if ( type.StartsWith("message/", IGNORE_CASE) ) {
      if ( type.Equals("message/rfc822",   IGNORE_CASE) ) return CT_RFC822;
      if ( type.Equals("message/external-body", IGNORE_CASE) )
	 return CT_EXTERNAL;
      if ( type.Equals("message/partial",  IGNORE_CASE) ) return CT_PARTIAL;
      return CT_UNKNOWN;
   }

   if ( type.StartsWith("multipart/", IGNORE_CASE) ) {
      if ( type.Equals("multipart/mixed",       IGNORE_CASE) )
	 return CT_MIXED;
      if ( type.Equals("multipart/digest",      IGNORE_CASE) )
	 return CT_DIGEST;
      if ( type.Equals("multipart/alternative", IGNORE_CASE) )
	 return CT_ALTERNATIVE;
      if ( type.Equals("multipart/parallel",    IGNORE_CASE) )
	 return CT_PARALLEL;
      if ( type.Equals("multipart/signed",    IGNORE_CASE) )
	 return CT_SIGNED;
      if ( type.Equals("multipart/encrypted",    IGNORE_CASE) )
	 return CT_ENCRYPTED;
      return CT_UNKNOWN;
   }

   if ( type.StartsWith("video/", IGNORE_CASE) ) {
      if ( type.Equals("video/mpeg", IGNORE_CASE) ) return CT_MPEG;
      return CT_UNKNOWN;
   }

   return CT_UNKNOWN;

} // End ContentType

MimeEncodingType
EncodingType(CharC type)
{
   if ( type.Length() == 0 ) return ET_NONE;

   if ( type.Equals(SEVEN_BIT_S,        IGNORE_CASE) ) return ET_7BIT;
   if ( type.Equals(EIGHT_BIT_S,        IGNORE_CASE) ) return ET_8BIT;
   if ( type.Equals(QUOTED_PRINTABLE_S, IGNORE_CASE) ) return ET_QP;
   if ( type.Equals(BASE64_S,           IGNORE_CASE) ) return ET_BASE_64;
   if ( type.Equals(BINARY_S,           IGNORE_CASE) ) return ET_BINARY;
   if ( type.Equals(UUENCODE_S,         IGNORE_CASE) ) return ET_UUENCODE;
   if ( type.Equals(BINHEX_S,           IGNORE_CASE) ) return ET_BINHEX;

   return ET_UNKNOWN;

} // End EncodingType

MimeGroupType
GroupType(CharC type)
{
   if ( type.Equals("text",		IGNORE_CASE) ||
	type.StartsWith("text/",	IGNORE_CASE) ) return GT_TEXT;
   if ( type.Equals("application",	IGNORE_CASE) ||
	type.StartsWith("application/",	IGNORE_CASE) ) return GT_APPLICATION;
   if ( type.Equals("image",		IGNORE_CASE) ||
	type.StartsWith("image/",	IGNORE_CASE) ) return GT_IMAGE;
   if ( type.Equals("audio",		IGNORE_CASE) ||
	type.StartsWith("audio/",	IGNORE_CASE) ) return GT_AUDIO;
   if ( type.Equals("message",		IGNORE_CASE) ||
	type.StartsWith("message/",	IGNORE_CASE) ) return GT_MESSAGE;
   if ( type.Equals("multipart",	IGNORE_CASE) ||
	type.StartsWith("multipart/",	IGNORE_CASE) ) return GT_MULTIPART;
   if ( type.Equals("video",		IGNORE_CASE) ||
	type.StartsWith("video/",	IGNORE_CASE) ) return GT_VIDEO;
   return GT_UNKNOWN;

} // End GroupType

CharC
AccessTypeStr(MimeAccessType type)
{
   switch (type) {
      case AT_INLINE:		return "in-line";
      case AT_LOCAL_FILE:	return LOCAL_FILE_S;
      case AT_ANON_FTP:		return ANON_FTP_S;
      case AT_FTP:		return FTP_S;
      case AT_TFTP:		return TFTP_S;
      case AT_MAIL_SERVER:	return MAIL_SERVER_S;
   }

   return "";

} // End AccessTypeStr

CharC
ContentTypeStr(MimeContentType type)
{
   switch (type) {

      case CT_PLAIN:		return "text/plain";
      case CT_RICH:		return "text/richtext";
      case CT_ENRICHED:		return "text/enriched";
      case CT_MIXED:		return "multipart/mixed";
      case CT_ALTERNATIVE:	return "multipart/alternative";
      case CT_DIGEST:		return "multipart/digest";
      case CT_PARALLEL:		return "multipart/parallel";
      case CT_SIGNED:		return "multipart/signed";
      case CT_ENCRYPTED:	return "multipart/encrypted";
      case CT_RFC822:		return "message/rfc822";
      case CT_PARTIAL:		return "message/partial";
      case CT_EXTERNAL:		return "message/external-body";
      case CT_OCTET:		return "application/octet-stream";
      case CT_POSTSCRIPT:	return "application/postscript";
      case CT_PGP_SIG:		return "application/pgp-signature";
      case CT_PGP_ENC:		return "application/pgp-encrypted";
      case CT_GIF:		return "image/gif";
      case CT_JPEG:		return "image/jpeg";
      case CT_BASIC_AUDIO:	return "audio/basic";
      case CT_MPEG:		return "video/mpeg";
      case CT_UNKNOWN:		return "unknown";
   }

   return "";

} // End ContentTypeStr

CharC
EncodingTypeStr(MimeEncodingType type)
{
   switch (type) {
      case ET_NONE:	return "none";
      case ET_7BIT:	return SEVEN_BIT_S;
      case ET_8BIT:	return EIGHT_BIT_S;
      case ET_BASE_64:	return BASE64_S;
      case ET_BINARY:	return BINARY_S;
      case ET_BINHEX:	return BINHEX_S;
      case ET_QP:	return QUOTED_PRINTABLE_S;
      case ET_UUENCODE:	return UUENCODE_S;
      case ET_UNKNOWN:	return "unknown";
   }

   return "";

} // End EncodingTypeStr

CharC
GroupTypeStr(MimeGroupType type)
{
   switch (type) {
      case GT_TEXT:		return TEXT_S;
      case GT_MULTIPART:	return MULTIPART_S;
      case GT_MESSAGE:		return MESSAGE_S;
      case GT_APPLICATION:	return APPLICATION_S;
      case GT_IMAGE:		return IMAGE_S;
      case GT_AUDIO:		return AUDIO_S;
      case GT_VIDEO:		return VIDEO_S;
      case GT_UNKNOWN:		return "unknown";
   }

   return "";

} // End GroupTypeStr

