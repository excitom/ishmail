/*
 * $Id: HalAppC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
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

#ifndef _HalAppC_h_
#define _HalAppC_h_

#include "StringC.h"
#include "CallbackListC.h"
#include "PtrListC.h"

#include <X11/Intrinsic.h>
#include <Xm/Xm.h>

class HalShellC;
class WorkingBoxC;
class HelpC;

//
// This type is used to send data to an exit callback
//
typedef struct {

   Boolean	interrupted;	// True if ctrl-c caused this
   Boolean	cancelExit;	// If anybody sets this to true, we won't exit

} HalExitDataT;

//
// Class definition
//

class HalAppC {

public:

   CallbackListC	exitCalls;
   PtrListC		shellList;	// List of HalShellC's

   Cursor		busyCursor;	// Watch cursor
   int			busyCount;
   Boolean		busyOn;

   Widget		msgDialog;
   WorkingBoxC		*workBox;	// Working dialog
   HelpC		*helpWin;	// On-line help object
   Boolean		hasHelp;	// True if help available
   StringC		className;	// Name of application class

//
// Handler for signals
//
   static void		HandleInterrupt(void*, void*);

//
// Handlers for X errors
//
   static void		HandleXError(Display*, XErrorEvent*);
   static void		HandleXIOError(Display*);

   StringC		name;		// Name of application
   XtAppContext		context;	// X application context
   Display		*display;	// X display
   int			screenNum;	// Screen number for display
   Screen		*screen;	// Screen pointer for display
   GC			gc;		// Default GC of screen
   XFontStruct		*font;		// Default font structure of GC
   Font			fid;		// Default font id of GC
   Widget		appShell;	// Application shell (not displayed)
   Atom			delWinAtom;
   Pixmap		messagePM;	// Application specified pixmap for
					//  XmDIALOG_MESSAGE or
					//  XmDIALOG_INFORMATIONAL PopupMessage
   Pixmap		errorPM;	// Application specified pixmap for
					//    XmDIALOG_ERROR PopupMessage
   Pixmap		warningPM;	// Application specified pixmap for
					//    XmDIALOG_WARNING PopupMessage
   Pixmap		questionPM;	// Application specified pixmap for
					//    XmDIALOG_QUESTION PopupMessage

   Cursor		helpCursor;	// Question-mark cursor
   Boolean		quickHelpEnabled;
   Boolean		xRunning;	// True if server is up
   int			argCount;	// Number of command line arguments
   char			**argVec;	// Command line arguments
   int			exitSignal;	// If an interrupt was received

//
// This constructor initializes X
//
   HalAppC(int *argc, char **argv, const char *name, const char *clss,
	   String *fallbackResources=NULL, 
	   XrmOptionDescRec *options=NULL, Cardinal numoptions=0);
   virtual ~HalAppC();

//
// Turn busy cursor on or off
//
   void			BusyCursor(Boolean);


//
// Get the working box
//
   WorkingBoxC*		WorkBox();
   HelpC*		HelpWin();	// On-line help object
   Boolean		HasHelp();

//
// Turn quick help on or off
//
   void			QuickHelp(Boolean);

//
// Display string in popup message window
//
   void			PopupMessage(const char*,
                                     unsigned char type=XmDIALOG_ERROR);
   void			PopupMessage(const char*, Widget,
                                     unsigned char type=XmDIALOG_ERROR);

//
// Main application loop
//
   inline void		MainLoop() { XtAppMainLoop(context); }

//
// Add/remove shells to/from the list of known shells
//
   void			RegisterShell(HalShellC*);
   void			UnregisterShell(HalShellC*);

//
// Manage exit callbacks
//
   inline void	AddExitCallback(CallbackFn *fn, void *data) {
      AddCallback(exitCalls, fn, data);
   }
   inline void	RemoveExitCallback(CallbackFn *fn, void *data) {
      RemoveCallback(exitCalls, fn, data);
   }

//
// Help menu callbacks
//
   static void		DoExit       (Widget, Boolean, XtPointer);
   static void		DoHelp       (Widget, XtPointer, XtPointer);
   static void		DoIndexHelp  (Widget, XtPointer, XtPointer);

//
// Casts
//
   inline operator	Widget() const		{ return appShell; }
   inline operator	Window() const		{ return XtWindow(appShell); }
};

extern HalAppC	*halApp;

#endif // _HalAppC_h_
