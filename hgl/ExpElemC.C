/*
 *  $Id: ExpElemC.C,v 1.2 2000/05/07 12:26:10 fnevgeny Exp $
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

#include <config.h>
#include "ExpElemC.h"
#include "BoolExpC.h"
#include "HalAppC.h"

/*-------------------------------------------------------------------------
 * ExpElemC constructors
 */

ExpElemC::ExpElemC(ExpElemC::ExpElemT t, Widget w)
{
   type   = t;
   widget = w;
   term   = NULL;
   wd = ht = 0;
}

ExpElemC::ExpElemC(TermExpC *t, Widget w)
{
   type   = TERMINAL;
   widget = w;
   term   = t;
   wd = ht = 0;
}

/*-------------------------------------------------------------------------
 * ExpElemC destructor
 */

ExpElemC::~ExpElemC()
{
   if ( halApp->xRunning ) {
      XtUnmanageChild(widget);
      XtDestroyWidget(widget);
   }
   delete term;
}

/*-------------------------------------------------------------------------
 * ExpElemC assignment
 */

ExpElemC&
ExpElemC::operator=(const ExpElemC& e)
{
   type   = e.type;
   widget = e.widget;
   term   = e.term;
   wd     = e.wd;
   ht     = e.ht;

   return *this;
}

/*-------------------------------------------------------------------------
 * ExpElemC equivalence
 */

int
ExpElemC::operator==(const ExpElemC& e) const
{
   return ( type == e.type && widget == e.widget && term == e.term );
}

/*-------------------------------------------------------------------------
 * Get element widget size
 */

void
ExpElemC::GetSize()
{
   XtVaGetValues(widget, XmNwidth, &wd, XmNheight, &ht, NULL);
}

