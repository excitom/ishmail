/*
 *  $Id: Base64.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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
#ifndef _Base64_h_
#define _Base64_h_

#include <X11/Intrinsic.h>

#include <stdio.h>

class CharC;
class StringC;
class RegexC;

//
// Base 64 encoding routines
//
extern Boolean	TextToText64(CharC istr, StringC *ostr, Boolean text=False,
			     Boolean breakLines=True);
extern Boolean	FileToFile64(const char *ifile, const char *ofile,
			     Boolean text=False, FILE *ifp=NULL, FILE *ofp=NULL,
			     u_int offset=0, u_int length=0);
extern Boolean	TextToFile64(CharC istr, const char *ofile, Boolean text=False,
			     Boolean breakLines=True, FILE *ofp=NULL);
extern Boolean	FileToText64(const char *ifile, StringC *ostr,
			     Boolean text=False, FILE *ifp=NULL, u_int offset=0,
			     u_int length=0);
extern Boolean	TextToText1522B(CharC istr, CharC charset, StringC *ostr);

//
// Base 64 decoding routines
//
extern Boolean	Text64ToText(CharC istr, StringC *ostr, Boolean text=False);
extern Boolean	File64ToFile(const char *ifile, const char *ofile,
			     Boolean text=False, FILE *ifp=NULL, FILE *ofp=NULL,
			     u_int offset=0, u_int length=0);
extern Boolean	Text64ToFile(CharC istr, const char *ofile, Boolean text=False,
			     FILE *ofp=NULL);
extern Boolean	File64ToText(const char *ifile, StringC *ostr,
			     Boolean text=False, FILE *ifp=NULL, u_int offset=0,
			     u_int length=0);

#endif // _Base64_h_
