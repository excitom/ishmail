/*
 * $Id: SortMgrC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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

#ifndef _SortMgrC_h_
#define _SortMgrC_h_

#include "MailSortKeyC.h"

#include <hgl/StringC.h>
#include <hgl/SortKeyListC.h>

class CharC;

/*--------------------------------------------------------------------------
 * Message sort information
 */

class SortMgrC {

   Boolean		threaded;
   SortKeyC::SortDirT	threadDir;
   MailSortKeyC		*numberKey;
   MailSortKeyC		*statusKey;
   MailSortKeyC		*senderKey;
   MailSortKeyC		*toKey;
   MailSortKeyC		*subjectKey;
   MailSortKeyC		*dateKey;
   MailSortKeyC		*linesKey;
   MailSortKeyC		*bytesKey;

   SortKeyListC		keyList;
   StringC		keyStr;

public:

// Methods

   SortMgrC(CharC);
   ~SortMgrC();

   void		BuildKeyString();
   void		Set(CharC);

      PTR_QUERY(StringC&,		KeyString,	keyStr);
      PTR_QUERY(SortKeyListC&,		KeyList,	keyList);
   MEMBER_QUERY(Boolean,		Threaded,	threaded);
   MEMBER_QUERY(SortKeyC::SortDirT,	ThreadDir,	threadDir);
      PTR_QUERY(MailSortKeyC*,		NumberKey,	numberKey);
      PTR_QUERY(MailSortKeyC*,		StatusKey,	statusKey);
      PTR_QUERY(MailSortKeyC*,		SenderKey,	senderKey);
      PTR_QUERY(MailSortKeyC*,		ToKey,		toKey);
      PTR_QUERY(MailSortKeyC*,		SubjectKey,	subjectKey);
      PTR_QUERY(MailSortKeyC*,		DateKey,	dateKey);
      PTR_QUERY(MailSortKeyC*,		LinesKey,	linesKey);
      PTR_QUERY(MailSortKeyC*,		BytesKey,	bytesKey);
};

#endif // _SortMgrC_h_
