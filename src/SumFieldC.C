/*
 * $Id: SumFieldC.C,v 1.2 2000/05/07 12:26:13 fnevgeny Exp $
 *
 * Copyright (c) 1994 HAL Computer Systems International, Ltd.
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

#include "SumFieldC.h"

#include <Xm/TextF.h>
#include <Xm/ToggleB.h>
#include <values.h>

/*---------------------------------------------------------------
 *  Method to set data
 */

void
SumFieldC::Set(SumFieldType sft, Widget tb, Widget posTF, Widget minTF,
		 Widget maxTF, Widget titleTF)
{
   char	*cs;

   if ( XmTextFieldGetLastPosition(posTF) > 0 ) {
      cs = XmTextFieldGetString(posTF);
      pos = atoi(cs);
      XtFree(cs);
   } else {
      pos = MAXINT;
   }

   if ( XmTextFieldGetLastPosition(minTF) > 0 ) {
      cs = XmTextFieldGetString(minTF);
      min = atoi(cs);
      XtFree(cs);
   } else {
      min = 0;
   }

   if ( XmTextFieldGetLastPosition(maxTF) > 0 ) {
      cs = XmTextFieldGetString(maxTF);
      max = atoi(cs);
      XtFree(cs);
   } else {
      max = 0;
   }

   cs = XmTextFieldGetString(titleTF);
   title = cs;
   XtFree(cs);

   type = sft;
   show = XmToggleButtonGetState(tb);

} // End Set

/*------------------------------------------------------------------------
 * Method to compare two fields
 */

int
SumFieldC::ComparePositions(const void *a, const void *b)
{
   SumFieldC	*fa = (SumFieldC *)a;
   SumFieldC	*fb = (SumFieldC *)b;

   return (fa->pos < fb->pos) ? -1 : ((fa->pos > fb->pos) ? 1 : 0);
}
