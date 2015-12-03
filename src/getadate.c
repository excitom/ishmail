/*
 * $Id: getadate.c,v 1.3 2000/05/07 12:26:13 fnevgeny Exp $
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

#include <stdio.h>

#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#if !defined(HAVE_TM_ZONE) && !defined(HAVE_TZNAME)
# error "I don't know how to handle timezone on your OS!"
#endif

char *days[]   = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

extern int	debuglev;

char *
get_822_date()
{
   struct timeval	tv;
   struct timezone	tz;
   struct tm		*t;
   int			dstflag;
   int			zone;
   char			*zonename;
   static char		buffer[BUFSIZ];	/* static character buffer       */

   gettimeofday(&tv, &tz);
   t = localtime(&tv.tv_sec);
   tzset();

#ifdef HAVE_TZNAME
   if ( debuglev > 1 ) printf("daylight: %d\n", daylight);
   if ( debuglev > 1 ) printf("tm_isdst: %d\n", t->tm_isdst);
   dstflag  = daylight ? t->tm_isdst : 0;
#else
   if ( debuglev > 1 ) printf("tm_isdst: %d\n", t->tm_isdst);
   dstflag  = t->tm_isdst ? 1 : 0;
#endif
   if ( debuglev > 1 ) printf("dstflag: %d\n", dstflag);

#ifdef HAVE_TZNAME
   if ( debuglev > 1 ) printf("timezone: %d\n", timezone);
   zone = (dstflag * 60) - (timezone / 60);
#else
   if ( debuglev > 1 ) printf("tm_gmtoff: %d\n", t->tm_gmtoff);
   zone = (int)t->tm_gmtoff / (int)60;
#endif
   if ( debuglev > 1 ) printf("zone: %d\n", zone);

#if defined(HAVE_TM_ZONE)
   zonename = t->tm_zone;
#else /* HAVE_TZNAME */
   zonename = tzname[dstflag];
#endif
   if ( debuglev > 1 ) printf("zonename: %s\n", zonename);

   sprintf(buffer, "%s, %d %s %d %02d:%02d:%02d %+03d%02d (%s)\n",
           days[t->tm_wday],
	   t->tm_mday,
	   months[t->tm_mon],
	   t->tm_year+1900,
	   t->tm_hour,
	   t->tm_min,
	   t->tm_sec,
	   zone/60,
	   abs(zone) % 60,
	   zonename);

   if ( debuglev > 1 ) printf("date string: %s\n", buffer);
   return buffer;

} /* End get_822_date */
