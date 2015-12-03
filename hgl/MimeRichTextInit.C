/*
 *  $Id: MimeRichTextInit.C,v 1.3 2000/05/31 13:05:30 evgeny Exp $
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
#include "MimeRichTextC.h"
#include "MimeRichTextP.h"
#include "rsrc.h"
#include "HalAppC.h"
#include "StrCase.h"
#include "CharC.h"
#include "WArgList.h"
#include "RegexC.h"

#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/DrawingA.h>
#include <Xm/ScrolledW.h>
#include <Xm/ScrollBar.h>
#include <Xm/AtomMgr.h>
#include <X11/Xatom.h>
#include <X11/Xmu/Atoms.h>

extern int	debuglev;

Atom		MimeRichTextP::mimeRichAtom = (Atom)NULL;

//
// Translations are shared by all
//

XtTranslations  MimeRichTextP::defaultTrans1;
XtTranslations  MimeRichTextP::defaultTrans2;
XtTranslations  MimeRichTextP::delLeftTrans;
XtTranslations  MimeRichTextP::delRightTrans;
XtTranslations  MimeRichTextP::emacsTrans;
XtActionsRec	MimeRichTextP::actions[MIME_ACTION_COUNT] = {
   "MimeRichTextC-move-left-char",	(XtActionProc)ActMoveLeftChar,
   "MimeRichTextC-move-left-word",	(XtActionProc)ActMoveLeftWord,
   "MimeRichTextC-move-right-char",	(XtActionProc)ActMoveRightChar,
   "MimeRichTextC-move-right-word",	(XtActionProc)ActMoveRightWord,
   "MimeRichTextC-move-line-beg",	(XtActionProc)ActMoveLineBeg,
   "MimeRichTextC-move-line-end",	(XtActionProc)ActMoveLineEnd,
   "MimeRichTextC-move-up-line",	(XtActionProc)ActMoveUpLine,
   "MimeRichTextC-move-up-para",	(XtActionProc)ActMoveUpPara,
   "MimeRichTextC-move-down-line",	(XtActionProc)ActMoveDownLine,
   "MimeRichTextC-move-down-para",	(XtActionProc)ActMoveDownPara,
   "MimeRichTextC-move-file-beg",	(XtActionProc)ActMoveFileBeg,
   "MimeRichTextC-move-file-end",	(XtActionProc)ActMoveFileEnd,
   "MimeRichTextC-delete-left-char",	(XtActionProc)ActDeleteLeftChar,
   "MimeRichTextC-delete-left-word",	(XtActionProc)ActDeleteLeftWord,
   "MimeRichTextC-delete-right-char",	(XtActionProc)ActDeleteRightChar,
   "MimeRichTextC-delete-right-word",	(XtActionProc)ActDeleteRightWord,
   "MimeRichTextC-delete-line-beg",	(XtActionProc)ActDeleteLineBeg,
   "MimeRichTextC-delete-line-end",	(XtActionProc)ActDeleteLineEnd,
   "MimeRichTextC-select-begin",	(XtActionProc)ActSelectBegin,
   "MimeRichTextC-select-motion",	(XtActionProc)ActSelectMotion,
   "MimeRichTextC-select-extend",	(XtActionProc)ActSelectExtend,
   "MimeRichTextC-select-end",		(XtActionProc)ActSelectEnd,
   "MimeRichTextC-select-left-char",	(XtActionProc)ActSelectLeftChar,
   "MimeRichTextC-select-left-word",	(XtActionProc)ActSelectLeftWord,
   "MimeRichTextC-select-right-char",	(XtActionProc)ActSelectRightChar,
   "MimeRichTextC-select-right-word",	(XtActionProc)ActSelectRightWord,
   "MimeRichTextC-select-line-beg",	(XtActionProc)ActSelectLineBeg,
   "MimeRichTextC-select-line-end",	(XtActionProc)ActSelectLineEnd,
   "MimeRichTextC-select-up-line",	(XtActionProc)ActSelectUpLine,
   "MimeRichTextC-select-up-para",	(XtActionProc)ActSelectUpPara,
   "MimeRichTextC-select-down-line",	(XtActionProc)ActSelectDownLine,
   "MimeRichTextC-select-down-para",	(XtActionProc)ActSelectDownPara,
   "MimeRichTextC-select-file-beg",	(XtActionProc)ActSelectFileBeg,
   "MimeRichTextC-select-file-end",	(XtActionProc)ActSelectFileEnd,
   "MimeRichTextC-delete-selection",	(XtActionProc)ActDeleteSelection,
   "MimeRichTextC-cut-selection",	(XtActionProc)ActCutSelection,
   "MimeRichTextC-copy-selection",	(XtActionProc)ActCopySelection,
   "MimeRichTextC-paste",		(XtActionProc)ActPaste,
   "MimeRichTextC-undo",		(XtActionProc)ActUndo,
   "MimeRichTextC-insert-self",		(XtActionProc)ActInsertSelf,
   "MimeRichTextC-refresh",		(XtActionProc)ActRefresh,
   "MimeRichTextC-plain",		(XtActionProc)ActPlain,
   "MimeRichTextC-bold",		(XtActionProc)ActBold,
   "MimeRichTextC-italic",		(XtActionProc)ActItalic,
   "MimeRichTextC-fixed",		(XtActionProc)ActFixed,
   "MimeRichTextC-smaller",		(XtActionProc)ActSmaller,
   "MimeRichTextC-bigger",		(XtActionProc)ActBigger,
   "MimeRichTextC-underline",		(XtActionProc)ActUnderline,
   "MimeRichTextC-center",		(XtActionProc)ActCenter,
   "MimeRichTextC-flush-left",		(XtActionProc)ActFlushLeft,
   "MimeRichTextC-flush-right",		(XtActionProc)ActFlushRight,
   "MimeRichTextC-flush-both",		(XtActionProc)ActFlushBoth,
   "MimeRichTextC-no-fill",		(XtActionProc)ActNoFill,
   "MimeRichTextC-left-margin-in",	(XtActionProc)ActLeftMarginIn,
   "MimeRichTextC-left-margin-out",	(XtActionProc)ActLeftMarginOut,
   "MimeRichTextC-right-margin-in",	(XtActionProc)ActRightMarginIn,
   "MimeRichTextC-right-margin-out",	(XtActionProc)ActRightMarginOut,
   "MimeRichTextC-excerpt-more",	(XtActionProc)ActExcerptMore,
   "MimeRichTextC-excerpt-less",	(XtActionProc)ActExcerptLess,
   "MimeRichTextC-scroll-up-line",	(XtActionProc)ActScrollUpLine,
   "MimeRichTextC-scroll-up-page",	(XtActionProc)ActScrollUpPage,
   "MimeRichTextC-scroll-down-line",	(XtActionProc)ActScrollDownLine,
   "MimeRichTextC-scroll-down-page",	(XtActionProc)ActScrollDownPage,
   "MimeRichTextC-scroll-top",		(XtActionProc)ActScrollTop,
   "MimeRichTextC-scroll-bottom",	(XtActionProc)ActScrollBottom,
   "MimeRichTextC-line-to-top",		(XtActionProc)ActLineToTop,
   "MimeRichTextC-line-to-center",	(XtActionProc)ActLineToCenter,
   "MimeRichTextC-line-to-bottom",	(XtActionProc)ActLineToBottom,
   "MimeRichTextC-follow-url",		(XtActionProc)ActFollowURL,
   "MimeRichTextC-search",		(XtActionProc)ActSearch,
   "MimeRichTextC-ignore",		(XtActionProc)ActIgnore,
   "MimeRichTextC-post-menu",		(XtActionProc)ActPostMenu
};

#if 0
#endif

/*----------------------------------------------------------------------
 * double click table for cut and paste in 8 bits
 *
 * This table is divided in four parts :
 *
 *	- control characters	[0,0x1f] U [0x80,0x9f]
 *	- separators		[0x20,0x3f] U [0xa0,0xb9]
 *	- binding characters	[0x40,0x7f] U [0xc0,0xff]
 *  	- execeptions
 */

int MimeRichTextP::charClasses[256] = {
/* NUL  SOH  STX  ETX  EOT  ENQ  ACK  BEL */
    32,   1,   1,   1,   1,   1,   1,   1,
/*  BS   HT   NL   VT   NP   CR   SO   SI */
     1,  32,   1,   1,   1,   1,   1,   1,
/* DLE  DC1  DC2  DC3  DC4  NAK  SYN  ETB */
     1,   1,   1,   1,   1,   1,   1,   1,
/* CAN   EM  SUB  ESC   FS   GS   RS   US */
     1,   1,   1,   1,   1,   1,   1,   1,
/*  SP    !    "    #    $    %    &    ' */
    32,  33,  34,  35,  36,  37,  38,  39,
/*   (    )    *    +    ,    -    .    / */
    40,  41,  42,  43,  44,  45,  46,  47,
/*   0    1    2    3    4    5    6    7 */
    48,  48,  48,  48,  48,  48,  48,  48,
/*   8    9    :    ;    <    =    >    ? */
    48,  48,  58,  59,  60,  61,  62,  63,
/*   @    A    B    C    D    E    F    G */
    64,  48,  48,  48,  48,  48,  48,  48,
/*   H    I    J    K    L    M    N    O */
    48,  48,  48,  48,  48,  48,  48,  48,
/*   P    Q    R    S    T    U    V    W */ 
    48,  48,  48,  48,  48,  48,  48,  48,
/*   X    Y    Z    [    \    ]    ^    _ */
    48,  48,  48,  91,  92,  93,  94,  48,
/*   `    a    b    c    d    e    f    g */
    96,  48,  48,  48,  48,  48,  48,  48,
/*   h    i    j    k    l    m    n    o */
    48,  48,  48,  48,  48,  48,  48,  48,
/*   p    q    r    s    t    u    v    w */
    48,  48,  48,  48,  48,  48,  48,  48,
/*   x    y    z    {    |    }    ~  DEL */
    48,  48,  48, 123, 124, 125, 126,   1,
/* x80  x81  x82  x83  IND  NEL  SSA  ESA */
     1,   1,   1,   1,   1,   1,   1,   1,
/* HTS  HTJ  VTS  PLD  PLU   RI  SS2  SS3 */
     1,   1,   1,   1,   1,   1,   1,   1,
/* DCS  PU1  PU2  STS  CCH   MW  SPA  EPA */
     1,   1,   1,   1,   1,   1,   1,   1,
/* x98  x99  x9A  CSI   ST  OSC   PM  APC */
     1,   1,   1,   1,   1,   1,   1,   1,
/*   -    i   c/    L   ox   Y-    |   So */
   160, 161, 162, 163, 164, 165, 166, 167,
/*  ..   c0   ip   <<    _        R0    - */
   168, 169, 170, 171, 172, 173, 174, 175,
/*   o   +-    2    3    '    u   q|    . */
   176, 177, 178, 179, 180, 181, 182, 183,
/*   ,    1    2   >>  1/4  1/2  3/4    ? */
   184, 185, 186, 187, 188, 189, 190, 191,
/*  A`   A'   A^   A~   A:   Ao   AE   C, */
    48,  48,  48,  48,  48,  48,  48,  48,
/*  E`   E'   E^   E:   I`   I'   I^   I: */
    48,  48,  48,  48,  48,  48,  48,  48,
/*  D-   N~   O`   O'   O^   O~   O:    X */ 
    48,  48,  48,  48,  48,  48,  48, 216,
/*  O/   U`   U'   U^   U:   Y'    P    B */
    48,  48,  48,  48,  48,  48,  48,  48,
/*  a`   a'   a^   a~   a:   ao   ae   c, */
    48,  48,  48,  48,  48,  48,  48,  48,
/*  e`   e'   e^   e:    i`  i'   i^   i: */
    48,  48,  48,  48,  48,  48,  48,  48,
/*   d   n~   o`   o'   o^   o~   o:   -: */
    48,  48,  48,  48,  48,  48,  48,  248,
/*  o/   u`   u'   u^   u:   y'    P   y: */
    48,  48,  48,  48,  48,  48,  48,  48};

/*----------------------------------------------------------------------
 * Constructor
 */

MimeRichTextC::MimeRichTextC(Widget parent, const char *name, ArgList argv,
			     Cardinal argc)
{
   priv = new MimeRichTextP;
   priv->pub = this;

   priv->realized        = False;
   priv->defer           = 0;
   priv->changed         = False;
   priv->textGC          = NULL;
   priv->cursorHideCount = 0;
   priv->hasFocus        = False;
   priv->clickCount      = 0;
   priv->cursorTimer     = (XtIntervalId)NULL;
   priv->clickTimer      = (XtIntervalId)NULL;
   priv->scrollTimer     = (XtIntervalId)NULL;
   priv->selectOn	 = False;
   priv->okToEndSelection= False;
   priv->selectTime	 = 0;
   priv->mousePaste	 = False;
   priv->sendExcerpt	 = True;
   priv->topTextLine     = NULL;
   priv->botTextLine     = NULL;
   priv->topSaveLine     = NULL;
   priv->botSaveLine     = NULL;
   priv->topSoftLine     = NULL;
   priv->botSoftLine     = NULL;
   priv->inputLine	 = NULL;
   priv->inputCmd	 = NULL;
   priv->checkFroms      = False;
   priv->inputTextBuf.AutoShrink(False);
   priv->textPm          = (Pixmap)NULL;
   priv->stipplePm       = (Pixmap)NULL;
   priv->hsbAlwaysOn     = False;
   priv->vsbAlwaysOn     = False;

//
// Save args passed in
//
   WArgList	args;
   int i=0; for (i=0; i<argc; i++) {
      Arg	*arg = &argv[i];
      args.Add(arg->name, arg->value);
   }

   char		*cl = "MimeRichTextC";

//
// If we're in a scrolled window, this is the widget hierarchy:
//
// ScrolledWindow		textSW
//    Form			nameForm
//       MimeRichTextC		name
//
   if ( XmIsScrolledWindow(parent) ) {

      priv->textSW     = parent;
      priv->hlForm     = NULL;
      priv->scrollForm = NULL;
      priv->textFrame  = NULL;
      priv->textVSB    = NULL;
      priv->textHSB    = NULL;
      priv->hlThick    = 0;
   }

//
// If we're not in a scrolled window, this is the widget hierarchy:
//
// ???				parent
//    Form			name
//       Form			highlightForm
//          Frame		textFrame
//             MimeRichTextC	textArea
//       ScrollBar		textVSB
//       ScrollBar		textHSB
//
   else {

      priv->textSW   = NULL;
      priv->textClip = NULL;

      args.ResizePolicy(XmRESIZE_NONE);
      priv->scrollForm = XmCreateForm(parent, (char*)name, ARGS);

      args.Reset();
      args.LeftAttachment(XmATTACH_FORM);
      args.RightAttachment(XmATTACH_FORM);
      args.TopAttachment(XmATTACH_FORM);
      args.BottomAttachment(XmATTACH_FORM);
      args.MarginWidth(0);
      args.MarginHeight(0);
      args.ShadowThickness(0);
      args.BorderWidth(0);
      priv->hlForm = XmCreateForm(priv->scrollForm, "richHighlightForm", ARGS);

      priv->hlThick = get_int(cl, MainWidget(), "highlightThickness", 1);

      args.Reset();
      args.MarginWidth(0);
      args.MarginHeight(0);
      args.BorderWidth(0);
      args.LeftAttachment(XmATTACH_FORM, priv->hlThick);
      args.RightAttachment(XmATTACH_FORM, priv->hlThick);
      args.TopAttachment(XmATTACH_FORM, priv->hlThick);
      args.BottomAttachment(XmATTACH_FORM, priv->hlThick);
      priv->textFrame = XmCreateFrame(priv->hlForm, "richTextFrame", ARGS);

      args.Reset();
      args.LeftAttachment(XmATTACH_OPPOSITE_WIDGET, priv->hlForm);
      args.RightAttachment(XmATTACH_OPPOSITE_WIDGET, priv->hlForm);
      args.TopAttachment(XmATTACH_NONE);
      args.BottomAttachment(XmATTACH_FORM);
      args.Orientation(XmHORIZONTAL);
      args.ProcessingDirection(XmMAX_ON_RIGHT);
      args.ShowArrows(True);
      args.MappedWhenManaged(False);
      priv->textHSB = XmCreateScrollBar(priv->scrollForm, "richTextHSB", ARGS);
      priv->hsbAlwaysOn = get_boolean(priv->textHSB, "alwaysOn", False);

      args.Reset();
      args.LeftAttachment(XmATTACH_NONE);
      args.RightAttachment(XmATTACH_FORM);
      args.TopAttachment(XmATTACH_OPPOSITE_WIDGET, priv->hlForm);
      args.BottomAttachment(XmATTACH_OPPOSITE_WIDGET, priv->hlForm);
      args.Orientation(XmVERTICAL);
      args.ProcessingDirection(XmMAX_ON_BOTTOM);
      args.ShowArrows(True);
      args.MappedWhenManaged(False);
      priv->textVSB = XmCreateScrollBar(priv->scrollForm, "richTextVSB", ARGS);
      priv->vsbAlwaysOn = get_boolean(priv->textVSB, "alwaysOn", False);

      XtAddCallback(priv->textHSB, XmNdecrementCallback,
		    (XtCallbackProc)MimeRichTextP::HandleHScroll, priv);
      XtAddCallback(priv->textHSB, XmNdragCallback,
		    (XtCallbackProc)MimeRichTextP::HandleHScroll, priv);
      XtAddCallback(priv->textHSB, XmNincrementCallback,
		    (XtCallbackProc)MimeRichTextP::HandleHScroll, priv);
      XtAddCallback(priv->textHSB, XmNpageDecrementCallback,
		    (XtCallbackProc)MimeRichTextP::HandleHScroll, priv);
      XtAddCallback(priv->textHSB, XmNpageIncrementCallback,
		    (XtCallbackProc)MimeRichTextP::HandleHScroll, priv);
      XtAddCallback(priv->textHSB, XmNtoBottomCallback,
		    (XtCallbackProc)MimeRichTextP::HandleHScroll, priv);
      XtAddCallback(priv->textHSB, XmNtoTopCallback,
		    (XtCallbackProc)MimeRichTextP::HandleHScroll, priv);
      XtAddCallback(priv->textHSB, XmNvalueChangedCallback,
		    (XtCallbackProc)MimeRichTextP::HandleHScroll, priv);

      XtAddCallback(priv->textVSB, XmNdecrementCallback,
		    (XtCallbackProc)MimeRichTextP::HandleVScroll, priv);
      XtAddCallback(priv->textVSB, XmNdragCallback,
		    (XtCallbackProc)MimeRichTextP::HandleVScroll, priv);
      XtAddCallback(priv->textVSB, XmNincrementCallback,
		    (XtCallbackProc)MimeRichTextP::HandleVScroll, priv);
      XtAddCallback(priv->textVSB, XmNpageDecrementCallback,
		    (XtCallbackProc)MimeRichTextP::HandleVScroll, priv);
      XtAddCallback(priv->textVSB, XmNpageIncrementCallback,
		    (XtCallbackProc)MimeRichTextP::HandleVScroll, priv);
      XtAddCallback(priv->textVSB, XmNtoBottomCallback,
		    (XtCallbackProc)MimeRichTextP::HandleVScroll, priv);
      XtAddCallback(priv->textVSB, XmNtoTopCallback,
		    (XtCallbackProc)MimeRichTextP::HandleVScroll, priv);
      XtAddCallback(priv->textVSB, XmNvalueChangedCallback,
		    (XtCallbackProc)MimeRichTextP::HandleVScroll, priv);

      parent = priv->textFrame;

//
// Get offsets to use if scrollbars are not displayed
//
      XtVaGetValues(priv->hlForm,
		    XmNrightOffset,  &priv->noScrollRoff,
		    XmNbottomOffset, &priv->noScrollBoff, NULL);

//
// Get size and offset for scrollbars
//
      int	roff, boff;
      Dimension	sbwd, sbht;
      XtVaGetValues(priv->textVSB,
		    XmNrightOffset,  &roff, XmNwidth,  &sbwd, NULL);
      XtVaGetValues(priv->textHSB,
		    XmNbottomOffset, &boff, XmNheight, &sbht, NULL);

//
// Get offsets to use if scrollbars are displayed
//
      priv->scrollRoff = priv->noScrollRoff + sbwd + roff;
      priv->scrollBoff = priv->noScrollBoff + sbht + boff;

//
// Place scroll bars if they're always on
//
      if ( priv->vsbAlwaysOn ) {
	 XtVaSetValues(priv->hlForm,  XmNrightOffset, priv->scrollRoff, NULL);
	 XtVaSetValues(priv->textVSB, XmNmappedWhenManaged, True, NULL);
      }
      else {
	 XtVaSetValues(priv->hlForm,  XmNrightOffset, priv->noScrollRoff, NULL);
      }

      if ( priv->hsbAlwaysOn ) {
	 XtVaSetValues(priv->hlForm,  XmNbottomOffset, priv->scrollBoff, NULL);
	 XtVaSetValues(priv->textHSB, XmNmappedWhenManaged, True, NULL);
      }
      else {
	 XtVaSetValues(priv->hlForm,  XmNbottomOffset, priv->noScrollBoff,NULL);
      }

   } // End if we're creating our own scroll bars

   priv->hsbOn  = priv->hsbAlwaysOn;
   priv->vsbOn  = priv->vsbAlwaysOn;
   priv->hsbVal = priv->vsbVal = 0;
   priv->hsbMax = priv->vsbMax = 0;

   args.Reset();
   args.ResizePolicy(XmRESIZE_NONE);
   args.MarginWidth(0);
   args.MarginHeight(0);
   args.ShadowThickness(0);
   args.BorderWidth(0);
   args.UserData(this);
   priv->textDA = XmCreateDrawingArea(parent, "richTextArea", ARGS);

//
// Read and load fonts so we can get size of rows and columns
//
   priv->defPlainFontName = get_string(cl, MainWidget(), "font",
				       "-misc-fixed-medium-r-normal--13*");
   priv->defFixedFontName = get_string(cl, MainWidget(), "fixedFont",
				       "-misc-fixed-medium-r-normal--13*");
   priv->plainFont    = priv->defPlainFontName;
   priv->fixedFont    = priv->defFixedFontName;
   priv->lineSpacing  = (char)get_int(cl, MainWidget(), "lineSpacing",  1);
   priv->forceFixed   = get_boolean(cl, MainWidget(), "forceFixed",   False);

//
// Set the margins
//
   priv->marginWd  = (Dimension)get_int(cl, MainWidget(), "marginWidth",  5);
   priv->marginHt  = (Dimension)get_int(cl, MainWidget(), "marginHeight", 5);
   priv->marginWd2 = (Dimension)2 * priv->marginWd;
   priv->marginHt2 = (Dimension)2 * priv->marginHt;

//
// Set width based on row and column counts if necessary
//
   priv->singleLine = get_boolean(cl, MainWidget(), "singleLine",   False);
   int	colCount = get_int(cl, MainWidget(), "columnCount", 0);
   int	rowCount = get_int(cl, MainWidget(), "rowCount",    0);

//
// See if rows or columns was passed in
//
   for (i=0; i<argc; i++) {
      Arg	*arg = &argv[i];
      if ( strcasecmp(arg->name, "rows") == 0 )
	 rowCount = (int)arg->value;
      else if ( strcasecmp(arg->name, "columns") == 0 )
	 colCount = (int)arg->value;
   }

   if ( priv->singleLine ) rowCount = 1;

//
// Set size based on rows and columns if necessary
//
   if ( colCount > 0 ) {
      priv->drawWd = CharWidth() * colCount + priv->marginWd2;
      XtVaSetValues(priv->textDA, XmNwidth, priv->drawWd, NULL);
   }
   else
      XtVaGetValues(priv->textDA, XmNwidth, &priv->drawWd, NULL);

   if ( rowCount > 0 ) {
      priv->drawHt = ((CharHeight() + priv->lineSpacing) * rowCount)
      		   - priv->lineSpacing + 2/*for accents on top line*/
		   + priv->marginHt2;
      XtVaSetValues(priv->textDA, XmNheight, priv->drawHt, NULL);
   }
   else
      XtVaGetValues(priv->textDA, XmNheight, &priv->drawHt, NULL);

   priv->maxWd = priv->drawWd;
   priv->maxHt = priv->drawHt;

   XtManageChild(priv->textDA);

   if ( !priv->textSW ) {
      XtManageChild(priv->textFrame);
      XtManageChild(priv->hlForm);
      XtManageChild(priv->textHSB);
      XtManageChild(priv->textVSB);
   }

   XtAddCallback(priv->textDA, XmNexposeCallback,
   		 (XtCallbackProc)MimeRichTextP::HandleExpose, priv);

//
// Read resources
//
   priv->tabStop     = (char)get_int(cl, MainWidget(), "tabStop",	 8);
   priv->indentWd    =       get_int(cl, MainWidget(), "indentWidth", 32);
   priv->autoScrollInterval
		     = get_int(cl, MainWidget(), "autoScrollInterval", 10);
   priv->maxLength   = get_int(cl, MainWidget(), "maxTextLength", 200000);
   priv->timerOk     = get_boolean(cl, MainWidget(), "timerOk", True);

   StringC	typeStr = get_string(cl, MainWidget(), "textType", "plain");
   if ( strcasecmp(typeStr, "rich")          == 0 ||
        strcasecmp(typeStr, "richtext")      == 0 ||
        strcasecmp(typeStr, "text/richtext") == 0 )
      priv->textType = TT_RICH;
   else if ( strcasecmp(typeStr, "enriched")      == 0 ||
	     strcasecmp(typeStr, "text/enriched") == 0 )
      priv->textType = TT_ENRICHED;
   else if ( strcasecmp(typeStr, "html")        == 0 ||
	     strcasecmp(typeStr, "x-html")      == 0 ||
	     strcasecmp(typeStr, "text/x-html") == 0 ||
	     strcasecmp(typeStr, "text/html")   == 0 )
      priv->textType = TT_HTML;
   else
      priv->textType = TT_PLAIN;

   if ( priv->singleLine )
      priv->resizeWidth = True;
   else
      priv->resizeWidth = get_boolean(cl, MainWidget(), "resizeWidth", False);

   priv->fgColor  =
      get_color(cl, MainWidget(), "foreground",     XtDefaultForeground);
   priv->bgColor  =
      get_color(cl, MainWidget(), "background",     XtDefaultBackground);
   priv->urlColor =
      get_color(cl, MainWidget(), "urlColor",       priv->fgColor);
   priv->linkColor =
      get_color(cl, MainWidget(), "linkColor",      priv->urlColor);
   priv->hlColor  =
      get_color(cl, MainWidget(), "highlightColor", priv->fgColor);
   if ( priv->hlForm )
      XtVaGetValues(priv->hlForm, XmNbackground, &priv->hlFormBg, NULL);
   priv->cursorColor = priv->fgColor ^ priv->bgColor;

   XtVaSetValues(priv->textDA, XmNbackground, priv->bgColor,
			       XmNforeground, priv->fgColor, NULL);

//
// Initialize character set
//
   char	*cs = StrCaseStr(priv->defPlainFontName, "iso8859");
   if ( cs ) priv->defCharset = cs;
   else	     priv->defCharset = "us-ascii";
   priv->defCharset = get_string(cl, MainWidget(), "charset", priv->defCharset);

   if ( priv->defCharset.StartsWith("iso-", IGNORE_CASE) )
      priv->defCharset(3,1) = "";	// Remove '-'

   if ( debuglev > 1 ) cout <<"Character set is " <<priv->defCharset <<endl;
   priv->curCharset = AddCharset(priv->defCharset);

//
// Get size of excerpt string
//
   priv->excerptStr = get_string(cl, MainWidget(), "excerptPrefix", "> ");
   int	dir, asc, dsc;
   XCharStruct	size;
   XTextExtents(priv->plainFont.xfont, priv->excerptStr,
   		priv->excerptStr.size(), &dir, &asc, &dsc, &size);
   priv->excerptWd = size.width;

//
// Add actions and translations if necessary
//
   static Boolean       actionsAdded = False;
   if ( !actionsAdded ) {

      XtAppAddActions(halApp->context, priv->actions, XtNumber(priv->actions));
      actionsAdded = True;

//
// These translations cannot be specified in a resource file due to the fact
//    that Motif overrides them after the resource file is processed.
//
      priv->defaultTrans1 = XtParseTranslationTable("\
c s	<Key>osfLeft:		MimeRichTextC-select-left-word()\n\
c	<Key>osfLeft:		MimeRichTextC-move-left-word()\n\
s	<Key>osfLeft:		MimeRichTextC-select-left-char()\n\
	<Key>osfLeft:		MimeRichTextC-move-left-char()\n\
c s	<Key>osfRight:		MimeRichTextC-select-right-word()\n\
c	<Key>osfRight:		MimeRichTextC-move-right-word()\n\
s	<Key>osfRight:		MimeRichTextC-select-right-char()\n\
	<Key>osfRight:		MimeRichTextC-move-right-char()\n\
c s	<Key>osfUp:		MimeRichTextC-select-up-para()\n\
c 	<Key>osfUp:		MimeRichTextC-move-up-para()\n\
s	<Key>osfUp:		MimeRichTextC-select-up-line()\n\
	<Key>osfUp:		MimeRichTextC-move-up-line()\n\
c s	<Key>osfDown:		MimeRichTextC-select-down-para()\n\
c 	<Key>osfDown:		MimeRichTextC-move-down-para()\n\
s	<Key>osfDown:		MimeRichTextC-select-down-line()\n\
	<Key>osfDown:		MimeRichTextC-move-down-line()\n\
c s	<Key>osfBeginLine:	MimeRichTextC-select-file-beg()\n\
c	<Key>osfBeginLine:	MimeRichTextC-move-file-beg()\n\
s	<Key>osfBeginLine:	MimeRichTextC-select-line-beg()\n\
	<Key>osfBeginLine:	MimeRichTextC-move-line-beg()\n\
c s	<Key>osfEndLine:	MimeRichTextC-select-file-end()\n\
c	<Key>osfEndLine:	MimeRichTextC-move-file-end()\n\
s	<Key>osfEndLine:	MimeRichTextC-select-line-end()\n\
	<Key>osfEndLine:	MimeRichTextC-move-line-end()\n\
s	<Key>osfBeginData:	MimeRichTextC-select-file-beg()\n\
	<Key>osfBeginData:	MimeRichTextC-move-file-beg()\n\
s	<Key>osfEndData:	MimeRichTextC-select-file-end()\n\
	<Key>osfEndData:	MimeRichTextC-move-file-end()\n\
c s	<Key>osfBackSpace:	MimeRichTextC-delete-line-beg()\n\
c	<Key>osfBackSpace:	MimeRichTextC-delete-left-word()\n\
s	<Key>osfBackSpace:	MimeRichTextC-delete-left-char()\n\
	<Key>osfBackSpace:	MimeRichTextC-delete-left-char()\n\
c s	<Key>osfDelete:		MimeRichTextC-delete-line-end()\n\
c	<Key>osfDelete:		MimeRichTextC-delete-right-word()\n\
s	<Key>osfDelete:		MimeRichTextC-delete-right-char()\n\
	<Key>osfDelete:		MimeRichTextC-delete-right-char()\n\
	<Key>osfClear:		MimeRichTextC-delete-selection()\n\
	<Key>osfCut:		MimeRichTextC-cut-selection()\n\
	<Key>osfCopy:		MimeRichTextC-copy-selection()\n\
c	<Key>osfPageUp:		MimeRichTextC-scroll-top()\n\
	<Key>osfPageUp:		MimeRichTextC-scroll-up-page()\n\
c	<Key>osfPageDown:	MimeRichTextC-scroll-bottom()\n\
	<Key>osfPageDown:	MimeRichTextC-scroll-down-page()\n\
	<Key>osfInsert:		MimeRichTextC-paste()\n\
	<Key>osfPaste:		MimeRichTextC-paste()\n\
	<Key>osfUndo:		MimeRichTextC-undo()\n\
c	<Key>u:			MimeRichTextC-delete-line-beg()\n\
c	<Key>w:			MimeRichTextC-delete-left-word()\n\
c	<Key>h:			MimeRichTextC-delete-left-char()\n\
c	<Key>v:			MimeRichTextC-paste()\n\
c	<Key>z:			MimeRichTextC-undo()\n\
c	<Key>p:			MimeRichTextC-plain()\n\
c	<Key>b:			MimeRichTextC-bold()\n\
c	<Key>i:			MimeRichTextC-italic()\n\
c	<Key>f:			MimeRichTextC-fixed()\n\
c	<Key>period:		MimeRichTextC-bigger()\n\
c	<Key>comma:		MimeRichTextC-smaller()\n\
c	<Key>-:			MimeRichTextC-underline()\n\
c	<Key>=:			MimeRichTextC-center()\n\
c	<Key>bracketleft:	MimeRichTextC-flush-left()\n\
c	<Key>bracketright:	MimeRichTextC-flush-right()\n\
c s	<Key>period:		MimeRichTextC-left-margin-in()\n\
c s	<Key>comma:		MimeRichTextC-left-margin-out()\n\
Mod1	<Key>period:		MimeRichTextC-excerpt-more()\n\
Mod1	<Key>comma:		MimeRichTextC-excerpt-less()\n\
c	<Key>l:			MimeRichTextC-refresh()\n\
c	<Key>/:			MimeRichTextC-search()\n\
	<Key>:			MimeRichTextC-insert-self()\n\
!c	<Btn1Down>:		MimeRichTextC-follow-url()\n\
!s	<Btn1Down>:		MimeRichTextC-select-extend()\n\
~c ~s	<Btn1Down>:		MimeRichTextC-select-begin()\n\
!s	<Btn1Motion>:		MimeRichTextC-select-extend()\n\
~c ~s	<Btn1Motion>:		MimeRichTextC-select-motion()\n\
~c	<Btn1Up>:		MimeRichTextC-select-end()\n\
	<Btn2Up>:		MimeRichTextC-paste()\n\
	<Btn3Down>:		MimeRichTextC-post-menu()\
");

/*
	<Btn1Down>:		MimeRichTextC-select-begin()\n\
	<Btn2Up>:		MimeRichTextC-paste()\n\
	<Btn3Down>:		MimeRichTextC-post-menu()\n\
c	<Btn1Up>:		MimeRichTextC-follow-url()\n\
	<Btn1Up>:		MimeRichTextC-select-end()\n\
s	<Btn1Down>:		MimeRichTextC-select-extend()\n\
s	<Btn1Motion>:		MimeRichTextC-select-extend()\n\
	<Btn1Motion>:		MimeRichTextC-select-motion()\n\
*/

/*
c	<Btn1Down>,<Btn1Up>:	MimeRichTextC-follow-url()\n\
s	<Btn1Down>:		MimeRichTextC-select-extend()\n\
	<Btn1Down>(4):		MimeRichTextC-quad-click()\n\
	<Btn1Down>(3):		MimeRichTextC-trip-click()\n\
	<Btn1Down>(2):		MimeRichTextC-doub-click()\n\
	<Btn1Down>:		MimeRichTextC-sing-click()\n\
s	<Btn1Up>:		MimeRichTextC-select-end()\n\
	<Btn1Up>(4):		MimeRichTextC-select-end()\n\
	<Btn1Up>(3):		MimeRichTextC-select-end()\n\
	<Btn1Up>(2):		MimeRichTextC-select-end()\n\
	<Btn1Down>,<Btn1Up>:	MimeRichTextC-select-end()\n\
	<Btn2Up>:		MimeRichTextC-paste()\n\
*/

/*
!c	<Btn1Down>:		MimeRichTextC-follow-url()\n\
!s	<Btn1Down>:		MimeRichTextC-select-extend()\n\
~c ~s	<Btn1Down>:		MimeRichTextC-select-begin()\n\
!s	<Btn1Motion>:		MimeRichTextC-select-extend()\n\
~c ~s	<Btn1Motion>:		MimeRichTextC-select-motion()\n\
~c	<Btn1Up>:		MimeRichTextC-select-end()\n\
	<Btn2Up>:		MimeRichTextC-paste()\n\
*/

//
// These translations handle the behavior of the "delete" key.  People are
//   so particular...
//
      priv->delRightTrans = XtParseTranslationTable("\
c s	<Key>osfDelete:	MimeRichTextC-delete-line-end()\n\
c	<Key>osfDelete:	MimeRichTextC-delete-right-word()\n\
s	<Key>osfDelete:	MimeRichTextC-delete-right-char()\n\
	<Key>osfDelete:	MimeRichTextC-delete-right-char()\n\
");

      priv->delLeftTrans = XtParseTranslationTable("\
c s	<Key>osfDelete:	MimeRichTextC-delete-line-beg()\n\
c	<Key>osfDelete:	MimeRichTextC-delete-left-word()\n\
s	<Key>osfDelete:	MimeRichTextC-delete-left-char()\n\
	<Key>osfDelete:	MimeRichTextC-delete-left-char()\n\
");

//
// These translations mimic emacs and override any default translations
//
      priv->emacsTrans = XtParseTranslationTable("\
c	<Key>b:			MimeRichTextC-move-left-char()\n\
Mod1	<Key>b:			MimeRichTextC-move-left-word()\n\
c 	<Key>f:			MimeRichTextC-move-right-char()\n\
Mod1	<Key>f:			MimeRichTextC-move-right-word()\n\
c 	<Key>a:			MimeRichTextC-move-line-beg()\n\
c	<Key>e:			MimeRichTextC-move-line-end()\n\
c	<Key>p:			MimeRichTextC-move-up-line()\n\
Mod1	<Key>bracketleft:	MimeRichTextC-move-up-para()\n\
c	<Key>n:			MimeRichTextC-move-down-line()\n\
Mod1	<Key>bracketright:	MimeRichTextC-move-down-para()\n\
Mod1	<Key>comma:		MimeRichTextC-move-file-beg()\n\
Mod1	<Key>period:		MimeRichTextC-move-file-end()\n\
c	<Key>z:			MimeRichTextC-scroll-up-line()\n\
Mod1	<Key>z:			MimeRichTextC-scroll-down-line()\n\
c	<Key>d:			MimeRichTextC-delete-right-char()\n\
c	<Key>k:			MimeRichTextC-delete-line-end()\n\
c s	<Key>b:			MimeRichTextC-select-left-char()\n\
Mod1 s	<Key>b:			MimeRichTextC-select-left-word()\n\
c s 	<Key>f:			MimeRichTextC-select-right-char()\n\
Mod1 s	<Key>f:			MimeRichTextC-select-right-word()\n\
c s	<Key>a:			MimeRichTextC-select-line-beg()\n\
c s	<Key>e:			MimeRichTextC-select-line-end()\n\
c s	<Key>p:			MimeRichTextC-select-up-line()\n\
Mod1 s	<Key>bracketleft:	MimeRichTextC-select-up-para()\n\
c s	<Key>n:			MimeRichTextC-select-down-line()\n\
Mod1 s	<Key>bracketright:	MimeRichTextC-select-down-para()\n\
Mod1 s	<Key>comma:		MimeRichTextC-select-file-beg()\n\
Mod1 s	<Key>period:		MimeRichTextC-select-file-end()\n\
c	<Key>y:			MimeRichTextC-undo()\n\
c	<Key>0:			MimeRichTextC-plain()\n\
c	<Key>1:			MimeRichTextC-bold()\n\
c	<Key>2:			MimeRichTextC-italic()\n\
c	<Key>3:			MimeRichTextC-fixed()\n\
c	<Key>4:			MimeRichTextC-bigger()\n\
Mod1	<Key>4:			MimeRichTextC-smaller()\n\
c	<Key>5:			MimeRichTextC-flush-left()\n\
Mod1	<Key>5:			MimeRichTextC-flush-right()\n\
c	<Key>6:			MimeRichTextC-left-margin-in()\n\
Mod1	<Key>6:			MimeRichTextC-left-margin-out()\n\
c	<Key>7:			MimeRichTextC-right-margin-in()\n\
Mod1	<Key>7:			MimeRichTextC-right-margin-out()\n\
c	<Key>8:			MimeRichTextC-excerpt-more()\n\
Mod1	<Key>8:			MimeRichTextC-excerpt-less()\
");

   } // End if actions not added

//
// Read translation resources
//
   priv->tabTraverses = get_boolean(cl, MainWidget(), "tabTraverses", False);
   priv->delLikeBs    = get_boolean(cl, MainWidget(), "deleteMeansBackspace",
								      False);
   priv->emacsMode    = get_boolean(cl, MainWidget(), "emacsMode",    False);
   priv->userTrans    = get_string (cl, priv->textDA, "translations");
   priv->userTrans.Trim();

   priv->UpdateTranslations();

//
// Initialize character classes for double-click
//
   StringC	charClassDef = get_string(cl, MainWidget(), "charClass", "");
   SetCharClasses(charClassDef);

//
// Initialize cursor
//
   priv->editable = get_boolean(cl, MainWidget(), "editable",        True);
   float rate     = get_float  (cl, MainWidget(), "cursorBlinkRate", 1.0);
   float onTime   = get_float  (cl, MainWidget(), "cursorOnPercent", 75.0);
   float blinkInterval = (rate > 0.0) ? (float)1000/rate : 0.0;
   priv->cursorOnTime  = (int)(blinkInterval * (onTime/(float)100));
   priv->cursorOffTime = (int)(blinkInterval - (float)priv->cursorOnTime);

//
// Add event handler for focus change
//
   Widget	shell = parent;
   while ( !XtIsShell(shell) ) shell = XtParent(shell);

   unsigned char	focusPolicy;
   XtVaGetValues(shell, XmNkeyboardFocusPolicy, &focusPolicy, NULL);
   if ( focusPolicy == XmPOINTER )
      XtAddEventHandler(priv->textDA, EnterWindowMask|LeaveWindowMask, False,
			(XtEventHandler)MimeRichTextP::HandleFocusChange,
			(XtPointer)priv);
   else
      XtAddEventHandler(priv->textDA, FocusChangeMask, False,
			(XtEventHandler)MimeRichTextP::HandleFocusChange,
			(XtPointer)priv);

//
// We took multi-click translation out of trans table.  Go back to
//    ActSelectMotion
//
#if 0
//
// Add event handler for button 1 motion.  This cannot co-exist in the
//    regular translation table because of the presence of multi-click
//    events.  (Xt volume 5, page 876).
//
   XtAddEventHandler(priv->textDA, Button1MotionMask, False,
		     (XtEventHandler)MimeRichTextP::HandleButton1Motion,
		     (XtPointer)priv);
#endif

//
// Get the command to be used when a URL is clicked
//
   priv->webCommand = get_string(cl, MainWidget(), "webBrowserCommand",
   				 "Mosaic %s");

//
// Create atom for cut and paste
//
   if ( !MimeRichTextP::mimeRichAtom )
      MimeRichTextP::mimeRichAtom =
	 XmInternAtom(halApp->display, MIME_ENRICHED_ATOM_NAME, False);

//
// Add a blank line
//
   priv->NewInputLine();

} // End constructor

/*----------------------------------------------------------------------
 * Destructor
 */

MimeRichTextC::~MimeRichTextC()
{
   if ( halApp->xRunning ) {

      if ( priv->cursorTimer ) XtRemoveTimeOut(priv->cursorTimer);
      XtReleaseGC(priv->textDA, priv->textGC);
      if ( priv->textPm    ) XFreePixmap(halApp->display, priv->textPm);
      if ( priv->stipplePm ) XFreePixmap(halApp->display, priv->stipplePm);

      if ( priv->editable && priv->realized )
	 XmImUnregister(priv->textDA);
   }

   priv->selectOn = False;	// So selection won't get erased
   priv->Reset();

   delete priv;

} // End Destructor

/*----------------------------------------------------------------------
 * Method to clear out the line lists
 */

void
MimeRichTextP::Reset()
{
   if ( selectOn ) {
      DrawSelection();
      selectOn = False;
   }

   inputTextBuf.Clear();
   inputLine = NULL;
   inputCmd  = NULL;

//
// Delete text lines
//
   TextLineC	*line = topTextLine;
   TextLineC	*next;
   while ( line ) {
      next = line->next;
      delete line;
      line = next;
   }

   line = topSaveLine;
   while ( line ) {
      next = line->next;
      delete line;
      line = next;
   }

   topTextLine = NULL;
   botTextLine = NULL;
   topSaveLine = NULL;
   botSaveLine = NULL;
   topSoftLine = NULL;
   botSoftLine = NULL;

   NewInputLine();
#if 0
   if ( curCharset != 0 )
      AddCharsetCommand(pub->Charset(curCharset), False/*not negate*/);
#endif

#if 0
   if ( textType == TT_PLAIN )
      inputCmd->state.PushJust(JC_NOFILL);
#endif

   cursorPos.Set   (inputLine, 0, 0);
   selectBegPos.Set(inputLine, 0, 0);
   selectEndPos.Set(inputLine, 0, 0);
   nextPartPos.Set (inputLine, 0, 0);
   moreParts = True;

} // End MimeRichTextP::Reset

/*-----------------------------------------------------------------------
 *  Create a new text line and set its initial state.
 */

void
MimeRichTextP::NewInputLine()
{
//
// If we're in single line mode, just add a space to the input
//
   if ( singleLine && !inputLastWhite )
      inputTextBuf += ' ';

   FlushTextBuf();

   if ( singleLine && inputLine ) return;

//
// Add a new line to the list
//
   TextLineC	*prevLine = inputLine;
   inputLine = new TextLineC;
   if ( prevLine )
      AddLine(inputLine, prevLine);
   else {
      inputLine->index = 0;
      topTextLine = botTextLine = inputLine;
   }

//
// Move all commands to the right of inputCmd to the new line
//
   if ( prevLine ) {

      void	*tmp = (void*)inputCmd;
      int	startPos = prevLine->cmdList.indexOf(tmp) + 1;
      if ( startPos >= 0 ) {

//
// Add commands to new line
//
	 int	i;
	 for (i=startPos; i<prevLine->cmdList.size(); i++) {
	    RichCmdC	*cmd = prevLine->Cmd(i);
	    inputLine->AddCommand(cmd);
	 }

//
// Remove commands from previous line
//
	 for (i=prevLine->cmdList.size()-1; i>=startPos; i--)
	    prevLine->cmdList.remove(i);
      }

      TextPosC	curPos(prevLine, startPos-1, inputCmd->text->size());
      FixPosAfterBreak(&cursorPos, curPos, inputLine);

   } // End if there is a previous line

//
// Copy the current state
//
   RichCmdC	*cmd = inputLine->Cmd(0);

   if ( inputCmd ) cmd->state = inputCmd->state;
   inputCmd = cmd;

   inputLastWhite = True;

} // End NewInputLine

/*----------------------------------------------------------------------
 * Clear the data
 */

void
MimeRichTextC::Clear()
{
   priv->Reset();
   priv->changed = True;
   if ( !priv->defer ) {
      Refresh();
      CallCallbacks(priv->changeCalls, this);
   }
}

/*----------------------------------------------------------------------
 * Set text string
 */

void
MimeRichTextC::SetString(CharC cs)
{
   priv->Reset();
   AddString(cs);
}

void
MimeRichTextC::AddString(CharC cs)
{
   if ( debuglev > 1 ) cout <<"AddString with (" <<cs <<")" NL;

   if ( cs.Length() > 10000 ) halApp->BusyCursor(True);

#if 0
   Boolean	truncated = False;
   if ( cs.Length() > priv->maxLength ) {
      cs.CutEnd(cs.Length() - priv->maxLength);
      truncated = True;
   }
#endif

   Defer(True);

//
// Create an input line if necessary
//
   if ( !priv->botTextLine ) priv->Reset();

   priv->inputLine = priv->botTextLine;
   priv->inputCmd  = priv->inputLine->Cmd(priv->inputLine->cmdList.size()-1);

//
// Create text command if necessary
//
   if ( !priv->inputCmd->IsText() ) {
      RichCmdC	*rcmd = new RichCmdC(RC_TEXT);
      rcmd->state = priv->inputCmd->state;
      priv->inputLine->AddCommand(rcmd, /*after*/priv->inputCmd);
      priv->inputCmd = rcmd;
   }

//
// Add string based on type
//
   if ( priv->textType == TT_ENRICHED  ) AddStringEnriched(cs);
   else if ( priv->textType == TT_RICH ) AddStringRichtext(cs);
   else if ( priv->textType == TT_HTML ) AddStringHTML(cs);
   else				         AddStringPlain(cs);

#if 0
   if ( truncated ) {
      StringC	msg("\n\n<center><bold>... DISPLAY TRUNCATED BY ");
      msg += halApp->name;
      msg += " ...</bold></center>\n\n";
      AddStringEnriched(msg);
   }
#endif

   Defer(False);

   if ( cs.Length() > 10000 ) halApp->BusyCursor(False);

} // End AddString

/*----------------------------------------------------------------------
 * Set text string in enriched mode
 */

void
MimeRichTextC::AddStringEnriched(CharC cs)
{
   if ( cs.Length() > 10000 ) halApp->BusyCursor(True);

   TextTypeT	saveType = priv->textType;
   priv->textType = TT_ENRICHED;

//
// Create text command if necessary
//
   if ( !priv->inputCmd->IsText() ) {
      RichCmdC	*rcmd = new RichCmdC(RC_TEXT);
      rcmd->state = priv->inputCmd->state;
      priv->inputLine->AddCommand(rcmd, /*after*/priv->inputCmd);
      priv->inputCmd = rcmd;
   }

   int			inParam = 0;
   StringC		paramCmd;
   StringC		paramText;
   static StringC	*cmd = NULL;
   if ( !cmd ) cmd = new StringC;

//
// Loop through input
//
   u_int	i = 0;
   if ( priv->checkFroms && priv->BeginningOfInputLine() &&
        cs.StartsWith(">From ") ) i++;

   while ( i < cs.Length() ) {

      char	c = cs[i];

//
// Look at current character
//
      if ( c == '<' ) {

	 c = cs[++i];
	 if ( c == '<' ) {	// Second '<'
	    if ( inParam ) paramText += c;
	    else	   priv->inputTextBuf += c;
	    c = cs[++i];
	 }

	 else { // Look for end of command and process

//
// Get command
//
	    Boolean	negate = False;
	    if ( c == '/' ) {
	       negate = True;
	       c = cs[++i];
	    }

	    cmd->Clear();
	    int	pos = cs.PosOf('>', i);
	    if ( pos >= 0 ) {
	       *cmd = cs(i, (u_int)pos-i);
	       i = pos + 1;
	       c = cs[i];
	    }

//
// If we're in a parameter, the only command we'll recognize is another param
//    command.
//
	    if ( inParam ) {

	       if ( strcasecmp(*cmd, "param") == 0 ) {

		  inParam += (negate ? -1: 1);
		  if ( inParam < 0 ) inParam = 0;

//
// If we have a command that was waiting on a parameter, issue it now
//
		  if ( !inParam && paramCmd.size() > 0 ) {
		     priv->AddParamCommand(paramCmd, paramText);
		     paramCmd.Clear();
		     paramText.Clear();
		  }
	       }

	    } // End if inside parameter block

//
// Look for a parameter command
//
	    else if ( strcasecmp(*cmd, "param") == 0 ) {

	       inParam += (negate ? -1: 1);
	       if ( inParam < 0 ) inParam = 0;

	    } // End if got parameter command

//
// If we're starting a color range, look for the parameters
//
	    else if ( strcasecmp(*cmd, "x-color") == 0 && !negate ) {
	       paramCmd = *cmd;
	    }

//
// If we're starting a link, look for the parameters
//
	    else if ( strcasecmp(*cmd, "x-link") == 0 && !negate ) {
	       paramCmd = *cmd;
	    }

//
// Process command
//
	    else if ( !AddCommand(*cmd, negate) ) {	// Unknown command
	       if ( priv->inputTextBuf.size() > 0 &&
		    !isspace(priv->inputTextBuf.LastChar()) )
		  priv->inputTextBuf += ' ';
	    }

	 } // End if found a command

      } // End if left angle

      else if ( c == '\n' ) { // Skip this one and copy all others

	 c = cs[++i];

	 if ( c == '\n' ) {

	    while ( c == '\n' ) {
	       if ( !inParam ) priv->NewInputLine();
	       c = cs[++i];
	    }
	 }

	 else if ( c && !inParam ) {	// Single newline becomes space
#if 0
	    if ( priv->inputTextBuf.size() > 0 &&
		 !isspace(priv->inputTextBuf.LastChar()) )
#endif
	       priv->inputTextBuf += ' ';
	 }

//
// Skip > if necessary
//
	 if ( priv->checkFroms && cs.StartsWith(">From ", i) ) c = cs[++i];

      } // End if found a newline

      else {	// Printable character
	 if ( inParam ) paramText += c;
	 else		priv->inputTextBuf += c;
	 i++;
      }

   } // End for each input character

//
// Transfer final text
//
   priv->FlushTextBuf();
   priv->textType = saveType;

//
// Redraw if necessary
//
   priv->changed = True;
   if ( !priv->defer ) {
      Refresh();
      CallCallbacks(priv->changeCalls, this);
   }

   if ( cs.Length() > 10000 ) halApp->BusyCursor(False);

} // End AddStringEnriched

/*----------------------------------------------------------------------
 * Set text string in richtext mode
 */

void
MimeRichTextC::AddStringRichtext(CharC cs)
{
   if ( cs.Length() > 10000 ) halApp->BusyCursor(True);

   TextTypeT	saveType = priv->textType;
   priv->textType = TT_RICH;

//
// Create text command if necessary
//
   if ( !priv->inputCmd->IsText() ) {
      RichCmdC	*rcmd = new RichCmdC(RC_TEXT);
      rcmd->state = priv->inputCmd->state;
      priv->inputLine->AddCommand(rcmd, /*after*/priv->inputCmd);
      priv->inputCmd = rcmd;
   }

   char		comment   = 0;
   static StringC	*cmd = NULL;
   if ( !cmd ) cmd = new StringC;

//
// Loop through input
//
   u_int	i = 0;
   if ( priv->checkFroms && priv->BeginningOfInputLine() &&
	cs.StartsWith(">From ") ) i++;

   while ( i < cs.Length() ) {

      char	c = cs[i];

//
// Look at current character
//
      if ( c == '<' ) {

         c = cs[++i];

//
// Get command
//
	 Boolean	negate = False;
	 if ( c == '/' ) {
	    negate = True;
	    c = cs[++i];
	 }

	 cmd->Clear();
	 int pos = cs.PosOf('>', i);
	 if ( pos >= 0 ) {
	    *cmd = cs(i, (u_int)pos-i);
	    i = pos + 1;
	    c = cs[i];
	 }

//
// If we're in a comment, the only thing we'll recognize is more comment
//    commands.
//
	 if ( comment ) {
	    if ( strcasecmp(*cmd, "comment") == 0 ) {
	       comment += (negate ? -1: 1);
#ifdef __CHAR_UNSIGNED__
	       if ( comment > 127 ) comment = 0;
#else
	       if ( comment < 0 ) comment = 0;
#endif
	    }
	 }

	 else {

//
// Process command
//
	    if ( strcasecmp(*cmd, "nl") == 0 )
	       priv->NewInputLine();

	    else if ( strcasecmp(*cmd, "np") == 0 ||
	              strcasecmp(*cmd, "paragraph") == 0 ) {
	       priv->NewInputLine();
	       priv->NewInputLine();
	    }

	    else if ( strcasecmp(*cmd, "lt") == 0 )
	       priv->inputTextBuf += '<';

	    else if ( strcasecmp(*cmd, "comment") == 0 ) {
	       comment += (negate ? -1: 1);
#ifdef __CHAR_UNSIGNED__
	       if ( comment > 127 ) comment = 0;
#else
	       if ( comment < 0 ) comment = 0;
#endif
	    }

	    else if ( !AddCommand(*cmd, negate) ) {	// Unknown command
	       if ( priv->inputTextBuf.size() > 0 &&
		    !isspace(priv->inputTextBuf.LastChar()) )
		  priv->inputTextBuf += ' ';
	    }

	 } // End if not in comment

      } // End if left angle

      else if ( c == '\n' ) {

         c = cs[++i];
	 if ( c && c != '\n' && !comment ) // Single newline becomes space
	    priv->inputTextBuf += ' ';

//
// Skip > if necessary
//
         if ( priv->checkFroms && cs.StartsWith(">From ", i) ) c = cs[++i];

      } // End if found a newline

      else { // Printable character
	 if ( !comment ) priv->inputTextBuf += c;
	 i++;
      }

   } // End for each input character

//
// Transfer final text
//
   priv->FlushTextBuf();
   priv->textType = saveType;

//
// Redraw if necessary
//
   priv->changed = True;
   if ( !priv->defer ) {
      Refresh();
      CallCallbacks(priv->changeCalls, this);
   }

   if ( cs.Length() > 10000 ) halApp->BusyCursor(False);

} // End AddStringRichtext

/*----------------------------------------------------------------------
 * Set text string in plaintext mode
 */

void
MimeRichTextC::AddStringPlain(CharC cs)
{
   if ( cs.Length() > 10000 ) halApp->BusyCursor(True);

   TextTypeT	saveType = priv->textType;
   priv->textType = TT_PLAIN;

//
// Create text command if necessary
//
   if ( !priv->inputCmd->IsText() ) {
      RichCmdC	*rcmd = new RichCmdC(RC_TEXT);
      rcmd->state = priv->inputCmd->state;
      priv->inputLine->AddCommand(rcmd, /*after*/priv->inputCmd);
      priv->inputCmd = rcmd;
   }

   u_int	offset = 0;
   if ( priv->checkFroms && priv->BeginningOfInputLine() &&
        cs.StartsWith(">From ", offset) ) offset++;

//
// Loop through input.  For each newline, create a text line
//
   int		nlPos = cs.PosOf('\n');
   while ( nlPos >= 0 ) {
      priv->inputTextBuf = cs(offset, nlPos-offset);
      priv->NewInputLine();
      offset = nlPos + 1;
      if ( priv->checkFroms && cs.StartsWith(">From ", offset) ) offset++;
      nlPos = cs.PosOf('\n', offset);
   }

//
// Transfer final text
//
   u_int	len = cs.Length() - offset;
   if ( len > 0 ) {
      priv->inputTextBuf = cs(offset, len);
      priv->FlushTextBuf();
   }

   priv->textType = saveType;

//
// Redraw if necessary
//
   priv->changed = True;
   if ( !priv->defer ) {
      Refresh();
      CallCallbacks(priv->changeCalls, this);
   }

   if ( cs.Length() > 10000 ) halApp->BusyCursor(False);

} // End AddStringPlain

/*----------------------------------------------------------------------
 * Set text string in HTML mode
 */

void
MimeRichTextC::AddStringHTML(CharC cs)
{
   AddStringPlain(cs);
}

/*----------------------------------------------------------------------
 * Copy text buffer to current input line's command buffer
 */

void
MimeRichTextP::FlushTextBuf()
{
   static Boolean	flushing = False;

   if ( flushing || !inputCmd || inputTextBuf.size() == 0 ) return;
   flushing = True;

//
// remove extra whitespace if we're filling
//
   Boolean fillOk = False;
//         (textType != TT_PLAIN && inputCmd->state.CurJustCmd() != JC_NOFILL);
   if ( fillOk ) {

      Boolean	prevWhite = inputLastWhite;

//
// make a pass and see if there are any spaces to be removed
//
      char	*ip = inputTextBuf;
      char	*ep = ip + inputTextBuf.size();
      Boolean	needsFill = False;
      while ( !needsFill && ip < ep ) {
//	 Boolean	thisWhite = isspace(*ip);
	 Boolean	thisWhite = (*ip == ' ');
	 needsFill = (prevWhite && thisWhite);
//	 prevWhite = thisWhite;
	 prevWhite = isspace(*ip);
	 ip++;
      }

      if ( needsFill ) {

	 prevWhite = inputLastWhite;
	 char	*newBuf = new char[inputTextBuf.size()+1];
	 char	*op = newBuf;
	 ip = inputTextBuf;
	 while ( ip < ep ) {
//	    Boolean	thisWhite = isspace(*ip);
	    Boolean	thisWhite = (*ip == ' ');
	    if ( !thisWhite || !prevWhite ) *op++ = *ip;
//	    prevWhite = thisWhite;
	    prevWhite = isspace(*ip);
	    ip++;
	 }
	 *op = 0;

         inputTextBuf = newBuf;
	 delete newBuf;

      } // End if this string needs filling
      
   } // End if filling is ok

//
// Look for URL's in non-editable text only.  It's too hard to keep looking
//    for them if the user can make changes.
//
   CharC	range = inputTextBuf;
   if ( !editable && range.Contains(':') &&
	( range.Contains("http://")	||
	  range.Contains("https://")	||
          range.Contains("ftp://")	||
          range.Contains("gopher://")	||
          range.Contains("telnet://")	||
          range.Contains("mailto:")	||
          range.Contains("afs://")	||
          range.Contains("file:/")	||
          range.Contains("news:")	||
          range.Contains("nntp:")	||
          range.Contains("mid:")	||
          range.Contains("cid:")	||
          range.Contains("wais://")	||
          range.Contains("prospero://") ) ) {
      FlushTextURL(range);
   }

//
// copy current text buffer
//
   else {
      if ( debuglev > 1 ) cout <<"Adding text ^" <<inputTextBuf <<"^" <<endl;
      *inputCmd->text += inputTextBuf;
   }

   inputLastWhite = isspace(inputTextBuf.LastChar());
   inputTextBuf.Clear();

   flushing = False;

} // End FlushTextBuf

/*----------------------------------------------------------------------
 * See if text is a valid URL prefix
 */

static Boolean
IsURL(CharC text)
{
   return ( text.Equals("http://")	||
	    text.Equals("https://")	||
	    text.Equals("ftp://")	||
	    text.Equals("gopher://")	||
	    text.Equals("telnet://")	||
	    text.Equals("mailto:")	||
	    text.Equals("afs://")	||
	    text.Equals("file:/")	||
	    text.Equals("news:")	||
	    text.Equals("nntp:")	||
	    text.Equals("mid:")		||
	    text.Equals("cid:")		||
	    text.Equals("wais://")	||
	    text.Equals("prospero://") );
}

/*----------------------------------------------------------------------
 * Copy text buffer to current input line's command buffer while marking
 *    any URL's
 */

void
MimeRichTextP::FlushTextURL(CharC buffer)
{
   static RegexC	*urlPat = NULL;

//
// URL patterns.
// Pattern matches any://, any:/ or any:
//
   if ( !urlPat ) {
      //urlPat = new RegexC("[a-z]+://[a-zA-Z0-9\$_@\.&\+!\*\"'(),%/-]+");
      urlPat = new RegexC("\\([a-z]+:/*\\)[^ \t\n()<>\"]+[a-zA-Z0-9]");
   }

//
// Check for pattern. Make sure "any" is valid
//
   int		pos = urlPat->search(buffer);
   while ( pos >= 0 && !IsURL(buffer((*urlPat)[1])) ) {
      pos += (*urlPat)[1].length();
      pos = urlPat->search(buffer, pos);
   }

//
// See if we found a valid URL
//
   if ( pos < 0 ) {
      if ( debuglev > 1 ) cout <<"Adding false URL ^" <<buffer <<"^" <<endl;
      *inputCmd->text += buffer;
      return;
   }

   int		offset = 0;
   while ( pos >= 0 ) {

//
// Add text up to this point
//
      int	len = pos - offset;
      if ( len > 0 ) {
	 if ( debuglev > 1 )
	    cout <<"Adding text ^" <<buffer(offset, len) <<"^" <<endl;
	 *inputCmd->text += buffer(offset, len);
      }

//
// Create a new command for the URL
//
      RichCmdC  *rcmd = new RichCmdC(RC_TEXT);
      rcmd->state = inputCmd->state;
      rcmd->state.PushFont(FC_URL);
      inputLine->AddCommand(rcmd, /*after*/inputCmd);
      inputCmd = rcmd;

//
// Add the URL text
//
      RangeC	urlRange = (*urlPat)[0];

      if ( debuglev > 1 )
	 cout <<"Adding URL text ^" <<buffer(urlRange) <<"^" <<endl;
      *inputCmd->text += buffer(urlRange);

//
// Create a new command for the plain text again
//
      rcmd = new RichCmdC(RC_TEXT);
      rcmd->state = inputCmd->state;
      rcmd->state.PopFont();	// Remove URL markup
      inputLine->AddCommand(rcmd, /*after*/inputCmd);
      inputCmd = rcmd;

//
// Look for next pattern
//
      offset = pos + urlRange.length();
      pos = urlPat->search(buffer, offset);
      while ( pos >= 0 && !IsURL(buffer((*urlPat)[1])) ) {
	 pos += (*urlPat)[1].length();
	 pos = urlPat->search(buffer, pos);
      }

   } // End for each pattern

//
// Add any remaining text
//
   int	len = buffer.Length() - offset;
   if ( len > 0 ) {
      if ( debuglev > 1 )
	 cout <<"Adding text ^" <<buffer(offset, len) <<"^" <<endl;
      *inputCmd->text += buffer(offset, len);
   }

} // End FlushTextURL

/*----------------------------------------------------------------------
 * Add a command to the command list of the current input line
 */

Boolean
MimeRichTextC::AddCommand(CharC cmd, Boolean negate)
{
   if ( cmd.StartsWith("iso-", IGNORE_CASE) ) {
      StringC	name(cmd);
      name(3,1) = "";	// Remove '-'
      return priv->AddCharsetCommand(name, negate);
   }
   else if ( cmd.StartsWith("iso", IGNORE_CASE) ) {
      return priv->AddCharsetCommand(cmd, negate);
   }

   if ( debuglev > 1 ) {
      cout <<"Found command: " <<cmd;
      if ( negate ) cout <<" off";
      else          cout <<" on";
      cout <<endl;
   }

   Boolean	legal = True;
   Boolean	stateChange = False;
   Boolean	lineChange  = False;
   int		level;
   TextStateC	*inputState = &priv->inputCmd->state;

   if      ( cmd.Equals("bold", IGNORE_CASE) ) {
      if ( negate ) legal = (inputState->CurFontCmd() == FC_BOLD);
      level = inputState->Bold();
      stateChange = ((negate && level == 1) || (!negate && level == 0));
   }
   else if ( cmd.Equals("italic", IGNORE_CASE) ) {
      if ( negate ) legal = (inputState->CurFontCmd() == FC_ITALIC);
      level = inputState->Italic();
      stateChange = ((negate && level == 1) || (!negate && level == 0));
   }
   else if ( cmd.Equals("fixed", IGNORE_CASE) ) {
      if ( negate ) legal = (inputState->CurFontCmd() == FC_FIXED);
      level = inputState->Fixed();
      stateChange = ((negate && level == 1) || (!negate && level == 0));
   }
   else if ( cmd.Equals("smaller", IGNORE_CASE) ) {
      if ( negate ) legal = (inputState->CurFontCmd() == FC_SMALLER);
      level = inputState->Smaller();
      stateChange = ((negate && level > 0) || !negate);
   }
   else if ( cmd.Equals("bigger", IGNORE_CASE) ) {
      if ( negate ) legal = (inputState->CurFontCmd() == FC_BIGGER);
      level = inputState->Bigger();
      stateChange = ((negate && level > 0) || !negate);
   }

   else if ( cmd.Equals("center", IGNORE_CASE) ) {
      if ( negate ) legal = (inputState->CurJustCmd() == JC_CENTER);
      level = inputState->Center();
      stateChange = ((negate && level == 1) || (!negate && level == 0));
      lineChange = !priv->BeginningOfInputLine();
   }
   else if ( cmd.Equals("flushleft", IGNORE_CASE) ) {
      if ( negate ) legal = (inputState->CurJustCmd() == JC_LEFT);
      level = inputState->FlushLeft();
      stateChange = ((negate && level == 1) || (!negate && level == 0));
      lineChange = !priv->BeginningOfInputLine();
   }
   else if ( cmd.Equals("flushright", IGNORE_CASE) ) {
      if ( negate ) legal = (inputState->CurJustCmd() == JC_RIGHT);
      level = inputState->FlushRight();
      stateChange = ((negate && level == 1) || (!negate && level == 0));
      lineChange = !priv->BeginningOfInputLine();
   }
   else if ( cmd.Equals("flushboth", IGNORE_CASE) ) {
      if ( negate ) legal = (inputState->CurJustCmd() == JC_BOTH);
      level = inputState->FlushBoth();
      stateChange = ((negate && level == 1) || (!negate && level == 0));
      lineChange = !priv->BeginningOfInputLine();
   }
   else if ( cmd.Equals("nofill", IGNORE_CASE) ) {
      if ( negate ) legal = (inputState->CurJustCmd() == JC_NOFILL);
      level = inputState->NoFill();
      stateChange = ((negate && level == 1) || (!negate && level == 0));
      lineChange = !priv->BeginningOfInputLine();
   }

   else if ( cmd.Equals("underline", IGNORE_CASE) ) {
      level = inputState->Underline();
      if ( negate ) legal = (level > 0);
      stateChange = True;
   }

   else if ( cmd.Equals("excerpt", IGNORE_CASE) ) {
      level = inputState->Excerpt();
      if ( negate ) legal = (level > 0);
      stateChange = True;
      lineChange = !priv->BeginningOfInputLine();
   }

   else if ( cmd.Equals("indent", IGNORE_CASE) ) {
      level = inputState->LIndent();
      if ( negate ) legal = (level > 0);
      stateChange = True;
   }
   else if ( cmd.Equals("indentright", IGNORE_CASE) ) {
      level = inputState->RIndent();
      if ( negate ) legal = (level > 0);
      stateChange = True;
   }

   else if ( cmd.Equals("x-color", IGNORE_CASE) ) {
      level = inputState->ColorCmdCount();
      // We only get negates here.  AddParamCommand gets the others
      /*if ( negate )*/ legal = (level > 0);
      stateChange = True;
   }

   else if ( cmd.Equals("x-link", IGNORE_CASE) ) {
      level = inputState->PLinkCount();
      // We only get negates here.  AddParamCommand gets the others
      /*if ( negate )*/ legal = (level > 0);
      stateChange = True;
   }

   else {
      return False;
   }

   if ( !legal ) return True;

//
// Store any pending text
//
   priv->FlushTextBuf();

//
// Create a new line if necessary
//
   if ( lineChange ) {
      priv->NewInputLine();
   }

//
// If we didn't create a new line (which automatically creates a new command
//    object) and the input state is changing and the text is not empty, we
//    need a new command object.
//
   else if ( stateChange && priv->inputCmd->text->size() > 0 ) {
      RichCmdC	*rcmd = new RichCmdC(RC_TEXT);
      rcmd->state = priv->inputCmd->state;
      priv->inputLine->AddCommand(rcmd, /*after*/priv->inputCmd);
      priv->inputCmd = rcmd;
   }

   inputState = &priv->inputCmd->state;

//
// Process this command
//
   if ( cmd.Equals("bold", IGNORE_CASE) ) {
      if ( negate ) inputState->PopFont();
      else	    inputState->PushFont(FC_BOLD);
   }
   else if ( cmd.Equals("italic", IGNORE_CASE) ) {
      if ( negate ) inputState->PopFont();
      else	    inputState->PushFont(FC_ITALIC);
   }
   else if ( cmd.Equals("fixed", IGNORE_CASE) ) {
      if ( negate ) inputState->PopFont();
      else	    inputState->PushFont(FC_FIXED);
   }
   else if ( cmd.Equals("smaller", IGNORE_CASE) ) {
      if ( negate ) inputState->PopFont();
      else	    inputState->PushFont(FC_SMALLER);
   }
   else if ( cmd.Equals("bigger", IGNORE_CASE) ) {
      if ( negate ) inputState->PopFont();
      else	    inputState->PushFont(FC_BIGGER);
   }
   else if ( cmd.Equals("center", IGNORE_CASE) ) {
      if ( negate ) inputState->PopJust();
      else	    inputState->PushJust(JC_CENTER);
   }
   else if ( cmd.Equals("flushleft", IGNORE_CASE) ) {
      if ( negate ) inputState->PopJust();
      else	    inputState->PushJust(JC_LEFT);
   }
   else if ( cmd.Equals("flushright", IGNORE_CASE) ) {
      if ( negate ) inputState->PopJust();
      else	    inputState->PushJust(JC_RIGHT);
   }
   else if ( cmd.Equals("flushboth", IGNORE_CASE) ) {
      if ( negate ) inputState->PopJust();
      else	    inputState->PushJust(JC_BOTH);
   }
   else if ( cmd.Equals("nofill", IGNORE_CASE) ) {
      if ( negate ) inputState->PopJust();
      else	    inputState->PushJust(JC_NOFILL);
   }

   else if ( cmd.Equals("underline", IGNORE_CASE) ) {
      if ( negate ) inputState->UnderlineLess();
      else	    inputState->UnderlineMore();
   }
   else if ( cmd.Equals("excerpt", IGNORE_CASE) ) {
      if ( negate ) inputState->ExcerptLess();
      else	    inputState->ExcerptMore();
   }
   else if ( cmd.Equals("indent", IGNORE_CASE) ) {
      if ( negate ) inputState->LIndentLess();
      else	    inputState->LIndentMore();
   }
   else if ( cmd.Equals("indentright", IGNORE_CASE) ) {
      if ( negate ) inputState->RIndentLess();
      else	    inputState->RIndentMore();
   }
   else if ( cmd.Equals("x-color", IGNORE_CASE) ) {
      // We only get negates here.  AddParamCommand gets the others
      /*if ( negate )*/ inputState->PopColor();
   }
   else if ( cmd.Equals("x-link", IGNORE_CASE) ) {
      // We only get negates here.  AddParamCommand gets the others
      /*if ( negate )*/ inputState->PopLink();
			inputState->PopColor();
   }

   return True;

} // End AddCommand

/*----------------------------------------------------------------------
 * Add a command requiring a parameter to the command list of the
 *    current input line
 */

Boolean
MimeRichTextP::AddParamCommand(StringC& cmd, StringC& param)
{
   if ( debuglev > 1 )
      cout <<"Found command: " <<cmd <<" with param: " <<param <<endl;

   TextStateC	*inputState = &inputCmd->state;

//
// We now acknowledge two commands, x-color and x-link
//
   Pixel	color;
   Boolean	isColor = False;
   if ( cmd.Equals("x-color", IGNORE_CASE) ) {
      if ( !PixelValue(textDA, param, &color) ) return False;
      isColor = True;
   }
   else if ( !cmd.Equals("x-link", IGNORE_CASE) ) {
      return False;
   }

//
// Store any pending text
//
   FlushTextBuf();

//
// If the input state is changing and the text is not empty, we
//    need a new command object.
//
   if ( inputCmd->text->size() > 0 ) {
      RichCmdC	*rcmd = new RichCmdC(RC_TEXT);
      rcmd->state = inputCmd->state;
      inputLine->AddCommand(rcmd, /*after*/inputCmd);
      inputCmd = rcmd;
   }

   inputState = &inputCmd->state;

//
// Process this command
//
   if (isColor) {
      inputState->PushColor(color);
   }
   else {
      // inputState->PushColor(linkColor);
      inputState->PushLink(param);
   }

   return True;

} // End AddParamCommand

/*----------------------------------------------------------------------
 * Add a character set command to the command list of the current input line
 */

Boolean
MimeRichTextP::AddCharsetCommand(CharC cmd, Boolean negate)
{
   if ( debuglev > 1 ) {
      cout <<"Found command: " <<cmd;
      if ( negate ) cout <<" off";
      else          cout <<" on";
      cout <<endl;
   }

//
// Add this character set to the list
//
   int	charsetNum = pub->AddCharset(cmd);

   Boolean	legal = True;
   Boolean	stateChange = False;
   int		level;
   TextStateC	*inputState = &inputCmd->state;

   switch (charsetNum) {

      case 1:
	 if ( negate ) legal = (inputState->CurFontCmd() == FC_CHARSET_1);
	 level = inputState->Charset1();
	 stateChange = ((negate && level == 1) || (!negate && level == 0));
	 break;

      case 2:
	 if ( negate ) legal = (inputState->CurFontCmd() == FC_CHARSET_2);
	 level = inputState->Charset2();
	 stateChange = ((negate && level == 1) || (!negate && level == 0));
	 break;

      case 3:
	 if ( negate ) legal = (inputState->CurFontCmd() == FC_CHARSET_3);
	 level = inputState->Charset3();
	 stateChange = ((negate && level == 1) || (!negate && level == 0));
	 break;

      case 4:
	 if ( negate ) legal = (inputState->CurFontCmd() == FC_CHARSET_4);
	 level = inputState->Charset4();
	 stateChange = ((negate && level == 1) || (!negate && level == 0));
	 break;

      case 5:
	 if ( negate ) legal = (inputState->CurFontCmd() == FC_CHARSET_5);
	 level = inputState->Charset5();
	 stateChange = ((negate && level == 1) || (!negate && level == 0));
	 break;

      case 6:
	 if ( negate ) legal = (inputState->CurFontCmd() == FC_CHARSET_6);
	 level = inputState->Charset6();
	 stateChange = ((negate && level == 1) || (!negate && level == 0));
	 break;

      case 7:
	 if ( negate ) legal = (inputState->CurFontCmd() == FC_CHARSET_7);
	 level = inputState->Charset7();
	 stateChange = ((negate && level == 1) || (!negate && level == 0));
	 break;

      case 8:
	 if ( negate ) legal = (inputState->CurFontCmd() == FC_CHARSET_8);
	 level = inputState->Charset8();
	 stateChange = ((negate && level == 1) || (!negate && level == 0));
	 break;

      case 9:
	 if ( negate ) legal = (inputState->CurFontCmd() == FC_CHARSET_9);
	 level = inputState->Charset9();
	 stateChange = ((negate && level == 1) || (!negate && level == 0));
	 break;

      default:
         return False;

   } // End switch character set number

   if ( !legal ) return True;

//
// Store any pending text
//
   FlushTextBuf();

//
// If the input state is changing and the text is not empty, we need a new
//    command object.
//
   if ( stateChange && inputCmd->text->size() > 0 ) {
      RichCmdC	*rcmd = new RichCmdC(RC_TEXT);
      rcmd->state = inputCmd->state;
      inputLine->AddCommand(rcmd, /*after*/inputCmd);
      inputCmd = rcmd;
   }

   inputState = &inputCmd->state;

//
// Process this command
//
   if ( negate ) inputState->PopFont();
   else switch (charsetNum) {

      case 1: inputState->PushFont(FC_CHARSET_1); break;
      case 2: inputState->PushFont(FC_CHARSET_2); break;
      case 3: inputState->PushFont(FC_CHARSET_3); break;
      case 4: inputState->PushFont(FC_CHARSET_4); break;
      case 5: inputState->PushFont(FC_CHARSET_5); break;
      case 6: inputState->PushFont(FC_CHARSET_6); break;
      case 7: inputState->PushFont(FC_CHARSET_7); break;
      case 8: inputState->PushFont(FC_CHARSET_8); break;
      case 9: inputState->PushFont(FC_CHARSET_9); break;

   } // End switch character set number

   return True;

} // End AddCharsetCommand

/*----------------------------------------------------------------------
 * Method to add a new character set
 */

int
MimeRichTextC::AddCharset(CharC cs)
{
   unsigned	count = priv->charsetList.size();
   int i=0; for (i=0; i<count; i++) {
      StringC	*name = (StringC*)*priv->charsetList[i];
      if ( cs.Equals(*name, IGNORE_CASE) ) return i;
   }

   if ( count == MAX_CHARSET_COUNT ) return 0;

   StringC	*newCharset = new StringC(cs);
   void	*tmp = newCharset;
   priv->charsetList.add(tmp);

   return (priv->charsetList.size() - 1);

} // End AddCharset

/*----------------------------------------------------------------------
 * Method to specify character set
 */

void
MimeRichTextC::SetCharset(StringC name)
{
   if ( strncasecmp(name, "iso-", 4) == 0 )
      name(3,1) = "";	// Remove '-'

   priv->curCharset = AddCharset(name);

#if 0
   if ( priv->inputCmd )
      priv->AddCharsetCommand(name, False/*not negate*/);
#endif
}

/*----------------------------------------------------------------------
 * Method to return the name of the specified character set
 */

const StringC&
MimeRichTextC::Charset(int num) const
{
   if ( num < 0 || num >= priv->charsetList.size() )
      num = 0;

   return *(StringC*)*priv->charsetList[num];
}

const StringC&
MimeRichTextC::Charset() const
{
   return Charset(priv->curCharset);
}

/*----------------------------------------------------------------------
 * Method to specify plain font
 */

void
MimeRichTextC::SetPlainFont(const char *cs)
{
//
// Save the current row and column counts
//
   int	rows = RowCount();
   int	cols = ColumnCount();

   priv->plainFont = cs;

//
// Get size of excerpt string
//
   int	dir, asc, dsc;
   XCharStruct	size;
   XTextExtents(priv->plainFont.xfont, priv->excerptStr,
   		priv->excerptStr.size(), &dir, &asc, &dsc, &size);
   priv->excerptWd = size.width;

//
// Calculate a new size for the shell that contains this widget
//
   Widget	shell = XtParent(MainWidget());
   while ( !XtIsShell(shell) ) shell = XtParent(shell);
   SetSize(rows, cols, shell);

//
// Update input manager
//
   if ( priv->realized ) {

      WArgList	args;
      XmFontList	fontList = XmFontListCreate(priv->plainFont.xfont,
					       XmSTRING_DEFAULT_CHARSET);
      args.FontList(fontList);
      XmImSetValues(priv->textDA, ARGS);
   }

//
// Redraw if necessary
//
   priv->changed = True;
   if ( !priv->defer ) Refresh();

} // End SetPlainFont

/*----------------------------------------------------------------------
 * Method to specify fixed font
 */

void
MimeRichTextC::SetFixedFont(const char *cs)
{
   priv->fixedFont = cs;

//
// Redraw if necessary
//
   priv->changed = True;
   if ( !priv->defer ) Refresh();
}

/*----------------------------------------------------------------------
 * Method to specify that fixed font is to be used always
 */

void
MimeRichTextC::ForceFixed(Boolean val)
{
   if ( val == priv->forceFixed ) return;

   priv->forceFixed = val;

//
// Redraw if necessary
//
   priv->changed = True;
   if ( !priv->defer ) Refresh();
}

/*----------------------------------------------------------------------
 * Methods to set foreground and background colors
 */

void
MimeRichTextC::SetBackground(Pixel bg)
{
   priv->bgColor = bg;
   priv->changed = True;

   WArgList	args;
   args.Background(priv->bgColor);

   XtSetValues(priv->textDA, ARGS);
   if ( priv->realized )
      XmImSetValues(priv->textDA, ARGS);

   if ( !priv->defer ) Refresh();
}

void
MimeRichTextC::SetForeground(Pixel fg)
{
   priv->fgColor = fg;
   priv->changed = True;

   WArgList	args;
   args.Foreground(priv->fgColor);

   XtSetValues(priv->textDA, ARGS);
   if ( priv->realized )
      XmImSetValues(priv->textDA, ARGS);

   if ( !priv->defer ) Refresh();
}

/*-----------------------------------------------------------------------
 *  Method to set type of text
 */

void
MimeRichTextC::SetTextType(TextTypeT type)
{
   priv->textType = type;
}

/*-----------------------------------------------------------------------
 *  Method to set the editability
 */

void
MimeRichTextC::SetEditable(Boolean val)
{
   if ( priv->editable == val ) return;

   priv->editable = val;

//
// Update input methods
//
   if ( priv->realized ) {

      if ( priv->editable ) {

	 XmImRegister(priv->textDA, 0);

	 XPoint		spot = priv->CursorLocation();

	 Pixmap		bgPixmap;
	 XtVaGetValues(priv->textDA, XmNbackgroundPixmap, &bgPixmap, NULL);

	 XmFontList	fontList = XmFontListCreate(priv->plainFont.xfont,
						    XmSTRING_DEFAULT_CHARSET);

	 WArgList		args;
	 args.Background(priv->bgColor);
	 args.Foreground(priv->fgColor);
	 args.BackgroundPixmap(bgPixmap);
	 args.FontList(fontList);
	 args.Add("XmNlineSpacing",   priv->lineSpacing);
	 args.Add("XmNspotLocation", &spot);

	 XmImSetValues(priv->textDA, ARGS);
      }
      else {
	 XmImUnregister(priv->textDA);
      }

   } // End if realized

} // End SetEditable

/*----------------------------------------------------------------------
 * Method to parse character class ranges.  Takes a string of the form:
 * 
 *    low[-high]:val[,low[-high]:val[...]]
 * 
 * and sets the indicated ranges to the specified values.
 *
 */

void
MimeRichTextC::SetCharClasses(const char *s)
{
   if ( !s || !s[0] ) return;

   int	base    = 10;			// in case we ever add octal, hex
   int	low     = -1;			// bounds of range [0..127]
   int	high    = -1;			// bounds of range [0..127]
   int	acc     = 0;			// accumulator
   int	numbers = 0;			// count of numbers per range
   int	digits  = 0;			// count of digits in a number
   int	len     = strlen(s);

   register int i=0; for (i=0; i<len; i++) {
      char	c = s[i];

      if ( isspace(c) ) {
	 continue;
      }

      else if ( isdigit(c) ) {
	 acc = acc * base + (c - '0');
	 digits++;
	 continue;
      }

      else if (c == '-') {
	 low = acc;
	 acc = 0;
	 if (digits == 0) {
//	    cerr << "Missing number in range string \"" << s
//		 << "\" (position " << i << ")" NL;
	    return;
	 }
	 digits = 0;
	 numbers++;
	 continue;
      }

      else if (c == ':') {
	 if (numbers == 0)      low  = acc;
	 else if (numbers == 1) high = acc;
	 else {
//	    cerr << "Too many numbers in range string \"" << s
//		 << "\" (position " << i << ")" NL;
	    return;
	 }
	 digits = 0;
	 numbers++;
	 acc = 0;
	 continue;
      }

      else if (c == ',') {	// now, process it

	 if (high < 0) {
	     high = low;
	     numbers++;
	 }
	 if (numbers != 2) {
//	    cerr << "Bad value number in range string \"" << s
//		 << "\" (position " << i << ")" NL;
	    return;
	 }
	 else if ( !priv->SetCharClassRange(low, high, acc) ) {
//	    cerr << "Bad range in range string \"" << s
//		 << "\" (position " << i << ")" NL;
	 }

	 low = high = -1;
	 acc = 0;
	 digits = 0;
	 numbers = 0;
	 continue;
      }

      else {
//	 cerr << "Bad character in range string \"" << s
//	      << "\" (position " << i << ")" NL;
	 return;
      }

   } // End for each character

   if (low < 0 && high < 0) return;

/*
 * now, process it
 */

   if (high < 0) high = low;
   if (numbers < 1 || numbers > 2) {
//      cerr << "Bad value number in range string \"" << s
//	   << "\" (position " << i << ")" NL;
   } else if ( !priv->SetCharClassRange (low, high, acc) ) {
//      cerr << "Bad range in range string \"" << s
//	   << "\" (position " << i << ")" NL;
   }

   return;

} // End SetCharClasses

/*----------------------------------------------------------------------
 * Method to set character class for specified range
 */

Boolean
MimeRichTextP::SetCharClassRange(int low, int high, int value)
{
   if (low < 0 || high > 255 || high < low) return False;

   for (; low <= high; low++) charClasses[low] = value;
   return True;
}

/*-----------------------------------------------------------------------
 *  Constructor for text line data
 */

TextLineC::TextLineC()
{
   index    = -1;
   softLine = NULL;
   next     = NULL;
   prev     = NULL;

   cmdList.AllowDuplicates(FALSE);
   cmdList.SetSorted(FALSE);

//
// Add an initial command
//
   AddCommand(new RichCmdC(RC_TEXT));
}

/*-----------------------------------------------------------------------
 *  Destructor for text line data
 */

TextLineC::~TextLineC()
{
   SoftLineC	*sline = softLine;
   while ( sline && sline->textLine == this ) {
      SoftLineC	*next = sline->next;
      delete sline;
      sline = next;
   }

//
// Delete state and string data structures
//
   u_int	count = cmdList.size();
   int i=0; for (i=0; i<count; i++) {
      RichCmdC	*cmd   = Cmd(i);
      delete cmd;
   }

} // End TextLineC destructor

/*----------------------------------------------------------------------
 * Add a command to the text line command list
 */

void
TextLineC::AddCommand(const RichCmdC *cmd, int pos)
{
   void	*tmp = (void*)cmd;
   if ( pos != NULL_CMD_POS )
      cmdList.insert(tmp, pos);
   else
      cmdList.append(tmp);
}

/*----------------------------------------------------------------------
 * Add a command to the text line command list
 */

void
TextLineC::AddCommand(const TextStateC *ts, const StringC *sp, int pos)
{
   RichCmdC	*cmd = new RichCmdC(RC_TEXT);
   cmd->state = *ts;
   *cmd->text = *sp;
   AddCommand(cmd, pos);
}

/*----------------------------------------------------------------------
 * Add a command to the text line command list after the given command
 */

void
TextLineC::AddCommand(const RichCmdC *cmd, const RichCmdC *after)
{
   void	*tmp = (void*)after;
   int	pos = cmdList.indexOf(tmp);

   tmp = (void*)cmd;
   if ( pos != cmdList.NULL_INDEX )
      cmdList.insert(tmp, pos+1);
   else
      cmdList.append(tmp);
}

/*----------------------------------------------------------------------
 * Add a soft line to the soft line list
 */

void
TextLineC::AddSoftLine(SoftLineC *line)
{
   if ( !softLine ) {
      softLine = line;
      return;
   }

   SoftLineC	*last = LastSoft();
   last->next = line;
   line->prev = last;
}

/*----------------------------------------------------------------------
 * See if this is a blank line
 */

Boolean
TextLineC::IsBlank()
{
   u_int	count = cmdList.size();
   int i=0; for (i=0; i<count; i++) {
      RichCmdC	*cmd = Cmd(i);
      if ( cmd->IsGraphic() || (cmd->text && cmd->text->length() > 0) )
	 return False;
   }

   return True;

} // End IsBlank

/*----------------------------------------------------------------------
 * Remove all excerpting from this line
 */

void
TextLineC::RemoveExcerpt()
{
   u_int	count = cmdList.size();
   int i=0; for (i=0; i<count; i++) {
      TextStateC	*state = State(i);
      state->Excerpt(0);
   }

} // End RemoveExcerpt

/*-----------------------------------------------------------------------
 *  Constructors for text state data
 */

TextStateC::TextStateC()
{
   jstack.AutoShrink(False);
   fstack.AutoShrink(False);
   cstack.AllowDuplicates(TRUE);
   cstack.SetSorted(FALSE);
   plink = NULL;
   Reset();
}

TextStateC::TextStateC(const TextStateC& ts)
{
   jstack.AutoShrink(False);
   fstack.AutoShrink(False);
   cstack.AllowDuplicates(TRUE);
   cstack.SetSorted(FALSE);
   plink = NULL;
   *this = ts;
}

/*-----------------------------------------------------------------------
 *  Reset text state to initial values
 */

void
TextStateC::Reset()
{
   excerpt	= 0;
   lindent	= 0;
   rindent	= 0;
   underline	= 0;

   jstack.Clear();
   fstack.Clear();
   cstack.removeAll();

   if (plink) {
     delete plink;
     plink = NULL;
   }

} // End TextStateC::Reset

/*-----------------------------------------------------------------------
 *  Text state Assignment operator
 */

void
TextStateC::operator=(const TextStateC& ts)
{
   excerpt	= ts.excerpt;
   lindent	= ts.lindent;
   rindent	= ts.rindent;
   underline	= ts.underline;
   jstack	= ts.jstack;
   fstack	= ts.fstack;
   cstack	= ts.cstack;
   if (plink != NULL) {
     delete plink;
     plink = NULL;
   }
   if (ts.plink) {
     plink = new StringC(*ts.plink);
   }

} // End operator=

/*-----------------------------------------------------------------------
 *  Text state comparison operator
 */

int
TextStateC::operator==(const TextStateC& ts) const
{
//
// Compare color lists
//
   if ( cstack.size() != ts.cstack.size() ) return False;

   u_int	count = cstack.size();
   int i=0; for (i=0; i<count; i++) {
      if ( *cstack[i] != *ts.cstack[i] ) return False;
   }

   if (plink == NULL) {
     if (ts.plink != NULL) return False;
   } else if (ts.plink == NULL) return False;
   else if (*plink != *ts.plink) return False;

   return (excerpt	== ts.excerpt &&
	   lindent	== ts.lindent &&
	   rindent	== ts.rindent &&
	   underline	== ts.underline &&
	   jstack	== ts.jstack &&
	   fstack	== ts.fstack);

} // End operator==

/*-----------------------------------------------------------------------
 *  Return a pointer to the current font as described by the font stack
 */

FontDataC*
MimeRichTextP::CurFont(const TextStateC& ts)
{
//
// See which base font to start with
//
   FontDataC	*font;
   //if ( ts.Fixed() || (forceFixed && !plainFont.IsFixed()) )
   if ( forceFixed || ts.Fixed() )
      font = &fixedFont;
   else
      font = &plainFont;

//
// See if a character set is specified.  If so, use the last one.  Otherwise
//    apply the default character set.
//
   int		charsetNum = curCharset;
   unsigned	count = ts.FontStack().size();
   int i=0; for (i=0; i<count; i++) {

      FontCmdT	cmd = (FontCmdT)ts.FontStack()[i];
      switch (cmd) {

	 case (FC_CHARSET_1):	charsetNum = 1;	break;
	 case (FC_CHARSET_2):	charsetNum = 2;	break;
	 case (FC_CHARSET_3):	charsetNum = 3;	break;
	 case (FC_CHARSET_4):	charsetNum = 4;	break;
	 case (FC_CHARSET_5):	charsetNum = 5;	break;
	 case (FC_CHARSET_6):	charsetNum = 6;	break;
	 case (FC_CHARSET_7):	charsetNum = 7;	break;
	 case (FC_CHARSET_8):	charsetNum = 8;	break;
	 case (FC_CHARSET_9):	charsetNum = 9;	break;

	 case (FC_BOLD):
	 case (FC_ITALIC):
	 case (FC_SMALLER):
	 case (FC_BIGGER):
	 case (FC_PLAIN):
	 case (FC_FIXED):
	 case (FC_UNDERLINE):
	 case (FC_URL):
	    break;

      } // End switch command

   } // End for each command

//
// Apply the charset
//
   font = font->Charset(pub->Charset(charsetNum));

//
// Process remaining commands
//
   for (i=0; i<count; i++) {

      FontCmdT	cmd = (FontCmdT)ts.FontStack()[i];
      switch (cmd) {

	 case (FC_BOLD):
	    font = font->Bold();
	    break;

	 case (FC_ITALIC):
	    font = font->Italic();
	    break;

	 case (FC_SMALLER):
	    font = font->Smaller();
	    break;

	 case (FC_BIGGER):
	    font = font->Bigger();
	    break;

	 case (FC_CHARSET_1):
	 case (FC_CHARSET_2):
	 case (FC_CHARSET_3):
	 case (FC_CHARSET_4):
	 case (FC_CHARSET_5):
	 case (FC_CHARSET_6):
	 case (FC_CHARSET_7):
	 case (FC_CHARSET_8):
	 case (FC_CHARSET_9):
	 case (FC_PLAIN):
	 case (FC_FIXED):
	 case (FC_UNDERLINE):
	 case (FC_URL):
	    break;

      } // End switch font command

   } // End for each font command

   return font;

} // End CurFont

/*-----------------------------------------------------------------------
 *  TextPosC constructors
 */

TextPosC::TextPosC()
{
   textLine = NULL;
   cmdPos   = 0;
   strPos   = 0;
}

TextPosC::TextPosC(TextLineC *l, int cpos, int spos)
{
   Set(l, cpos, spos);
}

TextPosC::TextPosC(const ScreenPosC& s)
{
   *this = s;
}

/*-----------------------------------------------------------------------
 *  TextPosC assignment
 */

void
TextPosC::Set(TextLineC *tline, int cpos, int spos)
{
   textLine = tline;
   cmdPos   = cpos;
   strPos   = spos;
}

TextPosC&
TextPosC::operator=(const TextPosC& t)
{
   Set(t.textLine, t.cmdPos, t.strPos);
   return *this;
}

TextPosC&
TextPosC::operator=(const ScreenPosC& s)
{
   if ( s.softLine ) textLine = s.softLine->textLine;
   else		     textLine = NULL;

   if ( s.data ) {
      cmdPos = s.data->cmdPos;
      strPos = s.data->textOff + s.charPos;
   }
   else {
      cmdPos = 0;
      strPos = 0;
   }

   return *this;
}

/*-----------------------------------------------------------------------
 *  TextPosC comparison
 */

int
TextPosC::operator==(const TextPosC& t) const
{
   return (textLine == t.textLine && cmdPos == t.cmdPos && strPos == t.strPos);
}

int
TextPosC::operator<(const TextPosC& t) const
{
   if ( !t.textLine ) return False;
   if ( !  textLine ) return True;

   if      ( textLine->index < t.textLine->index ) return True;
   else if ( textLine->index > t.textLine->index ) return False;

   if      ( cmdPos < t.cmdPos ) return True;
   else if ( cmdPos > t.cmdPos ) return False;

   if ( strPos < t.strPos ) return True;

   return False;
}

int
TextPosC::operator>(const TextPosC& t) const
{
   if ( !  textLine ) return False;
   if ( !t.textLine ) return True;

   if      ( textLine->index > t.textLine->index ) return True;
   else if ( textLine->index < t.textLine->index ) return False;

   if      ( cmdPos > t.cmdPos ) return True;
   else if ( cmdPos < t.cmdPos ) return False;

   if ( strPos > t.strPos ) return True;

   return False;
}

/*-----------------------------------------------------------------------
 *  ScreenPosC constructors
 */

ScreenPosC::ScreenPosC()
{
   softLine = NULL;
   data     = NULL;
   charPos  = 0;
   x        = 0;
}

ScreenPosC::ScreenPosC(SoftLineC *l, int xx)
{
   Set(l, xx);
}

ScreenPosC::ScreenPosC(const TextPosC& t)
{
   *this = t;
}

/*-----------------------------------------------------------------------
 *  ScreenPosC assignment
 */

void
ScreenPosC::Set(SoftLineC *l, int xx)
{
   softLine = l;
   x        = xx;
   data     = NULL;
   charPos  = 0;

   if ( softLine ) data = softLine->PickData(x);

   Snap();

   if ( data ) charPos = data->CharPos(x);
}

void
ScreenPosC::Set(SoftLineC *l, RichDrawDataC *d, int pos)
{
   softLine = l;
   data     = d;
   charPos  = pos;

   if ( data ) {

      RichCmdC	*cmd = data->softLine->textLine->Cmd(data->cmdPos);

      if ( pos == 0 )
	 x = data->x;

      else if ( cmd->IsText() ) {

	 if ( pos == data->string.size() )
	    x = data->x + data->width;

	 else {
	    int  dir, asc, dsc;
	    XCharStruct  size;
	    XTextExtents(data->font->xfont, data->string, pos, &dir, &asc, &dsc,
			 &size);
	    x = data->x + size.width;
	 }
      }

      else {	// It's a graphic
	 x = data->x + data->width;
      }
   }

   else if ( softLine )
      x = softLine->bounds.xmin;

   else
      x = 0;
}

ScreenPosC&
ScreenPosC::operator=(const ScreenPosC& s)
{
   softLine = s.softLine;
   data     = s.data;
   charPos  = s.charPos;
   x        = s.x;
   return *this;
}

ScreenPosC&
ScreenPosC::operator=(const TextPosC& t)
{
   softLine = NULL;
   data     = NULL;
   charPos  = 0;
   x        = 0;

   if ( !t.textLine ) return *this;

//
// Loop through lines and data until we reach the command position
//
   TextLineC	*tline = t.textLine;
   RichCmdC	*cmd   = tline->Cmd(t.cmdPos);

   SoftLineC	*sline = tline->softLine;
   while ( sline && sline->textLine == tline ) {

      RichDrawDataC	*data = sline->drawData;

//
// Look out for blank lines
//
      if ( !data ) {
	 Set(sline, NULL, 0);
	 return *this;
      }

//
// Loop through data
//
      while ( data ) {

	 if ( data->cmdPos == t.cmdPos ) {

	    if ( cmd->IsText() ) {

	       int	lastPos = data->textOff + data->string.size();

	       if ( t.strPos >= data->textOff && t.strPos <= lastPos ) {
		  Set(sline, data, t.strPos - data->textOff);
		  return *this;
	       }

//
// We could have passed the position if the cursor is at the end of a line
//    with blank space that was removed by wrapping
//
	       else if ( t.strPos < data->textOff ) {

		  if ( data->prev ) {
		     if ( sline->prev )
			Set(sline->prev, data->prev,
					 t.strPos - data->prev->textOff);
		     else
			Set(sline,       data->prev,
					 t.strPos - data->prev->textOff);
		  }
		  else			// Beginning of current line
		     Set(sline, data, 0);

		  return *this;
	       }

	    } // End if command is text

	    else {
	       Set(sline, data, t.strPos);
	       return *this;
	    }

	 } // End if matching command found

	 data = data->next;

      } // End for each data segment

      sline = sline->next;

   } // End for each line

//
// If we got this far, put it at the end of the last data block
//
   if ( tline->softLine ) {

      SoftLineC	*sline = tline->LastSoft();
      RichDrawDataC	*data = sline->LastData();
      if ( data )
	 Set(sline, data, data->string.size());
      else
	 Set(sline, NULL, 0);
   }

   return *this;

} // End operator==

/*-----------------------------------------------------------------------
 *  ScreenPosC comparison
 */

int
ScreenPosC::operator==(const ScreenPosC& s) const
{
   return (softLine == s.softLine && x == s.x);
}

int
ScreenPosC::operator<(const ScreenPosC& s) const
{
   return ( softLine->bounds.ymin < s.softLine->bounds.ymin ||
            (softLine == s.softLine && x < s.x) );
}

int
ScreenPosC::operator>(const ScreenPosC& s) const
{
   return ( softLine->bounds.ymin > s.softLine->bounds.ymin ||
            (softLine == s.softLine && x > s.x) );
}

/*-----------------------------------------------------------------------
 *  ScreenPosC snap position to character grid
 */

void
ScreenPosC::Snap()
{
   if ( !softLine ) return;

   if ( x <= softLine->bounds.xmin )      x = softLine->bounds.xmin;
   else if ( x >= softLine->bounds.xmax ) x = softLine->bounds.xmax;
   else if ( data )		          x = data->Snap(x);
   else				          x = softLine->bounds.xmax;

} // End ScreenPosC::Snap

/*-----------------------------------------------------------------------
 *  Find the position that corresponds to the next command 
 */

Boolean
MimeRichTextP::FindPosNextCmd(TextPosC& curPos, TextPosC *newPos)
{
   TextLineC	*curLine = curPos.textLine;
   int		curCmd   = curPos.cmdPos;

   unsigned	ccount = curLine->cmdList.size();
   curCmd++;
   if ( curCmd < ccount ) {
      newPos->Set(curLine, curCmd, 0);
      return True;
   }

   while ( curLine->next ) {

      curLine = curLine->next;
      ccount  = curLine->cmdList.size();
      if ( ccount > 0 ) {
	 newPos->Set(curLine, 0, 0);
	 return True;
      }
   }

   return False;

} // End FindPosNextCmd

/*-----------------------------------------------------------------------
 *  Find the position that corresponds to the previous command 
 */

Boolean
MimeRichTextP::FindPosPrevCmd(TextPosC& curPos, TextPosC *newPos)
{
   TextLineC	*curLine = curPos.textLine;
   int		curCmd   = curPos.cmdPos;

   curCmd--;
   if ( curCmd >= 0 ) {

      RichCmdC	*cmd = curLine->Cmd(curCmd);
      int	strPos = cmd->LastPos();
      newPos->Set(curLine, curCmd, cmd->LastPos());
      return True;
   }

   while ( curLine->prev ) {

      curLine = curLine->prev;
      curCmd  = curLine->cmdList.size() - 1;

      if ( curCmd >= 0 ) {

	 RichCmdC	*cmd = curLine->Cmd(curCmd);
	 newPos->Set(curLine, curCmd, cmd->LastPos());
	 return True;
      }
   }

   return False;

} // End FindPosPrevCmd

/*-----------------------------------------------------------------------
 *  Return a string describing the text state at the cursor
 */

StringC
MimeRichTextC::CursorStateString()
{
   StringC	str("");
   if ( priv->lastCursorState.Fixed() ) str += "Fixed";

   if ( priv->lastCursorState.Bold() ) {
      if ( str.size() > 0 ) str += "-";
      str += "Bold";
   }

   if ( priv->lastCursorState.Italic() ) {
      if ( str.size() > 0 ) str += "-";
      str += "Italic";
   }

   if ( priv->lastCursorState.Smaller() ) {
      if ( str.size() > 0 ) str += "-";
      str += "Smaller(";
      str += priv->lastCursorState.Smaller();
      str += ")";
   }

   if ( priv->lastCursorState.Bigger() ) {
      if ( str.size() > 0 ) str += "-";
      str += "Bigger(";
      str += priv->lastCursorState.Bigger();
      str += ")";
   }

   if ( priv->lastCursorState.Underline() ) {
      if ( str.size() > 0 ) str += "-";
      str += "Underline";
   }

   if ( priv->lastCursorState.Excerpt() ) {
      if ( str.size() > 0 ) str += "-";
      str += "Excerpt(";
      str += (int)priv->lastCursorState.Excerpt();
      str += ")";
   }

   if ( priv->lastCursorState.LIndent() ) {
      if ( str.size() > 0 ) str += "-";
      str += "IndentLeft(";
      str += (int)priv->lastCursorState.LIndent();
      str += ")";
   }

   if ( priv->lastCursorState.RIndent() ) {
      if ( str.size() > 0 ) str += "-";
      str += "IndentRight(";
      str += (int)priv->lastCursorState.RIndent();
      str += ")";
   }

   switch ( priv->lastCursorState.CurJustCmd() ) {
      case JC_LEFT:	break;
      case JC_RIGHT:
         if ( str.size() > 0 ) str += "-";
	 str += "FlushRight";
	 break;
      case JC_BOTH:
         if ( str.size() > 0 ) str += "-";
         str += "FlushBoth";
	 break;
      case JC_CENTER:
         if ( str.size() > 0 ) str += "-";
         str += "Center";
	 break;
      case JC_NOFILL:
	 break;
   }

   return str;

} // End CursorStateString

/*-----------------------------------------------------------------------
 *  Method to insert a line break at the "curPos" position.  Return the
 *     first position in the new line in "newPos".  Adjust "updPos" if
 *     if is affected.
 */

void
MimeRichTextP::InsertLineBreak(TextPosC curPos, TextPosC *newPos)
{
//   if ( debuglev > 2 )
//      cout <<"Inserting line break at pos " <<curPos.cmdPos <<", "
//	   <<curPos.strPos <<" in line " <<curPos.textLine <<endl;

//
// See if the current command needs to be split.  We want to end up at
//    the beginning or end of a command.
//
   RichCmdC	*curCmd = curPos.Cmd();
   if ( curCmd->IsText() && curPos.strPos > 0 &&
      			    curPos.strPos < curCmd->LastPos() ) {
//      if ( debuglev > 2 ) cout <<"   splitting current command" <<endl;
      SplitCommand(curPos, newPos);
      curPos = *newPos;
   }

//
// See how much is to be moved
//
   int	startPos;
   if ( curPos.strPos == 0 ) startPos = curPos.cmdPos;
   else			     startPos = curPos.cmdPos + 1;

//
// Create a new line and delete the default command if it's not needed.
//
   TextLineC	*curLine = curPos.textLine;
   TextLineC	*newLine = new TextLineC;
//   if ( debuglev > 2 ) cout <<"   created new line " <<newLine <<endl;

   if ( startPos < curLine->cmdList.size() ) {

      newLine->DeleteCmd(0);

//
// Move all commands to the right of curPos to the next line
//
      int i=startPos; for (i=startPos; i<curLine->cmdList.size(); i++) {
	 RichCmdC	*cmd = curLine->Cmd(i);
	 newLine->AddCommand(cmd);
      }
   }

   else {
      *newLine->State(0) = *curLine->State(curLine->cmdList.size()-1);
   }

   newPos->Set(newLine, 0, 0);

//
// Remove commands from first line
//
   int i;
   for (i=curLine->cmdList.size()-1; i>=startPos; i--)
      curLine->cmdList.remove(i);

//
// Leave at least one command in the first line
//
   if ( curLine->cmdList.size() == 0 ) {
      RichCmdC	*cmd = new RichCmdC(RC_TEXT);
      curLine->AddCommand(cmd);
      *curLine->State(0) = *newLine->State(0);
   }

//
// See which positions are affected
//
   FixPosAfterBreak(&cursorPos,    curPos, newLine);
   FixPosAfterBreak(&selectBegPos, curPos, newLine);
   FixPosAfterBreak(&selectEndPos, curPos, newLine);
   FixPosAfterBreak(&lastDelPos,   curPos, newLine);

//
// Format the new line
//
   FormatLine(newLine);
   AddLine(newLine, curLine);

} // InsertLineBreak

/*-----------------------------------------------------------------------
 *  Method to break a text command into two at the given position.  The
 *     new position will point to the beginning of the new command.
 */

void
MimeRichTextP::SplitCommand(const TextPosC& curPos, TextPosC *newPos)
{
//
// Create a new command.
//
   TextLineC	*curLine = curPos.textLine;
   RichCmdC	*curCmd = curPos.Cmd();
   RichCmdC	*newCmd = new RichCmdC(RC_TEXT);
   newCmd->state = curCmd->state;

//
// Add the new command after the current command
//
   newPos->textLine = curLine;
   newPos->cmdPos = curPos.cmdPos + 1;
   newPos->strPos = 0;
   curLine->AddCommand(newCmd, newPos->cmdPos);

//
// If the split command is text, move some text to the new command
//
   if ( curCmd->IsText() && curPos.strPos < curCmd->LastPos() ) {
      *newCmd->text = *curCmd->text;
      (*newCmd->text)(0,curPos.strPos) = "";
      curCmd->text->Clear(curPos.strPos);
   }

//
// See which positions are affected
//
   FixPosAfterSplit(&cursorPos,    curPos);
   FixPosAfterSplit(&selectBegPos, curPos);
   FixPosAfterSplit(&selectEndPos, curPos);
   FixPosAfterSplit(&lastDelPos,   curPos);

} // End SplitCommand

/*-----------------------------------------------------------------------
 *  Method to insert a command at the given position.
 */

void
MimeRichTextP::InsertCommand(RichCmdTypeT type, Boolean after,
			     const TextPosC& curPos, TextPosC *newPos)
{
//
// Create a new command.
//
   TextLineC	*curLine = curPos.textLine;
   RichCmdC	*curCmd  = curPos.Cmd();
   RichCmdC	*newCmd = new RichCmdC(type);
   newCmd->state    = curCmd->state;
   newPos->textLine = curPos.textLine;
   newPos->cmdPos   = curPos.cmdPos;
   newPos->strPos   = 0;
   if ( after ) newPos->cmdPos++;

//
// Add the new command
//
   curLine->AddCommand(newCmd, newPos->cmdPos);

//
// See which positions are affected
//
   FixPosAfterInsert(&cursorPos,    *newPos);
   FixPosAfterInsert(&selectBegPos, *newPos);
   FixPosAfterInsert(&selectEndPos, *newPos);
   FixPosAfterInsert(&lastDelPos,   *newPos);

} // End InsertCommand

/*-----------------------------------------------------------------------
 *  Method to delete text from the specified command.
 */

void
MimeRichTextP::DeleteText(const TextPosC& delPos, int len)
{
   if ( len <= 0 ) return;

   RichCmdC	*cmd = delPos.Cmd();
   (*cmd->text)(delPos.strPos, len) = "";

//
// Update any affected positions
//
   FixPosAfterDelText(&cursorPos,    delPos, len);
   FixPosAfterDelText(&selectBegPos, delPos, len);
   FixPosAfterDelText(&selectEndPos, delPos, len);
   FixPosAfterDelText(&lastDelPos,   delPos, len);
}

/*-----------------------------------------------------------------------
 *  Method to move text from a command to another line
 */

void
MimeRichTextP::MoveText(const TextPosC& movPos, int len, TextLineC *dstLine,
			int dstCmdPos)
{
   if ( len <= 0 ) return;

//
// Create a new command
//
   RichCmdC	*delCmd = movPos.Cmd();
   RichCmdC	*newCmd = new RichCmdC(RC_TEXT);
   newCmd->state = delCmd->state;
   *newCmd->text = (*delCmd->text)(movPos.strPos, len);
   dstLine->AddCommand(newCmd, dstCmdPos);

//
// Delete original info
//
   DeleteText(movPos, len);

} // End MoveText

/*-----------------------------------------------------------------------
 *  Method to delete the command at the given position.
 */

void
MimeRichTextP::DeleteCommand(const TextPosC& delPos)
{
   RichCmdC	*cmd = delPos.Cmd();

//
// If this is the last text command on the line, just clear it
//
   if ( cmd->IsText() && delPos.textLine->cmdList.size() == 1 ) {

      int	len = cmd->text->size();
      cmd->text->Clear();

      FixPosAfterDelText(&cursorPos,    delPos, len);
      FixPosAfterDelText(&selectBegPos, delPos, len);
      FixPosAfterDelText(&selectEndPos, delPos, len);
      FixPosAfterDelText(&lastDelPos,   delPos, len);
   }

   else {

      delPos.textLine->DeleteCmd(delPos.cmdPos);

//
// If this line is now empty, add a blank text command
//
      if ( delPos.textLine->cmdList.size() == 0 ) {
	 cmd = new RichCmdC(RC_TEXT);
	 delPos.textLine->AddCommand(cmd);
      }

//
// Update any affected positions
//
      FixPosAfterDelCmd(&cursorPos,    delPos);
      FixPosAfterDelCmd(&selectBegPos, delPos);
      FixPosAfterDelCmd(&selectEndPos, delPos);
      FixPosAfterDelCmd(&lastDelPos,   delPos);

   } // End if not deleting last text command

} // End DeleteCommand

/*-----------------------------------------------------------------------
 *  Method to move the command at the given position to another line.
 */

void
MimeRichTextP::MoveCommand(TextPosC& movPos, TextLineC *dstLine, int dstCmdPos)
{
   RichCmdC	*movCmd = movPos.Cmd();

//
// If this is the last text command on the line, just clear it
//
   if ( movCmd->IsText() && movPos.textLine->cmdList.size() == 1 ) {
      MoveText(movPos, movCmd->text->size(), dstLine, dstCmdPos);
   }

   else {

      movPos.textLine->cmdList.remove(movPos.cmdPos);
      dstLine->AddCommand(movCmd, dstCmdPos);

//
// If this line is now empty, add a blank text command
//
      if ( movPos.textLine->cmdList.size() == 0 ) {
	 RichCmdC	*cmd = new RichCmdC(RC_TEXT);
	 movPos.textLine->AddCommand(cmd);
      }

//
// Update any affected positions
//
      FixPosAfterDelCmd(&cursorPos,    movPos);
      FixPosAfterDelCmd(&selectBegPos, movPos);
      FixPosAfterDelCmd(&selectEndPos, movPos);
      FixPosAfterDelCmd(&lastDelPos,   movPos);

   } // End if not moving last remaining text command

} // End MoveCommand

/*-----------------------------------------------------------------------
 *  Method to delete the specified line
 */

void
MimeRichTextP::DeleteLine(TextLineC *line)
{
   if ( line->index >= 0 ) {

      RemoveLine(line);

//
// Update any affected positions
//
      FixPosAfterDelLine(&cursorPos,    line);
      FixPosAfterDelLine(&selectBegPos, line);
      FixPosAfterDelLine(&selectEndPos, line);
      FixPosAfterDelLine(&lastDelPos,   line);
   }

   delete line;

} // End DeleteLine

/*-----------------------------------------------------------------------
 *  Method to move a line to another line list
 */

void
MimeRichTextP::MoveLine(TextLineC *line, TextLineC *prev)
{
   if ( line->index >= 0 ) {

      RemoveLine(line);

//
// Update any affected positions
//
      FixPosAfterDelLine(&cursorPos,    line);
      FixPosAfterDelLine(&selectBegPos, line);
      FixPosAfterDelLine(&selectEndPos, line);
      FixPosAfterDelLine(&lastDelPos,   line);
   }

   AddLine(line, prev);

} // End MoveLine

/*-----------------------------------------------------------------------
 *  Methods to remove lines from the line list
 */

void
MimeRichTextP::RemoveLine(TextLineC *line)
{
   RemoveLines(line, line);
}

void
MimeRichTextP::RemoveLines(TextLineC *begLine, TextLineC *endLine)
{
   TextLineC	*tprev = begLine->prev;
   TextLineC	*tnext = endLine->next;

   if ( tprev ) tprev->next = tnext;
   else		topTextLine = tnext;

   if ( tnext ) tnext->prev = tprev;
   else		botTextLine = tprev;

   SoftLineC	*sprev = (tprev ? tprev->LastSoft()  : (SoftLineC*)NULL);
   SoftLineC	*snext = (tnext ? tnext->FirstSoft() : (SoftLineC*)NULL);

   if ( sprev ) sprev->next = snext;
   if ( snext ) snext->prev = sprev;

   if ( topSoftLine->textLine == begLine ) topSoftLine = snext;
   if ( botSoftLine->textLine == endLine ) botSoftLine = sprev;

//
// Update all affected indexes
//
   SetLineIndex(tnext, begLine->index);

   TextLineC	*line = begLine;
   while ( line && line != tnext ) {
      line->index = -1;
      line = line->next;
   }

} // End RemoveLines

/*-----------------------------------------------------------------------
 *  Method to merge two lines
 */

void
MimeRichTextP::MergeLines(TextLineC *lineA, TextLineC *lineB)
{
   lineA->cmdList += lineB->cmdList;

//
// Update any affected positions
//
   FixPosAfterMergeLines(&cursorPos,    lineA, lineB);
   FixPosAfterMergeLines(&selectBegPos, lineA, lineB);
   FixPosAfterMergeLines(&selectEndPos, lineA, lineB);
   FixPosAfterMergeLines(&lastDelPos,   lineA, lineB);

   lineB->cmdList.removeAll();	// So they don't get deleted

} // End MergeLines

/*-----------------------------------------------------------------------
 *  Method to merge two commands
 */

void
MimeRichTextP::MergeCommands(TextPosC& posA, TextPosC& posB)
{
   RichCmdC	*cmdA = posA.Cmd();
   RichCmdC	*cmdB = posB.Cmd();

   *cmdA->text += *cmdB->text;

   FixPosAfterMergeCmds(&cursorPos,    posA, posB);
   FixPosAfterMergeCmds(&selectBegPos, posA, posB);
   FixPosAfterMergeCmds(&selectEndPos, posA, posB);
   FixPosAfterMergeCmds(&lastDelPos,   posA, posB);

   cmdB->text->Clear();

} // End MergeCommands

/*-----------------------------------------------------------------------
 *  Method to see if updPos is affected by a line break at the end of brkPos
 */

void
MimeRichTextP::FixPosAfterBreak(TextPosC *updPos, const TextPosC& brkPos,
			        TextLineC *newLine)
{
   if ( updPos->textLine == brkPos.textLine &&
	updPos->cmdPos   >= brkPos.cmdPos ) {

      if ( updPos->cmdPos == brkPos.cmdPos &&
	   updPos->strPos >= brkPos.strPos )
	 updPos->strPos -= brkPos.strPos;

      updPos->cmdPos  -= brkPos.cmdPos;
      updPos->textLine = newLine;
   }

} // FixPosAfterBreak

/*-----------------------------------------------------------------------
 *  Method to see if updPos is affected by a command split at splPos
 */

void
MimeRichTextP::FixPosAfterSplit(TextPosC *updPos, const TextPosC& splPos)
{
   if ( updPos->textLine == splPos.textLine &&
	updPos->cmdPos   >= splPos.cmdPos ) {

      if ( updPos->cmdPos == splPos.cmdPos ) {
	 if ( updPos->strPos >= splPos.strPos ) {
	    updPos->strPos -= splPos.strPos;
	    updPos->cmdPos++;
	 }
      }
      else
	 updPos->cmdPos++;
   }

} // FixPosAfterSplit

/*-----------------------------------------------------------------------
 *  Method to see if updPos is affected by a command insertion at insPos
 */

void
MimeRichTextP::FixPosAfterInsert(TextPosC *updPos, const TextPosC& insPos)
{
   if ( updPos->textLine == insPos.textLine &&
	updPos->cmdPos   >= insPos.cmdPos )
      updPos->cmdPos++;
}

/*-----------------------------------------------------------------------
 *  Method to see if updPos is affected by a text deletion at delPos.
 */

void
MimeRichTextP::FixPosAfterDelText(TextPosC *updPos, const TextPosC& delPos,
				  int len)
{
   if ( updPos->textLine == delPos.textLine &&
	updPos->cmdPos   == delPos.cmdPos &&
	updPos->strPos   >  delPos.strPos ) {

      updPos->strPos -= len;
      if ( updPos->strPos < delPos.strPos )
	 updPos->strPos = delPos.strPos;
   }

} // FixPosAfterDelText

/*-----------------------------------------------------------------------
 *  Method to see if updPos is affected by a command deletion at delPos.
 */

void
MimeRichTextP::FixPosAfterDelCmd(TextPosC *updPos, const TextPosC& delPos)
{
   if ( updPos->textLine == delPos.textLine &&
	updPos->cmdPos   >= delPos.cmdPos ) {

//
// The update command was deleted.
//
      if ( updPos->cmdPos == delPos.cmdPos ) {

//
// If this is still a valid command position, use it.
//
	 if ( updPos->cmdPos < updPos->textLine->cmdList.size() )
	    updPos->strPos = 0;
	 else if ( updPos->cmdPos > 0 ) {
	    updPos->cmdPos--;
	    RichCmdC	*cmd = updPos->Cmd();
	    updPos->strPos = cmd->LastPos();
	 }
      }

      else {
	 updPos->cmdPos--;
      }
   }

} // FixPosAfterDelCmd

/*-----------------------------------------------------------------------
 *  Method to see if updPos is affected by a line deletion.
 */

void
MimeRichTextP::FixPosAfterDelLine(TextPosC *updPos, TextLineC *line)
{
   if ( updPos->textLine == line ) {

//
// See if there is a line after
//
      if ( line->next )
	 updPos->Set(line->next, 0, 0);

//
// See if there is a line before
//

      else if ( line->prev ) {
	 int		count = line->prev->cmdList.size();
	 RichCmdC	*lastCmd = line->prev->Cmd(count-1);
	 updPos->Set(line->prev, count-1, lastCmd->LastPos());
      }

      else
	 updPos->Set(NULL, 0, 0);
   }

} // FixPosAfterDelLine

/*-----------------------------------------------------------------------
 *  Method to see if updPos is affected by a line merge of B into A.
 */

void
MimeRichTextP::FixPosAfterMergeLines(TextPosC *updPos, TextLineC *lineA,
				     TextLineC *lineB)
{
   if ( updPos->textLine == lineB ) {
      int	offset = lineA->cmdList.size() - lineB->cmdList.size();
      updPos->cmdPos += offset;
      updPos->textLine = lineA;
   }
}

/*-----------------------------------------------------------------------
 *  Method to see if updPos is affected by a command merge of B into A.
 */

void
MimeRichTextP::FixPosAfterMergeCmds(TextPosC *updPos, const TextPosC& posA,
				    const TextPosC& posB)
{
   if ( updPos->textLine == posB.textLine &&
        updPos->cmdPos   == posB.cmdPos ) {
      int	offset = posA.Cmd()->text->size() - posB.Cmd()->text->size();
      updPos->strPos  += offset;
      updPos->cmdPos   = posA.cmdPos;
      updPos->textLine = posA.textLine;
   }
}

/*-----------------------------------------------------------------------
 *  Return the average character width
 */

int
MimeRichTextC::CharWidth() const
{
   int	wd;

   if ( priv->forceFixed ) //&& !priv->plainFont.IsFixed() )
      wd = (priv->fixedFont.xfont->min_bounds.width +
	    priv->fixedFont.xfont->max_bounds.width) / (int)2;
   else
      wd = (priv->plainFont.xfont->min_bounds.width +
	    priv->plainFont.xfont->max_bounds.width) / (int)2;

   return wd;
}

/*-----------------------------------------------------------------------
 *  Return the max character height
 */

int
MimeRichTextC::CharHeight() const
{
   int	ht;

   if ( priv->forceFixed ) //&& !priv->plainFont.IsFixed() )
      ht = priv->fixedFont.xfont->ascent + priv->fixedFont.xfont->descent;
   else
      ht = priv->plainFont.xfont->ascent + priv->plainFont.xfont->descent;

   return ht;
}

/*-----------------------------------------------------------------------
 *  Return the margins
 */

int MimeRichTextC::MarginHeight() const { return priv->marginHt; }
int MimeRichTextC::MarginWidth()  const { return priv->marginWd; }

/*-----------------------------------------------------------------------
 *  Return the max line height
 */

int
MimeRichTextC::LineHeight() const
{
   int	ht = CharHeight() + priv->lineSpacing;
   return ht;
}

/*-----------------------------------------------------------------------
 *  Return the row and column counts
 */

int
MimeRichTextC::RowCount() const
{
   if ( priv->singleLine ) return 1;

   int ht = priv->textSW ? priv->clipHt - 1
   			 : priv->drawHt - 2/*for accents on top line*/;
   ht -= priv->marginHt2;
   ht += priv->lineSpacing;

   int	rows = (ht > 0) ? (int)((float)ht / (float)LineHeight()) : 0;
   return rows;
}

int
MimeRichTextC::ColumnCount() const
{
   int wd = priv->textSW ? priv->clipWd - 1 : priv->drawWd;
   wd -= priv->marginWd2;

   int	cols = (wd > 0) ? (int)((float)wd / (float)CharWidth()) : 0;
   return cols;
}

/*-----------------------------------------------------------------------
 *  Add a graphic object
 */

void
MimeRichTextC::AddGraphic(RichGraphicC *rg)
{
//
// Store any pending text
//
   priv->FlushTextBuf();

//
// Create a new graphic command
//
   RichCmdC	*newCmd = new RichCmdC(RC_GRAPHIC);
   newCmd->state   = priv->inputCmd->state;
   newCmd->graphic = rg;

   rg->owner = priv;
   void	*tmp = (void*)rg;
   priv->graphicList.add(tmp);

//
// Add the graphic command to the end of the input stream
//
   priv->inputLine->AddCommand(newCmd, /*after*/priv->inputCmd);
   priv->inputCmd = newCmd;

} // End AddGraphic

/*-----------------------------------------------------------------------
 *  Query methods
 */

Pixel	MimeRichTextC::Background()    const { return priv->bgColor; }
Pixel	MimeRichTextC::Foreground()    const { return priv->fgColor; }
Boolean	MimeRichTextC::IsEditable()    const { return priv->editable; }
Widget	MimeRichTextC::TextArea()      const { return priv->textDA; }
Widget	MimeRichTextC::MainWidget()    const
   { return priv->textSW ? priv->textDA : priv->scrollForm; }
Drawable	MimeRichTextC::DrawTo()      const
   { return priv->textPm ? priv->textPm : priv->textWin; }

const StringC& MimeRichTextC::DefCharset() const
		{ return priv->defCharset; }
const StringC& MimeRichTextC::DefFixedFontName() const
		{ return priv->defFixedFontName; }
const StringC& MimeRichTextC::DefPlainFontName() const
		{ return priv->defPlainFontName; }
const StringC& MimeRichTextC::FixedFontName() const
		{ return priv->fixedFont.Name(); }
const StringC& MimeRichTextC::PlainFontName() const
		{ return priv->plainFont.Name(); }

PtrListC& MimeRichTextC::GraphicList() const { return priv->graphicList; }

/*-----------------------------------------------------------------------
 *  Assignment methods
 */

void
MimeRichTextC::ResizeWidth(Boolean val)
{
   if ( priv->resizeWidth == val ) return;

   priv->resizeWidth = val;

   priv->changed = True;
   if ( !priv->defer ) Refresh();
}

void MimeRichTextC::CheckFroms(Boolean val)   { priv->checkFroms   = val; }

void MimeRichTextC::AddStateChangeCallback(CallbackFn *fn, void *data)
     { AddCallback(priv->stateCalls, fn, data); }
void MimeRichTextC::RemoveStateChangeCallback(CallbackFn *fn, void *data)
     { RemoveCallback(priv->stateCalls, fn, data); }

void MimeRichTextC::AddTextChangeCallback(CallbackFn *fn, void *data)
     { AddCallback(priv->changeCalls, fn, data); }
void MimeRichTextC::RemoveTextChangeCallback(CallbackFn *fn, void *data)
     { RemoveCallback(priv->changeCalls, fn, data); }

void MimeRichTextC::AddLinkAccessCallback(CallbackFn *fn, void *data)
     { AddCallback(priv->linkCalls, fn, data); }
void MimeRichTextC::RemoveLinkAccessCallback(CallbackFn *fn, void *data)
     { RemoveCallback(priv->linkCalls, fn, data); }

/*-----------------------------------------------------------------------
 *  Comparison
 */

int
MimeRichTextC::compare(const MimeRichTextC& t) const
{
   int	result = 0;
   if      ( t.priv->textDA < priv->textDA ) result = -1;
   else if ( t.priv->textDA > priv->textDA ) result =  1;
   return result;
}

/*-----------------------------------------------------------------------
 *  See if there is any text in this widget
 */

Boolean
MimeRichTextC::IsEmpty() const
{
   TextLineC	*line = priv->topTextLine;
   while ( line ) {

      u_int	ccount = line->cmdList.size();
      int j=0; for (j=0; j<ccount; j++) {

//
// See if there's any data in this command
//
	 RichCmdC	*cmd = line->Cmd(j);
	 if ( (cmd->IsText() && cmd->text && cmd->text->size() > 0) ||
	       cmd->IsGraphic() )
	    return False;
      }

      line = line->next;
   }

   return True;
}

/*-----------------------------------------------------------------------
 *  See if there is any markup applied to the text in this widget
 */

Boolean
MimeRichTextC::HasMarkup() const
{
   TextLineC	*tline = priv->topTextLine;
   while ( tline ) {

      unsigned	ccount = tline->cmdList.size();
      int c=0; for (c=0; c<ccount; c++) {

	 TextStateC	*state = tline->State(c);
	 if ( state->Excerpt() > 0 || state->Underline() > 0 ||
	      state->LIndent() > 0 || state->RIndent()   > 0 ||
	      state->FontStack().size() > state->URL() ||
	      state->JustStack().size() > 0 )
	    return True;

      } // End for each command

      tline = tline->next;

   } // End for each text line

   return False;

} // End HasMarkup

/*-------------------------------------------------------------------
 *  Determine if there are any 8-bit characters in a string
 */

static Boolean
Contains8Bit(CharC str)
{
//
// Loop through the string
//
   const char	*cp = str.Addr();
   const char	*ep = cp + str.Length();
   while ( cp < ep ) {
#ifdef __CHAR_UNSIGNED__
      if ( *cp > 127 ) return True;
#else
      if ( *cp < 0 ) return True;
#endif
      cp++;
   }

   return False;

} // End Contains8Bit

/*-----------------------------------------------------------------------
 *  See if this text can be accurately represented as plain text
 */

Boolean
MimeRichTextC::CanBePlain() const
{
   TextLineC	*tline = priv->topTextLine;
   while ( tline ) {

      unsigned	ccount = tline->cmdList.size();
      int c=0; for (c=0; c<ccount; c++) {

	 TextStateC	*state = tline->State(c);
	 if ( state->Underline() > 0 )
	    return False;

	 if ( state->ColorCmdCount() > 0 )
	    return False;

	 if ( state->JustStack().size() >
	      (state->FlushLeft() + state->NoFill()) )
	    return False;

	 if ( state->FontStack().size() > state->URL() ) {

//
// See if the character set commands should apply.  They only matter if the
//    text contains 8-bit characters
//
	    if ( state->Charset() > 0 ) {
	       StringC	*text = tline->Text(c);
	       if ( text && Contains8Bit(*text) )
		  return False;
	    }

	    else // We have some other font commands
	       return False;
	 }

      } // End for each command

      tline = tline->next;

   } // End for each text line

   return True;

} // End CanBePlain

/*-----------------------------------------------------------------------
 *  See if this text contains any 8-bit characters
 */

Boolean
MimeRichTextC::Has8Bit() const
{
   TextLineC	*tline = priv->topTextLine;
   while ( tline ) {

      unsigned	ccount = tline->cmdList.size();
      int c=0; for (c=0; c<ccount; c++) {

	 StringC	*text = tline->Text(c);
	 if ( text && Contains8Bit(*text) )
	    return True;

      } // End for each command

      tline = tline->next;

   } // End for each text line

   return False;

} // End Has8Bit

/*-----------------------------------------------------------------------
 *  See if there are any graphic commands in this widget
 */

Boolean
MimeRichTextC::HasGraphics() const
{
   TextLineC	*tline = priv->topTextLine;
   while ( tline ) {

      unsigned	ccount = tline->cmdList.size();
      int c=0; for (c=0; c<ccount; c++) {

	 RichCmdC	*cmd = tline->Cmd(c);
	 if ( cmd->IsGraphic() ) return True;

      } // End for each command

      tline = tline->next;

   } // End for each text line

   return False;

} // End HasGraphics

/*-----------------------------------------------------------------------
 *  See if there is exactly one graphic command and no text in this widget
 */

Boolean
MimeRichTextC::IsSingleGraphic() const
{
   unsigned	gcount = 0;
   TextLineC	*tline = priv->topTextLine;
   while ( tline ) {

      unsigned	ccount = tline->cmdList.size();
      int c=0; for (c=0; c<ccount; c++) {

	 RichCmdC	*cmd = tline->Cmd(c);
	 if ( cmd->IsGraphic() ) {
	    gcount++;
	    if ( gcount > 1 ) return False;
	 }
	 else if ( cmd->text->size() > 0 )
	    return False;

      } // End for each command

      tline = tline->next;

   } // End for each text line

   return True;

} // End IsSingleGraphic

/*----------------------------------------------------------------------
 * Method to build a string from the current data
 */

void
MimeRichTextC::GetString(StringC& outbuf, TextTypeT type, int lineSize)
{
   if ( !priv->topTextLine ) return;

//
// If we're not realized, GetRangeData won't work
//
   if ( priv->realized ) {

      TextPosC	topPos(priv->topTextLine, 0, 0);

      TextLineC	*tline = priv->botTextLine;
      unsigned	cmdCount = tline->cmdList.size();
      RichCmdC	*cmd     = tline->Cmd(cmdCount-1);
      unsigned	strSize = cmd->LastPos();

      TextPosC	botPos(tline, cmdCount-1, strSize);

      priv->GetRangeData(&topPos, &botPos, outbuf, type, True, lineSize);

   } // End if realized

   else {

//
// Loop through text lines
//
      TextStateC	nullState;
      TextStateC	*prevState = &nullState;
      TextLineC		*tline = priv->topTextLine;
      while ( tline ) {

	 if ( tline != priv->topTextLine ) {
	    outbuf += '\n';
	    if ( type != TT_PLAIN ) outbuf += '\n';
	 }

	 u_int		ccount = tline->cmdList.size();
	 for (int c=0; c<ccount; c++) {

	    RichCmdC	*cmd = tline->Cmd(c);

//
// Get state change commands
//
	    TextStateC	*thisState = &cmd->state;
            priv->GetStateCommands(*prevState, *thisState, outbuf, type);
	    prevState = thisState;

//
// Get text
//
	    if ( cmd->IsText() ) outbuf += *cmd->text;
	    else		 cmd->graphic->GetText(outbuf);

	 } // End for each command

	 tline = tline->next;

      } // End for each text line

   } // End if not realized

   if ( debuglev > 1 ) cout <<"String is: [" <<outbuf <<"]" <<endl;

} // End GetString

/*----------------------------------------------------------------------
 * Method to set the prefix display in excerpts
 */

void
MimeRichTextC::SetExcerptString(CharC str)
{
   priv->excerptStr = str;

//
// Get the new size of the string
//
   int	dir, asc, dsc;
   XCharStruct	size;
   XTextExtents(priv->plainFont.xfont, priv->excerptStr,
   		priv->excerptStr.size(), &dir, &asc, &dsc, &size);
   priv->excerptWd = size.width;

//
// Redraw if necessary
//
   priv->changed = True;
   if ( !priv->defer ) Refresh();
}

/*----------------------------------------------------------------------
 * Method to reset pointer for looping through data with GetNextPart
 */

void
MimeRichTextC::ResetPartIndex()
{
   priv->nextPartPos.Set(priv->topTextLine, 0, 0);
   priv->moreParts = True;
}

/*----------------------------------------------------------------------
 * Method to return the text or graphic pointer for the next section after
 *    nextPartPos.  A section is a continuous run of text or a graphic.
 *    A NULL graphic is returned if we hit the end.
 */

RichCmdTypeT
MimeRichTextC::GetNextPart(StringC& outbuf, TextTypeT textType,
			   RichGraphicC **graphic, int lineSize)
{
   if ( !priv->moreParts ) {
      *graphic = NULL;
      return RC_GRAPHIC;
   }

//
// If we're at the beginning of a graphic command, return that one.
//
   RichCmdC	*cmd   = priv->nextPartPos.Cmd();
   if ( cmd->IsGraphic() && priv->nextPartPos.strPos == 0 ) {

      *graphic = cmd->graphic;

//
// Find the next command.  If it's on the same line, move to its beginning.
//    If it's on another line, move to the end of this graphic.  We do this
//    so we don't lose any newlines after graphics.
//
      TextPosC	newPos;
      priv->moreParts = priv->FindPosNextCmd(priv->nextPartPos, &newPos);
      if ( priv->moreParts ) {
	 ScreenPosC	sp1 = priv->nextPartPos;
	 ScreenPosC	sp2 = newPos;
	 if ( sp1.softLine == sp2.softLine )
	    priv->nextPartPos = newPos;	// To beg of next command
	 else
	    priv->nextPartPos.strPos++;	// To end of graphic
      }

      return RC_GRAPHIC;

   } // End if current command is a graphic

//
// If we're at the end of a graphic command, this must be the end of a line
//    (see above).  If so, add a newline and move to the beginning of the
//    next line.
//
   if ( cmd->IsGraphic() && priv->nextPartPos.strPos == 1 ) {

      outbuf += '\n';
      if ( textType == TT_ENRICHED ) outbuf += '\n';

//
// Move to the next command.  There has to be one or else we wouldn't be in
//    this block.
//
      priv->FindPosNextCmd(priv->nextPartPos, &priv->nextPartPos);

   } // End if at end of line

//
// If we're in a text command, find the next non-text command
//
   TextPosC	endPos = priv->nextPartPos;
   priv->moreParts = priv->FindPosNextCmd(priv->nextPartPos, &endPos);
   while ( priv->moreParts && endPos.Cmd()->IsText() )
      priv->moreParts = priv->FindPosNextCmd(endPos, &endPos);

//
// If we ran out of parts, move to the end if the last part
//
   if ( !priv->moreParts )
      endPos.strPos = endPos.Cmd()->text->size();

   priv->GetRangeData(&priv->nextPartPos, &endPos, outbuf, textType,
		      /*closeState=*/!priv->moreParts, lineSize);

   priv->nextPartPos = endPos;
   return RC_TEXT;

} // End GetNextPart

/*----------------------------------------------------------------------
 * Method to set the cursor position to a graphic
 */

void
MimeRichTextC::MoveCursor(RichGraphicC *graphic, int pos)
{
   TextLineC	*tline = priv->topTextLine;
   while ( tline ) {

      unsigned	ccount = tline->cmdList.size();
      int c=0; for (c=0; c<ccount; c++) {

	 RichCmdC	*cmd = tline->Cmd(c);
	 if ( cmd->IsGraphic() && cmd->graphic == graphic ) {

//
// Update the cursor
//
	    priv->HideCursor();
	    priv->cursorPos.Set(tline, c, pos);
	    priv->ScrollToCursor();
	    ScreenPosC	spos = priv->cursorPos;
	    priv->desiredCursorX = spos.x;
	    priv->ShowCursor();

	    return;
	 }
      }

      tline = tline->next;
   }

} // End MoveCursor

/*----------------------------------------------------------------------
 * Method to determine if we're at the beginning of an input line
 */

Boolean
MimeRichTextP::BeginningOfInputLine()
{
   if ( !inputLine ) return True;

   RichCmdC	*cmd = inputLine->Cmd(0);

   return (inputTextBuf.size() == 0 &&
           (!cmd || (cmd->IsText() && cmd->text->size() == 0)));
}

/*-----------------------------------------------------------------------
 *  Handle horizontal scroll
 */

void
MimeRichTextP::HandleHScroll(Widget, MimeRichTextP *This,
			     XmScrollBarCallbackStruct *sb)
{
//
// If this is a drag scroll, see if there are any more scroll events coming.
//    If there are, ignore this one.
//
   if ( sb->reason == XmCR_DRAG && sb->event->type == MotionNotify ) {

      XMotionEvent	*ev = (XMotionEvent*)sb->event;
      XEvent	next;
      if ( XCheckTypedEvent(ev->display, ev->type, &next) ) {
	 XPutBackEvent(ev->display, &next);
	 return;
      }
   }

   This->hsbVal = sb->value;
   if ( This->realized ) This->pub->Refresh();

} // End HandleHScroll

/*-----------------------------------------------------------------------
 *  Handle vertical scroll
 */

void
MimeRichTextP::HandleVScroll(Widget, MimeRichTextP *This,
			     XmScrollBarCallbackStruct *sb)
{
//
// If this is a drag scroll, see if there are any more scroll events coming.
//    If there are, ignore this one.
//
   if ( sb->reason == XmCR_DRAG && sb->event->type == MotionNotify ) {

      XMotionEvent	*ev = (XMotionEvent*)sb->event;
      XEvent	next;
      if ( XCheckTypedEvent(ev->display, ev->type, &next) ) {
	 XPutBackEvent(ev->display, &next);
	 return;
      }
   }

   This->vsbVal = sb->value;
   if ( This->realized ) This->pub->Refresh();

} // End HandleVScroll

/*-----------------------------------------------------------------------
 *  Set the number of rows and columns
 */

void
MimeRichTextC::SetSize(int rows, int cols, Widget ref)
{
   if ( priv->singleLine ) rows = 1;

   Dimension	newWd = CharWidth() * cols + priv->marginWd*2;
   Dimension	newHt = ((CharHeight() + priv->lineSpacing) * rows)
		      - priv->lineSpacing + 2/*for accents on top line*/
		      + priv->marginHt*2;

   priv->SetVisibleSize(newWd, newHt, ref);

} // End SetSize

/*-----------------------------------------------------------------------
 *  Set the sensitivity of the widgets
 */

void
MimeRichTextC::SetSensitive(Boolean sensitive)
{
   Widget	w = MainWidget();
   Boolean	isSens = XtIsSensitive(w);
   if ( ( sensitive &&  isSens) || (!sensitive && !isSens) ) return;

   XtSetSensitive(w, sensitive);
   Refresh();
}

/*-----------------------------------------------------------------------
 *  Constructor for RichGraphicC
 */

RichGraphicC::RichGraphicC()
{
   okToDelete = True;
   shown      = False;
   owner      = NULL;
}

/*-----------------------------------------------------------------------
 *  Destructor for RichGraphicC
 */

RichGraphicC::~RichGraphicC()
{
   if ( owner ) {
      void	*tmp = (void*)this;
      owner->graphicList.remove(tmp);
   }
}

/*-----------------------------------------------------------------------
 *  Show and hide methods
 */

void
RichGraphicC::Hide()
{
   shown = False;
   HideIt();
}

void
RichGraphicC::Show()
{
   shown = True;
   ShowIt();
}

/*-----------------------------------------------------------------------
 *  Method to enable/disable emacs translations
 */

void
MimeRichTextC::SetEmacsMode(Boolean val)
{
   if ( priv->emacsMode == val ) return;

   priv->emacsMode = val;
   priv->UpdateTranslations();
}

/*-----------------------------------------------------------------------
 *  Method to enable/disable emacs translations
 */

void
MimeRichTextC::SetDeleteLikeBackspace(Boolean val)
{
   if ( priv->delLikeBs == val ) return;

   priv->delLikeBs = val;
   priv->UpdateTranslations();
}

/*-----------------------------------------------------------------------
 *  Method to set translations
 */

void
MimeRichTextP::UpdateTranslations()
{
//
// Start with default translations
//
   XtOverrideTranslations(textDA, defaultTrans1);
//   XtOverrideTranslations(textDA, defaultTrans2);

//
// Apply delete key functionality
//
   if ( delLikeBs )
      XtOverrideTranslations(textDA, delLeftTrans);
   else
      XtOverrideTranslations(textDA, delRightTrans);

//
// Apply emacs overrides if necessary
//
   if ( emacsMode )
      XtOverrideTranslations(textDA, emacsTrans);

//
// Apply user overrides if necessary
//
   if ( userTrans.size() > 0 ) {

      if ( userTrans.StartsWith("#override") )
	 XtOverrideTranslations(textDA, XtParseTranslationTable(userTrans));
      else if ( userTrans.StartsWith("#augment") )
	 XtAugmentTranslations(textDA,  XtParseTranslationTable(userTrans));
      else
	 XtVaSetValues(textDA, XmNtranslations,
		       XtParseTranslationTable(userTrans), NULL);
   }

} // End UpdateTranslations

/*-----------------------------------------------------------------------
 *  Method to determine if undelete will do anything
 */

Boolean
MimeRichTextC::UndeleteOk() const
{
   return (priv->topSaveLine != NULL);
}

/*-----------------------------------------------------------------------
 *  Link a new line into the text line list after the specified line
 */

void
MimeRichTextP::AddLine(TextLineC* tnew, TextLineC *tprev)
{
//   if ( debuglev > 2 ) cout <<"Adding line " <<tnew <<" after " <<tprev <<endl;

   TextLineC	*tnext = tprev->next;
//   if ( debuglev > 2 ) cout <<"   tnext is " <<tnext <<endl;

   if ( tnext ) tnext->prev = tnew;
                tprev->next = tnew;
   tnew->prev = tprev;
   tnew->next = tnext;
   if ( tprev == botTextLine ) {
      botTextLine = tnew;
//      if ( debuglev > 2 ) cout <<"   tprev was bottom line" <<endl;
   }

//
// Update the links between soft lines
//
   SoftLineC *sprev  = tprev->LastSoft();
   SoftLineC *snext  = tnext ? tnext->FirstSoft() : (SoftLineC*)NULL;
   SoftLineC *nfirst = tnew->FirstSoft();
   SoftLineC *nlast  = tnew->LastSoft();

   if ( sprev  ) sprev->next  = nfirst;
   if ( snext  ) snext->prev  = nlast;
   if ( nfirst ) nfirst->prev = sprev;
   if ( nlast  ) nlast->next  = snext;

   if ( sprev == botSoftLine ) botSoftLine = nlast;

//
// Update text line indexes
//
   SetLineIndex(tnew, tprev->index+1);

} // End AddLine

/*-----------------------------------------------------------------------
 * Update text line indexes
 */

void
MimeRichTextP::SetLineIndex(TextLineC *line, int index)
{
   while ( line ) {
//      if ( debuglev > 2 )
//	 cout <<"   Setting index for line " <<line <<" to " <<index <<endl;
      line->index = index++;
      line = line->next;
   }
}

/*-----------------------------------------------------------------------
 * Update web browser command
 */

void
MimeRichTextC::SetWebCommand(const char *cmd)
{
   priv->webCommand = cmd;
}
