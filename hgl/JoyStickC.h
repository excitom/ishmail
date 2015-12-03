/*
 *  $Id: JoyStickC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
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

#ifndef _JoyStickC_h_
#define _JoyStickC_h_

#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <Xm/Xm.h>

#include "CallbackListC.h"

#define GRAB_JOYSTICK		1
#define MOVE_JOYSTICK		2
#define RELEASE_JOYSTICK	3

class JoyStickC {
   private:
      int               pos_x;
      int               pos_y;
      XtIntervalId      timer;
      int               interval;
      int               daWd;
      int               daHt;
      int               state;

      Widget		viewDA;
      GC		viewGC;
      Window		viewWin;
      Display	       *viewDSP;

      Pixel		bkgdColor;
      Pixel		stickColor;
      int		stickWidth;
      int		stick_x;
      int		stick_y;
      CallbackListC	moveCalls;

      void Draw();

      inline void  CallMoveCallbacks() {
 	 CallCallbacks(moveCalls, this);
      }

      static void TimerProc(JoyStickC*, XtIntervalId*);
      static void HandleResize(Widget, JoyStickC*, XEvent*, Boolean*);
      static void HandleMapChange(Widget, JoyStickC*, XEvent*, Boolean*);
      static void DoButtonPress(Widget, JoyStickC*, XButtonEvent*, Boolean*);
      static void DoButtonMotion(Widget, JoyStickC*, XMotionEvent*, Boolean*);
      static void DoButtonRelease(Widget, JoyStickC*, XButtonEvent*, Boolean*);
      static void DoExpose(Widget, JoyStickC*, XmDrawingAreaCallbackStruct*);

   public:
      JoyStickC(Widget, char *nm = "joyStick", ArgList argv=NULL, Cardinal argc=0);
     ~JoyStickC();

      void GetMovePercentage(int*, int*);

      inline void  AddMoveCallback(CallbackFn *fn, void *data) {
 	 AddCallback(moveCalls, fn, data);
      }
      inline void  RemoveMoveCallback(CallbackFn *fn, void *data) {
 	 RemoveCallback(moveCalls, fn, data);
      }
      inline operator Widget()  const { return viewDA; }
      int State() { return(state); }
};

# endif
