/*
 * $Id: PrintWinC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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

#ifndef _PrintWinC_h_
#define _PrintWinC_h_

#include <hgl/HalDialogC.h>
#include <hgl/StringC.h>

class PrintWinC;
class LoginWinC;
class MsgPartC;
class PtrListC;
class MsgC;
class VItemListC;

typedef struct {

   pid_t		pid;
   int			msgnum;
   PrintWinC		*win;
   StringC		tmpFile;
   StringC		cmdStr;
   int			status;

} PrintProcT;

/*---------------------------------------------------------------
 *  Data for fetching external body parts
 */

typedef struct {

   PrintWinC	*win;
   MsgPartC	*part;
   StringC	file;
   StringC	cmdStr;
   pid_t	pid;

} FetchProcT;

class PrintWinC : public HalDialogC {

   enum PrintTypeT {
      PRINT_PLAIN,
      PRINT_RICH,
      PRINT_ENRICHED
   };

//
// Widgets
//
   Widget		hdrFrame;
   Widget		hdrAllTB;
   Widget		hdrDispTB;
   Widget		hdrNoneTB;
   Widget		ordFrame;
   Widget		ordParallelTB;
   Widget		ordSerialTB;
   Widget		cmdTF;
   Widget		printQueryWin;
   Widget		printOneYesTB;
   Widget		printOneNoTB;
   Widget		printAllYesTB;
   Widget		printAllNoTB;
   Widget		getQueryWin;
   Widget		getOneYesTB;
   Widget		getOneNoTB;
   Widget		getAllYesTB;
   Widget		getAllNoTB;
   LoginWinC		*loginWin;

//
// Data
//
   Boolean		askAboutPrint;	// Set false if user has answered
					//    "print all" or "print none"
   Boolean		printAll;	// True if user has answered
					//    "print all"
   Boolean		cancelled;	// True if user cancelled job
   StringC		printStr;
   StringC		multiPrintStr;
   static PtrListC      *fetchProcList;

//
// Callbacks
//
   static void		DoPrint(Widget, PrintWinC*, XtPointer);
   static void		PrintDone(int, PrintProcT*);
   static void		FinishedGet(int, FetchProcT*);

//
// Private methods
//
   Boolean		InsertMessage(MsgPartC*, FILE*, PtrListC&, PrintTypeT);
   Boolean		OkToGet(MsgPartC*);
   Boolean		OkToPrint(MsgPartC*);
   Boolean		OkToSend(MsgPartC*);
   Boolean		Print(MsgC*);
   Boolean		PrintFile(char*, PrintTypeT type=PRINT_PLAIN,
   				  Boolean isTmp=True, int num=-1);
   Boolean		PrintHeaders(MsgC*, FILE*, PrintTypeT);
   Boolean		PrintMime(MsgC*);
   Boolean		PrintNonText(MsgPartC*);
   Boolean		PrintParallel(VItemListC&);
   Boolean		PrintSerial(VItemListC&);
   Boolean		PrintSerialMime(VItemListC&);
   Boolean		PrintSkipMessage(MsgPartC*, FILE*);
   Boolean		PrintText(MsgPartC*, FILE*, PtrListC&, PrintTypeT,
				  Boolean doNext=True);
   Boolean		GetPart(MsgPartC*);

public:

// Methods

   PrintWinC(Widget);
   ~PrintWinC();

   void			ShowOrder();
   void			HideOrder();
};

#endif // _PrintWinC_h_
