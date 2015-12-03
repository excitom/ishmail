/*
 *  $Id: FontChooserC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
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
#ifndef _FontChooserC_h_
#define _FontChooserC_h_

#include "HalDialogC.h"
#include "StringListC.h"
#include "CallbackListC.h"

class RowColC;
class CharC;

typedef struct {

   char		*name;
   StringC	family;
   StringC	style;
   StringC	pixelSize;
   StringC	pointSize;
   Boolean	fixed;

} FontRecT;

class FontChooserC : public HalDialogC {

//
// Widgets
//
   Widget	familyLabel;
   Widget	styleLabel;
   Widget	sizeLabel;
   Widget	familyListW;
   Widget	styleListW;
   Widget	sizeListW;
   Widget	sizePixelsTB;
   Widget	sizePointsTB;
   Widget	showFixedTB;
   Widget	showPropTB;
   Widget	sampText;
   Widget	nameTF;

//
// Data
//
   char			**fontNames;
   int			fontCount;
   FontRecT		*fontData;
   XFontStruct		*sampleFont;
   StringC		sampleStr;
   XmFontList		sampleFontList;
   StringC		selectedFamily;
   StringC		selectedStyle;
   StringC		selectedSize;
   CallbackListC	okCalls;

//
// Callbacks
//
   static int	CompareSizes(const StringC*, const StringC*);
   static void	DoOk        (Widget, FontChooserC*, XtPointer);
   static void	DoPopup     (Widget, FontChooserC*, XtPointer);
   static void	SelectFamily(Widget, FontChooserC*, XmListCallbackStruct*);
   static void	SelectSize  (Widget, FontChooserC*, XmListCallbackStruct*);
   static void	SelectStyle (Widget, FontChooserC*, XmListCallbackStruct*);
   static void	ToggleProp  (Widget, FontChooserC*,
   			     XmToggleButtonCallbackStruct*);
   static void	ToggleSize  (Widget, FontChooserC*,
   			     XmToggleButtonCallbackStruct*);

//
// Private methods
//
   Boolean		FamilyOk(CharC);
   CharC		GetNamePart(char*, int);
   StringC		GetFamily(char*);
   StringC		GetPixelSize(char*);
   StringC		GetPointSize(char*);
   StringC		GetStyle(char*);
   Boolean		IsFixed(char*);
   void			SetList(Widget, StringListC&);
   Boolean		SizeOk(CharC);
   Boolean		StyleOk(CharC);
   void			UpdateLists(Boolean changeFamily=True,
   				    Boolean changeStyle=True,
				    Boolean changeSize=True);
   void			UpdateSample();

public:

//
// Constructor and destructor
//
   FontChooserC(Widget, char *name="fontChooser", ArgList argv=NULL,
		Cardinal argc=0);
   ~FontChooserC();

//
// Public methods
//
   inline void	AddOkCallback(CallbackFn *fn, void *data) {
      AddCallback(okCalls, fn, data);
   }
   inline void	RemoveOkCallback(CallbackFn *fn, void *data) {
      RemoveCallback(okCalls, fn, data);
   }

   void		AllowFixed(Boolean);
   void		AllowProp (Boolean);
   void		ShowFixed (Boolean);
   void		ShowProp  (Boolean);
};

#endif // _FontChooserC_h_
