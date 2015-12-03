/*
 *  $Id: AddrMisc.C,v 1.3 2000/06/19 12:44:14 evgeny Exp $
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
#include "IshAppC.h"
#include "MailPrefC.h"
#include "AddressC.h"

#include <hgl/CharC.h>

/*---------------------------------------------------------------
 *  Function to determine if the given address corresponds to the
 *  current user.
 */

Boolean
IsMyAddress(CharC addr)
{
   AddressC fromAddr(ishApp->mailPrefs->fromHeader);
   return ( addr.Equals(ishApp->userId,		  IGNORE_CASE) ||
            addr.Equals(ishApp->userAtHost,	  IGNORE_CASE) ||
            addr.Equals(ishApp->userAtDomain,	  IGNORE_CASE) ||
            addr.Equals(ishApp->userAtHostDomain, IGNORE_CASE) ||
            addr.Search(fromAddr.addr,            IGNORE_CASE) );
}

