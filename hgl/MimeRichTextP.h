/*
 *  $Id: MimeRichTextP.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
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
#ifndef _MimeRichTextP_h_
#define _MimeRichTextP_h_

#include "FontDataC.h"
#include "RectC.h"
#include "StringC.h"
#include "PtrListC.h"
#include "PixelListC.h"
#include "CallbackListC.h"

#include <X11/Intrinsic.h>
#include <Xm/Xm.h>

#define MIME_ACTION_COUNT	73

/*----------------------------------------------------------------------
 * Class to hold state information about the text
 */

class TextStateC {

   char			excerpt;	// Current level of excerpting
   char			lindent;	// Current level of left indenting
   char			rindent;	// Current Level of right indenting
   char			underline;	// Current level of underlining

   StringC		fstack;		// Stack of font commands
   StringC		jstack;		// Stack of justify commands
   PixelListC		cstack;		// Stack of color commands
   StringC*		plink;		// Pointer to link value

public:

   TextStateC();
   TextStateC(const TextStateC&);

   inline char		Excerpt()   const { return excerpt; }
   inline char		LIndent()   const { return lindent; }
   inline char		RIndent()   const { return rindent; }
   inline char		Underline() const { return underline; }

   inline void		ExcerptMore()   { excerpt++; }
   inline void		ExcerptLess()   { if ( excerpt > 0 ) excerpt--; }
   inline void		Excerpt(char val) { excerpt = val; }
   inline void		LIndentMore()   { lindent++; }
   inline void		LIndentLess()   { if ( lindent > 0 ) lindent--; }
   inline void		LIndent(char val) { lindent = val; }
   inline void		RIndentMore()   { rindent++; }
   inline void		RIndentLess()   { if ( rindent > 0 ) rindent--; }
   inline void		RIndent(char val) { rindent = val; }
   inline void		UnderlineMore() { underline++; }
   inline void		UnderlineLess() { if ( underline > 0 ) underline--; }
   inline void		Underline(char val) { underline = val; }

   inline int	CmdCount(FontCmdT fc) const {
      int	count = 0;
      for (int i=0; i<fstack.size(); i++) if ( fstack[i] == fc ) count++;
      return count;
   }
   inline int	CmdCount(JustCmdT jc) const {
      int	count = 0;
      for (int i=0; i<jstack.size(); i++) if ( jstack[i] == jc ) count++;
      return count;
   }
   inline int	ColorCount(Pixel pix) const {
      int	count = 0;
      for (int i=0; i<cstack.size(); i++) if ( *cstack[i] == pix ) count++;
      return count;
   }
   inline int	PLinkCount() const	{ return (plink ? 1 : 0); }
   inline int	ColorCmdCount() const   { return cstack.size(); }
   inline int	Bold() const		{ return CmdCount(FC_BOLD);   }
   inline int	Italic() const		{ return CmdCount(FC_ITALIC); }
   inline int	Fixed() const		{ return CmdCount(FC_FIXED);  }
   inline int	Smaller() const		{ return CmdCount(FC_SMALLER);  }
   inline int	Bigger() const		{ return CmdCount(FC_BIGGER);  }
   inline int	Charset1() const	{ return CmdCount(FC_CHARSET_1);  }
   inline int	Charset2() const	{ return CmdCount(FC_CHARSET_2);  }
   inline int	Charset3() const	{ return CmdCount(FC_CHARSET_3);  }
   inline int	Charset4() const	{ return CmdCount(FC_CHARSET_4);  }
   inline int	Charset5() const	{ return CmdCount(FC_CHARSET_5);  }
   inline int	Charset6() const	{ return CmdCount(FC_CHARSET_6);  }
   inline int	Charset7() const	{ return CmdCount(FC_CHARSET_7);  }
   inline int	Charset8() const	{ return CmdCount(FC_CHARSET_8);  }
   inline int	Charset9() const	{ return CmdCount(FC_CHARSET_9);  }
   inline int	URL() const		{ return CmdCount(FC_URL);   }
   inline int	Center() const		{ return CmdCount(JC_CENTER);  }
   inline int	FlushLeft() const	{ return CmdCount(JC_LEFT);  }
   inline int	FlushRight() const	{ return CmdCount(JC_RIGHT);  }
   inline int	FlushBoth() const	{ return CmdCount(JC_BOTH);  }
   inline int	NoFill() const		{ return CmdCount(JC_NOFILL);  }
   inline int	Charset() const		{ return (Charset1() + Charset2() +
						  Charset3() + Charset4() +
						  Charset5() + Charset6() +
						  Charset7() + Charset8() +
						  Charset9()); }

   inline       StringC&	FontStack()       { return fstack; }
   inline const StringC&	FontStack() const { return fstack; }
   inline FontCmdT	CurFontCmd() const {
      return (FontCmdT)fstack.LastChar();
   }
   inline void		PushFont(FontCmdT fcmd) {
      fstack += (char)fcmd;
   }
   inline void		PopFont() {
      if ( fstack.size() > 0 ) fstack.Clear(fstack.size()-1);
   }

   inline       StringC&	JustStack()       { return jstack; }
   inline const StringC&	JustStack() const { return jstack; }
   inline JustCmdT	CurJustCmd() const {
      return (JustCmdT)jstack.LastChar();
   }
   inline void		PushJust(JustCmdT jcmd) {
      jstack += (char)jcmd;
   }
   inline void		PopJust() {
      if ( jstack.size() > 0 ) jstack.Clear(jstack.size()-1);
   }

   inline       PixelListC&	ColorStack()       { return cstack; }
   inline const PixelListC&	ColorStack() const { return cstack; }
   inline Pixel	CurColor() const {
      return (cstack.size() > 0 ? *cstack[cstack.size()-1] : (Pixel)NULL);
   }
   inline void		PushColor(Pixel pix) {
      cstack.add(pix);
   }
   inline void		PopColor() {
      if ( cstack.size() > 0 ) cstack.remove(cstack.size()-1);
   }

   inline	StringC*	PLink()		    { return plink; }
   inline void		PushLink(StringC &lnk) {
      if (plink) delete plink; plink = new StringC(lnk);
   }
   inline void		PopLink() {
      if (plink) delete plink; plink = NULL;
   }

   void		Reset();
   void		operator=(const TextStateC&);
   int		operator==(const TextStateC&) const;
   int		operator!=(const TextStateC& t) const { return !(*this == t); }
};

#define NULL_CMD_POS	-1

/*----------------------------------------------------------------------
 * Rich/Enriched text drawing data
 */

class SoftLineC;

class RichDrawDataC {

public:

   int		x;
   int		width;
   FontDataC	*font;
   StringC	string;
   Boolean	underline;
   SoftLineC	*softLine;	// Soft line to which this data belongs
   int		cmdPos;		// Index of the command that this
   				//    data represents.
   int		textOff;	// Where this data string starts relative
   				//    to the command string.
   RichGraphicC		*graphic;// If this is a graphic object
   RichDrawDataC	*prev;	// Previous draw data
   RichDrawDataC	*next;	// Next draw data

   RichDrawDataC() {
      x         = 0;
      width     = 0;
      font      = NULL;
      underline = False;
      softLine  = NULL;
      cmdPos    = 0;
      textOff   = 0;
      graphic   = NULL;
      prev      = NULL;
      next      = NULL;
   }

   int		Snap(int);
   int		CharPos(int);
   int		LastPos() { return (graphic ? 1 : string.size()); }

//
// comparison
//
   inline int	compare(const RichDrawDataC& d) const {
      int	result = 0;
      if      ( d.width < width ) result = -1;
      else if ( d.width > width ) result =  1;
      return result;
   }
   inline int	operator<(const RichDrawDataC& d) const
	{ return (compare(d) < 0); }
   inline int	operator>(const RichDrawDataC& d) const
	{ return (compare(d) > 0); }
   inline int	operator==(const RichDrawDataC& d) const
	{ return (compare(d) == 0); }
   inline int	operator!=(const RichDrawDataC& d) const
	{ return (compare(d) != 0); }
};

/*----------------------------------------------------------------------
 * Rich/Enriched line data as wrapped and displayed on the screen
 */

class TextLineC;

class SoftLineC {

public:

   TextLineC		*textLine;
   RectC		bounds;
   int			totalWd;
   int			descent;
   RichDrawDataC	*drawData;	// Linked list of drawing elements
   SoftLineC		*prev;
   SoftLineC		*next;

   SoftLineC();
   ~SoftLineC();

   void			AddDrawData(RichDrawDataC*);
   RichDrawDataC	*PickData(int);

   inline RichDrawDataC	*FirstData() { return drawData; }
   inline RichDrawDataC	*LastData() {
      RichDrawDataC	*data = drawData;
      if ( data ) {
	 while ( data->next ) data = data->next;
      }
      return data;
   }
//
// comparison
//
   inline int	compare(const SoftLineC& l) const {
      int	result = 0;
      if      ( l.bounds.ymin < bounds.ymin ) result = -1;
      else if ( l.bounds.ymin > bounds.ymin ) result =  1;
      return result;
   }
   inline int	operator<(const SoftLineC& l) const
	{ return (compare(l) < 0); }
   inline int	operator>(const SoftLineC& l) const
	{ return (compare(l) > 0); }
   inline int	operator==(const SoftLineC& l) const
	{ return (compare(l) == 0); }
   inline int	operator!=(const SoftLineC& l) const
	{ return (compare(l) != 0); }
};

/*----------------------------------------------------------------------
 * Each command consists of a text state and a string.
 */

class RichCmdC {

public:

   RichCmdTypeT	type;
   TextStateC	state;
   union {
      StringC		*text;
      RichGraphicC	*graphic;
   };

   Boolean	IsText()    { return (type == RC_TEXT); }
   Boolean	IsGraphic() { return (type == RC_GRAPHIC); }
   int		LastPos()   { return (type == RC_TEXT ? text->size() : 1); }

   RichCmdC(RichCmdTypeT t) {
      type = t;
      if ( IsText() ) text = new StringC;
      else	      graphic = NULL;
   }

   ~RichCmdC() {
      if      ( IsText()                       ) delete text;
      else if ( graphic && graphic->okToDelete ) delete graphic;
   }

};

/*----------------------------------------------------------------------
 * Rich/Enriched unformatted line data as defined by the user.  TextLines
 *    are defined by newlines in the input.
 */

class TextLineC {

public:

   PtrListC		cmdList;	// List of strings and state changes
   SoftLineC		*softLine;	// wrapped lines for this line
   RectC		bounds;		// Combination of all soft lines
   int			index;
   TextLineC		*prev;
   TextLineC		*next;

   TextLineC();
   ~TextLineC();

   void			AddCommand(const RichCmdC*, int pos=NULL_CMD_POS);
   void			AddCommand(const RichCmdC*, const RichCmdC*);
   void			AddCommand(const TextStateC*, const StringC*,
   				   int pos=NULL_CMD_POS);
   void			AddSoftLine(SoftLineC*);

   inline RichCmdC	*Cmd(int i) {
      return ((i>=0 && i<cmdList.size()) ? (RichCmdC*)*cmdList[i]
      					 : (RichCmdC*)NULL);
   }
   inline RichCmdC	*CurCmd() { return Cmd(cmdList.size()-1); }
   inline TextStateC	*State(int i) {
      RichCmdC	*cmd = Cmd(i);
      return (cmd ? &cmd->state : (TextStateC*)NULL);
   }
   inline StringC	*Text(int i) {
      RichCmdC	*cmd = Cmd(i);
      return (cmd && cmd->IsText() ? cmd->text : (StringC*)NULL);
   }
   inline RichGraphicC	*Graphic(int i) {
      RichCmdC	*cmd = Cmd(i);
      return (cmd && cmd->IsGraphic() ? cmd->graphic : (RichGraphicC*)NULL);
   }
   inline void		DeleteCmd(int i) {
      RichCmdC	*cmd = Cmd(i);
      if ( cmd ) {
	 delete cmd;
	 cmdList.remove(i);
      }
   }
   inline SoftLineC	*FirstSoft() { return softLine; }
   inline SoftLineC	*LastSoft() {
      SoftLineC	*line = softLine;
      SoftLineC	*last = softLine;
      while ( line && line->textLine == this ) {
	 line = line->next;
	 if ( line && line->textLine == this ) last = line;
      }
      return last;
   }

   Boolean		IsBlank();
   void			RemoveExcerpt();

//
// comparison
//
   inline int		compare(const TextLineC&) const { return 0; }
   inline int		operator<(const TextLineC&) const { return 0; }
   inline int		operator>(const TextLineC&) const { return 0; }
   inline int		operator==(const TextLineC&) const { return 1; }
   inline int		operator!=(const TextLineC&) const { return 0; }
};

/*----------------------------------------------------------------------
 * Class to hold information about a text location
 */

class ScreenPosC;

class TextPosC {

public:

   TextLineC	*textLine;
   int		cmdPos;
   int		strPos;

   TextPosC();
   TextPosC(TextLineC*, int, int);
   TextPosC(const ScreenPosC&);

   RichCmdC	*Cmd()   const
      { return textLine ? textLine->Cmd(cmdPos)   : (RichCmdC*)NULL; }
   TextStateC	*State() const
      { return textLine ? textLine->State(cmdPos) : (TextStateC*)NULL; }

   void		Set(TextLineC*, int, int);
   TextPosC&	operator=(const TextPosC&);
   TextPosC&	operator=(const ScreenPosC&);

   int	operator==(const TextPosC&) const;
   int	operator<(const TextPosC&) const;
   int	operator>(const TextPosC&) const;
   inline int operator!=(const TextPosC& tp) const { return !(*this == tp); }
   inline int operator<=(const TextPosC& tp) const { return !(*this > tp); }
   inline int operator>=(const TextPosC& tp) const { return !(*this < tp); }
};

/*----------------------------------------------------------------------
 * Class to hold information about a screen location
 */

class ScreenPosC {

public:

   SoftLineC		*softLine;
   RichDrawDataC	*data;
   int			charPos;	// Position within data string
   int			x;		// Position on screen

   ScreenPosC();
   ScreenPosC(SoftLineC*, int);
   ScreenPosC(const TextPosC&);

   void	Set(SoftLineC*, int x);
   void	Set(SoftLineC*, RichDrawDataC*, int pos);
   ScreenPosC&	operator=(const ScreenPosC&);
   ScreenPosC&	operator=(const TextPosC&);

   int	operator==(const ScreenPosC&) const;
   int	operator<(const ScreenPosC&) const;
   int	operator>(const ScreenPosC&) const;
   inline int operator!=(const ScreenPosC& sp) const { return !(*this == sp); }
   inline int operator<=(const ScreenPosC& sp) const { return !(*this > sp); }
   inline int operator>=(const ScreenPosC& sp) const { return !(*this < sp); }

   void	Snap();
};

/*----------------------------------------------------------------------
 * Rich/Enriched text drawing class
 */

class CharC;

class MimeRichTextP {

   friend class MimeRichTextC;
   friend class RichSearchWinC;
   friend class RichGraphicC;

   MimeRichTextC	*pub;

   Widget		textSW;		// Scrolled window parent
   Widget		textClip;	// Scrolled window clip window

   Widget		scrollForm;	// Parent if no scrolled window
   Widget		hlForm;		// Highlight
   Widget		textFrame;	// Scrollbars
   Widget		textVSB;	// Scrollbars
   Widget		textHSB;	// Scrollbars

   Widget		textDA;		// Drawing area

//
// Data for scrolling
//
   int		hsbVal;			// Position of horizontal scroll bar
   int		vsbVal;			// Position of vertical scroll bar
   int		hsbMax;			// horizontal scroll bar max - slider
   int		vsbMax;			// vertical scroll bar max - slider
   Boolean	hsbOn;			// True if scrolling enabled
   Boolean	vsbOn;			// True if scrolling enabled
   Boolean	hsbAlwaysOn;		// User setting
   Boolean	vsbAlwaysOn;		// User setting
   int		scrollRoff;		// Offset used if scrolling
   int		scrollBoff;		// Offset used if scrolling
   int		noScrollRoff;		// Offset used if not scrolling
   int		noScrollBoff;		// Offset used if not scrolling
   Pixmap	textPm;
   u_int	textPmWd;
   u_int	textPmHt;

//
// Input parser variables
//
   RichCmdC	*inputCmd;	// Current input state and text
   TextLineC	*inputLine;	// Current line being filled
   StringC	inputTextBuf;	// Working text buffer
   Boolean	inputLastWhite;	// True if last character was whitespace

   StringC	excerptStr;	// Prefix used for excerpts
   int		excerptWd;	// Size of excerpt string

   TextPosC		lastDelPos;	// Start position of deleted text
   SoftLineC		*topSoftLine;	// Pointers to soft line lists
   SoftLineC		*botSoftLine;
   TextLineC		*topTextLine;	// Pointers to text line lists
   TextLineC		*botTextLine;
   TextLineC		*topSaveLine;	// Pointers to saved line lists
   TextLineC		*botSaveLine;
   u_int		maxLength;

//
// Drawing variables
//
   Boolean		realized;
   Window		textWin;
   GC			textGC;
   Pixel		fgColor;
   Pixel		bgColor;
   Pixel		urlColor;
   Pixel		linkColor;
   Pixel		cursorColor;
   Pixmap		stipplePm;
   Dimension		marginWd,  marginHt;
   Dimension		marginWd2, marginHt2;
   Dimension		clipWd, clipHt;	// Size of clip win (if in scroll win)
   Dimension		initWd, initHt;	// Initial size of drawing area
   Dimension		drawWd, drawHt;	// Current size of drawing area
   Dimension		maxWd,  maxHt;	// Max size of drawing area
   int			textWd, textHt;	// Size of text data
   int			textX,  textY;
   Dimension		indentWd;
   int			hlThick;
   Pixel		hlColor;
   Pixel		hlFormBg;
   char			lineSpacing;
   Boolean		resizeWidth;
   Boolean		editable;
   Boolean		hasFocus;
   char			defer;
   Boolean		changed;
   TextTypeT		textType;
   char			tabStop;
   Boolean		checkFroms;
   Boolean		singleLine;
   Boolean		tabTraverses;	// If True, TAB causes traversal and
   					// Ctrl-TAB inserts tab.  If False,
					// vice-versa.
   Boolean		forceFixed;	// Always use fixed font
   PtrListC		graphicList;	// List of graphic objects for quick
   					//    picking

//
// Cursor data
//
   TextPosC		cursorPos;
   TextPosC		lastCursorPos;	// Saved in HideCur, checked in ShowCur
   int			desiredCursorX;
   Boolean		cursorOn;
   int			cursorHideCount;
   int			cursorOnTime;
   int			cursorOffTime;
   XtIntervalId		cursorTimer;

   CallbackListC	stateCalls;	// When state under cursor changes
   TextStateC		lastCursorState;

//
// Selection data
//
   int			clickCount;
   XtIntervalId		clickTimer;
   Widget		clickWidget;
   XButtonEvent		clickEvent;
   Boolean		timerOk;
   Boolean		mousePaste;

   TextPosC		selectBegPos;
   TextPosC		selectEndPos;
   Boolean		selectOn;
   Time			selectTime;
   Boolean		okToEndSelection;
   enum { SELECT_CHAR, SELECT_WORD, SELECT_LINE, SELECT_PAGE } selectType;
   TextPosC		baseSelectBegPos;
   TextPosC		baseSelectEndPos;
   Boolean		sendExcerpt;
   StringC		webCommand;	// Run when URL clicked

   static Atom		mimeRichAtom;
   static int		charClasses[256];

//
// Font structures
//
   StringC		defCharset;
   StringC		defPlainFontName;
   StringC		defFixedFontName;

   int			curCharset;
   PtrListC		charsetList;
   FontDataC		plainFont;
   FontDataC		fixedFont;

//
// Used for auto scrolling
//
   XMotionEvent	scrollEvent;		// Motion event that started scrolling
   XtIntervalId	scrollTimer;
   char		*vScrollAction;		// Auto scroll action
   char		*hScrollAction;		// Auto scroll action
   int		autoScrollInterval;

   TextPosC		nextPartPos;	// Used in GetNextPart
   Boolean		moreParts;

   CallbackListC	changeCalls;	// When text is modified by user

//
// Translations
//
   static XtActionsRec          actions[MIME_ACTION_COUNT];
   static XtTranslations	defaultTrans1;
   static XtTranslations	defaultTrans2;
   static XtTranslations	delRightTrans;
   static XtTranslations	delLeftTrans;
   static XtTranslations	emacsTrans;
   StringC			userTrans;
   Boolean			emacsMode;	// True if using emacsTrans
   Boolean			delLikeBs;	// True if delete goes left

//
// Used for when button clicked in link area
//
   CallbackListC	linkCalls;	// When link area is clicked

//
// Static methods
//
   static void		HandleAutoScroll (MimeRichTextP*, XtIntervalId*);
   static void		HandleExpose     (Widget, MimeRichTextP*,
				          XmDrawingAreaCallbackStruct*);
   static void		HandleFormResize (Widget, MimeRichTextP*, XEvent*,
   					  Boolean*);
   static void		HandleSWResize   (Widget, MimeRichTextP*, XEvent*,
					  Boolean*);
   static void		HandleFocusChange(Widget, MimeRichTextP*, XEvent*,
					  Boolean*);
   static void		HandleHScroll    (Widget, MimeRichTextP*,
				          XmScrollBarCallbackStruct*);
   static void		HandleVScroll    (Widget, MimeRichTextP*,
				          XmScrollBarCallbackStruct*);
   static void		CursorBlink      (MimeRichTextP*, XtIntervalId*);
   static void		ClickReset       (MimeRichTextP*, XtIntervalId*);
   static void		LoseSelection    (Widget, Atom*);
   static Boolean	SendSelection    (Widget, Atom*, Atom*, Atom*,
   					  XtPointer*, unsigned long*, int*);
   static void		ReceiveSelection (Widget, XtPointer, Atom*, Atom*,
   					  XtPointer, unsigned long*, int*);
   static void		ReceiveTargets   (Widget, XtPointer, Atom*, Atom*,
				          XtPointer, unsigned long*, int*);

//
// Action procs
//
   static void  ActMoveLeftChar   (Widget, XKeyEvent*, String*, Cardinal*);
   static void  ActMoveLeftWord   (Widget, XKeyEvent*, String*, Cardinal*);
   static void  ActMoveRightChar  (Widget, XKeyEvent*, String*, Cardinal*);
   static void  ActMoveRightWord  (Widget, XKeyEvent*, String*, Cardinal*);
   static void  ActMoveLineBeg    (Widget, XKeyEvent*, String*, Cardinal*);
   static void  ActMoveLineEnd    (Widget, XKeyEvent*, String*, Cardinal*);
   static void  ActMoveUpLine     (Widget, XKeyEvent*, String*, Cardinal*);
   static void  ActMoveUpPara     (Widget, XKeyEvent*, String*, Cardinal*);
   static void  ActMoveDownLine   (Widget, XKeyEvent*, String*, Cardinal*);
   static void  ActMoveDownPara   (Widget, XKeyEvent*, String*, Cardinal*);
   static void  ActMoveFileBeg    (Widget, XKeyEvent*, String*, Cardinal*);
   static void  ActMoveFileEnd    (Widget, XKeyEvent*, String*, Cardinal*);

   static void  ActDeleteLeftChar (Widget, XKeyEvent*, String*, Cardinal*);
   static void  ActDeleteLeftWord (Widget, XKeyEvent*, String*, Cardinal*);
   static void  ActDeleteRightChar(Widget, XKeyEvent*, String*, Cardinal*);
   static void  ActDeleteRightWord(Widget, XKeyEvent*, String*, Cardinal*);
   static void  ActDeleteLineBeg  (Widget, XKeyEvent*, String*, Cardinal*);
   static void  ActDeleteLineEnd  (Widget, XKeyEvent*, String*, Cardinal*);

#if 0
   static void  ActSingClick      (Widget, XButtonEvent*, String*, Cardinal*);
   static void  ActDoubClick      (Widget, XButtonEvent*, String*, Cardinal*);
   static void  ActTripClick      (Widget, XButtonEvent*, String*, Cardinal*);
   static void  ActQuadClick      (Widget, XButtonEvent*, String*, Cardinal*);
#endif

   static void  ActSelectMotion   (Widget, XMotionEvent*, String*, Cardinal*);
//   static void  HandleButton1Motion(Widget, MimeRichTextP*, XMotionEvent*,
//   				    Boolean*);

   static void  ActSelectBegin    (Widget, XButtonEvent*, String*, Cardinal*);
   static void  ActSelectEnd      (Widget, XButtonEvent*, String*, Cardinal*);
   static void  ActSelectExtend   (Widget, XEvent*,       String*, Cardinal*);
   static void  ActSelectLeftChar (Widget, XKeyEvent*,    String*, Cardinal*);
   static void  ActSelectLeftWord (Widget, XKeyEvent*,    String*, Cardinal*);
   static void  ActSelectRightChar(Widget, XKeyEvent*,    String*, Cardinal*);
   static void  ActSelectRightWord(Widget, XKeyEvent*,    String*, Cardinal*);
   static void  ActSelectLineBeg  (Widget, XKeyEvent*,    String*, Cardinal*);
   static void  ActSelectLineEnd  (Widget, XKeyEvent*,    String*, Cardinal*);
   static void  ActSelectUpLine   (Widget, XKeyEvent*,    String*, Cardinal*);
   static void  ActSelectUpPara   (Widget, XKeyEvent*,    String*, Cardinal*);
   static void  ActSelectDownLine (Widget, XKeyEvent*,    String*, Cardinal*);
   static void  ActSelectDownPara (Widget, XKeyEvent*,    String*, Cardinal*);
   static void  ActSelectFileBeg  (Widget, XKeyEvent*,    String*, Cardinal*);
   static void  ActSelectFileEnd  (Widget, XKeyEvent*,    String*, Cardinal*);

   static void  ActDeleteSelection(Widget, XKeyEvent*,    String*, Cardinal*);
   static void  ActCutSelection   (Widget, XKeyEvent*,    String*, Cardinal*);
   static void  ActCopySelection  (Widget, XKeyEvent*,    String*, Cardinal*);
   static void  ActPaste          (Widget, XEvent*,       String*, Cardinal*);
   static void  ActUndo           (Widget, XKeyEvent*,    String*, Cardinal*);
   static void  ActInsertSelf     (Widget, XKeyEvent*,    String*, Cardinal*);
   static void  ActRefresh        (Widget, XKeyEvent*,    String*, Cardinal*);
   static void  ActIgnore         (Widget, XKeyEvent*,    String*, Cardinal*);
   static void  ActPostMenu       (Widget, XButtonEvent*, String*, Cardinal*);

   static void  ActScrollUpLine   (Widget, XKeyEvent*, String*, Cardinal*);
   static void  ActScrollUpPage   (Widget, XKeyEvent*, String*, Cardinal*);
   static void  ActScrollDownLine (Widget, XKeyEvent*, String*, Cardinal*);
   static void  ActScrollDownPage (Widget, XKeyEvent*, String*, Cardinal*);
   static void  ActScrollTop      (Widget, XKeyEvent*, String*, Cardinal*);
   static void  ActScrollBottom   (Widget, XKeyEvent*, String*, Cardinal*);
   static void  ActLineToTop      (Widget, XKeyEvent*, String*, Cardinal*);
   static void  ActLineToCenter   (Widget, XKeyEvent*, String*, Cardinal*);
   static void  ActLineToBottom   (Widget, XKeyEvent*, String*, Cardinal*);

   static void	ActPlain          (Widget, XKeyEvent*, String*, Cardinal*);
   static void	ActBold           (Widget, XKeyEvent*, String*, Cardinal*);
   static void	ActItalic         (Widget, XKeyEvent*, String*, Cardinal*);
   static void	ActFixed          (Widget, XKeyEvent*, String*, Cardinal*);
   static void	ActSmaller        (Widget, XKeyEvent*, String*, Cardinal*);
   static void	ActBigger         (Widget, XKeyEvent*, String*, Cardinal*);
   static void	ActUnderline      (Widget, XKeyEvent*, String*, Cardinal*);
   static void	ActCenter         (Widget, XKeyEvent*, String*, Cardinal*);
   static void	ActFlushLeft      (Widget, XKeyEvent*, String*, Cardinal*);
   static void	ActFlushRight     (Widget, XKeyEvent*, String*, Cardinal*);
   static void	ActFlushBoth      (Widget, XKeyEvent*, String*, Cardinal*);
   static void	ActNoFill         (Widget, XKeyEvent*, String*, Cardinal*);
   static void	ActLeftMarginIn   (Widget, XKeyEvent*, String*, Cardinal*);
   static void	ActLeftMarginOut  (Widget, XKeyEvent*, String*, Cardinal*);
   static void	ActRightMarginIn  (Widget, XKeyEvent*, String*, Cardinal*);
   static void	ActRightMarginOut (Widget, XKeyEvent*, String*, Cardinal*);
   static void	ActExcerptMore    (Widget, XKeyEvent*, String*, Cardinal*);
   static void	ActExcerptLess    (Widget, XKeyEvent*, String*, Cardinal*);

   static void  ActFollowURL      (Widget, XButtonEvent*, String*, Cardinal*);
   static void  ActSearch         (Widget, XKeyEvent*,    String*, Cardinal*);

//
// Private methods
//
   Boolean		BeginningOfInputLine();
   Boolean		AddCharsetCommand(CharC, Boolean negate=False);
   Boolean		AddParamCommand(StringC&, StringC&);
   void			AddLine(TextLineC*, TextLineC*);
   void			ChangeFont(FontCmdT);
   void			ChangeCharset(CharC);
   void			ChangeColor(StringC);
   void			ChangeJust(JustCmdT);
   void			ChangeMargin(MarginCmdT);
   void			CheckPixmapSize();
   void			CompactLine(TextLineC*);
   void			CompactSelection();
   void			CopyText(StringC&, StringC&, TextTypeT, TextStateC*,
   				 int);
   FontDataC		*CurFont(const TextStateC&);
   XPoint		CursorLocation();
   void			DeleteCommand(const TextPosC&);
   void			DeleteLine(TextLineC*);
   void			DeleteText(const TextPosC&, int);
   void			DeleteRange(TextPosC&, TextPosC&);
   void			DeleteSelection();
   void			DrawCursor();
   void			DrawSelection();
   void			DrawSelectionRange(TextPosC*, TextPosC*);
   void			DrawLine(SoftLineC*, long *lastFont, Pixel *lastColor,
   				 Boolean clear=False);
   void			DrawLines(SoftLineC*, SoftLineC*);
   void			DrawScreen(RectC&);
   int			Distance(ScreenPosC&, ScreenPosC&);
   Boolean		FindPosPrevChar(TextPosC&, TextPosC*);
   Boolean		FindPosPrevWord(TextPosC&, TextPosC*);
   Boolean		FindPosPrevCmd (TextPosC&, TextPosC*);
   Boolean		FindPosNextChar(TextPosC&, TextPosC*);
   Boolean		FindPosNextWord(TextPosC&, TextPosC*);
   Boolean		FindPosNextCmd (TextPosC&, TextPosC*);
   Boolean		FindPosBegWord (TextPosC&, TextPosC*);
   Boolean		FindPosEndWord (TextPosC&, TextPosC*);
   Boolean		FindPosBegClass(TextPosC&, TextPosC*);
   Boolean		FindPosEndClass(TextPosC&, TextPosC*);
   int			FitText(char*, XFontStruct*, int);
   void			FlushTextBuf();
   void			FlushTextURL(CharC);
   void			FixPosAfterBreak(TextPosC*, const TextPosC&,
					      TextLineC*);
   void			FixPosAfterDelCmd(TextPosC*, const TextPosC&);
   void			FixPosAfterDelLine(TextPosC*, TextLineC*);
   void			FixPosAfterDelText(TextPosC*, const TextPosC&, int);
   void			FixPosAfterInsert(TextPosC*, const TextPosC&);
   void			FixPosAfterMergeCmds(TextPosC*, const TextPosC&,
					     const TextPosC&);
   void			FixPosAfterMergeLines(TextPosC*, TextLineC*,TextLineC*);
   void			FixPosAfterSplit(TextPosC*, const TextPosC&);
   void			FormatScreen();
   void			FormatLine(TextLineC*);
   SoftLineC		*FormatText(TextLineC*, SoftLineC*, int, TextStateC&,
				    int*, int*, int*, int*);
   SoftLineC		*FormatGraphic(TextLineC*, SoftLineC*, int, TextStateC&,
				       int*, int*, int*, int*);
   void			GetLineSize(SoftLineC*);
   void			GetLineText(TextPosC*, TextPosC*, StringC&, TextTypeT,
   				    int);
   void			GetLineText(TextLineC*, StringC&, TextTypeT, int);
   void			GetRangeData(TextPosC*, TextPosC*, StringC&, TextTypeT,
   				     Boolean closeState=True, int lineSize=0);
   void			GetSelectionData(StringC&, TextTypeT type=TT_PLAIN);
   void			GetStateCommands(TextStateC&, TextStateC&, StringC&,
   					 TextTypeT);
   void			GetStateCommands(TextPosC&, StringC&, TextTypeT);
   void			HideCursor();
   void			HideLineGraphics(SoftLineC*);
   void			InsertCommand(RichCmdTypeT, Boolean, const TextPosC&,
				      TextPosC*);
   void			InsertLineBreak(TextPosC, TextPosC*);
   void			InsertSavedLines(TextPosC);
   void			Justify(SoftLineC*);
   void			JustifyLines();
   void			LineChanged(TextLineC*, Boolean forcePlace=False);
   void			LinesChanged(TextLineC*, TextLineC*,
				     Boolean forcePlace=False);
   Boolean		LineFullyVisible(SoftLineC*);
   Boolean		LineVisible(SoftLineC*);
   int			MaxLineWidth();
   void			MergeCommands(TextPosC&, TextPosC&);
   void			MergeLines(TextLineC*, TextLineC*);
   void			MoveCommand(TextPosC&, TextLineC*,
				    int dstPos=NULL_CMD_POS);
   void			MoveLine(TextLineC*, TextLineC*);
   void			MoveLines(int, int, int);
   void			MoveText(const TextPosC&, int, TextLineC*,
   				 int dstPos=NULL_CMD_POS);
   void			NewInputLine();
   void			PasteText(Widget, XButtonEvent*, TextTypeT, CharC);
   RichDrawDataC	*PickData(SoftLineC*, int);
   RichGraphicC		*PickGraphic(int, int);
   SoftLineC		*PickLine(int);
   void			PlaceLines(TextLineC *startLine=NULL);
   void			RemoveLine(TextLineC*);
   void			RemoveLines(TextLineC*, TextLineC*);
   void			Reset();
   void			ResetLineList();
   Boolean		ScrollToCursor();
   Boolean		ScrollToPosition(ScreenPosC&);
   void			Search();
   void			SelectionChanged();
   Boolean		SetCharClassRange(int, int, int);
   void			SetLineIndex(TextLineC*, int);
   void			SetLinePosition(TextLineC*, int);
   void			SetNext(SoftLineC*, SoftLineC*);
   void			SetVisibleSize(Dimension, Dimension, Widget ref=NULL);
   void			ShowCursor();
   void			SplitCommand(const TextPosC&, TextPosC*);
   void			UpdateMotionSelection(int, int, Boolean);
   void			UpdateSelection(TextPosC&, TextPosC&);
   void			UpdateTranslations();

public:

   MimeRichTextP() {}
};

#endif // _MimeRichTextP_h_
