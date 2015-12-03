/*
 * $Id: HalMainWinC.C,v 1.2 2000/05/07 12:26:10 fnevgeny Exp $
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

#include "HalMainWinC.h"
#include "HalAppC.h"
#include "WArgList.h"

#include <X11/Shell.h>
#include <Xm/Protocols.h>
#include <Xm/MainW.h>
#include <Xm/Frame.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include <Xm/Label.h>
#include <Xm/CascadeB.h>
#include <Xm/PushB.h>
#include <Xm/AtomMgr.h>

/*----------------------------------------------------------------------
 * Constructor
 */

HalMainWinC::HalMainWinC(const char *name, Widget parent)
: HalTopLevelC(name, parent)
{
//
// Catch kill from the window manager.  Exit from the app rather than just
//    close the window.
//
   XmRemoveWMProtocolCallback(shell, halApp->delWinAtom, (XtCallbackProc)DoHide,
			      (caddr_t)this);
   XmAddWMProtocolCallback(shell, halApp->delWinAtom, (XtCallbackProc)DoExit,
			   (caddr_t)this);

//
// Manage menu bar
//
   AddMenuBar();

} // End HalMainWinC constructor
