/*
 * $Id: FontDataC.C,v 1.2 2000/05/07 12:26:10 fnevgeny Exp $
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

#include "FontDataC.h"
#include "StrCase.h"
#include "HalAppC.h"
#include "CharC.h"

extern int	debug1, debug2;

/*----------------------------------------------------------------------
 * Constructors
 */

FontDataC::FontDataC()
{
   Init();
}

FontDataC::FontDataC(const char *csname)
{
   Init();
   *this = csname;
}

/*----------------------------------------------------------------------
 * Initialization method
 */

void
FontDataC::Init()
{
   charSetNames.AllowDuplicates(FALSE);
   charSetNames.SetSorted(FALSE);
   charSetFonts.AllowDuplicates(TRUE);
   charSetFonts.SetSorted(FALSE);
   freeCharSet.AllowDuplicates(TRUE);
   freeCharSet.SetSorted(FALSE);

   pixels        = 0;
   points        = 0;
   normSpace     = 0;
   endSpace      = 0;
   superX        = 0;
   superY        = 0;
   subX          = 0;
   subY          = 0;
   underY        = 0;
   underThick    = 0;
   loaded        = False;
   xfont         = NULL;
   fid           = halApp->fid;
   bigger        = NULL;
   smaller       = NULL;
   bold          = NULL;
   nonBold       = NULL;
   italic        = NULL;
   nonItalic     = NULL;
   freeBigger    = False;
   freeSmaller   = False;
   freeBold      = False;
   freeItalic    = False;
   freeNonBold   = False;
   freeNonItalic = False;

} // End Init

/*----------------------------------------------------------------------
 * Reset method
 */

void
FontDataC::Reset(Boolean init)
{
   if ( loaded && halApp->xRunning ) XFreeFont(halApp->display, xfont);
   if ( freeBigger     && bigger    != this ) delete bigger;
   if ( freeSmaller    && smaller   != this ) delete smaller;
   if ( freeBold       && bold      != this ) delete bold;
   if ( freeItalic     && italic    != this ) delete italic;
   if ( freeNonBold    && nonBold   != this ) delete nonBold;
   if ( freeNonItalic  && nonItalic != this ) delete nonItalic;

   unsigned	count = charSetFonts.size();
   for (int i=0; i<count; i++) {
      if ( *freeCharSet[i] ) {
	 FontDataC	*data = (FontDataC*)*charSetFonts[i];
	 delete data;
      }
   }

   charSetFonts.removeAll();
   charSetNames.removeAll();
   freeCharSet.removeAll();

   if ( init ) Init();

} // End Reset

/*----------------------------------------------------------------------
 * Destructor
 */

FontDataC::~FontDataC()
{
   Reset(False/*Don't init*/);
}

/*----------------------------------------------------------------------
 * Assignment operator
 */

FontDataC&
FontDataC::operator=(FontDataC& that)
{
   Reset();

   name          = that.name;
   foundry       = that.foundry;
   family        = that.family;
   weight        = that.weight;
   slant         = that.slant;
   setWidth      = that.setWidth;
   addStyle      = that.addStyle;
   pixelSize     = that.pixelSize;
   pointSize     = that.pointSize;
   resStr        = that.resStr;
   spacing       = that.spacing;
   avgWidth      = that.avgWidth;
   charset       = that.charset;
   pixels        = that.pixels;
   points        = that.points;
   normSpace     = that.normSpace;
   endSpace      = that.endSpace;
   superX        = that.superX;
   superY        = that.superY;
   subX          = that.subX;
   subY          = that.subY;
   underY        = that.underY;
   underThick    = that.underThick;
   xfont         = that.xfont;
   fid           = that.fid;
   bigger        = that.bigger;
   smaller       = that.smaller;
   bold          = that.bold;
   italic        = that.italic;
   nonBold       = that.nonBold;
   nonItalic     = that.nonItalic;
   loaded        = False;
   freeBigger    = False;
   freeSmaller   = False;
   freeBold      = False;
   freeItalic    = False;
   freeNonBold   = False;
   freeNonItalic = False;

//
// Copy alternate character sets
//
   u_int	count = that.charSetFonts.size();
   int	no = FALSE;
   for (int i=0; i<count; i++) {
      charSetFonts.add(*that.charSetFonts[i]);
      charSetNames.add(*that.charSetNames[i]);
      freeCharSet.add(no);
   }

   return *this;

} // End assignment from FontDataC

/*----------------------------------------------------------------------
 * Assignment operator
 */

FontDataC&
FontDataC::operator=(const char *csname)
{
   Reset();

   name = csname;
   name.toLower();
   DecodeName();

   if ( name.size() == 0 ||
	(xfont = XLoadQueryFont(halApp->display, name)) == NULL ) {
      xfont  = halApp->font;
      fid    = halApp->fid;
      loaded = False;
   } else {
      loaded = True;
      fid    = xfont->fid;
   }
   GetProps();

   return *this;

} // End assignment from char*

/*----------------------------------------------------------------------
 * Method to expand a font name and extract the XFLD components
 */

void
FontDataC::DecodeName()
{
   if ( name.size() == 0 ) return;

//
// Call XListFonts to expand the name to its full specification.
//
   int	actual;
   char	**names = XListFonts(halApp->display, name, 1, &actual);

   if ( actual > 0 ) name = names[0];
   else	{
      StringC	msg = "I could not find a font matching the name:\n\n";
      msg += name + "\n\n";
      msg += "You may wish to look in your resource files and change any\n";
      msg += "references to this font.";
      halApp->PopupMessage(msg);
      name.Clear();
   }

   if ( names ) XFreeFontNames(names);

//
// Extract the XFLD components if the name matches the pattern.
//
   int	dashCount = name.NumberOf('-');
   if ( dashCount >= 13 ) { 

      u_int	start = 0;
      if ( name.StartsWith('-') ) start++;

      int	which = 1;
      int	pos = name.PosOf('-', start);
      int	resStart;
      CharC	range;
      while ( which <= 13 && pos >= start ) {

	 range = name.Range(start, pos-start);
	 switch(which) {

	    case( 1): foundry	= range; break;
	    case( 2): family	= range; break;
	    case( 3): weight	= range; break;
	    case( 4): slant	= range; break;
	    case( 5): setWidth	= range; break;
	    case( 6): addStyle	= range; break;
	    case( 7): pixelSize	= range; break;
	    case( 8): pointSize	= range; break;
	    case( 9): resStart	= start; break;
	    case(10): resStr	= name(resStart, pos-resStart); break;
	    case(11): spacing	= range; break;
	    case(12): avgWidth	= range; break;
	    // Charset goes to end
	    case(13): charset	= name.Range(start, name.size()); break;
	 }

	 which++;
	 start = pos+1;
	 pos   = name.PosOf('-', start);

      } // End for each part of font name

      StringC	tmp = pixelSize;
      pixels = atoi(tmp);

      tmp = pointSize;
      points = atoi(tmp);

   } // End if pattern matches

} // End DecodeName

/*----------------------------------------------------------------------
 * Method to get the properties for the specified font
 */

void
FontDataC::GetProps()
{
   unsigned long	propVal;
   if ( XGetFontProperty(xfont, XA_NORM_SPACE, &propVal) ) {
      normSpace = (unsigned)propVal;
   } else {
      normSpace = xfont->max_bounds.width;
   }

   if ( XGetFontProperty(xfont, XA_END_SPACE, &propVal) ) {
      endSpace = (unsigned)propVal;
   } else {
      endSpace = 2 * xfont->max_bounds.width;
   }

   if ( XGetFontProperty(xfont, XA_SUPERSCRIPT_X, &propVal) ) {
      superX = (int)propVal;
   } else {
      superX = 0;
   }

   if ( XGetFontProperty(xfont, XA_SUPERSCRIPT_Y, &propVal) ) {
      superY = (int)propVal;
   } else {
      superY = (xfont->ascent + xfont->descent)/(int)2;
   }

   if ( XGetFontProperty(xfont, XA_SUBSCRIPT_X, &propVal) ) {
      subX = (int)propVal;
   } else {
      subX = 0;
   }

   if ( XGetFontProperty(xfont, XA_SUBSCRIPT_Y, &propVal) ) {
      subY = (int)propVal;
   } else {
      subY = (xfont->ascent + xfont->descent)/(int)2;
   }

   if ( XGetFontProperty(xfont, XA_UNDERLINE_POSITION, &propVal) ) {
      underY = (int)propVal;
   } else {
      underY = -1;
   }

   if ( XGetFontProperty(xfont, XA_UNDERLINE_THICKNESS, &propVal) ) {
      underThick = (unsigned)propVal;
   } else {
      underThick = 1;
   }

} // End GetFontProps

/*----------------------------------------------------------------------
 * Method to look build a font name pattern from the given parts.
 */

#if 0
void
FontDataC::BuildName(CharC foundryPart, CharC familyPart, CharC weightPart,
		     CharC slantPart, CharC setWidthPart, CharC addStylePart,
		     CharC pixelSizePart, CharC pointSizePart, CharC resPart,
		     CharC spacingPart, CharC avgWidthPart, CharC charsetPart,
		     StringC& fname)
{
   fname = '-';
   fname += foundryPart;
   fname += '-';
   fname += familyPart;
   fname += '-';
   fname += weightPart;
   fname += '-';
   fname += slantPart;
   fname += '-';
   fname += setWidthPart;
   fname += '-';
   fname += addStylePart;
   fname += '-';
   fname += pixelSizePart;
   fname += '-';
   fname += pointSizePart;
   fname += '-';
   fname += resPart;
   fname += '-';
   fname += spacingPart;
   fname += '-';
   fname += avgWidthPart;
   fname += '-';
   fname += charsetPart;

} // End BuildName
#endif

#define BuildName(foundryPart, familyPart, weightPart, slantPart, setWidthPart, addStylePart, pixelSizePart, pointSizePart, resPart, spacingPart, avgWidthPart, charsetPart, fname) \
{ \
   fname = '-'; \
   fname += foundryPart; \
   fname += '-'; \
   fname += familyPart; \
   fname += '-'; \
   fname += weightPart; \
   fname += '-'; \
   fname += slantPart; \
   fname += '-'; \
   fname += setWidthPart; \
   fname += '-'; \
   fname += addStylePart; \
   fname += '-'; \
   fname += pixelSizePart; \
   fname += '-'; \
   fname += pointSizePart; \
   fname += '-'; \
   fname += resPart; \
   fname += '-'; \
   fname += spacingPart; \
   fname += '-'; \
   fname += avgWidthPart; \
   fname += '-'; \
   fname += charsetPart; \
 \
} // End BuildName

/*----------------------------------------------------------------------
 * Method to return a font larger than the current font
 */

FontDataC*
FontDataC::Bigger()
{
   if ( !loaded && !bigger ) bigger = this;

//
// See if we already know this
//
   if ( bigger ) return bigger;

   if ( debug1 ) cout <<"Looking for a bigger font for: " <<name NL;

//
// Try different patterns, getting less specific as failures mount
//
   StringC	bname;
   BuildName(foundry, family, weight, slant, setWidth, addStyle,
	     "*", "*", resStr, spacing, "*", charset, bname);
   Boolean	found = FindBigger(bname);
   if ( !found ) { // Try without res
      BuildName(foundry, family, weight, slant, setWidth, addStyle,
		"*", "*", "*", spacing, "*", charset, bname);
      found = FindBigger(bname);
   }
   if ( !found ) { // Try without width
      BuildName(foundry, family, weight, slant, "*", "*",
		"*", "*", resStr, spacing, "*", charset, bname);
      found = FindBigger(bname);
      if ( !found ) { // Try without res
	 BuildName(foundry, family, weight, slant, "*", "*",
		   "*", "*", "*", spacing, "*", charset, bname);
	 found = FindBigger(bname);
      }
   }
   if ( !found ) { // Try without foundry
      BuildName("*", family, weight, slant, "*", "*",
		"*", "*", resStr, spacing, "*", charset, bname);
      found = FindBigger(bname);
      if ( !found ) { // Try without res
	 BuildName("*", family, weight, slant, "*", "*",
		   "*", "*", "*", spacing, "*", charset, bname);
	 found = FindBigger(bname);
      }
   }
   if ( !found ) { // Try without family
      BuildName("*", "*", weight, slant, "*", "*",
		"*", "*", resStr, spacing, "*", charset, bname);
      found = FindBigger(bname);
      if ( !found ) { // Try without res
	 BuildName("*", "*", weight, slant, "*", "*",
		   "*", "*", "*", spacing, "*", charset, bname);
	 found = FindBigger(bname);
      }
   }
   if ( !found ) { // Try without spacing
      BuildName("*", "*", weight, slant, "*", "*",
		"*", "*", resStr, "*", "*", charset, bname);
      found = FindBigger(bname);
      if ( !found ) { // Try without res
	 BuildName("*", "*", weight, slant, "*", "*",
		   "*", "*", "*", "*", "*", charset, bname);
	 found = FindBigger(bname);
      }
   }
   if ( !found ) { // Try without weight
      BuildName(foundry, family, "*", slant, "*", "*",
		"*", "*", resStr, "*", "*", charset, bname);
      found = FindBigger(bname);
      if ( !found ) { // Try without res
	 BuildName(foundry, family, "*", slant, "*", "*",
		   "*", "*", "*", "*", "*", charset, bname);
	 found = FindBigger(bname);
      }
   }
   if ( !found ) { // Try without slant
      BuildName(foundry, family, weight, "*", "*", "*",
		"*", "*", resStr, "*", "*", charset, bname);
      found = FindBigger(bname);
      if ( !found ) { // Try without res
	 BuildName(foundry, family, weight, "*", "*", "*",
		   "*", "*", "*", "*", "*", charset, bname);
	 found = FindBigger(bname);
      }
   }
   if ( !found ) { // Try without foundry or weight
      BuildName("*", family, "*", slant, "*", "*",
		"*", "*", resStr, "*", "*", charset, bname);
      found = FindBigger(bname);
      if ( !found ) { // Try without res
	 BuildName("*", family, "*", slant, "*", "*",
		   "*", "*", "*", "*", "*", charset, bname);
	 found = FindBigger(bname);
      }
   }
   if ( !found ) { // Try without foundry or slant
      BuildName("*", family, weight, "*", "*", "*",
		"*", "*", resStr, "*", "*", charset, bname);
      found = FindBigger(bname);
      if ( !found ) { // Try without res
	 BuildName("*", family, weight, "*", "*", "*",
		   "*", "*", "*", "*", "*", charset, bname);
	 found = FindBigger(bname);
      }
   }
   if ( !found ) { // Try without foundry, family or weight
      BuildName("*", "*", "*", slant, "*", "*",
		"*", "*", resStr, "*", "*", charset, bname);
      found = FindBigger(bname);
      if ( !found ) { // Try without res
	 BuildName("*", "*", "*", slant, "*", "*",
		   "*", "*", "*", "*", "*", charset, bname);
	 found = FindBigger(bname);
      }
   }
   if ( !found ) { // Try without foundry, family or slant
      BuildName("*", "*", weight, "*", "*", "*",
		"*", "*", resStr, "*", "*", charset, bname);
      found = FindBigger(bname);
      if ( !found ) { // Try without res
	 BuildName("*", "*", weight, "*", "*", "*",
		   "*", "*", "*", "*", "*", charset, bname);
	 found = FindBigger(bname);
      }
   }
   if ( !found ) { // Try without weight or slant
      BuildName(foundry, family, "*", "*", "*", "*",
		"*", "*", resStr, "*", "*", charset, bname);
      found = FindBigger(bname);
      if ( !found ) { // Try without res
	 BuildName(foundry, family, "*", "*", "*", "*",
		   "*", "*", "*", "*", "*", charset, bname);
	 found = FindBigger(bname);
      }
   }
   if ( !found ) { // Try without foundry, weight or slant
      BuildName("*", family, "*", "*", "*", "*",
		"*", "*", resStr, "*", "*", charset, bname);
      found = FindBigger(bname);
      if ( !found ) { // Try without res
	 BuildName("*", family, "*", "*", "*", "*",
		   "*", "*", "*", "*", "*", charset, bname);
	 found = FindBigger(bname);
      }
   }
   if ( !found ) { // Try without foundry, family, weight or slant
      BuildName("*", "*", "*", "*", "*", "*",
		"*", "*", resStr, "*", "*", charset, bname);
      found = FindBigger(bname);
      if ( !found ) { // Try without res
	 BuildName("*", "*", "*", "*", "*", "*",
		   "*", "*", "*", "*", "*", charset, bname);
	 found = FindBigger(bname);
      }
   }

//
// If nothing worked, return this font
//
   if ( !found ) {

#if 0
      StringC errmsg("I could not find a bigger version of the font: ");
      errmsg += name;
      halApp->PopupMessage(errmsg);
#else
      if ( debug1 )
	 cout <<"I could not find a bigger version of the font: " <<name <<endl;
#endif

      if ( debug1 ) cout <<"Using: " <<name NL;
      bigger = this;
      return bigger;
   }

//
// Create a new font structure
//
   if ( debug1 ) cout <<"Creating bigger font: " <<bname NL;
   bigger = new FontDataC(bname);
   if ( !bigger->loaded ) {
      delete bigger;
      bigger = this;
   }

   freeBigger = True;
   return bigger;

} // End Bigger

/*----------------------------------------------------------------------
 * Method to look through a list of font names for a font bigger than
 *    the current one.
 */

Boolean
FontDataC::FindBigger(StringC& fname)
{
   if ( debug2 ) cout <<"Trying bigger: " <<fname NL;

   int	actual;
   char	**names = XListFonts(halApp->display, fname, 100, &actual);
   if ( !names ) return False;

//
// See if there's a bigger point size in the list
//
   Boolean	found = False;
   int		i = 0;
   CharC	name;
   StringC	sizeStr;
   while ( !found && i<actual ) {

//
// Extract the point size information.  It is found after the 8th dash.
//
      name = names[i];
      int	pos       = name.PosOf('-');
      int	dashCount = (pos>=0) ? 1 : 0;
      while ( pos >= 0 && dashCount < 8 ) {
	 pos = name.PosOf('-', (u_int)pos+1);
	 if ( pos >= 0 ) dashCount++;
      }

      if ( pos >= 0 && dashCount == 8 ) {
	 sizeStr = name.NextWord((u_int)pos+1, '-');
	 int	size = atoi(sizeStr);
	 found = (size > this->points);
      }

      if ( found ) fname = name;
      else	   i++;

   } // End while a bigger font has not been found

//
// If no font was found, see if there's a bigger pixel size in the list
//
   i = 0;
   while ( !found && i<actual ) {

//
// Extract the pixel size information.  It is found after the 7th dash.
//
      name = names[i];
      int	pos       = name.PosOf('-');
      int	dashCount = (pos>=0) ? 1 : 0;
      while ( pos >= 0 && dashCount < 7 ) {
	 pos = name.PosOf('-', (u_int)pos+1);
	 if ( pos >= 0 ) dashCount++;
      }

      if ( pos >= 0 && dashCount == 7 ) {
	 sizeStr = name.NextWord((u_int)pos+1, '-');
	 int	size = atoi(sizeStr);
	 found = (size > this->pixels);
      }

      if ( found ) fname = name;
      else	   i++;

   } // End while a bigger font has not been found

   XFreeFontNames(names);

   return found;

} // End FindBigger

/*----------------------------------------------------------------------
 * Method to return a font smaller than the current font
 */

FontDataC*
FontDataC::Smaller()
{
   if ( !loaded && !smaller ) smaller = this;

//
// See if we already know this
//
   if ( smaller ) return smaller;

   if ( debug1 ) cout <<"Looking for a smaller font for: " <<name NL;

//
// Try different patterns, getting less specific as failures mount
//
   StringC	bname;
   BuildName(foundry, family, weight, slant, setWidth, addStyle,
	     "*", "*", resStr, spacing, "*", charset, bname);
   Boolean	found = FindSmaller(bname);
   if ( !found ) { // Try without res
      BuildName(foundry, family, weight, slant, setWidth, addStyle,
		"*", "*", "*", spacing, "*", charset, bname);
      found = FindSmaller(bname);
   }
   if ( !found ) { // Try without width
      BuildName(foundry, family, weight, slant, "*", "*",
		"*", "*", resStr, spacing, "*", charset, bname);
      found = FindSmaller(bname);
      if ( !found ) { // Try without res
	 BuildName(foundry, family, weight, slant, "*", "*",
		   "*", "*", "*", spacing, "*", charset, bname);
	 found = FindSmaller(bname);
      }
   }
   if ( !found ) { // Try without foundry
      BuildName("*", family, weight, slant, "*", "*",
		"*", "*", resStr, spacing, "*", charset, bname);
      found = FindSmaller(bname);
      if ( !found ) { // Try without res
	 BuildName("*", family, weight, slant, "*", "*",
		   "*", "*", "*", spacing, "*", charset, bname);
	 found = FindSmaller(bname);
      }
   }
   if ( !found ) { // Try without family
      BuildName("*", "*", weight, slant, "*", "*",
		"*", "*", resStr, spacing, "*", charset, bname);
      found = FindSmaller(bname);
      if ( !found ) { // Try without res
	 BuildName("*", "*", weight, slant, "*", "*",
		   "*", "*", "*", spacing, "*", charset, bname);
	 found = FindSmaller(bname);
      }
   }
   if ( !found ) { // Try without spacing
      BuildName("*", "*", weight, slant, "*", "*",
		"*", "*", resStr, "*", "*", charset, bname);
      found = FindSmaller(bname);
      if ( !found ) { // Try without res
	 BuildName("*", "*", weight, slant, "*", "*",
		   "*", "*", "*", "*", "*", charset, bname);
	 found = FindSmaller(bname);
      }
   }
   if ( !found ) { // Try without weight
      BuildName(foundry, family, "*", slant, "*", "*",
		"*", "*", resStr, "*", "*", charset, bname);
      found = FindSmaller(bname);
      if ( !found ) { // Try without res
	 BuildName(foundry, family, "*", slant, "*", "*",
		   "*", "*", "*", "*", "*", charset, bname);
	 found = FindSmaller(bname);
      }
   }
   if ( !found ) { // Try without slant
      BuildName(foundry, family, weight, "*", "*", "*",
		"*", "*", resStr, "*", "*", charset, bname);
      found = FindSmaller(bname);
      if ( !found ) { // Try without res
	 BuildName(foundry, family, weight, "*", "*", "*",
		   "*", "*", "*", "*", "*", charset, bname);
	 found = FindSmaller(bname);
      }
   }
   if ( !found ) { // Try without foundry or weight
      BuildName("*", family, "*", slant, "*", "*",
		"*", "*", resStr, "*", "*", charset, bname);
      found = FindSmaller(bname);
      if ( !found ) { // Try without res
	 BuildName("*", family, "*", slant, "*", "*",
		   "*", "*", "*", "*", "*", charset, bname);
	 found = FindSmaller(bname);
      }
   }
   if ( !found ) { // Try without foundry or slant
      BuildName("*", family, weight, "*", "*", "*",
		"*", "*", resStr, "*", "*", charset, bname);
      found = FindSmaller(bname);
      if ( !found ) { // Try without res
	 BuildName("*", family, weight, "*", "*", "*",
		   "*", "*", "*", "*", "*", charset, bname);
	 found = FindSmaller(bname);
      }
   }
   if ( !found ) { // Try without foundry, family or weight
      BuildName("*", "*", "*", slant, "*", "*",
		"*", "*", resStr, "*", "*", charset, bname);
      found = FindSmaller(bname);
      if ( !found ) { // Try without res
	 BuildName("*", "*", "*", slant, "*", "*",
		   "*", "*", "*", "*", "*", charset, bname);
	 found = FindSmaller(bname);
      }
   }
   if ( !found ) { // Try without foundry, family or slant
      BuildName("*", "*", weight, "*", "*", "*",
		"*", "*", resStr, "*", "*", charset, bname);
      found = FindSmaller(bname);
      if ( !found ) { // Try without res
	 BuildName("*", "*", weight, "*", "*", "*",
		   "*", "*", "*", "*", "*", charset, bname);
	 found = FindSmaller(bname);
      }
   }
   if ( !found ) { // Try without weight or slant
      BuildName(foundry, family, "*", "*", "*", "*",
		"*", "*", resStr, "*", "*", charset, bname);
      found = FindSmaller(bname);
      if ( !found ) { // Try without res
	 BuildName(foundry, family, "*", "*", "*", "*",
		   "*", "*", "*", "*", "*", charset, bname);
	 found = FindSmaller(bname);
      }
   }
   if ( !found ) { // Try without foundry, weight or slant
      BuildName("*", family, "*", "*", "*", "*",
		"*", "*", resStr, "*", "*", charset, bname);
      found = FindSmaller(bname);
      if ( !found ) { // Try without res
	 BuildName("*", family, "*", "*", "*", "*",
		   "*", "*", "*", "*", "*", charset, bname);
	 found = FindSmaller(bname);
      }
   }
   if ( !found ) { // Try without foundry, family, weight or slant
      BuildName("*", "*", "*", "*", "*", "*",
		"*", "*", resStr, "*", "*", charset, bname);
      found = FindSmaller(bname);
      if ( !found ) { // Try without res
	 BuildName("*", "*", "*", "*", "*", "*",
		   "*", "*", "*", "*", "*", charset, bname);
	 found = FindSmaller(bname);
      }
   }

//
// If nothing worked, return this font
//
   if ( !found ) {

#if 0
      StringC errmsg("I could not find a smaller version of the font: ");
      errmsg += name;
      halApp->PopupMessage(errmsg);
#else
      if ( debug1 )
	 cout <<"I could not find a smaller version of the font: " <<name<<endl;
#endif

      if ( debug1 ) cout <<"Using: " <<name NL;
      smaller = this;
      return smaller;
   }

//
// Create a new font structure
//
   if ( debug1 ) cout <<"Creating smaller font: " <<bname NL;
   smaller = new FontDataC(bname);
   if ( !smaller->loaded ) {
      delete smaller;
      smaller = this;
   }

   freeSmaller = True;
   return smaller;

} // End Smaller

/*----------------------------------------------------------------------
 * Method to look through a list of font names for a font matching the
 *    given criteria that is smaller than the current one.
 */

Boolean
FontDataC::FindSmaller(StringC& fname)
{
   if ( debug2 ) cout <<"Trying smaller: " <<fname NL;

   int	actual;
   char	**names = XListFonts(halApp->display, fname, 100, &actual);
   if ( !names ) return False;

//
// See if there's a smaller point size in the list
//
   Boolean	found = False;
   int		i = 0;
   CharC	name;
   StringC	sizeStr;
   while ( !found && i<actual ) {

//
// Extract the point size information.  It is found after the 8th dash.
//
      name = names[i];
      int	pos       = name.PosOf('-');
      int	dashCount = (pos>=0) ? 1 : 0;
      while ( pos >= 0 && dashCount < 8 ) {
	 pos = name.PosOf('-', (u_int)pos+1);
	 if ( pos >= 0 ) dashCount++;
      }

      if ( pos >= 0 && dashCount == 8 ) {
	 sizeStr = name.NextWord((u_int)pos+1, '-');
	 int	size = atoi(sizeStr);
	 found = (size < this->points);
      }

      if ( found ) fname = name;
      else	   i++;

   } // End while a smaller font has not been found

//
// If no font was found, see if there's a smaller pixel size in the list
//
   i = 0;
   while ( !found && i<actual ) {

//
// Extract the pixel size information.  It is found after the 7th dash.
//
      name = names[i];
      int	pos       = name.PosOf('-');
      int	dashCount = (pos>=0) ? 1 : 0;
      while ( pos >= 0 && dashCount < 7 ) {
	 pos = name.PosOf('-', (u_int)pos+1);
	 if ( pos >= 0 ) dashCount++;
      }

      if ( pos >= 0 && dashCount == 7 ) {
	 sizeStr = name.NextWord((u_int)pos+1, '-');
	 int	size = atoi(sizeStr);
	 found = (size < this->pixels);
      }

      if ( found ) fname = name;
      else	   i++;

   } // End while a smaller font has not been found

   XFreeFontNames(names);

   return found;

} // End FindSmaller

/*----------------------------------------------------------------------
 * Method to return a bold version of the current font
 */

FontDataC*
FontDataC::Bold()
{
   if ( !loaded && !bold ) bold = this;

//
// See if we already know this
//
   if ( bold ) return bold;

//
// See if this is already a bold font
//
   if ( weight.Equals("bold", IGNORE_CASE) ) {
      bold = this;
      return bold;
   }

   if ( debug1 ) cout <<"Looking for a bold font for: " <<name NL;

//
// See if there is a bold version of this font.
//
   StringC	bname;
   BuildName(foundry, family, "bold", slant, setWidth, addStyle, pixelSize,
	     pointSize, resStr, spacing, avgWidth, charset, bname);
   if ( FindFont(bname) ) return CreateBold(bname);

//
// Try a quick "bold" on the end of the name
//
   bname = name + "*bold";
   if ( FindFont(bname) ) return CreateBold(bname);

// Any width or add style
   BuildName(foundry, family, "bold", slant, "*", "*", pixelSize,
   	     pointSize, resStr, spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateBold(bname);

// Any res
   BuildName(foundry, family, "bold", slant, "*", "*", pixelSize,
   	     pointSize, "*", spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateBold(bname);

// Point size only
   BuildName(foundry, family, "bold", slant, "*", "*", "*",
   	     pointSize, resStr, spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateBold(bname);

// Any res
   BuildName(foundry, family, "bold", slant, "*", "*", "*",
   	     pointSize, "*", spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateBold(bname);

// Pixel size only
   BuildName(foundry, family, "bold", slant, "*", "*", pixelSize,
	     "*", resStr, spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateBold(bname);

// Any res
   BuildName(foundry, family, "bold", slant, "*", "*", pixelSize,
	     "*", "*", spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateBold(bname);

// Any foundry or family
   BuildName("*", "*", "bold", slant, setWidth, addStyle, pixelSize,
	     pointSize, resStr, spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateBold(bname);

// Any res
   BuildName("*", "*", "bold", slant, setWidth, addStyle, pixelSize,
	     pointSize, "*", spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateBold(bname);

// Any width or add style
   BuildName("*", "*", "bold", slant, "*", "*", pixelSize,
	     pointSize, resStr, spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateBold(bname);

// Any res
   BuildName("*", "*", "bold", slant, "*", "*", pixelSize,
	     pointSize, "*", spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateBold(bname);

// Point size only
   BuildName("*", "*", "bold", slant, "*", "*", "*",
	     pointSize, resStr, spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateBold(bname);

// Any res
   BuildName("*", "*", "bold", slant, "*", "*", "*",
	     pointSize, "*", spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateBold(bname);

// Pixel size only
   BuildName("*", "*", "bold", slant, "*", "*", pixelSize,
	     "*", resStr, spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateBold(bname);

// Any res
   BuildName("*", "*", "bold", slant, "*", "*", pixelSize,
	     "*", "*", spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateBold(bname);

//
// Try bigger fonts
//
   BuildName(foundry, family, "bold", slant, setWidth, addStyle, pixelSize,
	     pointSize, resStr, spacing, "*", charset, bname);
   if ( FindBigger(bname) ) return CreateBold(bname);

// Any res
   BuildName(foundry, family, "bold", slant, setWidth, addStyle,
	     pixelSize, pointSize, "*", spacing, "*", charset, bname);
   if ( FindBigger(bname) ) return CreateBold(bname);

// Any width or add style
   BuildName(foundry, family, "bold", slant, "*", "*", pixelSize,
	     pointSize, resStr, spacing, "*", charset, bname);
   if ( FindBigger(bname) ) return CreateBold(bname);

// Any res
   BuildName(foundry, family, "bold", slant, "*", "*", pixelSize,
	     pointSize, "*", spacing, "*", charset, bname);
   if ( FindBigger(bname) ) return CreateBold(bname);

// Point size only
   BuildName(foundry, family, "bold", slant, "*", "*", "*",
	     pointSize, resStr, spacing, "*", charset, bname);
   if ( FindBigger(bname) ) return CreateBold(bname);

// Any res
   BuildName(foundry, family, "bold", slant, "*", "*", "*",
	     pointSize, "*", spacing, "*", charset, bname);
   if ( FindBigger(bname) ) return CreateBold(bname);

// Pixel size only
   BuildName(foundry, family, "bold", slant, "*", "*", pixelSize,
	     "*", resStr, spacing, "*", charset, bname);
   if ( FindBigger(bname) ) return CreateBold(bname);

// Any res
   BuildName(foundry, family, "bold", slant, "*", "*", pixelSize,
	     "*", "*", spacing, "*", charset, bname);
   if ( FindBigger(bname) ) return CreateBold(bname);

// Any foundry or family
   BuildName("*", "*", "bold", slant, setWidth, addStyle, pixelSize,
	     pointSize, resStr, spacing, "*", charset, bname);
   if ( FindBigger(bname) ) return CreateBold(bname);

// Any res
   BuildName("*", "*", "bold", slant, setWidth, addStyle, pixelSize,
	     pointSize, "*", spacing, "*", charset, bname);
   if ( FindBigger(bname) ) return CreateBold(bname);

// Any width or add style
   BuildName("*", "*", "bold", slant, "*", "*", pixelSize,
	     pointSize, resStr, spacing, "*", charset, bname);
   if ( FindBigger(bname) ) return CreateBold(bname);

// Any res
   BuildName("*", "*", "bold", slant, "*", "*", pixelSize,
	     pointSize, "*", spacing, "*", charset, bname);
   if ( FindBigger(bname) ) return CreateBold(bname);

// Point size only
   BuildName("*", "*", "bold", slant, "*", "*", "*",
	     pointSize, resStr, spacing, "*", charset, bname);
   if ( FindBigger(bname) ) return CreateBold(bname);

// Any res
   BuildName("*", "*", "bold", slant, "*", "*", "*",
	     pointSize, "*", spacing, "*", charset, bname);
   if ( FindBigger(bname) ) return CreateBold(bname);

// Pixel size only
   BuildName("*", "*", "bold", slant, "*", "*", pixelSize,
	     "*", resStr, spacing, "*", charset, bname);
   if ( FindBigger(bname) ) return CreateBold(bname);

// Any res
   BuildName("*", "*", "bold", slant, "*", "*", pixelSize,
	     "*", "*", spacing, "*", charset, bname);
   if ( FindBigger(bname) ) return CreateBold(bname);

//
// If that didn't work, return this font
//
#if 0
   StringC errmsg("I could not find a bold version of the font: ");
   errmsg += name;
   halApp->PopupMessage(errmsg);
#else
   if ( debug1 )
      cout <<"I could not find a bold version of the font: " <<name <<endl;
#endif

   bold = this;
   return bold;

} // End Bold

/*----------------------------------------------------------------------
 * Method to create the specified bold font
 */

FontDataC*
FontDataC::CreateBold(char *bname)
{
//
// Create a new font structure
//
   if ( debug1 ) cout <<"Found bold font: " <<bname <<endl;
   bold = new FontDataC(bname);
   if ( !bold->loaded ) {
      delete bold;
      bold = this;
   }

   freeBold = True;
   return bold;

} // End CreateBold

/*----------------------------------------------------------------------
 * Method to return a non-bold version of the current font
 */

FontDataC*
FontDataC::NonBold()
{
   if ( !loaded && !nonBold ) nonBold = this;

//
// See if we already know this
//
   if ( nonBold ) return nonBold;

//
// See if this is already an italic font
//
   if ( !weight.Equals("bold", IGNORE_CASE) ) {
      nonBold = this;
      return nonBold;
   }

   if ( debug1 ) cout <<"Looking for a non-bold font for: " <<name NL;

//
// See if there is an medium or regular version of this font.
//
   StringC	bname;
   BuildName(foundry, family, "medium", slant, setWidth, addStyle, pixelSize,
	     pointSize, resStr, spacing, avgWidth, charset, bname);
   if ( FindFont(bname) ) return CreateNonBold(bname);

// Try regular
   BuildName(foundry, family, "regular", slant, setWidth, addStyle, pixelSize,
	     pointSize, resStr, spacing, avgWidth, charset, bname);
   if ( FindFont(bname) ) return CreateNonBold(bname);

// Any width or add style
   BuildName(foundry, family, "medium", slant, "*", "*", pixelSize,
	     pointSize, resStr, spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateNonBold(bname);

// regular
   BuildName(foundry, family, "regular", slant, "*", "*", pixelSize,
	     pointSize, resStr, spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateNonBold(bname);

// Any res, medium
   BuildName(foundry, family, "medium", slant, "*", "*", pixelSize,
	     pointSize, "*", spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateNonBold(bname);

// Any res, regular
   BuildName(foundry, family, "regular", slant, "*", "*", pixelSize,
	     pointSize, "*", spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateNonBold(bname);

// Point size only
   BuildName(foundry, family, "medium", slant, "*", "*", "*",
	     pointSize, resStr, spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateNonBold(bname);

// regular
   BuildName(foundry, family, "regular", slant, "*", "*", "*",
	     pointSize, resStr, spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateNonBold(bname);

// Any res, medium
   BuildName(foundry, family, "medium", slant, "*", "*", "*",
	     pointSize, "*", spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateNonBold(bname);

// Any res, regular
   BuildName(foundry, family, "regular", slant, "*", "*", "*",
	     pointSize, "*", spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateNonBold(bname);

// Pixel size only
   BuildName(foundry, family, "medium", slant, "*", "*", pixelSize,
	     "*", resStr, spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateNonBold(bname);

// regular
   BuildName(foundry, family, "regular", slant, "*", "*", pixelSize,
	     "*", resStr, spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateNonBold(bname);

// Any res, medium
   BuildName(foundry, family, "medium", slant, "*", "*", pixelSize,
	     "*", "*", spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateNonBold(bname);

// Any res, regular
   BuildName(foundry, family, "regular", slant, "*", "*", pixelSize,
	     "*", "*", spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateNonBold(bname);

// Any foundry or family
   BuildName("*", "*", "medium", slant, setWidth, addStyle, pixelSize,
	     pointSize, resStr, spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateNonBold(bname);

// regular
   BuildName("*", "*", "regular", slant, setWidth, addStyle, pixelSize,
	     pointSize, resStr, spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateNonBold(bname);

// Any res, medium
   BuildName("*", "*", "medium", slant, setWidth, addStyle, pixelSize,
	     pointSize, "*", spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateNonBold(bname);

// Any res, regular
   BuildName("*", "*", "regular", slant, setWidth, addStyle, pixelSize,
	     pointSize, "*", spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateNonBold(bname);

// Any width or addStyle
   BuildName("*", "*", "medium", slant, "*", "*", pixelSize,
	     pointSize, resStr, spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateNonBold(bname);

// regular
   BuildName("*", "*", "regular", slant, "*", "*", pixelSize,
	     pointSize, resStr, spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateNonBold(bname);

// Any res, medium
   BuildName("*", "*", "medium", slant, "*", "*", pixelSize,
	     pointSize, "*", spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateNonBold(bname);

// Any res, regular
   BuildName("*", "*", "regular", slant, "*", "*", pixelSize,
	     pointSize, "*", spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateNonBold(bname);

// Point size only
   BuildName("*", "*", "medium", slant, "*", "*", "*",
	     pointSize, resStr, spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateNonBold(bname);

// regular
   BuildName("*", "*", "regular", slant, "*", "*", "*",
	     pointSize, resStr, spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateNonBold(bname);

// Any res, medium
   BuildName("*", "*", "medium", slant, "*", "*", "*",
	     pointSize, "*", spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateNonBold(bname);

// Any res, regular
   BuildName("*", "*", "regular", slant, "*", "*", "*",
	     pointSize, "*", spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateNonBold(bname);

// Pixel size only
   BuildName("*", "*", "medium", slant, "*", "*", pixelSize,
	     "*", resStr, spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateNonBold(bname);

// regular
   BuildName("*", "*", "regular", slant, "*", "*", pixelSize,
	     "*", resStr, spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateNonBold(bname);

// Any res, medium
   BuildName("*", "*", "medium", slant, "*", "*", pixelSize,
	     "*", "*", spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateNonBold(bname);

// Any res, regular
   BuildName("*", "*", "regular", slant, "*", "*", pixelSize,
	     "*", "*", spacing, "*", charset, bname);
   if ( FindFont(bname) ) return CreateNonBold(bname);

//
// Look for different sized fonts, but only bigger ones
//
   BuildName(foundry, family, "medium", slant, setWidth, addStyle, pixelSize,
	     pointSize, resStr, spacing, avgWidth, charset, bname);
   if ( FindBigger(bname) ) return CreateNonBold(bname);

// Try regular
   BuildName(foundry, family, "regular", slant, setWidth, addStyle, pixelSize,
	     pointSize, resStr, spacing, avgWidth, charset, bname);
   if ( FindBigger(bname) ) return CreateNonBold(bname);

// Any res
   BuildName(foundry, family, "medium", slant, setWidth, addStyle, pixelSize,
	     pointSize, "*", spacing, avgWidth, charset, bname);
   if ( FindBigger(bname) ) return CreateNonBold(bname);

// Try regular
   BuildName(foundry, family, "regular", slant, setWidth, addStyle, pixelSize,
	     pointSize, "*", spacing, avgWidth, charset, bname);
   if ( FindBigger(bname) ) return CreateNonBold(bname);

// Any width or add style
   BuildName(foundry, family, "medium", slant, "*", "*", pixelSize,
	     pointSize, resStr, spacing, "*", charset, bname);
   if ( FindBigger(bname) ) return CreateNonBold(bname);

// regular
   BuildName(foundry, family, "regular", slant, "*", "*", pixelSize,
	     pointSize, resStr, spacing, "*", charset, bname);
   if ( FindBigger(bname) ) return CreateNonBold(bname);

// Any res, medium
   BuildName(foundry, family, "medium", slant, "*", "*", pixelSize,
	     pointSize, "*", spacing, "*", charset, bname);
   if ( FindBigger(bname) ) return CreateNonBold(bname);

// Any res, regular
   BuildName(foundry, family, "regular", slant, "*", "*", pixelSize,
	     pointSize, "*", spacing, "*", charset, bname);
   if ( FindBigger(bname) ) return CreateNonBold(bname);

// Any foundry or family
   BuildName("*", "*", "medium", slant, setWidth, addStyle, pixelSize,
	     pointSize, resStr, spacing, "*", charset, bname);
   if ( FindBigger(bname) ) return CreateNonBold(bname);

// regular
   BuildName("*", "*", "regular", slant, setWidth, addStyle, pixelSize,
	     pointSize, resStr, spacing, "*", charset, bname);
   if ( FindBigger(bname) ) return CreateNonBold(bname);

// Any res, medium
   BuildName("*", "*", "medium", slant, setWidth, addStyle, pixelSize,
	     pointSize, "*", spacing, "*", charset, bname);
   if ( FindBigger(bname) ) return CreateNonBold(bname);

// Any res, regular
   BuildName("*", "*", "regular", slant, setWidth, addStyle, pixelSize,
	     pointSize, "*", spacing, "*", charset, bname);
   if ( FindBigger(bname) ) return CreateNonBold(bname);

// Any width or addStyle
   BuildName("*", "*", "medium", slant, "*", "*", pixelSize,
	     pointSize, resStr, spacing, "*", charset, bname);
   if ( FindBigger(bname) ) return CreateNonBold(bname);

// regular
   BuildName("*", "*", "regular", slant, "*", "*", pixelSize,
	     pointSize, resStr, spacing, "*", charset, bname);
   if ( FindBigger(bname) ) return CreateNonBold(bname);

// Any res, medium
   BuildName("*", "*", "medium", slant, "*", "*", pixelSize,
	     pointSize, "*", spacing, "*", charset, bname);
   if ( FindBigger(bname) ) return CreateNonBold(bname);

// Any res, regular
   BuildName("*", "*", "regular", slant, "*", "*", pixelSize,
	     pointSize, "*", spacing, "*", charset, bname);
   if ( FindBigger(bname) ) return CreateNonBold(bname);

//
// If nothing worked, return this font
//
#if 0
   StringC errmsg("I could not find a non-bold version of the font: ");
   errmsg += name;
   halApp->PopupMessage(errmsg);
#else
   if ( debug1 )
      cout <<"I could not find a non-bold version of the font: " <<name<<endl;
#endif

   nonBold = this;
   return nonBold;

} // End NonBold

/*----------------------------------------------------------------------
 * Method to create the specified non-bold font
 */

FontDataC*
FontDataC::CreateNonBold(char *bname)
{
//
// Create a new font structure
//
   if ( debug1 ) cout <<"Found non-bold font: " <<bname <<endl;
   nonBold = new FontDataC(bname);
   if ( !nonBold->loaded ) {
      delete nonBold;
      nonBold = this;
   }

   freeNonBold = True;
   return nonBold;

} // End CreateNonBold

/*----------------------------------------------------------------------
 * Method to return an italic version of the current font
 */

FontDataC*
FontDataC::Italic()
{
   if ( !loaded && !italic ) italic = this;

//
// See if we already know this
//
   if ( italic ) return italic;

//
// See if this is already an italic font
//
   if ( slant.Equals('o', IGNORE_CASE) || slant.Equals('i', IGNORE_CASE) ) {
      italic = this;
      return italic;
   }

   if ( debug1 ) cout <<"Looking for an italic font for: " <<name NL;

//
// See if there is an oblique or italic version of this font.
//
   StringC	iname;
   BuildName(foundry, family, weight, "o", setWidth, addStyle, pixelSize,
	     pointSize, resStr, spacing, avgWidth, charset, iname);
   if ( FindFont(iname) ) return CreateItalic(iname);

// Try italic
   BuildName(foundry, family, weight, "i", setWidth, addStyle, pixelSize,
	     pointSize, resStr, spacing, avgWidth, charset, iname);
   if ( FindFont(iname) ) return CreateItalic(iname);

// Any width or add style
   BuildName(foundry, family, weight, "o", "*", "*", pixelSize,
	     pointSize, resStr, spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateItalic(iname);

// Italic
   BuildName(foundry, family, weight, "i", "*", "*", pixelSize,
	     pointSize, resStr, spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateItalic(iname);

// Any res, oblique
   BuildName(foundry, family, weight, "o", "*", "*", pixelSize,
	     pointSize, "*", spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateItalic(iname);

// Any res, italic
   BuildName(foundry, family, weight, "i", "*", "*", pixelSize,
	     pointSize, "*", spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateItalic(iname);

// Point size only
   BuildName(foundry, family, weight, "o", "*", "*", "*",
	     pointSize, resStr, spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateItalic(iname);

// Italic
   BuildName(foundry, family, weight, "i", "*", "*", "*",
	     pointSize, resStr, spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateItalic(iname);

// Any res, oblique
   BuildName(foundry, family, weight, "o", "*", "*", "*",
	     pointSize, "*", spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateItalic(iname);

// Any res, italic
   BuildName(foundry, family, weight, "i", "*", "*", "*",
	     pointSize, "*", spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateItalic(iname);

// Pixel size only
   BuildName(foundry, family, weight, "o", "*", "*", pixelSize,
	     "*", resStr, spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateItalic(iname);

// Italic
   BuildName(foundry, family, weight, "i", "*", "*", pixelSize,
	     "*", resStr, spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateItalic(iname);

// Any res, oblique
   BuildName(foundry, family, weight, "o", "*", "*", pixelSize,
	     "*", "*", spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateItalic(iname);

// Any res, italic
   BuildName(foundry, family, weight, "i", "*", "*", pixelSize,
	     "*", "*", spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateItalic(iname);

// Any foundry or family
   BuildName("*", "*", weight, "o", setWidth, addStyle, pixelSize,
	     pointSize, resStr, spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateItalic(iname);

// Italic
   BuildName("*", "*", weight, "i", setWidth, addStyle, pixelSize,
	     pointSize, resStr, spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateItalic(iname);

// Any res, oblique
   BuildName("*", "*", weight, "o", setWidth, addStyle, pixelSize,
	     pointSize, "*", spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateItalic(iname);

// Any res, italic
   BuildName("*", "*", weight, "i", setWidth, addStyle, pixelSize,
	     pointSize, "*", spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateItalic(iname);

// Any width or addStyle
   BuildName("*", "*", weight, "o", "*", "*", pixelSize,
	     pointSize, resStr, spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateItalic(iname);

// Italic
   BuildName("*", "*", weight, "i", "*", "*", pixelSize,
	     pointSize, resStr, spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateItalic(iname);

// Any res, oblique
   BuildName("*", "*", weight, "o", "*", "*", pixelSize,
	     pointSize, "*", spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateItalic(iname);

// Any res, italic
   BuildName("*", "*", weight, "i", "*", "*", pixelSize,
	     pointSize, "*", spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateItalic(iname);

// Point size only
   BuildName("*", "*", weight, "o", "*", "*", "*",
	     pointSize, resStr, spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateItalic(iname);

// Italic
   BuildName("*", "*", weight, "i", "*", "*", "*",
	     pointSize, resStr, spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateItalic(iname);

// Any res, oblique
   BuildName("*", "*", weight, "o", "*", "*", "*",
	     pointSize, "*", spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateItalic(iname);

// Any res, italic
   BuildName("*", "*", weight, "i", "*", "*", "*",
	     pointSize, "*", spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateItalic(iname);

// Pixel size only
   BuildName("*", "*", weight, "o", "*", "*", pixelSize,
	     "*", resStr, spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateItalic(iname);

// Italic
   BuildName("*", "*", weight, "i", "*", "*", pixelSize,
	     "*", resStr, spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateItalic(iname);

// Any res, oblique
   BuildName("*", "*", weight, "o", "*", "*", pixelSize,
	     "*", "*", spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateItalic(iname);

// Any res, italic
   BuildName("*", "*", weight, "i", "*", "*", pixelSize,
	     "*", "*", spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateItalic(iname);

//
// Look for different sized fonts, but only bigger ones
//
   BuildName(foundry, family, weight, "o", setWidth, addStyle, pixelSize,
	     pointSize, resStr, spacing, avgWidth, charset, iname);
   if ( FindBigger(iname) ) return CreateItalic(iname);

// Try italic
   BuildName(foundry, family, weight, "i", setWidth, addStyle, pixelSize,
	     pointSize, resStr, spacing, avgWidth, charset, iname);
   if ( FindBigger(iname) ) return CreateItalic(iname);

// Any res
   BuildName(foundry, family, weight, "o", setWidth, addStyle, pixelSize,
	     pointSize, "*", spacing, avgWidth, charset, iname);
   if ( FindBigger(iname) ) return CreateItalic(iname);

// Try italic
   BuildName(foundry, family, weight, "i", setWidth, addStyle, pixelSize,
	     pointSize, "*", spacing, avgWidth, charset, iname);
   if ( FindBigger(iname) ) return CreateItalic(iname);

// Any width or add style
   BuildName(foundry, family, weight, "o", "*", "*", pixelSize,
	     pointSize, resStr, spacing, "*", charset, iname);
   if ( FindBigger(iname) ) return CreateItalic(iname);

// Italic
   BuildName(foundry, family, weight, "i", "*", "*", pixelSize,
	     pointSize, resStr, spacing, "*", charset, iname);
   if ( FindBigger(iname) ) return CreateItalic(iname);

// Any res, oblique
   BuildName(foundry, family, weight, "o", "*", "*", pixelSize,
	     pointSize, "*", spacing, "*", charset, iname);
   if ( FindBigger(iname) ) return CreateItalic(iname);

// Any res, italic
   BuildName(foundry, family, weight, "i", "*", "*", pixelSize,
	     pointSize, "*", spacing, "*", charset, iname);
   if ( FindBigger(iname) ) return CreateItalic(iname);

// Any foundry or family
   BuildName("*", "*", weight, "o", setWidth, addStyle, pixelSize,
	     pointSize, resStr, spacing, "*", charset, iname);
   if ( FindBigger(iname) ) return CreateItalic(iname);

// Italic
   BuildName("*", "*", weight, "i", setWidth, addStyle, pixelSize,
	     pointSize, resStr, spacing, "*", charset, iname);
   if ( FindBigger(iname) ) return CreateItalic(iname);

// Any res, oblique
   BuildName("*", "*", weight, "o", setWidth, addStyle, pixelSize,
	     pointSize, "*", spacing, "*", charset, iname);
   if ( FindBigger(iname) ) return CreateItalic(iname);

// Any res, italic
   BuildName("*", "*", weight, "i", setWidth, addStyle, pixelSize,
	     pointSize, "*", spacing, "*", charset, iname);
   if ( FindBigger(iname) ) return CreateItalic(iname);

// Any width or addStyle
   BuildName("*", "*", weight, "o", "*", "*", pixelSize,
	     pointSize, resStr, spacing, "*", charset, iname);
   if ( FindBigger(iname) ) return CreateItalic(iname);

// Italic
   BuildName("*", "*", weight, "i", "*", "*", pixelSize,
	     pointSize, resStr, spacing, "*", charset, iname);
   if ( FindBigger(iname) ) return CreateItalic(iname);

// Any res, oblique
   BuildName("*", "*", weight, "o", "*", "*", pixelSize,
	     pointSize, "*", spacing, "*", charset, iname);
   if ( FindBigger(iname) ) return CreateItalic(iname);

// Any res, italic
   BuildName("*", "*", weight, "i", "*", "*", pixelSize,
	     pointSize, "*", spacing, "*", charset, iname);
   if ( FindBigger(iname) ) return CreateItalic(iname);

//
// If nothing worked, return this font
//
#if 0
   StringC errmsg("I could not find an italic version of the font: ");
   errmsg += name;
   halApp->PopupMessage(errmsg);
#else
   if ( debug1 )
      cout <<"I could not find an italic version of the font: " <<name<<endl;
#endif

   italic = this;
   return italic;

} // End Italic

/*----------------------------------------------------------------------
 * Method to create the specified italic font
 */

FontDataC*
FontDataC::CreateItalic(char *iname)
{
//
// Create a new font structure
//
   if ( debug1 ) cout <<"Found italic font: " <<iname <<endl;
   italic = new FontDataC(iname);
   if ( !italic->loaded ) {
      delete italic;
      italic = this;
   }

   freeItalic = True;
   return italic;

} // End CreateItalic

/*----------------------------------------------------------------------
 * Method to return a non-italic version of the current font
 */

FontDataC*
FontDataC::NonItalic()
{
   if ( !loaded && !nonItalic ) nonItalic = this;

//
// See if we already know this
//
   if ( nonItalic ) return nonItalic;

//
// See if this is already a non-italic font
//
   if ( !slant.Equals('o', IGNORE_CASE) && !slant.Equals('i', IGNORE_CASE) ) {
      nonItalic = this;
      return nonItalic;
   }

   if ( debug1 ) cout <<"Looking for a non-italic font for: " <<name <<endl;

//
// See if there is a non-italic version of this font.
//
   StringC	iname;
   BuildName(foundry, family, weight, "r", setWidth, addStyle, pixelSize,
	     pointSize, resStr, spacing, avgWidth, charset, iname);
   if ( FindFont(iname) ) return CreateNonItalic(iname);

// Any width or add style
   BuildName(foundry, family, weight, "r", "*", "*", pixelSize,
   	     pointSize, resStr, spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateNonItalic(iname);

// Any res
   BuildName(foundry, family, weight, "r", "*", "*", pixelSize,
   	     pointSize, "*", spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateNonItalic(iname);

// Point size only
   BuildName(foundry, family, weight, "r", "*", "*", "*",
   	     pointSize, resStr, spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateNonItalic(iname);

// Any res
   BuildName(foundry, family, weight, "r", "*", "*", "*",
   	     pointSize, "*", spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateNonItalic(iname);

// Pixel size only
   BuildName(foundry, family, weight, "r", "*", "*", pixelSize,
	     "*", resStr, spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateNonItalic(iname);

// Any res
   BuildName(foundry, family, weight, "r", "*", "*", pixelSize,
	     "*", "*", spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateNonItalic(iname);

// Any foundry or family
   BuildName("*", "*", weight, "r", setWidth, addStyle, pixelSize,
	     pointSize, resStr, spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateNonItalic(iname);

// Any res
   BuildName("*", "*", weight, "r", setWidth, addStyle, pixelSize,
	     pointSize, "*", spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateNonItalic(iname);

// Any width or add style
   BuildName("*", "*", weight, "r", "*", "*", pixelSize,
	     pointSize, resStr, spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateNonItalic(iname);

// Any res
   BuildName("*", "*", weight, "r", "*", "*", pixelSize,
	     pointSize, "*", spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateNonItalic(iname);

// Point size only
   BuildName("*", "*", weight, "r", "*", "*", "*",
	     pointSize, resStr, spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateNonItalic(iname);

// Any res
   BuildName("*", "*", weight, "r", "*", "*", "*",
	     pointSize, "*", spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateNonItalic(iname);

// Pixel size only
   BuildName("*", "*", weight, "r", "*", "*", pixelSize,
	     "*", resStr, spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateNonItalic(iname);

// Any res
   BuildName("*", "*", weight, "r", "*", "*", pixelSize,
	     "*", "*", spacing, "*", charset, iname);
   if ( FindFont(iname) ) return CreateNonItalic(iname);

//
// Try bigger fonts
//
   BuildName(foundry, family, weight, "r", setWidth, addStyle, pixelSize,
	     pointSize, resStr, spacing, "*", charset, iname);
   if ( FindBigger(iname) ) return CreateNonItalic(iname);

// Any res
   BuildName(foundry, family, weight, "r", setWidth, addStyle,
	     pixelSize, pointSize, "*", spacing, "*", charset, iname);
   if ( FindBigger(iname) ) return CreateNonItalic(iname);

// Any width or add style
   BuildName(foundry, family, weight, "r", "*", "*", pixelSize,
	     pointSize, resStr, spacing, "*", charset, iname);
   if ( FindBigger(iname) ) return CreateNonItalic(iname);

// Any res
   BuildName(foundry, family, weight, "r", "*", "*", pixelSize,
	     pointSize, "*", spacing, "*", charset, iname);
   if ( FindBigger(iname) ) return CreateNonItalic(iname);

// Point size only
   BuildName(foundry, family, weight, "r", "*", "*", "*",
	     pointSize, resStr, spacing, "*", charset, iname);
   if ( FindBigger(iname) ) return CreateNonItalic(iname);

// Any res
   BuildName(foundry, family, weight, "r", "*", "*", "*",
	     pointSize, "*", spacing, "*", charset, iname);
   if ( FindBigger(iname) ) return CreateNonItalic(iname);

// Pixel size only
   BuildName(foundry, family, weight, "r", "*", "*", pixelSize,
	     "*", resStr, spacing, "*", charset, iname);
   if ( FindBigger(iname) ) return CreateNonItalic(iname);

// Any res
   BuildName(foundry, family, weight, "r", "*", "*", pixelSize,
	     "*", "*", spacing, "*", charset, iname);
   if ( FindBigger(iname) ) return CreateNonItalic(iname);

// Any foundry or family
   BuildName("*", "*", weight, "r", setWidth, addStyle, pixelSize,
	     pointSize, resStr, spacing, "*", charset, iname);
   if ( FindBigger(iname) ) return CreateNonItalic(iname);

// Any res
   BuildName("*", "*", weight, "r", setWidth, addStyle, pixelSize,
	     pointSize, "*", spacing, "*", charset, iname);
   if ( FindBigger(iname) ) return CreateNonItalic(iname);

// Any width or add style
   BuildName("*", "*", weight, "r", "*", "*", pixelSize,
	     pointSize, resStr, spacing, "*", charset, iname);
   if ( FindBigger(iname) ) return CreateNonItalic(iname);

// Any res
   BuildName("*", "*", weight, "r", "*", "*", pixelSize,
	     pointSize, "*", spacing, "*", charset, iname);
   if ( FindBigger(iname) ) return CreateNonItalic(iname);

// Point size only
   BuildName("*", "*", weight, "r", "*", "*", "*",
	     pointSize, resStr, spacing, "*", charset, iname);
   if ( FindBigger(iname) ) return CreateNonItalic(iname);

// Any res
   BuildName("*", "*", weight, "r", "*", "*", "*",
	     pointSize, "*", spacing, "*", charset, iname);
   if ( FindBigger(iname) ) return CreateNonItalic(iname);

// Pixel size only
   BuildName("*", "*", weight, "r", "*", "*", pixelSize,
	     "*", resStr, spacing, "*", charset, iname);
   if ( FindBigger(iname) ) return CreateNonItalic(iname);

// Any res
   BuildName("*", "*", weight, "r", "*", "*", pixelSize,
	     "*", "*", spacing, "*", charset, iname);
   if ( FindBigger(iname) ) return CreateNonItalic(iname);

//
// If that didn't work, return this font
//
#if 0
   StringC errmsg("I could not find a non-italic version of the font: ");
   errmsg += name;
   halApp->PopupMessage(errmsg);
#else
   if ( debug1 )
      cout <<"I could not find a non-italic version of the font: " <<name NL;
#endif

   nonItalic = this;
   return nonItalic;

} // End NonItalic

/*----------------------------------------------------------------------
 * Method to create the specified non-italic font
 */

FontDataC*
FontDataC::CreateNonItalic(char *iname)
{
//
// Create a new font structure
//
   if ( debug1 ) cout <<"Found non-italic font: " <<iname <<endl;
   nonItalic = new FontDataC(iname);
   if ( !nonItalic->loaded ) {
      delete nonItalic;
      nonItalic = this;
   }

   freeNonItalic = True;
   return nonItalic;

} // End CreateNonItalic

/*----------------------------------------------------------------------
 * Method to return a comparable font with the specified charset
 */

FontDataC*
FontDataC::Charset(StringC cSet)
{
//
// See if this is already in the desired charset
//
   if ( !loaded || cSet.length() == 0 || charset.Equals(cSet, IGNORE_CASE) ||
	 cSet.Equals("us-ascii", IGNORE_CASE) )
      return this;

//
// See if we've already loaded this character set
//
   int	index = charSetNames.indexOf(cSet);
   if ( index != charSetNames.NULL_INDEX ) {
      FontDataC	*fd = (FontDataC*)*charSetFonts[index];
      if ( !fd ) fd = this;
      return fd;
   }

   if ( debug1 ) cout <<"Looking for alternate character set: " <<cSet
		      <<" for: " <<name NL;

//
// Change the character set and see if there is a match
//
   StringC	cname;
   BuildName(foundry, family, weight, slant, setWidth, addStyle, pixelSize,
	     pointSize, resStr, spacing, avgWidth, cSet, cname);
   if ( FindFont(cname) ) return CreateCharset(cname, cSet);

// Any res
   BuildName(foundry, family, weight, slant, setWidth, addStyle, pixelSize,
	     pointSize, "*", spacing, avgWidth, cSet, cname);
   if ( FindFont(cname) ) return CreateCharset(cname, cSet);

// Any width or style
   BuildName(foundry, family, weight, slant, "*", "*", pixelSize,
	     pointSize, resStr, spacing, "*", cSet, cname);
   if ( FindFont(cname) ) return CreateCharset(cname, cSet);

// Any res
   BuildName(foundry, family, weight, slant, "*", "*", pixelSize,
	     pointSize, "*", spacing, "*", cSet, cname);
   if ( FindFont(cname) ) return CreateCharset(cname, cSet);

// Any spacing
   BuildName(foundry, family, weight, slant, "*", "*", pixelSize,
	     pointSize, resStr, "*", "*", cSet, cname);
   if ( FindFont(cname) ) return CreateCharset(cname, cSet);

// Any res
   BuildName(foundry, family, weight, slant, "*", "*", pixelSize,
	     pointSize, "*", "*", "*", cSet, cname);
   if ( FindFont(cname) ) return CreateCharset(cname, cSet);

// Any foundry
   BuildName("*", family, weight, slant, "*", "*", pixelSize,
	     pointSize, resStr, "*", "*", cSet, cname);
   if ( FindFont(cname) ) return CreateCharset(cname, cSet);

// Any res
   BuildName("*", family, weight, slant, "*", "*", pixelSize,
	     pointSize, "*", "*", "*", cSet, cname);
   if ( FindFont(cname) ) return CreateCharset(cname, cSet);

// Any family
   BuildName("*", "*", weight, slant, "*", "*", pixelSize,
	     pointSize, resStr, "*", "*", cSet, cname);
   if ( FindFont(cname) ) return CreateCharset(cname, cSet);

// Any res
   BuildName("*", "*", weight, slant, "*", "*", pixelSize,
	     pointSize, "*", "*", "*", cSet, cname);
   if ( FindFont(cname) ) return CreateCharset(cname, cSet);

// Any bigger
   BuildName("*", "*", weight, slant, "*", "*", pixelSize,
	     pointSize, resStr, "*", "*", cSet, cname);
   if ( FindBigger(cname) ) return CreateCharset(cname, cSet);

// Any res
   BuildName("*", "*", weight, slant, "*", "*", pixelSize,
	     pointSize, "*", "*", "*", cSet, cname);
   if ( FindBigger(cname) ) return CreateCharset(cname, cSet);

// Any weight
   BuildName("*", "*", "*", slant, "*", "*", "*",
	     "*", resStr, "*", "*", cSet, cname);
   if ( FindFont(cname) ) return CreateCharset(cname, cSet);

// Any res
   BuildName("*", "*", "*", slant, "*", "*", "*",
	     "*", "*", "*", "*", cSet, cname);
   if ( FindFont(cname) ) return CreateCharset(cname, cSet);

// Any slant
   BuildName("*", "*", weight, "*", "*", "*", "*",
	     "*", resStr, "*", "*", cSet, cname);
   if ( FindFont(cname) ) return CreateCharset(cname, cSet);

// Any res
   BuildName("*", "*", weight, "*", "*", "*", "*",
	     "*", "*", "*", "*", cSet, cname);
   if ( FindFont(cname) ) return CreateCharset(cname, cSet);

// Any with correct size
   BuildName("*", "*", "*", "*", "*", "*", "*",
	     pointSize, resStr, "*", "*", cSet, cname);
   if ( FindFont(cname) ) return CreateCharset(cname, cSet);

// Any res
   BuildName("*", "*", "*", "*", "*", "*", "*",
	     pointSize, "*", "*", "*", cSet, cname);
   if ( FindFont(cname) ) return CreateCharset(cname, cSet);

// Any with correct size
   BuildName("*", "*", "*", "*", "*", "*", pixelSize,
	     "*", resStr, "*", "*", cSet, cname);
   if ( FindFont(cname) ) return CreateCharset(cname, cSet);

// Any res
   BuildName("*", "*", "*", "*", "*", "*", pixelSize,
	     "*", "*", "*", "*", cSet, cname);
   if ( FindFont(cname) ) return CreateCharset(cname, cSet);

// Any
   BuildName("*", "*", "*", "*", "*", "*", "*",
	     "*", resStr, "*", "*", cSet, cname);
   if ( FindFont(cname) ) return CreateCharset(cname, cSet);

// Any res
   BuildName("*", "*", "*", "*", "*", "*", "*",
	     "*", "*", "*", "*", cSet, cname);
   if ( FindFont(cname) ) return CreateCharset(cname, cSet);

   StringC errmsg("I could not find a font with the character set: ");
   errmsg += cSet;
   errmsg += ".\nThe font: ";
   errmsg += name;
   errmsg += "\nwill be used instead.";
   halApp->PopupMessage(errmsg);

   if ( debug1 ) cout <<"Using: " <<name <<endl;
   void	*tmp = (void*)this;

   charSetNames.add(cSet);
   charSetFonts.add(tmp);
   freeCharSet.add(FALSE);

   return this;

} // End Charset

/*----------------------------------------------------------------------
 * Method to create the specified charset font
 */

FontDataC*
FontDataC::CreateCharset(char *cname, char *cSet)
{
//
// Create a new font structure
//
   if ( debug1 ) cout <<"Using: " <<cname <<endl;
   FontDataC	*csFont = new FontDataC(cname);
   int		freeIt  = TRUE;

   if ( !csFont->loaded ) {

      delete csFont;

      StringC errmsg("I could not find a font with the character set: ");
      errmsg += cSet;
      errmsg += ".\nThe font: ";
      errmsg += name;
      errmsg += "\nwill be used instead.";
      halApp->PopupMessage(errmsg);

      if ( debug1 ) cout <<"Using: " <<name NL;
      csFont = this;
      freeIt = FALSE;
   }

   void	*tmp = (void*)csFont;

   charSetNames.add(cSet);
   charSetFonts.add(tmp);
   freeCharSet.add(freeIt);
   return csFont;

} // End CreateCharset

/*----------------------------------------------------------------------
 * Method to return a font matching the requested criteria
 */

Boolean
FontDataC::FindFont(StringC& fname)
{
   if ( debug2 ) cout <<"Trying: " <<fname NL;

   int	actual;
   char	**names = XListFonts(halApp->display, fname, 1, &actual);
   if ( !names ) return False;

   fname = names[0];
   XFreeFontNames(names);

   return True;

} // End FindFont

/*----------------------------------------------------------------------
 * Method to determine whether this font is a monospaced font
 */

Boolean
FontDataC::IsFixed()
{
   return (spacing.Equals('m', IGNORE_CASE) ||
   	   spacing.Equals('c', IGNORE_CASE));
}

/*----------------------------------------------------------------------
 * Method to determine whether this font is a bold font
 */

Boolean
FontDataC::IsBold()
{
   return weight.Equals("bold", IGNORE_CASE);
}

/*----------------------------------------------------------------------
 * Method to determine whether this font is an italic font
 */

Boolean
FontDataC::IsItalic()
{
   return (slant.Equals('o', IGNORE_CASE) ||
	   slant.Equals('i', IGNORE_CASE));
}
