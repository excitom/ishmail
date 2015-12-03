/*
 * $Id: Base.h,v 1.4 2000/08/20 13:57:06 evgeny Exp $
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

#ifndef _Base_h_
#define _Base_h_

#include <stream.h>
#include <stdlib.h>
#include <ctype.h> 
#include <sys/param.h> 
#ifdef WITH_DMALLOC
# include <string.h>
# define DMALLOC_FUNC_CHECK 1
# include <dmalloc.h>
#endif

#define MOD_SIZEOF(T) (sizeof(T)&sizeof(T)-1 ? i%sizeof(T) : i&sizeof(T)-1)

inline unsigned mod_sizeof_unsigned(unsigned i)	{ return(MOD_SIZEOF(unsigned));}

inline unsigned div_sizeof_unsigned(unsigned i)	{ return (i >> 2); }

#ifndef FALSE
#define FALSE 0
#define TRUE (!FALSE)
#endif

// Use this when creating managed widgets
#ifndef MANAGE
#define MANAGE 1
#endif

#ifndef NL
#define NL <<endl
#endif

#ifndef SP
#define SP << ' ' <<
#endif

#ifndef MIN
#define MIN(a,b) ((a)<(b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b) ((a)>(b) ? (a) : (b))
#endif

#define MAX_DIGITS_IN_INT  64

typedef RETSIGTYPE (*SIG_PF)(int);

#ifndef toascii
#define toascii(c)      ((c) & 0177)
#endif

#ifndef isascii
#define isascii(c)      (!((c) & ~0177))
#endif

//---------------------------------------------------------------------------
//  System-independent versions of toupper and tolower

inline char to_upper(unsigned char c) {
   return (islower(c) ? (c-'a'+'A') : c);
}

inline char to_lower(unsigned char c){
   return (isupper(c) ? (c-'A'+'a') : c);
}

#ifndef MEMBER_QUERY
# define MEMBER_QUERY(TYP,METH,MEMB) \
	inline TYP		METH(void) const	{ return MEMB; }
#endif

#ifndef PTR_QUERY
#define PTR_QUERY(TYP,METH,MEMB) \
	inline TYP		METH(void)		{ return MEMB; } \
	inline const TYP	METH(void) const	{ return MEMB; }
#endif

#endif // _Base_h_
