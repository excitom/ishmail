/*
 *  $Id: MimeEncode.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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

#ifndef _MimeEncode_h_
#define _MimeEncode_h_

#include <X11/Intrinsic.h>

#include <stdio.h>

class CharC;
class StringC;

//
// UUencode encoding routines
//
extern Boolean	FileToFileUU(const char*, const char*, FILE *ofp=NULL);
extern Boolean	FileToTextUU(const char*, StringC*);
extern Boolean	TextToFileUU(CharC,       const char*);
extern Boolean	TextToTextUU(CharC,       StringC*);

//
// UUencode decoding routines
//
extern Boolean	FileUUToFile(const char*, const char*);
extern Boolean	FileUUToText(const char*, StringC*);
extern Boolean	TextUUToFile(CharC,       const char*);
extern Boolean	TextUUToText(CharC,       StringC*);

//
// BinHex encoding routines
//
extern Boolean	FileToFileBH(const char*, const char*, FILE *ofp=NULL);
extern Boolean	FileToTextBH(const char*, StringC*);
extern Boolean	TextToFileBH(CharC,       const char*);
extern Boolean	TextToTextBH(CharC,       StringC*);

//
// BinHex decoding routines
//
extern Boolean	FileBHToFile(const char*, const char*);
extern Boolean	FileBHToText(const char*, StringC*);
extern Boolean	TextBHToFile(CharC,       const char*);
extern Boolean	TextBHToText(CharC,       StringC*);

#endif // _MimeEncode_h_
