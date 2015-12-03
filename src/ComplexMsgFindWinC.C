/*
 * $Id: ComplexMsgFindWinC.C,v 1.4 2000/08/07 11:05:16 evgeny Exp $
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

#include "ComplexMsgFindWinC.h"
#include "IshAppC.h"
#include "MainWinC.h"
#include "MsgItemC.h"
#include "AppPrefC.h"
#include "ReadWinC.h"

#include <hgl/WArgList.h>
#include <hgl/rsrc.h>
#include <hgl/RowColC.h>
#include <hgl/FieldViewC.h>
#include <hgl/VBoxC.h>
#include <hgl/HalAppC.h>
#include <hgl/TextMisc.h>

#include <Xm/PushB.h>
#include <Xm/TextF.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/Label.h>

/*---------------------------------------------------------------
 *  Constructor
 */

ComplexMsgFindWinC::ComplexMsgFindWinC(Widget parent) : BoolExpWinC(parent)
{
   WArgList	args;
   Widget	wlist[33];

   findIndex = 0;

   numOp  = MsgNumExpC::LT;
   dateOp = MsgDateExpC::LT;
   lineOp = MsgLineExpC::LT;
   byteOp = MsgByteExpC::LT;
   stat   = MSG_NEW;

//
// Create termForm hierarchy
//
//   RowColC	termRC
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   RowColC	*termRC = new RowColC(termForm, "termRC", ARGS);
   termRC->Defer(True);

//
// Set up 3 columns
//
   termRC->SetOrientation(RcROW_MAJOR);
   termRC->SetColCount(3);
   termRC->SetColAlignment(XmALIGNMENT_CENTER);
   termRC->SetColWidthAdjust(RcADJUST_EQUAL);
   termRC->SetColWidthAdjust(2, RcADJUST_ATTACH);
   termRC->SetColResize(False);
   termRC->SetColResize(2, True);

//
// Create termRC children
//
//   PushButton		fromPB
//   Label		fromContLabel
//   TextField		fromTF
//   PushButton		toPB
//   Label		toContLabel
//   TextField		toTF
//   PushButton		subjPB
//   Label		subjContLabel
//   TextField		subjTF
//   PushButton		headPB
//   Label		headContLabel
//   TextField		headTF
//   PushButton		bodyPB
//   Label		bodyContLabel
//   TextField		bodyTF
//   PushButton		msgPB
//   Label		msgContLabel
//   TextField		msgTF
//   PushButton		datePB
//   OptionMenu		dateOM
//   TextField		dateTF
//   PushButton		statPB
//   Label		statIsLabel
//   OptionMenu		statOM
//   PushButton		numPB
//   OptionMenu		numOM
//   TextField		numTF
//   PushButton		linePB
//   OptionMenu		lineOM
//   TextField		lineTF
//   PushButton		bytePB
//   OptionMenu		byteOM
//   TextField		byteTF
//

   Widget fromPB = XmCreatePushButton(*termRC, "fromPB", 0,0);
   Widget fromContLabel = XmCreateLabel(*termRC, "fromContLabel", 0,0);
   fromTF        = CreateTextField (*termRC, "fromTF", 0,0);

   Widget toPB   = XmCreatePushButton(*termRC, "toPB",   0,0);
   Widget toContLabel = XmCreateLabel(*termRC, "toContLabel", 0,0);
   toTF          = CreateTextField (*termRC, "toTF",   0,0);

   Widget subjPB = XmCreatePushButton(*termRC, "subjPB", 0,0);
   Widget subjContLabel = XmCreateLabel(*termRC, "subjContLabel", 0,0);
   subjTF        = CreateTextField (*termRC, "subjTF", 0,0);

   Widget headPB = XmCreatePushButton(*termRC, "headPB", 0,0);
   Widget headContLabel = XmCreateLabel(*termRC, "headContLabel", 0,0);
   headTF        = CreateTextField (*termRC, "headTF", 0,0);

   Widget bodyPB = XmCreatePushButton(*termRC, "bodyPB", 0,0);
   Widget bodyContLabel = XmCreateLabel(*termRC, "bodyContLabel", 0,0);
   bodyTF        = CreateTextField (*termRC, "bodyTF", 0,0);

   Widget msgPB  = XmCreatePushButton(*termRC, "msgPB",  0,0);
   Widget msgContLabel = XmCreateLabel(*termRC, "msgContLabel", 0,0);
   msgTF         = CreateTextField (*termRC, "msgTF",  0,0);

   Widget datePB = XmCreatePushButton(*termRC, "datePB", 0,0);
   Widget dateOM = XmCreateOptionMenu(*termRC, "dateOM", 0,0);
   dateTF        = CreateTextField (*termRC, "dateTF", 0,0);

   Widget statPB = XmCreatePushButton(*termRC, "statPB", 0,0);
   Widget statIsLabel = XmCreateLabel(*termRC, "statIsLabel", 0,0);
   Widget statOM = XmCreateOptionMenu(*termRC, "statOM", 0,0);

   Widget numPB  = XmCreatePushButton(*termRC, "numPB",  0,0);
   Widget numOM  = XmCreateOptionMenu(*termRC, "numOM",  0,0);
   numTF         = CreateTextField (*termRC, "numTF",  0,0);

   Widget linePB = XmCreatePushButton(*termRC, "linePB", 0,0);
   Widget lineOM = XmCreateOptionMenu(*termRC, "lineOM", 0,0);
   lineTF        = CreateTextField (*termRC, "lineTF", 0,0);

   Widget bytePB = XmCreatePushButton(*termRC, "bytePB", 0,0);
   Widget byteOM = XmCreateOptionMenu(*termRC, "byteOM", 0,0);
   byteTF        = CreateTextField (*termRC, "byteTF", 0,0);

   XtAddCallback(fromPB, XmNactivateCallback, (XtCallbackProc)AddFrom,
		 (XtPointer)this);
   XtAddCallback(toPB, XmNactivateCallback, (XtCallbackProc)AddTo,
		 (XtPointer)this);
   XtAddCallback(subjPB, XmNactivateCallback, (XtCallbackProc)AddSubject,
		 (XtPointer)this);
   XtAddCallback(headPB, XmNactivateCallback, (XtCallbackProc)AddHead,
		 (XtPointer)this);
   XtAddCallback(bodyPB, XmNactivateCallback, (XtCallbackProc)AddBody,
		 (XtPointer)this);
   XtAddCallback(msgPB, XmNactivateCallback, (XtCallbackProc)AddMsg,
		 (XtPointer)this);
   XtAddCallback(datePB, XmNactivateCallback, (XtCallbackProc)AddDate,
		 (XtPointer)this);
   XtAddCallback(statPB, XmNactivateCallback, (XtCallbackProc)AddStatus,
		 (XtPointer)this);
   XtAddCallback(numPB,  XmNactivateCallback, (XtCallbackProc)AddNumber,
		 (XtPointer)this);
   XtAddCallback(linePB, XmNactivateCallback, (XtCallbackProc)AddLine,
		 (XtPointer)this);
   XtAddCallback(bytePB, XmNactivateCallback, (XtCallbackProc)AddByte,
		 (XtPointer)this);

   XtAddCallback(fromTF, XmNvalueChangedCallback, (XtCallbackProc)EnablePB,
		 (XtPointer)fromPB);
   XtAddCallback(toTF, XmNvalueChangedCallback, (XtCallbackProc)EnablePB,
		 (XtPointer)toPB);
   XtAddCallback(subjTF, XmNvalueChangedCallback, (XtCallbackProc)EnablePB,
		 (XtPointer)subjPB);
   XtAddCallback(headTF, XmNvalueChangedCallback, (XtCallbackProc)EnablePB,
		 (XtPointer)headPB);
   XtAddCallback(bodyTF, XmNvalueChangedCallback, (XtCallbackProc)EnablePB,
		 (XtPointer)bodyPB);
   XtAddCallback(msgTF, XmNvalueChangedCallback, (XtCallbackProc)EnablePB,
		 (XtPointer)msgPB);
   XtAddCallback(dateTF, XmNvalueChangedCallback, (XtCallbackProc)EnablePB,
		 (XtPointer)datePB);
   XtAddCallback(numTF, XmNvalueChangedCallback, (XtCallbackProc)EnablePB,
		 (XtPointer)numPB);
   XtAddCallback(lineTF, XmNvalueChangedCallback, (XtCallbackProc)EnablePB,
		 (XtPointer)linePB);
   XtAddCallback(byteTF, XmNvalueChangedCallback, (XtCallbackProc)EnablePB,
		 (XtPointer)bytePB);

//
// Set the initial state of the push buttons
//
   EnablePB(fromTF, fromPB, NULL);
   EnablePB(toTF,   toPB,   NULL);
   EnablePB(subjTF, subjPB, NULL);
   EnablePB(headTF, headPB, NULL);
   EnablePB(bodyTF, bodyPB, NULL);
   EnablePB(msgTF,  msgPB,  NULL);
   EnablePB(dateTF, datePB, NULL);
   EnablePB(numTF,  numPB,  NULL);
   EnablePB(lineTF, linePB, NULL);
   EnablePB(byteTF, bytePB, NULL);

//
// Create datePD hierarchy
//
//    PulldowmMenu	datePD
//       PushButton	   ltPB
//       PushButton	   lePB
//       PushButton	   eqPB
//       PushButton	   gePB
//       PushButton	   gtPB
//       PushButton	   nePB
//
   Widget	datePD = XmCreatePulldownMenu(*termRC, "datePD", 0,0);

   args.Reset();
   args.UserData((XtPointer)MsgDateExpC::LT);
   Widget ltPB = XmCreatePushButton(datePD, "ltPB", ARGS);
   args.UserData((XtPointer)MsgDateExpC::LE);
   Widget lePB = XmCreatePushButton(datePD, "lePB", ARGS);
   args.UserData((XtPointer)MsgDateExpC::EQ);
   Widget eqPB = XmCreatePushButton(datePD, "eqPB", ARGS);
   args.UserData((XtPointer)MsgDateExpC::GE);
   Widget gePB = XmCreatePushButton(datePD, "gePB", ARGS);
   args.UserData((XtPointer)MsgDateExpC::GT);
   Widget gtPB = XmCreatePushButton(datePD, "gtPB", ARGS);
   args.UserData((XtPointer)MsgDateExpC::NE);
   Widget nePB = XmCreatePushButton(datePD, "nePB", ARGS);

   XtAddCallback(ltPB, XmNactivateCallback, (XtCallbackProc)ChangeDateOp,
		 (XtPointer)this);
   XtAddCallback(lePB, XmNactivateCallback, (XtCallbackProc)ChangeDateOp,
		 (XtPointer)this);
   XtAddCallback(eqPB, XmNactivateCallback, (XtCallbackProc)ChangeDateOp,
		 (XtPointer)this);
   XtAddCallback(gePB, XmNactivateCallback, (XtCallbackProc)ChangeDateOp,
		 (XtPointer)this);
   XtAddCallback(gtPB, XmNactivateCallback, (XtCallbackProc)ChangeDateOp,
		 (XtPointer)this);
   XtAddCallback(nePB, XmNactivateCallback, (XtCallbackProc)ChangeDateOp,
		 (XtPointer)this);

   XtVaSetValues(dateOM, XmNsubMenuId, datePD, NULL);

   wlist[0] = ltPB;
   wlist[1] = lePB;
   wlist[2] = eqPB;
   wlist[3] = gePB;
   wlist[4] = gtPB;
   wlist[5] = nePB;
   XtManageChildren(wlist, 6);	// datePD children

//
// Create statPD hierarchy
//
//    PulldowmMenu	statPD
//       PushButton	   newPB
//       PushButton	   readPB
//       PushButton	   deletedPB
//       PushButton	   savedPB
//       PushButton	   repliedPB
//       PushButton	   forwardedPB
//       PushButton	   resentPB
//       PushButton	   printedPB
//       PushButton	   filteredPB
//       PushButton	   mimePB
//
   Widget	statPD = XmCreatePulldownMenu(*termRC, "statPD", 0,0);

   args.Reset();
   args.UserData((XtPointer)MSG_NEW);
   Widget	newPB       = XmCreatePushButton(statPD, "newPB",       ARGS);
   args.UserData((XtPointer)MSG_READ);
   Widget	readPB      = XmCreatePushButton(statPD, "readPB",      ARGS);
   args.UserData((XtPointer)MSG_DELETED);
   Widget	deletedPB   = XmCreatePushButton(statPD, "deletedPB",   ARGS);
   args.UserData((XtPointer)MSG_SAVED);
   Widget	savedPB     = XmCreatePushButton(statPD, "savedPB",     ARGS);
   args.UserData((XtPointer)MSG_REPLIED);
   Widget	repliedPB   = XmCreatePushButton(statPD, "repliedPB",   ARGS);
   args.UserData((XtPointer)MSG_FORWARDED);
   Widget	forwardedPB = XmCreatePushButton(statPD, "forwardedPB", ARGS);
   args.UserData((XtPointer)MSG_RESENT);
   Widget	resentPB    = XmCreatePushButton(statPD, "resentPB",    ARGS);
   args.UserData((XtPointer)MSG_PRINTED);
   Widget	printedPB   = XmCreatePushButton(statPD, "printedPB",   ARGS);
   args.UserData((XtPointer)MSG_FILTERED);
   Widget	filteredPB  = XmCreatePushButton(statPD, "filteredPB",  ARGS);
   args.UserData((XtPointer)MSG_MIME);
   Widget	mimePB      = XmCreatePushButton(statPD, "mimePB",      ARGS);

   AddActivate(newPB,		ChangeStatus, this);
   AddActivate(readPB,		ChangeStatus, this);
   AddActivate(deletedPB,	ChangeStatus, this);
   AddActivate(savedPB,		ChangeStatus, this);
   AddActivate(repliedPB,	ChangeStatus, this);
   AddActivate(forwardedPB,	ChangeStatus, this);
   AddActivate(resentPB,	ChangeStatus, this);
   AddActivate(printedPB,	ChangeStatus, this);
   AddActivate(filteredPB,	ChangeStatus, this);
   AddActivate(mimePB,		ChangeStatus, this);

   XtVaSetValues(statOM, XmNsubMenuId, statPD, NULL);

   wlist[0] = newPB;
   wlist[1] = readPB;
   wlist[2] = savedPB;
   wlist[3] = deletedPB;
   wlist[4] = repliedPB;
   wlist[5] = forwardedPB;
   wlist[6] = resentPB;
   wlist[7] = printedPB;
   wlist[8] = filteredPB;
   wlist[9] = mimePB;
   XtManageChildren(wlist, 10);	// statPD children

//
// Create numPD hierarchy
//
//    PulldowmMenu	numPD
//       PushButton	   ltPB
//       PushButton	   lePB
//       PushButton	   eqPB
//       PushButton	   gePB
//       PushButton	   gtPB
//       PushButton	   nePB
//
   Widget	numPD = XmCreatePulldownMenu(*termRC, "numPD", 0,0);

   args.Reset();
   args.UserData((XtPointer)MsgNumExpC::LT);
   ltPB = XmCreatePushButton(numPD, "ltPB", ARGS);
   args.UserData((XtPointer)MsgNumExpC::LE);
   lePB = XmCreatePushButton(numPD, "lePB", ARGS);
   args.UserData((XtPointer)MsgNumExpC::EQ);
   eqPB = XmCreatePushButton(numPD, "eqPB", ARGS);
   args.UserData((XtPointer)MsgNumExpC::GE);
   gePB = XmCreatePushButton(numPD, "gePB", ARGS);
   args.UserData((XtPointer)MsgNumExpC::GT);
   gtPB = XmCreatePushButton(numPD, "gtPB", ARGS);
   args.UserData((XtPointer)MsgNumExpC::NE);
   nePB = XmCreatePushButton(numPD, "nePB", ARGS);

   XtAddCallback(ltPB, XmNactivateCallback, (XtCallbackProc)ChangeNumOp,
		 (XtPointer)this);
   XtAddCallback(lePB, XmNactivateCallback, (XtCallbackProc)ChangeNumOp,
		 (XtPointer)this);
   XtAddCallback(eqPB, XmNactivateCallback, (XtCallbackProc)ChangeNumOp,
		 (XtPointer)this);
   XtAddCallback(gePB, XmNactivateCallback, (XtCallbackProc)ChangeNumOp,
		 (XtPointer)this);
   XtAddCallback(gtPB, XmNactivateCallback, (XtCallbackProc)ChangeNumOp,
		 (XtPointer)this);
   XtAddCallback(nePB, XmNactivateCallback, (XtCallbackProc)ChangeNumOp,
		 (XtPointer)this);

   XtVaSetValues(numOM, XmNsubMenuId, numPD, NULL);

   wlist[0] = ltPB;
   wlist[1] = lePB;
   wlist[2] = eqPB;
   wlist[3] = gePB;
   wlist[4] = gtPB;
   wlist[5] = nePB;
   XtManageChildren(wlist, 6);	// numPD children

//
// Create linePD hierarchy
//
//    PulldowmMenu	linePD
//       PushButton	   ltPB
//       PushButton	   lePB
//       PushButton	   eqPB
//       PushButton	   gePB
//       PushButton	   gtPB
//       PushButton	   nePB
//
   Widget	linePD = XmCreatePulldownMenu(*termRC, "linePD", 0,0);

   args.Reset();
   args.UserData((XtPointer)MsgLineExpC::LT);
   ltPB = XmCreatePushButton(linePD, "ltPB", ARGS);
   args.UserData((XtPointer)MsgLineExpC::LE);
   lePB = XmCreatePushButton(linePD, "lePB", ARGS);
   args.UserData((XtPointer)MsgLineExpC::EQ);
   eqPB = XmCreatePushButton(linePD, "eqPB", ARGS);
   args.UserData((XtPointer)MsgLineExpC::GE);
   gePB = XmCreatePushButton(linePD, "gePB", ARGS);
   args.UserData((XtPointer)MsgLineExpC::GT);
   gtPB = XmCreatePushButton(linePD, "gtPB", ARGS);
   args.UserData((XtPointer)MsgLineExpC::NE);
   nePB = XmCreatePushButton(linePD, "nePB", ARGS);

   XtAddCallback(ltPB, XmNactivateCallback, (XtCallbackProc)ChangeLineOp,
		 (XtPointer)this);
   XtAddCallback(lePB, XmNactivateCallback, (XtCallbackProc)ChangeLineOp,
		 (XtPointer)this);
   XtAddCallback(eqPB, XmNactivateCallback, (XtCallbackProc)ChangeLineOp,
		 (XtPointer)this);
   XtAddCallback(gePB, XmNactivateCallback, (XtCallbackProc)ChangeLineOp,
		 (XtPointer)this);
   XtAddCallback(gtPB, XmNactivateCallback, (XtCallbackProc)ChangeLineOp,
		 (XtPointer)this);
   XtAddCallback(nePB, XmNactivateCallback, (XtCallbackProc)ChangeLineOp,
		 (XtPointer)this);

   XtVaSetValues(lineOM, XmNsubMenuId, linePD, NULL);

   wlist[0] = ltPB;
   wlist[1] = lePB;
   wlist[2] = eqPB;
   wlist[3] = gePB;
   wlist[4] = gtPB;
   wlist[5] = nePB;
   XtManageChildren(wlist, 6);	// linePD children

//
// Create bytePD hierarchy
//
//    PulldowmMenu	bytePD
//       PushButton	   ltPB
//       PushButton	   lePB
//       PushButton	   eqPB
//       PushButton	   gePB
//       PushButton	   gtPB
//       PushButton	   nePB
//
   Widget	bytePD = XmCreatePulldownMenu(*termRC, "bytePD", 0,0);

   args.Reset();
   args.UserData((XtPointer)MsgByteExpC::LT);
   ltPB = XmCreatePushButton(bytePD, "ltPB", ARGS);
   args.UserData((XtPointer)MsgByteExpC::LE);
   lePB = XmCreatePushButton(bytePD, "lePB", ARGS);
   args.UserData((XtPointer)MsgByteExpC::EQ);
   eqPB = XmCreatePushButton(bytePD, "eqPB", ARGS);
   args.UserData((XtPointer)MsgByteExpC::GE);
   gePB = XmCreatePushButton(bytePD, "gePB", ARGS);
   args.UserData((XtPointer)MsgByteExpC::GT);
   gtPB = XmCreatePushButton(bytePD, "gtPB", ARGS);
   args.UserData((XtPointer)MsgByteExpC::NE);
   nePB = XmCreatePushButton(bytePD, "nePB", ARGS);

   XtAddCallback(ltPB, XmNactivateCallback, (XtCallbackProc)ChangeByteOp,
		 (XtPointer)this);
   XtAddCallback(lePB, XmNactivateCallback, (XtCallbackProc)ChangeByteOp,
		 (XtPointer)this);
   XtAddCallback(eqPB, XmNactivateCallback, (XtCallbackProc)ChangeByteOp,
		 (XtPointer)this);
   XtAddCallback(gePB, XmNactivateCallback, (XtCallbackProc)ChangeByteOp,
		 (XtPointer)this);
   XtAddCallback(gtPB, XmNactivateCallback, (XtCallbackProc)ChangeByteOp,
		 (XtPointer)this);
   XtAddCallback(nePB, XmNactivateCallback, (XtCallbackProc)ChangeByteOp,
		 (XtPointer)this);

   XtVaSetValues(byteOM, XmNsubMenuId, bytePD, NULL);

   wlist[0] = ltPB;
   wlist[1] = lePB;
   wlist[2] = eqPB;
   wlist[3] = gePB;
   wlist[4] = gtPB;
   wlist[5] = nePB;
   XtManageChildren(wlist, 6);	// bytePD children

   int	wcount = 0;
   wlist[wcount++] = fromPB;
   wlist[wcount++] = fromContLabel;
   wlist[wcount++] = fromTF;
   wlist[wcount++] = toPB;
   wlist[wcount++] = toContLabel;
   wlist[wcount++] = toTF;
   wlist[wcount++] = subjPB;
   wlist[wcount++] = subjContLabel;
   wlist[wcount++] = subjTF;
   wlist[wcount++] = headPB;
   wlist[wcount++] = headContLabel;
   wlist[wcount++] = headTF;
   wlist[wcount++] = bodyPB;
   wlist[wcount++] = bodyContLabel;
   wlist[wcount++] = bodyTF;
   wlist[wcount++] = msgPB;
   wlist[wcount++] = msgContLabel;
   wlist[wcount++] = msgTF;
   wlist[wcount++] = datePB;
   wlist[wcount++] = dateOM;
   wlist[wcount++] = dateTF;
   wlist[wcount++] = statPB;
   wlist[wcount++] = statIsLabel;
   wlist[wcount++] = statOM;
   wlist[wcount++] = numPB;
   wlist[wcount++] = numOM;
   wlist[wcount++] = numTF;
   wlist[wcount++] = linePB;
   wlist[wcount++] = lineOM;
   wlist[wcount++] = lineTF;
   wlist[wcount++] = bytePB;
   wlist[wcount++] = byteOM;
   wlist[wcount++] = byteTF;
   XtManageChildren(wlist, wcount);	// termRC children

   XtManageChild(*termRC);	// termForm children

//
// Tell the RowColC about it's children
//
   wcount = 0;
   wlist[wcount++] = fromPB;
   wlist[wcount++] = fromContLabel;
   wlist[wcount++] = fromTF;
   wlist[wcount++] = toPB;
   wlist[wcount++] = toContLabel;
   wlist[wcount++] = toTF;
   wlist[wcount++] = subjPB;
   wlist[wcount++] = subjContLabel;
   wlist[wcount++] = subjTF;
   wlist[wcount++] = headPB;
   wlist[wcount++] = headContLabel;
   wlist[wcount++] = headTF;
   wlist[wcount++] = bodyPB;
   wlist[wcount++] = bodyContLabel;
   wlist[wcount++] = bodyTF;
   wlist[wcount++] = msgPB;
   wlist[wcount++] = msgContLabel;
   wlist[wcount++] = msgTF;
   wlist[wcount++] = datePB;
   wlist[wcount++] = dateOM;
   wlist[wcount++] = dateTF;
   wlist[wcount++] = statPB;
   wlist[wcount++] = statIsLabel;
   wlist[wcount++] = statOM;
   wlist[wcount++] = numPB;
   wlist[wcount++] = numOM;
   wlist[wcount++] = numTF;
   wlist[wcount++] = linePB;
   wlist[wcount++] = lineOM;
   wlist[wcount++] = lineTF;
   wlist[wcount++] = bytePB;
   wlist[wcount++] = byteOM;
   wlist[wcount++] = byteTF;
   termRC->AddChildren(wlist, wcount);

   termRC->Defer(False);

//
// Add "find prev" and "find all" buttons after the apply button
//
   args.Reset();
   args.PositionIndex(2);
   Widget findPrevPB = XmCreatePushButton(buttonRC, "findPrevPB", ARGS);
   XtAddCallback(findPrevPB, XmNactivateCallback, (XtCallbackProc)DoFindPrev,
		 (XtPointer)this);
   XtManageChild(findPrevPB);

   args.PositionIndex(3);
   Widget findAllPB = XmCreatePushButton(buttonRC, "findAllPB", ARGS);
   XtAddCallback(findAllPB, XmNactivateCallback, (XtCallbackProc)DoFindAll,
		 (XtPointer)this);
   XtManageChild(findAllPB);

   XtUnmanageChild(okPB);	// We only use the apply button
   ShowInfoMsg();		// Add message line

   HandleHelp();

   AddApplyCallback((CallbackFn *)DoFind, (void *)this);

} // End constructor

/*---------------------------------------------------------------
 *  Method to display dialog
 */

void
ComplexMsgFindWinC::Show()
{
   findIndex = 0;
   BoolExpWinC::Show();
}

/*---------------------------------------------------------------
 *  Callback routine to add terms to window
 */

void
ComplexMsgFindWinC::AddFrom(Widget, ComplexMsgFindWinC *This, XtPointer)
{
   char *cs = XmTextFieldGetString(This->fromTF);
   This->AddTermExp(new MsgFromExpC(cs));
   XtFree(cs);
}

void
ComplexMsgFindWinC::AddTo(Widget, ComplexMsgFindWinC *This, XtPointer)
{
   char *cs = XmTextFieldGetString(This->toTF);
   This->AddTermExp(new MsgToExpC(cs));
   XtFree(cs);
}

void
ComplexMsgFindWinC::AddSubject(Widget, ComplexMsgFindWinC *This, XtPointer)
{
   char *cs = XmTextFieldGetString(This->subjTF);
   This->AddTermExp(new MsgSubjExpC(cs));
   XtFree(cs);
}

void
ComplexMsgFindWinC::AddHead(Widget, ComplexMsgFindWinC *This, XtPointer)
{
   char *cs = XmTextFieldGetString(This->headTF);
   This->AddTermExp(new MsgHeadExpC(cs));
   XtFree(cs);
}

void
ComplexMsgFindWinC::AddBody(Widget, ComplexMsgFindWinC *This, XtPointer)
{
   char *cs = XmTextFieldGetString(This->bodyTF);
   This->AddTermExp(new MsgBodyExpC(cs));
   XtFree(cs);
}

void
ComplexMsgFindWinC::AddMsg(Widget, ComplexMsgFindWinC *This, XtPointer)
{
   char *cs = XmTextFieldGetString(This->msgTF);
   This->AddTermExp(new MsgMsgExpC(cs));
   XtFree(cs);
}

void
ComplexMsgFindWinC::AddDate(Widget, ComplexMsgFindWinC *This, XtPointer)
{
   char *cs = XmTextFieldGetString(This->dateTF);
   This->AddTermExp(new MsgDateExpC(This->dateOp, cs));
   XtFree(cs);
}

void
ComplexMsgFindWinC::AddStatus(Widget, ComplexMsgFindWinC *This, XtPointer)
{
   This->AddTermExp(new MsgStatExpC(This->stat));
}

void
ComplexMsgFindWinC::AddNumber(Widget, ComplexMsgFindWinC *This, XtPointer)
{
   char *cs = XmTextFieldGetString(This->numTF);
   This->AddTermExp(new MsgNumExpC(This->numOp, atoi(cs)));
   XtFree(cs);
}

void
ComplexMsgFindWinC::AddLine(Widget, ComplexMsgFindWinC *This, XtPointer)
{
   char *cs = XmTextFieldGetString(This->lineTF);
   This->AddTermExp(new MsgLineExpC(This->lineOp, atoi(cs)));
   XtFree(cs);
}

void
ComplexMsgFindWinC::AddByte(Widget, ComplexMsgFindWinC *This, XtPointer)
{
   char *cs = XmTextFieldGetString(This->byteTF);
   This->AddTermExp(new MsgByteExpC(This->byteOp, atoi(cs)));
   XtFree(cs);
}

/*---------------------------------------------------------------
 *  Callback routine to handle entry of text in a text field.  The given
 *  push button is enabled only if the text field is not empty.
 */

void
ComplexMsgFindWinC::EnablePB(Widget tf, Widget pb, XtPointer)
{
   char	*cs = XmTextFieldGetString(tf);
   XtSetSensitive(pb, strlen(cs)>0);
   XtFree(cs);
}

/*---------------------------------------------------------------
 *  Callback routines to handle operator menu selections
 */

void
ComplexMsgFindWinC::ChangeDateOp(Widget w, ComplexMsgFindWinC *This, XtPointer)
{
   MsgDateExpC::MsgDateOp	ptr;
   XtVaGetValues(w, XmNuserData, &ptr, NULL);
   This->dateOp = ptr;
}

void
ComplexMsgFindWinC::ChangeNumOp(Widget w, ComplexMsgFindWinC *This, XtPointer)
{
   MsgNumExpC::MsgNumOp	ptr;
   XtVaGetValues(w, XmNuserData, &ptr, NULL);
   This->numOp = ptr;
}

void
ComplexMsgFindWinC::ChangeLineOp(Widget w, ComplexMsgFindWinC *This, XtPointer)
{
   MsgLineExpC::MsgLineOp	ptr;
   XtVaGetValues(w, XmNuserData, &ptr, NULL);
   This->lineOp = ptr;
}

void
ComplexMsgFindWinC::ChangeByteOp(Widget w, ComplexMsgFindWinC *This, XtPointer)
{
   MsgByteExpC::MsgByteOp	ptr;
   XtVaGetValues(w, XmNuserData, &ptr, NULL);
   This->byteOp = ptr;
}

/*---------------------------------------------------------------
 *  Callback routine to handle press of status type button
 */

void
ComplexMsgFindWinC::ChangeStatus(Widget w, ComplexMsgFindWinC *This, XtPointer)
{
   MsgStatusT	ptr;
   XtVaGetValues(w, XmNuserData, &ptr, NULL);
   This->stat = ptr;
}

/*---------------------------------------------------------------
 *  Callback routine to handle press of "find prev" button in find
 *     message dialog
 */

void
ComplexMsgFindWinC::DoFindPrev(Widget, ComplexMsgFindWinC *This, XtPointer)
{
//
// Replace the apply callback
//
   This->RemoveApplyCallback((CallbackFn *)DoFind, (void *)This);
   This->AddApplyCallback((CallbackFn *)DoFindPrev2, (void *)This);

   This->Apply();

//
// Restore the apply callback
//
   This->RemoveApplyCallback((CallbackFn *)DoFindPrev2, (void *)This);
   This->AddApplyCallback((CallbackFn *)DoFind, (void *)This);

} // End DoFindPrev

/*---------------------------------------------------------------
 *  Callback routine to handle press of "find all" button in find
 *     message dialog
 */

void
ComplexMsgFindWinC::DoFindAll(Widget, ComplexMsgFindWinC *This, XtPointer)
{
//
// Replace the apply callback
//
   This->RemoveApplyCallback((CallbackFn *)DoFind, (void *)This);
   This->AddApplyCallback((CallbackFn *)DoFindAll2, (void *)This);

   This->Apply();

//
// Restore the apply callback
//
   This->RemoveApplyCallback((CallbackFn *)DoFindAll2, (void *)This);
   This->AddApplyCallback((CallbackFn *)DoFind, (void *)This);

} // End DoFindAll

/*---------------------------------------------------------------
 *  Callback routine to handle press of find button in find message dialog
 */

void
ComplexMsgFindWinC::DoFind(BoolExpC *be, ComplexMsgFindWinC *This)
{
   This->ClearMessage();
   if ( !be ) return;

//
// Loop through message list of current folder
//
   VItemListC&	msgList = ishApp->mainWin->MsgVBox().VisItems();
   unsigned	count = msgList.size();
   MsgItemC	*item = NULL;
   Boolean	found = False;
   int		index;
   int		start;

//
// If there is a single selected message, use that as the start index
//
   VItemListC&	selList = ishApp->mainWin->MsgVBox().SelItems();
   if ( selList.size() == 1 ) {
      VItemC	*item = selList[0];
      start = msgList.indexOf(item) + 1;
      if ( start >= count ) start = 0;
   }
   else
      start = This->findIndex;

//
// Loop from the start position to the end of the list
//
   for (index=start; !found && index<count; index++) {
      item = (MsgItemC*)msgList[index];
      found = be->Match(item);
   }

//
// Loop from the beginning of the list to the start position
//
   if ( !found ) {
      for (index=0; !found && index<start; index++) {
	 item = (MsgItemC*)msgList[index];
	 found = be->Match(item);
      }
   }

//
// Select the item if it was found
//
   if ( found ) {

//
// See if there is an unpinned reading window displayed
//
      found = False;
      unsigned	rcount = ishApp->readWinList.size();
      for (int i=0; !found && i<rcount; i++) {
	 ReadWinC	*readWin = (ReadWinC*)*ishApp->readWinList[i];
	 found = (readWin->IsShown() && !readWin->Pinned());
      }

      if ( found ) {
	 ishApp->DisplayMessage(item->msg);
      } else {
	 ishApp->mainWin->FieldView().ScrollToItem(*item);
	 ishApp->mainWin->MsgVBox().SelectItemOnly(*item);
      }

//
// Next time, start at message after this one
//
      This->findIndex = index + 1;
      if ( This->findIndex >= count ) This->findIndex = 0;
   }

   else {
      ishApp->mainWin->MsgVBox().DeselectAllItems();
      This->findIndex = 0;
      This->Message("No match");
      XBell(halApp->display, ishApp->appPrefs->bellVolume);
   }

} // End DoFind

/*---------------------------------------------------------------
 *  Callback routine to handle press of "find prev" button
 */

void
ComplexMsgFindWinC::DoFindPrev2(BoolExpC *be, ComplexMsgFindWinC *This)
{
   This->ClearMessage();
   if ( !be ) return;

//
// Loop through message list of current folder
//
   VItemListC&	msgList = ishApp->mainWin->MsgVBox().VisItems();
   unsigned	count = msgList.size();
   MsgItemC	*item = NULL;
   Boolean	found = False;
   int		index;
   int		start;

//
// If there is a single selected message, use that as the start index
//
   VItemListC&	selList = ishApp->mainWin->MsgVBox().SelItems();
   if ( selList.size() == 1 ) {
      VItemC	*item = selList[0];
      start = msgList.indexOf(item) - 1;
      if ( start <= 0 ) start = count - 1;
   }
   else
      start = This->findIndex;

//
// Loop from the start position to the beginning of the list
//
   for (index=start; !found && index>=0; index--) {
      item = (MsgItemC*)msgList[index];
      found = be->Match(item);
   }

//
// Loop from the end of the list to the start position
//
   if ( !found ) {
      for (index=count-1; !found && index>start; index--) {
	 item = (MsgItemC*)msgList[index];
	 found = be->Match(item);
      }
   }

//
// Select the item if it was found
//
   if ( found ) {

//
// See if there is an unpinned reading window displayed
//
      found = False;
      unsigned	rcount = ishApp->readWinList.size();
      for (int i=0; !found && i<rcount; i++) {
	 ReadWinC	*readWin = (ReadWinC*)*ishApp->readWinList[i];
	 found = (readWin->IsShown() && !readWin->Pinned());
      }

      if ( found ) {
	 ishApp->DisplayMessage(item->msg);
      } else {
	 ishApp->mainWin->FieldView().ScrollToItem(*item);
	 ishApp->mainWin->MsgVBox().SelectItemOnly(*item);
      }

//
// Next time, start at message before this one
//
      This->findIndex = index - 1;
      if ( This->findIndex <= 0 ) This->findIndex = count-1;
   }

   else {
      ishApp->mainWin->MsgVBox().DeselectAllItems();
      This->findIndex = 0;
      This->Message("No match");
      XBell(halApp->display, ishApp->appPrefs->bellVolume);
   }

} // End DoFindPrev2

/*---------------------------------------------------------------
 *  Callback routine for Apply function
 */

void
ComplexMsgFindWinC::DoFindAll2(BoolExpC *be, ComplexMsgFindWinC *This)
{
   This->ClearMessage();
   if ( !be ) return;

   ishApp->mainWin->BusyCursor(True);

//
// Loop through message list of current folder
//
   This->findIndex = 0;

   ishApp->mainWin->MsgVBox().DeselectAllItems(False);

   VItemListC&	msgList = ishApp->mainWin->MsgVBox().VisItems();
   unsigned	count = msgList.size();

   for (int i=0; i<count; i++) {

      MsgItemC	*item = (MsgItemC*)msgList[i];
      if ( be->Match(item) )
	 ishApp->mainWin->MsgVBox().SelectItem(*item, False);

   } // End for each item


   VItemListC&	selItems = ishApp->mainWin->MsgVBox().SelItems();
   if ( selItems.size() > 0 ) {

      StringC	tmp;
      tmp += (int)selItems.size();
      tmp += " item";
      if ( selItems.size() > 1 ) tmp += "s";
      tmp += " selected";
      This->Message(tmp);

//
// See if there is an unpinned reading window displayed
//
      Boolean	found = False;
      unsigned	rcount = ishApp->readWinList.size();
      for (int i=0; !found && i<rcount; i++) {
	 ReadWinC	*readWin = (ReadWinC*)*ishApp->readWinList[i];
	 found = (readWin->IsShown() && !readWin->Pinned());
      }

      MsgItemC	*item = (MsgItemC*)selItems[0];
      if ( found )
	 ishApp->DisplayMessage(item->msg);
      else
	 ishApp->mainWin->FieldView().ScrollToItem(*item);
   }
   else {
      This->Message("No match");
      XBell(halApp->display, ishApp->appPrefs->bellVolume);
   }

   ishApp->mainWin->MsgVBox().Refresh();
   ishApp->mainWin->EnableButtons();

   ishApp->mainWin->BusyCursor(False);

} // End DoFindAll2
