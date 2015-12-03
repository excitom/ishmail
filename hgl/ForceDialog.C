/*
 * $Id: ForceDialog.C,v 1.2 2000/05/07 12:26:10 fnevgeny Exp $
 *
 * Copyright (c) 1992 HaL Computer Systems, Inc.  All rights reserved.
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

/*
 * This procedure will ensure that, if a dialog window is being mapped,
 * its contents become visible before returning.  It is intended to be
 * used just before a bout of computing that doesn't service the display.
 * You should still call XmUpdateDisplay() at intervals during this
 * computing if possible.
 *
 * The monitoring of window states is necessary because attempts to map
 * the dialog are redirected to the window manager (if there is one) and
 * this introduces a significant delay before the window is actually mapped
 * and exposed.  This code works under mwm, twm, uwm, and no-wm.  It
 * doesn't work (but doesn't hang) with olwm if the mainwindow is iconified.
 *
 * The argument to ForceDialog is any widget in the dialog (often it
 * will be the BulletinBoard child of a DialogShell).
 */

#ifdef ICL
#include <stddef.h>
#endif
#include <Xm/Xm.h>

void
ForceDialog(Widget w)
{
//
// Locate the shell we are interested in.  In a particular instance, you
// may know these shells already.
//
   Widget diashell = w;
   while ( !XtIsShell(diashell) ) {
      diashell = XtParent(diashell);
   }

//
// Locate its primary window's shell (which may be the same)
//
   Widget topshell = diashell;
   while ( !XtIsTopLevelShell(topshell) ) {
      topshell = XtParent(topshell);
   }

   if ( XtIsRealized(diashell) && XtIsRealized(topshell) ) {

      Display	   *dpy	     = XtDisplay(topshell);
      Window	   diawindow = XtWindow(diashell);
      Window	   topwindow = XtWindow(topshell);
      XtAppContext cxt	     = XtWidgetToApplicationContext(diashell);

//
// Wait for the dialog to be mapped.  It's guaranteed to become so unless...
//
      XWindowAttributes	xwa;
      while ( XGetWindowAttributes(dpy, diawindow, &xwa),
	      xwa.map_state != IsViewable ) {

//
// ...if the primary is (or becomes) unviewable or unmapped, it's
//   probably iconified, and nothing will happen.
//
	 if ( XGetWindowAttributes(dpy, topwindow, &xwa),
	      xwa.map_state != IsViewable ) {
	    break;
	 }

//
// At this stage, we are guaranteed there will be an event of some kind.
//   Beware; we are presumably in a callback, so this can recurse.
//
#if 0
//
// This code will allow timers to be called which is not good.  We don't
//    want to be interrupted.
//
	 XEvent	event;
	 XtAppNextEvent(cxt, &event);
	 XtDispatchEvent(&event);
#else
	 XtAppProcessEvent(cxt, XtIMXEvent);
#endif
      }
   }

//
// The next XSync() will get an expose event if the dialog was unmapped.
//
   XmUpdateDisplay(topshell);

} // End ForceDialog
