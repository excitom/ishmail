/*
 * $Id: MsgItemC.C,v 1.2 2000/05/07 12:26:12 fnevgeny Exp $
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

#include "MsgItemC.h"
#include "MsgC.h"
#include "IshAppC.h"
#include "SumPrefC.h"
#include "SumFieldC.h"
#include "HeaderValC.h"
#include "AddressC.h"
#include "AddrMisc.h"
#include "MsgStatus.h"
#include "SortMgrC.h"
#include "MailSortKeyC.h"

#include <hgl/StringListC.h>

#include <time.h>

#include "FolderC.h"
#include "Misc.h"

#include "new_mail.xpm"
#include "read_mail.xpm"
#include "saved_mail.xpm"
#include "unread_mail.xpm"
#include "current_mail.xpm"
#include "deleted_mail.xpm"
#include "new_mime.xpm"
#include "read_mime.xpm"
#include "saved_mime.xpm"
#include "unread_mime.xpm"
#include "partial_mime.xpm"
#include "current_mime.xpm"
#include "deleted_mime.xpm"

#define COLUMN_OF(S)	 ishApp->sumPrefs->sumColumn[SumFieldC::S]
#define FIELD_OF(S)	&ishApp->sumPrefs->sumFieldList[COLUMN_OF(S)]
//#define FIELD_SHOWN(S)	 ishApp->sumPrefs->sumFieldList[SumFieldC::S].show
#define SET_FIELD(S,V)	Field(COLUMN_OF(S), V);

/*------------------------------------------------------------------------
 * MsgItemC constructor
 */

MsgItemC::MsgItemC(MsgC *m)
{
   msg = m;

   prevInThread = NULL;
   nextInThread = NULL;

   LoadFields();
   UpdateStatus();
}

/*------------------------------------------------------------------------
 * MsgItemC destructor
 */

MsgItemC::~MsgItemC()
{
   if ( prevInThread ) prevInThread->nextInThread = nextInThread;
   if ( nextInThread ) nextInThread->prevInThread = prevInThread;
}

/*------------------------------------------------------------------------
 * Method to load the view item fields based on the current info
 */

void
MsgItemC::LoadFields()
{
   StringListC	fieldList;
   fieldList.AllowDuplicates(TRUE);

//
// Start off with the desired number of blank fields
//
   StringC	blank("");
   for (int i=0; i<SumFieldC::SUMMARY_FIELD_COUNT; i++)
      fieldList.add(blank);

//
// Load the field strings with their correct values
//
   StringC	tmp;
   SumFieldC	*field = FIELD_OF(MSG_NUM);
   if ( field->show ) {
      tmp += msg->Number();
      *fieldList[COLUMN_OF(MSG_NUM)] = tmp;
   }

   field = FIELD_OF(STATUS);
   if ( field->show ) {
      tmp.Clear();
      GetStatusString(tmp);
      *fieldList[COLUMN_OF(STATUS)]  = tmp;
   }

   field = FIELD_OF(SUBJECT);
   if ( field->show ) {
      tmp.Clear();
      msg->GetSubjectText(tmp);
      *fieldList[COLUMN_OF(SUBJECT)] = tmp;
   }

   field = FIELD_OF(DATE);
   if ( field->show ) {

      Boolean	needDate = True;
      if ( ishApp->sumPrefs->dateFormat.size() > 0 ) {

	 time_t	epochTime = msg->Time();
	 struct tm	*tm = localtime(&epochTime);
	 char	buf[128];
	 int	size = strftime(buf, 128, ishApp->sumPrefs->dateFormat, tm);
	 if ( size > 0 ) {
	    buf[size] = 0;
	    *fieldList[COLUMN_OF(DATE)] = buf;
	    needDate = False;
	 }
      }

      if ( needDate ) {

	 tmp.Clear();
	 HeaderValC   *date = msg->Date();
	 if ( date ) date->GetValueText(tmp);
	 else	  tmp = "[No date]";

	 *fieldList[COLUMN_OF(DATE)] = tmp;
      }
   }

   field = FIELD_OF(FROM);
   if ( field->show ) {

      AddressC	*from = msg->From();
      AddressC	*to   = msg->To();
      if ( from ) {

	 if ( IsMyAddress(from->addr) && to ) {
	    if ( to->name ) {
	       tmp.Clear();
	       to->name->GetValueText(tmp);
	       *fieldList[COLUMN_OF(FROM)] = "TO " + tmp;
	    }
	    else {
	       *fieldList[COLUMN_OF(FROM)] = "TO " + to->addr;
	    }
	 }

	 else if ( from->name ) {
	    tmp.Clear();
	    from->name->GetValueText(tmp);
	    *fieldList[COLUMN_OF(FROM)] = tmp;
	 }

	 else {
	    *fieldList[COLUMN_OF(FROM)] = from->addr;
	 }

      } // End if From: header is present

      else {
	 *fieldList[COLUMN_OF(FROM)] = "Unknown";
      }
   }

   field = FIELD_OF(LINES);
   if ( field->show ) {
      tmp.Clear();
      tmp += msg->BodyLines();
      *fieldList[COLUMN_OF(LINES)] = tmp;
   }

   field = FIELD_OF(BYTES);
   if ( field->show ) {
      tmp.Clear();
      tmp += msg->BodyBytes();
      *fieldList[COLUMN_OF(BYTES)] = tmp;
   }

   FieldList(fieldList);

} // End LoadFields

/*------------------------------------------------------------------------
 * Methods to return status flags
 */

Boolean MsgItemC::IsCurrent()   const { return  msg->IsSet(MSG_VIEWED); }
Boolean MsgItemC::IsDeleted()   const { return  msg->IsSet(MSG_DELETED); }
Boolean MsgItemC::IsFiltered()  const { return  msg->IsSet(MSG_FILTERED); }
Boolean MsgItemC::IsForwarded() const { return  msg->IsSet(MSG_FORWARDED); }
Boolean MsgItemC::IsMime()      const { return  msg->IsSet(MSG_MIME); }
Boolean MsgItemC::IsNew()       const { return  msg->IsSet(MSG_NEW); }
Boolean MsgItemC::IsPartial()   const { return  msg->IsSet(MSG_PARTIAL); }
Boolean MsgItemC::IsPrinted()   const { return  msg->IsSet(MSG_PRINTED); }
Boolean MsgItemC::IsRead()      const { return  msg->IsSet(MSG_READ); }
Boolean MsgItemC::IsReplied()   const { return  msg->IsSet(MSG_REPLIED); }
Boolean MsgItemC::IsResent()    const { return  msg->IsSet(MSG_RESENT); }
Boolean MsgItemC::IsSaved()     const { return  msg->IsSet(MSG_SAVED); }
Boolean MsgItemC::IsUnread()    const { return !msg->IsSet(MSG_READ); }
Boolean MsgItemC::IsViewed()    const { return  msg->IsSet(MSG_VIEWED); }

/*------------------------------------------------------------------------
 * Methods to set status flags
 */

#define SET_FLAG(F) \
{ \
   if ( msg->IsSet(F) ) return; \
   msg->SetStatus(F); \
   UpdateStatus(); \
}

void MsgItemC::SetDeleted()   SET_FLAG(MSG_DELETED)
void MsgItemC::SetFiltered()  SET_FLAG(MSG_FILTERED)
void MsgItemC::SetForwarded() SET_FLAG(MSG_FORWARDED)
void MsgItemC::SetNew()       SET_FLAG(MSG_NEW)
void MsgItemC::SetPrinted()   SET_FLAG(MSG_PRINTED)
void MsgItemC::SetRead()      SET_FLAG(MSG_READ)
void MsgItemC::SetReplied()   SET_FLAG(MSG_REPLIED)
void MsgItemC::SetResent()    SET_FLAG(MSG_RESENT)
void MsgItemC::SetSaved()     SET_FLAG(MSG_SAVED)
void MsgItemC::SetCurrent()   SET_FLAG(MSG_VIEWED)

/*------------------------------------------------------------------------
 * Methods to clear status flags
 */

#define CLEAR_FLAG(F) \
{ \
   if ( !msg->IsSet(F) ) return; \
   msg->ClearStatus(F); \
   UpdateStatus(); \
}

void MsgItemC::ClearDeleted()   CLEAR_FLAG(MSG_DELETED)
void MsgItemC::ClearFiltered()  CLEAR_FLAG(MSG_FILTERED)
void MsgItemC::ClearForwarded() CLEAR_FLAG(MSG_FORWARDED)
void MsgItemC::ClearNew()       CLEAR_FLAG(MSG_NEW)
void MsgItemC::ClearPrinted()   CLEAR_FLAG(MSG_PRINTED)
void MsgItemC::ClearRead()      CLEAR_FLAG(MSG_READ)
void MsgItemC::ClearReplied()   CLEAR_FLAG(MSG_REPLIED)
void MsgItemC::ClearResent()    CLEAR_FLAG(MSG_RESENT)
void MsgItemC::ClearSaved()     CLEAR_FLAG(MSG_SAVED)
void MsgItemC::ClearCurrent()   CLEAR_FLAG(MSG_VIEWED)

/*------------------------------------------------------------------------
 * Method to update folder and summary entry
 */

void
MsgItemC::UpdateStatus()
{
   if ( ishApp->exiting ) return;

//
// Set status field and pixmap
//
   StringC	tmp;
   GetStatusString(tmp);
   SET_FIELD(STATUS, tmp);

   XpmT	xpm = StatusPixmap();
   SetPixmaps(NULL, xpm);

   tmp.Clear();
   tmp += msg->Number();
   SET_FIELD(MSG_NUM, tmp);

} // End UpdateStatus

/*------------------------------------------------------------------------
 * Method to build message status string
 */

void
MsgItemC::GetStatusString(StringC& statStr)
{
   statStr.Clear();

   if ( IsCurrent()    ) statStr += "*";

   if ( IsPartial()    ) statStr += "m";
   else if ( IsMime()  ) statStr += "M";

   if ( IsNew()        ) statStr += "N";
   else if ( !IsRead() ) statStr += "U";

   if ( IsDeleted()    ) statStr += "D";
   if ( IsSaved()      ) statStr += "S";
   if ( IsReplied()    ) statStr += ">";
   if ( IsForwarded()  ) statStr += "F";
   if ( IsResent()     ) statStr += "B";
   if ( IsPrinted()    ) statStr += "P";
   if ( IsFiltered()   ) statStr += "|";

} // End GetStatusString

/*------------------------------------------------------------------------
 * Method to choose message status pixmap
 */

XpmT
MsgItemC::StatusPixmap()
{
   XpmT	xpm;
   if ( IsCurrent() )      xpm = IsMime() ? current_mime_xpm : current_mail_xpm;
   else if ( IsDeleted() ) xpm = IsMime() ? deleted_mime_xpm : deleted_mail_xpm;
   else if ( IsSaved() )   xpm = IsMime() ? saved_mime_xpm   : saved_mail_xpm;
   else if ( IsPartial() ) xpm = partial_mime_xpm;
   else if ( IsNew() )     xpm = IsMime() ? new_mime_xpm     : new_mail_xpm;
   else if ( !IsRead() )   xpm = IsMime() ? unread_mime_xpm  : unread_mail_xpm;
   else			   xpm = IsMime() ? read_mime_xpm    : read_mail_xpm;

   return xpm;

} // End StatusPixmap

/*------------------------------------------------------------------------
 * Method to compare two message items
 */

int
MsgItemC::MsgItemCompare(const void *a, const void *b)
{
   MsgItemC	*mia = *(MsgItemC **)a;
   MsgItemC	*mib = *(MsgItemC **)b;
   int		result = 0;

   SortMgrC	*sortMgr = mia->msg->folder->SortMgr();

//
// Loop through the sort keys until we get a non-zero result
//
   SortKeyListC&	keyList  = sortMgr->KeyList();
   unsigned		keyCount = keyList.size();
   for (int i=0; (i<keyCount && result==0); i++) {

      MailSortKeyC	*key = (MailSortKeyC *)keyList[i];
      if ( sortMgr->Threaded() )
	 result = key->CompareThreads(*mia, *mib, sortMgr->ThreadDir());
      else
	 result = key->CompareMessages(*mia, *mib);
   }

//
// If these message are equal, fall back to the number
//
   if ( result == 0 ) {
      if      ( mia->msg->Number() < mib->msg->Number() ) result = -1;
      else if ( mia->msg->Number() > mib->msg->Number() ) result =  1;
   }

   return result;

} // End MsgItemCompare

