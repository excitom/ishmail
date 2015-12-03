/*
 * $Id: LocalTextWinC.C,v 1.2 2000/05/07 12:26:12 fnevgeny Exp $
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

#include "LocalTextWinC.h"
#include "IshAppC.h"
#include "ReadPrefC.h"

#include <hgl/WArgList.h>
#include <hgl/MimeRichTextC.h>
#include <hgl/TextMisc.h>
#include <hgl/CharC.h>

#include <Xm/ScrolledW.h>
#include <Xm/Text.h>
#include <Xm/PushB.h>

/*---------------------------------------------------------------
 *  Main window constructor
 */

LocalTextWinC::LocalTextWinC(Widget parent) : HalDialogC("localTextWin", parent)
{
   WArgList	args;

//
// Create appForm hierarchy
//
// appForm
//	MimeRichTextC	rich
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   args.Rows(ishApp->readPrefs->visBodyRows);
   args.Columns(ishApp->readPrefs->visCols);
   rich = new MimeRichTextC(appForm, "richText", ARGS);
   rich->ResizeWidth(!ishApp->readPrefs->wrap);

   XtManageChild(rich->MainWidget());

//
// Create buttonRC hierarchy
//
// buttonRC
//    PushButton	okPB
//    PushButton	helpPB
//
   AddButtonBox();
   savePB  = XmCreatePushButton(buttonRC, "savePB",  0,0);
   printPB = XmCreatePushButton(buttonRC, "printPB", 0,0);
   pipePB  = XmCreatePushButton(buttonRC, "pipePB",  0,0);
   nextPB  = XmCreatePushButton(buttonRC, "nextPB",  0,0);
   prevPB  = XmCreatePushButton(buttonRC, "prevPB",  0,0);
   Widget	okPB   = XmCreatePushButton(buttonRC, "okPB",   0,0);
   Widget	helpPB = XmCreatePushButton(buttonRC, "helpPB", 0,0);

#if 000
   XtManageChild(savePB);
   XtManageChild(printPB);
   XtManageChild(pipePB);
   XtManageChild(nextPB);
   XtManageChild(prevPB);
#endif
   XtManageChild(okPB);
   XtManageChild(helpPB);

#if 000
   XtAddCallback(savePB, XmNactivateCallback, (XtCallbackProc)DoSave,
		 (XtPointer)this);
   XtAddCallback(printPB, XmNactivateCallback, (XtCallbackProc)DoPrint,
		 (XtPointer)this);
   XtAddCallback(pipePB, XmNactivateCallback, (XtCallbackProc)DoPipe,
		 (XtPointer)this);
   XtAddCallback(nextPB, XmNactivateCallback, (XtCallbackProc)DoNext,
		 (XtPointer)this);
   XtAddCallback(prevPB, XmNactivateCallback, (XtCallbackProc)DoPrev,
		 (XtPointer)this);
#endif
   XtAddCallback(okPB, XmNactivateCallback, (XtCallbackProc)DoHide,
		 (XtPointer)this);
   XtAddCallback(helpPB, XmNactivateCallback, (XtCallbackProc)HalAppC::DoHelp,
		 (XtPointer)"helpcard");

   HandleHelp();

} // End constructor

/*---------------------------------------------------------------
 *  Destructor
 */

LocalTextWinC::~LocalTextWinC()
{
   delete rich;
}

/*---------------------------------------------------------------
 *  Method to set whether ">From " strings are to be handled
 */

void
LocalTextWinC::CheckFroms(Boolean val)
{
   rich->CheckFroms(val);
}

/*---------------------------------------------------------------
 *  Method to set plaintext character array
 */

void
LocalTextWinC::SetString(const CharC& c)
{
   rich->SetTextType(TT_PLAIN);
   rich->ForceFixed(True);
   rich->SetString(c);
   rich->ScrollTop();
}

/*---------------------------------------------------------------
 *  Method to set richtext character array
 */

void
LocalTextWinC::SetRichString(const CharC& c)
{
   rich->SetTextType(TT_RICH);
   rich->ForceFixed(False);
   rich->SetString(c);
   rich->ScrollTop();
}

/*---------------------------------------------------------------
 *  Method to set enriched character array
 */

void
LocalTextWinC::SetEnrichedString(const CharC& c)
{
   rich->SetTextType(TT_ENRICHED);
   rich->ForceFixed(False);
   rich->SetString(c);
   rich->ScrollTop();
}

/*---------------------------------------------------------------
 *  Method to set dialog title
 */

void
LocalTextWinC::SetTitle(const char *cs)
{
   XtVaSetValues(*this, XmNtitle, cs, NULL);
}

/*---------------------------------------------------------------
 *  Method to set the number of columns displayed
 */

void
LocalTextWinC::SetSize(int rows, int cols)
{
   rich->SetSize(rows, cols, *this);
}

/*---------------------------------------------------------------
 *  Method to set whether text wraps at the margin.
 */

void
LocalTextWinC::SetWrap(Boolean val)
{
   rich->ResizeWidth(!val);
}
