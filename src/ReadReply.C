/*
 *  $Id: ReadReply.C,v 1.2 2000/05/07 12:26:12 fnevgeny Exp $
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
#include "ReadWinC.h"
#include "ReadWinP.h"
#include "SendWinC.h"
#include "IshAppC.h"
#include "MsgListC.h"

/*---------------------------------------------------------------
 *  Callbacks to reply to message
 */

void
ReadWinP::DoReply(Widget, ReadWinP *This, XtPointer)
{
   SendWinC	*sendWin = ishApp->GetSendWin();
   sendWin->Reply(This->pub->msg, /*all*/False, /*inc*/False);
}

void
ReadWinP::DoReplyInc(Widget, ReadWinP *This, XtPointer)
{
   SendWinC	*sendWin = ishApp->GetSendWin();
   sendWin->Reply(This->pub->msg, /*all*/False, /*inc*/True);
}

void
ReadWinP::DoReplyAll(Widget, ReadWinP *This, XtPointer)
{
   SendWinC	*sendWin = ishApp->GetSendWin();
   sendWin->Reply(This->pub->msg, /*all*/True, /*inc*/False);
}

void
ReadWinP::DoReplyAllInc(Widget, ReadWinP *This, XtPointer)
{
   SendWinC	*sendWin = ishApp->GetSendWin();
   sendWin->Reply(This->pub->msg, /*all*/True, /*inc*/True);
}

/*---------------------------------------------------------------
 *  Callback to forward message
 */

void
ReadWinP::DoForward(Widget, ReadWinP *This, XtPointer)
{
   SendWinC	*sendWin = ishApp->GetSendWin();

   MsgListC	msgList;
   msgList.add(This->pub->msg);

   sendWin->Forward(msgList, False/*Don't encapsulate*/);
}

void
ReadWinP::DoForward822(Widget, ReadWinP *This, XtPointer)
{
   SendWinC	*sendWin = ishApp->GetSendWin();

   MsgListC	msgList;
   msgList.add(This->pub->msg);

   sendWin->Forward(msgList, True/*Encapsulate*/);
}

/*---------------------------------------------------------------
 *  Callback to resend message
 */

void
ReadWinP::DoResend(Widget, ReadWinP *This, XtPointer)
{
   SendWinC	*sendWin = ishApp->GetSendWin();

   MsgListC	msgList;
   msgList.add(This->pub->msg);

   sendWin->Resend(msgList);
}

