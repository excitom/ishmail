/*
 *  $Id: FontPrefWinC.C,v 1.2 2000/05/07 12:26:11 fnevgeny Exp $
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
#include "FontPrefWinC.h"
#include "FontPrefC.h"
#include "IshAppC.h"
#include "MainWinC.h"

#include <hgl/WArgList.h>
#include <hgl/WXmString.h>
#include <hgl/TBoxC.h>
#include <hgl/VBoxC.h>
#include <hgl/rsrc.h>
#include <hgl/RowColC.h>
#include <hgl/MimeRichTextC.h>
#include <hgl/StrCase.h>
#include <hgl/TextMisc.h>
#include <hgl/FontChooserC.h>
#include <hgl/FontDataC.h>

#include <Xm/Label.h>
#include <Xm/TextF.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>
#include <Xm/CascadeB.h>
#include <Xm/Text.h>
#include <Xm/List.h>
#include <Xm/Separator.h>

#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>

#define SHOW_MENU	0

/*---------------------------------------------------------------
 *  Main window constructor
 */

FontPrefWinC::FontPrefWinC(Widget par) : OptWinC(par, "fontWin")
{
   WArgList	args;
   Widget 	wlist[16];

   pbFont    = NULL;
   labelFont = NULL;
   textFont  = NULL;
   richFont  = NULL;
   listFont  = NULL;
   chooser   = NULL;

   curTF = NULL;

//
// Create appForm hierarchy
//
// appForm
//    RowColC			fontRC
//       Label			pbLabel
//       TextField		pbTF
//       PushButton		pbChoosePB
//       Label			labelLabel
//       TextField		labelTF
//       PushButton		labelChoosePB
//       Label			textLabel
//       TextField		textTF
//       PushButton		textChoosePB
//       Label			richLabel
//       TextField		richTF
//       PushButton		richChoosePB
//       Label			listLabel
//       TextField		listTF
//       PushButton		listChoosePB
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_NONE);
   RowColC	*fontRC = new RowColC(appForm, "fontRC", ARGS);
   fontRC->Defer(True);

//
// Set up 3 columns
//
   fontRC->SetOrientation(RcROW_MAJOR);
   fontRC->SetColCount(3);
   fontRC->SetColAlignment(XmALIGNMENT_CENTER);
   fontRC->SetColAlignment(0, XmALIGNMENT_END);
   fontRC->SetColWidthAdjust(0, RcADJUST_NONE);
   fontRC->SetColWidthAdjust(1, RcADJUST_ATTACH);
   fontRC->SetColWidthAdjust(2, RcADJUST_EQUAL);
   fontRC->SetColResize(False);
   fontRC->SetColResize(1, True);

   pbLabel       = XmCreateLabel     (*fontRC, "pbLabel",    0,0);
   pbTF          = CreateTextField   (*fontRC, "pbTF",       0,0);
   pbChoosePB    = XmCreatePushButton(*fontRC, "pbChoosePB", 0,0);
   labelLabel    = XmCreateLabel     (*fontRC, "labelLabel", 0,0);
   labelTF       = CreateTextField   (*fontRC, "labelTF",    0,0);
   labelChoosePB = XmCreatePushButton(*fontRC, "labelChoosePB", 0,0);
   textLabel     = XmCreateLabel     (*fontRC, "textLabel",  0,0);
   textTF        = CreateTextField   (*fontRC, "textTF",     0,0);
   textChoosePB  = XmCreatePushButton(*fontRC, "textChoosePB", 0,0);
   richLabel     = XmCreateLabel     (*fontRC, "richLabel",  0,0);
   richTF        = CreateTextField   (*fontRC, "richTF",     0,0);
   richChoosePB  = XmCreatePushButton(*fontRC, "richChoosePB", 0,0);
   listLabel     = XmCreateLabel     (*fontRC, "listLabel",  0,0);
   listTF        = CreateTextField   (*fontRC, "listTF",     0,0);
   listChoosePB  = XmCreatePushButton(*fontRC, "listChoosePB", 0,0);

   XtAddCallback(pbChoosePB, XmNactivateCallback,    (XtCallbackProc)PickFont,
		 (XtPointer)this);
   XtAddCallback(labelChoosePB, XmNactivateCallback, (XtCallbackProc)PickFont,
		 (XtPointer)this);
   XtAddCallback(textChoosePB, XmNactivateCallback,  (XtCallbackProc)PickFont,
		 (XtPointer)this);
   XtAddCallback(richChoosePB, XmNactivateCallback,  (XtCallbackProc)PickFont,
		 (XtPointer)this);
   XtAddCallback(listChoosePB, XmNactivateCallback,  (XtCallbackProc)PickFont,
		 (XtPointer)this);

   Cardinal	wcount = 0;
   wlist[wcount++] = pbLabel;
   wlist[wcount++] = pbTF;
   wlist[wcount++] = pbChoosePB;
   wlist[wcount++] = labelLabel;
   wlist[wcount++] = labelTF;
   wlist[wcount++] = labelChoosePB;
   wlist[wcount++] = textLabel;
   wlist[wcount++] = textTF;
   wlist[wcount++] = textChoosePB;
   wlist[wcount++] = richLabel;
   wlist[wcount++] = richTF;
   wlist[wcount++] = richChoosePB;
   wlist[wcount++] = listLabel;
   wlist[wcount++] = listTF;
   wlist[wcount++] = listChoosePB;
   fontRC->SetChildren(wlist, wcount);
   XtManageChild(*fontRC);	// appForm children
   fontRC->Defer(False);

   HandleHelp();

//
// Initialize fields
//
   FontPrefC	*prefs = ishApp->fontPrefs;
   XmTextFieldSetString(pbTF,    prefs->buttonFont);
   XmTextFieldSetString(labelTF, prefs->labelFont);
   XmTextFieldSetString(textTF,  prefs->textFont);
   XmTextFieldSetString(richTF,  prefs->richFont);
   XmTextFieldSetString(listTF,  prefs->listFont);

} // End constructor

/*---------------------------------------------------------------
 *  Main window destructor
 */

FontPrefWinC::~FontPrefWinC()
{
   delete pbFont;
   delete labelFont;
   delete textFont;
   delete richFont;
   delete listFont;
}

/*---------------------------------------------------------------
 *  Method to apply changes
 */

Boolean
FontPrefWinC::Apply()
{
   BusyCursor(True);
   FontPrefC	*prefs = ishApp->fontPrefs;

//
// Read the name of the pushbutton font
//
   char	*cs = XmTextFieldGetString(pbTF);
   StringC	pbFontStr(cs);
   XtFree(cs);
   pbFontStr.Trim();
   if ( pbFontStr == prefs->buttonFont ) pbFontStr.Clear();

   if ( pbFontStr.size() > 0 ) {

      if ( debuglev > 0 ) cout <<"Push button font is: " <<pbFontStr <<endl;

      if ( pbFont ) *pbFont = pbFontStr;
      else	     pbFont = new FontDataC(pbFontStr);

//
// Try to load this font
//
      if ( !pbFont->loaded ) {
	 StringC	msg("Can't load font: ");
	 msg += pbFontStr;
	 set_invalid(pbTF, True, True);
	 PopupMessage(msg);
	 BusyCursor(False);
	 return False;
      }
   }

//
// Read the name of the label font
//
   cs = XmTextFieldGetString(labelTF);
   StringC	labelFontStr(cs);
   XtFree(cs);
   labelFontStr.Trim();
   if ( labelFontStr == prefs->labelFont ) labelFontStr.Clear();

   if ( labelFontStr.size() > 0 ) {

      if ( debuglev > 0 ) cout <<"Label font is: " <<labelFontStr <<endl;

      if ( labelFont ) *labelFont = labelFontStr;
      else		labelFont = new FontDataC(labelFontStr);

//
// Try to load this font
//
      if ( !labelFont->loaded ) {
	 StringC	msg("Can't load font: ");
	 msg += labelFontStr;
	 set_invalid(labelTF, True, True);
	 PopupMessage(msg);
	 BusyCursor(False);
	 return False;
      }
   }

//
// Read the name of the text font
//
   cs = XmTextFieldGetString(textTF);
   StringC	textFontStr(cs);
   XtFree(cs);
   textFontStr.Trim();
   if ( textFontStr == prefs->textFont ) textFontStr.Clear();

   if ( textFontStr.size() > 0 ) {

      if ( debuglev > 0 ) cout <<"Text font is: " <<textFontStr <<endl;

      if ( textFont ) *textFont = textFontStr;
      else	       textFont = new FontDataC(textFontStr);

//
// Try to load this font
//
      if ( !textFont->loaded ) {
	 StringC	msg("Can't load font: ");
	 msg += textFontStr;
	 set_invalid(textTF, True, True);
	 PopupMessage(msg);
	 BusyCursor(False);
	 return False;
      }
   }

//
// Read the name of the rich font
//
   cs = XmTextFieldGetString(richTF);
   StringC	richFontStr(cs);
   XtFree(cs);
   richFontStr.Trim();
   if ( richFontStr == prefs->richFont ) richFontStr.Clear();

   if ( richFontStr.size() > 0 ) {

      if ( debuglev > 0 ) cout <<"Rich font is: " <<richFontStr <<endl;

      if ( richFont ) *richFont = richFontStr;
      else	       richFont = new FontDataC(richFontStr);

//
// Try to load this font
//
      if ( !richFont->loaded ) {
	 StringC	msg("Can't load font: ");
	 msg += richFontStr;
	 set_invalid(richTF, True, True);
	 PopupMessage(msg);
	 BusyCursor(False);
	 return False;
      }
   }

//
// Read the name of the message list font
//
   cs = XmTextFieldGetString(listTF);
   StringC	listFontStr(cs);
   XtFree(cs);
   listFontStr.Trim();
   if ( listFontStr == prefs->listFont ) listFontStr.Clear();

   if ( listFontStr.size() > 0 ) {

      if ( debuglev > 0 ) cout <<"Message list font is: " <<listFontStr <<endl;

      if ( listFont ) *listFont = listFontStr;
      else	       listFont = new FontDataC(listFontStr);

//
// Try to load this font
//
      if ( !listFont->loaded ) {
	 StringC	msg("Can't load font: ");
	 msg += listFontStr;
	 set_invalid(listTF, True, True);
	 PopupMessage(msg);
	 BusyCursor(False);
	 return False;
      }
   }

//
// Update resources
//
   if ( pbFont && pbFont->loaded && pbFontStr.size() > 0 )
      prefs->buttonFont = pbFontStr;
   if ( labelFont && labelFont->loaded && labelFontStr.size() > 0 )
      prefs->labelFont = labelFontStr;
   if ( textFont && textFont->loaded && textFontStr.size() > 0 )
      prefs->textFont = textFontStr;
   if ( richFont && richFont->loaded && richFontStr.size() > 0 )
      prefs->richFont = richFontStr;
   if ( listFont && listFont->loaded && listFontStr.size() > 0 )
      prefs->listFont = listFontStr;

   prefs->WriteDatabase();
   if ( applyAll ) prefs->WriteFile();

   WArgList	args;

//
// Update the push button font
//
   if ( pbFont && pbFont->loaded && pbFontStr.size() > 0 ) {

//
// Create a font list
//
      XmFontListEntry fent =
	 XmFontListEntryCreate(XmFONTLIST_DEFAULT_TAG, XmFONT_IS_FONT,
			       pbFont->xfont);
      XmFontList	 flist = XmFontListAppendEntry(NULL, fent);

//
// Update the resource for all existing buttons
//
      args.FontList(flist);
      UpdateButtons(*halApp, ARGS);

   } // End if there is a push button font

//
// Update the label font
//
   if ( labelFont && labelFont->loaded && labelFontStr.size() > 0 ) {

//
// Create a font list
//
      XmFontListEntry fent =
       XmFontListEntryCreate(XmFONTLIST_DEFAULT_TAG, XmFONT_IS_FONT,
       			     labelFont->xfont);
      XmFontList	 flist = XmFontListAppendEntry(NULL, fent);

//
// Update the resource for all existing buttons
//
      args.FontList(flist);
      UpdateLabels(*halApp, ARGS);

   } // End if there is a label font

//
// Update the text font
//
   if ( textFontStr.size() > 0 ) {

      if ( textFont && textFont->loaded ) {
//
// Create a font list
//
	 XmFontListEntry fent =
	   XmFontListEntryCreate(XmFONTLIST_DEFAULT_TAG, XmFONT_IS_FONT,
				 textFont->xfont);
	 XmFontList	 flist = XmFontListAppendEntry(NULL, fent);

//
// Update the resource for all existing buttons
//
	 args.FontList(flist);
	 UpdateText(*halApp, ARGS);
      }

      UpdateRichFixed(*halApp, textFontStr);

   } // End if there is a text font

//
// Update the rich font
//
   if ( richFontStr.size() > 0 )
      UpdateRich(*halApp, richFontStr);

//
// Update the message list font
//
   if ( listFont && listFont->loaded && listFontStr.size() > 0 ) {

//
// Create a font list
//
      XmFontListEntry	fent;
      XmFontList	flist;

      if ( listFont->IsBold() ) {
	 fent  = XmFontListEntryCreate("bold", XmFONT_IS_FONT, listFont->xfont);
	 flist = XmFontListAppendEntry(NULL, fent);
	 FontDataC	*nonBold = listFont->NonBold();
	 fent  = XmFontListEntryCreate("plain", XmFONT_IS_FONT, nonBold->xfont);
	 flist = XmFontListAppendEntry(flist, fent);
      }
      else {
	 fent  = XmFontListEntryCreate("plain", XmFONT_IS_FONT,listFont->xfont);
	 flist = XmFontListAppendEntry(NULL, fent);
	 FontDataC	*bold = listFont->Bold();
	 fent  = XmFontListEntryCreate("bold", XmFONT_IS_FONT, bold->xfont);
	 flist = XmFontListAppendEntry(flist, fent);
      }

//
// Update the font for all lists
//
      args.FontList(flist);
      UpdateLists(*halApp, ARGS);

//
// Update the font for all view boxes
//
      ishApp->mainWin->FolderVBox().SetFontList(flist);
      ishApp->mainWin->MsgVBox().SetFontList(flist);

   } // End if there is a list font

   BusyCursor(False);

   return True;

} // End Apply

/*---------------------------------------------------------------
 *  Callback used when a font pushbutton is selected
 */

void
FontPrefWinC::PickFont(Widget w, FontPrefWinC *This, XtPointer)
{
   This->BusyCursor(True);

   if ( !This->chooser ) {
      This->chooser = new FontChooserC(*This);
      This->chooser->AddOkCallback((CallbackFn*)FinishPickFont, This);
   }

   if ( w == This->pbChoosePB ) {
      This->curTF = This->pbTF;
      This->chooser->ShowProp(True);
      This->chooser->ShowFixed(True);
      This->chooser->AllowProp(True);
   }
   else if ( w == This->labelChoosePB ) {
      This->curTF = This->labelTF;
      This->chooser->ShowProp(True);
      This->chooser->ShowFixed(True);
      This->chooser->AllowProp(True);
   }
   else if ( w == This->textChoosePB ) {
      This->curTF = This->textTF;
      This->chooser->ShowProp(False);
      This->chooser->ShowFixed(True);
      This->chooser->AllowProp(False);
   }
   else if ( w == This->richChoosePB ) {
      This->curTF = This->richTF;
      This->chooser->ShowProp(True);
      This->chooser->ShowFixed(True);
      This->chooser->AllowProp(True);
   }
   else if ( w == This->listChoosePB ) {
      This->curTF = This->listTF;
      This->chooser->ShowProp(True);
      This->chooser->ShowFixed(True);
      This->chooser->AllowProp(True);
   }

   This->chooser->Show();

   This->BusyCursor(False);

} // End PickFont

void
FontPrefWinC::FinishPickFont(char *name, FontPrefWinC *This)
{
   if ( debuglev > 0 ) cout <<"Selected font is: " <<name <<endl;
   XmTextFieldSetString(This->curTF, name);
}

/*---------------------------------------------------------------
 *  Callback used when a font menu is about to be posted
 */

void
FontPrefWinC::SetField(Widget w, FontPrefWinC *This, XtPointer)
{
   XtPointer	data;
   XtVaGetValues(w, XmNuserData, &data, NULL);
   This->curTF = (Widget)data;

} // End SetField

/*---------------------------------------------------------------
 *  Method used to update the font for all pushbuttons
 */

void
FontPrefWinC::UpdateButtons(Widget w, ArgList argv, Cardinal argc)
{
   if ( !w || !XtIsWidget(w) ) return;

   if ( debuglev > 1 ) cout <<"Checking " <<w <<" (" <<XtName(w) <<")" <<endl;

//
// If this is a composite, loop through the children and check them.
//    Don't loop through our own menu because we don't want the fonts to
//    change there.
//
   if ( XtIsComposite(w) && w != fontPD ) {

      WidgetList	list;
      Cardinal		count;
      XtVaGetValues(w, XmNnumChildren, &count, XmNchildren, &list, NULL);
      if ( debuglev > 1 ) cout <<"   has " <<count <<" children" <<endl;

//
// Loop through children
//
      for (int i=0; i<count; i++)
	 UpdateButtons(list[i], argv, argc);

   } // End if this is a composite widget
   
//
// If this is a pushbutton or cascade button, update the resources
//
   else if ( XmIsPushButton(w) || XmIsCascadeButton(w) ) {
      if ( debuglev > 1 ) cout <<"Found button: " <<XtName(w) <<endl;
      XtSetValues(w, argv, argc);
   }

//
// If there are any popups associated with this widget, check them
//
   if ( w->core.num_popups > 0 ) {

      if ( debuglev > 1 ) cout <<"   has " <<w->core.num_popups <<" popups" <<endl;
      for (int i=0; i<w->core.num_popups; i++)
	 UpdateButtons(w->core.popup_list[i], argv, argc);

   } // End if there are any popups

} // End UpdateButtons

/*---------------------------------------------------------------
 *  Method used to update the font for all labels and toggle buttons
 */

void
FontPrefWinC::UpdateLabels(Widget w, ArgList argv, Cardinal argc)
{
   if ( !w || !XtIsWidget(w) ) return;

   if ( debuglev > 1 ) cout <<"Checking " <<XtName(w) <<endl;

//
// If this is a composite, loop through the children and check them.
//    Don't loop through our own menu because we don't want the fonts to
//    change there.
//
   if ( XtIsComposite(w) && w != fontPD ) {

      WidgetList	list;
      Cardinal		count;
      XtVaGetValues(w, XmNnumChildren, &count, XmNchildren, &list, NULL);

//
// Loop through children
//
      for (int i=0; i<count; i++)
	 UpdateLabels(list[i], argv, argc);

   } // End if this is a composite widget
   
//
// If this is a pushbutton or cascade button, update the resources
//
   else if ( XmIsLabel(w) && !(XmIsPushButton(w) || XmIsCascadeButton(w)) ) {
      if ( debuglev > 1 ) cout <<"Found label: " <<XtName(w) <<endl;
      XtSetValues(w, argv, argc);
   }

//
// If there are any popups associated with this widget, check them
//
   if ( w->core.num_popups > 0 ) {

      for (int i=0; i<w->core.num_popups; i++)
	 UpdateLabels(w->core.popup_list[i], argv, argc);

   } // End if there are any popups

} // End UpdateLabels

/*---------------------------------------------------------------
 *  Method used to update the font for all text and text field widgets
 */

void
FontPrefWinC::UpdateText(Widget w, ArgList argv, Cardinal argc)
{
   if ( !w || !XtIsWidget(w) ) return;

   if ( debuglev > 1 ) cout <<"Checking " <<XtName(w) <<endl;

//
// If this is a composite, loop through the children and check them.
//    Don't loop through our own menu because we don't want the fonts to
//    change there.
//
   if ( XtIsComposite(w) && w != fontPD ) {

      WidgetList	list;
      Cardinal		count;
      XtVaGetValues(w, XmNnumChildren, &count, XmNchildren, &list, NULL);

//
// Loop through children
//
      for (int i=0; i<count; i++)
	 UpdateText(list[i], argv, argc);

   } // End if this is a composite widget
   
//
// If this is a pushbutton or cascade button, update the resources
//
   else if ( XmIsText(w) || XmIsTextField(w) ) {
      if ( debuglev > 1 ) cout <<"Found text: " <<XtName(w) <<endl;
      XtSetValues(w, argv, argc);
   }

//
// If there are any popups associated with this widget, check them
//
   if ( w->core.num_popups > 0 ) {

      for (int i=0; i<w->core.num_popups; i++)
	 UpdateText(w->core.popup_list[i], argv, argc);

   } // End if there are any popups

} // End UpdateText

/*---------------------------------------------------------------
 *  Method used to update the base font for all richtext widgets
 */

void
FontPrefWinC::UpdateRich(Widget w, const char *fontname)
{
   if ( !w || !XtIsWidget(w) ) return;

   if ( debuglev > 1 ) cout <<"Checking " <<XtName(w) <<endl;

//
// If this is a richtext widget, get the object pointer from its user data
//
   if ( strcasecmp(XtName(w), "richTextArea") == 0 ) {

      MimeRichTextC	*rt;
      XtVaGetValues(w, XmNuserData, &rt, NULL);

      if ( rt ) rt->SetPlainFont(fontname);

   } // End if this is a richtext widget

//
// If this is a composite, loop through the children and check them.
//    Don't loop through our own menu because we don't want the fonts to
//    change there.
//
   if ( XtIsComposite(w) && w != fontPD ) {

      WidgetList	list;
      Cardinal		count;
      XtVaGetValues(w, XmNnumChildren, &count, XmNchildren, &list, NULL);

//
// Loop through children
//
      for (int i=0; i<count; i++)
	 UpdateRich(list[i], fontname);

   } // End if this is a composite widget

//
// If there are any popups associated with this widget, check them
//
   if ( w->core.num_popups > 0 ) {

      for (int i=0; i<w->core.num_popups; i++)
	 UpdateRich(w->core.popup_list[i], fontname);

   } // End if there are any popups
   
} // End UpdateRich

/*---------------------------------------------------------------
 *  Method used to update the fixed font for all richtext widgets
 */

void
FontPrefWinC::UpdateRichFixed(Widget w, const char *fontname)
{
   if ( !w || !XtIsWidget(w) ) return;

   if ( debuglev > 1 ) cout <<"Checking " <<XtName(w) <<endl;

//
// If this is a richtext widget, get the object pointer from its user data
//
   if ( strcasecmp(XtName(w), "richTextArea") == 0 ) {

      MimeRichTextC	*rt;
      XtVaGetValues(w, XmNuserData, &rt, NULL);

      if ( rt ) rt->SetFixedFont(fontname);

   } // End if this is a richtext widget

//
// If this is a composite, loop through the children and check them.
//    Don't loop through our own menu because we don't want the fonts to
//    change there.
//
   if ( XtIsComposite(w) && w != fontPD ) {

      WidgetList	list;
      Cardinal		count;
      XtVaGetValues(w, XmNnumChildren, &count, XmNchildren, &list, NULL);

//
// Loop through children
//
      for (int i=0; i<count; i++)
	 UpdateRichFixed(list[i], fontname);

   } // End if this is a composite widget

//
// If there are any popups associated with this widget, check them
//
   if ( w->core.num_popups > 0 ) {

      for (int i=0; i<w->core.num_popups; i++)
	 UpdateRichFixed(w->core.popup_list[i], fontname);

   } // End if there are any popups
   
} // End UpdateRichFixed

/*---------------------------------------------------------------
 *  Method used to update the font for all list widgets
 */

void
FontPrefWinC::UpdateLists(Widget w, ArgList argv, Cardinal argc)
{
   if ( !w || !XtIsWidget(w) ) return;

   if ( debuglev > 1 ) cout <<"Checking " <<XtName(w) <<endl;

//
// If this is a composite, loop through the children and check them.
//    Don't loop through our own menu because we don't want the fonts to
//    change there.
//
   if ( XtIsComposite(w) && w != fontPD ) {

      WidgetList	list;
      Cardinal		count;
      XtVaGetValues(w, XmNnumChildren, &count, XmNchildren, &list, NULL);

//
// Loop through children
//
      for (int i=0; i<count; i++)
	 UpdateLists(list[i], argv, argc);

   } // End if this is a composite widget
   
//
// If this is a list widget, update the resources
//
   else if ( XmIsList(w) ) {
      if ( debuglev > 1 ) cout <<"Found list: " <<XtName(w) <<endl;
      XtSetValues(w, argv, argc);
   }

//
// If there are any popups associated with this widget, check them
//
   if ( w->core.num_popups > 0 ) {

      for (int i=0; i<w->core.num_popups; i++)
	 UpdateLists(w->core.popup_list[i], argv, argc);

   } // End if there are any popups

} // End UpdateLists
