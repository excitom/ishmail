/*
 *  $Id: FileMisc.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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
#ifndef _FileMisc_h_
#define _FileMisc_h_

#include <stdio.h>

class CharC;
class StringC;
class StringDictC;
class StringListC;

extern CharC		BaseName(CharC);
extern CharC		DirName(CharC);
extern StringC		FullPathname(StringC);
extern void		GetFileType  (CharC, StringC&);
extern void		GetTypeSuffix(CharC, StringC&);
extern Boolean		IsDir(char*);
extern Boolean		MakeDir(StringC);
extern Boolean		MakeFile(StringC);
extern StringDictC&	MimeTypeDict();
extern Boolean		RemoveDir(StringC);
extern Boolean		CopyFile(char *srcFile, char *dstFile,
				 Boolean addBlank=False,
				 Boolean protectFroms=False,
				 FILE *srcFp=NULL, FILE *dstFp=NULL);
extern Boolean		WriteHeaders(StringListC& headList, char *dstFile,
				     Boolean addBlank, FILE *dstFp=NULL);

#endif // _FileMisc_h_
