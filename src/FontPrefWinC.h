/*
 *  $Id: FontPrefWinC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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
#ifndef _FontPrefWinC_h_
#define _FontPrefWinC_h_

#include "OptWinC.h"

class FontChooserC;
class FontDataC;

class FontPrefWinC : public OptWinC {

//
// Widgets
//
   Widget	fontRC;
   Widget	pbLabel;
   Widget	pbTF;
   Widget	pbChoosePB;
   Widget	labelLabel;
   Widget	labelTF;
   Widget	labelChoosePB;
   Widget	textLabel;
   Widget	textTF;
   Widget	textChoosePB;
   Widget	richLabel;
   Widget	richTF;
   Widget	richChoosePB;
   Widget	listLabel;
   Widget	listTF;
   Widget	listChoosePB;
   Widget	curTF;
   Widget	fontPD;
   FontChooserC	*chooser;

   FontDataC	*pbFont;
   FontDataC	*labelFont;
   FontDataC	*textFont;
   FontDataC	*richFont;
   FontDataC	*listFont;

//
// Static methods
//
   static void	SetField     (Widget, FontPrefWinC*, XtPointer);
   static void	PickFont     (Widget, FontPrefWinC*, XtPointer);
   static void	FinishPickFont(char*, FontPrefWinC*);

//
// Private methods
//
   Boolean	Apply();
   void		UpdateButtons  (Widget, ArgList, Cardinal);
   void		UpdateLabels   (Widget, ArgList, Cardinal);
   void		UpdateText     (Widget, ArgList, Cardinal);
   void		UpdateRich     (Widget, const char*);
   void		UpdateLists    (Widget, ArgList, Cardinal);
   void		UpdateRichFixed(Widget, const char*);

public:

// Methods

   FontPrefWinC(Widget);
   ~FontPrefWinC();
};

#endif // _FontPrefWinC_h_
