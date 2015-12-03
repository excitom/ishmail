/*
 *  $Id: SigPrefC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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
#ifndef _SigPrefC_h
#define _SigPrefC_h

#include "PrefC.h"

enum SigTypeT {
   PLAIN_SIG,		// Always use plain
   ENRICHED_SIG,	// Always use enriched
   ENRICHED_MIME_SIG	// Use enriched only in MIME messages
};

class SigPrefWinC;

class SigPrefC : public PrefC {

//
// Private data.  We don't want anyone setting these directly.
//
   StringC		signature;      // Fallback signature string
   SigPrefWinC		*prefWin;

//
// As typed by the user
//
   struct {
      StringC		extPSigFile;	// External plain    signature file
      StringC		extESigFile;	// External enriched signature file
      StringC		intPSigFile;	// Internal plain    signature file
      StringC		intESigFile;	// Internal enriched signature file
   } orig;

//
// Fully expanded
//
   StringC		extPSigFile;	// External plain    signature file
   StringC		extESigFile;	// External enriched signature file
   StringC		intPSigFile;	// Internal plain    signature file
   StringC		intESigFile;	// Internal enriched signature file

//
// Private methods
//
   StringC		Sig(StringC&);

public:

//
// Public data
//
   Boolean		appendSig;	// Add signature to outgoing messages
   Boolean		addPrefix;	// Add "--" if true
   SigTypeT		type;		// Which one to use

//
// Public methods
//
    SigPrefC();
   ~SigPrefC();

   void			SetExtPSigFile(const char*);
   void			SetExtESigFile(const char*);
   void			SetIntPSigFile(const char*);
   void			SetIntESigFile(const char*);

   StringC&		ExtPSigFile()	{ return extPSigFile; }
   StringC&		ExtESigFile()	{ return extESigFile; }
   StringC&		IntPSigFile()	{ return intPSigFile; }
   StringC&		IntESigFile()	{ return intESigFile; }

   StringC&		OrigExtPSigFile()	{ return orig.extPSigFile; }
   StringC&		OrigExtESigFile()	{ return orig.extESigFile; }
   StringC&		OrigIntPSigFile()	{ return orig.intPSigFile; }
   StringC&		OrigIntESigFile()	{ return orig.intESigFile; }

   inline StringC	Sig()			{ return Sig(extPSigFile); }
   inline StringC	EnrichedSig()		{ return Sig(extESigFile); }
   inline StringC	InternalSig()		{ return Sig(intPSigFile); }
   inline StringC	InternalEnrichedSig()	{ return Sig(intESigFile); }

   void			Edit(Widget);
   Boolean		WriteDatabase();
   Boolean		WriteFile();
};

#endif // _SigPrefC_h
