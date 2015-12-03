/*
 * $Id: SendWinC.h,v 1.2 2001/07/28 18:26:03 evgeny Exp $
 *
 * Copyright (c) 1993 HAL Computer Systems International, Ltd.
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

#ifndef _SendWinC_h_
#define _SendWinC_h_

#include "MailPrefC.h"
#include "MimeTypes.h"

#include <hgl/HalTopLevelC.h>
#include <hgl/CallbackC.h>

class SendWinP;
class MsgC;
class MsgListC;
class MsgPartC;
class CharC;
class ButtonMgrC;
class VItemListC;
class StringListC;
class MimeRichTextC;

/*---------------------------------------------------------------
 *  Possible uses for this window
 */

enum SendWinTypeT {
   SEND_NEW,
   SEND_REPLY,
   SEND_FORWARD,
   SEND_RESEND,
   SEND_EDIT		// Edit-only, no delivery
};

//=======================================================================

class SendWinC : public HalTopLevelC {

   friend class SendWinP;

   SendWinP	*priv;		// Private stuff

public:

//
// Public data
//
   SendWinTypeT		type;
   Boolean		ccVis;
   Boolean		bccVis;
   Boolean		fccVis;
   Boolean		otherVis;
   int			maxFieldsPerLine;
   Boolean		changed;

// Methods

   SendWinC(const char*, Widget, SendWinTypeT winType=SEND_NEW);
   ~SendWinC();

   void			AddSig(Boolean);
   Boolean		AddingSig();
   Boolean		CheckingAddresses();
   int			BodyRowCount();
   MimeRichTextC	*BodyText();
   ButtonMgrC		*ButtonMgr();
   Boolean		Changed();
   Boolean		Close(Boolean save=False);
   int			ColumnCount();
   StringC&		DescTemplate();
   void			EditMsg(MsgC*, Boolean, CallbackFn*, void*);
   void			Forward(MsgListC&, Boolean encapsulate=False);
   int			HeadRowCount();
   Boolean		IsEditOnly() { return type == SEND_EDIT; }
   Boolean		IsEmpty();
   Boolean		IsForward()  { return type == SEND_FORWARD; }
   Boolean		IsNew()	     { return type == SEND_NEW; }
   Boolean		IsReply()    { return type == SEND_REPLY; }
   Boolean		IsResend()   { return type == SEND_RESEND; }
   Boolean		LoadFile(const char*);
   OutgoingMailTypeT	MailType();
   void			MessageDeleted(MsgC*);
   void			PlaceHeaderFields();
   void			Reply(MsgC*, Boolean, Boolean);
   void			Resend(MsgListC&);
   void			SetAddressChecking(Boolean);
   void			SetAutoSaveFile(const char*);
   void			SetBcc(const char*);
   void			SetBody(CharC);
   void			SetCc(const char*);
   void			SetCc(StringListC&);
   void			SetChanged(Boolean);
   void			SetKeys(Boolean, Boolean);
   void			SetMailType(OutgoingMailTypeT);
   void			SetOther(const char*);
   void			SetReplyMsg(MsgC*);
   void			SetSize(int, int);
   void			SetSubject(const char*);
   void			SetTextType(MimeContentType);
   void			SetTo(const char*);
   void			SetWrap(Boolean);
   void			Show();
   MimeContentType	TextType();
   Widget		ToTF() const;
   void			UpdateFcc();
   void			UpdateVisibleFields();
   Boolean		Wrapping();
};

#endif // _SendWinC_h_
