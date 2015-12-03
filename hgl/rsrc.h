/*
 * $Id: rsrc.h,v 1.3 2001/03/29 11:14:03 evgeny Exp $
 *
 * Copyright (c) 1992 HAL Computer Systems International, Ltd.
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

#ifndef _rsrc_h_
#define _rsrc_h_

#include "StringC.h"

#include <Xm/Xm.h>

#include <stdio.h>

typedef struct {
   StringC	name;
   int		r, g, b;
} RgbColorT;

class IntListC;
class StringListC;
class MimeRichTextC;

extern void	set_sensitivity(Widget, Boolean);
extern void	set_optional   (Widget, Boolean);
extern void	set_invalid    (Widget, Boolean, Boolean restore=False);
extern void	set_invalid    (MimeRichTextC*, Boolean, Boolean restore=False);
extern void	set_color      (Widget, const char*, const char*, Pixel def=0);

//
// get_ routines that use the widget's class name
//
extern Boolean 	get_boolean(Widget, const char*, Boolean def=False);
extern Pixel   	get_color  (Widget, const char*, Pixel def=0);
extern Pixel   	get_color  (Widget, const char*, const char*);
extern Cursor  	get_cursor (Widget, const char*, const char *def="");
extern float	get_float  (Widget, const char*, float def=0.0);
extern int	get_gravity(Widget, const char*, const char *def="NorthWest");
extern int	get_int    (Widget, const char*, int def=0);
extern unsigned char get_orient(Widget, const char*,
				unsigned char def=XmVERTICAL);
extern unsigned char get_shadow_type(Widget, const char*,
				     unsigned char def=XmSHADOW_IN);
extern StringC 	get_string (Widget, const char*, const char *def="");

//
// get_ routines that accept a custom class name
//
extern Boolean 	get_boolean(const char*, Widget, const char*,
			    Boolean def=False);
extern Pixel   	get_color  (const char*, Widget, const char*, Pixel def=0);
extern Pixel   	get_color  (const char*, Widget, const char*, const char*);
extern Cursor  	get_cursor (const char*, Widget, const char*,
			    const char *def="");
extern float	get_float  (const char*, Widget, const char*, float def=0.0);
extern int	get_gravity(const char*, Widget, const char*,
			    const char *def="NorthWest");
extern int	get_int    (const char*, Widget, const char*, int def=0);
extern unsigned char get_orient(const char*, Widget, const char*,
				unsigned char def=XmVERTICAL);
extern unsigned char get_shadow_type(const char*, Widget, const char*,
				     unsigned char def=XmSHADOW_IN);
extern StringC 	get_string (const char*, Widget, const char*,
			    const char *def="");

//
// get_ routines that accept a class name, widget name and parent
//
extern Boolean 	get_boolean(const char*, const char*, const char*, Widget,
			    Boolean def=False);
extern Pixel   	get_color  (const char*, const char*, const char*, Widget,
			    Pixel def=0);
extern Pixel   	get_color  (const char*, const char*, const char*, Widget,
			    const char*);
extern Cursor  	get_cursor (const char*, const char*, const char*, Widget,
			    const char *def="");
extern float	get_float  (const char*, const char*, const char*, Widget,
			    float def=0.0);
extern int	get_gravity(const char*, const char*, const char*, Widget,
			    const char *def="NorthWest");
extern int	get_int    (const char*, const char*, const char*, Widget,
			    int def=0);
extern u_char	get_orient (const char*, const char*, const char*, Widget,
			    unsigned char def=XmVERTICAL);
extern StringC 	get_string (const char*, const char*, const char*, Widget,
			    const char *def="");
extern u_char	get_shadow_type(const char*, const char*, const char*, Widget,
				unsigned char def=XmSHADOW_IN);

//
// Misc routines
//

extern void	restore_text(Widget, XtPointer, XtPointer);
extern void	restore_richtext(MimeRichTextC*, XtPointer);

extern StringC	ColorName(Widget, Pixel);
extern Boolean	PixelValue(Widget, const char*, Pixel*);

extern void	WriteResource   (FILE*, const char*, const char*);
extern void	WriteResource   (FILE*, const char*, float);
extern void	WriteResource   (FILE*, const char*, int);
extern void	WriteResource   (FILE*, const char*, unsigned);
extern void	WriteResource   (FILE*, const char*, Dimension);
extern void	WriteResource   (FILE*, const char*, Boolean);
extern void	WriteShadowType (FILE*, const char*, unsigned char);
extern void	WriteOrientation(FILE*, const char*, unsigned char);
extern void	WriteResource   (FILE*, const char*, const IntListC&);
extern void	WriteResource   (FILE*, const char*, const StringListC&);

#endif // _rsrc_h_
