/*
 * $Id: rsrc.C,v 1.4 2001/03/29 11:14:03 evgeny Exp $
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

#include <config.h>

#include "rsrc.h"

#include "IntListC.h"
#include "StringListC.h"
#include "RegexC.h"
#include "HalAppC.h"
#include "MimeRichTextC.h"

#include <X11/StringDefs.h>
#include <Xm/Xm.h>
#include <X11/Xmu/Converters.h>

#define BLACK	BlackPixel(halApp->display, DefaultScreen(halApp->display))
#define WHITE	WhitePixel(halApp->display, DefaultScreen(halApp->display))

/*---------------------------------------------------------------
 *  Return the color for a custom resource
 */

Pixel
get_color(Widget w, const char *rsrc, Pixel def)
{
   XtResource	res;
   res.resource_name   = (String)rsrc;
   res.resource_class  = XtCColor;
   res.resource_type   = XtRPixel;
   res.resource_size   = sizeof(Pixel);
   res.resource_offset = 0;
   res.default_type    = XtRPixel;
   res.default_addr    = (XtPointer)&def;

   Pixel	color;
   XtGetApplicationResources(w, &color, &res, 1, NULL, 0);
   return color;

} // End get_color

Pixel
get_color(Widget w, const char *rsrc, const char *def)
{
   XtResource	res;
   res.resource_name   = (String)rsrc;
   res.resource_class  = XtCColor;
   res.resource_type   = XtRPixel;
   res.resource_size   = sizeof(Pixel);
   res.resource_offset = 0;
   res.default_type    = XtRString;
   res.default_addr    = (XtPointer)def;

   Pixel	color;
   XtGetApplicationResources(w, &color, &res, 1, NULL, 0);
   return color;

} // End get_color

/*---------------------------------------------------------------
 *  Use a custom resource in place of a motif resource
 */

void
set_color(Widget w, const char *mrsrc, const char *crsrc, Pixel def)
{
   XtVaSetValues(w, mrsrc, get_color(w, crsrc, def), NULL);
}

/*---------------------------------------------------------------
 *  Routine to change color of widget for sensitivity feedback
 */

void
set_sensitivity(Widget w, Boolean set)
{
   if ( set ) {
      set_color(w, XmNbackground, XmNbackground, WHITE);
      set_color(w, XmNforeground, XmNforeground, BLACK);
   } else {
      set_color(w, XmNbackground, "insensitiveBackground", WHITE);
      set_color(w, XmNforeground, "insensitiveForeground", BLACK);
   }

   XtSetSensitive(w, set);

} // End set_sensitivity

/*---------------------------------------------------------------
 *  Routine to change color of widget for optional feedback
 */

void
set_optional(Widget w, Boolean set)
{
   if ( set ) {
      set_color(w, XmNbackground, "optionalBackground", WHITE);
      set_color(w, XmNforeground, "optionalForeground", BLACK);
   } else {
      set_color(w, XmNbackground, XmNbackground, WHITE);
      set_color(w, XmNforeground, XmNforeground, BLACK);
   }

} // End set_optional

/*---------------------------------------------------------------
 *  Routine to change color of widget for invalid feedback
 */

void
set_invalid(Widget w, Boolean set, Boolean restore)
{
   if ( set ) {
      set_color(w, XmNbackground, "invalidBackground", BLACK);
      set_color(w, XmNforeground, "invalidForeground", WHITE);
      XmProcessTraversal(w, XmTRAVERSE_CURRENT);

      if ( restore )
	 XtAddCallback(w, XmNvalueChangedCallback, restore_text, NULL);

   } else {
      set_color(w, XmNbackground, XmNbackground, WHITE);
      set_color(w, XmNforeground, XmNforeground, BLACK);
   }

} // End set_invalid

/*---------------------------------------------------------------
 *  Routine to change color of MimeRichTextC for invalid feedback
 */

void
set_invalid(MimeRichTextC *w, Boolean set, Boolean restore)
{
   Pixel	bg, fg;
   char		*cl = "MimeRichTextC";
   if ( set ) {
      bg = get_color(cl, w->MainWidget(), "invalidBackground", BLACK);
      fg = get_color(cl, w->MainWidget(), "invalidForeground", WHITE);
   }
   else {
      bg = get_color(cl, w->MainWidget(), "background", WHITE);
      fg = get_color(cl, w->MainWidget(), "foreground", BLACK);
   }

   w->Defer(True);
   w->SetBackground(bg);
   w->SetForeground(fg);
   w->Defer(False);
   XmProcessTraversal(w->TextArea(), XmTRAVERSE_CURRENT);

   if ( set && restore )
      w->AddTextChangeCallback((CallbackFn*)restore_richtext, NULL);

} // End set_invalid

/*---------------------------------------------------------------
 *  Return the string for a custom resource
 */

StringC
get_string(Widget w, const char *rsrc, const char *def)
{
   XtResource	res;
   res.resource_name   = (String)rsrc;
   res.resource_class  = XtCString;
   res.resource_type   = XtRString;
   res.resource_size   = sizeof(String);
   res.resource_offset = 0;
   res.default_type    = XtRString;
   res.default_addr    = (XtPointer)def;

   String	string;
   XtGetApplicationResources(w, &string, &res, 1, NULL, 0);
   return string;

} // End get_string

/*---------------------------------------------------------------
 *  Return the value of a boolean custom resource
 */

Boolean
get_boolean(Widget w, const char *rsrc, Boolean def)
{
   XtResource	res;
   res.resource_name   = (String)rsrc;
   res.resource_class  = XtCBoolean;
   res.resource_type   = XtRBoolean;
   res.resource_size   = sizeof(Boolean);
   res.resource_offset = 0;
   res.default_type    = XtRBoolean;
   res.default_addr    = (XtPointer)&def;

   Boolean	value;
   XtGetApplicationResources(w, &value, &res, 1, NULL, 0);
   return value;

} // End get_boolean

/*---------------------------------------------------------------
 *  Return the value of an integer custom resource
 */

int
get_int(Widget w, const char *rsrc, int def)
{
   XtResource	res;
   res.resource_name   = (String)rsrc;
   res.resource_class  = "Integer";
   res.resource_type   = XtRInt;
   res.resource_size   = sizeof(int);
   res.resource_offset = 0;
   res.default_type    = XtRInt;
   res.default_addr    = (XtPointer)&def;

   int	value;
   XtGetApplicationResources(w, &value, &res, 1, NULL, 0);
   return value;

} // End get_int

/*---------------------------------------------------------------
 *  Return the value of a float custom resource
 */

float
get_float(Widget w, const char *rsrc, float def)
{
   float	local_def = def;

   XtResource	res;
   res.resource_name   = (String)rsrc;
   res.resource_class  = "Float";
   res.resource_type   = XtRFloat;
   res.resource_size   = sizeof(float);
   res.resource_offset = 0;
   res.default_type    = XtRFloat;
   res.default_addr    = (XtPointer)&local_def;

   float	value;
   XtGetApplicationResources(w, &value, &res, 1, NULL, 0);
   return value;

} // End get_float

/*---------------------------------------------------------------
 *  Return the value of a cursor custom resource
 */

Cursor
get_cursor(Widget w, const char *rsrc, const char *def)
{
   XtResource	res;
   res.resource_name   = (String)rsrc;
   res.resource_class  = XtCCursor;
   res.resource_type   = XtRCursor;
   res.resource_size   = sizeof(Cursor);
   res.resource_offset = 0;
   res.default_type    = XtRString;
   res.default_addr    = (XtPointer)def;

   Cursor	value;
   XtGetApplicationResources(w, &value, &res, 1, NULL, 0);
   return value;

} // End get_cursor

/*---------------------------------------------------------------
 *  Return the value of a gravity custom resource
 */

int
get_gravity(Widget w, const char *rsrc, const char *def)
{
   XtResource	res;
   res.resource_name   = (String)rsrc;
   res.resource_class  = "Gravity";
   res.resource_type   = XtRGravity;
   res.resource_size   = sizeof(int);
   res.resource_offset = 0;
   res.default_type    = XtRString;
   res.default_addr    = (XtPointer)def;

   int	value;
   XtGetApplicationResources(w, &value, &res, 1, NULL, 0);
   return value;

} // End get_gravity

/*---------------------------------------------------------------
 *  Return the value of a shadow type custom resource
 */

unsigned char
get_shadow_type(Widget w, const char *rsrc, unsigned char def)
{
   XtResource	res;
   res.resource_name   = (String)rsrc;
   res.resource_class  = XmCShadowType;
   res.resource_type   = XmRShadowType;
   res.resource_size   = sizeof(unsigned char);
   res.resource_offset = 0;
   res.default_type    = XmRShadowType;
   res.default_addr    = (XtPointer)&def;

   unsigned char	value;
   XtGetApplicationResources(w, &value, &res, 1, NULL, 0);
   return value;

} // End get_shadow_type

/*---------------------------------------------------------------
 *  Callback routine to automatically reset a text field marked invalid
 */

void
restore_text(Widget w, XtPointer, XtPointer)
{
   XtRemoveCallback(w, XmNvalueChangedCallback, restore_text, NULL);
   set_invalid(w, False);
}

/*---------------------------------------------------------------
 *  Callback routine to automatically reset a MimeRichTextC field
 *     marked invalid
 */

void
restore_richtext(MimeRichTextC *w, XtPointer)
{
   w->RemoveTextChangeCallback((CallbackFn*)restore_richtext, NULL);
   set_invalid(w, False);
}

/*---------------------------------------------------------------
 *  Database of rgb color names and values
 */

#define BUFSIZE		512
#define MAX_RGB		255
#define MAX_XRGB	65535
#define RGB_SCALE	(MAX_XRGB/MAX_RGB)


/*---------------------------------------------------------------
 *  Function to convert pixel value into color name
 */

StringC
ColorName(Widget w, Pixel pixel)
{
//
// Look up rgb values
//
   XColor       xcolor;
   XrmValue	fromVal;
   XrmValue	toVal;
   fromVal.addr = (XPointer)&pixel;
   fromVal.size = sizeof(Pixel);
   toVal.addr	= (XPointer)&xcolor;
   toVal.size	= sizeof(XColor);

   if ( !XtConvertAndStore(w, XtRPixel, &fromVal, XtRColor, &toVal) )
      return "";

//
// Build definition
//
   char		defn[16];
   sprintf(defn, "#%04x%04x%04x", (int)xcolor.red, (int)xcolor.green,
				  (int)xcolor.blue);
   return defn;

} // End ColorName


/*---------------------------------------------------------------
 *  Function to convert a color name into a pixel value
 */

Boolean
PixelValue(Widget w, const char *cs, Pixel *color)
{
   XrmValue	fromVal;
   XrmValue	toVal;
   XColor xc, dummy;

   XAllocNamedColor(halApp->display,
       DefaultColormap(halApp->display, DefaultScreen(halApp->display)),
       cs, &xc, &dummy);

//
// Convert color
//
   fromVal.addr = (XPointer)cs;
   fromVal.size = strlen(cs) + 1;
   toVal.addr   = (XPointer)color;
   toVal.size   = sizeof(Pixel);
   return XtConvertAndStore(w, XtRString, &fromVal, XtRPixel, &toVal);

} // End PixelValue

/*---------------------------------------------------------------
 *  Return the value of an orientation custom resource
 */

unsigned char
get_orient(Widget w, const char *rsrc, unsigned char def)
{
   XtResource	res;
   res.resource_name   = (String)rsrc;
   res.resource_class  = XtCOrientation;
   res.resource_type   = XtROrientation;
   res.resource_size   = sizeof(unsigned char);
   res.resource_offset = 0;
   res.default_type    = XtROrientation;
   res.default_addr    = (XtPointer)&def;

   unsigned char	value;
   XtGetApplicationResources(w, &value, &res, 1, NULL, 0);
   return value;

} // End get_orient

/*----------------------------------------------------------------------
 * Replace '\' with "\\"
 */

static void
AddEscapes(StringC& str)
{
   int	pos = 0;
   while ( pos < str.size() ) {
      if ( str[pos] == '\\' ) {
	 str(pos,0) = "\\";
	 pos += 2;
      } else if ( str[pos] == '\n' ) {
	 str(pos,0) = "\\n\\";
	 pos += 4;
      } else {
	 pos++;
      }
   }
}

/*----------------------------------------------------------------------
 * Subroutines to write attributes to a file
 */

void
WriteResource(FILE *fp, const char *rsrc, const char *value)
{
   StringC	valStr = value;
   AddEscapes(valStr);
   fprintf(fp, "%s:\t%s\n", rsrc, (char *)valStr);
}

void
WriteResource(FILE *fp, const char *rsrc, float value)
{
   fprintf(fp, "%s:\t%f\n", rsrc, value);
}

void
WriteResource(FILE *fp, const char *rsrc, int value)
{
   fprintf(fp, "%s:\t%d\n", rsrc, value);
}

void
WriteResource(FILE *fp, const char *rsrc, unsigned value)
{
   fprintf(fp, "%s:\t%u\n", rsrc, value);
}

void
WriteResource(FILE *fp, const char *rsrc, Dimension value)
{
   fprintf(fp, "%s:\t%d\n", rsrc, value);
}

void
WriteResource(FILE *fp, const char *rsrc, Boolean value)
{
   fprintf(fp, "%s:\t%s\n", rsrc, value ? "True" : "False");
}

void
WriteResource(FILE *fp, const char *rsrc, const IntListC& list)
{
   unsigned	count = list.size();
   if ( count > 0 ) {
      fprintf(fp, "%s:", rsrc);
      for (int i=0; i<count; i++) fprintf(fp, "\t%d", *list[i]);
      fprintf(fp, "\n");
   }
}
void
WriteResource(FILE *fp, const char *rsrc, const StringListC& list)
{
   unsigned	count = list.size();
   if ( count > 0 ) {
      fprintf(fp, "%s:", rsrc);
      StringC	str;
      for (int i=0; i<count; i++) {
	 str = *list[i];
	 AddEscapes(str);
	 fprintf(fp, "\t%s", (char *)str);
      }
      fprintf(fp, "\n");
   }
}

void
WriteShadowType(FILE *fp, const char *rsrc, unsigned char value)
{
   fprintf(fp, "%s:\t", rsrc);
   char	*str;
   switch (value) {
      case (XmSHADOW_OUT):	  str = "Shadow_Out";		break;
      case (XmSHADOW_ETCHED_IN):  str = "Shadow_Etched_In";	break;
      case (XmSHADOW_ETCHED_OUT): str = "Shadow_Etched_Out";	break;
      default:			  str = "Shadow_In";		break;
   }
   fprintf(fp, "%s\n", str);
}

void
WriteOrientation(FILE *fp, const char *rsrc, unsigned char value)
{
   fprintf(fp, "%s:\t%s\n", rsrc,
			      value==XmVERTICAL ? "Vertical" : "Horizontal");
}

//************************************************************************
// get_* routines accepting a class name
//************************************************************************

/*---------------------------------------------------------------
 *  Return the color for a custom resource
 */

Pixel
get_color(const char *wclass, Widget w, const char *rsrc, Pixel def)
{
   XtResource	res;
   res.resource_name   = (String)rsrc;
   res.resource_class  = XtCColor;
   res.resource_type   = XtRPixel;
   res.resource_size   = sizeof(Pixel);
   res.resource_offset = 0;
   res.default_type    = XtRPixel;
   res.default_addr    = (XtPointer)&def;

   Pixel	color;
   XtGetSubresources(XtParent(w), &color, XtName(w), (String)wclass, &res, 1,
		     NULL, 0);
   return color;

} // End get_color

Pixel
get_color(const char *wclass, Widget w, const char *rsrc, const char *def)
{
   XtResource	res;
   res.resource_name   = (String)rsrc;
   res.resource_class  = XtCColor;
   res.resource_type   = XtRPixel;
   res.resource_size   = sizeof(Pixel);
   res.resource_offset = 0;
   res.default_type    = XtRString;
   res.default_addr    = (XtPointer)def;

   Pixel	color;
   XtGetSubresources(XtParent(w), &color, XtName(w), (String)wclass, &res, 1,
		     NULL, 0);
   return color;

} // End get_color

/*---------------------------------------------------------------
 *  Return the string for a custom resource
 */

StringC
get_string(const char *wclass, Widget w, const char *rsrc, const char *def)
{
   XtResource	res;
   res.resource_name   = (String)rsrc;
   res.resource_class  = XtCString;
   res.resource_type   = XtRString;
   res.resource_size   = sizeof(String);
   res.resource_offset = 0;
   res.default_type    = XtRString;
   res.default_addr    = (XtPointer)def;

   String	string;
   XtGetSubresources(XtParent(w), &string, XtName(w), (String)wclass, &res, 1,
		     NULL, 0);
   return string;

} // End get_string

/*---------------------------------------------------------------
 *  Return the value of a boolean custom resource
 */

Boolean
get_boolean(const char *wclass, Widget w, const char *rsrc, Boolean def)
{
   XtResource	res;
   res.resource_name   = (String)rsrc;
   res.resource_class  = XtCBoolean;
   res.resource_type   = XtRBoolean;
   res.resource_size   = sizeof(Boolean);
   res.resource_offset = 0;
   res.default_type    = XtRBoolean;
   res.default_addr    = (XtPointer)&def;

   Boolean	value;
   XtGetSubresources(XtParent(w), &value, XtName(w), (String)wclass, &res, 1,
		     NULL, 0);
   return value;

} // End get_boolean

/*---------------------------------------------------------------
 *  Return the value of an integer custom resource
 */

int
get_int(const char *wclass, Widget w, const char *rsrc, int def)
{
   XtResource	res;
   res.resource_name   = (String)rsrc;
   res.resource_class  = "Integer";
   res.resource_type   = XtRInt;
   res.resource_size   = sizeof(int);
   res.resource_offset = 0;
   res.default_type    = XtRInt;
   res.default_addr    = (XtPointer)&def;

   int	value;
   XtGetSubresources(XtParent(w), &value, XtName(w), (String)wclass, &res, 1,
		     NULL, 0);
   return value;

} // End get_int

/*---------------------------------------------------------------
 *  Return the value of a float custom resource
 */

float
get_float(const char *wclass, Widget w, const char *rsrc, float def)
{
   float	local_def = def;

   XtResource	res;
   res.resource_name   = (String)rsrc;
   res.resource_class  = "Float";
   res.resource_type   = XtRFloat;
   res.resource_size   = sizeof(float);
   res.resource_offset = 0;
   res.default_type    = XtRFloat;
   res.default_addr    = (XtPointer)&local_def;

   float	value;
   XtGetSubresources(XtParent(w), &value, XtName(w), (String)wclass, &res, 1,
		     NULL, 0);
   return value;

} // End get_float

/*---------------------------------------------------------------
 *  Return the value of a cursor custom resource
 */

Cursor
get_cursor(const char *wclass, Widget w, const char *rsrc, const char *def)
{
   XtResource	res;
   res.resource_name   = (String)rsrc;
   res.resource_class  = XtCCursor;
   res.resource_type   = XtRCursor;
   res.resource_size   = sizeof(Cursor);
   res.resource_offset = 0;
   res.default_type    = XtRString;
   res.default_addr    = (XtPointer)def;

   Cursor	value;
   XtGetSubresources(XtParent(w), &value, XtName(w), (String)wclass, &res, 1,
		     NULL, 0);
   return value;

} // End get_cursor

/*---------------------------------------------------------------
 *  Return the value of a gravity custom resource
 */

int
get_gravity(const char *wclass, Widget w, const char *rsrc, const char *def)
{
   XtResource	res;
   res.resource_name   = (String)rsrc;
   res.resource_class  = "Gravity";
   res.resource_type   = XtRGravity;
   res.resource_size   = sizeof(int);
   res.resource_offset = 0;
   res.default_type    = XtRString;
   res.default_addr    = (XtPointer)def;

   int	value;
   XtGetSubresources(XtParent(w), &value, XtName(w), (String)wclass, &res, 1,
		     NULL, 0);
   return value;

} // End get_gravity

/*---------------------------------------------------------------
 *  Return the value of a shadow type custom resource
 */

unsigned char
get_shadow_type(const char *wclass, Widget w, const char *rsrc,
		unsigned char def)
{
   XtResource	res;
   res.resource_name   = (String)rsrc;
   res.resource_class  = XmCShadowType;
   res.resource_type   = XmRShadowType;
   res.resource_size   = sizeof(unsigned char);
   res.resource_offset = 0;
   res.default_type    = XmRShadowType;
   res.default_addr    = (XtPointer)&def;

   unsigned char	value;
   XtGetSubresources(XtParent(w), &value, XtName(w), (String)wclass, &res, 1,
		     NULL, 0);
   return value;

} // End get_shadow_type

/*---------------------------------------------------------------
 *  Return the value of an orientation custom resource
 */

unsigned char
get_orient(const char *wclass, Widget w, const char *rsrc, unsigned char def)
{
   XtResource	res;
   res.resource_name   = (String)rsrc;
   res.resource_class  = XtCOrientation;
   res.resource_type   = XtROrientation;
   res.resource_size   = sizeof(unsigned char);
   res.resource_offset = 0;
   res.default_type    = XtROrientation;
   res.default_addr    = (XtPointer)&def;

   unsigned char	value;
   XtGetSubresources(XtParent(w), &value, XtName(w), (String)wclass, &res, 1,
		     NULL, 0);
   return value;

} // End get_orient

//************************************************************************
// get_* routines accepting a widget name and parent
//************************************************************************

/*---------------------------------------------------------------
 *  Return the color for a custom resource
 */

Pixel
get_color(const char *wclass, const char *wname, const char *rsrc, Widget par,
	  Pixel def)
{
   XtResource	res;
   res.resource_name   = (String)rsrc;
   res.resource_class  = XtCColor;
   res.resource_type   = XtRPixel;
   res.resource_size   = sizeof(Pixel);
   res.resource_offset = 0;
   res.default_type    = XtRPixel;
   res.default_addr    = (XtPointer)&def;

   Pixel	color;
   XtGetSubresources(par, &color, (String)wname, (String)wclass, &res, 1,
		     NULL, 0);
   return color;

} // End get_color

Pixel
get_color(const char *wclass, const char *wname, const char *rsrc, Widget par,
	  const char *def)
{
   XtResource	res;
   res.resource_name   = (String)rsrc;
   res.resource_class  = XtCColor;
   res.resource_type   = XtRPixel;
   res.resource_size   = sizeof(Pixel);
   res.resource_offset = 0;
   res.default_type    = XtRString;
   res.default_addr    = (XtPointer)def;

   Pixel	color;
   XtGetSubresources(par, &color, (String)wname, (String)wclass, &res, 1,
   		     NULL, 0);
   return color;

} // End get_color

/*---------------------------------------------------------------
 *  Return the string for a custom resource
 */

StringC
get_string(const char *wclass, const char *wname, const char *rsrc, Widget par,
	   const char *def)
{
   XtResource	res;
   res.resource_name   = (String)rsrc;
   res.resource_class  = XtCString;
   res.resource_type   = XtRString;
   res.resource_size   = sizeof(String);
   res.resource_offset = 0;
   res.default_type    = XtRString;
   res.default_addr    = (XtPointer)def;

   String	string;
   XtGetSubresources(par, &string, (String)wname, (String)wclass, &res, 1,
		     NULL, 0);
   return string;

} // End get_string

/*---------------------------------------------------------------
 *  Return the value of a boolean custom resource
 */

Boolean
get_boolean(const char *wclass, const char *wname, const char *rsrc, Widget par,
	    Boolean def)
{
   XtResource	res;
   res.resource_name   = (String)rsrc;
   res.resource_class  = XtCBoolean;
   res.resource_type   = XtRBoolean;
   res.resource_size   = sizeof(Boolean);
   res.resource_offset = 0;
   res.default_type    = XtRBoolean;
   res.default_addr    = (XtPointer)&def;

   Boolean	value;
   XtGetSubresources(par, &value, (String)wname, (String)wclass, &res, 1,
		     NULL, 0);
   return value;

} // End get_boolean

/*---------------------------------------------------------------
 *  Return the value of an integer custom resource
 */

int
get_int(const char *wclass, const char *wname, const char *rsrc, Widget par,
	int def)
{
   XtResource	res;
   res.resource_name   = (String)rsrc;
   res.resource_class  = "Integer";
   res.resource_type   = XtRInt;
   res.resource_size   = sizeof(int);
   res.resource_offset = 0;
   res.default_type    = XtRInt;
   res.default_addr    = (XtPointer)&def;

   int	value;
   XtGetSubresources(par, &value, (String)wname, (String)wclass, &res, 1,
		     NULL, 0);
   return value;

} // End get_int

/*---------------------------------------------------------------
 *  Return the value of a float custom resource
 */

float
get_float(const char *wclass, const char *wname, const char *rsrc, Widget par,
	  float def)
{
   float	local_def = def;

   XtResource	res;
   res.resource_name   = (String)rsrc;
   res.resource_class  = "Float";
   res.resource_type   = XtRFloat;
   res.resource_size   = sizeof(float);
   res.resource_offset = 0;
   res.default_type    = XtRFloat;
   res.default_addr    = (XtPointer)&local_def;

   float	value;
   XtGetSubresources(par, &value, (String)wname, (String)wclass, &res, 1,
		     NULL, 0);
   return value;

} // End get_float

/*---------------------------------------------------------------
 *  Return the value of a cursor custom resource
 */

Cursor
get_cursor(const char *wclass, const char *wname, const char *rsrc, Widget par,
	   const char *def)
{
   XtResource	res;
   res.resource_name   = (String)rsrc;
   res.resource_class  = XtCCursor;
   res.resource_type   = XtRCursor;
   res.resource_size   = sizeof(Cursor);
   res.resource_offset = 0;
   res.default_type    = XtRString;
   res.default_addr    = (XtPointer)def;

   Cursor	value;
   XtGetSubresources(par, &value, (String)wname, (String)wclass, &res, 1,
		     NULL, 0);
   return value;

} // End get_cursor

/*---------------------------------------------------------------
 *  Return the value of a gravity custom resource
 */

int
get_gravity(const char *wclass, const char *wname, const char *rsrc, Widget par,
	    const char *def)
{
   XtResource	res;
   res.resource_name   = (String)rsrc;
   res.resource_class  = "Gravity";
   res.resource_type   = XtRGravity;
   res.resource_size   = sizeof(int);
   res.resource_offset = 0;
   res.default_type    = XtRString;
   res.default_addr    = (XtPointer)def;

   int	value;
   XtGetSubresources(par, &value, (String)wname, (String)wclass, &res, 1,
		     NULL, 0);
   return value;

} // End get_gravity

/*---------------------------------------------------------------
 *  Return the value of a shadow type custom resource
 */

unsigned char
get_shadow_type(const char *wclass, const char *wname, const char *rsrc,
		Widget par, unsigned char def)
{
   XtResource	res;
   res.resource_name   = (String)rsrc;
   res.resource_class  = XmCShadowType;
   res.resource_type   = XmRShadowType;
   res.resource_size   = sizeof(unsigned char);
   res.resource_offset = 0;
   res.default_type    = XmRShadowType;
   res.default_addr    = (XtPointer)&def;

   unsigned char	value;
   XtGetSubresources(par, &value, (String)wname, (String)wclass, &res, 1,
		     NULL, 0);
   return value;

} // End get_shadow_type

/*---------------------------------------------------------------
 *  Return the value of an orientation custom resource
 */

unsigned char
get_orient(const char *wclass, const char *wname, const char *rsrc, Widget par,
	   unsigned char def)
{
   XtResource	res;
   res.resource_name   = (String)rsrc;
   res.resource_class  = XtCOrientation;
   res.resource_type   = XtROrientation;
   res.resource_size   = sizeof(unsigned char);
   res.resource_offset = 0;
   res.default_type    = XtROrientation;
   res.default_addr    = (XtPointer)&def;

   unsigned char	value;
   XtGetSubresources(par, &value, (String)wname, (String)wclass, &res, 1,
		     NULL, 0);
   return value;

} // End get_orient
