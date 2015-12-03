/*
 * $Id: VItemC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
 *
 * Copyright (c) 1992 HaL Computer Systems, Inc.  All rights reserved.
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

#ifndef _VItemC_h_
#define _VItemC_h_

#include "StringListC.h"
#include "CallbackC.h"
#include "PixmapC.h"	// For XbmT and XpmT

#include <X11/Intrinsic.h>

class VBoxC;
class PtrListC;
class RectC;

//
// This is the compare function type
//
typedef int     (*CompareFn)(const void*, const void*);

//
// View item class
//
class VItemC {

public:

   enum ImageSourceT {
      IMAGE_FILE,
      XBM_DATA,
      XPM_DATA
   };

//
// Return components
//
      PTR_QUERY(StringListC&,	FieldList,	fieldList)
   MEMBER_QUERY(ImageSourceT,	ImageSource,	imageSrc)
      PTR_QUERY(StringC&,	Name,		name)
      PTR_QUERY(StringC&,	Label,		label)
      PTR_QUERY(char*,		LabelTag,	labelTag)
      PTR_QUERY(StringC&,	LgImageFile,	lgImageFile)
      PTR_QUERY(XbmT&,		LgXbmData,	lgXbmData)
   MEMBER_QUERY(XpmT,		LgXpmData,	lgXpmData)
      PTR_QUERY(StringC&,	SmImageFile,	smImageFile)
      PTR_QUERY(XbmT&,		SmXbmData,	smXbmData)
   MEMBER_QUERY(XpmT,		SmXpmData,	smXpmData)
      PTR_QUERY(void*,		UserData,	userData)
   MEMBER_QUERY(Boolean,	ValidDropSite,	validDropSite)
      PTR_QUERY(void*,		ViewData,	viewData)

//
// Set components
//
   void		Field(int, const char*);
   void		FieldList(const StringListC&);
   inline void	Name(const char *s)	{ name = s; }
   void		Label(const char *s);
   void		SetPixmaps(const char*, const char*);
   void		SetPixmaps(const XbmT*, const XbmT*);
   void		SetPixmaps(const XpmT,  const XpmT);
   void		SetLabelTag(const char*);
   void		SetFieldTag(int, const char*);

//
// Query components
//
   char		*FieldTag(int);
   StringC	*Field(int);

//
// Constructors
//
   VItemC();
   VItemC(const char *n);
   VItemC(const char *n, StringC l);
   VItemC(const char *n, const char *l, const char *s);
   VItemC(const char *n, const XbmT*, const XbmT*);
   VItemC(const char *n, const XpmT,  const XpmT);

   virtual	~VItemC();
   virtual void	printOn(ostream&) const {}

   virtual int	operator==(const VItemC&) const;
   inline int	operator!=(const VItemC& i) const { return !(*this==i); }

   int	compare(const VItemC&) const;
   inline int	operator<(const VItemC& i) const { return (compare(i) < 0); }
   inline int	operator>(const VItemC& i) const { return (compare(i) > 0); }

//
// Add callbacks
//
   inline void	AddSelectCallback(CallbackFn *fn, void *data) {
      AddCallback(&selectCalls, fn, data);
   }
   inline void	AddDeselectCallback(CallbackFn *fn, void *data) {
      AddCallback(&deselectCalls, fn, data);
   }
   inline void	AddOpenCallback(CallbackFn *fn, void *data) {
      AddCallback(&openCalls, fn, data);
   }
   inline void	AddFieldChangeCallback(CallbackFn *fn, void *data) {
      AddCallback(&fieldChangeCalls, fn, data);
   }
   inline void	AddLabelChangeCallback(CallbackFn *fn, void *data) {
      AddCallback(&labelChangeCalls, fn, data);
   }
   inline void	AddPixmapChangeCallback(CallbackFn *fn, void *data) {
      AddCallback(&pixmapChangeCalls, fn, data);
   }

//
// Call callbacks
//
   inline void	CallSelectCallbacks()	{ CallCallbacks(&selectCalls,	this); }
   inline void	CallDeselectCallbacks()	{ CallCallbacks(&deselectCalls,	this); }
   inline void	CallOpenCallbacks()	{ CallCallbacks(&openCalls,	this); }
   inline void	CallFieldChangeCallbacks() {
      CallCallbacks(&fieldChangeCalls, this);
   }
   inline void	CallLabelChangeCallbacks() {
      CallCallbacks(&labelChangeCalls, this);
   }
   inline void	CallPixmapChangeCallbacks() {
      CallCallbacks(&pixmapChangeCalls, this);
   }

//
// Remove callbacks
//
   inline void	RemoveSelectCallback(CallbackFn *fn, void *data) {
      RemoveCallback(&selectCalls, fn, data);
   }
   inline void	RemoveDeselectCallback(CallbackFn *fn, void *data) {
      RemoveCallback(&deselectCalls, fn, data);
   }
   inline void	RemoveOpenCallback(CallbackFn *fn, void *data) {
      RemoveCallback(&openCalls, fn, data);
   }
   inline void	RemoveFieldChangeCallback(CallbackFn *fn, void *data) {
      RemoveCallback(&fieldChangeCalls, fn, data);
   }
   inline void	RemoveLabelChangeCallback(CallbackFn *fn, void *data) {
      RemoveCallback(&labelChangeCalls, fn, data);
   }
   inline void	RemovePixmapChangeCallback(CallbackFn *fn, void *data) {
      RemoveCallback(&pixmapChangeCalls, fn, data);
   }

//
// Specify the function used to compare two VItemCs
//
   inline void	SetCompareFunction(CompareFn comp) { compFunc = comp; }

//
// Let the user or view store a pointer with a view item.
//
   inline void	SetUserData(void* ptr) { userData = ptr; }
   inline void	SetViewData(void* ptr) { viewData = ptr; }

//
// Specify whether drops are ok here
//
   inline void	ValidDropSite(Boolean val) { validDropSite = val; }

//
// Manage popup menu
//
   inline Boolean	HasMenu() { return (menu != NULL); }
   void			PostMenu(XButtonEvent*);
   void			SetMenu(Widget, Widget parent=NULL);
   inline void	AddMenuCallback(CallbackFn *fn, void *data) {
      AddCallback(&menuCalls, fn, data);
   }
   inline void	CallMenuCallbacks()	{ CallCallbacks(&menuCalls, this); }
   inline void	RemoveMenuCallback(CallbackFn *fn, void *data) {
      RemoveCallback(&menuCalls, fn, data);
   }

//
// Hierarchy info
//
   VItemC	*parent;
   VItemC	*child;
   VItemC	*prev;
   VItemC	*next;
   void		AddChild(VItemC*, VItemC *after=NULL);
   int		ChildCount();

//
// Used for self-drawing items
//
   virtual void		GetPrefSize(int *wd, int *ht);
   virtual void		Draw(RectC& area, GC, Drawable dst, Boolean invert);
   virtual Boolean	SingleClick(int x, int y, int state);
   virtual Boolean	DoubleClick(int x, int y, int state);

protected:

   int		 type;
   StringC	 name;		// Used for sorting
   StringC	 label;		// Used for display
   char		*labelTag;	// Used when view box uses font list
   StringListC	 fieldList;
   PtrListC	*fieldTagList;	// Used when view box uses font list
   void*	 userData;
   void*	 viewData;
   Widget	 menu;		// Popup menu

   StringC	lgImageFile;	// Name of large image file
   StringC	smImageFile;	// Name of small image file
   XbmT		lgXbmData;	// Data for large x bitmap
   XbmT		smXbmData;	// Data for small x bitmap
   XpmT		lgXpmData;	// Data for large xpm pixmap
   XpmT		smXpmData;	// Data for small xpm pixmap
   ImageSourceT imageSrc;	// Where does data come from
   Boolean	validDropSite;	// True if drops allowed here

//
// Callbacks
//
   CallbackC	*selectCalls;
   CallbackC	*deselectCalls;
   CallbackC	*openCalls;
   CallbackC	*fieldChangeCalls;
   CallbackC	*labelChangeCalls;
   CallbackC	*pixmapChangeCalls;
   CallbackC	*menuCalls;

//
// Function used to compare two VItemCs
//
   CompareFn	compFunc;
   static int	DefaultComp(const void *a, const void *b) {
      VItemC	*via = *(VItemC **)a;
      VItemC	*vib = *(VItemC **)b;
      return ( via->name.compare(vib->name) );
   }

private:

   void		Init();
};

//
// Method to allow printing of VItemC
//

inline ostream&
operator<<(ostream& strm, const VItemC& i)
{
   i.printOn(strm);
   return(strm);
}

//
// This type determines how an item is drawn
//
typedef enum {

   AS_IS,		// Draw inverted if item is selected
   NORMAL,		// Always draw not inverted
   INVERT		// Always draw inverted

} VItemDrawModeT;

#endif // _VItemC_h_
