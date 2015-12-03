/*
 *  $Id: SumPrefWinC.C,v 1.3 2000/05/07 12:26:13 fnevgeny Exp $
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
#include "SumPrefWinC.h"
#include "SumPrefC.h"
#include "IshAppC.h"
#include "MainWinC.h"
#include "UndelWinC.h"

#include <hgl/WArgList.h>
#include <hgl/rsrc.h>
#include <hgl/FieldViewC.h>
#include <hgl/RowColC.h>
#include <hgl/TextMisc.h>

#include <Xm/Label.h>
#include <Xm/ToggleB.h>
#include <Xm/TextF.h>
#include <Xm/Separator.h>
#include <Xm/Form.h>

static StringC	*intStr = NULL;

inline char *
StringOf(int I) {
   if ( !intStr ) intStr = new StringC;
   intStr->Clear();
   *intStr += (I);
   return *intStr;
}

inline char *
FieldOf(int I) {
   return ( (I) == 0 ) ? (char *) "" : StringOf(I);
}

/*---------------------------------------------------------------
 *  Main window constructor
 */

						    // historical name
SumPrefWinC::SumPrefWinC(Widget par) : OptWinC(par, "summaryWin")
{
   WArgList	args;

//
// Create appForm hierarchy
//
// appForm
//    RowColC		fieldRC
//    ToggleButton	iconTB
//    Form		dateFormatForm
//
   args.Reset();
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.TopAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_NONE);
   RowColC	*fieldRC = new RowColC(appForm, "fieldRC", ARGS);

   args.Reset();
   args.TopAttachment(XmATTACH_WIDGET, *fieldRC);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_NONE);
   iconTB      = XmCreateToggleButton(appForm, "iconTB", ARGS);

   args.Reset();
   args.TopAttachment(XmATTACH_WIDGET, iconTB);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_NONE);
   Widget dateFormatForm = XmCreateForm(appForm, "dateFormatForm", ARGS);

//
// Use row-major layout with 5 columns
//
   fieldRC->Defer(True);
   fieldRC->SetOrientation(RcROW_MAJOR);
   fieldRC->SetColCount(9);

//
// Center all columns
//
   fieldRC->SetColAlignment(XmALIGNMENT_CENTER);

//
// Leave the widget sizes alone for all columns except the first and last.
//
   fieldRC->SetColWidthAdjust(RcADJUST_NONE);
   fieldRC->SetColWidthAdjust(0, RcADJUST_EQUAL);
   fieldRC->SetColWidthAdjust(8, RcADJUST_ATTACH);

//
// Allow only the last column to be resized
//
   fieldRC->SetColResize(False);
   fieldRC->SetColResize(8, True);

//
// Create the first row
//
//    Label	fieldTitle
//    Separator	titleSep1
//    Label	posTitle
//    Separator	titleSep2
//    Label	minTitle
//    Separator	titleSep3
//    Label	maxTitle
//    Separator	titleSep4
//    Label	titleTitle
//
   args.Reset();
   args.Alignment(XmALIGNMENT_CENTER);
   Widget	fieldTitle = XmCreateLabel(*fieldRC, "fieldTitle", ARGS);
   Widget	posTitle   = XmCreateLabel(*fieldRC, "posTitle",   ARGS);
   Widget	minTitle   = XmCreateLabel(*fieldRC, "minTitle",   ARGS);
   Widget	maxTitle   = XmCreateLabel(*fieldRC, "maxTitle",   ARGS);
   Widget	titleTitle = XmCreateLabel(*fieldRC, "titleTitle", ARGS);

   args.Reset();
   args.Orientation(XmVERTICAL);
   Widget	titleSep1 = XmCreateSeparator(*fieldRC, "titleSep1", ARGS);
   Widget	titleSep2 = XmCreateSeparator(*fieldRC, "titleSep2", ARGS);
   Widget	titleSep3 = XmCreateSeparator(*fieldRC, "titleSep3", ARGS);
   Widget	titleSep4 = XmCreateSeparator(*fieldRC, "titleSep4", ARGS);

//
// Create the 2nd (number) row
//
//    ToggleButton	numberTB
//    TextField		numberPosTF
//    TextField		numberMinTF
//    TextField		numberMaxTF
//    TextField		numberTitleTF
//
   numberTB      = XmCreateToggleButton(*fieldRC, "numberTB",      0,0);
   numberPosTF   = CreateTextField   (*fieldRC, "posTF",   0,0);
   numberMinTF   = CreateTextField   (*fieldRC, "minTF",   0,0);
   numberMaxTF   = CreateTextField   (*fieldRC, "maxTF",   0,0);
   numberTitleTF = CreateTextField   (*fieldRC, "titleTF", 0,0);

//
// Create the 3rd (status) row
//
//    ToggleButton	statusTB
//    TextField		statusPosTF
//    TextField		statusMinTF
//    TextField		statusMaxTF
//    TextField		statusTitleTF
//
   statusTB      = XmCreateToggleButton(*fieldRC, "statusTB",      0,0);
   statusPosTF   = CreateTextField   (*fieldRC, "posTF",   0,0);
   statusMinTF   = CreateTextField   (*fieldRC, "minTF",   0,0);
   statusMaxTF   = CreateTextField   (*fieldRC, "maxTF",   0,0);
   statusTitleTF = CreateTextField   (*fieldRC, "titleTF", 0,0);

//
// Create the 4th (sender) row
//
//    ToggleButton	senderTB
//    TextField		senderPosTF
//    TextField		senderMinTF
//    TextField		senderMaxTF
//    TextField		senderTitleTF
//
   senderTB      = XmCreateToggleButton(*fieldRC, "senderTB",      0,0);
   senderPosTF   = CreateTextField   (*fieldRC, "posTF",   0,0);
   senderMinTF   = CreateTextField   (*fieldRC, "minTF",   0,0);
   senderMaxTF   = CreateTextField   (*fieldRC, "maxTF",   0,0);
   senderTitleTF = CreateTextField   (*fieldRC, "titleTF", 0,0);

//
// Create the 5th (subject) row
//
//    ToggleButton	subjectTB
//    TextField		subjectPosTF
//    TextField		subjectMinTF
//    TextField		subjectMaxTF
//    TextField		subjectTitleTF
//
   subjectTB      = XmCreateToggleButton(*fieldRC, "subjectTB",      0,0);
   subjectPosTF   = CreateTextField   (*fieldRC, "posTF",   0,0);
   subjectMinTF   = CreateTextField   (*fieldRC, "minTF",   0,0);
   subjectMaxTF   = CreateTextField   (*fieldRC, "maxTF",   0,0);
   subjectTitleTF = CreateTextField   (*fieldRC, "titleTF", 0,0);

//
// Create the 6th (date) row
//
//    ToggleButton	dateTB
//    TextField		datePosTF
//    TextField		dateMinTF
//    TextField		dateMaxTF
//    TextField		dateTitleTF
//
   dateTB      = XmCreateToggleButton(*fieldRC, "dateTB",  0,0);
   datePosTF   = CreateTextField   (*fieldRC, "posTF",   0,0);
   dateMinTF   = CreateTextField   (*fieldRC, "minTF",   0,0);
   dateMaxTF   = CreateTextField   (*fieldRC, "maxTF",   0,0);
   dateTitleTF = CreateTextField   (*fieldRC, "titleTF", 0,0);

//
// Create the 7th (line) row
//
//    ToggleButton	lineTB
//    TextField		linePosTF
//    TextField		lineMinTF
//    TextField		lineMaxTF
//    TextField		lineTitleTF
//
   lineTB      = XmCreateToggleButton(*fieldRC, "lineTB",  0,0);
   linePosTF   = CreateTextField   (*fieldRC, "posTF",   0,0);
   lineMinTF   = CreateTextField   (*fieldRC, "minTF",   0,0);
   lineMaxTF   = CreateTextField   (*fieldRC, "maxTF",   0,0);
   lineTitleTF = CreateTextField   (*fieldRC, "titleTF", 0,0);

//
// Create the 8th (byte) row
//
//    ToggleButton	byteTB
//    TextField		bytePosTF
//    TextField		byteMinTF
//    TextField		byteMaxTF
//    TextField		byteTitleTF
//
   byteTB      = XmCreateToggleButton(*fieldRC, "byteTB",  0,0);
   bytePosTF   = CreateTextField   (*fieldRC, "posTF",   0,0);
   byteMinTF   = CreateTextField   (*fieldRC, "minTF",   0,0);
   byteMaxTF   = CreateTextField   (*fieldRC, "maxTF",   0,0);
   byteTitleTF = CreateTextField   (*fieldRC, "titleTF", 0,0);

//
// Manage widgets
//
   Widget	wlist[10];
   wlist[0] = fieldTitle;
   wlist[1] = titleSep1;
   wlist[2] = posTitle;
   wlist[3] = titleSep2;
   wlist[4] = minTitle;
   wlist[5] = titleSep3;
   wlist[6] = maxTitle;
   wlist[7] = titleSep4;
   wlist[8] = titleTitle;
   fieldRC->AddChildren(wlist, 9);

   wlist[0] = numberTB;
   wlist[1] = NULL;
   wlist[2] = numberPosTF;
   wlist[3] = NULL;
   wlist[4] = numberMinTF;
   wlist[5] = NULL;
   wlist[6] = numberMaxTF;
   wlist[7] = NULL;
   wlist[8] = numberTitleTF;
   fieldRC->AddChildren(wlist, 9);

   wlist[0] = statusTB;
   wlist[1] = NULL;
   wlist[2] = statusPosTF;
   wlist[3] = NULL;
   wlist[4] = statusMinTF;
   wlist[5] = NULL;
   wlist[6] = statusMaxTF;
   wlist[7] = NULL;
   wlist[8] = statusTitleTF;
   fieldRC->AddChildren(wlist, 9);

   wlist[0] = senderTB;
   wlist[1] = NULL;
   wlist[2] = senderPosTF;
   wlist[3] = NULL;
   wlist[4] = senderMinTF;
   wlist[5] = NULL;
   wlist[6] = senderMaxTF;
   wlist[7] = NULL;
   wlist[8] = senderTitleTF;
   fieldRC->AddChildren(wlist, 9);

   wlist[0] = subjectTB;
   wlist[1] = NULL;
   wlist[2] = subjectPosTF;
   wlist[3] = NULL;
   wlist[4] = subjectMinTF;
   wlist[5] = NULL;
   wlist[6] = subjectMaxTF;
   wlist[7] = NULL;
   wlist[8] = subjectTitleTF;
   fieldRC->AddChildren(wlist, 9);

   wlist[0] = dateTB;
   wlist[1] = NULL;
   wlist[2] = datePosTF;
   wlist[3] = NULL;
   wlist[4] = dateMinTF;
   wlist[5] = NULL;
   wlist[6] = dateMaxTF;
   wlist[7] = NULL;
   wlist[8] = dateTitleTF;
   fieldRC->AddChildren(wlist, 9);

   wlist[0] = lineTB;
   wlist[1] = NULL;
   wlist[2] = linePosTF;
   wlist[3] = NULL;
   wlist[4] = lineMinTF;
   wlist[5] = NULL;
   wlist[6] = lineMaxTF;
   wlist[7] = NULL;
   wlist[8] = lineTitleTF;
   fieldRC->AddChildren(wlist, 9);

   wlist[0] = byteTB;
   wlist[1] = NULL;
   wlist[2] = bytePosTF;
   wlist[3] = NULL;
   wlist[4] = byteMinTF;
   wlist[5] = NULL;
   wlist[6] = byteMaxTF;
   wlist[7] = NULL;
   wlist[8] = byteTitleTF;
   fieldRC->AddChildren(wlist, 9);

//
// Center all the rows
//
   fieldRC->SetRowAlignment(XmALIGNMENT_CENTER);

//
// Leave the heights alone in all rows except the first
//
   fieldRC->SetRowHeightAdjust(RcADJUST_NONE);
   fieldRC->SetRowHeightAdjust(0, RcADJUST_EQUAL);

//
// Don't allow the rows to be resized
//
   fieldRC->SetRowResize(False);
   fieldRC->Defer(False);

//
// Create dateFormatForm hierarchy
//
// dateFormatForm
//    Label	dateFormatLabel
//    TextField	dateFormatTF
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   Widget dateFormatLabel =
      XmCreateLabel(dateFormatForm, "dateFormatLabel", ARGS);

   args.LeftAttachment(XmATTACH_WIDGET, dateFormatLabel);
   args.RightAttachment(XmATTACH_FORM);
   dateFormatTF = CreateTextField(dateFormatForm, "dateFormatTF", ARGS);

   XtManageChild(dateFormatLabel);
   XtManageChild(dateFormatTF);

   wlist[0] = *fieldRC;
   wlist[1] = iconTB;
   wlist[2] = dateFormatForm;
   XtManageChildren(wlist, 3);	// appForm children

   HandleHelp();

//
// Initialize values
//
   SumPrefC	*prefs = ishApp->sumPrefs;

   SumFieldC::SumFieldType	t = SumFieldC::MSG_NUM;
   int		 c =  prefs->sumColumn[t];
   SumFieldC	*f = &prefs->sumFieldList[c];
   XmToggleButtonSetState(numberTB, f->show, False);
   XmTextFieldSetString(numberPosTF, StringOf(f->pos));
   XmTextFieldSetString(numberMinTF, FieldOf(f->min));
   XmTextFieldSetString(numberMaxTF, FieldOf(f->max));
   XmTextFieldSetString(numberTitleTF, f->title);

   t = SumFieldC::STATUS;
   c =  prefs->sumColumn[t];
   f = &prefs->sumFieldList[c];
   XmToggleButtonSetState(statusTB, f->show, False);
   XmTextFieldSetString(statusPosTF, StringOf(f->pos));
   XmTextFieldSetString(statusMinTF, FieldOf(f->min));
   XmTextFieldSetString(statusMaxTF, FieldOf(f->max));
   XmTextFieldSetString(statusTitleTF, f->title);

   t = SumFieldC::FROM;
   c =  prefs->sumColumn[t];
   f = &prefs->sumFieldList[c];
   XmToggleButtonSetState(senderTB, f->show, False);
   XmTextFieldSetString(senderPosTF, StringOf(f->pos));
   XmTextFieldSetString(senderMinTF, FieldOf(f->min));
   XmTextFieldSetString(senderMaxTF, FieldOf(f->max));
   XmTextFieldSetString(senderTitleTF, f->title);

   t = SumFieldC::SUBJECT;
   c =  prefs->sumColumn[t];
   f = &prefs->sumFieldList[c];
   XmToggleButtonSetState(subjectTB, f->show, False);
   XmTextFieldSetString(subjectPosTF, StringOf(f->pos));
   XmTextFieldSetString(subjectMinTF, FieldOf(f->min));
   XmTextFieldSetString(subjectMaxTF, FieldOf(f->max));
   XmTextFieldSetString(subjectTitleTF, f->title);

   t = SumFieldC::DATE;
   c =  prefs->sumColumn[t];
   f = &prefs->sumFieldList[c];
   XmToggleButtonSetState(dateTB, f->show, False);
   XmTextFieldSetString(datePosTF, StringOf(f->pos));
   XmTextFieldSetString(dateMinTF, FieldOf(f->min));
   XmTextFieldSetString(dateMaxTF, FieldOf(f->max));
   XmTextFieldSetString(dateTitleTF, f->title);

   t = SumFieldC::LINES;
   c =  prefs->sumColumn[t];
   f = &prefs->sumFieldList[c];
   XmToggleButtonSetState(lineTB, f->show, False);
   XmTextFieldSetString(linePosTF, StringOf(f->pos));
   XmTextFieldSetString(lineMinTF, FieldOf(f->min));
   XmTextFieldSetString(lineMaxTF, FieldOf(f->max));
   XmTextFieldSetString(lineTitleTF, f->title);

   t = SumFieldC::BYTES;
   c =  prefs->sumColumn[t];
   f = &prefs->sumFieldList[c];
   XmToggleButtonSetState(byteTB, f->show, False);
   XmTextFieldSetString(bytePosTF, StringOf(f->pos));
   XmTextFieldSetString(byteMinTF, FieldOf(f->min));
   XmTextFieldSetString(byteMaxTF, FieldOf(f->max));
   XmTextFieldSetString(byteTitleTF, f->title);
   
   XmToggleButtonSetState(iconTB, prefs->showPixmaps, False);
   XmTextFieldSetString(dateFormatTF, prefs->dateFormat);

} // End constructor

/*---------------------------------------------------------------
 *  Method to display window settings
 */

void
SumPrefWinC::Show()
{
   SumPrefC	*prefs = ishApp->sumPrefs;
   FieldViewC&	view   = ishApp->mainWin->FieldView();

//
// Update widths that may have changed
//
   SumFieldC::SumFieldType	t = SumFieldC::MSG_NUM;
   int		 c =  prefs->sumColumn[t];
   SumFieldC	*f = &prefs->sumFieldList[c];
   f->min = view.ColumnMinWidth(c);
   f->max = view.ColumnMaxWidth(c);
   XmTextFieldSetString(numberMinTF, FieldOf(f->min));
   XmTextFieldSetString(numberMaxTF, FieldOf(f->max));

   t = SumFieldC::STATUS;
   c =  prefs->sumColumn[t];
   f = &prefs->sumFieldList[c];
   f->min = view.ColumnMinWidth(c);
   f->max = view.ColumnMaxWidth(c);
   XmTextFieldSetString(statusMinTF, FieldOf(f->min));
   XmTextFieldSetString(statusMaxTF, FieldOf(f->max));

   t = SumFieldC::FROM;
   c =  prefs->sumColumn[t];
   f = &prefs->sumFieldList[c];
   f->min = view.ColumnMinWidth(c);
   f->max = view.ColumnMaxWidth(c);
   XmTextFieldSetString(senderMinTF, FieldOf(f->min));
   XmTextFieldSetString(senderMaxTF, FieldOf(f->max));

   t = SumFieldC::SUBJECT;
   c =  prefs->sumColumn[t];
   f = &prefs->sumFieldList[c];
   f->min = view.ColumnMinWidth(c);
   f->max = view.ColumnMaxWidth(c);
   XmTextFieldSetString(subjectMinTF, FieldOf(f->min));
   XmTextFieldSetString(subjectMaxTF, FieldOf(f->max));

   t = SumFieldC::DATE;
   c =  prefs->sumColumn[t];
   f = &prefs->sumFieldList[c];
   f->min = view.ColumnMinWidth(c);
   f->max = view.ColumnMaxWidth(c);
   XmTextFieldSetString(dateMinTF, FieldOf(f->min));
   XmTextFieldSetString(dateMaxTF, FieldOf(f->max));

   t = SumFieldC::LINES;
   c =  prefs->sumColumn[t];
   f = &prefs->sumFieldList[c];
   f->min = view.ColumnMinWidth(c);
   f->max = view.ColumnMaxWidth(c);
   XmTextFieldSetString(lineMinTF, FieldOf(f->min));
   XmTextFieldSetString(lineMaxTF, FieldOf(f->max));

   t = SumFieldC::BYTES;
   c =  prefs->sumColumn[t];
   f = &prefs->sumFieldList[c];
   f->min = view.ColumnMinWidth(c);
   f->max = view.ColumnMaxWidth(c);
   XmTextFieldSetString(byteMinTF, FieldOf(f->min));
   XmTextFieldSetString(byteMaxTF, FieldOf(f->max));
   
   OptWinC::Show();

} // End Show

/*---------------------------------------------------------------
 *  Method to apply settings
 */

Boolean
SumPrefWinC::Apply()
{
   SumPrefC	*prefs = ishApp->sumPrefs;

   BusyCursor(True);

//
// Initialize the field structures
//
   SumFieldC::SumFieldType	t = SumFieldC::MSG_NUM;
   SumFieldC	*f = &prefs->sumFieldList[t];
   f->Set(t, numberTB, numberPosTF, numberMinTF, numberMaxTF, numberTitleTF);

   t = SumFieldC::STATUS;
   f = &prefs->sumFieldList[t];
   f->Set(t, statusTB, statusPosTF, statusMinTF, statusMaxTF, statusTitleTF);

   t = SumFieldC::FROM;
   f = &prefs->sumFieldList[t];
   f->Set(t, senderTB, senderPosTF, senderMinTF, senderMaxTF, senderTitleTF);

   t = SumFieldC::SUBJECT;
   f = &prefs->sumFieldList[t];
   f->Set(t, subjectTB, subjectPosTF, subjectMinTF, subjectMaxTF,
   	     subjectTitleTF);

   t = SumFieldC::DATE;
   f = &prefs->sumFieldList[t];
   f->Set(t, dateTB, datePosTF, dateMinTF, dateMaxTF, dateTitleTF);

   t = SumFieldC::LINES;
   f = &prefs->sumFieldList[t];
   f->Set(t, lineTB, linePosTF, lineMinTF, lineMaxTF, lineTitleTF);

   t = SumFieldC::BYTES;
   f = &prefs->sumFieldList[t];
   f->Set(t, byteTB, bytePosTF, byteMinTF, byteMaxTF, byteTitleTF);

   Boolean	showPixmaps = XmToggleButtonGetState(iconTB);

//
// Make sure at least one field is selected
//
   Boolean	fieldVis = showPixmaps;
   for (int i=0; !fieldVis && i<SumFieldC::SUMMARY_FIELD_COUNT; i++) {
      f = &prefs->sumFieldList[i];
      if ( f->show ) fieldVis = True;
   }

   if ( !fieldVis ) {
      StringC	errmsg = "You must select at least one field to be displayed.";
      PopupMessage(errmsg);
      return False;
   }

//
// Put fields in order
//
   prefs->SortFields();

   prefs->showPixmaps = showPixmaps;
   char	*cs = XmTextFieldGetString(dateFormatTF);
   prefs->dateFormat = cs;
   XtFree(cs);

//
// Update main and trash windows
//
   ishApp->mainWin->UpdateFields();
   if ( prefs->showPixmaps ) ishApp->mainWin->FieldView().ShowPixmaps();
   else			     ishApp->mainWin->FieldView().HidePixmaps();

   if ( ishApp->undelWin ) {
      UndelWinC	*uw = ishApp->undelWin;
      uw->UpdateFields();
      if ( prefs->showPixmaps ) uw->fieldView->ShowPixmaps();
      else			uw->fieldView->HidePixmaps();
   }

   prefs->WriteDatabase();

//
// Write to file if necessary
//
   if ( applyAll ) prefs->WriteFile();

   BusyCursor(False);

   return True;

} // End Apply
