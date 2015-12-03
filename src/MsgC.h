/*
 * $Id: MsgC.h,v 1.2 2000/09/19 16:42:02 evgeny Exp $
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

#ifndef _MsgC_h_
#define _MsgC_h_

#include "MsgStatus.h"

#include <hgl/CharC.h>
#include <hgl/StringC.h>

#include <X11/Intrinsic.h>

enum MsgTypeT {
   MH_MSG,	// A message in an MH   folder (MhMsgC::MsgC)
   UNIX_MSG,	// A message in a  Unix folder (UnixMsgC::FilePartMsgC::MsgC)
   MMDF_MSG,	// A message in an MMDF folder (MmdfMsgC::FilePartMsgC::MsgC)
   IMAP_MSG,	// A message in an IMAP folder (ImapMsgC::MsgC)
   FILE_MSG	// A message in a file (FileMsgC::MsgC)
};

class	RegexC;
class	HeaderC;
class	HeaderValC;
class	StringC;
class	AddressC;
class	MsgPartC;
class	FolderC;
class	MsgItemC;
class	RuleDictC;

class MsgC {

protected:

   time_t	 epochTime;	// Numerical value of date

   MsgPartC	*body;
   Boolean	bodySizeKnown;

   AddressC	*from;
   AddressC	*to;
   AddressC	*cc;
   AddressC	*replyTo;
   AddressC	*returnPath;
   AddressC	*sender;
   char		*thread;
   StringC	msgId;
   int		number;

   void			AddAddress(HeaderC*, AddressC**);
   void			CheckHeader(HeaderC*);
   Boolean		ReadBodyText(FILE*, StringC&) const;
   Boolean		ReadHeaderText(FILE*, StringC&) const;
   void			GetThread(HeaderC*);
   virtual void		ScanBody() = 0;
   void			ScanBodyFile(FILE*);
   void			ScanHeadFile(FILE*);
   virtual void		*TerminatingLineFn();


public:

   static RegexC	*rePat;
   FolderC		*folder;
   MsgItemC		*icon;
   MsgTypeT		type;

   MsgC(FolderC *fld);
   virtual ~MsgC();

//
// Operators
//
   inline int	compare   (const MsgC& m) const { return (this != &m); }
   inline int	operator==(const MsgC& m) const { return (compare(m) == 0); }
   inline int	operator!=(const MsgC& m) const { return (compare(m) != 0); }
   inline int	operator< (const MsgC& m) const { return (compare(m) <  0); }
   inline int	operator> (const MsgC& m) const { return (compare(m) >  0); }

//
// Status flags
//
   u_int	 	status;		// Status flags
   long			indexOffset;	// Offset in index file

   virtual void		ClearStatus(MsgStatusT, Boolean write=True);
   void			GetStatusString(StringC&) const;
   Boolean		IsSet(MsgStatusT) const;
   virtual void		SetStatus(MsgStatusT, Boolean write=True);
   Boolean		ReadIndex(FILE*);
   void			WriteIndex(FILE*);

   virtual Boolean	HasOpenFroms() const { return False; }
   virtual Boolean	HasOpenFroms()       { return False; }
   virtual Boolean	HasSafeFroms() const { return False; }
   virtual Boolean	HasSafeFroms()       { return False; }

   Boolean		IsMh()		const { return (type == MH_MSG); }
   Boolean		IsUnix()	const { return (type == UNIX_MSG); }
   Boolean		IsMmdf()	const { return (type == MMDF_MSG); }
   Boolean		IsImap()	const { return (type == IMAP_MSG); }
   Boolean		IsFile()	const { return (type == FILE_MSG); }

   Boolean		IsChanged()	const { return  IsSet(MSG_CHANGED); }
   Boolean		IsCurrent()	const { return  IsSet(MSG_VIEWED); }
   Boolean		IsDeleted()	const { return  IsSet(MSG_DELETED); }
   Boolean		IsFiltered()	const { return  IsSet(MSG_FILTERED); }
   Boolean		IsForwarded()	const { return  IsSet(MSG_FORWARDED); }
   Boolean		IsMime()	const { return  IsSet(MSG_MIME); }
   Boolean		IsNew()		const { return  IsSet(MSG_NEW); }
   Boolean		IsPartial()	const { return  IsSet(MSG_PARTIAL); }
   Boolean		IsPrinted()	const { return  IsSet(MSG_PRINTED); }
   Boolean		IsRead()	const { return  IsSet(MSG_READ); }
   Boolean		IsReplied()	const { return  IsSet(MSG_REPLIED); }
   Boolean		IsResent()	const { return  IsSet(MSG_RESENT); }
   Boolean		IsSaved()	const { return  IsSet(MSG_SAVED); }
   Boolean		IsUnread()	const { return !IsSet(MSG_READ); }
   Boolean		IsViewed()	const { return  IsSet(MSG_VIEWED); }
   Boolean		WasAnnounced()	const { return  IsSet(MSG_ANNOUNCED); }

   void			SetAnnounced()	{ SetStatus(MSG_ANNOUNCED); }
   void			SetChanged()	{ SetStatus(MSG_CHANGED); }
   void			SetCurrent()	{ SetStatus(MSG_VIEWED); }
   void			SetDeleted()	{ SetStatus(MSG_DELETED); }
   void			SetFiltered()	{ SetStatus(MSG_FILTERED); }
   void			SetForwarded()	{ SetStatus(MSG_FORWARDED); }
   void			SetMime()	{ SetStatus(MSG_MIME); }
   void			SetNew()	{ SetStatus(MSG_NEW); }
   void			SetPartial()	{ SetStatus(MSG_PARTIAL); }
   void			SetPrinted()	{ SetStatus(MSG_PRINTED); }
   void			SetRead()	{ SetStatus(MSG_READ); }
   void			SetReplied()	{ SetStatus(MSG_REPLIED); }
   void			SetResent()	{ SetStatus(MSG_RESENT); }
   void			SetSaved()	{ SetStatus(MSG_SAVED); }
   void			SetViewed()	{ SetCurrent(); }

   void			ClearChanged()	{ ClearStatus(MSG_CHANGED); }
   void			ClearCurrent()	{ ClearStatus(MSG_VIEWED); }
   void			ClearDeleted()	{ ClearStatus(MSG_DELETED); }
   void			ClearFiltered()	{ ClearStatus(MSG_FILTERED); }
   void			ClearForwarded(){ ClearStatus(MSG_FORWARDED); }
   void			ClearMime()	{ ClearStatus(MSG_MIME); }
   void			ClearNew()	{ ClearStatus(MSG_NEW); }
   void			ClearPartial()	{ ClearStatus(MSG_PARTIAL); }
   void			ClearPrinted()	{ ClearStatus(MSG_PRINTED); }
   void			ClearRead()	{ ClearStatus(MSG_READ); }
   void			ClearReplied()	{ ClearStatus(MSG_REPLIED); }
   void			ClearResent()	{ ClearStatus(MSG_RESENT); }
   void			ClearSaved()	{ ClearStatus(MSG_SAVED); }
   void			ClearViewed()	{ ClearCurrent(); }

   void			Print(ostream& strm) const;
   u_long		SpaceNeeded();
   virtual u_long	ExtraSpaceNeeded() { return 0; }

//
// Body info
//
   MsgPartC		*Body();
   MsgPartC		*QuickBody() { return body; }	// No scanning
   MsgPartC		*Part(CharC);

//
// Return text for specific body parts
//
   virtual Boolean	GetPartText(const MsgPartC*, StringC&, Boolean getHead=False,
   				    Boolean getExtHead=False,
				    Boolean getBody=True) const = 0;
   virtual Boolean	GetPartText(const MsgPartC*, StringC&, Boolean getHead=False,
   				    Boolean getExtHead=False,
				    Boolean getBody=True) = 0;
   virtual Boolean	GetFileData(MsgPartC*, StringC&) const = 0;
   virtual Boolean	GetFileData(MsgPartC*, StringC&)       = 0;

//
// Return text for entire body
//
   virtual Boolean	GetBodyText(StringC& text) const
	{ return GetPartText(body, text, False, True, True); }
   virtual Boolean	GetBodyText(StringC& text)
   	{ return GetPartText(body, text, False, True, True); }

//
// Header info
//
   virtual Boolean	 GetHeaderText(StringC&) const = 0;
   virtual Boolean	 GetHeaderText(StringC&)       = 0;

   HeaderC		*Headers() const;
   HeaderC		*Header(CharC) const;
   HeaderValC		*HeaderValue(CharC) const;
   void			 ReplaceHeaders(StringC&);

   virtual HeaderValC	*Date() const	{ return HeaderValue("Date"); }
   time_t		 Time()	const	{ return epochTime; }

   HeaderValC		*Subject() const { return HeaderValue("Subject"); }
   char			*Thread()  const { return thread; }
   void			 GetSubjectText(StringC&) const;

//
// Addresses.  Guaranteed to be available after construction.
//
   AddressC	*AddressOf(const char *) const;
   AddressC	*Cc()		const { return cc; }
   AddressC	*From()		const { return from; }
   AddressC	*ReplyTo()	const { return replyTo; }
   AddressC	*ReturnPath()	const { return returnPath; }
   AddressC	*Sender()	const { return sender; }
   AddressC	*To()		const { return to; }
   void		 GetFromName(StringC&) const;

//
// Sizes.  All but bodyLines are guaranteed to be available after construction.
// Getting bodyLines requires scanning the body.
//
   u_int	HeadOffset();
   int		HeadBytes();
   int		HeadLines();
   u_int	BodyOffset();
   int		BodyBytes();
   int		BodyLines();

//
// Holds the last positions at which the headers and body were written.  This
//   is used for relocating the message after a save.
//
   long		headWritePos;
   long		bodyWritePos;

//
// Message number and id
//
   int	Number()    const { return number; }
   void	SetNumber(int);
   StringC&	Id() { return msgId; }

//
// Attributions
//
   StringC		Attribution();
   StringC		BeginForward();
   StringC		EndForward();

//
// Write message to file
//
   Boolean		CopyBody(FILE*, FILE*, Boolean addBlank,
   					       Boolean protectFroms);
   Boolean		WriteBody(const char*, Boolean addBlank,
   					       Boolean protectFroms);
   virtual Boolean	WriteBody(FILE*,       Boolean addBlank,
					       Boolean protectFroms) = 0;
   Boolean		WriteFile(const char*, Boolean copyHead,
					       Boolean allHead,
					       Boolean statHead,
					       Boolean addBlank,
					       Boolean protectFroms);
   Boolean		WriteFile(FILE*,       Boolean copyHead,
					       Boolean allHead,
					       Boolean statHead,
					       Boolean addBlank,
					       Boolean protectFroms);
   Boolean		WriteHeaders(const char*, Boolean allHead=True,
   						  Boolean statHead=True);
   Boolean		WriteHeaders(FILE*, Boolean allHead=True,
   					    Boolean statHead=True);

//
// Use the last write positions as the new offsets
//
   void			UpdateOffsets();

   void			CreateIcon();
   StringC		*Match(const RuleDictC&);
};

//
// Method to allow printing of MsgC
//

inline ostream& operator<<(ostream& strm, MsgC& m)
{
   m.Print(strm);
   return(strm);
}

#endif // _MsgC_h_
