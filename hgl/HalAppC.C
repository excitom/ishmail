/*
 * $Id: HalAppC.C,v 1.11 2001/03/12 20:40:38 evgeny Exp $
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

#include "HalAppC.h"
#include "HalShellC.h"
#include "HglResources.h"
#include "WArgList.h"
#include "WXmString.h"
#include "WorkingBoxC.h"
#include "SignalRegistry.h"
#include "MimeRichTextC.h"
#include "rsrc.h"
#include "HelpC.h"
#include "CharC.h"

#include <Xm/RepType.h>
#include <Xm/AtomMgr.h>
#include <Xm/Protocols.h>
#include <Xm/DragDrop.h>
#include <Xm/Screen.h>
#include <Xm/Text.h>
#include <Xm/MessageB.h>

#include <X11/cursorfont.h>
#include <X11/Xmu/Converters.h>
#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>

#ifdef WITH_EDITRES
#  include <X11/Xmu/Editres.h>
#endif

#include <signal.h>
#include <errno.h>
#include <fcntl.h>

HalAppC	*halApp = NULL;	// Global application object

int	debug1   = 0;
int	debug2   = 0;
int	debuglev = 0;

#if defined(AIX) || defined(HPUX)
Boolean	memMapOk = False;	// AIX keeps files open and we run out.  It
				//    simply doesn't work well on HP.
#else
Boolean	memMapOk = True;
#endif

static char	**origArgv;
static int	  origArgc;

static void
DoSaveYourself(Widget, HalAppC*, XtPointer)
{
//
// Exit the current application
//
   if ( debuglev > 0 ) cout <<"Got WM_SAVE_YOURSELF message" <<endl;

//
// Ignore terminating signals
//
   SIG_PF	oldhup  = signal(SIGHUP,  SIG_IGN);
   SIG_PF	oldint  = signal(SIGINT,  SIG_IGN);
   SIG_PF	oldterm = signal(SIGTERM, SIG_IGN);

//
// Loop through exit callbacks from the back to the front.  This will
// approximate the C++ destoy methods.
//
   HalExitDataT		data;
   data.cancelExit  = False;
   data.interrupted = True;

   CallbackListC	tmpCalls = halApp->exitCalls; // In case a callback
						      // removes itself
   u_int	count = halApp->exitCalls.size();
   if ( debuglev > 0 ) cout <<"   Found " <<count <<" exit callbacks" <<endl;
   for (int i=count-1; i>=0; i--) {
      if ( debuglev > 0 ) cout <<"   Calling callback " <<i <<endl;
      (*tmpCalls[i])(&data);
   }

//
// Do not delete the followers here.  Since you have received the SaveYourself
// atom, assume that the other HGL items will receive it as well.
//

//
// Set the command to be used next time
//
   XSetCommand(halApp->display, (Window)*halApp, origArgv, origArgc);

   if ( debuglev > 0 ) cout <<"   Goodbye" <<endl;
   exit(0);
}


//
// DJL - static callbacks used to handle shutdowns of related HGL-based
//       applications.  The "leader" application will have to ensure that
//	 all the "follower" applications are started with the environment
//	 variable HGL_LEAD_WIN set to the window of halApp.
//

static PtrListC windowList;
static Atom hglPropAtom;
static Atom cancelAtom;
static Atom windowAtom;

static void
StopFollowers()
{
  Window root, parent;
  Window *children;
  unsigned int num_children = 0;

  XQueryTree(halApp->display, RootWindowOfScreen(halApp->screen), &root,
	     &parent, &children, &num_children);

  for (int i = 0; i < windowList.size(); i++) {
    Window sendWin = (Window)(*windowList[i]);
    for (int j = 0; j < num_children; j++) {
      if (sendWin == children[j]) {
	XClientMessageEvent clientMsgEvent;
	clientMsgEvent.type = ClientMessage;
	clientMsgEvent.window = sendWin;
	clientMsgEvent.message_type = hglPropAtom;
	clientMsgEvent.format = 32;
	clientMsgEvent.data.l[0] = cancelAtom;
	XSendEvent(halApp->display, sendWin, False, NoEventMask,
		   (XEvent*)&clientMsgEvent);
	XFlush(halApp->display);
      }
    }
  }
  windowList.removeAll();
  XFree(children);
} // End DoStopFollowers

static void
DoAddFollower(Widget, XtPointer, XtPointer call_data)
{
  XmAnyCallbackStruct *acs = (XmAnyCallbackStruct*)call_data;
  XClientMessageEvent *event = (XClientMessageEvent*)(acs->event);
  Window newWin = event->data.l[1];
  windowList.append((PtrT)newWin);
} 

static void
InformLeader()
{
  char *leadWinStr = getenv("HGL_LEAD_WINDOW");
  if (leadWinStr == NULL) {
    return;
  }
  Window leadWin = atol(leadWinStr);
  if (leadWin != 0) {
    XClientMessageEvent clientMsgEvent;
    clientMsgEvent.type = ClientMessage;
    clientMsgEvent.window = leadWin;
    clientMsgEvent.message_type = hglPropAtom;
    clientMsgEvent.format = 32;
    clientMsgEvent.data.l[0] = windowAtom;
    clientMsgEvent.data.l[1] = (Window)(*halApp);
    XSendEvent(halApp->display, leadWin, False, NoEventMask,
	       (XEvent*)&clientMsgEvent);
    XFlush(halApp->display);
  }
} // End InformLeader

/*----------------------------------------------------------------------
 * Initialize X
 */

HalAppC::HalAppC(int *argc, char **argv, const char *_name, const char *clss,
		 String *fallbackResources,
                 XrmOptionDescRec *options, Cardinal numoptions)
{
//
// Save original arguments
//
   origArgc = *argc;
   origArgv = new char*[origArgc];
   int	i;
   for (i=0; i<origArgc; i++) {
      int	len = strlen(argv[i]);
      origArgv[i] = new char[len+1];
      strcpy(origArgv[i], argv[i]);
   }

   xRunning   = False;
   halApp     = this;
   name       = _name;
   helpCursor = (Cursor)NULL;
   helpWin    = NULL;
   hasHelp    = False;
   workBox    = NULL;
   className  = clss;
   exitSignal = 0;

#if XtSpecificationRelease > 4
   XtSetLanguageProc(NULL, NULL, NULL);
#endif

   XtToolkitInitialize();
   context = XtCreateApplicationContext();

//
// Load the fallback default resources.
//
   if ( !fallbackResources ) {

      XtAppSetFallbackResources(context, HglFallbackResources);

   } else {
//
//    Since we can only set the fallback resources 1 time we
//    need to concatenate the user's resources onto the hgl resources.
//
      String *str;
      int num_hgl = 0;
      int num_usr = 0;
      for ( str=HglFallbackResources; *str!=NULL; str++, num_hgl++ );
      for ( str=fallbackResources;    *str!=NULL; str++, num_usr++ );

      String *resources = new String[num_usr+num_hgl+1];
      int j, i=0;
      for (j=0; j<num_hgl; j++) resources[i++] = HglFallbackResources[j];
      for (j=0; j<num_usr; j++) resources[i++] = fallbackResources[j];
      resources[i] = NULL;

      XtAppSetFallbackResources(context, resources);

   } // End if user supplied resources

//
// Create the display.
//
   display = XtOpenDisplay(context, NULL, name, clss, options, numoptions, 
			   argc, argv);
   if ( display == NULL) {
      cerr << name << ":  Can't open display\n";
      exit(1);
   }
   if ( debuglev > 0 ) cout <<"Display opened" <<endl;

//
// Save command line arguments that weren't removed by X.
// Look for debug arguments and remove them.
//
   argCount = *argc - 1;	// Don't copy argv[0]
   argVec   = NULL;
   if ( argCount > 0 ) {

      argVec = new char*[argCount];
      char	**arg = argv+1;
      int	j = 0;
      for (i=0; i<argCount; i++) {

	 if ( strcmp(*arg, "-D") == 0 ) {	// Enable debugging
	    if ( i < argCount - 1 ) {
               i++;
	       arg++;
	       debuglev = atoi(*arg);
            } else {
               cerr << name << ":  -D implies exactly one argument\n";
               exit(1);
            }
	    
            debug1 = debuglev >= 1;
	    debug2 = debuglev >= 2;
	 }

	 else {
	    argVec[j] = new char[strlen(*arg)+1];
	    strcpy(argVec[j], *arg);
	    j++;
	 }
	 arg++;
      }

      argCount = j;
   }

   xRunning = True;

//
// Make sure display connection closes during forks
//
   fcntl(ConnectionNumber(display), F_SETFD, 1);

//
// Add error handlers
//
   XSetErrorHandler((XErrorHandler)HandleXError);
   XSetIOErrorHandler((XIOErrorHandler)HandleXIOError);

//
// Add converters
//
   XmRepTypeInstallTearOffModelConverter();
   XtAppAddConverter(context, XtRString, XtRGravity, XmuCvtStringToGravity,0,0);

//
// Initialize resource database
//
   XrmInitialize();

//
// Create shell
//
   WArgList	args;
   args.Reset();
   args.MappedWhenManaged(False);
   args.X((DisplayWidth(display,0))/2);
   args.Y((DisplayHeight(display,0))/2);
   args.Width(1);
   args.Height(1);
   if ( debuglev > 0 ) cout <<"Creating app shell" <<endl;
   appShell = XtAppCreateShell(name, clss,
			       applicationShellWidgetClass, display, ARGS);
//
// EditRes
//
#ifdef WITH_EDITRES
   if ( debuglev > 0 ) cout <<"Registering EditRes protocol" <<endl;
   XtAddEventHandler(appShell, (EventMask) 0, True,
                        _XEditResCheckMessages, NULL);
#endif

   XtRealizeWidget(appShell);

//
// See if memory mapping, help editing or private colors is requested
//
   Boolean memMapOkRes   = get_boolean(appShell, "memoryMappingOk", memMapOk);
   Boolean editHelp      = get_boolean(appShell, "editHelp",        False);
   Boolean privateColors = get_boolean(appShell, "privateColors",   False);
   memMapOk = memMapOkRes;
   CharC	argStr;
   for (i=0; i<argCount; i++) {

      argStr = argVec[i];
      if ( argStr.StartsWith('-') ) argStr.CutBeg(1);
      if ( argStr.Equals("noMemMap", IGNORE_CASE) ) {

	 memMapOk = False;

//
// Remove this argument from the list
//
	 delete argVec[i];
	 for (int j=i+1; j<argCount; j++) argVec[j-1] = argVec[j];
	 argCount--;
	 i--;
      }

      else if ( argStr.Equals("editHelp", IGNORE_CASE) ) {

	 editHelp = True;

//
// Remove this argument from the list
//
	 delete argVec[i];
	 for (int j=i+1; j<argCount; j++) argVec[j-1] = argVec[j];
	 argCount--;
	 i--;
      }

      else if ( argStr.Equals("privateColors", IGNORE_CASE) ) {

	 privateColors = True;

//
// Remove this argument from the list
//
	 delete argVec[i];
	 for (int j=i+1; j<argCount; j++) argVec[j-1] = argVec[j];
	 argCount--;
	 i--;
      }
   }

//
// Get default values
//
   screenNum = DefaultScreen(display);
   screen    = DefaultScreenOfDisplay(display);
   gc        = DefaultGC(display, screenNum);
   font      = XQueryFont(display, XGContextFromGC(gc));

   unsigned long	mask;
   XGCValues	vals;
   mask = GCFont;
   XGetGCValues(display, gc, mask, &vals);

   fid = vals.font;

//
// See if we need to create our own colormap
//
   if ( privateColors ) {
      Colormap	privMap =
         XCopyColormapAndFree(display, XDefaultColormapOfScreen(screen));
      XtVaSetValues(appShell, XmNcolormap, privMap, NULL);
   }

//
// Catch terminating signals
//
   AddSignalCallback(SIGHUP,  (CallbackFn*)HandleInterrupt, NULL);
   AddSignalCallback(SIGINT,  (CallbackFn*)HandleInterrupt, NULL);
   AddSignalCallback(SIGTERM, (CallbackFn*)HandleInterrupt, NULL);

//
// Add a signal handler for SIGPIPE.  We can get this if the X server goes
//    down just before a write attempt.
//
   AddSignalCallback(SIGPIPE, (CallbackFn*)HandleInterrupt, NULL);

#if 0
//
// Temporary
//
   AddSignalCallback(SIGQUIT, (CallbackFn*)HandleInterrupt, NULL);
   AddSignalCallback(SIGILL,  (CallbackFn*)HandleInterrupt, NULL);
   AddSignalCallback(SIGTRAP, (CallbackFn*)HandleInterrupt, NULL);
   AddSignalCallback(SIGIOT,  (CallbackFn*)HandleInterrupt, NULL);
   AddSignalCallback(SIGABRT, (CallbackFn*)HandleInterrupt, NULL);
   AddSignalCallback(SIGEMT,  (CallbackFn*)HandleInterrupt, NULL);
   AddSignalCallback(SIGFPE,  (CallbackFn*)HandleInterrupt, NULL);
   AddSignalCallback(SIGBUS,  (CallbackFn*)HandleInterrupt, NULL);
   AddSignalCallback(SIGSEGV, (CallbackFn*)HandleInterrupt, NULL);
   AddSignalCallback(SIGSYS,  (CallbackFn*)HandleInterrupt, NULL);
   AddSignalCallback(SIGALRM, (CallbackFn*)HandleInterrupt, NULL);
   AddSignalCallback(SIGURG,  (CallbackFn*)HandleInterrupt, NULL);
   AddSignalCallback(SIGSTOP, (CallbackFn*)HandleInterrupt, NULL);
   AddSignalCallback(SIGTSTP, (CallbackFn*)HandleInterrupt, NULL);
   AddSignalCallback(SIGCONT, (CallbackFn*)HandleInterrupt, NULL);
   //AddSignalCallback(SIGCHLD, (CallbackFn*)HandleInterrupt, NULL);
   AddSignalCallback(SIGTTIN, (CallbackFn*)HandleInterrupt, NULL);
   AddSignalCallback(SIGTTOU, (CallbackFn*)HandleInterrupt, NULL);
   AddSignalCallback(SIGIO,   (CallbackFn*)HandleInterrupt, NULL);
   AddSignalCallback(SIGXCPU, (CallbackFn*)HandleInterrupt, NULL);
   AddSignalCallback(SIGXFSZ, (CallbackFn*)HandleInterrupt, NULL);
   AddSignalCallback(SIGVTALRM, (CallbackFn*)HandleInterrupt, NULL);
   AddSignalCallback(SIGPROF, (CallbackFn*)HandleInterrupt, NULL);
   AddSignalCallback(SIGWINCH, (CallbackFn*)HandleInterrupt, NULL);
   AddSignalCallback(SIGLOST, (CallbackFn*)HandleInterrupt, NULL);
   AddSignalCallback(SIGUSR1, (CallbackFn*)HandleInterrupt, NULL);
   AddSignalCallback(SIGUSR2, (CallbackFn*)HandleInterrupt, NULL);
#endif

//
// Create busy cursor
//
   busyCursor = XCreateFontCursor(display, XC_watch);
   busyCount = 0;
   busyOn = False;

   Boolean	hasDnD = get_boolean(appShell, "hasDragAndDrop", True);

   if ( hasDnD ) {

//
// Create icons for drag and drop states
//
      Widget
#if (XmVersion < 2000) || (XmVersion == 2000 && XmUPDATE_LEVEL == 0)
	 validDropIcon   = XmCreateDragIcon(appShell, "validDropIcon",   0,0),
	 invalidDropIcon = XmCreateDragIcon(appShell, "invalidDropIcon", 0,0),
	 noDropIcon      = XmCreateDragIcon(appShell, "noDropIcon",      0,0);
#else
	 validDropIcon   = XmCreateDragIcon(appShell, "validDropIcon2",   0,0),
	 invalidDropIcon = XmCreateDragIcon(appShell, "invalidDropIcon2", 0,0),
	 noDropIcon      = XmCreateDragIcon(appShell, "noDropIcon2",      0,0);
#endif
//
// Store icons with screen
//
      Widget	xmscreen = XmGetXmScreen(XtScreen(appShell));
      XtVaSetValues(xmscreen, XmNdefaultValidCursorIcon,   validDropIcon,
			      XmNdefaultInvalidCursorIcon, invalidDropIcon,
			      XmNdefaultNoneCursorIcon,    noDropIcon,
			      NULL);
   }

//
// Create atom used for deleting windows
//
   delWinAtom = XmInternAtom(display, "WM_DELETE_WINDOW", False);

   exitCalls.AllowDuplicates(TRUE);
   shellList.AllowDuplicates(FALSE);
   quickHelpEnabled = get_boolean(appShell, "showQuickHelp", True);
   msgDialog = NULL;
   messagePM  =
   errorPM    =
   questionPM =
   warningPM  = (Pixmap)NULL;
   helpWin    = NULL;

//
// Add callback to detect VUE and CDE desktop termination
//
   Atom	saveAtom = XmInternAtom(display, "WM_SAVE_YOURSELF", False);
   XmAddWMProtocols(appShell, &saveAtom, 1);
   XmAddWMProtocolCallback(appShell, saveAtom, (XtCallbackProc)DoSaveYourself,
			   this);

//
// DJL - add a protocol which will allow the program to be shut down using
// interclient communication.  This should be put in place of using SIGTERM.
// It will be up to the applications to handle getting the value of the
// appShell between applications.
//
   hglPropAtom = XmInternAtom(display, "_HGL_PROPERTY", False);
   if (debuglev > 0) cout << "Interned hglPropAtom successfully" << endl;

   cancelAtom = XmInternAtom(display, "_HGL_CANCEL", False);
   if (debuglev > 0) cout << "Interned cancelAtom successfully" << endl;

   windowAtom = XmInternAtom(display, "_HGL_WINDOW", False);
   if (debuglev > 0) cout << "Interned windowAtom successfully" << endl;

   XmAddProtocols(appShell, hglPropAtom, &cancelAtom, 1);
   XmAddProtocolCallback(appShell, hglPropAtom, cancelAtom, 
			 (XtCallbackProc)DoExit, (caddr_t)True);
   if (debuglev > 0) cout << "Added callback for cancelAtom" << endl;

   XmAddProtocols(appShell, hglPropAtom, &windowAtom, 1);
   XmAddProtocolCallback(appShell, hglPropAtom, windowAtom, 
			 (XtCallbackProc)DoAddFollower, (caddr_t)True);
   if (debuglev > 0) cout << "Added callback for windowAtom" << endl;

   InformLeader();
   if (debuglev > 0) cout << "Informed the leader window" << endl;

//
// Initialize help system
//
   if ( debuglev > 0 ) cout <<"Initializing help" <<endl;
   helpWin = new HelpC(editHelp);
   hasHelp = helpWin->isActive;
   if ( hasHelp )
      helpCursor = XCreateFontCursor(display, XC_question_arrow);

} // End HalAppC constructor

/*----------------------------------------------------------------------
 * Destructor
 */

HalAppC::~HalAppC()
{
// Remove the protocols just to be sure.
   XmRemoveProtocols(appShell, hglPropAtom, &cancelAtom, 1);
   XmRemoveProtocolCallback(appShell, hglPropAtom, cancelAtom,
                         (XtCallbackProc)DoExit, (caddr_t)True);
   if (debuglev > 0) cout << "Removed callback for cancelAtom" << endl;

   XmRemoveProtocols(appShell, hglPropAtom, &windowAtom, 1);
   XmRemoveProtocolCallback(appShell, hglPropAtom, windowAtom,
                         (XtCallbackProc)DoAddFollower, (caddr_t)True);
   if (debuglev > 0) cout << "Removed callback for windowAtom" << endl;

// Delete callback structures

   if ( debuglev > 0 ) cout <<"Deleting exit calls" <<endl;
   DeleteCallbacks(exitCalls);

#ifndef NO_HELP
   if ( debuglev > 0 ) cout <<"Deleting help" <<endl;
   delete helpWin;
#endif

   if ( workBox ) delete workBox;

   if ( xRunning ) {
      if ( debuglev > 0 ) cout <<"Freeing X resources" <<endl;
      // Don't do this even though purify reports a memory leak
      // BadFont errors will result
      // XFreeFont(display, font);
      Colormap	map;
      XtVaGetValues(appShell, XmNcolormap, &map, NULL);
      if ( map != XDefaultColormapOfScreen(screen) )
	 XFreeColormap(display, map);
      if ( debuglev > 0 ) cout <<"Freed the Colormap" <<endl;
      XFreeCursor(display, busyCursor);
      if ( debuglev > 0 ) cout <<"Freed the Cursor" <<endl;
      // This just doesn't work - because appShell is not mapped??
      // XtDestroyWidget(appShell);
      // if ( debuglev > 0 ) cout <<"Destroyed the appShell" <<endl;
      XCloseDisplay(display);
      if ( debuglev > 0 ) cout <<"Closed the display" <<endl;
   }

//
// Delete command line arguments
//
   if ( debuglev > 0 ) cout <<"Deleting command line arguments" << endl;
   for (int i=0; i<argCount; i++) {
      char	*arg = argVec[i];
      delete arg;
   }
   delete argVec;
   if ( debuglev > 0 ) cout <<"Leaving HalAppC destructor" << endl;

} // End HalAppC destructor

/*----------------------------------------------------------------------
 * Exit callback
 */

void
HalAppC::DoExit(Widget, Boolean interrupted, XtPointer)
{
   if ( debuglev > 0 ) cout <<"Entering HalAppC::DoExit" <<endl;

//
// Ignore terminating signals
//
   SIG_PF	oldhup  = signal(SIGHUP,  SIG_IGN);
   SIG_PF	oldint  = signal(SIGINT,  SIG_IGN);
   SIG_PF	oldterm = signal(SIGTERM, SIG_IGN);

//
// Loop through exit callbacks from the back to the front.  This will
// approximate the C++ destoy methods.
//
   HalExitDataT		data;
   data.cancelExit  = False;
   data.interrupted = interrupted;

   CallbackListC	tmpCalls = halApp->exitCalls; // In case a callback
						      // removes itself
   unsigned	count = halApp->exitCalls.size();
   for (int i=count-1; !data.cancelExit && i>=0; i--) {
      if ( debuglev > 0 ) cout <<"   Calling callback " <<i <<endl;
      (*tmpCalls[i])(&data);
   }

   if ( data.cancelExit ) {

//
// Reinstate terminating signals
//
      signal(SIGHUP,  oldhup);
      signal(SIGINT,  oldint);
      signal(SIGTERM, oldterm);

      if ( debuglev > 0 ) cout <<"   Exit cancelled" <<endl;
      return;
   }

   if ( debuglev > 0 ) cout <<"   Deleting followers" << endl;
   StopFollowers();

   if ( debuglev > 0 ) cout <<"   Deleting app" <<endl;
   delete halApp;

   if ( debuglev > 0 ) cout <<"   Goodbye" <<endl;
   exit(0);
}

/*----------------------------------------------------------------------
 * Help menu callbacks
 */

#ifdef NO_HELP
void HalAppC::DoHelp     (Widget, XtPointer, XtPointer) {}
void HalAppC::DoIndexHelp(Widget, XtPointer, XtPointer) {}
#else
void
HalAppC::DoHelp(Widget w, XtPointer resource, XtPointer)
{
   if ( resource ) halApp->helpWin->ShowCard(w, (char *)resource, w);
   else            halApp->helpWin->ShowCard(w, "contextHelp", w);
}

void
HalAppC::DoIndexHelp(Widget w, XtPointer, XtPointer)
{
    halApp->helpWin->ShowIndex(w);
}
#endif

/*---------------------------------------------------------------
 *  TimeOut used to finish exit processing since there may be X calls in the
 *     user callbacks.
 */

void
DoExit0(XtPointer, XtIntervalId*)
{
   halApp->DoExit(NULL, True/*interrupted*/, NULL);
}

/*----------------------------------------------------------------------
 * Interrupt handler for ctrl-c
 */

void
HalAppC::HandleInterrupt(void *sd, void*)
{
   SignalDataT	*sigData = (SignalDataT*)sd;

   switch (sigData->signum) {

      case(SIGHUP):
      case(SIGINT):
      case(SIGTERM):
//
// Finish processing in a TimeOut handler in case the user callbacks contain
//    any X calls.
//
	 if ( debuglev > 0 ) cout <<"Adding timeout to do X cleanup" <<endl;
	 halApp->exitSignal = sigData->signum;
	 XtAppAddTimeOut(halApp->context, 0, (XtTimerCallbackProc)DoExit0, 0);
	 break;

      default:
         /* Ignore */
         break;
   }
}

/*----------------------------------------------------------------------
 * Interrupt handler for X errors
 */

void
HalAppC::HandleXError(Display *dpy, XErrorEvent *event)
{
   char	buffer[BUFSIZ];
   XGetErrorText(dpy, event->error_code, buffer, BUFSIZ);

   char	*mtype = "XlibMessage";
   char	mesg[BUFSIZ];
   XGetErrorDatabaseText(dpy, mtype, "XError", "X Error", mesg, BUFSIZ);
   fprintf(stderr, "\n%s:  %s\n  ", mesg, buffer);

   XGetErrorDatabaseText(dpy, mtype, "MajorCode", "Request Major code %d", mesg,
			 BUFSIZ);
   fprintf(stderr, mesg, event->request_code);

   if ( event->request_code < 128 ) {
      char	number[32];
      sprintf(number, "%d", event->request_code);
      XGetErrorDatabaseText(dpy, "XRequest", number, "", buffer, BUFSIZ);
      fprintf(stderr, " (%s)\n", buffer);
   }

   switch (event->error_code) {

      case (BadWindow):
      case (BadPixmap):
      case (BadCursor):
      case (BadFont):
      case (BadDrawable):
      case (BadColor):
      case (BadGC):
      case (BadIDChoice):
      case (BadValue):
      case (BadAtom):

         if ( event->error_code == BadValue )
	    XGetErrorDatabaseText(dpy, mtype, "Value", "Value 0x%x", mesg,
				  BUFSIZ);
	 else if ( event->error_code == BadAtom )
	    XGetErrorDatabaseText(dpy, mtype, "AtomID", "AtomID 0x%x", mesg,
				  BUFSIZ);
	 else
	    XGetErrorDatabaseText(dpy, mtype, "ResourceID", "ResourceID 0x%x",
				  mesg, BUFSIZ);

	 fputs("  ", stderr);
	 fprintf(stderr, mesg, event->resourceid);
	 fputs("\n", stderr);
	 break;
   }

   XGetErrorDatabaseText(dpy, mtype, "ErrorSerial", "Error Serial #%d", mesg,
			  BUFSIZ);
   fputs("  ", stderr);
   fprintf(stderr, mesg, event->serial);
   XGetErrorDatabaseText(dpy, mtype, "CurrentSerial", "Current Serial #%d",
			 mesg, BUFSIZ);
   fputs("\n", stderr);
   fflush(stderr);

#if 0
   StringC	msg = halApp->name + ": " + buffer;
   cerr <<msg NL;
//   kill(getpid(), SIGTRAP);
//   halApp->PopupMessage(msg);
#endif
}

/*----------------------------------------------------------------------
 * Interrupt handler for X IO errors
 */

void
HalAppC::HandleXIOError(Display *d)
{
   if (errno == EPIPE) {
      cerr <<halApp->name <<": X connection to " <<DisplayString(d)
           <<" broken (Server error - EPIPE)." NL;
   }
   else {
      StringC	msg = halApp->name + ": fatal IO error on X Server \"";
      msg += DisplayString(d);
      msg += "\"";
      perror(msg);
   }

   halApp->xRunning = False;
   halApp->exitSignal = SIGPIPE;
   DoExit(NULL, True/*interrupted*/, NULL);
}

/*----------------------------------------------------------------------
 *  Turn the quick help on or off for all known windows
 */

void
HalAppC::QuickHelp(Boolean val)
{
   if ( quickHelpEnabled == val ) return;

   unsigned	count = shellList.size();
   for (int i=0; i<count; i++) {
      HalShellC	*shell = (HalShellC *)*shellList[i];
      if ( val ) shell->ShowQuickHelp();
      else	 shell->HideQuickHelp();
   }

   quickHelpEnabled = val;
}

/*----------------------------------------------------------------------
 *  Turn the specified cursor on for all shell widgets in the given
 *     hierarchy.
 */

static void
DefineCursor(Widget parent, Cursor c)
{
   if ( !XtIsComposite(parent) ) return;

   if ( XtIsShell(parent) && XtWindow(parent) )
      XDefineCursor(halApp->display, XtWindow(parent), c);

//
// Loop through the children.
//
   WidgetList	list;
   Cardinal	count;
   XtVaGetValues(parent, XmNnumChildren, &count, XmNchildren, &list, NULL);

   int	i;
   for (i=0; i<count; i++) {
      Widget	w = list[i];
      DefineCursor(w, c);
   }

//
// If there are any popups associated with this widget, check them
//
   for (i=0; i<parent->core.num_popups; i++) {
      Widget	w = parent->core.popup_list[i];
      DefineCursor(w, c);
   }

} // End DefineCursor

/*----------------------------------------------------------------------
 *  Restore the default cursor for all shell widgets in the given
 *     hierarchy.
 */

static void
UndefineCursor(Widget parent)
{
   if ( !XtIsComposite(parent) ) return;

   if ( XtIsShell(parent) && XtWindow(parent) )
      XUndefineCursor(halApp->display, XtWindow(parent));

//
// If this is a composite loop through the children.
//
   WidgetList	list;
   Cardinal	count;
   XtVaGetValues(parent, XmNnumChildren, &count, XmNchildren, &list, NULL);

   int	i;
   for (i=0; i<count; i++) {
      Widget	w = list[i];
      UndefineCursor(w);
   }

//
// If there are any popups associated with this widget, check them
//
   for (i=0; i<parent->core.num_popups; i++) {
      Widget	w = parent->core.popup_list[i];
      UndefineCursor(w);
   }

} // End UndefineCursor

/*----------------------------------------------------------------------
 *  Turn the busy cursor on or off for all known windows
 */

void
HalAppC::BusyCursor(Boolean on)
{
   if ( !xRunning ) return;

   if ( on ) {

//
// Show watch if not already on
//
      if ( !busyOn ) {
	 unsigned	count = shellList.size();
	 for (int i=0; i<count; i++) {
	    HalShellC	*shell = (HalShellC *)*shellList[i];
	    if ( (Window)*shell ) DefineCursor(*shell, busyCursor);
	 }
	 XFlush(display);
	 busyOn = True;
      }
      busyCount++;		// Up the count

   } // End if turning on

   else {

      if ( busyCount>0 ) busyCount--;		// Lower the count

//
// Reset cursor if necessary
//
      if ( busyCount==0 && busyOn ) {
	 unsigned	count = shellList.size();
	 for (int i=0; i<count; i++) {
	    HalShellC	*shell = (HalShellC *)*shellList[i];
	    if ( (Window)*shell ) UndefineCursor(*shell);
	 }
	 XFlush(display);
	 busyOn = False;
      }

   } // End if turning off

} // End HalAppC BusyCursor

/*----------------------------------------------------------------------
 * Register a shell
 */

void
HalAppC::RegisterShell(HalShellC *shell)
{
   void	*ptr = shell;
   shellList.add(ptr);
   if ( busyOn && (Window)*shell ) DefineCursor(*shell, busyCursor);
   if ( quickHelpEnabled ) shell->ShowQuickHelp();
   else			   shell->HideQuickHelp();
}

/*----------------------------------------------------------------------
 * Unregister a shell
 */

void
HalAppC::UnregisterShell(HalShellC *shell)
{
   void	*ptr = shell;
   shellList.remove(ptr);
   if ( busyOn && (Window)*shell ) UndefineCursor(*shell);
}

/*----------------------------------------------------------------------
 *  Display message in popup window
 */

static void
AcknowledgePopup(Widget, Boolean *answered, XtPointer)
{
   *answered = True;
}

void
HalAppC::PopupMessage(const char *msg, Widget parent, unsigned char type)
{
   if ( !xRunning ) {
      cerr <<msg <<endl;
      return;
   }

   static Boolean	answered;
   static MimeRichTextC	*errText;
   WArgList		args;

   if ( !msgDialog ) {

      Boolean useErrorText = get_boolean(*this, "useErrorText", False);
      args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
      msgDialog = XmCreateMessageDialog(*this, "msgDialog", ARGS);

      XtUnmanageChild(XmMessageBoxGetChild(msgDialog, XmDIALOG_CANCEL_BUTTON));
      XtUnmanageChild(XmMessageBoxGetChild(msgDialog, XmDIALOG_HELP_BUTTON));

      XtAddCallback(XmMessageBoxGetChild(msgDialog, XmDIALOG_OK_BUTTON),
		    XmNactivateCallback, (XtCallbackProc)AcknowledgePopup,
		    (XtPointer)&answered);

//
// Trap window manager close function
//
      XmAddWMProtocolCallback(XtParent(msgDialog), halApp->delWinAtom,
			      (XtCallbackProc)AcknowledgePopup,
			      (caddr_t)&answered);

      if (useErrorText) {
	XtUnmanageChild(XmMessageBoxGetChild(msgDialog, XmDIALOG_MESSAGE_LABEL));
	errText = new MimeRichTextC(msgDialog, "errText", 0 ,0);
	XtManageChild(errText->MainWidget());
      }
      else {
	errText = NULL;
      }
   }

//
// Set the dialog's parent so it pops up in the desired place
//
   if ( !parent  || !XtIsRealized(parent) || !XtIsManaged(parent)) 
     parent = appShell;
   Widget	dShell = XtParent(msgDialog);
   dShell->core.parent = parent;
   XtVaSetValues(dShell, XmNtransientFor, parent, NULL);

//
// Set the message and type
//
   args.Reset();
   WXmString	str;
   if ( errText == NULL ) {
     str = msg;
     args.MessageString((XmString)str);
   }
   args.DialogType(type);

//
// DJL - when setting the dialog type, also check to see about the dialog title
//
   StringC newTitle;
   if ( type == XmDIALOG_ERROR ) {
      if ( errorPM )
         args.SymbolPixmap(errorPM);
      newTitle = get_string(*this, "errorPopupTitle", "");
   }
   else if ( type == XmDIALOG_WARNING ) {
      if ( warningPM )
         args.SymbolPixmap(warningPM);
      newTitle = get_string(*this, "warningPopupTitle", "");
   }
   else if ( type == XmDIALOG_QUESTION ) {
      if ( questionPM )
         args.SymbolPixmap(questionPM);
      newTitle = get_string(*this, "questionPopupTitle", "");
   }
   else {
      if ( messagePM )
         args.SymbolPixmap(messagePM);
      newTitle = get_string(*this, "messagePopupTitle", "");
   }
   if (newTitle.size()) {
      args.Title((char*)newTitle);
   }    

   XtSetValues(msgDialog, ARGS);

   if (errText != NULL) {
     CharC msgStr(msg);
     errText->SetString(msgStr);
     errText->ScrollTop();
   }

   XtManageChild(msgDialog);
   XMapRaised(display, XtWindow(XtParent(msgDialog)));

//
// Wait for the popup to be acknowledged
//
   answered = False;
   while ( !answered ) {
      XtAppProcessEvent(context, XtIMXEvent);
      XSync(display, False);
   }

} // End HalAppC PopupMessage

void
HalAppC::PopupMessage(const char *msg, unsigned char type)
{
   PopupMessage(msg, appShell, type);
}


WorkingBoxC*
HalAppC::WorkBox()
{
//
// Create working dialog
//
   if ( !workBox ) workBox = new WorkingBoxC(appShell);
   return(workBox);
}


HelpC*
HalAppC::HelpWin()
{
   return helpWin;
}


Boolean
HalAppC::HasHelp()
{
   return hasHelp;
}
