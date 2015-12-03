/*
 * $Id: WorkingBoxC.C,v 1.2 2000/05/07 12:26:11 fnevgeny Exp $
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

#include "HalAppC.h"
#include "WorkingBoxC.h"
#include "WXmString.h"
#include "ForceDialog.h"
#include "WorkingXbm.h"
#include "WArgList.h"
#include "rsrc.h"

#include <Xm/MessageB.h>
#include <Xm/Form.h>
#include <Xm/Scale.h>
#include <Xm/Label.h>
#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>

/*---------------------------------------------------------------
 *  Constructor
 */

WorkingBoxC::WorkingBoxC(Widget parent)
{

//
// Create dialog
//
   WArgList	args;
   realParent = parent;

   args.Reset();
   args.AutoUnmanage(False); 
   dialog = XmCreateWorkingDialog(parent, "workingDialog", ARGS);
   shell = XtParent(dialog);
   XtVaSetValues(dialog, XmNautoUnmanage, True, NULL);
   symbol = XmMessageBoxGetChild(dialog, XmDIALOG_SYMBOL_LABEL);

//
// Add scale
//
   scaleForm = XmCreateForm(dialog, "scaleForm", 0,0);

   args.DecimalPoints(0);
   args.Sensitive(False);
   args.HighlightOnEnter(False);
   args.HighlightThickness(0);
   args.Maximum(100);
   args.Minimum(0);
   args.Orientation(XmHORIZONTAL);
   args.Value(0);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.TopAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_NONE);
   scale = XmCreateScale(scaleForm, "scale", ARGS);
   XtManageChild(scale);

   args.Reset();
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_POSITION, 50);
   args.TopAttachment(XmATTACH_WIDGET, scale);
   args.BottomAttachment(XmATTACH_NONE);
   args.Alignment(XmALIGNMENT_BEGINNING);
   Widget	startLabel = XmCreateLabel(scaleForm, "startLabel", ARGS);
   XtManageChild(startLabel);

   args.Reset();
   args.LeftAttachment(XmATTACH_POSITION, 50);
   args.RightAttachment(XmATTACH_FORM);
   args.TopAttachment(XmATTACH_WIDGET, scale);
   args.BottomAttachment(XmATTACH_NONE);
   args.Alignment(XmALIGNMENT_END);
   Widget	endLabel   = XmCreateLabel(scaleForm, "endLabel", ARGS);
   XtManageChild(endLabel);

//
// Remove unnecessary buttons
//
   XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_OK_BUTTON));
   XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));

//
// Add callback for cancel
//
   XtAddCallback(dialog, XmNcancelCallback, (XtCallbackProc)Cancel,
                 (XtPointer)this);

//
// Initialize variables
//
   cancel  = False;
   pixmaps = NULL;
   pixnum  = 0;
   gc      = NULL;

//
// Get the resource to disable key press activates
//
   allowKeyPress = get_boolean("WorkingBoxC", XtParent(dialog), 
			       "allowKeyPress", True);

//
// Create pixmaps when window pops up
//
   XtAddCallback(shell, XmNpopupCallback, (XtCallbackProc)Popup,
		 (XtPointer)this);

   return;

} // End WorkingBoxC::Constructor

/*---------------------------------------------------------------
 *  Destructor
 */

WorkingBoxC::~WorkingBoxC()
{

//
// Delete pixmaps
//
   if ( pixmaps ) {
      if ( halApp->xRunning ) {
	 for (int i=0; i<WORKING_PIXMAP_COUNT; i++)
	    XFreePixmap(halApp->display, pixmaps[i]);
      }
      delete[] pixmaps;
   }

   if ( halApp->xRunning ) {
#ifndef NO_COPY_AREA
      if ( gc ) XtReleaseGC(dialog, gc);
#endif

      XtDestroyWidget(dialog);
   }

} // End WorkingBoxC::Destructor

void
WorkingBoxC::Popup(Widget, WorkingBoxC *wb, XtPointer)
{
//
// Don't allow resizing
//
   Dimension	wd, ht;
   XtVaGetValues(*wb, XmNwidth, &wd, XmNheight, &ht, NULL);
   XtVaSetValues(*wb, XmNminWidth, wd, XmNminHeight, ht,
		      XmNmaxWidth, wd, XmNmaxHeight, ht, NULL);

//
// Create pixmaps
//
   wb->drawable = XtWindow((Widget)*wb);
   unsigned int	depth = DefaultDepthOfScreen(XtScreen((Widget)*wb));
   Pixel	fg, bg;
   XtVaGetValues(*wb, XmNforeground, &fg, XmNbackground, &bg, NULL);

   wb->pixmaps = new Pixmap[WORKING_PIXMAP_COUNT];
   for (int i=0; i<WORKING_PIXMAP_COUNT; i++) {
      PixmapDataT	*data = &pixmapData[i];
      wb->pixmaps[i] =
	 XCreatePixmapFromBitmapData(halApp->display, wb->drawable,
				     (char *) data->bits,
				     data->width, data->height, fg, bg, depth);
//
// Get the size of the pixmaps (assume all the same size)
//
      if (!i) {
	wb->pixmap_w = data->width;
	wb->pixmap_h = data->height;
      }
   }

   XtVaSetValues(*wb, XmNsymbolPixmap, wb->pixmaps[0], NULL);
   wb->pixnum = 0;
#ifndef NO_COPY_AREA
   Dimension hlt, st, mh, mw, ml, mt;
   XtVaGetValues(wb->symbol, XmNx, &wb->pixmap_x, XmNy, &wb->pixmap_y,
		 XmNhighlightThickness, &hlt, XmNshadowThickness, &st,
		 XmNmarginHeight, &mh, XmNmarginWidth, &mw, 
		 XmNmarginLeft, &ml, XmNmarginTop, &mt, NULL);
   wb->pixmap_x += hlt + st + mw + ml;
   wb->pixmap_y += hlt + st + mh + mt;

//
// Create a graphics context for drawing
//
   XGCValues    vals;
   wb->gc = XtGetGC(*wb, 0, &vals);
#if 0
   wb->gc = XCreateGC(halApp->display, wb->drawable, 0, NULL);
#endif

#endif

//
// This callback is no longer needed
//
   XtRemoveCallback(wb->shell, XmNpopupCallback, (XtCallbackProc)Popup,
		    (XtPointer)wb);

} // End WorkingBoxC::Popup

/*---------------------------------------------------------------
 *  Method used to set the message
 */

static Boolean keepWaiting;

static void
WaitForHalfSecond(XtPointer, XtIntervalId*)
{
  keepWaiting = False;
}

void
WorkingBoxC::Message(const char *msg)
{
   keepWaiting = True;

   WXmString	str(msg);
   XtVaSetValues(dialog, XmNmessageString, (XmString)str, NULL);
   XtAppAddTimeOut(halApp->context, 500, WaitForHalfSecond, NULL);

   while (keepWaiting) {
     UpdateDisplay();
   }
}

/*---------------------------------------------------------------
 *  Method used to display the box
 */

void
WorkingBoxC::Show(Widget parent)
{
//
// Set the dialog's parent so it pops up in the desired place
//
   shell->core.parent = parent;
   XtVaSetValues(shell, XmNtransientFor, parent, NULL);

   XtManageChild(dialog);
   XMapRaised(halApp->display, XtWindow(shell));
   ForceDialog(dialog);

//
// Display the first pixmap
//
#ifdef NO_COPY_AREA
   XtVaSetValues(dialog, XmNsymbolPixmap, pixmaps[0], NULL);
#else
   XCopyArea(halApp->display, pixmaps[0], drawable, gc,
             /*srcx*/0, /*srcy*/0, pixmap_w, pixmap_h,
             /*dstx*/pixmap_x, /*dsty*/pixmap_y);
#endif

   pixnum = 0;
   cancel = False;
}

void
WorkingBoxC::Show()
{
   Show(realParent);
}

/*---------------------------------------------------------------
 *  Method used to display the box
 */

void
WorkingBoxC::Hide()
{
   XtUnmanageChild(dialog);
}

/*---------------------------------------------------------------
 *  Callback used to cancel operations in progress
 */

void
WorkingBoxC::Cancel(Widget, WorkingBoxC *wb, XtPointer eventPtr)
{
   if (!wb->allowKeyPress) {
     XmPushButtonCallbackStruct *pbcs = (XmPushButtonCallbackStruct *)eventPtr;
     if (pbcs->event->xany.type == KeyPress) return;
   }
   wb->cancel = True;
   wb->Hide();
}

/*---------------------------------------------------------------
 *  Method used to see if operation was cancelled
 */

Boolean
WorkingBoxC::Cancelled()
{
   Boolean return_val = False;
   if ( XtIsManaged(dialog) ) {

//
// Change the displayed pixmap
//
      if ( ++pixnum >= WORKING_PIXMAP_COUNT )
	 pixnum = 0;

#ifdef NO_COPY_AREA
      XtVaSetValues(dialog, XmNsymbolPixmap, pixmaps[pixnum], NULL);
#else
      XCopyArea(halApp->display, pixmaps[pixnum], drawable, gc,
		/*srcx*/0, /*srcy*/0,
		pixmap_w, pixmap_h, /*dstx*/pixmap_x, /*dsty*/pixmap_y);
#endif

//
// Process outstanding events
//
      UpdateDisplay();
      if (cancel) {
	return_val = True;
	cancel = False;
      }

   } // End if dialog managed

   return return_val;

} // End Cancelled

/*---------------------------------------------------------------
 *  Method used to keep display current
 */

void
WorkingBoxC::UpdateDisplay()
{
//
// Process outstanding events destined for this window
//
   XSync(halApp->display, False);

   XEvent	event;
   while ( XCheckWindowEvent(halApp->display, XtWindow(dialog), -1, &event) )
      XtDispatchEvent(&event);

//
// Flush other button and key events
//
   long eMask = KeyPressMask|KeyReleaseMask|ButtonPressMask|ButtonReleaseMask;
   while ( XCheckMaskEvent(halApp->display, eMask, &event) );

//
// Run other display events
//
   while ( XCheckMaskEvent(halApp->display, ~eMask, &event) )
     XtDispatchEvent(&event);

//
// Handle timers and other events
//
   XtInputMask	mask = XtIMTimer | XtIMAlternateInput;
   while ( XtAppPending(halApp->context) & mask )
      XtAppProcessEvent(halApp->context, mask);

} // End UpdateDisplay

/*---------------------------------------------------------------
 *  Methods to control display of scale
 */

void
WorkingBoxC::ShowScale()
{
   XtManageChild(scaleForm);
}

void
WorkingBoxC::HideScale()
{
   XtUnmanageChild(scaleForm);
}

void
WorkingBoxC::SetScaleValue(int val)
{
   XmScaleSetValue(scale, val);
}

void
WorkingBoxC::UnmanageButton()
{
  XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON));
}

void
WorkingBoxC::ManageButton()
{
  XtManageChild(XmMessageBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON));
}

