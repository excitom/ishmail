/*
 * $Id: StrCase.C,v 1.4 2000/05/31 13:03:55 evgeny Exp $
 *
 * Copyright (c) 1994 HAL Computer Systems International, Ltd.
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

#include "StrCase.h"

#include <stdio.h>
#include <string.h>

// FIXME: HAVE_STRNCASECMP
/*---------------------------------------------------------------
 *  Function to search for any form of pat in src.
 */

char *
StrCaseStr(char *src, char *pat)
{
   int	patLen = strlen(pat);
   int	srcLen = strlen(src);
   char	*end = src + srcLen - patLen;

   while ( src <= end ) {
      if ( strncasecmp(src, pat, patLen) == 0 ) return src;
      src++;
   }

   return NULL;

} // End StrCaseStr
