/*
 * $Id: RowColC.C,v 1.2 2000/05/07 12:26:11 fnevgeny Exp $
 *
 * Copyright (c) 1994 HAL Computer Systems International, Ltd.
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

#include "RowColC.h"
#include "HalAppC.h"
#include "rsrc.h"
#include "RowColP.h"

#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>
#include <Xm/PrimitiveP.h>

extern int	debug1, debug2;

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//----------------------------------------------------------------------------
// RowC constructor
//

RowC::RowC(RowColC *parent) : RowOrColC(parent)
{
   RowColPart	*rc = GetRowColPart(par->rcw);
   alignment = rc->rowAlignment;
   adjust    = rc->rowAdjust;
   resizeOk  = rc->rowResizeOk;
}

//----------------------------------------------------------------------------
// Method to determine the preferred height of this row
//

void
RowC::CalcPrefSize()
{
   unsigned	count = childList.size();
   prefSize = 0;
   for (int i=0; i<count; i++) {

      RowColChildC	*child = childList[i];
      if ( !child->w || !child->col->visible ) continue;

      int	size = child->PrefHeight();
      if ( size > prefSize ) prefSize = size;
   }

//
// Add the title column member of this row if present
//
   if ( par->titleCol && par->titleCol->visible ) {

      int		index = par->rowList.indexOf(this);
      if ( index < par->titleCol->childList.size() ) {

	 RowColChildC	*child = par->titleCol->childList[index];
	 if ( child->w ) {
	    int	size = child->PrefHeight();
	    if ( size > prefSize ) prefSize = size;
	 }
      }

   } // End if the title column is visible

} // End RowC CalcPrefSize

//----------------------------------------------------------------------------
// Method to set heights of children
//

void
RowC::SizeChildren()
{
   unsigned	count = childList.size();
   for (int i=0; i<count; i++) {
      RowColChildC	*child = childList[i];
      if      ( adjust == RcADJUST_NONE  ) child->curHt = child->PrefHeight();
      else if ( adjust == RcADJUST_EQUAL ) child->curHt = prefSize;
      else				   child->curHt = realSize;
   }

//
// Size the title column member of this row if present
//
   if ( par->titleCol ) {
      int		index = par->rowList.indexOf(this);
      if ( index < par->titleCol->childList.size() ) {
	 RowColChildC	*child = par->titleCol->childList[index];
	 if      ( adjust == RcADJUST_NONE ) child->curHt = child->PrefHeight();
	 else if ( adjust == RcADJUST_EQUAL) child->curHt = prefSize;
	 else				     child->curHt = realSize;
      }
   }

} // End RowC SizeChildren

//----------------------------------------------------------------------------
// Method to set y positions of children
//

void
RowC::PlaceChildren()
{
   unsigned	count = childList.size();
   for (int i=0; i<count; i++) {

      RowColChildC	*child = childList[i];
      child->curY = pos;

      if ( alignment == XmALIGNMENT_CENTER )
	 child->curY += (Position)((int)(realSize - child->curHt) / (int)2);
      else if ( alignment == XmALIGNMENT_END )
	 child->curY += (Position)(realSize - child->curHt);
   }

//
// Place the title column member of this row if present
//
   if ( par->titleCol ) {

      int		index = par->rowList.indexOf(this);
      if ( index < par->titleCol->childList.size() ) {

	 RowColChildC	*child = par->titleCol->childList[index];
	 child->curY = pos;

	 if ( alignment == XmALIGNMENT_CENTER )
	    child->curY += (Position)((int)(realSize - child->curHt)/(int)2);
	 else if ( alignment == XmALIGNMENT_END )
	    child->curY += (Position)(realSize - child->curHt);
      }
   }

} // End RowC PlaceChildren

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//----------------------------------------------------------------------------
// ColC constructor
//

ColC::ColC(RowColC *par) : RowOrColC(par)
{
   RowColPart	*rc = GetRowColPart(par->rcw);
   alignment = rc->colAlignment;
   adjust    = rc->colAdjust;
   resizeOk  = rc->colResizeOk;
}

//----------------------------------------------------------------------------
// Method to determine the preferred width of this column
//

void
ColC::CalcPrefSize()
{
   unsigned	count = childList.size();
   prefSize = 0;
   for (int i=0; i<count; i++) {

      RowColChildC	*child = childList[i];
      if ( !child->w || !child->row->visible ) continue;

      int	size = child->PrefWidth();
      if ( size > prefSize ) prefSize = size;
   }

//
// Add the title row member of this column if present
//
   if ( par->titleRow && par->titleRow->visible ) {

      int		index = par->colList.indexOf(this);
      if ( index < par->titleRow->childList.size() ) {

	 RowColChildC	*child = par->titleRow->childList[index];
	 if ( child->w ) {
	    int	size = child->PrefWidth();
	    if ( size > prefSize ) prefSize = size;
	 }
      }

   } // End if the title row is visible

} // End ColC CalcPrefSize

//----------------------------------------------------------------------------
// Method to set widths of children
//

void
ColC::SizeChildren()
{
   unsigned	count = childList.size();
   for (int i=0; i<count; i++) {
      RowColChildC	*child = childList[i];
      if      ( adjust == RcADJUST_NONE  ) child->curWd = child->PrefWidth();
      else if ( adjust == RcADJUST_EQUAL ) child->curWd = prefSize;
      else				   child->curWd = realSize;
   }

//
// Size the title row member of this column if present
//
   if ( par->titleRow ) {
      int		index = par->colList.indexOf(this);
      if ( index < par->titleRow->childList.size() ) {
	 RowColChildC	*child = par->titleRow->childList[index];
	 if      ( adjust == RcADJUST_NONE  ) child->curWd = child->PrefWidth();
	 else if ( adjust == RcADJUST_EQUAL ) child->curWd = prefSize;
	 else				      child->curWd = realSize;
      }
   }

} // End ColC SizeChildren

//----------------------------------------------------------------------------
// Method to set x positions of children
//

void
ColC::PlaceChildren()
{
   unsigned	count = childList.size();
   for (int i=0; i<count; i++) {

      RowColChildC	*child = childList[i];
      child->curX = pos;

      if ( alignment == XmALIGNMENT_CENTER )
	 child->curX += (Position)((int)(realSize - child->curWd) / (int)2);
      else if ( alignment == XmALIGNMENT_END )
	 child->curX += (Position)(realSize - child->curWd);
   }

//
// Place the title row member of this column if present
//
   if ( par->titleRow ) {
      int		index = par->colList.indexOf(this);
      if ( index < par->titleRow->childList.size() ) {
	 RowColChildC	*child = par->titleRow->childList[index];
	 child->curX = pos;

	 if ( alignment == XmALIGNMENT_CENTER )
	    child->curX += (Position)((int)(realSize - child->curWd) / (int)2);
	 else if ( alignment == XmALIGNMENT_END )
	    child->curX += (Position)(realSize - child->curWd);
      }
   }

} // End ColC PlaceChildren

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
//----------------------------------------------------------------------------
// RowColC constructor
//

RowColC::RowColC(Widget parent, const char *name, ArgList argv, Cardinal argc)
{
//
// Create drawing area
//
   rcw = CreateRowCol(parent, (char *)name, argv, argc);
   RowColPart	*rc = GetRowColPart(rcw);

//
// Add callbacks
//
   XtAddCallback(rcw, XmNexposeCallback, (XtCallbackProc)HandleExpose,
		 (XtPointer)this);
   XtAddCallback(rcw, XmNresizeCallback, (XtCallbackProc)HandleResize,
		 (XtPointer)this);
   XtAddCallback(rcw, RcNgeometryCallback,
   		 (XtCallbackProc)HandleGeometryRequest, (XtPointer)this);
   XtAddCallback(rcw, RcNchildGeometryCallback,
   		 (XtCallbackProc)HandleChildResize, (XtPointer)this);
   XtAddEventHandler(rcw, StructureNotifyMask, False,
		     (XtEventHandler)HandleMapChange, (XtPointer)this);

   childList.AllowDuplicates(FALSE);
   rowList.AllowDuplicates(FALSE);
   colList.AllowDuplicates(FALSE);

   titleColWidget = NULL;
   titleRowWidget = NULL;
   titleCol       = NULL;
   titleRow       = NULL;

   newWd        = 0;
   newHt        = 0;
   exposed      = False;
   deferred     = False;
   changed      = False;
   deferCount   = 0;
   selfResize   = False;
   inResize     = False;
   refreshTimer = (XtIntervalId)NULL;

//
// Create initial rows or columns
//
   if ( rc->orient == RcROW_MAJOR ) {
      for (int i=0; i<rc->colCount; i++) {
	 ColC	*col = new ColC(this);
	 colList.add(col);
      }
   } else {
      for (int i=0; i<rc->rowCount; i++) {
	 RowC	*row = new RowC(this);
	 rowList.add(row);
      }
   }

} // End RowColC constructor

//----------------------------------------------------------------------------
// RowColC destructor
//

RowColC::~RowColC()
{
   Defer(True);	// Do this so it won't try to refresh
   Reset();
}

//----------------------------------------------------------------------------
// Method to add a single child to the list.  If "child" is NULL, it means
//    the user wants an empty cell.
//

void
RowColC::AddChild(Widget wp)
{
   if ( wp && childList.includes(wp) ) return;

   Defer(True);

//
// Add child
//
   RowColChildC	*child = new RowColChildC(wp);
   childList.add(child);

   RowColPart	*rc = GetRowColPart(rcw);
   if ( rc->orient == RcROW_MAJOR ) {

//
// See if this child can be added to the last row
//
      RowC	*row = NULL;
      if ( rowList.size() > 0 ) row = (RowC*)rowList[rowList.size()-1];

      if ( !row || row->childList.size() == rc->colCount ) {
	 row = new RowC(this);
	 rowList.add(row);
	 rc->rowCount++;
      }

      row->AddChild(child);
      child->row = row;

//
// Also add this one to the appropriate column
//
      ColC	*col = NULL;
      int	colNum = row->childList.size() - 1;
      if ( colNum < colList.size() ) {
	 col = (ColC*)colList[colNum];
	 col->AddChild(child);
	 child->col = col;
      }

   } // End if ROW_MAJOR

   else {

//
// See if this child can be added to the last column
//
      ColC	*col = NULL;
      if ( colList.size() > 0 ) col = (ColC*)colList[colList.size()-1];

      if ( !col || col->childList.size() == rc->rowCount ) {
	 col = new ColC(this);
	 colList.add(col);
	 rc->colCount++;
      }

      col->AddChild(child);
      child->col = col;

//
// Also add this one to the appropriate row
//
      RowC	*row = NULL;
      int	rowNum = col->childList.size() - 1;
      if ( rowNum < rowList.size() ) {
	 row = (RowC*)rowList[rowNum];
	 row->AddChild(child);
	 child->row = row;
      }

   } // End if COL_MAJOR

   Defer(False);

} // End RowColC AddChild

//----------------------------------------------------------------------------
// Methods to add several children to the list
//

void
RowColC::AddChildren(WidgetListC& list)
{
   AddChildren(list.start(), list.size());
}

void
RowColC::AddChildren(Widget *list, int count)
{
   Defer(True);
   for (int i=0; i<count; i++) AddChild(list[i]);
   Defer(False);
}

//----------------------------------------------------------------------------
// Methods to set the child list
//

void
RowColC::SetChildren(WidgetListC& list)
{
   SetChildren(list.start(), list.size());
}

void
RowColC::SetChildren(Widget *list, int count)
{
   Defer(True);
   Reset();
   for (int i=0; i<count; i++) AddChild(list[i]);
   Defer(False);
}

//----------------------------------------------------------------------------
// Method to remove all children
//

void
RowColC::Reset()
{
   unsigned	count;
   int		i;
   RowColPart	*rc = GetRowColPart(rcw);

   if ( rc->orient == RcROW_MAJOR ) {

      count = rowList.size();
      for (i=0; i<count; i++) delete (RowC*)rowList[i];
      rowList.removeAll();

      count = colList.size();
      for (i=0; i<count; i++) colList[i]->Reset();

   } else {

      count = colList.size();
      for (i=0; i<count; i++) delete (ColC*)colList[i];
      colList.removeAll();

      count = rowList.size();
      for (i=0; i<count; i++) rowList[i]->Reset();
   }

   count = childList.size();
   for (i=0; i<count; i++) {
      RowColChildC	*child = childList[i];
      delete child;
   }

   childList.removeAll();

   if ( !deferred ) Refresh();
   else		    changed = True;

} // End RowColC Reset

/*-----------------------------------------------------------------------
 *  Method to set deferral mode
 */

void
RowColC::Defer(Boolean on)
{
   if	   ( on )	deferCount++;
   else if ( deferred ) deferCount--;

   deferred = (deferCount > 0);

   if ( !deferred && changed ) Refresh();

} // End RowColC Defer

#if 0
/*-----------------------------------------------------------------------
 *  Method to realize widgets
 */

static void
RealizeWidget(Widget w)
{
   XtRealizeWidget(w);

//
// If this is a composite, realize the children
//
   if ( XtIsComposite(w) ) {

      WidgetList	wlist;
      Cardinal		wcount;
      XtVaGetValues(w, XmNnumChildren, &wcount, XmNchildren, &wlist, 0);

      for (int i=0; i<wcount; i++)
	 RealizeWidget(wlist[i]);
   }
}
#endif

/*-----------------------------------------------------------------------
 *  Method to update geometry
 */

void
RowColC::Refresh()
{
#if 0
   if ( !XtIsRealized(rcw) ) {
      changed = True;
      return;
   }
#endif

   if ( debug2 )
      cout <<"RowColC(" <<rcw->core.name <<")::Refresh entered" <<endl;

   Boolean	saveDeferred = deferred;
   deferred = True;

   selfResize = True;

//
// Calculate preferred sizes
//
   Dimension	prefWd = PrefWidth();
   Dimension	prefHt = PrefHeight();

   if ( debug2 )
      cout <<"   we'd like to be: " <<prefWd <<" by " <<prefHt <<endl;

   if ( !inResize ) {

      if ( newWd > 0 ) {
	 prefWd = newWd;
	 newWd = 0;
	 if ( debug2 )
	    cout <<"   but the programmed width is: " <<prefWd <<endl;
      }

      if ( newHt > 0 ) {
	 prefHt = newHt;
	 newHt = 0;
	 if ( debug2 )
	    cout <<"   but the programmed height is: " <<prefHt <<endl;
      }

//
// Ask for the new size
//
      Dimension	realWd, realHt;
      XtGeometryResult answer =
	 XtMakeResizeRequest(rcw, prefWd, prefHt, &realWd, &realHt);

//
// Accept compromises
//
      if ( answer == XtGeometryAlmost ) {
	 prefWd = realWd;
	 prefHt = realHt;
	 XtMakeResizeRequest(rcw, prefWd, prefHt, NULL, NULL);
	 if ( debug2 ) cout <<"   but parent says almost, be: "
			    <<prefWd <<" by " <<prefHt <<endl;
      }

//
// Stick with what we got.
//
      else if ( answer == XtGeometryNo ) {
	 prefWd = rcw->core.width;
	 prefHt = rcw->core.height;
	 if ( debug2 ) cout <<"   but parent says no, be: "
	    		    <<prefWd <<" by " <<prefHt <<endl;
      }

//
// Some parents will say yes, but not change the size
//
      else if ( rcw->core.width != prefWd || rcw->core.height != prefHt ) {
	 prefWd = rcw->core.width;
	 prefHt = rcw->core.height;
	 if ( debug2 )
	    cout <<"   but parent lied: " <<prefWd <<" by " <<prefHt <<endl;
      }

   } // End if not called from HandleResize

//
// Update size and positions
//
   if ( titleRow ) titleRow->SetSize(titleRow->prefSize);
   if ( titleCol ) titleCol->SetSize(titleCol->prefSize);

   FitToWidth(prefWd);
   FitToHeight(prefHt);

   if ( changed ) {

//
// Refresh all rows and columns
//
      if ( titleRow ) titleRow->Refresh();
      if ( titleCol ) titleCol->Refresh();

      //cout <<"Refreshing rows" <<endl;
      unsigned	count = rowList.size();
      int	i;
      for (i=0; i<count; i++) rowList[i]->Refresh();

      //cout <<"Refreshing columns" <<endl;
      count = colList.size();
      for (i=0; i<count; i++) colList[i]->Refresh();

//
// Update the child geometries and visibilities
//
      RefreshChildren(childList);
      if ( titleRow ) RefreshChildren(titleRow->childList);
      if ( titleCol ) RefreshChildren(titleCol->childList);

   } // End if anything changed

   selfResize = False;

//
// Remove any resize events from the queue.  We don't need them.
//
   XSync(halApp->display, FALSE);
   XEvent	event;
   while ( XCheckTypedWindowEvent(halApp->display, XtWindow(rcw),
				  ConfigureNotify, &event) )
      ;

   deferred = saveDeferred;
   changed  = False;

   if ( debug2 )
      cout <<"RowColC(" <<rcw->core.name <<")::Refresh exited" <<endl;

} // End RowColC Refresh

/*-----------------------------------------------------------------------
 *  Method to loop through child list, updating geometries and visibilities
 */

void
RowColC::RefreshChildren(RowColChildListC& list)
{
   if ( debug2 )
      cout <<"RowColC(" <<rcw->core.name <<")::RefreshChildren entered" <<endl;

//
// Unmanage the widgets that will be off and the ones that will be changing.
//
   WidgetListC		unmanageList;
   WidgetListC		manageList;
   RowColChildListC	changeList;
#if 0
   Widget		traverseWidget = NULL;
#endif

   unsigned	count = list.size();
   int	i;
   for (i=0; i<count; i++) {

      RowColChildC	*child = list[i];
      if ( !child->w ) continue;

//
// Process child if it is visible
//
      if ( child->row->visible && child->col->visible ) {

//
// See if the geometry has changed.
//
	 Boolean posChange  = (child->curX != child->w->core.x ||
			       child->curY != child->w->core.y);
	 Boolean sizeChange = (child->curWd != child->w->core.width ||
			       child->curHt != child->w->core.height);

	 if ( posChange || sizeChange )
	    changeList.add(child);

//
// Manage the child again /*after changes are made or*/ if it is off.
//
	 if ( /*sizeChange ||*/ !XtIsManaged(child->w) ) {
	    if ( debug2 )
	       cout <<"   will   manage " <<child->w->core.name <<endl;
	    manageList.add(child->w);
	 }

      } // End if child is visible

//
// Unmanage the child if it will no longer be visible.
//
      else if ( XtIsManaged(child->w) ) {
	 if ( debug2 )
	    cout <<"   will unmanage " <<child->w->core.name <<endl;
	 if ( XmIsPrimitive(child->w) && Prim_HaveTraversal(child->w) ) {
	    if ( debug2 )
	       cout <<"   " <<child->w->core.name <<" has the traversal" <<endl;
#if 0
	    traverseWidget = child->w;
#endif
	 }
	 unmanageList.add(child->w);

      } // End if child is not visible

   } // End for each child

//
// Turn off widgets
//
   if ( unmanageList.size() > 0 ) {
      if ( debug2 )
	 cout <<"Unmanaging " <<unmanageList.size() <<" children" <<endl;
      XtUnmanageChildren(unmanageList.start(), unmanageList.size());
   }

//
// Update geometries
//
   count = changeList.size();
   for (i=0; i<count; i++) {

      RowColChildC	*child = changeList[i];

      if ( child->curWd > 0 && child->curHt > 0 ) {

	 if ( debug2 )
	    cout <<"   setting geometry for " <<child->w->core.name <<"\tto " 
		 <<dec(child->curWd,4) <<" by " <<dec(child->curHt,4) <<" at "
		 <<dec(child->curX,4)  <<", "   <<dec(child->curY,4)  <<endl;

	 XtConfigureWidget(child->w, child->curX, child->curY, child->curWd,
			   child->curHt, child->w->core.border_width);

#if 0
//
// Remove any resize events from the queue.  We don't need them.
//
	 XSync(halApp->display, FALSE);
	 XEvent	event;
	 while ( XCheckTypedWindowEvent(halApp->display, XtWindow(child->w),
					ConfigureNotify, &event) )
	    ;
#endif
      }
   }

//
// Turn on the visible widgets again
//
   if ( manageList.size() > 0 ) {
      if ( debug2 ) cout <<"Managing " <<manageList.size() <<" children" <<endl;
      XtManageChildren(manageList.start(), manageList.size());
   }

#if 0
//
// Reset the traversal
//
   if ( traverseWidget )
      XmProcessTraversal(traverseWidget, XmTRAVERSE_CURRENT);
#endif

} // End RowColC RefreshChildren

/*-----------------------------------------------------------------------
 *  Method to lay out the columns, using the given width
 */

void
RowColC::FitToWidth(Dimension daWd)
{
   if ( debug2 )
      cout <<"RowColC(" <<rcw->core.name <<")::FitToWidth" <<endl;

   Dimension	availWd;
   RowColPart	*rc = GetRowColPart(rcw);

//
// Loop through the columns.  Subtract the width of any that can be resized.
//
   unsigned	count = colList.size();
   unsigned	visCount = 0;
   int	i;
   for (i=0; i<count; i++) {
      ColC	*col = (ColC*)colList[i];
      if ( col->visible ) visCount++;
   }

   availWd = daWd - rc->marginWd*2 - rc->colSpacing*(visCount-1);

   unsigned	resizeCount = 0;
   for (i=0; i<count; i++) {
      ColC	*col = (ColC*)colList[i];
      if ( col->visible ) {
	 if ( col->resizeOk ) resizeCount++;
	 else {
	    col->SetSize(col->prefSize);
	    availWd -= col->prefSize;
	 }
      }
   }

//
// If there are any that can be resized and there is any available width,
//    divide it up.
//
   if ( availWd > 0 && resizeCount > 0 ) {

//
// If the columns are to be the same size, divide the space evenly.
//
      if ( rc->uniformCols ) {
	 int	colWd = (int)availWd / (int)resizeCount;
	 for (i=0; i<count; i++) {
	    ColC	*col = (ColC*)colList[i];
	    if ( col->visible && col->resizeOk ) col->SetSize(colWd);
	 }
      }

//
// If the columns don't have to be the same size, divide the space
//    proportionally.
//
      else {

//
// See how much space they would like to occupy
//
	 Dimension	prefTotal = 0;
	 for (i=0; i<count; i++) {
	    ColC	*col = (ColC*)colList[i];
	    if ( col->visible && col->resizeOk )
	       prefTotal += col->prefSize;
	 }

//
// Set the new size in proportion to the old size
//
	 if ( prefTotal > 0 ) {
	    for (i=0; i<count; i++) {
	       ColC	*col = (ColC*)colList[i];
	       if ( col->visible && col->resizeOk ) {
		  float	scale = (float)col->prefSize / (float)prefTotal;
		  int	newWd = (int)(scale * (float)availWd);
		  col->SetSize(newWd);
	       }
	    }
	 }

      } // End if space divided up proportionally

   } // End if there are columns to be resized

//
// Now set the column positions
//
   int	x = rc->marginWd;
   for (i=0; i<count; i++) {
      ColC	*col = (ColC*)colList[i];
      if ( col->visible ) {
	 col->SetPosition(x);
	 x += col->realSize + rc->colSpacing;
      }
   }

} // End RowColC FitToWidth

/*-----------------------------------------------------------------------
 *  Method to lay out the rows, using the given height
 */

void
RowColC::FitToHeight(Dimension daHt)
{
   if ( debug2 )
      cout <<"RowColC(" <<rcw->core.name <<")::FitToHeight" <<endl;

   Dimension	availHt;
   RowColPart	*rc = GetRowColPart(rcw);

//
// Loop through the rows.  Subtract the height of any that can be resized.
//
   unsigned	count = rowList.size();
   unsigned	visCount = 0;
   int	i;
   for (i=0; i<count; i++) {
      RowC	*row = (RowC*)rowList[i];
      if ( row->visible ) visCount++;
   }

   availHt = daHt - rc->marginHt*2 - rc->rowSpacing*(visCount-1);

   unsigned	resizeCount = 0;
   for (i=0; i<count; i++) {
      RowC	*row = (RowC*)rowList[i];
      if ( row->visible ) {
	 if ( row->resizeOk ) resizeCount++;
	 else {
	    row->SetSize(row->prefSize);
	    availHt -= row->prefSize;
	 }
      }
   }

//
// If there are any that can be resized and there is any available height,
//    divide it up.
//
   if ( availHt > 0 && resizeCount > 0 ) {

//
// If the rows are to be the same size, divide the space evenly.
//
      if ( rc->uniformRows ) {
	 int	rowHt = (int)availHt / (int)resizeCount;
	 for (i=0; i<count; i++) {
	    RowC	*row = (RowC*)rowList[i];
	    if ( row->visible && row->resizeOk ) row->SetSize(rowHt);
	 }
      }

//
// If the rows don't have to be the same size, divide the space
//    proportionally.
//
      else {

//
// See how much space they would like to occupy
//
	 Dimension	prefTotal = 0;
	 for (i=0; i<count; i++) {
	    RowC	*row = (RowC*)rowList[i];
	    if ( row->visible && row->resizeOk )
	       prefTotal += row->prefSize;
	 }

//
// Set the new size in proportion to the old size
//
	 if ( prefTotal > 0 ) {
	    for (i=0; i<count; i++) {
	       RowC	*row = (RowC*)rowList[i];
	       if ( row->visible && row->resizeOk ) {
		  float	scale = (float)row->prefSize / (float)prefTotal;
		  int	newHt = (int)(scale * (float)availHt);
		  row->SetSize(newHt);
	       }
	    }
	 }

      } // End if space divided up proportionally

   } // End if there are rows to be resized

//
// Now set the row positions
//
   int	y = rc->marginHt;
   for (i=0; i<count; i++) {
      RowC	*row = (RowC*)rowList[i];
      if ( row->visible ) {
	 row->SetPosition(y);
	 y += row->realSize + rc->rowSpacing;
      }
   }

} // End RowColC FitToHeight

/*-----------------------------------------------------------------------
 *  Callback for expose
 */

void
RowColC::HandleExpose(Widget w, RowColC *This, XtPointer)
{
   if ( debug2 )
      cout <<"RowColC(" <<This->rcw->core.name <<")::HandleExpose" <<endl;

   XtRemoveCallback(w, XmNexposeCallback, (XtCallbackProc)HandleExpose,
		    (XtPointer)This);

   This->ExposeIt();
}

void
RowColC::HandleMapChange(Widget, RowColC *This, XEvent *ev, Boolean*)
{
   if ( ev->type != MapNotify ) return;

   if ( debug2 )
      cout <<"RowColC(" <<This->rcw->core.name <<")::HandleMapChange" <<endl;

   XtRemoveEventHandler(This->rcw, StructureNotifyMask, False,
			(XtEventHandler)HandleMapChange, (XtPointer)This);

   This->ExposeIt();
}

void
RowColC::ExposeIt()
{
   if ( exposed ) return;

//
// Reset preferred sizes
//
   unsigned	count = childList.size();
   int	i;
   for (i=0; i<count; i++) {
      RowColChildC	*child = childList[i];
      child->prefWd = 0;
      child->prefHt = 0;
   }

   if ( titleRow ) {
      count = titleRow->childList.size();
      for (i=0; i<count; i++) {
	 RowColChildC	*child = titleRow->childList[i];
	 child->prefWd = 0;
	 child->prefHt = 0;
      }
   }

   if ( titleCol ) {
      count = titleCol->childList.size();
      for (i=0; i<count; i++) {
	 RowColChildC	*child = titleCol->childList[i];
	 child->prefWd = 0;
	 child->prefHt = 0;
      }
   }

   exposed = True;

//
// Always redraw after the first expose event
//
   if ( !deferred ) Refresh();
   else		    changed = True;

} // End ExposeIt

/*-----------------------------------------------------------------------
 *  Callback for resize
 */

void
RowColC::HandleResize(Widget w, RowColC *This, XtPointer)
{
   if ( This->inResize || This->selfResize ) return;

   if ( debug2 )
      cout <<"Resize for " <<XtName(w) <<" is "
	   <<w->core.width SP w->core.height <<endl;

   This->inResize = True;
   This->Refresh();
   This->inResize = False;
}

/*-----------------------------------------------------------------------
 *  Callbacks for child resize
 */

void
RowColC::HandleChildResize(Widget, RowColC *This, XtPointer data)
{
   RowColGeometryCallbackStruct *geo = (RowColGeometryCallbackStruct *)data;
   This->ChildResize(geo->widget, geo->desiredWd, geo->desiredHt);
}

void
RowColC::HandleTitleResize(Widget w, RowColC *This, XEvent *ev, Boolean*)
{
   if ( ev->type != ConfigureNotify && ev->type != MapNotify ) return;

//
// Let's find out what size the child really wants to be
//
   XtWidgetGeometry	geo;
   XtQueryGeometry(w, NULL, &geo);

   This->ChildResize(w, geo.width, geo.height);
}

void
RowColC::ChildResize(Widget w, Dimension newWd, Dimension newHt)
{
   if ( selfResize ) return;

//
// Look up this widget in the child list
//
   RowColChildC	*child = NULL;

   int	index = childList.indexOf(w);
   if ( index != childList.NULL_INDEX ) child = childList[index];

//
// If it wasn't found, try the title row
//
   if ( !child && titleRow ) {
      index = titleRow->childList.indexOf(w);
      if ( index != titleRow->childList.NULL_INDEX )
	 child = titleRow->childList[index];
   }

//
// If it still wasn't found, try the title column
//
   if ( !child && titleCol ) {
      index = titleCol->childList.indexOf(w);
      if ( index != titleCol->childList.NULL_INDEX )
	 child = titleCol->childList[index];
   }

//
// If it still wasn't found, punt
//
   if ( !child ) return;

   if ( child->prefWd == newWd && child->prefHt == newHt ) return;

   if ( debug2 )
      cout <<"Resize for child " <<XtName(w) <<" is "
	   <<newWd SP newHt <<endl;

   child->prefWd = newWd;
   child->prefHt = newHt;

   changed = True;

   if ( deferred ) return;

   if ( !child->row->visible || !child->col->visible ) return;

//
// Register a time-out proc to do the drawing.  We do this since we are likely
//   to get several child resizes together and we'd like to refresh only once.
//
   if ( refreshTimer ) {
//
// Kill the current timer
//
      XtRemoveTimeOut(refreshTimer);
      refreshTimer = (XtIntervalId)NULL;
   }

//
// Start another timer
//
   refreshTimer = XtAppAddTimeOut(halApp->context, 100/*usec*/,
				        (XtTimerCallbackProc)FinishChildResize,
				        (XtPointer)this);

} // End ChildResize

/*---------------------------------------------------------------
 *  Timer proc to perform a refresh after a child resize
 */

void
RowColC::FinishChildResize(RowColC *This, XtIntervalId*)
{
   This->refreshTimer = (XtIntervalId)NULL;
   This->changed = True;
   This->Refresh();
}

//----------------------------------------------------------------------------
// Method to reset the children to their original sizes and reload them.
//

void
RowColC::ReloadChildren()
{
   //cout <<"RowColC(" <<rcw->core.name <<")::ReloadChildren" <<endl;

   selfResize = True;

//
// Reset the preferred sizes of the children
//
   unsigned	wcount = childList.size();
   WidgetListC	wlist;
   wlist.AllowDuplicates(TRUE);
   for (int i=0; i<wcount; i++) {

      RowColChildC	*child = childList[i];
      wlist.add(child->w);

      child->prefWd = 0;
      child->prefHt = 0;
   }

//
// Re-set the list of widgets
//
   SetChildren(wlist);

   selfResize = False;
}

//----------------------------------------------------------------------------
// Method to set the number of columns.  This is only useful in ROW_MAJOR
//    orientation.
//

void
RowColC::SetColCount(int count)
{
   RowColPart	*rc = GetRowColPart(rcw);

   if ( rc->orient == RcCOL_MAJOR || rc->colCount == count ) return;

//
// Add or delete columns as necessary
//
   if ( count > rc->colCount ) {
      for (int i=rc->colCount; i<count; i++) {
	 ColC	*col = new ColC(this);
	 colList.add(col);
      }
   } else {
      for (int i=rc->colCount-1; i>=count; i--) {
	 ColC	*col = (ColC*)colList[i];
	 colList.remove(i);
	 delete col;
      }
   }

   rc->colCount = count;

   ReloadChildren();

} // End RowColC SetColCount

//----------------------------------------------------------------------------
// Method to set the number of rows.  This is only useful in COL_MAJOR
//    orientation.
//

void
RowColC::SetRowCount(int count)
{
   RowColPart	*rc = GetRowColPart(rcw);

   if ( rc->orient == RcROW_MAJOR || rc->rowCount == count ) return;

//
// Add or delete rows as necessary
//
   if ( count > rc->rowCount ) {

      for (int i=rc->rowCount; i<count; i++) {
	 RowC	*row = new RowC(this);
	 rowList.add(row);
      }

   } else {

      for (int i=rc->rowCount-1; i>=count; i--) {
	 RowC	*row = (RowC*)rowList[i];
	 rowList.remove(i);
	 delete row;
      }
   }

   rc->rowCount = count;

   ReloadChildren();

} // End RowColC SetRowCount

//----------------------------------------------------------------------------
// Method to set the number of pixels between columns.
//

void
RowColC::SetColSpacing(int space)
{
   RowColPart	*rc = GetRowColPart(rcw);

   if ( rc->colSpacing == space ) return;

   rc->colSpacing = space;

   if ( !deferred ) Refresh();
   else		    changed = True;
}

//----------------------------------------------------------------------------
// Method to set the number of pixels between rows.
//

void
RowColC::SetRowSpacing(int space)
{
   RowColPart	*rc = GetRowColPart(rcw);

   if ( rc->rowSpacing == space ) return;

   rc->rowSpacing = space;

   if ( !deferred ) Refresh();
   else		    changed = True;
}

//----------------------------------------------------------------------------
// Method to set the margin width
//

void
RowColC::SetMarginWidth(int wd)
{
   RowColPart	*rc = GetRowColPart(rcw);

   if ( rc->marginWd == wd ) return;

   rc->marginWd = wd;

   if ( !deferred ) Refresh();
   else		    changed = True;
}

//----------------------------------------------------------------------------
// Method to set the margin height
//

void
RowColC::SetMarginHeight(int ht)
{
   RowColPart	*rc = GetRowColPart(rcw);

   if ( rc->marginHt == ht ) return;

   rc->marginHt = ht;

   if ( !deferred ) Refresh();
   else		    changed = True;
}

//----------------------------------------------------------------------------
// Method to set the orientation.  If ROW_MAJOR, new children are added to
//   a row until there are colCount children in that row.  If COL_MAJOR, new
//   children are added to a column until there are rowCount children in that
//   column.
//

void
RowColC::SetOrientation(RcOrientT orient)
{
   RowColPart	*rc = GetRowColPart(rcw);

   if ( rc->orient == orient ) return;

//
// Delete all rows and columns
//
   unsigned	count = colList.size();
   int	i;
   for (i=0; i<count; i++) delete (ColC*)colList[i];
   colList.removeAll();

   count = rowList.size();
   for (i=0; i<count; i++) delete (RowC*)rowList[i];
   rowList.removeAll();

   rc->orient = orient;

//
// Create new fixed rows or columns
//
   if ( rc->orient == RcROW_MAJOR ) {
      rc->colCount = rc->rowCount;
      for (i=0; i<rc->colCount; i++) {
	 ColC	*col = new ColC(this);
	 colList.add(col);
      }
   }

   else {
      rc->rowCount = rc->colCount;
      for (i=0; i<rc->rowCount; i++) {
	 RowC	*row = new RowC(this);
	 rowList.add(row);
      }
   }

   ReloadChildren();

} // End RowColC SetOrientation

//----------------------------------------------------------------------------
// Method to set the height resize flag.  If True, the drawing area will
//    attempt to set its own height.  If False, the drawing area will use
//    whatever is available.
//

void
RowColC::SetResizeHeight(RcResizeT val)
{
   RowColPart	*rc = GetRowColPart(rcw);

   if ( rc->hResizePolicy == val ) return;

   rc->hResizePolicy = val;

   if ( !deferred ) Refresh();
   else		    changed = True;

} // End RowColC SetResizeHeight

//----------------------------------------------------------------------------
// Method to set the width resize flag.  If True, the drawing area will
//    attempt to set its own width.  If False, the drawing area will use
//    whatever is available.
//

void
RowColC::SetResizeWidth(RcResizeT val)
{
   RowColPart	*rc = GetRowColPart(rcw);

   if ( rc->wResizePolicy == val ) return;

   rc->wResizePolicy = val;

   if ( !deferred ) Refresh();
   else		    changed = True;

} // End RowColC SetResizeWidth

//----------------------------------------------------------------------------
// Method to set the flag that determines whether all columns are forced to
//   the same width.  If True, all columns will be as wide as the widest
//   column.
//

void
RowColC::SetUniformCols(Boolean val)
{
   RowColPart	*rc = GetRowColPart(rcw);

   if ( (rc->uniformCols && val) || (!rc->uniformCols && !val) ) return;

   rc->uniformCols = val;

   if ( !deferred ) Refresh();
   else		    changed = True;

} // End RowColC SetUniformCols

//----------------------------------------------------------------------------
// Method to set the flag that determines whether all rows are forced to
//   the same height.  If True, all rows will be as tall as the tallest
//   row.
//

void
RowColC::SetUniformRows(Boolean val)
{
   RowColPart	*rc = GetRowColPart(rcw);

   if ( (rc->uniformRows && val) || (!rc->uniformRows && !val) ) return;

   rc->uniformRows = val;

   if ( !deferred ) Refresh();
   else		    changed = True;

} // End RowColC SetUniformRows

//----------------------------------------------------------------------------
// Method to set the alignment of all columns
//

void
RowColC::SetColAlignment(RcAlignT align)
{
   Defer(True);

   RowColPart	*rc = GetRowColPart(rcw);
   rc->colAlignment = align;

   unsigned	count = colList.size();
   for (int i=0; i<count; i++)
      SetColAlignment(i, align);

   Defer(False);

} // End RowColC SetColAlignment

//----------------------------------------------------------------------------
// Method to set the alignment of a single column
//

void
RowColC::SetColAlignment(int i, RcAlignT align)
{
   if ( i < 0 || i >= colList.size() ) return;

   Defer(True);
   colList[i]->SetAlignment(align);
   Defer(False);

} // End RowColC SetColAlignment

//----------------------------------------------------------------------------
// Method to set the alignment of all rows
//

void
RowColC::SetRowAlignment(RcAlignT align)
{
   Defer(True);

   RowColPart	*rc = GetRowColPart(rcw);
   rc->rowAlignment = align;

   unsigned	count = rowList.size();
   for (int i=0; i<count; i++)
      SetRowAlignment(i, align);

   Defer(False);

} // End RowColC SetRowAlignment

//----------------------------------------------------------------------------
// Method to set the alignment of a single row
//

void
RowColC::SetRowAlignment(int i, RcAlignT align)
{
   if ( i < 0 || i >= rowList.size() ) return;

   Defer(True);
   rowList[i]->SetAlignment(align);
   Defer(False);

} // End RowColC SetRowAlignment

//----------------------------------------------------------------------------
// Method to set the resize flag for all columns.  When True, the columns
//    will grow as the drawing area widens and shrink as it narrows.
//

void
RowColC::SetColResize(Boolean val)
{
   Defer(True);

   unsigned	count = colList.size();
   for (int i=0; i<count; i++)
      SetColResize(i, val);

   Defer(False);

} // End RowColC SetColResize

//----------------------------------------------------------------------------
// Method to set the resize flag for a single column.  When True, the column
//    will grow as the drawing area widens and shrink as it narrows.  When
//    False, the column will always be as wide as its widest member.
//

void
RowColC::SetColResize(int i, Boolean val)
{
   if ( i < 0 || i >= colList.size() ) return;

   Defer(True);
   colList[i]->SetResize(val);
   Defer(False);

} // End RowColC SetColResize

//----------------------------------------------------------------------------
// Method to set the resize flag for all rows.  When True, the rows
//    will grow as the drawing area grows taller and shrink as it grows shorter.
//    When False, the rows will always be as tall as their tallest member.
//

void
RowColC::SetRowResize(Boolean val)
{
   Defer(True);

   unsigned	count = rowList.size();
   for (int i=0; i<count; i++)
      SetRowResize(i, val);

   Defer(False);

} // End RowColC SetRowResize

//----------------------------------------------------------------------------
// Method to set the resize flag for a single row.  When True, the row
//    will grow as the drawing area grow taller and shrink as it grows shorter.
//    When False, the row will always be as tall as its tallest member.
//

void
RowColC::SetRowResize(int i, Boolean val)
{
   if ( i < 0 || i >= rowList.size() ) return;

   Defer(True);
   rowList[i]->SetResize(val);
   Defer(False);

} // End RowColC SetRowResize

//----------------------------------------------------------------------------
// Method to set the width adjust mode of all columns.  A column can make all
//    its members as wide as the column itself, as wide as the widest member
//    or leave their widths alone.
//

void
RowColC::SetColWidthAdjust(RcAdjustT mode)
{
   Defer(True);

   unsigned	count = colList.size();
   for (int i=0; i<count; i++)
      SetColWidthAdjust(i, mode);

   Defer(False);

} // End RowColC SetColWidthAdjust

//----------------------------------------------------------------------------
// Method to set the width adjust mode of a single column.  A column can make
//    all its members as wide as the column itself, as wide as the widest
//    member or leave their widths alone.
//

void
RowColC::SetColWidthAdjust(int i, RcAdjustT mode)
{
   if ( i < 0 || i >= colList.size() ) return;

   Defer(True);
   colList[i]->SetAdjust(mode);
   Defer(False);

} // End RowColC SetColWidthAdjust

//----------------------------------------------------------------------------
// Method to set the height adjust mode of all rows.  A row can make all
//    its members as tall as the row itself, as tall as the tallest member
//    or leave their heights alone.
//

void
RowColC::SetRowHeightAdjust(RcAdjustT mode)
{
   Defer(True);

   unsigned	count = rowList.size();
   for (int i=0; i<count; i++)
      SetRowHeightAdjust(i, mode);

   Defer(False);

} // End RowColC SetRowHeightAdjust

//----------------------------------------------------------------------------
// Method to set the height adjust mode of a single row.  A row can make
//    all its members as tall as the row itself, as tall as the tallest
//    member or leave their heights alone.
//

void
RowColC::SetRowHeightAdjust(int i, RcAdjustT mode)
{
   if ( i < 0 || i >= rowList.size() ) return;

   Defer(True);
   rowList[i]->SetAdjust(mode);
   Defer(False);

} // End RowColC SetRowHeightAdjust

//----------------------------------------------------------------------------
// Method to set the visibility flag for a single column.  When False, the
//    children in the column are unmanaged.
//

void
RowColC::SetColVisible(int i, Boolean val)
{
   if ( i < 0 || i >= colList.size() ) return;

   Defer(True);
   colList[i]->SetVisible(val);
   Defer(False);

} // End RowColC SetColVisible

//----------------------------------------------------------------------------
// Method to set the visibility flag for a single row.  When False, the
//    children in the row are unmanaged.
//

void
RowColC::SetRowVisible(int i, Boolean val)
{
   if ( i < 0 || i >= rowList.size() ) return;

   Defer(True);
   rowList[i]->SetVisible(val);
   Defer(False);

} // End RowColC SetRowVisible

//----------------------------------------------------------------------------
// Method to return the alignment of a single column
//

RcAlignT
RowColC::ColAlignment(int i) const
{
   if ( i < 0 || i >= colList.size() ) return XmALIGNMENT_CENTER;

   return colList[i]->alignment;
}

//----------------------------------------------------------------------------
// Method to return the alignment of a single row
//

RcAlignT
RowColC::RowAlignment(int i) const
{
   if ( i < 0 || i >= rowList.size() ) return XmALIGNMENT_CENTER;

   return rowList[i]->alignment;
}

//----------------------------------------------------------------------------
// Method to return the column count
//

int
RowColC::ColCount() const
{
   RowColPart	*rc = GetRowColPart(rcw);
   return rc->colCount;
}

//----------------------------------------------------------------------------
// Method to return the row count
//

int
RowColC::RowCount() const
{
   RowColPart	*rc = GetRowColPart(rcw);
   return rc->rowCount;
}

//----------------------------------------------------------------------------
// Method to return the width adjust mode of a single column
//

RcAdjustT
RowColC::ColWidthAdjust(int i) const
{
   if ( i < 0 || i >= colList.size() ) return RcADJUST_NONE;

   return colList[i]->adjust;
}

//----------------------------------------------------------------------------
// Method to return the width adjust mode of a single row
//

RcAdjustT
RowColC::RowHeightAdjust(int i) const
{
   if ( i < 0 || i >= rowList.size() ) return RcADJUST_NONE;

   return rowList[i]->adjust;
}

//----------------------------------------------------------------------------
// Method to return the resize flag of a single column
//

Boolean
RowColC::ColResize(int i) const
{
   if ( i < 0 || i >= colList.size() ) return False;

   return colList[i]->resizeOk;
}

//----------------------------------------------------------------------------
// Method to return the resize flag of a single row
//

Boolean
RowColC::RowResize(int i) const
{
   if ( i < 0 || i >= rowList.size() ) return False;

   return rowList[i]->resizeOk;
}

//----------------------------------------------------------------------------
// Method to return the visibility flag of a single column
//

Boolean
RowColC::ColVisible(int i) const
{
   if ( i < 0 || i >= colList.size() ) return False;

   return colList[i]->visible;
}

//----------------------------------------------------------------------------
// Method to return the visibility flag of a single row
//

Boolean
RowColC::RowVisible(int i) const
{
   if ( i < 0 || i >= rowList.size() ) return False;

   return rowList[i]->visible;
}

//----------------------------------------------------------------------------
// Method to return the orientation
//

RcOrientT
RowColC::Orientation() const
{
   RowColPart	*rc = GetRowColPart(rcw);
   return rc->orient;
}

//----------------------------------------------------------------------------
// Method to return the width resize policy
//

RcResizeT
RowColC::ResizeWidth() const
{
   RowColPart	*rc = GetRowColPart(rcw);
   return rc->wResizePolicy;
}

//----------------------------------------------------------------------------
// Method to return the height resize policy
//

RcResizeT
RowColC::ResizeHeight() const
{
   RowColPart	*rc = GetRowColPart(rcw);
   return rc->hResizePolicy;
}

//----------------------------------------------------------------------------
// Method to return the uniform columns flag
//

Boolean
RowColC::UniformCols() const
{
   RowColPart	*rc = GetRowColPart(rcw);
   return rc->uniformCols;
}

//----------------------------------------------------------------------------
// Method to return the uniform rows flag
//

Boolean
RowColC::UniformRows() const
{
   RowColPart	*rc = GetRowColPart(rcw);
   return rc->uniformRows;
}

//----------------------------------------------------------------------------
// Method to add a single child to the title row.  If "child" is NULL, it means
//    the user wants an empty cell.
//

void
RowColC::AddTitleRowChild(Widget wp)
{
   Defer(True);

//
// Add child
//
   RowColChildC	*child = new RowColChildC(wp);

   if ( !titleRowWidget ) titleRowWidget = XtParent(wp);
   if ( !titleRow       ) titleRow       = new RowC(this);

   titleRow->AddChild(child);
   child->row = titleRow;

//
// Add an event handler to detect child size changes
//
   if ( wp )
      XtAddEventHandler(wp, StructureNotifyMask, False,
			(XtEventHandler)HandleTitleResize, (XtPointer)this);

//
// Point to the appropriate column
//
   int	colNum = titleRow->childList.size() - 1;
   if ( colNum < colList.size() )
      child->col = (ColC*)colList[colNum];

   Defer(False);

} // End RowColC AddTitleRowChild

//----------------------------------------------------------------------------
// Methods to add several children to the title row
//

void
RowColC::AddTitleRowChildren(WidgetListC& list)
{
   AddTitleRowChildren(list.start(), list.size());
}

void
RowColC::AddTitleRowChildren(Widget *list, int count)
{
   Defer(True);
   for (int i=0; i<count; i++) AddTitleRowChild(list[i]);
   Defer(False);
}

//----------------------------------------------------------------------------
// Methods to set the title row child list
//

void
RowColC::SetTitleRowChildren(WidgetListC& list)
{
   SetTitleRowChildren(list.start(), list.size());
}

void
RowColC::SetTitleRowChildren(Widget *list, int count)
{
   Defer(True);

   if ( titleRow ) {
      unsigned	ccnt = titleRow->childList.size();
      for (int i=0; i<ccnt; i++) delete titleRow->childList[i];
      titleRow->Reset();
   }

   titleRowWidget = NULL;

   for (int i=0; i<count; i++) AddTitleRowChild(list[i]);

   Defer(False);
}

//----------------------------------------------------------------------------
// Method to add a single child to the title column.  If "child" is NULL, it
//    means the user wants an empty cell.
//

void
RowColC::AddTitleColChild(Widget wp)
{
   Defer(True);

//
// Add child
//
   RowColChildC	*child = new RowColChildC(wp);

   if ( !titleColWidget ) titleColWidget = XtParent(wp);
   if ( !titleCol       ) titleCol       = new ColC(this);

   titleCol->AddChild(child);
   child->col = titleCol;

//
// Add an event handler to detect child size changes
//
   if ( wp )
      XtAddEventHandler(wp, StructureNotifyMask, False,
			(XtEventHandler)HandleTitleResize, (XtPointer)this);

//
// Point to the appropriate row
//
   int	rowNum = titleCol->childList.size() - 1;
   if ( rowNum < rowList.size() )
      child->row = (RowC*)rowList[rowNum];

   Defer(False);

} // End RowColC AddTitleColChild

//----------------------------------------------------------------------------
// Methods to add several children to the title column
//

void
RowColC::AddTitleColChildren(WidgetListC& list)
{
   AddTitleColChildren(list.start(), list.size());
}

void
RowColC::AddTitleColChildren(Widget *list, int count)
{
   Defer(True);
   for (int i=0; i<count; i++) AddTitleColChild(list[i]);
   Defer(False);
}

//----------------------------------------------------------------------------
// Methods to set the title column child list
//

void
RowColC::SetTitleColChildren(WidgetListC& list)
{
   SetTitleColChildren(list.start(), list.size());
}

void
RowColC::SetTitleColChildren(Widget *list, int count)
{
   Defer(True);

   if ( titleCol ) {
      unsigned	ccnt = titleCol->childList.size();
      for (int i=0; i<ccnt; i++) delete titleCol->childList[i];
      titleCol->Reset();
   }

   titleColWidget = NULL;

   for (int i=0; i<count; i++) AddTitleColChild(list[i]);

   Defer(False);
}

//----------------------------------------------------------------------------
// Method to set the width
//

void
RowColC::SetWidth(int wd)
{
   if ( wd == rcw->core.width ) return;

   newWd = wd;

   //cout <<"Setting " <<rcw->core.name <<" width to " <<wd <<endl;

   if ( !deferred ) Refresh();
   else		    changed = True;

} // End SetWidth

//----------------------------------------------------------------------------
// Method to set the height
//

void
RowColC::SetHeight(int ht)
{
   if ( ht == rcw->core.height ) return;

   newHt = ht;

   if ( !deferred ) Refresh();
   else		    changed = True;

} // End SetWidth

/*-----------------------------------------------------------------------
 *  Callback for geometry request
 */

void
RowColC::HandleGeometryRequest(Widget, RowColC *This, XtPointer data)
{
   RowColGeometryCallbackStruct *geo = (RowColGeometryCallbackStruct *)data;

   geo->desiredWd = This->PrefWidth();
   geo->desiredHt = This->PrefHeight();
}

/*-----------------------------------------------------------------------
 *  Method to determine the preferred width
 */

Dimension
RowColC::PrefWidth()
{
   RowColPart	*rc = GetRowColPart(rcw);

   if ( debug2 ) {
      cout <<"RowColC(" <<rcw->core.name <<")::PrefWidth" <<endl;
      cout <<"   width resize policy is ";
      switch (rc->wResizePolicy) {
	 case XmRESIZE_NONE: cout <<"NONE"; break;
	 case XmRESIZE_GROW: cout <<"GROW"; break;
	 case XmRESIZE_ANY:  cout <<"ANY";  break;
      }
      cout <<endl;
   }

//
// Have the columns recalculate their preferred widths.
//
   if ( titleCol ) titleCol->CalcPrefSize();

   unsigned	count = colList.size();
   int	i;
   for (i=0; i<count; i++) colList[i]->CalcPrefSize();

//
// If we can't change the total width we can stop here.  We did the above
//   calculations anyway so we would know the preferred sizes of the children.
//
   if ( inResize || (exposed && rc->wResizePolicy == XmRESIZE_NONE) ) {
      if ( debug2 ) cout <<"   no width resize is allowed" <<endl;
      return (rc->setWidth>0) ? rc->setWidth : rcw->core.width;
   }

   Dimension	totalWd;

//
// If the columns are to be the same width, get the maximum
//
   if ( rc->uniformCols ) {

      unsigned	visCount = 0;
      int	maxWd = 0;
      for (i=0; i<count; i++) {
	 ColC	*col = (ColC*)colList[i];
	 if ( col->visible ) {
	    visCount++;
	    if ( col->prefSize > maxWd ) maxWd = col->prefSize;
	 }
      }

      totalWd = rc->marginWd*2 + (maxWd*visCount) + rc->colSpacing*(visCount-1);

   } // End if columns need to be all the same width

//
// If the columns can be any size, add up the preferred widths
//
   else {

      totalWd = rc->marginWd*2;
      for (i=0; i<count; i++) {
	 ColC	*col = (ColC*)colList[i];
	 if ( col->visible ) totalWd += col->prefSize + rc->colSpacing;
      }
      totalWd -= rc->colSpacing;
   }

   if ( debug2 ) cout <<"   preferred width is " <<totalWd <<endl;

   if ( exposed && rc->wResizePolicy == XmRESIZE_GROW ) {

      Dimension	widgetWd = (rc->setWidth>0) ? rc->setWidth : rcw->core.width;

      if ( totalWd < widgetWd ) {
	 if ( debug2 ) cout <<"   the width is not allowed to shrink" <<endl;
	 return widgetWd;
      }
   }

   return totalWd;

} // End PrefWidth

/*-----------------------------------------------------------------------
 *  Method to determine the preferred height
 */

Dimension
RowColC::PrefHeight()
{
   RowColPart	*rc = GetRowColPart(rcw);

   if ( debug2 ) {
      cout <<"RowColC(" <<rcw->core.name <<")::PrefHeight" <<endl;
      cout <<"   height resize policy is ";
      switch (rc->hResizePolicy) {
	 case XmRESIZE_NONE: cout <<"NONE"; break;
	 case XmRESIZE_GROW: cout <<"GROW"; break;
	 case XmRESIZE_ANY:  cout <<"ANY";  break;
      }
      cout <<endl;
   }

//
// Have the rows recalculate their preferred heights
//
   if ( titleRow ) titleRow->CalcPrefSize();

   unsigned	count = rowList.size();
   int	i;
   for (i=0; i<count; i++) rowList[i]->CalcPrefSize();

//
// If we can't change the total height we can stop here.  We did the above
//   calculations anyway so we would know the preferred sizes of the children.
//
   if ( inResize || (exposed && rc->hResizePolicy == XmRESIZE_NONE) ) {
      if ( debug2 ) cout <<"   no height resize is allowed" <<endl;
      return (rc->setHeight>0) ? rc->setHeight : rcw->core.height;
   }

   Dimension	totalHt;

//
// If the rows are to be the same height, get the maximum
//
   if ( rc->uniformRows ) {

      unsigned	visCount = 0;
      int	maxHt = 0;
      for (i=0; i<count; i++) {
	 RowC	*row = (RowC*)rowList[i];
	 if ( row->visible ) {
	    visCount++;
	    if ( row->prefSize > maxHt ) maxHt = row->prefSize;
	 }
      }

      totalHt = rc->marginHt*2 + (maxHt*visCount) + rc->rowSpacing*(visCount-1);

   } // End if rows need to be all the same height

//
// If the rows can be any size, add up the preferred heights
//
   else {

      totalHt = rc->marginHt*2;
      for (i=0; i<count; i++) {
	 RowC	*row = (RowC*)rowList[i];
	 if ( row->visible ) totalHt += row->prefSize + rc->rowSpacing;
      }
      totalHt -= rc->rowSpacing;
   }

   if ( debug2 ) cout <<"   preferred height is " <<totalHt <<endl;

   if ( exposed && rc->hResizePolicy == XmRESIZE_GROW ) {

      Dimension	widgetHt = (rc->setHeight>0) ? rc->setHeight : rcw->core.height;

      if ( totalHt < widgetHt ) {
	 if ( debug2 ) cout <<"   the height is not allowed to shrink" <<endl;
	 return widgetHt;
      }
   }

   return totalHt;

} // End PrefHeight

//-----------------------------------------------------------------------
// Method to look up widget in list
//

int
RowColC::ChildListIndexOf(Widget w) const
{
   u_int	count = childList.size();
   for (int i=0; i<count; i++) {
      RowColChildC	*child = childList[i];
      if ( child->w == w ) return i;
   }

   return childList.NULL_INDEX;
}

Boolean
RowColC::ChildListIncludes(Widget w) const
{
   return (ChildListIndexOf(w) != childList.NULL_INDEX);
}

