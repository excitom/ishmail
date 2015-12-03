/*
 *  $Id: ImapMisc.h,v 1.2 2000/12/13 17:15:39 evgeny Exp $
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
#ifndef _ImapMisc_h
#define _ImapMisc_h

#include <X11/Intrinsic.h>

class CharC;
class RegexC;

extern void	BuildImapPat();
extern Boolean	IsImapName(CharC);
extern Boolean	IsImapUrl(CharC);
extern CharC	ImapServerPart(CharC);
extern CharC	ImapPathPart(CharC);

extern Boolean ParseImapUrl(CharC, StringC&, long *, Boolean *, StringC&, StringC&);
extern Boolean ParseImapName(CharC, StringC&, long *, Boolean *, StringC&, StringC&);

#endif // _ImapMisc_h
