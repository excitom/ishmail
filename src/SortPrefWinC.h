/*
 *  $Id: SortPrefWinC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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
#ifndef _SortPrefWinC_h_
#define _SortPrefWinC_h_

#include "OptWinC.h"

#include <hgl/WidgetListC.h>

class RowColC;
class StringC;
class SortMgrC;

/*--------------------------------------------------------------------------
 * Class for editing message sort expressions
 */

typedef struct {

   Widget		label;
   Widget		tb;	// Key toggle button
   Widget		atb;	// Ascending toggle button
   Widget		dtb;	// Descending toggle button
   char			*name;	// Displayed in key string

} KeyStruct;

class SortPrefWinC : public OptWinC {

   RowColC		*keyRC;
   KeyStruct		thread;
   KeyStruct		number;
   KeyStruct		status;
   KeyStruct		sender;
   KeyStruct		to;
   KeyStruct		subject;
   KeyStruct		date;
   KeyStruct		lines;
   KeyStruct		bytes;
   Widget		folderCurTB;
   Widget		folderSelTB;
   Widget		folderAllTB;
   Widget		folderDefTB;

   WidgetListC		selectedList;

   static void ToggleKey(Widget, SortPrefWinC*, XmToggleButtonCallbackStruct*);
   static void ToggleDir(Widget, SortPrefWinC*, XmToggleButtonCallbackStruct*);

//
// Private methods
//
   Boolean	Apply();
   void		BuildKeyString(StringC&);
   void		UpdateOrderLabel(KeyStruct);
   void		UpdateOrderLabels();

public:

// Methods

   SortPrefWinC(Widget);
   ~SortPrefWinC();

   void		Show(SortMgrC*);
   void		Show() { HalDialogC::Show(); }
};

#endif // _SortPrefWinC_h_
