/*
 * $Id: MsgItemC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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

#ifndef _MsgItemC_h_
#define _MsgItemC_h_

#include <hgl/VItemC.h>
#include <hgl/PixmapC.h>

class MsgC;
class StringC;

class MsgItemC : public VItemC {

private:

//
// Private methods
//
   void			GetStatusString(StringC&);
   XpmT			StatusPixmap();

public:

   MsgC			*msg;		// Message body information
   MsgItemC		*prevInThread;
   MsgItemC		*nextInThread;

   MsgItemC(MsgC*);
   ~MsgItemC();

//
// Set
//
   void			LoadFields();
   void			UpdateStatus();

//
// Set status
//
   void			SetCurrent();
   void			SetDeleted();
   void			SetFiltered();
   void			SetForwarded();
   void			SetNew();
   void			SetPrinted();
   void			SetRead();
   void			SetReplied();
   void			SetResent();
   void			SetSaved();
   void			SetViewed() { SetCurrent(); }

//
// Clear status
//
   void			ClearCurrent();
   void			ClearDeleted();
   void			ClearFiltered();
   void			ClearForwarded();
   void			ClearNew();
   void			ClearPrinted();
   void			ClearRead();
   void			ClearReplied();
   void			ClearResent();
   void			ClearSaved();
   void			ClearViewed() { ClearCurrent(); }

//
// Query status
//
   Boolean		IsCurrent()	const;
   Boolean		IsDeleted()	const;
   Boolean		IsFiltered()	const;
   Boolean		IsForwarded()	const;
   Boolean		IsMime()	const;
   Boolean		IsNew()		const;
   Boolean		IsPartial()	const;
   Boolean		IsPrinted()	const;
   Boolean		IsRead()	const;
   Boolean		IsReplied()	const;
   Boolean		IsResent()	const;
   Boolean		IsSaved()	const;
   Boolean		IsUnread()	const;
   Boolean		IsViewed()	const;

//
// Sorting
//
   static int		MsgItemCompare(const void*, const void*);
};

#endif // _MsgItemC_h_
