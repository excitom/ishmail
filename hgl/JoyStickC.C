/*
 * $Id: JoyStickC.C,v 1.2 2000/05/07 12:26:10 fnevgeny Exp $
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

#ifdef ICL
#include <stddef.h>
#endif

#include "JoyStickC.h"
#include "WArgList.h"
#include "HalAppC.h"
#include "rsrc.h"

#include <X11/X.h>
#include <Xm/Xm.h>
#include <Xm/DrawingA.h>

#ifdef debug
#define dprintf(a) printf a
#else
#define dprintf(a)
#endif


/*-----------------------------------------------------------------------
 *  Constructor
 */

JoyStickC::JoyStickC(Widget parent, char *nm, ArgList argv, Cardinal argc)
{
   viewDA = XmCreateDrawingArea(parent, nm, argv, argc);

   WArgList args;
   args.Reset();
   args.ResizePolicy(XmRESIZE_ANY);
   args.MarginWidth(0);
   args.MarginHeight(0);
   XtSetValues(viewDA, ARGS);

   timer  = 0;
   state  = 0;
   viewGC = NULL;
   viewDSP = halApp->display;

   stickColor = get_color("JoyStickC", viewDA, "stickColor",       "cyan");
   interval   = get_int  ("JoyStickC", viewDA, "joyStickInterval", 25);
   stickWidth = get_int  ("JoyStickC", viewDA, "joyStickWidth",    3);

//
// Add the event handlers and callbacks.
//
   XtAddEventHandler(viewDA, StructureNotifyMask, False,
			(XtEventHandler)HandleResize, (XtPointer)this);
   XtAddCallback(viewDA, XmNexposeCallback,
		 (XtCallbackProc)DoExpose, (XtPointer)this);
   XtAddEventHandler(viewDA, StructureNotifyMask, False,
		     (XtEventHandler)HandleMapChange, (XtPointer)this);
   XtAddEventHandler(viewDA, ButtonPressMask, False,
                     (XtEventHandler) DoButtonPress, (XtPointer)this);
   XtAddEventHandler(viewDA, Button1MotionMask, False,
                     (XtEventHandler) DoButtonMotion, (XtPointer)this);
   XtAddEventHandler(viewDA, ButtonReleaseMask, False,
                     (XtEventHandler) DoButtonRelease, (XtPointer)this);
}


/*-----------------------------------------------------------------------
 *  Destructor
 */
JoyStickC::~JoyStickC()
{
   if ( timer )
      XtRemoveTimeOut(timer);
}


/*-----------------------------------------------------------------------
 *  The handler of the button press.
 */
void
JoyStickC::DoButtonPress(Widget, JoyStickC* This, XButtonEvent* ev, Boolean*)
{
   if ( ev->button != Button1 ) return;

   This->state = GRAB_JOYSTICK;
   This->pos_x = ev->x;
   This->pos_y = ev->y;
   This->Draw();
   This->CallMoveCallbacks();
   This->timer = XtAppAddTimeOut(halApp->context, This->interval,
                                 (XtTimerCallbackProc)TimerProc,
				 (XtPointer)This);
}


/*-----------------------------------------------------------------------
 *  The handler of the button motion.
 */
void
JoyStickC::DoButtonMotion(Widget, JoyStickC *This, XMotionEvent *ev, Boolean *)
{
   This->state = MOVE_JOYSTICK;
//
// Throw the timer away if one is running.
//
   if ( This->timer ) {
      XtRemoveTimeOut(This->timer);
      This->timer = 0;
   }

//
// Throw away extra events in the queue.
//
   XEvent	next;
   int found = 0;
   while ( XCheckTypedWindowEvent(ev->display, ev->window, ev->type, &next) ) {
      found = 1;
   }
   if ( found ) {
      XPutBackEvent(ev->display, &next);
      return;
   }

//
// Redraw the stick.
//
   This->pos_x = ev->x;
   This->pos_y = ev->y;

   if ( This->pos_x < 1 ) {
      This->pos_x = 0;
   }
   else if ( This->pos_x > This->daWd-2 ) {
      This->pos_x = This->daWd-2;
   }
   if ( This->pos_y < 1 ) {
      This->pos_y = 0;
   } else if ( This->pos_y > This->daHt-2 ) {
      This->pos_y = This->daHt-2;
   }

//
// Redraw, call the callbacks and set the timer.
//
   This->Draw();
   This->CallMoveCallbacks();
   This->timer = XtAppAddTimeOut(halApp->context, This->interval,
                                 (XtTimerCallbackProc)TimerProc,
				 (XtPointer)This);
} // End DoButtonMotion


/*-----------------------------------------------------------------------
 *
 */
void
JoyStickC::DoButtonRelease(Widget, JoyStickC* This, XButtonEvent* ev, Boolean*)
{
   if ( ev->button != Button1 ) return;

   This->state = RELEASE_JOYSTICK;
   if ( This->timer )
      XtRemoveTimeOut(This->timer);
   This->timer = 0;
   This->CallMoveCallbacks();

//
// Move the stick back to the middle.
//
   This->pos_x = This->stick_x;
   This->pos_y = This->stick_y;
   This->Draw();
}


/*-----------------------------------------------------------------------
 *
 */
void
JoyStickC::HandleResize(Widget, JoyStickC* This, XEvent*, Boolean*)
{
   Dimension wd, ht;
   XtVaGetValues(This->viewDA, XmNwidth, &wd, XmNheight, &ht, NULL);
   This->daWd    = (int)wd;
   This->daHt    = (int)ht;
   This->stick_x = (int)wd/2;
   This->stick_y = (int)ht/2;

   This->pos_x = This->stick_x;
   This->pos_y = This->stick_y;

   This->Draw();
}


/*-----------------------------------------------------------------------
 * This method is called when the widget is mapped.
 */
void
JoyStickC::HandleMapChange(Widget w, JoyStickC *This, XEvent *ev, Boolean*)
{
   dprintf(("***** Entering HandleMapChange *****\n"));
   if ( ev->type != MapNotify ) return;

//
// We only need to do this once.
//
   XtRemoveEventHandler(w, StructureNotifyMask, False,
		        (XtEventHandler)HandleMapChange, (XtPointer)This);

#ifdef MAYBE_LATER
//
// Create graphics contexts for drawing GSH
//
   XtGCMask	modMask = GCClipMask | GCFillStyle | GCFont | GCFunction
			| GCGraphicsExposures | GCLineStyle | GCLineWidth
			| GCPlaneMask | GCForeground | GCArcMode | GCBackground
			| GCCapStyle | GCClipXOrigin | GCClipYOrigin
			| GCDashList | GCDashOffset | GCFillRule | GCJoinStyle
			| GCStipple | GCSubwindowMode | GCTile
			| GCTileStipXOrigin | GCTileStipYOrigin;
   XGCValues fixVals;
   This->viewGC  = XtAllocateGC(This->viewDA, 0, 0, &fixVals, modMask, 0);
#endif
#ifdef LATER
   XGCValues values;
   XtGCMask  valuemask = 0; // GCBackground | GCForeground; SKB
   if ( lineWidth != 0 ) {
      valuemask |= GCLineWidth;
      values.line_width = lineWidth;
   }

   // XtVaGetValues(viewDA, XmNbackground, &values.background,
		     // XmNforeground, &values.foreground, NULL);

   return XtGetGC(viewDA, valuemask, &values);
#endif
#ifdef LATER
//
// Create a pixmap for off-screen drawing
//
   unsigned depth = DefaultDepth(halApp->display,
				 DefaultScreen(halApp->display));
   This->viewPmWd = dawd;
   This->viewPmHt = daht;
   This->viewPm = XCreatePixmap(halApp->display, This->viewWin,
				This->viewPmWd, This->viewPmHt, depth);
#endif

   This->viewWin = XtWindow(This->viewDA);
   This->viewGC  = DefaultGC(This->viewDSP, 0);


//
// Set the background color of the joystick drawing area.
//
   Dimension wd, ht;
   XtVaGetValues(This->viewDA, XmNbackground, &This->bkgdColor,
                               XmNwidth, &wd, XmNheight, &ht, NULL);
   This->daWd    = (int)wd;
   This->daHt    = (int)ht;
   This->stick_x = (int)wd/2;
   This->stick_y = (int)ht/2;
   This->pos_x   = This->stick_x;
   This->pos_y   = This->stick_y;
   This->Draw();

   dprintf(("***** Leaving HandleMapChange *****\n"));

} // End HandleMapChange


/*-----------------------------------------------------------------------
 * This is the method that actually draws the joy stick.
 */

#define BLACK	BlackPixel(viewDSP, DefaultScreen(viewDSP))
#define WHITE	WhitePixel(viewDSP, DefaultScreen(viewDSP))

void
JoyStickC::Draw()
{
   if ( viewGC ) {

      XClearArea(viewDSP, viewWin, 0, 0, 0, 0, False);

//
// XDrawLine with start and end points the same produces errors with the
//    line attributes we are using.
//
      Boolean adjusted = False;
      if ( stick_x == pos_x && stick_y == pos_y ) pos_y--;
#if 0
      {
	 int	x = stick_x - (stickWidth / (int)2);
	 int	y = stick_y - (stickWidth / (int)2);

	 XSetForeground(viewDSP, viewGC, WHITE);
         XFillArc(viewDSP, viewWin, viewGC, x-1, y-1, stickWidth, stickWidth,
		  0, 360*64);

	 XSetForeground(viewDSP, viewGC, BLACK);
         XFillArc(viewDSP, viewWin, viewGC, x+1, y+1, stickWidth, stickWidth,
		  0, 360*64);

	 XSetForeground(viewDSP, viewGC, stickColor);
         XFillArc(viewDSP, viewWin, viewGC, x, y, stickWidth, stickWidth,
		  0, 360*64);
      }

      else
#endif
      {
	 XSetLineAttributes(viewDSP, viewGC, stickWidth,
			    LineSolid, CapRound, JoinBevel);

	 XSetForeground(viewDSP, viewGC, WHITE);
	 XDrawLine (viewDSP, viewWin, viewGC, stick_x-1, stick_y-1, pos_x-1,
								    pos_y-1);

	 XSetForeground(viewDSP, viewGC, BLACK);
	 XDrawLine (viewDSP, viewWin, viewGC, stick_x+1, stick_y+1, pos_x+1,
								    pos_y+1);

	 XSetForeground(viewDSP, viewGC, stickColor);
	 XDrawLine (viewDSP, viewWin, viewGC, stick_x, stick_y, pos_x, pos_y);
      }

      if ( adjusted ) pos_y++;

   } // End if viewGC

} // End Draw


/*-----------------------------------------------------------------------
 * A method when expose events are called.
 */
void
JoyStickC::DoExpose(Widget, JoyStickC* This, XmDrawingAreaCallbackStruct* cbs)
{
   XExposeEvent *ev = (XExposeEvent*)cbs->event;
   if ( ev->count > 0 ) return;
   This->Draw();
}


/*-----------------------------------------------------------------------
 *
 */
void
JoyStickC::TimerProc(JoyStickC *This, XtIntervalId*)
{
   This->CallMoveCallbacks();
//
// Add this timeout again.
//
   This->timer = XtAppAddTimeOut(halApp->context, This->interval,
                                 (XtTimerCallbackProc)TimerProc,
				 (XtPointer)This);
}


/*-----------------------------------------------------------------------
 *
 */
void
JoyStickC::GetMovePercentage(int *x, int *y)
{
   *x = ((pos_x - stick_x)*100)/(daWd/2);
   *y = ((pos_y - stick_y)*100)/(daHt/2);
}

// EOF JoyStickC.C
