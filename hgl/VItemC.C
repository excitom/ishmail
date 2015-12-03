/*
 * $Id: VItemC.C,v 1.5 2000/05/07 12:26:11 fnevgeny Exp $
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

#include <config.h>

#include "HalAppC.h"

#include "VItemC.h"
#include "VBoxC.h"
#include "PtrListC.h"

#include "file.xbm"
#include "sm_file.xbm"

#include <Xm/RowColumn.h>
#ifndef HAVE_XMADDTOPOSTFROMLIST_DECL
extern "C" void XmAddToPostFromList(Widget menu, Widget post_from_widget);
#endif

static XbmT	file_xbm    = { file_width,    file_height,    file_bits };
static XbmT	sm_file_xbm = { sm_file_width, sm_file_height, sm_file_bits };

/*-----------------------------------------------------------------------
 *  VItemC constructor with no arguments
 */

VItemC::VItemC() : name(""), label("")
{
   Init();
   SetPixmaps(&file_xbm, &sm_file_xbm);
}

/*-----------------------------------------------------------------------
 *  VItemC constructor with name argument
 */

VItemC::VItemC(const char *n) : name(n), label(n)
{
   Init();
   SetPixmaps(&file_xbm, &sm_file_xbm);
}

/*-----------------------------------------------------------------------
 *  VItemC constructor with name and large file argument
 */

VItemC::VItemC(const char *n, StringC l) : name(n), label(n)
{
   Init();
   SetPixmaps(l, "sm_" + l);
}

/*-----------------------------------------------------------------------
 *  VItemC constructor with name and large and small file arguments
 */

VItemC::VItemC(const char *n, const char *l, const char *s) : name(n), label(n)
{
   Init();
   SetPixmaps(l, s);
}

/*-----------------------------------------------------------------------
 *  VItemC constructor with name and xbm data
 */

VItemC::VItemC(const char *n, const XbmT *l, const XbmT *s) : name(n), label(n)
{
   Init();
   SetPixmaps(l, s);
}

/*-----------------------------------------------------------------------
 *  VItemC constructor with name and xpm data
 */

VItemC::VItemC(const char *n, const XpmT l, const XpmT s) : name(n), label(n)
{
   Init();
   SetPixmaps(l, s);
}

/*-----------------------------------------------------------------------
 *  Initialization method
 */

void
VItemC::Init()
{
   type			= 0;
   userData		= NULL;
   viewData		= NULL;
   compFunc		= DefaultComp;
   selectCalls		= NULL;
   deselectCalls	= NULL;
   openCalls		= NULL;
   fieldChangeCalls	= NULL;
   labelChangeCalls	= NULL;
   pixmapChangeCalls	= NULL;
   menuCalls		= NULL;
   validDropSite	= True;
   menu			= NULL;
   lgImageFile		= "";
   smImageFile		= "";
   lgXbmData		= file_xbm;
   smXbmData		= sm_file_xbm;
   lgXpmData		= NULL;
   smXpmData		= NULL;
   imageSrc		= XBM_DATA;
   labelTag		= NULL;
   fieldTagList		= NULL;

   fieldList.AllowDuplicates(TRUE);

   parent = NULL;
   child  = NULL;
   prev   = NULL;
   next   = NULL;
}

/*-----------------------------------------------------------------------
 *  VItemC destructor
 */

VItemC::~VItemC()
{
   if ( child ) child->parent = NULL;
   if ( prev  ) prev->next    = next;
   if ( next  ) next->prev    = prev;

#if 0
   if ( menu ) {
      Widget parent = XtParent(menu);
      if ( parent ) XmRemoveFromPostFromList(menu, parent);
   }
#endif

//
// Delete callback structures
//
   DeleteCallbacks(&selectCalls);
   DeleteCallbacks(&deselectCalls);
   DeleteCallbacks(&openCalls);
   DeleteCallbacks(&fieldChangeCalls);
   DeleteCallbacks(&labelChangeCalls);
   DeleteCallbacks(&pixmapChangeCalls);
   DeleteCallbacks(&menuCalls);

   delete labelTag;
   delete fieldTagList;

} // End VItemC Destructor

/*-----------------------------------------------------------------------
 *  == operator for VItemC
 */

int
VItemC::operator==(const VItemC& i) const
{
   return (name == i.name);

} // End VItemC operator==

/*-----------------------------------------------------------------------
 *  Method to change image file names
 */

void
VItemC::SetPixmaps(const char *l, const char *s)
{
   if ( imageSrc != IMAGE_FILE || lgImageFile != l || smImageFile != s ) {

      imageSrc = IMAGE_FILE;
      lgImageFile = l;
      smImageFile = s;

      CallPixmapChangeCallbacks();
   }
}

/*-----------------------------------------------------------------------
 *  Method to change xbm image data
 */

void
VItemC::SetPixmaps(const XbmT *l, const XbmT *s)
{
   Boolean	change = False;
   if ( imageSrc != XBM_DATA ) {
      imageSrc = XBM_DATA;
      change = True;
   }

   if ( l ) {
      if ( lgXbmData.bits != l->bits ) {
	 lgXbmData = *l;
	 change = True;
      }
   } else {
      if ( lgXbmData.bits != file_xbm.bits ) {
	 lgXbmData = file_xbm;
	 change = True;
      }
   }

   if ( s ) {
      if ( smXbmData.bits != s->bits ) {
	 smXbmData = *s;
	 change = True;
      }
   } else {
      if ( smXbmData.bits != sm_file_xbm.bits ) {
	 smXbmData = sm_file_xbm;
	 change = True;
      }
   }

   if ( change )
      CallPixmapChangeCallbacks();
}

/*-----------------------------------------------------------------------
 *  Method to change xbm image data
 */

void
VItemC::SetPixmaps(const XpmT l, const XpmT s)
{
   if ( imageSrc != XPM_DATA || lgXpmData != (XpmT)l || smXpmData != (XpmT)s ) {

      imageSrc = XPM_DATA;
      lgXpmData = (XpmT)l;
      smXpmData = (XpmT)s;

      CallPixmapChangeCallbacks();
   }
}

/*-----------------------------------------------------------------------
 *  Method to change a single field
 */

void
VItemC::Field(int i, const char *s)
{
   if ( i < 0 || i > fieldList.size() ) return;

   StringC	*string = fieldList[i];
   if ( *string != s ) {
      *string = s;
      CallFieldChangeCallbacks();
   }
}

/*-----------------------------------------------------------------------
 *  Method to change all fields
 */

void
VItemC::FieldList(const StringListC& list)
{
   fieldList = list;
   CallFieldChangeCallbacks();
}

/*-----------------------------------------------------------------------
 *  Method to change label string
 */

void
VItemC::Label(const char *s)
{
   if ( label != s ) {
      label = s;
      CallLabelChangeCallbacks();
   }
}

/*-----------------------------------------------------------------------
 *  Method to compare two view items
 */

int
VItemC::compare(const VItemC& i) const
{
   if ( !compFunc ) return 0;

   VItemC	*via = (VItemC *)this;
   VItemC	*vib = (VItemC *)&i;
   return (*compFunc)(&via, &vib);
}

/*-----------------------------------------------------------------------
 *  Method to set popup menu
 */

void
VItemC::SetMenu(Widget pu, Widget parent)
{
   menu = pu;
   if ( !pu ) return;

   if ( !parent ) parent = XtParent(pu);
   if (  parent ) XmAddToPostFromList(pu, parent);
}

/*-----------------------------------------------------------------------
 *  Method to display popup menu
 */

void
VItemC::PostMenu(XButtonEvent *ev)
{
   CallMenuCallbacks();
   XmMenuPosition(menu, ev);
   XtManageChild(menu);
}

/*-----------------------------------------------------------------------
 *  Method to set the font tag to be used by the label
 */

void
VItemC::SetLabelTag(const char *tag)
{
   if ( labelTag && tag && strcmp(labelTag, tag) == 0 ) return;

   if ( labelTag ) {
      delete labelTag;
      labelTag = NULL;
   }

   if ( tag ) {
      labelTag = new char[strlen(tag)+1];
      strcpy(labelTag, tag);
   }

   CallLabelChangeCallbacks();

} // End SetLabelTag

/*-----------------------------------------------------------------------
 *  Method to set the font tag to be used by a specific field
 */

void
VItemC::SetFieldTag(int num, const char *newTag)
{
//
// Create tag list if necessary
//
   if ( !fieldTagList ) {
      fieldTagList = new PtrListC;
      fieldTagList->AllowDuplicates(TRUE);
      fieldTagList->SetSorted(FALSE);
   }

//
// Add entries up to this point
//
   for (int i=fieldTagList->size(); i<=num; i++)
      fieldTagList->add(NULL);

//
// Point to this entry
//
   char	*oldTag = (char*)*(*fieldTagList)[num];

//
// See if they're already the same
//
   if ( oldTag && newTag && strcmp(oldTag, newTag) == 0 ) return;

//
// Delete old entry
//
   if ( oldTag ) {
      delete oldTag;
      oldTag = NULL;
   }

//
// Create new entry
//
   if ( newTag ) {
      oldTag = new char[strlen(newTag)+1];
      strcpy(oldTag, newTag);
   }

//
// Update list
//
   fieldTagList->remove(num);
   PtrT	tmp = (PtrT)oldTag;
   fieldTagList->insert(tmp, num);

   CallFieldChangeCallbacks();

} // End SetFieldTag

/*-----------------------------------------------------------------------
 *  Method to query the font tag to be used by a specific field
 */

char*
VItemC::FieldTag(int num)
{
   if ( !fieldTagList ) return NULL;

   if ( num >= fieldTagList->size() ) return NULL;

   return (char*)*(*fieldTagList)[num];

} // End FieldTag

/*-----------------------------------------------------------------------
 *  Method to query a specific field
 */

StringC*
VItemC::Field(int num)
{
   if ( num >= fieldList.size() ) return NULL;
   return fieldList[num];
}

/*-----------------------------------------------------------------------
 *  Default method to return preferred size
 */

void
VItemC::GetPrefSize(int *wd, int *ht)
{
   *wd = 100;
   *ht =  50;
}

/*-----------------------------------------------------------------------
 *  Default method to draw this item
 */

void
VItemC::Draw(RectC& area, GC gc, Drawable dst, Boolean invert)
{
   XSetForeground(halApp->display, gc, XBlackPixel(halApp->display,0));
   XSetLineAttributes(halApp->display, gc, 0, LineSolid, CapButt, JoinBevel);
   if ( invert )
      XFillArc(halApp->display, dst, gc, area.xmin, area.ymin,
					 area.wd-1, area.ht-1, 0, 360*64);
   else
      XDrawArc(halApp->display, dst, gc, area.xmin, area.ymin,
				         area.wd-1, area.ht-1, 0, 360*64);
}

/*-----------------------------------------------------------------------
 *  Default method to tell view we don't process our own single clicks
 */

Boolean
VItemC::SingleClick(int, int, int)
{
   return False;
}

/*-----------------------------------------------------------------------
 *  Default method to tell view we don't process our own double clicks
 */

Boolean
VItemC::DoubleClick(int, int, int)
{
   return False;
}

/*-----------------------------------------------------------------------
 *  Method to add a new child after the specified child.  If the "after"
 *     child is NULL, add the new child at the end.
 */

void
VItemC::AddChild(VItemC *item, VItemC *after)
{
   item->parent = this;

   if ( child ) {
      VItemC	*last = child;
      if ( after ) {
	 item->next = after->next;
	 item->prev = after;
	 if ( after->next ) after->next->prev = item;
	 after->next = item;
      }
      else {
	 while ( last->next ) last = last->next;
	 last->next = item;
	 item->prev = last;
      }
   }
   else {
      child = item;
      item->prev = NULL;
      item->next = NULL;
   }
}

/*-----------------------------------------------------------------------
 *  Method to return the number of children
 */

int
VItemC::ChildCount()
{
   int	count = 0;
   VItemC	*item = child;
   while ( item ) {
      count++;
      item = item->next;
   }

   return count;
}
