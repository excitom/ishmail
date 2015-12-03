/*
 * $Id: olias.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
 *
 * Copyright (c) 1992 HaL Computer Systems, Inc.  All rights reserved.
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


#ifndef _olias_h
#define _olias_h

#include <X11/Intrinsic.h>

typedef enum
{
  OLIAS_NOOP_EVENT = 0, OLIAS_DISPLAY_EVENT = 1,
} OliasEventType;

typedef struct
{
  OliasEventType    type;
  char             *infobase;
  char             *locator;
} OliasDisplayEvent;

typedef union
{
  OliasEventType     type;
  OliasDisplayEvent  display_event;
} OliasEvent;

typedef enum
{
  OLIAS_SUCCESS = 1, OLIAS_TIMEOUT = 2, OLIAS_LOCATOR_NOT_FOUND = 3
} OliasStatus;

_XFUNCPROTOBEGIN

extern OliasStatus olias_send_event (
#if NeedFunctionPrototypes
    Widget			/* toplevel */,
    OliasEvent * 		/* olias command */
#endif
);

_XFUNCPROTOEND

#endif /* _olias_h */
/* DO NOT ADD ANY LINES AFTER THIS #endif */
