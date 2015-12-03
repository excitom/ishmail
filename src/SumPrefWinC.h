/*
 *  $Id: SumPrefWinC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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
#ifndef _SumPrefWinC_h_
#define _SumPrefWinC_h_

#include "OptWinC.h"
#include "SumFieldC.h"

class SumPrefWinC : public OptWinC {

protected:

   Widget		numberForm;
   Widget		numberTB;
   Widget		numberPosTF;
   Widget		numberMinTF;
   Widget		numberMaxTF;
   Widget		numberTitleTF;
   Widget		statusForm;
   Widget		statusTB;
   Widget		statusPosTF;
   Widget		statusMinTF;
   Widget		statusMaxTF;
   Widget		statusTitleTF;
   Widget		senderForm;
   Widget		senderTB;
   Widget		senderPosTF;
   Widget		senderMinTF;
   Widget		senderMaxTF;
   Widget		senderTitleTF;
   Widget		subjectForm;
   Widget		subjectTB;
   Widget		subjectPosTF;
   Widget		subjectMinTF;
   Widget		subjectMaxTF;
   Widget		subjectTitleTF;
   Widget		dateForm;
   Widget		dateTB;
   Widget		datePosTF;
   Widget		dateMinTF;
   Widget		dateMaxTF;
   Widget		dateTitleTF;
   Widget		lineForm;
   Widget		lineTB;
   Widget		linePosTF;
   Widget		lineMinTF;
   Widget		lineMaxTF;
   Widget		lineTitleTF;
   Widget		byteForm;
   Widget		byteTB;
   Widget		bytePosTF;
   Widget		byteMinTF;
   Widget		byteMaxTF;
   Widget		byteTitleTF;
   Widget		iconTB;
   Widget		dateFormatTF;

//
// Private methods
//
   Boolean		Apply();
   void			Set(SumFieldC::SumFieldType, Widget, Widget, Widget,
						     Widget, Widget);
   void			Write();

public:

// Methods

   SumPrefWinC(Widget);

   void		Show();
};

#endif // _SumPrefWinC_h_
