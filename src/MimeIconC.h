/*
 *  $Id: MimeIconC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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
#ifndef _MimeIconC_h_
#define _MimeIconC_h_

#include <hgl/StringC.h>
#include <hgl/MimeRichTextC.h>	// For RichGraphicC
#include <hgl/CallbackListC.h>

#include <X11/Intrinsic.h>

class PixmapC;
class PtrListC;

class MimeIconC : public RichGraphicC {

protected:

//
// Resources
//
   Pixel		topShadColor;
   Pixel		botShadColor;
   int			labelSpacing;	// Between lines
   int			labelOffset;	// Between pixmap and label
   int			marginWd;
   int			marginHt;
   int			shadowThick;
   u_char		shadowType;
   StringC		pixmapFile;
   Boolean		highlighted;

//
// Drawing vars
//
   GC			gc;
   XFontStruct		*font;
   PixmapC		*pm;
   int			labelWd;
   int			labelHt;
   MimeRichTextC	*rt;
   int			animIndex;
   PtrListC		*animList;
   PixmapC		*animPm;

   void			GetPixmap();
   void			GetLabelSize();

//
// Animation variables
//
   static XtIntervalId	animTimer;
   static int		animInterval;
   static PtrListC	*activeAnims;

   static void		UpdateAnimation(XtPointer, XtIntervalId*);

//
// Callbacks
//
   CallbackListC	singleClickCalls;
   CallbackListC	doubleClickCalls;
   CallbackListC	menuCalls;

//
// Private methods
//
   virtual void		GetPixmapFile(StringC&);

public:

   Pixel		regBgColor;
   Pixel		regFgColor;
   Pixel		invBgColor;
   Pixel		invFgColor;
   StringC		labelStr;
   XButtonPressedEvent	*lastEvent;

   MimeIconC(char *name, MimeRichTextC*);
   ~MimeIconC();

   void			Animate(PtrListC*);
   void			AnimationOff();
   virtual Boolean	GetText(StringC&);
   void			DoubleClick(XButtonEvent*);
   void			HideIt() {}
   void			Highlight();
   Boolean		Highlighted() { return highlighted; }
   void			PostMenu(XButtonEvent*);
   int			PrefHeight();
   int			PrefWidth();
   void			Refresh();
   void			SetLabel(CharC);
   void			SingleClick(XButtonEvent*);
   void			ShowIt() {}
   void			Unhighlight();

//
// User callbacks
//
   inline void	AddSingleClickCallback(CallbackFn *fn, void *data)
      { AddCallback(singleClickCalls, fn, data); }
   inline void	AddDoubleClickCallback(CallbackFn *fn, void *data)
      { AddCallback(doubleClickCalls, fn, data); }
   inline void	AddMenuCallback(CallbackFn *fn, void *data)
      { AddCallback(menuCalls, fn, data); }

   inline void	CallSingleClickCallbacks()
      { CallCallbacks(singleClickCalls, this); }
   inline void	CallDoubleClickCallbacks()
      { CallCallbacks(doubleClickCalls, this); }
   inline void	CallMenuCallbacks()
      { CallCallbacks(menuCalls, this); }

   inline void	RemoveSingleClickCallback(CallbackFn *fn, void *data)
      { RemoveCallback(singleClickCalls, fn, data); }
   inline void	RemoveDoubleClickCallback(CallbackFn *fn, void *data)
      { RemoveCallback(doubleClickCalls, fn, data); }
   inline void	RemoveMenuCallback(CallbackFn *fn, void *data)
      { RemoveCallback(menuCalls, fn, data); }
};

#endif // _MimeIconC_h_
