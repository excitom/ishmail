/*
 *  $Id: SortPrefWinC.C,v 1.3 2001/03/07 10:41:36 evgeny Exp $
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
#include "SortPrefWinC.h"
#include "SortMgrC.h"
#include "SortPrefC.h"
#include "FolderC.h"
#include "FolderPrefC.h"
#include "IshAppC.h"
#include "MainWinC.h"

#include <hgl/WArgList.h>
#include <hgl/WXmString.h>
#include <hgl/RowColC.h>
#include <hgl/VBoxC.h>
#include <hgl/CharC.h>

#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/Label.h>

/*---------------------------------------------------------------
 *  Constructor
 */

SortPrefWinC::SortPrefWinC(Widget parent) : OptWinC(parent, "sortWin")
{
   WArgList	args;
   Widget	wlist[30];

   thread.name  = "THREAD";
   number.name  = "NUMBER";
   status.name  = "STATUS";
   sender.name  = "SENDER";
   to.name      = "TO";
   subject.name = "SUBJECT";
   date.name    = "DATE";
   lines.name   = "LINES";
   bytes.name   = "BYTES";

//
// Create appForm hierarchy
//
//    RowColC	keyRC
//    Frame	threadFrame
//    Frame	folderFrame
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_NONE);
   keyRC = new RowColC(appForm, "keyRC", ARGS);

   args.TopAttachment(XmATTACH_WIDGET, *keyRC);
   Widget	threadFrame = XmCreateFrame(appForm, "threadFrame", ARGS);

   args.TopAttachment(XmATTACH_WIDGET, threadFrame);
   Widget	folderFrame = XmCreateFrame(appForm, "folderFrame", ARGS);

//
// Set up 3 Columns
//
   keyRC->Defer(True);
   keyRC->SetOrientation(RcROW_MAJOR);
   keyRC->SetColCount(3);
   keyRC->SetColAlignment(XmALIGNMENT_BEGINNING);
   keyRC->SetColWidthAdjust(RcADJUST_EQUAL);
   keyRC->SetColResize(False);
   keyRC->SetColResize(0, True);
   keyRC->SetColAlignment(0, XmALIGNMENT_CENTER);

//
// Create keyRC hierarchy
//
// keyRC
//      Label		ordTitle
//      Label		keyTitle
//      Label		dirTitle
//      Label		numberLabel
//      ToggleButton	numberTB
//      Frame		numberFrame
//      Label		statusLabel
//      ToggleButton	statusTB
//      Frame		statusFrame
//      Label		senderLabel
//      ToggleButton	senderTB
//      Frame		senderFrame
//      Label		toLabel
//      ToggleButton	toTB
//      Frame		toFrame
//      Label		subjectLabel
//      ToggleButton	subjectTB
//      Frame		subjectFrame
//      Label		dateLabel
//      ToggleButton	dateTB
//      Frame		dateFrame
//      Label		linesLabel
//      ToggleButton	linesTB
//      Frame		linesFrame
//      Label		bytesLabel
//      ToggleButton	bytesTB
//      Frame		bytesFrame
//
   Widget	ordTitle     = XmCreateLabel(*keyRC,  "orderTitle",     0,0);
   Widget	keyTitle     = XmCreateLabel(*keyRC,  "keyTitle",       0,0);
   Widget	dirTitle     = XmCreateLabel(*keyRC,  "directionTitle", 0,0);
   Widget	numberFrame  = XmCreateFrame(*keyRC,  "numberFrame",    0,0);
   Widget	statusFrame  = XmCreateFrame(*keyRC,  "statusFrame",    0,0);
   Widget	senderFrame  = XmCreateFrame(*keyRC,  "senderFrame",    0,0);
   Widget	toFrame      = XmCreateFrame(*keyRC,      "toFrame",    0,0);
   Widget	subjectFrame = XmCreateFrame(*keyRC, "subjectFrame",    0,0);
   Widget	dateFrame    = XmCreateFrame(*keyRC,    "dateFrame",    0,0);
   Widget	linesFrame   = XmCreateFrame(*keyRC,   "linesFrame",    0,0);
   Widget	bytesFrame   = XmCreateFrame(*keyRC,   "bytesFrame",    0,0);

   args.Reset();
   args.Alignment(XmALIGNMENT_CENTER);
   args.RecomputeSize(False);
   number.label  = XmCreateLabel(*keyRC,  "numberLabel",  ARGS);
   status.label  = XmCreateLabel(*keyRC,  "statusLabel",  ARGS);
   sender.label  = XmCreateLabel(*keyRC,  "senderLabel",  ARGS);
   to.label      = XmCreateLabel(*keyRC,  "toLabel",      ARGS);
   subject.label = XmCreateLabel(*keyRC,  "subjectLabel", ARGS);
   date.label    = XmCreateLabel(*keyRC,  "dateLabel",    ARGS);
   lines.label   = XmCreateLabel(*keyRC,  "linesLabel",   ARGS);
   bytes.label   = XmCreateLabel(*keyRC,  "bytesLabel",   ARGS);

   args.Reset();
   args.Alignment(XmALIGNMENT_BEGINNING);
   number.tb     = XmCreateToggleButton(*keyRC,  "numberTB",     ARGS);
   status.tb     = XmCreateToggleButton(*keyRC,  "statusTB",     ARGS);
   sender.tb     = XmCreateToggleButton(*keyRC,  "senderTB",     ARGS);
   to.tb         = XmCreateToggleButton(*keyRC,  "toTB",         ARGS);
   subject.tb    = XmCreateToggleButton(*keyRC,  "subjectTB",    ARGS);
   date.tb       = XmCreateToggleButton(*keyRC,  "dateTB",       ARGS);
   lines.tb      = XmCreateToggleButton(*keyRC,  "linesTB",      ARGS);
   bytes.tb      = XmCreateToggleButton(*keyRC,  "bytesTB",      ARGS);

   XtAddCallback(number.tb, XmNvalueChangedCallback,
		 (XtCallbackProc)ToggleKey, (XtPointer)this);
   XtAddCallback(status.tb, XmNvalueChangedCallback,
		 (XtCallbackProc)ToggleKey, (XtPointer)this);
   XtAddCallback(subject.tb, XmNvalueChangedCallback,
		 (XtCallbackProc)ToggleKey, (XtPointer)this);
   XtAddCallback(sender.tb, XmNvalueChangedCallback,
		 (XtCallbackProc)ToggleKey, (XtPointer)this);
   XtAddCallback(to.tb, XmNvalueChangedCallback,
		 (XtCallbackProc)ToggleKey, (XtPointer)this);
   XtAddCallback(date.tb, XmNvalueChangedCallback,
		 (XtCallbackProc)ToggleKey, (XtPointer)this);
   XtAddCallback(lines.tb, XmNvalueChangedCallback,
		 (XtCallbackProc)ToggleKey, (XtPointer)this);
   XtAddCallback(bytes.tb, XmNvalueChangedCallback,
		 (XtCallbackProc)ToggleKey, (XtPointer)this);

//
// Fill frames
//
//   numberFrame
//      RadioBox	numberRadio
//   statusFrame
//      RadioBox	statusRadio
//   senderFrame
//      RadioBox	senderRadio
//   toFrame
//      RadioBox	toRadio
//   subjectFrame
//      RadioBox	subjectRadio
//   dateFrame
//      RadioBox	dateRadio
//   linesFrame
//      RadioBox	linesRadio
//   bytesFrame
//      RadioBox	bytesRadio
//
   args.Reset();
   args.Orientation(XmHORIZONTAL);
   args.Packing(XmPACK_TIGHT);
   Widget numberRadio  = XmCreateRadioBox(numberFrame,  "numberRadio",  ARGS);
   Widget statusRadio  = XmCreateRadioBox(statusFrame,  "statusRadio",  ARGS);
   Widget senderRadio  = XmCreateRadioBox(senderFrame,  "senderRadio",  ARGS);
   Widget toRadio      = XmCreateRadioBox(toFrame,      "toRadio",      ARGS);
   Widget subjectRadio = XmCreateRadioBox(subjectFrame, "subjectRadio", ARGS);
   Widget dateRadio    = XmCreateRadioBox(dateFrame,    "dateRadio",    ARGS);
   Widget linesRadio   = XmCreateRadioBox(linesFrame,   "linesRadio",   ARGS);
   Widget bytesRadio   = XmCreateRadioBox(bytesFrame,   "bytesRadio",   ARGS);

//
// Create numberRadio hierarchy
//
//    numberRadio
//       ToggleButton	   numberATB
//       ToggleButton	   numberDTB
//
   number.atb = XmCreateToggleButton(numberRadio, "ascendTB",  0,0);
   number.dtb = XmCreateToggleButton(numberRadio, "descendTB", 0,0);

   wlist[0] = number.atb;
   wlist[1] = number.dtb;
   XtManageChildren(wlist, 2);
   XtManageChild(numberRadio);	// numberFrame children

   XtAddCallback(number.atb, XmNvalueChangedCallback, (XtCallbackProc)ToggleDir,
		 (XtPointer)this);
   XtAddCallback(number.dtb, XmNvalueChangedCallback, (XtCallbackProc)ToggleDir,
		 (XtPointer)this);

//
// Create statusRadio hierarchy
//
//    statusRadio
//       ToggleButton	   statusATB
//       ToggleButton	   statusDTB
//
   status.atb = XmCreateToggleButton(statusRadio, "ascendTB",  0,0);
   status.dtb = XmCreateToggleButton(statusRadio, "descendTB", 0,0);

   wlist[0] = status.atb;
   wlist[1] = status.dtb;
   XtManageChildren(wlist, 2);
   XtManageChild(statusRadio);	// statusFrame children

   XtAddCallback(status.atb, XmNvalueChangedCallback, (XtCallbackProc)ToggleDir,
		 (XtPointer)this);
   XtAddCallback(status.dtb, XmNvalueChangedCallback, (XtCallbackProc)ToggleDir,
		 (XtPointer)this);

//
// Create senderRadio hierarchy
//
//    senderRadio
//       ToggleButton	   senderATB
//       ToggleButton	   senderDTB
//
   sender.atb = XmCreateToggleButton(senderRadio, "ascendTB",  0,0);
   sender.dtb = XmCreateToggleButton(senderRadio, "descendTB", 0,0);

   wlist[0] = sender.atb;
   wlist[1] = sender.dtb;
   XtManageChildren(wlist, 2);
   XtManageChild(senderRadio);	// senderFrame children

   XtAddCallback(sender.atb, XmNvalueChangedCallback, (XtCallbackProc)ToggleDir,
		 (XtPointer)this);
   XtAddCallback(sender.dtb, XmNvalueChangedCallback, (XtCallbackProc)ToggleDir,
		 (XtPointer)this);

//
// Create toRadio hierarchy
//
//    toRadio
//       ToggleButton	   toATB
//       ToggleButton	   toDTB
//
   to.atb = XmCreateToggleButton(toRadio, "ascendTB",  0,0);
   to.dtb = XmCreateToggleButton(toRadio, "descendTB", 0,0);

   wlist[0] = to.atb;
   wlist[1] = to.dtb;
   XtManageChildren(wlist, 2);
   XtManageChild(toRadio);	// toFrame children

   XtAddCallback(to.atb, XmNvalueChangedCallback, (XtCallbackProc)ToggleDir,
		 (XtPointer)this);
   XtAddCallback(to.dtb, XmNvalueChangedCallback, (XtCallbackProc)ToggleDir,
		 (XtPointer)this);

//
// Create subjectRadio hierarchy
//
//    subjectRadio
//       ToggleButton	   subjectATB
//       ToggleButton	   subjectDTB
//
   subject.atb = XmCreateToggleButton(subjectRadio, "ascendTB",  0,0);
   subject.dtb = XmCreateToggleButton(subjectRadio, "descendTB", 0,0);

   wlist[0] = subject.atb;
   wlist[1] = subject.dtb;
   XtManageChildren(wlist, 2);
   XtManageChild(subjectRadio);	// subjectFrame children

   XtAddCallback(subject.atb, XmNvalueChangedCallback,(XtCallbackProc)ToggleDir,
   		 (XtPointer)this);
   XtAddCallback(subject.dtb, XmNvalueChangedCallback,(XtCallbackProc)ToggleDir,
   		 (XtPointer)this);

//
// Create dateRadio hierarchy
//
//    dateRadio
//       ToggleButton	   dateATB
//       ToggleButton	   dateDTB
//
   date.atb = XmCreateToggleButton(dateRadio, "ascendTB",  0,0);
   date.dtb = XmCreateToggleButton(dateRadio, "descendTB", 0,0);

   wlist[0] = date.atb;
   wlist[1] = date.dtb;
   XtManageChildren(wlist, 2);
   XtManageChild(dateRadio);	// dateFrame children

   XtAddCallback(date.atb, XmNvalueChangedCallback, (XtCallbackProc)ToggleDir,
		 (XtPointer)this);
   XtAddCallback(date.dtb, XmNvalueChangedCallback, (XtCallbackProc)ToggleDir,
		 (XtPointer)this);

//
// Create linesRadio hierarchy
//
//    linesRadio
//       ToggleButton	   linesATB
//       ToggleButton	   linesDTB
//
   lines.atb = XmCreateToggleButton(linesRadio, "ascendTB",  0,0);
   lines.dtb = XmCreateToggleButton(linesRadio, "descendTB", 0,0);

   wlist[0] = lines.atb;
   wlist[1] = lines.dtb;
   XtManageChildren(wlist, 2);
   XtManageChild(linesRadio);	// linesFrame children

   XtAddCallback(lines.atb, XmNvalueChangedCallback, (XtCallbackProc)ToggleDir,
		 (XtPointer)this);
   XtAddCallback(lines.dtb, XmNvalueChangedCallback, (XtCallbackProc)ToggleDir,
		 (XtPointer)this);

//
// Create bytesRadio hierarchy
//
//    bytesRadio
//       ToggleButton	   bytesATB
//       ToggleButton	   bytesDTB
//
   bytes.atb = XmCreateToggleButton(bytesRadio, "ascendTB",  0,0);
   bytes.dtb = XmCreateToggleButton(bytesRadio, "descendTB", 0,0);

   wlist[0] = bytes.atb;
   wlist[1] = bytes.dtb;
   XtManageChildren(wlist, 2);
   XtManageChild(bytesRadio);	// bytesFrame children

   XtAddCallback(bytes.atb, XmNvalueChangedCallback, (XtCallbackProc)ToggleDir,
		 (XtPointer)this);
   XtAddCallback(bytes.dtb, XmNvalueChangedCallback, (XtCallbackProc)ToggleDir,
		 (XtPointer)this);

//
// Set initial state of buttons
//
   XmToggleButtonSetState(number.tb,   False, False);
   XmToggleButtonSetState(number.atb,  True,  False);
   XmToggleButtonSetState(number.dtb,  False, False);
   XmToggleButtonSetState(status.tb,   False, False);
   XmToggleButtonSetState(status.atb,  True,  False);
   XmToggleButtonSetState(status.dtb,  False, False);
   XmToggleButtonSetState(sender.tb,   False, False);
   XmToggleButtonSetState(sender.atb,  True,  False);
   XmToggleButtonSetState(sender.dtb,  False, False);
   XmToggleButtonSetState(to.tb,       False, False);
   XmToggleButtonSetState(to.atb,      True,  False);
   XmToggleButtonSetState(to.dtb,      False, False);
   XmToggleButtonSetState(subject.tb,  False, False);
   XmToggleButtonSetState(subject.atb, True,  False);
   XmToggleButtonSetState(subject.dtb, False, False);
   XmToggleButtonSetState(date.tb,     False, False);
   XmToggleButtonSetState(date.atb,    True,  False);
   XmToggleButtonSetState(date.dtb,    False, False);
   XmToggleButtonSetState(lines.tb,    False, False);
   XmToggleButtonSetState(lines.atb,   True,  False);
   XmToggleButtonSetState(lines.dtb,   False, False);
   XmToggleButtonSetState(bytes.tb,    False, False);
   XmToggleButtonSetState(bytes.atb,   True,  False);
   XmToggleButtonSetState(bytes.dtb,   False, False);

//
// Add widgets to keyRC
//
   int	wcount = 0;
   wlist[wcount++] = ordTitle;
   wlist[wcount++] = keyTitle;
   wlist[wcount++] = dirTitle;
   wlist[wcount++] = number.label;
   wlist[wcount++] = number.tb;
   wlist[wcount++] = numberFrame;
   wlist[wcount++] = status.label;
   wlist[wcount++] = status.tb;
   wlist[wcount++] = statusFrame;
   wlist[wcount++] = subject.label;
   wlist[wcount++] = subject.tb;
   wlist[wcount++] = subjectFrame;
   wlist[wcount++] = sender.label;
   wlist[wcount++] = sender.tb;
   wlist[wcount++] = senderFrame;
   wlist[wcount++] = to.label;
   wlist[wcount++] = to.tb;
   wlist[wcount++] = toFrame;
   wlist[wcount++] = date.label;
   wlist[wcount++] = date.tb;
   wlist[wcount++] = dateFrame;
   wlist[wcount++] = lines.label;
   wlist[wcount++] = lines.tb;
   wlist[wcount++] = linesFrame;
   wlist[wcount++] = bytes.label;
   wlist[wcount++] = bytes.tb;
   wlist[wcount++] = bytesFrame;
   keyRC->SetChildren(wlist, wcount);
   keyRC->Defer(False);

//
// Add buttons for specifying threading parameters
//
//  threadFrame
//     Form		threadForm
//        ToggleButton	   threadTB
//        Frame		   threadSortFrame
//           RadioBox	      threadRadio
//              ToggleButton	 threadOldTB
//              ToggleButton	 threadNewTB
//
   Widget	threadForm = XmCreateForm(threadFrame, "threadForm", 0,0);

   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_NONE);
   thread.tb = XmCreateToggleButton(threadForm, "threadTB", ARGS);

   XtAddCallback(thread.tb, XmNvalueChangedCallback,
		 (XtCallbackProc)ToggleKey, (XtPointer)this);

   args.TopAttachment(XmATTACH_WIDGET, thread.tb);
   args.BottomAttachment(XmATTACH_FORM);
   Widget threadSortFrame = XmCreateFrame(threadForm, "threadSortFrame", ARGS);

   args.Reset();
   args.Orientation(XmVERTICAL);
   args.Packing(XmPACK_TIGHT);
   Widget threadRadio  = XmCreateRadioBox(threadSortFrame, "threadRadio", ARGS);

   thread.atb = XmCreateToggleButton(threadRadio, "threadOldTB", 0,0);
   thread.dtb = XmCreateToggleButton(threadRadio, "threadNewTB", 0,0);

   wlist[0] = thread.atb;
   wlist[1] = thread.dtb;
   XtManageChildren(wlist, 2);	// threadRadio children
   XtManageChild(threadRadio);	// threadSortFrame children
   wlist[0] = thread.tb;
   wlist[1] = threadSortFrame;
   XtManageChildren(wlist, 2);	// threadForm children
   XtManageChild(threadForm);	// threadFrame children

   XmToggleButtonSetState(thread.tb,  False, False);
   XmToggleButtonSetState(thread.atb, True,  False);
   XmToggleButtonSetState(thread.dtb, False, False);

//
// Add buttons for specifying the folders to which this sort is to apply
//
// folderFrame
//    Radio		   folderRadio
//       ToggleButton	      folderCurTB
//       ToggleButton	      folderSelTB
//       ToggleButton	      folderAllTB
//       ToggleButton	      folderDefTB
//
   args.Reset();
   args.Orientation(XmVERTICAL);
   args.Packing(XmPACK_TIGHT);
   Widget folderRadio = XmCreateRadioBox(folderFrame, "folderRadio", ARGS);

   folderCurTB = XmCreateToggleButton(folderRadio, "folderCurTB", 0,0);
   folderSelTB = XmCreateToggleButton(folderRadio, "folderSelTB", 0,0);
   folderAllTB = XmCreateToggleButton(folderRadio, "folderAllTB", 0,0);
   folderDefTB = XmCreateToggleButton(folderRadio, "folderDefTB", 0,0);

   wlist[0] = folderCurTB;
   wlist[1] = folderSelTB;
   wlist[2] = folderAllTB;
   wlist[3] = folderDefTB;
   XtManageChildren(wlist, 4);		// folderRadio children

   XtManageChild(folderRadio);		// folderFrame children

   wlist[0] = *keyRC;
   wlist[1] = threadFrame;
   wlist[2] = folderFrame;
   XtManageChildren(wlist, 3);	// appForm children

   HandleHelp();

   XmToggleButtonSetState(folderDefTB, True, True);

} // End constructor

/*---------------------------------------------------------------
 *  Destructor
 */

SortPrefWinC::~SortPrefWinC()
{
   delete keyRC;
}

/*---------------------------------------------------------------
 *  Display the dialog using the information from the specified sort
 *     manager.
 */

void
SortPrefWinC::Show(SortMgrC *sortMgr)
{
//
// Turn off all keys
//
   XmToggleButtonSetState(thread.tb,  False, False);
   XmToggleButtonSetState(number.tb,  False, False);
   XmToggleButtonSetState(status.tb,  False, False);
   XmToggleButtonSetState(sender.tb,  False, False);
   XmToggleButtonSetState(to.tb,      False, False);
   XmToggleButtonSetState(subject.tb, False, False);
   XmToggleButtonSetState(date.tb,    False, False);
   XmToggleButtonSetState(lines.tb,   False, False);
   XmToggleButtonSetState(bytes.tb,   False, False);

//
// Initialize key states
//
   SortKeyListC&	keyList = sortMgr->KeyList();
   unsigned		count = keyList.size();
   for (int i=0; i<count; i++) {

      MailSortKeyC	*key = (MailSortKeyC*)keyList[i];
      Boolean		ascend = (key->Dir() == SortKeyC::ASCENDING);

//
// Initialize the state of the toggle buttons
//
      switch (key->Type()) {

	 case (MailSortKeyC::NUMBER):
	    XmToggleButtonSetState(number.tb,     True, False);
	    XmToggleButtonSetState(number.atb,  ascend, False);
	    XmToggleButtonSetState(number.dtb, !ascend, False);
	    selectedList.add(number.tb);
	    break;

	 case (MailSortKeyC::STATUS):
	    XmToggleButtonSetState(status.tb,     True, False);
	    XmToggleButtonSetState(status.atb,  ascend, False);
	    XmToggleButtonSetState(status.dtb, !ascend, False);
	    selectedList.add(status.tb);
	    break;

	 case (MailSortKeyC::SENDER):
	    XmToggleButtonSetState(sender.tb,     True, False);
	    XmToggleButtonSetState(sender.atb,  ascend, False);
	    XmToggleButtonSetState(sender.dtb, !ascend, False);
	    selectedList.add(sender.tb);
	    break;

	 case (MailSortKeyC::TO):
	    XmToggleButtonSetState(to.tb,     True, False);
	    XmToggleButtonSetState(to.atb,  ascend, False);
	    XmToggleButtonSetState(to.dtb, !ascend, False);
	    selectedList.add(to.tb);
	    break;

	 case (MailSortKeyC::SUBJECT):
	    XmToggleButtonSetState(subject.tb,     True, False);
	    XmToggleButtonSetState(subject.atb,  ascend, False);
	    XmToggleButtonSetState(subject.dtb, !ascend, False);
	    selectedList.add(subject.tb);
	    break;

	 case (MailSortKeyC::DATE):
	    XmToggleButtonSetState(date.tb,     True, False);
	    XmToggleButtonSetState(date.atb,  ascend, False);
	    XmToggleButtonSetState(date.dtb, !ascend, False);
	    selectedList.add(date.tb);
	    break;

	 case (MailSortKeyC::LINES):
	    XmToggleButtonSetState(lines.tb,     True, False);
	    XmToggleButtonSetState(lines.atb,  ascend, False);
	    XmToggleButtonSetState(lines.dtb, !ascend, False);
	    selectedList.add(lines.tb);
	    break;

	 case (MailSortKeyC::BYTES):
	    XmToggleButtonSetState(bytes.tb,     True, False);
	    XmToggleButtonSetState(bytes.atb,  ascend, False);
	    XmToggleButtonSetState(bytes.dtb, !ascend, False);
	    selectedList.add(bytes.tb);
	    break;

	 case (MailSortKeyC::THREAD):
	    break;

      } // End switch sort key type

   } // End for each sort key

   if ( sortMgr->Threaded() ) {
      Boolean		ascend = sortMgr->ThreadDir();
      XmToggleButtonSetState(thread.tb,     True, False);
      XmToggleButtonSetState(thread.atb,  ascend, False);
      XmToggleButtonSetState(thread.dtb, !ascend, False);
      selectedList.add(thread.tb);
   }

   UpdateOrderLabels();

//
// Initialize how this is to be applied
//
   if ( sortMgr == ishApp->mainWin->sortMgr )
      XmToggleButtonSetState(folderDefTB, True, True);
   else
      XmToggleButtonSetState(folderCurTB, True, True);

   OptWinC::Show();

} // End Show

/*---------------------------------------------------------------
 *  Method to apply settings
 */

Boolean
SortPrefWinC::Apply()
{
   SortPrefC	*prefs = ishApp->sortPrefs;

   BusyCursor(True);

//
// Build the string for this sort
//
   RegexC	tmp;
   StringC	*oldKeys;
   StringC	keyStr;
   BuildKeyString(keyStr);

   ishApp->Broadcast("");

   Boolean	sortCurrent = False;

//
// See if this sort applies to the current folder only
//
   if ( XmToggleButtonGetState(folderCurTB) ) {

      ishApp->mainWin->curFolder->Sort(keyStr);
      sortCurrent = True;

//
// Add this sort to the dictionary
//
      tmp     = ishApp->mainWin->curFolder->name;
      oldKeys = prefs->folderSortKeys.definitionOf(tmp);
      if ( oldKeys )
	 *oldKeys = keyStr;
      else
	 prefs->folderSortKeys.add(tmp, keyStr);
   }

   else if ( XmToggleButtonGetState(folderSelTB) ) {

//
// Loop through folders
//
      VItemListC&	slist  = ishApp->mainWin->FolderVBox().SelItems();
      if ( slist.size() == 0 ) {
	 StringC	errmsg("No folders are selected.");
	 PopupMessage(errmsg);
	 BusyCursor(False);
	 return False;
      }

      FolderListC&	flist  = ishApp->folderPrefs->OpenFolders();
      u_int		fcount = flist.size();
      for (int i=0; i<fcount; i++) {

	 FolderC	*folder = flist[i];
	 if ( slist.includes(folder->icon) ) {

	    folder->Sort(keyStr);
	    if ( folder == ishApp->mainWin->curFolder ) sortCurrent = True;

//
// Add this sort to the dictionary
//
	    tmp = folder->name;
	    oldKeys = prefs->folderSortKeys.definitionOf(tmp);
	    if ( oldKeys )
	       *oldKeys = keyStr;
	    else
	       prefs->folderSortKeys.add(tmp, keyStr);
	 }
      }
   }

   else {

      if ( XmToggleButtonGetState(folderAllTB) ) {

//
// Clear out the folder sort keys
//
	 prefs->folderSortKeys.removeAll();

//
// Loop through folders, removing any sort managers
//
	 FolderListC	*list  = &ishApp->folderPrefs->OpenFolders();
	 u_int		count = list->size();
	 int	i;
	 for (i=0; i<count; i++) {
	    FolderC	*folder = (*list)[i];
	    folder->SetSortMgr(NULL);
	 }

	 list  = &ishApp->folderPrefs->FileFolders();
	 count = list->size();
	 for (i=0; i<count; i++) {
	    FolderC	*folder = (*list)[i];
	    folder->SetSortMgr(NULL);
	 }

      } // End if updating all folders

//
// Update the sort manager for the main window
//
      ishApp->mainWin->sortMgr->Set(keyStr);
      prefs->sortKeys = keyStr;

      sortCurrent = True;
   }

   if ( sortCurrent ) {
      ishApp->mainWin->MsgVBox().Sort();
      ishApp->mainWin->MsgVBox().Refresh();
      if ( keyStr.size() > 0 ) ishApp->Broadcast("Messages sorted");
   }

//
// Write prefs
//
   prefs->WriteDatabase();
   if ( applyAll ) prefs->WriteFile();

   BusyCursor(False);

   return True;

} // End Apply

/*---------------------------------------------------------------
 *  Method to build the key string for this sort
 */

void
SortPrefWinC::BuildKeyString(StringC& keyStr)
{
   keyStr.Clear();

//
// Loop through the selected keys
//
   u_int	count = selectedList.size();
   for (int i=0; i<count; i++) {

      Widget	tb = *selectedList[i];

//
// See which key struct this one belongs to
//
      KeyStruct	*key = NULL;
      if      ( tb == thread.tb  ) key = &thread;
      else if ( tb == number.tb  ) key = &number;
      else if ( tb == status.tb  ) key = &status;
      else if ( tb == sender.tb  ) key = &sender;
      else if ( tb == to.tb      ) key = &to;
      else if ( tb == subject.tb ) key = &subject;
      else if ( tb == date.tb    ) key = &date;
      else if ( tb == lines.tb   ) key = &lines;
      else if ( tb == bytes.tb   ) key = &bytes;

      if ( key ) {

	 if ( i > 0 ) keyStr += ' ';
	 keyStr += key->name;

	 if ( XmToggleButtonGetState(key->dtb) ) keyStr += "(D)";
	 else					 keyStr += "(A)";

      } // End if we have a key struct

   } // End for each selected key

} // End BuildKeyString

/*---------------------------------------------------------------
 *  Method to update the labels displaying the order in which the
 *     sort keys were selected.
 */

void
SortPrefWinC::UpdateOrderLabels()
{
//
// Check each key struct
//
//   UpdateOrderLabel(thread);
   UpdateOrderLabel(number);
   UpdateOrderLabel(status);
   UpdateOrderLabel(sender);
   UpdateOrderLabel(to);
   UpdateOrderLabel(subject);
   UpdateOrderLabel(date);
   UpdateOrderLabel(lines);
   UpdateOrderLabel(bytes);

} // End UpdateOrderLabels

void
SortPrefWinC::UpdateOrderLabel(KeyStruct key)
{
//
// Loop through the selected list until we get to the requested key
//    Don't count the thread key
//
   u_int	count = selectedList.size();
   int		index = 1;
   StringC	str;
   for (int i=0; i<count; i++) {
      Widget	tb = *selectedList[i];
      if ( tb == key.tb ) {
	 str += index;
	 i = count;
      }
      else if ( tb != thread.tb )
	 index++;
   }

   if ( str.size() == 0 ) str = " ";
   WXmString	wstr = (char*)str;
   XtVaSetValues(key.label, XmNlabelString, (XmString)wstr, NULL);

} // End UpdateOrderLabel

/*---------------------------------------------------------------
 *  Callback routine to handle press of a sort key button
 */

void
SortPrefWinC::ToggleKey(Widget w, SortPrefWinC *This, XmToggleButtonCallbackStruct *tb)
{
//
// Add this button to or remove if from the list of selected buttons
//
   if ( tb->set ) This->selectedList.add(w);
   else		  This->selectedList.remove(w);

//
// Re-number the keys
//
   This->UpdateOrderLabels();

} // End ToggleKey

/*---------------------------------------------------------------
 *  Callback routines to handle press of a direction toggle button
 */

void
SortPrefWinC::ToggleDir(Widget w, SortPrefWinC *This, XmToggleButtonCallbackStruct *tb)
{
   if ( !tb->set ) return;

//
// See which key struct this one belongs to
//
   KeyStruct	*key = NULL;
   if      ( w == This->thread.atb  || w == This->thread.dtb )
      key = &This->thread;
   else if ( w == This->number.atb  || w == This->number.dtb )
      key = &This->number;
   else if ( w == This->status.atb  || w == This->status.dtb )
      key = &This->status;
   else if ( w == This->sender.atb  || w == This->sender.dtb )
      key = &This->sender;
   else if ( w == This->to.atb      || w == This->to.dtb )
      key = &This->to;
   else if ( w == This->subject.atb || w == This->subject.dtb )
      key = &This->subject;
   else if ( w == This->date.atb    || w == This->date.dtb )
      key = &This->date;
   else if ( w == This->lines.atb   || w == This->lines.dtb )
      key = &This->lines;
   else if ( w == This->bytes.atb   || w == This->bytes.dtb )
      key = &This->bytes;

   if ( !key ) return;

//
// Select the key if not already selected
//
   XmToggleButtonSetState(key->tb, True, True);

} // End ToggleDir

