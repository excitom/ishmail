/*
 * $Id: SortMgrC.C,v 1.2 2000/05/07 12:26:13 fnevgeny Exp $
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

#include "SortMgrC.h"
#include "MailSortKeyC.h"
#include "MsgItemC.h"

#include <hgl/RegexC.h>
#include <hgl/CharC.h>

#include <X11/Intrinsic.h>

/*---------------------------------------------------------------
 *  Constructor
 */

SortMgrC::SortMgrC(CharC str)
{
   threaded   = False;
   threadDir  = SortKeyC::ASCENDING;
   numberKey  = NULL;
   statusKey  = NULL;
   senderKey  = NULL;
   toKey      = NULL;
   subjectKey = NULL;
   dateKey    = NULL;
   linesKey   = NULL;
   bytesKey   = NULL;

   Set(str);

} // End constructor

/*---------------------------------------------------------------
 *  Destructor
 */

SortMgrC::~SortMgrC()
{
   delete numberKey;
   delete statusKey;
   delete senderKey;
   delete toKey;
   delete subjectKey;
   delete dateKey;
   delete linesKey;
   delete bytesKey;

} // End destructor

/*---------------------------------------------------------------
 *  Method to initialize with the given key string
 */

void
SortMgrC::Set(CharC str)
{
//
// Clean up
//
   keyList.removeAll();

   delete numberKey;
   delete statusKey;
   delete senderKey;
   delete toKey;
   delete subjectKey;
   delete dateKey;
   delete linesKey;
   delete bytesKey;

   threaded   = False;
   threadDir  = SortKeyC::ASCENDING;
   numberKey  = NULL;
   statusKey  = NULL;
   senderKey  = NULL;
   toKey      = NULL;
   subjectKey = NULL;
   dateKey    = NULL;
   linesKey   = NULL;
   bytesKey   = NULL;

   keyStr     = str;

//
// Extract sort keys
//
   if ( keyStr.size() > 0 ) {

      static	RegexC	*wordPat = NULL;
      static	RegexC	*keyPat  = NULL;
      if ( !wordPat ) {
	 wordPat = new RegexC("[^ \t]+");
	 keyPat  = new RegexC("\\(.+\\)(\\([AD]\\))");
      }

//
// Extract sort keys
//
      StringC	word, key;
      char	order;
      while ( wordPat->search(keyStr) >= 0 ) {

	 word = keyStr((*wordPat)[0]);
	 keyStr((*wordPat)[0]) = "";

	 word.toUpper();
	 //cout <<"word is " <<word NL;

//
// Decode sort key and direction
//
	 if ( keyPat->match(word) ) {
	    key   = word((*keyPat)[1]);
	    order = ((StringC)(word((*keyPat)[2])))[0];
	 } else {
	    key   = word;
	    order = 'A';
	 }
	 //cout <<"key is " <<key <<". Order is " <<order NL;
	 Boolean		descend = (order == 'D');
	 SortKeyC::SortDirT	dir = descend ? SortKeyC::DESCENDING
					      : SortKeyC::ASCENDING;

//
// Add sort key to expression
//
	 if ( key == "THREAD" ) {
	    threaded  = True;
	    threadDir = dir;
	 }
	 else if ( key == "NUMBER" ) {
	    numberKey = new MailSortKeyC(MailSortKeyC::NUMBER, dir);
	    keyList.append(numberKey);
	 }
	 else if ( key == "STATUS" ) {
	    statusKey = new MailSortKeyC(MailSortKeyC::STATUS, dir);
	    keyList.append(statusKey);
	 }
	 else if ( key == "SENDER" ) {
	    senderKey = new MailSortKeyC(MailSortKeyC::SENDER, dir);
	    keyList.append(senderKey);
	 }
	 else if ( key == "TO" ) {
	    toKey = new MailSortKeyC(MailSortKeyC::TO, dir);
	    keyList.append(toKey);
	 }
	 else if ( key == "SUBJECT" ) {
	    subjectKey = new MailSortKeyC(MailSortKeyC::SUBJECT, dir);
	    keyList.append(subjectKey);
	 }
	 else if ( key == "DATE" ) {
	    dateKey = new MailSortKeyC(MailSortKeyC::DATE, dir);
	    keyList.append(dateKey);
	 }
	 else if ( key == "LINES" ) {
	    linesKey = new MailSortKeyC(MailSortKeyC::LINES, dir);
	    keyList.append(linesKey);
	 }
	 else if ( key == "BYTES" || key == "SIZE" ) { // size for backward
							 // compatibility
	    bytesKey = new MailSortKeyC(MailSortKeyC::BYTES, dir);
	    keyList.append(bytesKey);
	 }

      } // End for each sort key

   } // End if any sort keys were found

//
// If the key list is empty, add a number key
//
   if ( keyList.size() == 0 ) {
      numberKey = new MailSortKeyC(MailSortKeyC::NUMBER, SortKeyC::ASCENDING);
      keyList.append(numberKey);
   }

//
// Regenerate the key string
//
   BuildKeyString();

} // End Set

/*---------------------------------------------------------------
 *  Method to construct key string from key list
 */

void
SortMgrC::BuildKeyString()
{
   keyStr = "";

   Boolean	needSpace = False;
   if ( threaded ) {

      keyStr += "THREAD";

      if ( threadDir == SortKeyC::ASCENDING ) keyStr += "(A)";
      else				      keyStr += "(D)";

      needSpace = True;
   }

//
// Build sort key string from list of keys
//
   unsigned	keyCount = keyList.size();
   for (int i=0; i<keyCount; i++) {

      MailSortKeyC	*key = (MailSortKeyC *)keyList[i];

      if ( needSpace ) keyStr += " ";

      switch ( key->Type() ) {

	 case (MailSortKeyC::NUMBER):
	    keyStr += "NUMBER";
	    break;

	 case (MailSortKeyC::STATUS):
	    keyStr += "STATUS";
	    break;

	 case (MailSortKeyC::SENDER):
	    keyStr += "SENDER";
	    break;

	 case (MailSortKeyC::TO):
	    keyStr += "TO";
	    break;

	 case (MailSortKeyC::SUBJECT):
	    keyStr += "SUBJECT";
	    break;

	 case (MailSortKeyC::DATE):
	    keyStr += "DATE";
	    break;

	 case (MailSortKeyC::LINES):
	    keyStr += "LINES";
	    break;

	 case (MailSortKeyC::BYTES):
	    keyStr += "BYTES";
	    break;

	 case (MailSortKeyC::THREAD):
	    break;
      }

      if ( key->Dir() == SortKeyC::ASCENDING ) keyStr += "(A)";
      else				       keyStr += "(D)";

      needSpace = True;

   } // End for each sort key

} // End BuildKeyString

