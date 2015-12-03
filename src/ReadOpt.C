/*
 *  $Id: ReadOpt.C,v 1.2 2000/05/07 12:26:12 fnevgeny Exp $
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
#include "ReadWinP.h"
#include "ReadWinC.h"
#include "IshAppC.h"
#include "ReadPrefC.h"
#include "ReadButtPrefC.h"

/*---------------------------------------------------------------
 *  Callback to handle Options->Preferences
 */

void
ReadWinP::DoOptPref(Widget, ReadWinP *This, XtPointer)
{
   ishApp->readPrefs->Edit(*This->pub);
}

/*---------------------------------------------------------------
 *  Callback to handle Options->Buttons
 */

void
ReadWinP::DoOptButt(Widget, ReadWinP *This, XtPointer)
{
   ishApp->readButtPrefs->Edit(*This->pub, This->pub->buttMgr);
}

/*---------------------------------------------------------------
 *  Callback to show all headers
 */

void
ReadWinP::DoOptHead(Widget, ReadWinP *This, XmToggleButtonCallbackStruct *tb)
{
//
// Redisplay headers
//
   This->DisplayHeaders();
}

/*---------------------------------------------------------------
 *  Callback to change text wrapping
 */

void
ReadWinP::DoOptWrap(Widget, ReadWinP *This, XmToggleButtonCallbackStruct *tb)
{
   This->pub->SetWrap(tb->set);
}

/*---------------------------------------------------------------
 *  Callback to handle change of view type
 */

void
ReadWinP::DoOptView(Widget w, ReadWinP *This, XmToggleButtonCallbackStruct *tb)
{
   if ( !tb->set ) return;

   ReadViewTypeT	type;
   if      ( w == This->viewOutlineTB ) type = READ_VIEW_OUTLINE;
   else if ( w == This->viewNestedTB  ) type = READ_VIEW_CONTAINER;
   else if ( w == This->viewSourceTB  ) type = READ_VIEW_SOURCE;
   else					type = READ_VIEW_FLAT;

   This->pub->SetViewType(type);
}

