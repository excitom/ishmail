/*
 *  $Id: MailPrefC.h,v 1.2 2000/06/19 12:42:04 evgeny Exp $
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
#ifndef _MailPrefC_h
#define _MailPrefC_h

#include "PrefC.h"
#include "MimeTypes.h"

#include <hgl/StringC.h>
#include <hgl/StringListC.h>

enum OutgoingMailTypeT {
   MAIL_PLAIN,
   MAIL_MIME,
   MAIL_ALT
};

enum FccTypeT {
   FCC_NONE,
   FCC_TO_FOLDER,
   FCC_BY_USER,
   FCC_BY_ADDRESS,
   FCC_BY_YEAR,
   FCC_BY_MONTH,
   FCC_BY_WEEK,
   FCC_BY_DAY
};

class MailPrefWinC;

class MailPrefC : public PrefC {

//
// Private data.  We don't want anyone setting these directly.
//
   MailPrefWinC		*prefWin;

//
// As typed by the user
//
   struct {
      StringC		deadFile;	// Use this file for saved compositions
      StringC		sendmailCmd;
      StringC		fccFolder;	// Use this folder if FCC_TO_FOLDER
      StringC		fccFolderDir;	// Use this directory if !FCC_TO_FOLDER
   } orig;

//
// Fully expanded
//
   StringC		deadFile;	// Use this file for saved compositions
   StringC		sendmailCmd;
   StringC		fccFolder;	// Use this folder if FCC_TO_FOLDER
   StringC		fccFolderDir;	// Use this directory if !FCC_TO_FOLDER

public:

//
// Public data
//
   Boolean		split;
   int			splitSize;
   Boolean		verifyAddresses;// Try to check addresses before sending
   Boolean		saveOnInterrupt;// Save interrupted messages
   StringC		fromHeader;	// The "From:" header
   StringC		otherHeaders;	// Other headers default
   OutgoingMailTypeT	mailType;
   MimeContentType	textType;	// Plain or enriched
   StringC		charset;
   MimeEncodingType	bodyEncType;
   MimeEncodingType	headEncType;
   StringListC		confirmAddrList;// List of "are-you-sure" addresses
   Boolean		confirmAddrs;	// Whether to check list
   FccTypeT		fccType;	// How to copy outgoing messages

//
// Public methods
//
    MailPrefC();
   ~MailPrefC();

   void		SetDeadFile(const char*);
   void		SetSendmailCmd(const char*);
   void		SetFccFolder(const char*);
   void		SetFccFolderDir(const char*);

   StringC&	DeadFile()	{ return deadFile; }
   StringC&	SendmailCmd()	{ return sendmailCmd; }
   StringC&	FccFolder()	{ return fccFolder; }
   StringC&	FccFolderDir()	{ return fccFolderDir; }

   StringC&	OrigDeadFile()		{ return orig.deadFile; }
   StringC&	OrigSendmailCmd()	{ return orig.sendmailCmd; }
   StringC&	OrigFccFolder()		{ return orig.fccFolder; }
   StringC&	OrigFccFolderDir()	{ return orig.fccFolderDir; }

   void		Edit(Widget);
   Boolean	WriteDatabase();
   Boolean	WriteFile();
};

#endif // _MailPrefC_h
