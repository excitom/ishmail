/*
 *  $Id: SendOpt.C,v 1.2 2000/05/07 12:26:13 fnevgeny Exp $
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
#include "SendWinC.h"
#include "SendWinP.h"
#include "IshAppC.h"
#include "CompPrefC.h"
#include "MailPrefC.h"
#include "ReplyPrefC.h"
#include "SigPrefC.h"
#include "SendButtPrefC.h"

#include <hgl/MimeRichTextC.h>

#include <Xm/ToggleB.h>

/*---------------------------------------------------------------
 *  Callbacks to handle preferences
 */

void
SendWinP::DoWinPrefs(Widget, SendWinP *This, XtPointer)
{
   ishApp->compPrefs->Edit(*This->pub);
}

void
SendWinP::DoMailPrefs(Widget, SendWinP *This, XtPointer)
{
   ishApp->mailPrefs->Edit(*This->pub);
}

void
SendWinP::DoReplyPrefs(Widget, SendWinP *This, XtPointer)
{
   ishApp->replyPrefs->Edit(*This->pub);
}

void
SendWinP::DoSigPrefs(Widget, SendWinP *This, XtPointer)
{
   ishApp->sigPrefs->Edit(*This->pub);
}

/*---------------------------------------------------------------
 *  Callback to handle custom buttons
 */

void
SendWinP::DoButtons(Widget, SendWinP *This, XtPointer)
{
   ishApp->sendButtPrefs->Edit(*This->pub, This->buttMgr);
}

/*---------------------------------------------------------------
 *  Callback to handle toggle of Cc field
 */

void
SendWinP::ToggleCc(Widget, SendWinP *This, XmToggleButtonCallbackStruct *tb)
{
   This->pub->ccVis = tb->set;
   This->PlaceHeaderFields();
}

/*---------------------------------------------------------------
 *  Callback to handle toggle of Bcc field
 */

void
SendWinP::ToggleBcc(Widget, SendWinP *This, XmToggleButtonCallbackStruct *tb)
{
   This->pub->bccVis = tb->set;
   This->PlaceHeaderFields();
}

/*---------------------------------------------------------------
 *  Callback to handle toggle of Fcc field
 */

void
SendWinP::ToggleFcc(Widget, SendWinP *This, XmToggleButtonCallbackStruct *tb)
{
   This->pub->fccVis = tb->set;
   This->PlaceHeaderFields();
}

/*---------------------------------------------------------------
 *  Callback to handle toggle of Other field
 */

void
SendWinP::ToggleOther(Widget, SendWinP *This, XmToggleButtonCallbackStruct *tb)
{
   This->pub->otherVis = tb->set;
   This->PlaceHeaderFields();
}

/*---------------------------------------------------------------
 *  Callback to handle toggle of wrap state
 */

void
SendWinP::ToggleWrap(Widget, SendWinP *This, XmToggleButtonCallbackStruct *tb)
{
   This->pub->SetWrap(tb->set);
}

/*---------------------------------------------------------------
 *  Callback to handle toggle of message type
 */

void
SendWinP::ToggleMime(Widget, SendWinP *This, XmToggleButtonCallbackStruct *tb)
{
   This->bodyText->ForceFixed(!tb->set ||
			      XmToggleButtonGetState(This->optTextPlainTB));
}

/*---------------------------------------------------------------
 *  Callback to handle toggle of message type
 */

void
SendWinP::ToggleMsgPlain(Widget, SendWinP *This,
			 XmToggleButtonCallbackStruct *tb)
{
   This->bodyText->ForceFixed(tb->set ||
			      XmToggleButtonGetState(This->optTextPlainTB));
   XmToggleButtonSetState(This->optMimeTB, !tb->set, False);
}

/*---------------------------------------------------------------
 *  Callback to handle toggle of text type
 */

void
SendWinP::ToggleTextPlain(Widget, SendWinP *This,
			  XmToggleButtonCallbackStruct *tb)
{
   This->bodyText->ForceFixed(tb->set ||
			      XmToggleButtonGetState(This->optMsgPlainTB));
}

