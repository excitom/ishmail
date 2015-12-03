/*
 *  $Id: SigPrefWinC.C,v 1.2 2000/05/07 12:26:13 fnevgeny Exp $
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

#include "SigPrefWinC.h"
#include "SigPrefC.h"
#include "IshAppC.h"
#include "edit.h"
#include "ShellExp.h"
#include "MainWinC.h"
#include "SendWinC.h"
#include "CompPrefC.h"

#include <hgl/WArgList.h>
#include <hgl/rsrc.h>
#include <hgl/RowColC.h>
#include <hgl/TextMisc.h>

#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/TextF.h>
#include <Xm/ToggleB.h>
#include <Xm/Frame.h>
#include <Xm/PushB.h>
#include <Xm/RowColumn.h>

#include <signal.h>

/*---------------------------------------------------------------
 *  Main window constructor
 */

SigPrefWinC::SigPrefWinC(Widget par) : OptWinC(par, "sigPrefWin")
{
   WArgList	args;
   Widget 	wlist[10];

   extPSigEditPid = 0;
   extESigEditPid = 0;
   intPSigEditPid = 0;
   intESigEditPid = 0;

//
// Create appForm hierarchy
//
// appForm
//    ToggleButton	enableTB
//    ToggleButton	prefixTB
//    Frame		typeFrame
//    Label		fieldTitle
//    RowColC		fieldRC
//    
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_NONE);
   enableTB = XmCreateToggleButton(appForm, "enableSigTB", ARGS);

   args.TopAttachment(XmATTACH_WIDGET, enableTB);
   prefixTB = XmCreateToggleButton(appForm, "sigPrefixTB", ARGS);

   args.TopAttachment(XmATTACH_WIDGET, prefixTB);
   args.RightAttachment(XmATTACH_FORM);
   Widget	typeFrame = XmCreateFrame(appForm, "typeFrame", ARGS);

   args.TopAttachment(XmATTACH_WIDGET, typeFrame);
   args.RightAttachment(XmATTACH_NONE);
   Widget	fieldTitle = XmCreateLabel(appForm, "fieldTitle", ARGS);

   args.TopAttachment(XmATTACH_WIDGET, fieldTitle);
   args.RightAttachment(XmATTACH_FORM);
   RowColC	*fieldRC = new RowColC(appForm, "fieldRC", ARGS);

//
// Set up 2 columns in fieldRC
//
   fieldRC->Defer(True);
   fieldRC->SetOrientation(RcROW_MAJOR);
   fieldRC->SetColCount(2);
   fieldRC->SetColAlignment(XmALIGNMENT_CENTER);
   fieldRC->SetColWidthAdjust(0, RcADJUST_EQUAL);
   fieldRC->SetColWidthAdjust(1, RcADJUST_ATTACH);
   fieldRC->SetColResize(0, False);
   fieldRC->SetColResize(1, True);

//
// Create typeFrame hierarchy
//
// typeFrame
//    RadioBox		typeRadio
//       ToggleButton	plainTB
//       ToggleButton	enrichedTB
//       ToggleButton	enrichedMimeTB
//
   args.Reset();
   args.Orientation(XmVERTICAL);
   args.Packing(XmPACK_TIGHT);
   Widget typeRadio = XmCreateRadioBox(typeFrame, "typeRadio", ARGS);

   plainTB        = XmCreateToggleButton(typeRadio, "plainTB",        0,0);
   enrichedTB     = XmCreateToggleButton(typeRadio, "enrichedTB",     0,0);
   enrichedMimeTB = XmCreateToggleButton(typeRadio, "enrichedMimeTB", 0,0);

   wlist[0] = plainTB;
   wlist[1] = enrichedTB;
   wlist[2] = enrichedMimeTB;
   XtManageChildren(wlist, 3);       // typeRadio children

   XtManageChild(typeRadio);  // typeFrame children

//
// Create fieldRC hierarchy
//
// fieldRC
//    Label		extPSigLabel
//    Form		extPSigForm
//    Label		extESigLabel
//    Form		extESigForm
//    Label		intPSigLabel
//    Form		intPSigForm
//    Label		intESigLabel
//    Form		intESigForm
//
   Widget extPSigLabel = XmCreateLabel(*fieldRC, "extPSigLabel", 0,0);
   Widget extPSigForm  = XmCreateForm (*fieldRC, "extPSigForm",  0,0);
   Widget extESigLabel = XmCreateLabel(*fieldRC, "extESigLabel", 0,0);
   Widget extESigForm  = XmCreateForm (*fieldRC, "extESigForm",  0,0);
   Widget intPSigLabel = XmCreateLabel(*fieldRC, "intPSigLabel", 0,0);
   Widget intPSigForm  = XmCreateForm (*fieldRC, "intPSigForm",  0,0);
   Widget intESigLabel = XmCreateLabel(*fieldRC, "intESigLabel", 0,0);
   Widget intESigForm  = XmCreateForm (*fieldRC, "intESigForm",  0,0);

//
// Create extPSigForm hierarchy
//
// extPSigForm
//    TextField		extPSigTF
//    PushButton	extPSigPB
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   Widget	extPSigPB = XmCreatePushButton(extPSigForm, "extPSigPB", ARGS);

   XtAddCallback(extPSigPB, XmNactivateCallback, (XtCallbackProc)DoEditExtPSig,
		 (XtPointer)this);

   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_WIDGET, extPSigPB);
   extPSigTF = CreateTextField(extPSigForm, "extPSigTF", ARGS);

   wlist[0] = extPSigPB;
   wlist[1] = extPSigTF;
   XtManageChildren(wlist, 2);

//
// Create extESigForm hierarchy
//
// extESigForm
//    TextField		extESigTF
//    PushButton	extESigPB
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   Widget	extESigPB = XmCreatePushButton(extESigForm, "extESigPB", ARGS);

   XtAddCallback(extESigPB, XmNactivateCallback, (XtCallbackProc)DoEditExtESig,
		 (XtPointer)this);

   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_WIDGET, extESigPB);
   extESigTF = CreateTextField(extESigForm, "extESigTF", ARGS);

   wlist[0] = extESigPB;
   wlist[1] = extESigTF;
   XtManageChildren(wlist, 2);

//
// Create intPSigForm hierarchy
//
// intPSigForm
//    TextField		intPSigTF
//    PushButton	intPSigPB
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   Widget	intPSigPB = XmCreatePushButton(intPSigForm, "intPSigPB", ARGS);

   XtAddCallback(intPSigPB, XmNactivateCallback, (XtCallbackProc)DoEditIntPSig,
		 (XtPointer)this);

   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_WIDGET, intPSigPB);
   intPSigTF = CreateTextField(intPSigForm, "intPSigTF", ARGS);

   wlist[0] = intPSigPB;
   wlist[1] = intPSigTF;
   XtManageChildren(wlist, 2);

//
// Create intESigForm hierarchy
//
// intESigForm
//    TextField		intESigTF
//    PushButton	intESigPB
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   Widget	intESigPB = XmCreatePushButton(intESigForm, "intESigPB", ARGS);

   XtAddCallback(intESigPB, XmNactivateCallback, (XtCallbackProc)DoEditIntESig,
		 (XtPointer)this);

   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_WIDGET, intESigPB);
   intESigTF = CreateTextField(intESigForm, "intESigTF", ARGS);

   wlist[0] = intESigPB;
   wlist[1] = intESigTF;
   XtManageChildren(wlist, 2);

   int	count = 0;
   wlist[count++] = extPSigLabel;
   wlist[count++] = extPSigForm;
   wlist[count++] = extESigLabel;
   wlist[count++] = extESigForm;
   wlist[count++] = intPSigLabel;
   wlist[count++] = intPSigForm;
   wlist[count++] = intESigLabel;
   wlist[count++] = intESigForm;
   //XtManageChildren(wlist, count);	// fieldRC children
   fieldRC->SetChildren(wlist, count);

   wlist[0] = enableTB;
   wlist[1] = prefixTB;
   wlist[2] = typeFrame;
   wlist[3] = fieldTitle;
   wlist[4] = *fieldRC;
   XtManageChildren(wlist, 5);	// appForm children

   fieldRC->Defer(False);

   HandleHelp();

} // End SigPrefWinC constructor

/*---------------------------------------------------------------
 *  Method to handle display
 */

void
SigPrefWinC::Show(Widget parent)
{
   if ( debuglev > 0 )
      cout <<"SigPrefWinC::Show(" <<XtName(parent) <<")" <<endl;

   SigPrefC	*prefs = ishApp->sigPrefs;

//
// See if this widget corresponds to a particular composition window
//
   SendWinC	*sendWin = NULL;
   if ( parent != (Widget)*ishApp->mainWin ) {
      unsigned	count = ishApp->sendWinList.size();
      for (int i=0; !sendWin && i<count; i++) {
	 SendWinC	*win = (SendWinC*)*ishApp->sendWinList[i];
	 if ( parent == (Widget)*win ) sendWin = win;
      }
   }

//
// Initialize settings
//
   if ( sendWin )
      XmToggleButtonSetState(enableTB, sendWin->AddingSig(), True);
   else
      XmToggleButtonSetState(enableTB, prefs->appendSig, True);

   XmToggleButtonSetState(prefixTB, prefs->addPrefix, True);

   switch (prefs->type) {
      case (PLAIN_SIG):
         XmToggleButtonSetState(plainTB, True, True);
	 break;
      case (ENRICHED_SIG):
         XmToggleButtonSetState(enrichedTB, True, True);
	 break;
      case (ENRICHED_MIME_SIG):
         XmToggleButtonSetState(enrichedMimeTB, True, True);
	 break;
   }

   if ( prefs->OrigExtPSigFile().size() )
      XmTextFieldSetString(extPSigTF, prefs->OrigExtPSigFile());
   if ( prefs->OrigExtESigFile().size() )
      XmTextFieldSetString(extESigTF, prefs->OrigExtESigFile());
   if ( prefs->OrigIntPSigFile().size() )
      XmTextFieldSetString(intPSigTF, prefs->OrigIntPSigFile());
   if ( prefs->OrigIntESigFile().size() )
      XmTextFieldSetString(intESigTF, prefs->OrigIntESigFile());

   OptWinC::Show(parent);

} // End Show

void
SigPrefWinC::Show()
{
   Show(*ishApp->mainWin);
}

/*---------------------------------------------------------------
 *  Method to apply settings
 */

Boolean
SigPrefWinC::Apply()
{
   SigPrefC	*prefs = ishApp->sigPrefs;

   Boolean	addSig  = XmToggleButtonGetState(enableTB);
   Boolean	usePSig = XmToggleButtonGetState(plainTB) ||
   			  XmToggleButtonGetState(enrichedMimeTB);
   Boolean	useESig = XmToggleButtonGetState(enrichedTB) ||
   			  XmToggleButtonGetState(enrichedMimeTB);

   char	*cs = XmTextFieldGetString(extPSigTF);
   StringC	extPSigStr = cs;
   XtFree(cs);

   cs = XmTextFieldGetString(extESigTF);
   StringC	extESigStr = cs;
   XtFree(cs);

   cs = XmTextFieldGetString(intPSigTF);
   StringC	intPSigStr = cs;
   XtFree(cs);

   cs = XmTextFieldGetString(intESigTF);
   StringC	intESigStr = cs;
   XtFree(cs);

   if ( intPSigStr.size() == 0 ) intPSigStr = extPSigStr;
   if ( intESigStr.size() == 0 ) intESigStr = extESigStr;

   if ( addSig ) {

      if ( usePSig && extPSigStr.size() <= 0 ) {
	 set_invalid(extPSigTF, True, True);
	 PopupMessage("Please enter a signature file name.");
	 return False;
      }

      if ( useESig && extESigStr.size() <= 0 ) {
	 set_invalid(extPSigTF, True, True);
	 PopupMessage("Please enter an enriched signature file name.");
	 return False;
      }

   } // End if addSig

   BusyCursor(True);

//
// Update preferences
//
   prefs->appendSig = addSig;

   unsigned	count = ishApp->sendWinList.size();
   for (int i=0; i<count; i++) {
      SendWinC	*sendWin = (SendWinC*)*ishApp->sendWinList[i];
      sendWin->AddSig(addSig);
   }

   prefs->addPrefix = XmToggleButtonGetState(prefixTB);

   if ( usePSig && useESig ) prefs->type = ENRICHED_MIME_SIG;
   else if ( usePSig )       prefs->type = PLAIN_SIG;
   else if ( useESig )       prefs->type = ENRICHED_SIG;

   prefs->SetExtPSigFile(extPSigStr);
   prefs->SetExtESigFile(extESigStr);
   prefs->SetIntPSigFile(intPSigStr);
   prefs->SetIntESigFile(intESigStr);

   prefs->WriteDatabase();

//
// Write to file if necessary
//
   if ( applyAll ) prefs->WriteFile();

   BusyCursor(False);

   return True;

} // End Apply

/*---------------------------------------------------------------
 *  Method to handle edit of signature file
 */

void
SigPrefWinC::EditSig(pid_t* pid, Widget tf)
{
   if ( *pid != 0 && kill(-(*pid), 0) == 0 ) {
      PopupMessage("There is already an edit in progress.");
      return;
   }

   char	*cs = XmTextFieldGetString(tf);
   StringC	file(cs);
   XtFree(cs);

   if ( file.size() == 0 ) {
      set_invalid(tf, True, True);
      PopupMessage("Please enter a signature file name.");
      return;
   }

   ShellExpand(file);

//
// Start the edit process
//
   CallbackC	modCall;
   CallbackC    doneCall;
   *pid = FilterFile(ishApp->compPrefs->editCmd, file, modCall, doneCall);

} // End EditSig

/*---------------------------------------------------------------
 *  Callbacks to handle edit of signature file
 */

void
SigPrefWinC::DoEditExtPSig(Widget, SigPrefWinC *This, XtPointer)
{
   This->EditSig(&This->extPSigEditPid, This->extPSigTF);
}

void
SigPrefWinC::DoEditExtESig(Widget, SigPrefWinC *This, XtPointer)
{
   This->EditSig(&This->extESigEditPid, This->extESigTF);
}

void
SigPrefWinC::DoEditIntPSig(Widget, SigPrefWinC *This, XtPointer)
{
   This->EditSig(&This->intPSigEditPid, This->intPSigTF);
}

void
SigPrefWinC::DoEditIntESig(Widget, SigPrefWinC *This, XtPointer)
{
   This->EditSig(&This->intESigEditPid, This->intESigTF);
}
