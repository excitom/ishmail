/*
 *  $Id: UnixMsgC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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
#ifndef _UnixMsgC_h_
#define _UnixMsgC_h_

#include "FilePartMsgC.h"

#include <hgl/StringC.h>

class UnixFolderC;

class UnixMsgC : public FilePartMsgC {

   u_int	contentLen;
   u_int	contentLenExtra;
   Boolean	useContentLen;	// Set if header present

   static Boolean	TerminatingLine(UnixMsgC*, CharC, Boolean);

protected:

   void		CheckFroms(CharC);
   u_long	ExtraSpaceNeeded()
	        { return (fromLine.size() + 1/*nl*/ + 1/*final blank line*/); }
   void		*TerminatingLineFn();

public:

   StringC	fromLine;

   UnixMsgC(UnixFolderC*, int, u_int, const char*);
   ~UnixMsgC();

   Boolean	HasOpenFroms() const;
   Boolean	HasOpenFroms();
   Boolean	HasSafeFroms() const;
   Boolean	HasSafeFroms();
};

#endif // _UnixMsgC_h_
