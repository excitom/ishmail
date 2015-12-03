/*
 *  $Id: FontChooserC.C,v 1.2 2000/05/07 12:26:10 fnevgeny Exp $
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
#include "FontChooserC.h"

#include "WArgList.h"
#include "HalAppC.h"
#include "WXmString.h"
#include "rsrc.h"
#include "CharC.h"
#include "TextMisc.h"

#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <Xm/Label.h>
#include <Xm/TextF.h>
#include <Xm/Text.h>
#include <Xm/List.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>

extern int	debug1, debug2;

#define	MAX_NUM_FONTS		32767
#define	MAX_DISPLAY_SIZE	150
#define	F_FOUNDRY		1	// adobe
#define	F_FAMILY		2	// courier
#define	F_WEIGHT		3	// medium,bold
#define	F_SLANT			4	// r,i,o
#define	F_WIDTH			5	// normal,semicondensed
#define	F_SERIF			6	// sans
#define	F_PIXELS		7	// 12
#define	F_POINTS		8	// 110
#define	F_FIXED			11	// c,p
#define	F_CHARSET1		13	// iso8859
#define	F_CHARSET2		14	// [1-9]
#define MAX_NAME_COMPONENT	14

/*-----------------------------------------------------------------------
 *  Determine whether a family entry should be displayed
 */

inline Boolean
FontChooserC::FamilyOk(CharC family)
{
   return (selectedFamily.size() == 0 || family == selectedFamily);
}

/*-----------------------------------------------------------------------
 *  Determine whether a style entry should be displayed
 */

inline Boolean
FontChooserC::StyleOk(CharC style)
{
   return (selectedStyle.size() == 0 || style == selectedStyle);
}

/*-----------------------------------------------------------------------
 *  Determine whether a size entry should be displayed
 */

inline Boolean
FontChooserC::SizeOk(CharC size)
{
   return (selectedSize.size() == 0 || size == selectedSize);
}

/*-----------------------------------------------------------------------
 *  Get the pixel size component from a font name
 */

inline StringC
FontChooserC::GetPixelSize(char *cs)
{
   return GetNamePart(cs, F_PIXELS);
}

/*-----------------------------------------------------------------------
 *  Determine whether a font is fixed-width
 */

inline Boolean
FontChooserC::IsFixed(char *cs)
{
   CharC	fixed = GetNamePart(cs, F_FIXED);
   return !fixed.Equals('p', IGNORE_CASE);
}

/*-----------------------------------------------------------------------
 *  FontChooserC constructor
 */

FontChooserC::FontChooserC(Widget parent, char* name, ArgList argv,
			   Cardinal argc)
 : HalDialogC(name, parent, argv, argc)
{
   WArgList	args;
   Widget	wlist[8];
   Cardinal	wcount;

//
// Get lists of all fonts
//
   fontNames = XListFonts(halApp->display, "-*-*-*-*-*-*-*-*-*-*-*-*-*-*",
			  MAX_NUM_FONTS, &fontCount);
   fontData  = new FontRecT[fontCount];

//
// Build data structures for each font.
// Also, find the largest font and use it for the sample text field
//
   int 		maxSize  = 0;
   int		maxIndex = 0;
   StringC	sizeStr;
   for (int i=0; i<fontCount; i++) {

      FontRecT	*rec = &fontData[i];
      rec->name      = fontNames[i];
      rec->family    = GetFamily   (rec->name);
      rec->style     = GetStyle    (rec->name);
      rec->pixelSize = GetPixelSize(rec->name);
      rec->pointSize = GetPointSize(rec->name);
      rec->fixed     = IsFixed     (rec->name);

      sizeStr = GetNamePart(rec->name, F_PIXELS);
      int	size = atoi(sizeStr);
      if ( size > maxSize && size < MAX_DISPLAY_SIZE ) {
	 maxIndex = i;
	 maxSize  = size;
      }
   }

   sampleFont     = XLoadQueryFont(halApp->display, fontNames[maxIndex]);
   sampleFontList = XmFontListCreate(sampleFont, XmSTRING_DEFAULT_CHARSET);

//
// Create the appForm children
//
// appForm
//    Form		listForm
//    Frame		propFrame
//    Frame		sizeFrame
//    Frame		sampFrame
//    Frame		nameFrame
//
   args.Reset();
   args.TopAttachment(XmATTACH_NONE);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   Widget	nameFrame  = XmCreateFrame(appForm, "nameFrame", ARGS);

   args.BottomAttachment(XmATTACH_WIDGET, nameFrame);
   Widget	sampFrame = XmCreateFrame(appForm, "sampFrame", ARGS);

   args.RightAttachment(XmATTACH_POSITION, 49);
   args.BottomAttachment(XmATTACH_WIDGET, sampFrame);
   Widget	propFrame = XmCreateFrame(appForm, "propFrame", ARGS);

   args.TopAttachment(XmATTACH_OPPOSITE_WIDGET, propFrame);
   args.LeftAttachment(XmATTACH_POSITION, 51);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_WIDGET, sampFrame);
   Widget	sizeFrame = XmCreateFrame(appForm, "sizeFrame", ARGS);

   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_WIDGET, propFrame);
   Widget	listForm = XmCreateForm(appForm, "listForm", ARGS);

//
// Create listForm hierarchy
//
//   Label	familyLabel
//   Label	styleLabel
//   Label	sizeLabel
//   List	familyListW
//   List	styleListW
//   List	sizeListW
//
   familyLabel = XmCreateLabel(listForm, "familyLabel", 0,0);
   styleLabel  = XmCreateLabel(listForm, "styleLabel",  0,0);
   sizeLabel   = XmCreateLabel(listForm, "sizeLabel",   0,0);

   args.Reset();
   args.SelectionPolicy(XmSINGLE_SELECT);
   args.VisibleItemCount(16);
   familyListW = XmCreateScrolledList(listForm, "familyList", ARGS);
   styleListW  = XmCreateScrolledList(listForm, "styleList",  ARGS);
   sizeListW   = XmCreateScrolledList(listForm, "sizeList",   ARGS);

   XtAddCallback(familyListW, XmNsingleSelectionCallback,
		 (XtCallbackProc)SelectFamily, this);
   XtAddCallback(styleListW, XmNsingleSelectionCallback,
		 (XtCallbackProc)SelectStyle, this);
   XtAddCallback(sizeListW, XmNsingleSelectionCallback,
		 (XtCallbackProc)SelectSize, this);

   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_NONE);
   XtSetValues(familyLabel, ARGS);

   args.Reset();
   args.TopAttachment(XmATTACH_WIDGET, familyLabel);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   XtSetValues(XtParent(familyListW), ARGS);

   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_WIDGET, XtParent(familyListW));
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_NONE);
   XtSetValues(styleLabel, ARGS);

   args.Reset();
   args.TopAttachment(XmATTACH_WIDGET, familyLabel);
   args.LeftAttachment(XmATTACH_OPPOSITE_WIDGET, styleLabel);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   XtSetValues(XtParent(styleListW), ARGS);

   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_WIDGET, XtParent(styleListW));
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_NONE);
   XtSetValues(sizeLabel, ARGS);

   args.Reset();
   args.TopAttachment(XmATTACH_WIDGET, familyLabel);
   args.LeftAttachment(XmATTACH_OPPOSITE_WIDGET, sizeLabel);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   XtSetValues(XtParent(sizeListW), ARGS);

   XtManageChild(familyListW);
   XtManageChild(styleListW);
   XtManageChild(sizeListW);

   wcount = 0;
   wlist[wcount++] = familyLabel;
   wlist[wcount++] = styleLabel;
   wlist[wcount++] = sizeLabel;
   wlist[wcount++] = XtParent(familyListW);
   wlist[wcount++] = XtParent(styleListW);
   wlist[wcount++] = XtParent(sizeListW);
   XtManageChildren(wlist, wcount);	// listForm children

//
// Create propFrame hierarchy
//
//   propFrame
//	RowColumn	propRC
//	   ToggleButton	   showFixedTB
//	   ToggleButton	   showPropTB
//
   args.Reset();
   args.Orientation(XmVERTICAL);
   args.Packing(XmPACK_COLUMN);
   Widget	propRC = XmCreateRowColumn(propFrame, "propRC", ARGS);
   showFixedTB = XmCreateToggleButton(propRC, "showFixedTB", 0,0);
   showPropTB  = XmCreateToggleButton(propRC, "showPropTB",  0,0);

   XmToggleButtonSetState(showFixedTB, True, False);
   XmToggleButtonSetState(showPropTB,  True, False);

   XtAddCallback(showFixedTB, XmNvalueChangedCallback,
   		 (XtCallbackProc)ToggleProp, this);
   XtAddCallback(showPropTB, XmNvalueChangedCallback,
   		 (XtCallbackProc)ToggleProp, this);

   XtManageChild(showFixedTB);
   XtManageChild(showPropTB);
   XtManageChild(propRC);

//
// Create sizeFrame hierarchy
//
//   sizeFrame
//	RadioBox	sizeRC
//	   ToggleButton	   sizePixelsTB
//	   ToggleButton	   sizePointsTB
//
   args.Reset();
   args.Orientation(XmVERTICAL);
   args.Packing(XmPACK_COLUMN);
   Widget	sizeRC = XmCreateRadioBox(sizeFrame, "sizeRC", ARGS);
   sizePixelsTB = XmCreateToggleButton(sizeRC, "sizePixelsTB", 0,0);
   sizePointsTB = XmCreateToggleButton(sizeRC, "sizePointsTB", 0,0);

   XmToggleButtonSetState(sizePixelsTB, True,  False);
   XmToggleButtonSetState(sizePointsTB, False, False);

   XtAddCallback(sizePixelsTB, XmNvalueChangedCallback,
   		 (XtCallbackProc)ToggleSize, this);
   XtAddCallback(sizePointsTB, XmNvalueChangedCallback,
   		 (XtCallbackProc)ToggleSize, this);

   XtManageChild(sizePixelsTB);
   XtManageChild(sizePointsTB);
   XtManageChild(sizeRC);

//
// Create sampFrame hierarchy
//
//   sampFrame
//	Label		sampTitle
//	Text		sampText
//
   args.Reset();
   args.ChildType(XmFRAME_TITLE_CHILD);
   args.ChildHorizontalAlignment(XmALIGNMENT_BEGINNING);
   Widget	sampTitle = XmCreateLabel(sampFrame, "sampTitle", ARGS);

   args.Reset();
   args.FontList(sampleFontList);
   args.EditMode(XmMULTI_LINE_EDIT);
   args.Editable(True);
   sampText = CreateText(sampFrame, "sampText", ARGS);
   char	*cs = XmTextGetString(sampText);
   sampleStr = cs;
   XtFree(cs);

   XtManageChild(sampTitle);
   XtManageChild(sampText);

//
// Create nameForm hierarchy
//
//   sampFrame
//	Label		nameTitle
//	TextField	nameTF
//
   args.Reset();
   args.ChildType(XmFRAME_TITLE_CHILD);
   args.ChildHorizontalAlignment(XmALIGNMENT_BEGINNING);
   Widget	nameTitle = XmCreateLabel(nameFrame, "nameTitle", ARGS);

   nameTF = CreateTextField(nameFrame, "nameTF", 0,0);

   XtManageChild(nameTitle);
   XtManageChild(nameTF);

   wcount = 0;
   wlist[wcount++] = listForm;
   wlist[wcount++] = propFrame;
   wlist[wcount++] = sizeFrame;
   wlist[wcount++] = sampFrame;
   wlist[wcount++] = nameFrame;
   XtManageChildren(wlist, wcount);	// appForm children
   
//
// Create buttonRC hierarchy
//
//   buttonRC
//	PushButton	okPB
//	PushButton	cancelPB
//	PushButton	helpPB
//
   AddButtonBox();
   Widget okPB     = XmCreatePushButton(buttonRC, "okPB",     0,0);
   Widget cancelPB = XmCreatePushButton(buttonRC, "cancelPB", 0,0);
   Widget helpPB   = XmCreatePushButton(buttonRC, "helpPB",   0,0);

   XtAddCallback(okPB,     XmNactivateCallback, (XtCallbackProc)DoOk,   this);
   XtAddCallback(cancelPB, XmNactivateCallback, (XtCallbackProc)DoHide, this);
   XtAddCallback(helpPB,   XmNactivateCallback, (XtCallbackProc)HalAppC::DoHelp,
   		 (XtPointer)"helpcard");

   wlist[0] = okPB;
   wlist[1] = cancelPB;
   wlist[2] = helpPB;
   XtManageChildren(wlist, 3);	// buttonRC children

   HandleHelp();

//
// Initialize the lists and sample text
//
   UpdateLists();
   UpdateSample();

   XtAddCallback(*this, XmNpopupCallback, (XtCallbackProc)DoPopup,
		 (XtPointer)this);

} // End constructor

/*-----------------------------------------------------------------------
 *  Handle initial display
 */

void
FontChooserC::DoPopup(Widget, FontChooserC *This, XtPointer)
{
   WArgList	args;

   args.Reset();
   args.RightAttachment(XmATTACH_SELF);
   args.RightOffset(0);
   XtSetValues(XtParent(This->familyListW), ARGS);
   XtSetValues(XtParent(This->styleListW), ARGS);

} // End DoPopup

/*-----------------------------------------------------------------------
 *  Destructor
 */

FontChooserC::~FontChooserC()
{
   if ( halApp->xRunning ) XFreeFontNames(fontNames);
   delete fontData;
   DeleteCallbacks(okCalls);
}

/*-----------------------------------------------------------------------
 *  Get the family string from a font name
 */

StringC
FontChooserC::GetFamily(char *cs)
{
   CharC	foundry  = GetNamePart(cs, F_FOUNDRY);
   CharC	family   = GetNamePart(cs, F_FAMILY);
   CharC	charset1 = GetNamePart(cs, F_CHARSET1);
   CharC	charset2 = GetNamePart(cs, F_CHARSET2);

   StringC	val;
   val = family;
   val += " (";
   val += foundry;

   if ( !charset1.Equals("iso8859", IGNORE_CASE) || charset2 != "1" ) {
      val += ", ";
      val += charset1;
      val += ' ';
      val += charset2;
   }
   val += ')';

   return val;

} // End GetFamily

/*-----------------------------------------------------------------------
 *  Get the style string from a font name
 */

StringC
FontChooserC::GetStyle(char *cs)
{
   CharC	weight = GetNamePart(cs, F_WEIGHT);
   CharC	width  = GetNamePart(cs, F_WIDTH);
   CharC	serif  = GetNamePart(cs, F_SERIF);
   CharC	slant  = GetNamePart(cs, F_SLANT);

   StringC	val = weight;

   if ( !width.Equals("normal", IGNORE_CASE) ) {
      val += ' ';
      val += width;
   }

   if ( serif.Length() > 0 ) {
      val += ' ';
      val += serif;
   }

   if ( slant.Equals('o', IGNORE_CASE) ) {
      val += ' ';
      val += "oblique";
   }
   else if ( slant.Equals('i', IGNORE_CASE) ) {
      val += ' ';
      val += "italic";
   }

   val.Trim();
   if ( val.size() == 0 ) val = "-";
   return val;

} // End GetStyle

/*-----------------------------------------------------------------------
 *  Get the point size component from a font name
 */

StringC
FontChooserC::GetPointSize(char *cs)
{
   StringC	val = GetNamePart(cs, F_POINTS);

   int	size = atoi(val);
   int	smaj = size / 10;
   int	smin = size % 10;

   val.Clear();
   val += smaj;

   if ( smin != 0 ) {
      val += '.';
      val += smin;
   }

   return val;

} // End GetPointSize

/*-----------------------------------------------------------------------
 *  Get a font name component from the specified position in the name
 */

CharC
FontChooserC::GetNamePart(char *cs, int pos)
{
   CharC	val;
   if ( pos < 0 || pos > MAX_NAME_COMPONENT )
      return val;

//
// Look for '-' between components in name.  Decrement 'pos' for each one found
//
   while (pos > 0 && *cs) {
      if ( *cs == '-') pos--;
      cs++;
   }

   if ( *cs ) {

      char	*start = cs;
      int	len    = 0;

      while ( *cs && *cs != '-' ) {
	 cs++;
	 len++;
      }

      val.Set(start, len);
   }

   return val;

} // End GetNamePart

/*-----------------------------------------------------------------------
 *  Get the font name component from the specified position in the name
 */

void
FontChooserC::UpdateLists(Boolean changeFamily, Boolean changeStyle,
			  Boolean changeSize)
{
   StringListC	familyNames;
   StringListC	styleNames;
   StringListC	sizeNames;

   familyNames.SetSorted(FALSE);
   styleNames.SetSorted(FALSE);
   sizeNames.SetSorted(FALSE);

   familyNames.AllowDuplicates(FALSE);
   styleNames.AllowDuplicates(FALSE);
   sizeNames.AllowDuplicates(FALSE);

   StringC	family;
   StringC	style;
   StringC	size;
   Boolean	showFixed = XmToggleButtonGetState(showFixedTB);
   Boolean	showProp  = XmToggleButtonGetState(showPropTB);
   Boolean	usePixels = XmToggleButtonGetState(sizePixelsTB);

//
// Loop through font names
//
   for (int i=0; i<fontCount; i++) {

      FontRecT	*rec = &fontData[i];

      if ( (rec->fixed && showFixed) || (!rec->fixed && showProp) ) {

	 StringC&	size = usePixels ? rec->pixelSize : rec->pointSize;
	 Boolean	familyOk = FamilyOk(rec->family);
	 Boolean	styleOk  = StyleOk(rec->style);
	 Boolean	sizeOk   = SizeOk(size);

	 if ( changeFamily && styleOk  && sizeOk ) familyNames.add(rec->family);
	 if ( changeStyle  && familyOk && sizeOk )  styleNames.add(rec->style);
	 if ( changeSize   && familyOk && styleOk)   sizeNames.add(size);

      } // End if this font passes

   } // End for each font

//
// Sort and display the lists
//
   if ( changeFamily ) {
      familyNames.sort();
      SetList(familyListW, familyNames);
   }

   if ( changeStyle ) {
      styleNames.sort();
      SetList(styleListW, styleNames);
   }

   if ( changeSize ) {
      sizeNames.sort((int (*)(const void*, const void*))CompareSizes);
      SetList(sizeListW, sizeNames);
   }

} // End UpdateLists

/*-----------------------------------------------------------------------
 *  Compare function for sizes
 */

int
FontChooserC::CompareSizes(const StringC *a, const StringC *b)
{
   int		ia = atoi(*a);
   int		ib = atoi(*b);

   if ( ia > ib ) return  1;
   if ( ib > ia ) return -1;
   return 0;
}

/*-----------------------------------------------------------------------
 *  Method to set an XmList using a StringList
 */

void
FontChooserC::SetList(Widget wlist, StringListC& slist)
{
   int		count = slist.size();
   XmString	*strList = new XmString[count];
   int	i;
   for (i=0; i<count; i++) {
      char	*cs = *slist[i];
      strList[i] = XmStringCreateLtoR(cs, XmFONTLIST_DEFAULT_TAG);
   }

   XtVaSetValues(wlist, XmNitems, strList, XmNitemCount, count, NULL);

   for (i=0; i<count; i++) XmStringFree(strList[i]);
   delete strList;
}

/*-----------------------------------------------------------------------
 *  Display the sample font
 */

void
FontChooserC::UpdateSample()
{
   if ( selectedFamily.size() == 0 ||
        selectedStyle.size()  == 0 ||
	selectedSize.size()   == 0 ) {

//
// Save current sample in case user changed it
//
      char	*cs = XmTextGetString(sampText);
      if ( strlen(cs) > 0 ) sampleStr = cs;
      XtFree(cs);

//
// Clear sample and name since no complete font is selected
//
      XmTextSetString(sampText, "");
      XmTextFieldSetString(nameTF, "");

      return;
   }

//
// Loop through the fonts until we find one that matches all selections
//
   StringC	family;
   StringC	style;
   StringC	size;
   Boolean	showFixed = XmToggleButtonGetState(showFixedTB);
   Boolean	showProp  = XmToggleButtonGetState(showPropTB);
   Boolean	usePixels = XmToggleButtonGetState(sizePixelsTB);

   for (int i=0; i<fontCount; i++) {

      FontRecT	*rec = &fontData[i];

      if ( (rec->fixed && showFixed) || (!rec->fixed && showProp) ) {

	 StringC&	size = usePixels ? rec->pixelSize : rec->pointSize;
	 if ( FamilyOk(rec->family) && StyleOk(rec->style) && SizeOk(size) ) {

//
// Save current string
//
	    char	*cs = XmTextGetString(sampText);
	    if ( strlen(cs) > 0 ) sampleStr = cs;
	    XtFree(cs);
	    XmTextSetString(sampText, "");

//
// Load new font
//
	    XFontStruct	*newFont = XLoadQueryFont(halApp->display, rec->name);
	    XmFontList	newFontList =
	    		   XmFontListCreate(newFont, XmSTRING_DEFAULT_CHARSET);

//
// Display sample text using new font
//
	    XtVaSetValues(sampText, XmNfontList, newFontList, NULL);
	    XmTextSetString(sampText, sampleStr);

//
// Delete previous font
//
	    XFreeFont(halApp->display, sampleFont);
	    XmFontListFree(sampleFontList);
	    sampleFont     = newFont;
	    sampleFontList = newFontList;

//
// Display font name
//
	    XmTextFieldSetString(nameTF, rec->name);

	 } // End if we have a complete font definition

      } // End if this font passes

   } // End for each font

} // End UpdateSample

/*-----------------------------------------------------------------------
 *  Handle toggle of a proportional/fixed flag
 */

void
FontChooserC::ToggleProp(Widget tb, FontChooserC *This,
			 XmToggleButtonCallbackStruct *data)
{
//
// If there is a selected family that is going away, clear the name now.
//
   if ( !data->set && This->selectedFamily.size() > 0 ) {

      Boolean	fixed = (tb == This->showFixedTB);
      for (int i=0; i<This->fontCount; i++) {

	 FontRecT	*rec = &This->fontData[i];
	 if ( rec->fixed == fixed && This->selectedFamily == rec->family ) {

	    XmListDeselectAllItems(This->familyListW);
	    This->selectedFamily.Clear();
	    i = This->fontCount;
	 }

      } // End for each font

   } // End if a selected name could go away

   This->UpdateLists();

} // End ToggleProp

/*-----------------------------------------------------------------------
 *  Handle toggle of a pixels/points flag
 */

void
FontChooserC::ToggleSize(Widget, FontChooserC *This,
			 XmToggleButtonCallbackStruct *tb)
{
   if ( !tb->set ) return;

   This->UpdateLists(False, False, True);
}

/*-----------------------------------------------------------------------
 *  Handle selection of a value in the family list
 */

void
FontChooserC::SelectFamily(Widget, FontChooserC *This, XmListCallbackStruct *l)
{
   WXmString	wstr = l->item;
   char		*val = wstr;

   if ( val == This->selectedFamily ) This->selectedFamily.Clear();
   else				      This->selectedFamily = val;

   XtFree(val);

   This->UpdateLists(False, True, True);
   This->UpdateSample();

} // End SelectFamily

/*-----------------------------------------------------------------------
 *  Handle selection of a value in the style list
 */

void
FontChooserC::SelectStyle(Widget, FontChooserC *This, XmListCallbackStruct *l)
{
   WXmString	wstr = l->item;
   char		*val = wstr;

   if ( val == This->selectedStyle ) This->selectedStyle.Clear();
   else				     This->selectedStyle = val;

   XtFree(val);

   This->UpdateLists(True, False, True);
   This->UpdateSample();

} // End SelectStyle

/*-----------------------------------------------------------------------
 *  Handle selection of a value in the size list
 */

void
FontChooserC::SelectSize(Widget, FontChooserC *This, XmListCallbackStruct *l)
{
   WXmString	wstr = l->item;
   char		*val = wstr;

   if ( val == This->selectedSize ) This->selectedSize.Clear();
   else				    This->selectedSize = val;

   XtFree(val);

   This->UpdateLists(True, True, False);
   This->UpdateSample();

} // End SelectSize

/*-----------------------------------------------------------------------
 *  Handle press of ok button
 */

void
FontChooserC::DoOk(Widget, FontChooserC *This, XtPointer)
{
   if ( XmTextFieldGetLastPosition(This->nameTF) == 0 ) {
      set_invalid(This->nameTF, True, True);
      StringC	errmsg("Please enter a font name.");
      This->PopupMessage(errmsg);
      return;
   }

   char	*cs = XmTextFieldGetString(This->nameTF);
   StringC	name(cs);
   XtFree(cs);
   name.Trim();

   if ( name.size() == 0 ) {
      set_invalid(This->nameTF, True, True);
      StringC	errmsg("Please enter a font name.");
      This->PopupMessage(errmsg);
      return;
   }

   CallCallbacks(This->okCalls, (char*)name);
   This->Hide();

} // End DoOk

/*-----------------------------------------------------------------------
 *  Methods to display certain fonts
 */

void
FontChooserC::ShowProp(Boolean val)
{
   XmToggleButtonSetState(showPropTB, val, True);
}

void
FontChooserC::ShowFixed(Boolean val)
{
   XmToggleButtonSetState(showFixedTB, val, True);
}

void
FontChooserC::AllowProp(Boolean val)
{
   if ( !val )
      XmToggleButtonSetState(showPropTB, False, True);
   XtSetSensitive(showPropTB, val);
}

void
FontChooserC::AllowFixed(Boolean val)
{
   if ( !val )
      XmToggleButtonSetState(showFixedTB, False, True);
   XtSetSensitive(showFixedTB, val);
}
