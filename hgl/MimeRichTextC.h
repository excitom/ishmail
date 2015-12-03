/*
 * $Id: MimeRichTextC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
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

#ifndef _MimeRichTextC_h_
#define _MimeRichTextC_h_

#include "RectC.h"
#include "StringC.h"
#include "CallbackC.h"

#include <X11/Intrinsic.h>

//
// Font commands are stored in the fstack member of the text state
//
   enum FontCmdT {
      FC_PLAIN = 'a',
      FC_BOLD,
      FC_ITALIC,
      FC_FIXED,
      FC_SMALLER,
      FC_BIGGER,
      FC_UNDERLINE,
      FC_CHARSET_1,
      FC_CHARSET_2,
      FC_CHARSET_3,
      FC_CHARSET_4,
      FC_CHARSET_5,
      FC_CHARSET_6,
      FC_CHARSET_7,
      FC_CHARSET_8,
      FC_CHARSET_9,
      FC_URL
   };

#define MAX_CHARSET_COUNT	10

//
// Justify commands are stored in the jstack member of the text state
//
   enum JustCmdT {
      JC_LEFT = 'a',
      JC_RIGHT,
      JC_BOTH,
      JC_CENTER,
      JC_NOFILL,
      JC_EXCERPT_MORE,
      JC_EXCERPT_LESS
   };

//
// Margin commands
//
   enum MarginCmdT {
      MC_LEFT_IN = 'a',
      MC_LEFT_OUT,
      MC_RIGHT_IN,
      MC_RIGHT_OUT
   };

//
// Text types we understand
//
   enum TextTypeT {
      TT_PLAIN = 0,
      TT_RICH,
      TT_ENRICHED,
      TT_HTML
   };

//
// command types we understand
//
   enum RichCmdTypeT {
      RC_GRAPHIC = 0,
      RC_TEXT
   };

/*----------------------------------------------------------------------
 * This class represents an in-line graphic
 */

class MimeRichTextP;

class RichGraphicC {

   virtual void		HideIt() = 0;
   virtual void		ShowIt() = 0;

public:

   RectC		bounds;
   Boolean		okToDelete;
   Boolean		shown;
   MimeRichTextP	*owner;

   			RichGraphicC();
   virtual		~RichGraphicC();
   void			Hide();
   void			Show();

   virtual void		Refresh()     = 0;
   virtual int		PrefWidth()   = 0;
   virtual int		PrefHeight()  = 0;
   virtual Boolean	GetText(StringC&) = 0;

   virtual void		SingleClick(XButtonEvent*) = 0;
   virtual void		DoubleClick(XButtonEvent*) = 0;
   virtual void		PostMenu(XButtonEvent*)    = 0;
};

/*----------------------------------------------------------------------
 * Rich/Enriched text drawing class
 */

class CharC;
class PtrListC;

class MimeRichTextC {

   friend class MimeRichTextP;

   MimeRichTextP	*priv;

public:

   MimeRichTextC(Widget, const char*, ArgList argv=NULL, Cardinal argc=0);
   ~MimeRichTextC();

//
// Query
//
   Pixel		Background() const;
   Boolean		CanBePlain() const;
   const StringC&	Charset(int) const;
   const StringC&	Charset() const;
   int			CharWidth() const;
   int			CharHeight() const;
   int			ColumnCount() const;
   StringC		CursorStateString();
   const StringC&	DefCharset() const;
   const StringC&	DefFixedFontName() const;
   const StringC&	DefPlainFontName() const;
   Drawable		DrawTo() const;
   const StringC&	FixedFontName() const;
   Pixel		Foreground() const;
   PtrListC&		GraphicList() const;
   Widget		MainWidget() const;
   Boolean		Has8Bit() const;
   Boolean		HasGraphics() const;
   Boolean		HasMarkup() const;
   Boolean		IsEditable() const;
   Boolean		IsEmpty() const;
   Boolean		IsSingleGraphic() const;
   int			LineHeight() const;
   int			MarginHeight() const;
   int			MarginWidth() const;
   const StringC&	PlainFontName() const;
   int			RowCount() const;
   Widget		TextArea() const;
   Boolean		UndeleteOk() const;

//
// Methods
//
   int			AddCharset(CharC);
   Boolean		AddCommand(CharC, Boolean negate=False);
   void			AddGraphic(RichGraphicC*);
   void			AddString(CharC);
   void			AddStringEnriched(CharC);
   void			AddStringHTML(CharC);
   void			AddStringPlain(CharC);
   void			AddStringRichtext(CharC);
   void			CheckFroms(Boolean val=True);
   void			Clear();
   void			Defer(Boolean);
   void			ForceFixed(Boolean);
   RichCmdTypeT		GetNextPart(StringC&, TextTypeT, RichGraphicC**,
   				    int lineSize=0);
   void			GetString(StringC&, TextTypeT type=TT_PLAIN,
   				  int lineSize=0);
   void			GraphicChanged(RichGraphicC*, Boolean sizeChanged=True);
   void			InsertGraphic(RichGraphicC*);
   void			InsertString(CharC);
   void			MoveCursor(RichGraphicC*, int);
   void			MoveCursor(Widget, int, int);
   void			Refresh();
   void			RemoveGraphic(RichGraphicC*);
   void			ResetPartIndex();
   void			ResizeWidth(Boolean val=True);
   void			SetBackground(Pixel);
   void			SetCharset(StringC);
   void			SetCharClasses(const char*);
   void			SetDeleteLikeBackspace(Boolean);
   void			SetEditable(Boolean);
   void			SetEmacsMode(Boolean);
   void			SetExcerptString(CharC);
   void			SetFixedFont(const char*);
   void			SetForeground(Pixel);
   void			SetPlainFont(const char*);
   void			SetSensitive(Boolean);
   void			SetSize(int rows, int cols, Widget reference=NULL);
   void			SetString(CharC);
   void			SetTextType(TextTypeT);
   void			SetWebCommand(const char*);

//
// Manage state change callbacks
//
   void		AddStateChangeCallback   (CallbackFn*, void*);
   void		RemoveStateChangeCallback(CallbackFn*, void*);
   void		AddTextChangeCallback    (CallbackFn*, void*);
   void		RemoveTextChangeCallback (CallbackFn*, void*);
   void		AddLinkAccessCallback    (CallbackFn*, void*);
   void		RemoveLinkAccessCallback (CallbackFn*, void*);

//
// Editing methods
//
   void		MoveLeftChar();
   void		MoveLeftWord();
   void		MoveRightChar();
   void		MoveRightWord();
   void		MoveLineBeg();
   void		MoveLineEnd();
   void		MoveUpLine();
   void		MoveUpPara();
   void		MoveDownLine();
   void		MoveDownPara();
   void		MoveFileBeg();
   void		MoveFileEnd();

   void		DeleteLeftChar();
   void		DeleteLeftWord();
   void		DeleteRightChar();
   void		DeleteRightWord();
   void		DeleteLineBeg();
   void		DeleteLineEnd();

   void		SelectLeftChar();
   void		SelectLeftWord();
   void		SelectRightChar();
   void		SelectRightWord();
   void		SelectLineBeg();
   void		SelectLineEnd();
   void		SelectUpLine();
   void		SelectUpPara();
   void		SelectDownLine();
   void		SelectDownPara();
   void		SelectFileBeg();
   void		SelectFileEnd();

   void		Undelete();

   void		ScrollUpLine();
   void		ScrollUpPage();
   void		ScrollDownLine();
   void		ScrollDownPage();
   void		ScrollTop();
   void		ScrollBottom();
   void		LineToTop();
   void		LineToCenter();
   void		LineToBottom();

   void		ChangeFont(FontCmdT);
   void		ChangeCharset(CharC);
   void		ChangeColor(StringC);
   void		ChangeJust(JustCmdT);
   void		ChangeMargin(MarginCmdT);

//
// comparison
//
   int	compare(const MimeRichTextC& t) const;
   inline int	operator<(const MimeRichTextC& t) const
	{ return (compare(t) < 0); }
   inline int	operator>(const MimeRichTextC& t) const
	{ return (compare(t) > 0); }
   inline int	operator!=(const MimeRichTextC& t) const
	{ return (compare(t) != 0); }
   inline int	operator==(const MimeRichTextC& t) const
	{ return (compare(t) == 0); }
};

inline ostream& operator<<(ostream& strm, const MimeRichTextC&) {return(strm);}

#define	ISHMAIL_ENRICHED_ATOM_NAME	"IshmailEnrichedText"
#define	MIME_ENRICHED_ATOM_NAME		"MimeEnrichedText"

#endif // _MimeRichTextC_h_
