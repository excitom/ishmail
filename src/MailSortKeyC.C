/*
 * $Id: MailSortKeyC.C,v 1.2 2000/05/07 12:26:12 fnevgeny Exp $
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

#include "MailSortKeyC.h"
#include "MsgItemC.h"
#include "MsgC.h"
#include "Misc.h"
#include "FolderC.h"
#include "AddressC.h"
#include "HeaderValC.h"

#include <hgl/StrCase.h>
#include <hgl/StringC.h>
#include <hgl/RegexC.h>

extern int	debuglev;

/*-----------------------------------------------------------------------
 * Constructors
 */

MailSortKeyC::MailSortKeyC(MailSortKeyC::MailSortKeyT t, SortKeyC::SortDirT d)
: SortKeyC(d)
{
   type = t;
}

MailSortKeyC::MailSortKeyC(const MailSortKeyC& k) : SortKeyC(k)
{
   type = k.type;
}

/*-----------------------------------------------------------------------
 * Message compare method
 */

int
MailSortKeyC::CompareMessages(MsgItemC& mi1, MsgItemC& mi2) const
{
   static StringC	*s1 = NULL;
   static StringC	*s2 = NULL;
   if ( !s1 ) {
      s1 = new StringC();
      s2 = new StringC();
   }

   MsgC	*msg1 = (MsgC*)mi1.msg;
   MsgC	*msg2 = (MsgC*)mi2.msg;
   int	val   = 0;

   if ( debuglev > 3 ) cout <<"Using ";

   switch (type) {

      case (NUMBER): {

	 int	n1 = mi1.msg->Number();
	 int	n2 = mi2.msg->Number();

	 val = n1 - n2;
	 if ( dir == DESCENDING ) val = -val;
	 if ( debuglev > 3 ) cout <<"number"; 

      } break;

      case (STATUS): {

	 int	less = (dir == ASCENDING) ? -1 :  1;

	 if ( mi1.IsNew() ) {
	    if ( mi2.IsNew() ) 			 val =  0;
	    else				 val =  less;
	 }
	 else if ( mi1.IsUnread() ) {
	    if ( mi2.IsNew() ) 	 		 val = -less;	// greater
	    else if ( mi2.IsUnread() )		 val =  0;
	    else				 val =  less;
	 }
	 else {
	    if ( mi2.IsNew() || mi2.IsUnread() ) val = -less;	// greater
	    else				 val =  0;
	 }

	 if ( debuglev > 3 ) cout <<"status"; 

      } break;

      case (SENDER): {

	 AddressC	*a1 = msg1->From();
	 AddressC	*a2 = msg2->From();

	 if ( !a1 && !a2 ) val =  0;
	 else if ( !a1 )   val = -1;
	 else if ( !a2 )   val =  1;

	 else {

	     s1->Clear();
	     s2->Clear();

	     if ( a1->name ) a1->name->GetValueText(*s1);
	     else	    *s1 = a1->addr;
	     if ( a2->name ) a2->name->GetValueText(*s2);
	     else	    *s2 = a2->addr;

	     val = (dir == ASCENDING ? s1->compare(*s2) : s2->compare(*s1));
	 }

	 if ( debuglev > 3 ) cout <<"sender"; 

      } break;

      case (TO): {

	 AddressC	*a1 = msg1->To();
	 AddressC	*a2 = msg2->To();

	 if ( !a1 && !a2 ) val =  0;
	 else if ( !a1 )   val = -1;
	 else if ( !a2 )   val =  1;

	 else {

	     s1->Clear();
	     s2->Clear();

	     if ( a1->name ) a1->name->GetValueText(*s1);
	     else	    *s1 = a1->addr;
	     if ( a2->name ) a2->name->GetValueText(*s2);
	     else	    *s2 = a2->addr;

	     val = (dir == ASCENDING ? s1->compare(*s2) : s2->compare(*s1));
	 }
	 if ( debuglev > 3 ) cout <<"TO"; 

      } break;

      case (SUBJECT): {

//
// The thread is the subject without the Re's
//
	 *s1 = msg1->Thread();
	 *s2 = msg2->Thread();

	 val = (dir == ASCENDING ? s1->compare(*s2) : s2->compare(*s1));
	 if ( debuglev > 3 ) cout <<"subject"; 

      } break;

      case (DATE):
	 val = (int)(dir == ASCENDING ? msg1->Time() - msg2->Time()
				      : msg2->Time() - msg1->Time());
	 if ( debuglev > 3 ) cout <<"date"; 
	 break;

      case (LINES):
	 val = (dir == ASCENDING ? msg1->BodyLines() - msg2->BodyLines()
				 : msg2->BodyLines() - msg1->BodyLines());
	 if ( debuglev > 3 ) cout <<"lines"; 
	 break;

      case (BYTES):
	 val = (dir == ASCENDING ? msg1->BodyBytes() - msg2->BodyBytes()
				 : msg2->BodyBytes() - msg1->BodyBytes());
	 if ( debuglev > 3 ) cout <<"bytes"; 
	 break;

      case (THREAD):
	 if ( debuglev > 3 ) cout <<"thread"; 
         break;

   } // End switch sort key type

   if ( debuglev > 3 ) {
      cout <<", msg " <<dec(msg1->Number(),4); 
      if      ( val < 0 ) cout <<" < ";
      else if ( val > 0 ) cout <<" > ";
      else		  cout <<" = ";
      cout <<"msg " <<dec(msg2->Number(),4) <<endl;
   }

   return val;

} // End CompareMessages

/*------------------------------------------------------------------------
 * Function to find the first message in the thread that contains the
 *    specified message.
 */

static MsgItemC*
StartOfThread(MsgItemC& mi)
{
   MsgItemC	*item = &mi;
   while ( item->prevInThread ) item = item->prevInThread;
   return item;
}

/*------------------------------------------------------------------------
 * Function to find the oldest message in the thread that contains the specified
 *    message.
 */

static MsgC*
OldestMessageInThread(MsgItemC& mi)
{
//
// Look for the beginning of the thread
//
   MsgC		*minMsg = mi.msg;
   time_t	minTime = minMsg->Time();

//
// Loop through non-deleted messages checking the date
//
   MsgItemC	*item = StartOfThread(mi);
   while ( item ) {

      MsgC	*msg = item->msg;
      if ( !item->IsDeleted() && msg && msg->Time() < minTime ) {
	 minTime = msg->Time();
	 minMsg  = msg;
      }

      item = item->nextInThread;

   } // End for each message 

   return minMsg;

} // End OldestMessageInThread

/*------------------------------------------------------------------------
 * Function to find the newest message in the thread that contains the specified
 *    message.
 */

static MsgC*
NewestMessageInThread(MsgItemC& mi)
{
//
// Look for the beginning of the thread
//
   MsgC		*maxMsg = mi.msg;
   time_t	maxTime = maxMsg->Time();

//
// Loop through non-deleted messages checking the date
//
   MsgItemC	*item = StartOfThread(mi);
   while ( item ) {

      MsgC	*msg = item->msg;
      if ( !item->IsDeleted() && msg && msg->Time() > maxTime ) {
	 maxTime = msg->Time();
	 maxMsg  = msg;
      }

      item = item->nextInThread;

   } // End for each message 

   return maxMsg;

} // End NewestMessageInThread

/*------------------------------------------------------------------------
 * Function to determine if there are any new messages in a thread
 */

static Boolean
ThreadIsNew(MsgItemC& mi)
{
//
// Look for the beginning of the thread
//
   MsgItemC	*item = StartOfThread(mi);

//
// Loop through non-deleted messages checking the status
//
   while ( item ) {

      if ( !item->IsDeleted() && item->IsNew() )
	 return True;

      item = item->nextInThread;

   } // End for each message 

   return False;

} // End ThreadIsNew

/*------------------------------------------------------------------------
 * Function to determine if there are any unread messages in a thread
 */

static Boolean
ThreadIsUnread(MsgItemC& mi)
{
//
// Look for the beginning of the thread
//
   MsgItemC	*item = StartOfThread(mi);

//
// Loop through non-deleted messages checking the status
//
   while ( item ) {

      if ( !item->IsDeleted() && item->IsUnread() )
	 return True;

      item = item->nextInThread;

   } // End for each message 

   return False;

} // End ThreadIsUnread

/*------------------------------------------------------------------------
 * Function to count the number of lines in a thread.
 */

static int
LinesInThread(MsgItemC& mi)
{
//
// Look for the beginning of the thread
//
   MsgItemC	*item = StartOfThread(mi);

//
// Loop through non-deleted messages adding the line counts
//
   int	lines = 0;
   while ( item ) {

      MsgC	*msg = item->msg;
      if ( !item->IsDeleted() && msg )
	 lines += msg->BodyLines();

      item = item->nextInThread;

   } // End for each message 

   return lines;

} // End LinesInThread

/*------------------------------------------------------------------------
 * Function to count the number of bytes in a thread.
 */

static int
BytesInThread(MsgItemC& mi)
{
//
// Look for the beginning of the thread
//
   MsgItemC	*item = StartOfThread(mi);

//
// Loop through non-deleted messages adding the byte counts
//
   int	bytes = 0;
   while ( item ) {

      MsgC	*msg = item->msg;
      if ( !item->IsDeleted() && msg )
	 bytes += msg->BodyBytes();

      item = item->nextInThread;

   } // End for each message 

   return bytes;

} // End BytesInThread

/*-----------------------------------------------------------------------
 * Thread compare method
 */

int
MailSortKeyC::CompareThreads(MsgItemC& mi1, MsgItemC& mi2,
			     SortKeyC::SortDirT threadDir) const
{
   static StringC	*s1 = NULL;
   static StringC	*s2 = NULL;
   if ( !s1 ) {
      s1 = new StringC();
      s2 = new StringC();
   }

//
// Get a representative message for each thread
//
   MsgC	*msg1 = mi1.msg;
   MsgC *msg2 = mi2.msg;
   int	val   = 0;

   if ( debuglev > 3 ) cout <<"Using ";

//
// Compare the messages
//
   switch (type) {

      case (NUMBER): {

	 if ( threadDir == MailSortKeyC::ASCENDING ) {
	    msg1 = OldestMessageInThread(mi1);
	    msg2 = OldestMessageInThread(mi2);
	    if ( debuglev > 3 ) cout <<"number of oldest in thread";
	 }
	 else {
	    msg1 = NewestMessageInThread(mi1);
	    msg2 = NewestMessageInThread(mi2);
	    if ( debuglev > 3 ) cout <<"number of newest in thread";
	 }

	 int	n1 = msg1->Number();
	 int	n2 = msg2->Number();
	 val = n1 - n2;
	 if ( dir == DESCENDING ) val = -val;

      } break;

      case (STATUS): {

	 int	less = (dir == ASCENDING) ? -1 :  1;

	 if ( ThreadIsNew(mi1) ) {
	    if ( ThreadIsNew(mi2) )		val =  0;
	    else				val =  less;
	 }
	 else if ( ThreadIsUnread(mi1) ) {
	    if ( ThreadIsNew(mi2) )		val = -less;	// greater
	    else if ( ThreadIsUnread(mi2) )	val =  0;
	    else				val =  less;
	 }
	 else { // mi1 thread is all read
	    if ( ThreadIsNew(mi2) ||
	       	 ThreadIsUnread(mi2) )		val = -less;	// greater
	    else				val =  0;
	 }

	 if ( debuglev > 3 ) cout <<"status of thread";

      } break;

      case (SENDER): {

	 if ( threadDir == MailSortKeyC::ASCENDING ) {
	    msg1 = OldestMessageInThread(mi1);
	    msg2 = OldestMessageInThread(mi2);
	    if ( debuglev > 3 ) cout <<"sender of oldest in thread";
	 }
	 else {
	    msg1 = NewestMessageInThread(mi1);
	    msg2 = NewestMessageInThread(mi2);
	    if ( debuglev > 3 ) cout <<"sender of newest in thread";
	 }

	 AddressC	*a1 = msg1->From();
	 AddressC	*a2 = msg2->From();

	 if ( !a1 && !a2 ) val =  0;
	 else if ( !a1 )   val = -1;
	 else if ( !a2 )   val =  1;

	 if ( a1->name ) a1->name->GetValueText(*s1);
	 else		 *s1 = a1->addr;
	 if ( a2->name ) a2->name->GetValueText(*s2);
	 else		 *s2 = a2->addr;

	 val = ( dir == ASCENDING ? s1->compare(*s2) : s2->compare(*s1) );

      } break;

      case (TO): {

	 if ( threadDir == MailSortKeyC::ASCENDING ) {
	    msg1 = OldestMessageInThread(mi1);
	    msg2 = OldestMessageInThread(mi2);
	    if ( debuglev > 3 ) cout <<"TO of oldest in thread";
	 }
	 else {
	    msg1 = NewestMessageInThread(mi1);
	    msg2 = NewestMessageInThread(mi2);
	    if ( debuglev > 3 ) cout <<"TO of newest in thread";
	 }

	 AddressC	*a1 = msg1->To();
	 AddressC	*a2 = msg2->To();

	 if ( !a1 && !a2 ) val =  0;
	 else if ( !a1 )   val = -1;
	 else if ( !a2 )   val =  1;

	 if ( a1->name ) a1->name->GetValueText(*s1);
	 else		 *s1 = a1->addr;
	 if ( a2->name ) a2->name->GetValueText(*s2);
	 else		 *s2 = a2->addr;

	 val = ( dir == ASCENDING ? s1->compare(*s2) : s2->compare(*s1) );

      } break;

      case (SUBJECT): {

//
// The thread is the subject without the Re's
//
	 *s1 = msg1->Thread();
	 *s2 = msg2->Thread();

	 val = ( dir == ASCENDING ? s1->compare(*s2) : s2->compare(*s1) );
	 if ( debuglev > 3 ) cout <<"subject of thread";
      }

      case (DATE):

	 if ( threadDir == MailSortKeyC::ASCENDING ) {
	    msg1 = OldestMessageInThread(mi1);
	    msg2 = OldestMessageInThread(mi2);
	    if ( debuglev > 3 ) cout <<"date of oldest in thread";
	 }
	 else {
	    msg1 = NewestMessageInThread(mi1);
	    msg2 = NewestMessageInThread(mi2);
	    if ( debuglev > 3 ) cout <<"date of newest in thread";
	 }

	 val = (int)(dir == ASCENDING ? msg1->Time() - msg2->Time()
				      : msg2->Time() - msg1->Time());
	 break;

      case (LINES): {
	 int	lines1 = LinesInThread(mi1);
	 int	lines2 = LinesInThread(mi2);
	 val = ( dir == ASCENDING ? lines1 - lines2 : lines2 - lines1 );
	 if ( debuglev > 3 ) cout <<"lines in thread";
      } break;

      case (BYTES): {
	 int	bytes1 = BytesInThread(mi1);
	 int	bytes2 = BytesInThread(mi2);
	 val = ( dir == ASCENDING ? bytes1 - bytes2 : bytes2 - bytes1 );
	 if ( debuglev > 3 ) cout <<"bytes in thread";
      } break;

      case (THREAD):
	 if ( debuglev > 3 ) cout <<"thread of thread";
	 break;

   } // End switch sort key type

   if ( debuglev > 3 ) {
      cout <<", msg " <<dec(msg1->Number(),4); 
      if      ( val < 0 ) cout <<" < ";
      else if ( val > 0 ) cout <<" > ";
      else		  cout <<" = ";
      cout <<"msg " <<dec(msg2->Number(),4) <<endl;
   }

   return val;

} // End CompareThreads

