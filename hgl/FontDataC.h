/*
 * $Id: FontDataC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
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

#ifndef _FontDataC_h_
#define _FontDataC_h_

#include <X11/Intrinsic.h>

#include "StringC.h"
#include "RangeC.h"
#include "StringListC.h"
#include "IntListC.h"
#include "PtrListC.h"
#include "CharC.h"

class FontDataC;

typedef PtrListC	FontListC;

//
// Structure containing font properties
//
class FontDataC {

private:

   FontDataC*		bigger;		// Pointer to next larger font
   FontDataC*		smaller;	// Pointer to next smaller font
   FontDataC*		bold;		// Pointer to bold version of font
   FontDataC*		italic;		// Pointer to italic version of font
   FontDataC*		nonBold;	// Pointer to bold version of font
   FontDataC*		nonItalic;	// Pointer to italic version of font
   StringListC		charSetNames;	// List of alternate charset names
   FontListC		charSetFonts;	// List of alternate charset fonts

   Boolean		freeBigger;	// True if bigger can be freed
   Boolean		freeSmaller;	// True if smaller can be freed
   Boolean		freeBold;	// True if bold can be freed
   Boolean		freeItalic;	// True if italic can be freed
   Boolean		freeNonBold;	// True if nonBold can be freed
   Boolean		freeNonItalic;	// True if nonItalic can be freed
   IntListC		freeCharSet;	// True if charset can be freed

//
// XFLD data
//
   StringC	name;
   CharC	foundry;
   CharC	family;
   CharC	weight;
   CharC	slant;
   CharC	setWidth;
   CharC	addStyle;
   CharC	pixelSize;
   CharC	pointSize;
   CharC	resStr;
   CharC	spacing;
   CharC	avgWidth;
   CharC	charset;
   int		pixels;
   int		points;

//
// Private methods
//
   void		BuildName(CharC, CharC, CharC, CharC, CharC, CharC, CharC,
   			  CharC, CharC, CharC, CharC, CharC, StringC&);
   FontDataC	*CreateBold(char*);
   FontDataC	*CreateCharset(char*, char*);
   FontDataC	*CreateItalic(char*);
   FontDataC	*CreateNonBold(char*);
   FontDataC	*CreateNonItalic(char*);
   Boolean	FindBigger (StringC&);
   Boolean	FindFont   (StringC&);
   Boolean	FindSmaller(StringC&);
   void		Init();
   void		Reset(Boolean init=True);

public:

//
// Properties
//
   unsigned	normSpace;	// Normal interword spacing
   unsigned	endSpace;	// Normal end-of-sentence spacing
   int		superX;		// x-offset to superscript origin
   int		superY;		// y-offset to superscript origin
   int		subX;		// x-offset to subscript origin
   int		subY;		// y-offset to subscript origin
   int		underY;		// y-offset to underline top
   unsigned	underThick;	// thickness of underline

   XFontStruct	*xfont;		// Pointer to font info
   Font		fid;		// Font id
   Boolean	loaded;		// This font was loaded ok

//
// Methods
//
   FontDataC();
   FontDataC(const char*);
   ~FontDataC();

   FontDataC&	operator=(FontDataC&);
   FontDataC&	operator=(const char*);

   FontDataC*	Charset(StringC);
   FontDataC*	Bigger();
   FontDataC*	Bold();
   void		DecodeName();
   void		GetProps();
   FontDataC*	Italic();
   Boolean	IsBold();
   Boolean	IsFixed();
   Boolean	IsItalic();
   FontDataC*	NonBold();
   FontDataC*	NonItalic();
   FontDataC*	Smaller();

   PTR_QUERY(StringC&,	Name,	name)

   inline CharC		Charset() { return charset; }
   inline int		CharWidth() { return xfont->max_bounds.width; }
//      return ((xfont->min_bounds.width + xfont->max_bounds.width) / (int)2);
//   }
};

#endif // _FontDataC_h_
