/*
 * $Id: WXmString.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
 *
 * Copyright (c) 1991 HaL Computer Systems, Inc.  All rights reserved.
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

#ifndef WXmString_h
#define WXmString_h

#ifndef _Xm_h
#include <Xm/Xm.h>
#endif

class WXmString {

protected:

   XmString	string;

public:

// Constructors

   inline	WXmString ()		{ string = 0; }
   inline	WXmString (XmString s)	{ string = XmStringCopy(s); }
   inline	WXmString (const WXmString& w)	{ string = w.Copy(); }

#if XmVersion >= 1002
   inline	WXmString (const char* t, char *tag=XmFONTLIST_DEFAULT_TAG) {
      string = XmStringCreateLtoR ((char *)t, tag);
   }
#else
   inline	WXmString (const char* t,
			   XmStringCharSet charset=XmSTRING_DEFAULT_CHARSET) {
      string = XmStringCreateLtoR ((char *)t, charset);
   }
#endif
   inline	WXmString (XmStringDirection d)	{
      string = XmStringDirectionCreate (d);
   }

// Destructor

   inline	~WXmString () {
      if (string) XmStringFree (string);
   }

// Operators

   inline WXmString&	operator= (const WXmString& w) {
      if (string) XmStringFree (string);
      string = XmStringCopy(w.string);
      return *this;
   }
   inline WXmString&	operator= (XmString s) {
      if (string) XmStringFree (string);
      string = XmStringCopy(s);
      return *this;
   }
   inline WXmString&	operator= (const char *cs) {
      if (string) XmStringFree (string);
#if XmVersion >= 1002
      string = XmStringCreateLtoR ((char *)cs, XmFONTLIST_DEFAULT_TAG);
#else
      string = XmStringCreateLtoR ((char *)cs, XmSTRING_DEFAULT_CHARSET);
#endif
      return *this;
   }

   inline		operator XmString () const { return string; }
   inline		operator XtArgVal () const { return (XtArgVal) string; }
   inline		operator char* () const	{
      char* ret;
#if XmVersion >= 1002
      XmStringGetLtoR (string, XmFONTLIST_DEFAULT_TAG, &ret);
#else
      XmStringGetLtoR (string, XmSTRING_DEFAULT_CHARSET, &ret);
#endif
      return ret;
   }
   inline Boolean	operator== (XmString s) const {
      return ByteCompare (s);
   }
   inline WXmString&	operator<< (const WXmString& s)	{
      XmString n = Concat(s.string);
      XmStringFree(string);
      string = n;
      return *this;
   }

// Methods

   inline void		Free ()	{
      if (string) XmStringFree (string);
      string = 0;
   }
   inline Dimension	Baseline (XmFontList fl) const {
      return XmStringBaseline (fl, string);
   }
   inline Boolean	ByteCompare (XmString s) const {
      return XmStringByteCompare (string, s);
   }
   inline Boolean	Compare (XmString s) const {
      return XmStringCompare (string, s);
   }
   inline XmString	Concat (XmString s) const {
      return XmStringConcat (string, s);
   }
   inline XmString	Copy () const { return XmStringCopy (string); }
   inline Boolean	Empty () const { return XmStringEmpty (string); }
   inline void		Extent (XmFontList fl, Dimension* w, Dimension* h)
   const {
      XmStringExtent (fl, string, w, h);
   }
#if XmVersion >= 1002
   inline Boolean	GetLtoR (char** text, char *tag=XmFONTLIST_DEFAULT_TAG)
   const {
      return XmStringGetLtoR (string, tag, text);
   }
#else
   inline Boolean	GetLtoR (char** text,
				 XmStringCharSet cs=XmSTRING_DEFAULT_CHARSET)
   const {
      return XmStringGetLtoR (string, cs, text);
   }
#endif
   inline Dimension	Height (XmFontList fl) const {
      return XmStringHeight (fl, string);
   }
   inline int		LineCount () const {
      return XmStringLineCount (string);
   }
   inline void		NConcat (XmString s, int n) {
      XmStringNConcat (string, s, n);
   }
   inline Dimension	Width (XmFontList fl) const {
      return XmStringWidth (fl, string);
   }
   inline void		Zero ()	{ string = 0; }

   WXmString  CopyUsingFont( char* newtag );

};

#endif
