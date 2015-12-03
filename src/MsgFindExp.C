/*
 * $Id: MsgFindExp.C,v 1.2 2000/05/07 12:26:12 fnevgeny Exp $
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

#include <config.h>

#include "MsgFindExp.h"
#include "MsgItemC.h"
#include "MsgC.h"
#include "AddressC.h"
#include "HeaderValC.h"
#include "HeaderC.h"
#include "date.h"

#include <hgl/StringC.h>
#include <hgl/WXmString.h>
#include <hgl/rsrc.h>

#include <Xm/Label.h>

/*--------------------------------------------------------------------------
 * MsgFromExpC methods
 */

MsgFromExpC::MsgFromExpC(const char *pattern)
{
   pat = pattern;
}

Widget
MsgFromExpC::CreateWidget(Widget parent, ArgList argv, Cardinal argc)
{
   Widget w = XmCreateLabel(parent, "fromLabel", argv, argc);
   StringC	label = get_string(w, "labelString", "From contains ");
   if ( label[label.size()-1] != ' ' ) label += (char)' ';

   label += (char)'"';
   label += pat;
   label += (char)'"';
   WXmString	wstr = (char *)label;
   XtVaSetValues(w, XmNlabelString, (XmString)wstr, NULL);

   return w;
}

Boolean
MsgFromExpC::Match(void *fromItem)
{
   MsgItemC	*item = (MsgItemC *)fromItem;
   MsgC		*msg  = item->msg;
   AddressC	*from = msg->From();
   if ( !from ) return False;

   if ( pat.search(from->full) >= 0 ) return True;

//
// Try name in case it is RFC1522 encoded
//
   if ( from->name ) {
      StringC	val;
      from->name->GetValueText(val);
      if ( pat.search(val) >= 0 ) return True;
   }

   return False;
}

/*--------------------------------------------------------------------------
 * MsgToExpC methods
 */

MsgToExpC::MsgToExpC(const char *pattern)
{
   pat = pattern;
}

Widget
MsgToExpC::CreateWidget(Widget parent, ArgList argv, Cardinal argc)
{
   Widget w = XmCreateLabel(parent, "toLabel", argv, argc);
   StringC	label = get_string(w, "labelString", "To contains ");
   if ( label[label.size()-1] != ' ' ) label += (char)' ';

   label += (char)'"';
   label += pat;
   label += (char)'"';
   WXmString	wstr = (char *)label;
   XtVaSetValues(w, XmNlabelString, (XmString)wstr, NULL);

   return w;
}

Boolean
MsgToExpC::Match(void *toItem)
{
   MsgItemC	*item = (MsgItemC *)toItem;
   MsgC		*msg  = item->msg;
   AddressC	*to = msg->To();
   if ( !to ) return False;

   if ( pat.search(to->full) >= 0 ) return True;

//
// Try name in case it is RFC1522 encoded
//
   if ( to->name ) {
      StringC	val;
      to->name->GetValueText(val);
      if ( pat.search(val) >= 0 ) return True;
   }

   return False;
}

/*--------------------------------------------------------------------------
 * MsgSubjExpC methods
 */

MsgSubjExpC::MsgSubjExpC(const char *pattern)
{
   pat = pattern;
}

Widget
MsgSubjExpC::CreateWidget(Widget parent, ArgList argv, Cardinal argc)
{
   Widget w = XmCreateLabel(parent, "subjectLabel", argv, argc);
   StringC label = get_string(w, "labelString", "Subject contains ");
   if ( label[label.size()-1] != ' ' ) label += (char)' ';

   label += (char)'"';
   label += pat;
   label += (char)'"';
   WXmString	wstr = (char *)label;
   XtVaSetValues(w, XmNlabelString, (XmString)wstr, NULL);

   return w;
}

Boolean
MsgSubjExpC::Match(void *subItem)
{
   MsgItemC	*item = (MsgItemC *)subItem;
   MsgC		*msg  = item->msg;

   StringC	val;
   msg->GetSubjectText(val);
   return (pat.search(val) >= 0);
}

/*--------------------------------------------------------------------------
 * MsgHeadExpC methods
 */

MsgHeadExpC::MsgHeadExpC(const char *pattern)
{
   pat = pattern;
}

Widget
MsgHeadExpC::CreateWidget(Widget parent, ArgList argv, Cardinal argc)
{
   Widget w = XmCreateLabel(parent, "headLabel", argv, argc);
   StringC label = get_string(w, "labelString", "Header contains ");
   if ( label[label.size()-1] != ' ' ) label += (char)' ';

   label += (char)'"';
   label += pat;
   label += (char)'"';
   WXmString	wstr = (char *)label;
   XtVaSetValues(w, XmNlabelString, (XmString)wstr, NULL);

   return w;
}

Boolean
MsgHeadExpC::Match(void *headItem)
{
   MsgItemC	*item = (MsgItemC *)headItem;
   MsgC		*msg  = item->msg;
   HeaderC	*head = msg->Headers();
   StringC	val;
   while ( head ) {
      val.Clear();
      head->GetValueText(val);
      if ( pat.search(val) >= 0 ) return True;
      head = head->next;
   }

   return False;
}

/*--------------------------------------------------------------------------
 * MsgBodyExpC methods
 */

MsgBodyExpC::MsgBodyExpC(const char *pattern)
{
   pat = pattern;
}

Widget
MsgBodyExpC::CreateWidget(Widget parent, ArgList argv, Cardinal argc)
{
   Widget w = XmCreateLabel(parent, "bodyLabel", argv, argc);
   StringC label = get_string(w, "labelString", "Body contains ");
   if ( label[label.size()-1] != ' ' ) label += (char)' ';

   label += (char)'"';
   label += pat;
   label += (char)'"';
   WXmString	wstr = (char *)label;
   XtVaSetValues(w, XmNlabelString, (XmString)wstr, NULL);

   return w;
}

Boolean
MsgBodyExpC::Match(void *bodyItem)
{
   MsgItemC	*item = (MsgItemC *)bodyItem;
   MsgC		*msg  = item->msg;
   StringC	val;
   msg->GetBodyText(val);

   return (pat.search(val) >= 0);
}

/*--------------------------------------------------------------------------
 * MsgMsgExpC methods
 */

MsgMsgExpC::MsgMsgExpC(const char *pattern)
{
   pat = pattern;
}

Widget
MsgMsgExpC::CreateWidget(Widget parent, ArgList argv, Cardinal argc)
{
   Widget w = XmCreateLabel(parent, "msgLabel", argv, argc);
   StringC label = get_string(w, "labelString", "Message contains ");
   if ( label[label.size()-1] != ' ' ) label += (char)' ';

   label += (char)'"';
   label += pat;
   label += (char)'"';
   WXmString	wstr = (char *)label;
   XtVaSetValues(w, XmNlabelString, (XmString)wstr, NULL);

   return w;
}

Boolean
MsgMsgExpC::Match(void *msgItem)
{
   MsgItemC	*item = (MsgItemC *)msgItem;
   MsgC		*msg  = item->msg;
   StringC	val;

//
// Search headers
//
   HeaderC	*head = msg->Headers();
   while ( head ) {
      val.Clear();
      head->GetValueText(val);
      if ( pat.search(val) >= 0 ) return True;
      head = head->next;
   }

//
// Search body
//
   val.Clear();
   msg->GetBodyText(val);

   return (pat.search(val) >= 0);
}

/*--------------------------------------------------------------------------
 * MsgDateExpC methods
 */

MsgDateExpC::MsgDateExpC(MsgDateOp o, const char *str)
{
   op  = o;
   dateStr = str;
   val = parsedate(dateStr, NULL);
}

Widget
MsgDateExpC::CreateWidget(Widget parent, ArgList argv, Cardinal argc)
{
   Widget w = XmCreateLabel(parent, "dateLabel", argv, argc);
   StringC	label = get_string(w, "labelString", "Date is ");
   if ( label[label.size()-1] != ' ' ) label += (char)' ';

   switch (op) {
      case (MsgDateExpC::LT): label += "< ";  break;
      case (MsgDateExpC::LE): label += "<= "; break;
      case (MsgDateExpC::EQ): label += "= ";  break;
      case (MsgDateExpC::GE): label += ">= "; break;
      case (MsgDateExpC::GT): label += "> ";  break;
      case (MsgDateExpC::NE): label += "!= "; break;
   }
   label += dateStr;
   WXmString	wstr = (char *)label;
   XtVaSetValues(w, XmNlabelString, (XmString)wstr, NULL);

   return w;
}

Boolean
MsgDateExpC::Match(void *dateItem)
{
   MsgItemC	*item = (MsgItemC *)dateItem;
   MsgC		*msg = item->msg;
   switch (op) {
      case (MsgDateExpC::LT): return (msg->Time() <  val);
      case (MsgDateExpC::LE): return (msg->Time() <= val);
      case (MsgDateExpC::EQ): return (msg->Time() == val);
      case (MsgDateExpC::GE): return (msg->Time() >= val);
      case (MsgDateExpC::GT): return (msg->Time() >  val);
      case (MsgDateExpC::NE): return (msg->Time() != val);
   }

   return False;
}

/*--------------------------------------------------------------------------
 * MsgStatExpC methods
 */

MsgStatExpC::MsgStatExpC(MsgStatusT s)
{
   stat = s;
}

Widget
MsgStatExpC::CreateWidget(Widget parent, ArgList argv, Cardinal argc)
{
   Widget w = XmCreateLabel(parent, "statusLabel", argv, argc);
   StringC	label = get_string(w, "labelString", "Status is ");
   if ( label[label.size()-1] != ' ' ) label += (char)' ';

   switch (stat) {
      case (MSG_NEW):		label += "New";		break;
      case (MSG_READ):		label += "Read";	break;
      case (MSG_DELETED):	label += "Deleted";	break;
      case (MSG_SAVED):		label += "Saved";	break;
      case (MSG_REPLIED):	label += "Replied to";	break;
      case (MSG_FORWARDED):	label += "Forwarded";	break;
      case (MSG_RESENT):	label += "Resent";	break;
      case (MSG_PRINTED):	label += "Printed";	break;
      case (MSG_FILTERED):	label += "Filtered";	break;
      case (MSG_MIME):		label += "MIME format";	break;
   }
   WXmString	wstr = (char *)label;
   XtVaSetValues(w, XmNlabelString, (XmString)wstr, NULL);

   return w;
}

Boolean
MsgStatExpC::Match(void *statItem)
{
   MsgItemC	*item = (MsgItemC *)statItem;
   Boolean	matched = False;

   switch (stat) {
      case (MSG_NEW):		matched = item->IsNew();	break;
      case (MSG_READ):		matched = item->IsRead();	break;
      case (MSG_DELETED):	matched = item->IsDeleted();	break;
      case (MSG_SAVED):		matched = item->IsSaved();	break;
      case (MSG_REPLIED):	matched = item->IsReplied();	break;
      case (MSG_FORWARDED):	matched = item->IsForwarded();	break;
      case (MSG_RESENT):	matched = item->IsResent();	break;
      case (MSG_PRINTED):	matched = item->IsPrinted();	break;
      case (MSG_FILTERED):	matched = item->IsFiltered();	break;
      case (MSG_MIME):		matched = item->IsMime();	break;
   }

   return matched;
}

/*--------------------------------------------------------------------------
 * MsgNumExpC methods
 */

MsgNumExpC::MsgNumExpC(MsgNumOp o, unsigned v)
{
   op  = o;
   val = v;
}

Widget
MsgNumExpC::CreateWidget(Widget parent, ArgList argv, Cardinal argc)
{
   Widget w = XmCreateLabel(parent, "numLabel", argv, argc);
   StringC	label = get_string(w, "labelString", "Number is ");
   if ( label[label.size()-1] != ' ' ) label += (char)' ';

   switch (op) {
      case (MsgNumExpC::LT): label += "< ";  break;
      case (MsgNumExpC::LE): label += "<= "; break;
      case (MsgNumExpC::EQ): label += "= ";  break;
      case (MsgNumExpC::GE): label += ">= "; break;
      case (MsgNumExpC::GT): label += "> ";  break;
      case (MsgNumExpC::NE): label += "!= "; break;
   }
   label += (int)val;
   WXmString	wstr = (char *)label;
   XtVaSetValues(w, XmNlabelString, (XmString)wstr, NULL);

   return w;
}

Boolean
MsgNumExpC::Match(void *numItem)
{
   MsgItemC	*item = (MsgItemC *)numItem;
   MsgC		*msg  = item->msg;
   switch (op) {
      case (MsgNumExpC::LT): return (msg->Number() <  val);
      case (MsgNumExpC::LE): return (msg->Number() <= val);
      case (MsgNumExpC::EQ): return (msg->Number() == val);
      case (MsgNumExpC::GE): return (msg->Number() >= val);
      case (MsgNumExpC::GT): return (msg->Number() >  val);
      case (MsgNumExpC::NE): return (msg->Number() != val);
   }

   return False;
}

/*--------------------------------------------------------------------------
 * MsgLineExpC methods
 */

MsgLineExpC::MsgLineExpC(MsgLineOp o, unsigned v)
{
   op  = o;
   val = v;
}

Widget
MsgLineExpC::CreateWidget(Widget parent, ArgList argv, Cardinal argc)
{
   Widget w = XmCreateLabel(parent, "lineLabel", argv, argc);
   StringC	label = get_string(w, "labelString", "Line count is ");
   if ( label[label.size()-1] != ' ' ) label += (char)' ';

   switch (op) {
      case (MsgLineExpC::LT): label += "< ";  break;
      case (MsgLineExpC::LE): label += "<= "; break;
      case (MsgLineExpC::EQ): label += "= ";  break;
      case (MsgLineExpC::GE): label += ">= "; break;
      case (MsgLineExpC::GT): label += "> ";  break;
      case (MsgLineExpC::NE): label += "!= "; break;
   }
   label += (int)val;
   WXmString	wstr = (char *)label;
   XtVaSetValues(w, XmNlabelString, (XmString)wstr, NULL);

   return w;
}

Boolean
MsgLineExpC::Match(void *lineItem)
{
   MsgItemC	*item = (MsgItemC *)lineItem;
   MsgC		*msg = item->msg;
   switch (op) {
      case (MsgLineExpC::LT): return (msg->BodyLines() <  val);
      case (MsgLineExpC::LE): return (msg->BodyLines() <= val);
      case (MsgLineExpC::EQ): return (msg->BodyLines() == val);
      case (MsgLineExpC::GE): return (msg->BodyLines() >= val);
      case (MsgLineExpC::GT): return (msg->BodyLines() >  val);
      case (MsgLineExpC::NE): return (msg->BodyLines() != val);
   }

   return False;
}

/*--------------------------------------------------------------------------
 * MsgByteExpC methods
 */

MsgByteExpC::MsgByteExpC(MsgByteOp o, unsigned v)
{
   op  = o;
   val = v;
}

Widget
MsgByteExpC::CreateWidget(Widget parent, ArgList argv, Cardinal argc)
{
   Widget w = XmCreateLabel(parent, "byteLabel", argv, argc);
   StringC	label = get_string(w, "labelString", "Byte count is ");
   if ( label[label.size()-1] != ' ' ) label += (char)' ';

   switch (op) {
      case (MsgByteExpC::LT): label += "< ";  break;
      case (MsgByteExpC::LE): label += "<= "; break;
      case (MsgByteExpC::EQ): label += "= ";  break;
      case (MsgByteExpC::GE): label += ">= "; break;
      case (MsgByteExpC::GT): label += "> ";  break;
      case (MsgByteExpC::NE): label += "!= "; break;
   }
   label += (int)val;
   WXmString	wstr = (char *)label;
   XtVaSetValues(w, XmNlabelString, (XmString)wstr, NULL);

   return w;
}

Boolean
MsgByteExpC::Match(void *byteItem)
{
   MsgItemC	*item = (MsgItemC *)byteItem;
   MsgC		*msg = item->msg;
   switch (op) {
      case (MsgByteExpC::LT): return (msg->BodyBytes() <  val);
      case (MsgByteExpC::LE): return (msg->BodyBytes() <= val);
      case (MsgByteExpC::EQ): return (msg->BodyBytes() == val);
      case (MsgByteExpC::GE): return (msg->BodyBytes() >= val);
      case (MsgByteExpC::GT): return (msg->BodyBytes() >  val);
      case (MsgByteExpC::NE): return (msg->BodyBytes() != val);
   }

   return False;
}
