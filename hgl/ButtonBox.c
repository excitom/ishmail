/*
 *  $Id: ButtonBox.c,v 1.4 2000/05/07 12:26:10 fnevgeny Exp $
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

#include <stdio.h>
#include "ButtonBoxP.h"

/********    Static Function Declarations    ********/

static void		ChangeManaged(Widget);
static void		ClassInitialize(void);
static void		ClassPartInitialize(WidgetClass);
static void		ConstraintInitialize(Widget, Widget, ArgList,
					     Cardinal*);
static Boolean		ConstraintSetValues(Widget, Widget, Widget, ArgList,
					    Cardinal*);
static void		DeleteChild(Widget);
static void		Destroy(Widget);
static void		HandleExpose(Widget, XEvent*, Region);
static XtGeometryResult GeometryManager(Widget, XtWidgetGeometry*,
					XtWidgetGeometry*);
static void		GetCellSizes(Widget, int, int, Dimension*, Dimension*);
static void		GetChildSize(Widget);
static void		GetPreferredSize(Widget,Dimension*,Dimension*);
static void		Initialize(Widget, Widget, ArgList, Cardinal*);
static void		InsertChild(Widget);
static int		ManagedChildCount(Widget);
static void		PlaceChildren(Widget, Dimension, Dimension);
static XtGeometryResult	QueryGeometry(Widget, XtWidgetGeometry*,
				      XtWidgetGeometry*);
static void		Refresh(Widget);
static void		Resize(Widget);
static Boolean		SetValues(Widget, Widget, Widget, ArgList, Cardinal*);
static void		SortChildren(Widget);
static Boolean		TrySize(Widget, int, int, Dimension*, Dimension*);

/********    End Static Function Declarations    ********/


/*  Resource definitions for ButtonBox
 */

static XmSyntheticResource syn_resources[] =
{
	{	XmNmarginWidth,
		sizeof (Dimension),
		XtOffsetOf( struct _ButtonBoxRec, bbox.marginWd),
		_XmFromHorizontalPixels,
		_XmToHorizontalPixels
	},

	{	XmNmarginHeight,
		sizeof (Dimension),
		XtOffsetOf( struct _ButtonBoxRec, bbox.marginHt),
		_XmFromVerticalPixels,
		_XmToVerticalPixels
	},
};


static XtResource resources[] =
{
	{	XmNmarginWidth,
		XmCMarginWidth, XmRHorizontalDimension, sizeof (Dimension),
		XtOffsetOf(ButtonBoxRec, bbox.marginWd),
		XmRImmediate, (XtPointer)3
	},

	{	XmNmarginHeight,
		XmCMarginHeight, XmRVerticalDimension, sizeof (Dimension),
		XtOffsetOf(ButtonBoxRec, bbox.marginHt),
		XmRImmediate, (XtPointer)3
	},

	{	XmNresizeCallback,
		XmCCallback, XmRCallback, sizeof (XtCallbackList),
		XtOffsetOf(ButtonBoxRec, bbox.resizeCallback),
		XmRImmediate, (XtPointer)NULL
	},

	{	XmNresizeWidth,
		XmCResizeWidth, XmRBoolean, sizeof(Boolean),
		XtOffsetOf(ButtonBoxRec, bbox.resizeWidth),
		XmRImmediate, (XtPointer)True
	},

	{	XmNresizeHeight,
		XmCResizeHeight, XmRBoolean, sizeof(Boolean),
		XtOffsetOf(ButtonBoxRec, bbox.resizeHeight),
		XmRImmediate, (XtPointer)True
	},

	{	BbNrowAlignment,
		BbCRowAlignment, XmRAlignment, sizeof(unsigned char),
		XtOffsetOf(ButtonBoxRec, bbox.rowAlignment),
		XmRImmediate, (XtPointer)XmALIGNMENT_BEGINNING
	},

	{	BbNcolAlignment,
		BbCColAlignment, XmRAlignment, sizeof(unsigned char),
		XtOffsetOf(ButtonBoxRec, bbox.colAlignment),
		XmRImmediate, (XtPointer)XmALIGNMENT_CENTER
	},

	{	XmNorientation,
		XmCOrientation, XmROrientation, sizeof(unsigned char),
		XtOffsetOf(ButtonBoxRec, bbox.orient),
		XmRImmediate, (XtPointer)XmHORIZONTAL
	},

	{	BbNrowSpacing,
		BbCRowSpacing, XmRDimension, sizeof(Dimension),
		XtOffsetOf(ButtonBoxRec, bbox.rowSpacing),
		XmRImmediate, (XtPointer)3
	},

	{	BbNcolSpacing,
		BbCColSpacing, XmRDimension, sizeof(Dimension),
		XtOffsetOf(ButtonBoxRec, bbox.colSpacing),
		XmRImmediate, (XtPointer)3
	},

	{	BbNuniformRows,
		BbCUniformRows, XmRBoolean, sizeof(Boolean),
		XtOffsetOf(ButtonBoxRec, bbox.uniformRows),
		XmRImmediate, (XtPointer)False
	},

	{	BbNuniformCols,
		BbCUniformCols, XmRBoolean, sizeof(Boolean),
		XtOffsetOf(ButtonBoxRec, bbox.uniformCols),
		XmRImmediate, (XtPointer)False
	},
};

static XtResource constraint_resources[] = {
    {
       XmNpositionIndex,
       XmCPositionIndex,
       XmRShort,
       sizeof(short),
       XtOffsetOf(ButtonBoxConstraintRec, bbox.posIndex),
       XmRImmediate,
       (XtPointer) XmLAST_POSITION
    },
};


/****************************************************************
 *
 * Full class record constant
 *
 ****************************************************************/

static XmBaseClassExtRec buttonBoxBaseClassExtRec = {
    NULL,				/* Next extension	*/
    NULLQUARK,				/* record type XmQmotif */
    XmBaseClassExtVersion,		/* version		*/
    sizeof(XmBaseClassExtRec),		/* size			*/
    NULL,				/* InitializePrehook	*/
    XmInheritSetValuesPrehook,		/* SetValuesPrehook	*/
    NULL,				/* InitializePosthook	*/
    XmInheritSetValuesPosthook,		/* SetValuesPosthook	*/
    XmInheritClass,			/* secondaryObjectClass	*/
    XmInheritSecObjectCreate,		/* secondaryCreate	*/
    XmInheritGetSecResData,		/* getSecRes data	*/
    { 0 },				/* fastSubclass flags	*/
    XmInheritGetValuesPrehook,		/* getValuesPrehook	*/
    XmInheritGetValuesPosthook,		/* getValuesPosthook	*/
    NULL,                               /* classPartInitPrehook */
    NULL,                               /* classPartInitPosthook*/
    NULL,                               /* ext_resources        */
    NULL,                               /* compiled_ext_resources*/
    0,                                  /* num_ext_resources    */
    FALSE,                              /* use_sub_resources    */
    NULL,				/* widgetNavigable      */
    XmInheritFocusChange		/* focusChange          */
};

externaldef(xmbuttonboxclassrec) ButtonBoxClassRec buttonBoxClassRec =
{
   {			/* core_class fields      */
      (WidgetClass) &xmManagerClassRec,		/* superclass         */
      "ButtonBox",				/* class_name         */
      sizeof(ButtonBoxRec),			/* widget_size        */
      ClassInitialize,	        		/* class_initialize   */
      ClassPartInitialize,			/* class_part_init    */
      FALSE,					/* class_inited       */
      Initialize,       			/* initialize         */
      NULL,					/* initialize_hook    */
      XtInheritRealize,				/* realize            */
      NULL,					/* actions	      */
      0,					/* num_actions	      */
      resources,				/* resources          */
      XtNumber(resources),			/* num_resources      */
      NULLQUARK,				/* xrm_class          */
      TRUE,					/* compress_motion    */
      XtExposeCompressMaximal | XtExposeGraphicsExposeMerged,
      						/* compress_exposure  */
      FALSE,					/* compress_enterlv   */
      FALSE,					/* visible_interest   */
      Destroy,			                /* destroy            */
      Resize,           			/* resize             */
      HandleExpose,	        		/* expose             */
      SetValues,                		/* set_values         */
      NULL,					/* set_values_hook    */
      XtInheritSetValuesAlmost,	        	/* set_values_almost  */
      NULL,					/* get_values_hook    */
      NULL,					/* accept_focus       */
      XtVersion,				/* version            */
      NULL,					/* callback_private   */
      NULL,					/* tm_table           */
      QueryGeometry,                    	/* query_geometry     */
      NULL,             	                /* display_accelerator*/
      (XtPointer)&buttonBoxBaseClassExtRec,	/* extension          */
   },
   {		/* composite_class fields */
      GeometryManager,    	                /* geometry_manager   */
      ChangeManaged,	                	/* change_managed     */
      InsertChild,				/* insert_child       */
      DeleteChild,		     		/* delete_child       */
      NULL,                                     /* extension          */
   },

   {		/* constraint_class fields */
      constraint_resources,			/* resource list        */
      XtNumber(constraint_resources),		/* num resources        */
      sizeof(ButtonBoxConstraintRec),		/* constraint size      */
      ConstraintInitialize,			/* init proc            */
      NULL,					/* destroy proc         */
      ConstraintSetValues,			/* set values proc      */
      NULL,                                     /* extension            */
   },

   {		/* manager_class fields */
      XtInheritTranslations,			/* translations           */
      syn_resources,				/* syn_resources      	  */
      XtNumber (syn_resources),			/* num_get_resources 	  */
      NULL,					/* syn_cont_resources     */
      0,					/* num_get_cont_resources */
      XmInheritParentProcess,                   /* parent_process         */
      NULL,					/* extension           */
   },

   {		/* ButtonBox class */
      (XtPointer) NULL,				/* extension pointer */
   }
};

externaldef(xmbuttonboxwidgetclass) WidgetClass buttonBoxWidgetClass
					= (WidgetClass)&buttonBoxClassRec;

extern int	debuglev;

/*---------------------------------------------------------------
 * Initialize class
 */

static void
ClassInitialize(
	void)
{
  buttonBoxBaseClassExtRec.record_type = XmQmotif;
}

/*---------------------------------------------------------------
 * Initialize class part
 */

static void
ClassPartInitialize(
        WidgetClass w_class)
{
/*   _XmFastSubclassInit(w_class, XmROW_COLUMN_BIT); */
   return;
}

/*---------------------------------------------------------------
 * Initialize widget
 */

static void
Initialize(
        Widget		wreq,
        Widget		wnew,
        ArgList		argv,
        Cardinal	*argc)
{
   if ( !XtWidth(wreq)  ) XtWidth(wnew)  = 16;
   if ( !XtHeight(wreq) ) XtHeight(wnew) = 16;

   if ( BB_rowAlignment(wreq) != XmALIGNMENT_BEGINNING &&
	BB_rowAlignment(wreq) != XmALIGNMENT_CENTER &&
	BB_rowAlignment(wreq) != XmALIGNMENT_END )
      BB_rowAlignment(wreq) = XmALIGNMENT_BEGINNING;

   if ( BB_colAlignment(wreq) != XmALIGNMENT_BEGINNING &&
	BB_colAlignment(wreq) != XmALIGNMENT_CENTER &&
	BB_colAlignment(wreq) != XmALIGNMENT_END )
      BB_colAlignment(wreq) = XmALIGNMENT_CENTER;

   if ( BB_orient(wreq) != XmVERTICAL &&
	BB_orient(wreq) != XmHORIZONTAL )
      BB_orient(wreq) = XmHORIZONTAL;

   BB_lastWd(wnew)      = 0;
   BB_lastHt(wnew)      = 0;
   BB_exposed(wnew)     = False;
   BB_resizeTimer(wnew) = 0;

} /* End Initialize */

/*---------------------------------------------------------------
 * Destroy widget
 */

static void
Destroy(
        Widget	bbw)
{
   if ( BB_resizeTimer(bbw) )
      XtRemoveTimeOut(BB_resizeTimer(bbw));

   XtRemoveAllCallbacks(bbw, XmNresizeCallback);

} /* End Destroy */

#if 0
/*---------------------------------------------------------------
 * Function to realize widget
 */

static void
Realize(
	Widget			bbw,
	XtValueMask		*mask,
	XSetWindowAttributes	*attr)
{
/*
 * Don't propagate events for row column widgets and set bit gravity to NW
 */
   (*mask) |= CWDontPropagate | CWBitGravity;
   attr->bit_gravity = NorthWestGravity;

   attr->do_not_propagate_mask = ButtonPressMask	|
   				 ButtonReleaseMask	|
				 KeyPressMask		|
				 KeyReleaseMask		|
				 PointerMotionMask;

   XtCreateWindow(bbw, InputOutput, CopyFromParent, *mask, attr);

} /* End Realize */
#endif

/*-----------------------------------------------------------------------
 * Function to add a child widget
 */

static void
InsertChild(
        Widget	w)
{
   Widget	bbw = XtParent(w);

/*
 * Use composite class insert proc to do all the dirty work
 */
   (*((XmManagerWidgetClass)xmManagerWidgetClass)->composite_class.insert_child)
   									(w);

/*
 * Set the correct posIndex values for everybody if the new child has been
 *    inserted in the list instead of put at the end.
 */
   if ( BB_posIndex(w) == XmLAST_POSITION || BB_posIndex(w) < 1 )
      BB_posIndex(w) = BB_numChildren(bbw);

#if 0
/*
 * For every child with a position >= to the new position, bump up the position
 */
   if ( BB_posIndex(w) < BB_numChildren(bbw) ) {

      Widget	*p    = BB_children(bbw);
      int	count = BB_numChildren(bbw);
      int	i;
      for (i=0; i<count; i++) {
	 if ( *p != w && BB_posIndex(*p) >= BB_posIndex(w) )
	    BB_posIndex(*p) = BB_posIndex(*p) + 1;
      }

      SortChildren(bbw);
   }
#endif

} /* End InsertChild */

/*-----------------------------------------------------------------------
 * Function to delete a child widget
 */

static void
DeleteChild(
        Widget	w)
{
   Widget bbw = XtParent(w);

/*
 * Use manager class delete proc to do all the dirty work
 */
   (*((XmManagerWidgetClass)xmManagerWidgetClass)->composite_class.delete_child)
   									(w);
/*
 * Set the correct positionIndex values for everybody if the child was not
 *    deleted from the end of the list.  XmManager delete_child has already
 *    decremented num_chidren!
 */
   if ( BB_posIndex(w) <= BB_numChildren(bbw) ) {

      int	i;
      int	count = BB_numChildren(bbw);
      Widget	*p    = BB_children(bbw);

      for (i=0; i<count; i++) {
	 BB_posIndex(*p) = i+1;
	 p++;
      }
   }

} /* End DeleteChild */

/*---------------------------------------------------------------
 * Handle change in children
 */

static void
ChangeManaged(
	Widget bbw)
{
   Boolean		anyChanged = False;
   Dimension		wd = 0;
   Dimension		ht = 0;
   int			i;

/*
 * Update the "was_managed" flag in the child constraint record and see if
 *    there are any changes.
 */
   for (i=0; i<BB_numChildren(bbw); i++) {

      Widget	w = BB_children(bbw)[i];
      if ( BB_wasManaged(w) != XtIsManaged(w) ) {
	 anyChanged = TRUE;
	 BB_wasManaged(w) = XtIsManaged(w);
      }
   }

   if ( !anyChanged ) return;

   SortChildren(bbw);

   if ( BB_exposed(bbw) )
      Refresh(bbw);

   _XmNavigChangeManaged(bbw);

} /* End ChangeManaged */

/*---------------------------------------------------------------
 * Put children in the order specified by their positionIndex resources
 */

static void
SortChildren(
	Widget bbw)
{
   int		i;
   short	minIndex, curIndex;
   int		count;
   int		wpos;
   Widget	*child;
   Boolean	sorted;
   WidgetList	wlist;

   if ( debuglev > 2 ) printf("ButtonBox(%s):SortChildren\n", XtName(bbw));

/*
 * See if any are out of place
 */
   count  = BB_numChildren(bbw);
   child  = BB_children(bbw);
   sorted = True;
   for (i=0; sorted && i<count; i++) {
      int	index = BB_posIndex(*child) - 1;
      if ( index != i ) sorted = False;
      child++;
   }

   if ( sorted ) return;

/*
 * Create a parallel array for the sorted list
 */
   wlist = (WidgetList)XtMalloc(count*sizeof(Widget));
   wpos = 0;

/*
 * Loop until the list is sorted
 */
   curIndex = -1;
   while ( !sorted ) {

/*
 * Find the minimum index greater than the current index
 */
      child    = BB_children(bbw);
      minIndex = 9999;
      for (i=0; i<count; i++) {
	 int	index = BB_posIndex(*child) - 1;
	 if ( index > curIndex && index < minIndex ) minIndex = index;
	 child++;
      }

      if ( minIndex < 9999 ) {
/*
 * Copy all widgets equal to the min index
 */
	 child = BB_children(bbw);
	 for (i=0; i<count; i++) {
	    int	index = BB_posIndex(*child) - 1;
	    if ( index == minIndex ) wlist[wpos++] = *child;
	    child++;
	 }

	 curIndex = minIndex;
      }

      else
	 sorted = True;

   } /* End while not sorted */

/*
 * Copy new list to original and update position index
 */
   child = BB_children(bbw);
   for (i=0; i<count; i++) {
      *child = wlist[i];
      BB_posIndex(*child) = i+1;
      child++;
   }

/*
 * Return allocated memory
 */
   XtFree((char*)wlist);

} /* End SortChildren */

/*---------------------------------------------------------------
 * Recalculate child positions
 */

static void
Refresh(
	Widget bbw)
{
   Dimension	wd, ht;

   if ( debuglev > 2 ) printf("ButtonBox(%s):Refresh\n", XtName(bbw));

/*
 * Calculate our preferred size
 */
   GetPreferredSize(bbw, &wd, &ht);

/*
 * See if there is a size change
 */
   if ( wd != XtWidth(bbw) || ht != XtHeight(bbw) ) {

      XtWidgetGeometry	desired;
      XtWidgetGeometry	allowed;
      XtGeometryResult	answer;

      desired.width  = wd;
      desired.height = ht;
      desired.request_mode = CWWidth | CWHeight;

      answer = XtMakeGeometryRequest(bbw, &desired, &allowed);

/*
 * On an Almost, accept the returned value and make a second request to get
 *    a Yes returned.
 */
      if ( answer == XtGeometryAlmost ) {
	 desired = allowed;
	 answer = XtMakeGeometryRequest(bbw, &desired, &allowed);
      }

      wd = desired.width;
      ht = desired.height;

   } /* End if there is a size change */

/*
 * Now place the widgets
 */
   PlaceChildren(bbw, wd, ht);

} /* End Refresh */

/*---------------------------------------------------------------
 * Count the number of managed children
 */

static int
ManagedChildCount(
	Widget		bbw)
{
   int		i;
   int		count = 0;
   Widget	*p    = BB_children(bbw);
   for (i=0; i<BB_numChildren(bbw); i++) {
      if ( XtIsManaged(*p) ) count++;
      p++;
   }

   return count;

} /* End ManagedChildCount */

/*---------------------------------------------------------------
 * Figure out what size we'd like to be
 */

static void
GetPreferredSize(
	Widget		bbw,
	Dimension	*wd,
	Dimension	*ht)
{
   int			rows, cols;
   Boolean		found = False;
   int			count = ManagedChildCount(bbw);

   if ( debuglev > 2 ) printf("ButtonBox(%s):GetPreferredSize\n", XtName(bbw));

/*
 * If we can't be resized, do nothing.
 */
   if ( !BB_resizeWidth(bbw) && !BB_resizeHeight(bbw) ) {
      *wd = XtWidth(bbw);
      *ht = XtHeight(bbw);
   }

/*
 * If we're completely resizable, use one row or column
 */
   else if ( BB_resizeWidth(bbw) && BB_resizeHeight(bbw) ) {

      *wd = 9999;
      *ht = 9999;

      if ( BB_orient(bbw) == XmHORIZONTAL )
	 TrySize(bbw, 1, count, wd, ht);
      else
	 TrySize(bbw, count, 1, wd, ht);
   }

/*
 * If the width is fixed, increase the number of rows until we have a fit
 */
   else if ( !BB_resizeWidth(bbw) ) {

      *wd = XtWidth(bbw);
      *ht = 9999;

/*
 * Increase the number of rows until we have a fit
 */
      for (rows=1; !found && rows <= count; ) {
	 cols = count / rows;
	 if ( count % rows > 0 ) cols++;
	 found = TrySize(bbw, rows, cols, wd, ht);
	 if ( !found ) rows++;
      }

      if ( !found )
	 TrySize(bbw, count, 1, wd, ht);

   } /* End if width if fixed */

   else { /* height fixed */

      *wd = 9999;
      *ht = XtHeight(bbw);

/*
 * Increase the number of columns until we have a fit
 */
      for (cols=1; !found && cols <= count; ) {
	 rows = count / cols;
	 if ( count % cols > 0 ) rows++;
	 found = TrySize(bbw, rows, cols, wd, ht);
	 if ( !found ) cols++;
      }

      if ( !found )
	 TrySize(bbw, 1, count, wd, ht);

   } /* End if height is fixed */

   if ( debuglev > 2 ) printf("   preferred size is %d by %d\n", *wd, *ht);
   return;

} /* End GetPreferredSize */

/*-----------------------------------------------------------------------
 * Function to see if a given number of rows and columns will fit
 */

static Boolean
TrySize(
	Widget		bbw,
	int		rows,
	int		cols,
	Dimension	*wd,
	Dimension	*ht)
{
   Dimension	needWd, needHt;
   int		i, r, c;
   Boolean	wdFits, htFits;
   int		count;
   Dimension	*rowSizes, *colSizes;

   if ( debuglev > 2 ) printf("   trying %d rows and %d cols\n", rows, cols);

/*
 * Calculate the size of each row and column
 */
   rowSizes = (Dimension*)XtMalloc(rows*sizeof(Dimension));
   colSizes = (Dimension*)XtMalloc(cols*sizeof(Dimension));

   GetCellSizes(bbw, rows, cols, rowSizes, colSizes);

/*
 * Now that we know the sizes of all rows and columns, see if they will fit
 *    in the available space
 */
   needWd = (Dimension)((BB_marginWd(bbw)*2) + (BB_colSpacing(bbw) * (cols-1)));
   needHt = (Dimension)((BB_marginHt(bbw)*2) + (BB_rowSpacing(bbw) * (rows-1)));
   for (c=0; c<cols; c++) needWd += (Dimension)colSizes[c];
   for (r=0; r<rows; r++) needHt += (Dimension)rowSizes[r];

   wdFits = (BB_resizeWidth(bbw)  || needWd <= *wd);
   htFits = (BB_resizeHeight(bbw) || needHt <= *ht);

/*
 * If we've got a fit, apply it.
 */
   if ( wdFits && BB_resizeWidth(bbw)  ) *wd = needWd;
   if ( htFits && BB_resizeHeight(bbw) ) *ht = needHt;

/*
 * Return allocated memory
 */
   XtFree((char*)rowSizes);
   XtFree((char*)colSizes);

   return (wdFits && htFits);

} /* End TrySize */

/*---------------------------------------------------------------
 * Get the sizes need for each cell using the specified number of rows and
 *    columns
 */

static void
GetCellSizes(
	Widget		bbw,
	int		rows,
	int		cols,
	Dimension	*rowSizes,
	Dimension	*colSizes)
{
   int	r, c, i, count;
   int	num;

   for (r=0; r<rows; r++) rowSizes[r] = 0;
   for (c=0; c<cols; c++) colSizes[c] = 0;

/*
 * Loop through widgets
 */
   count = BB_numChildren(bbw);
   num   = 0;
   for (i=0; i<count; i++) {

      Dimension		*rowSize, *colSize;
      int		index;
      Widget		w = BB_children(bbw)[i];

      if ( !XtIsManaged(w) ) continue;

/*
 * Calculate row and column number
 */
      if ( BB_orient(bbw) == XmHORIZONTAL ) {
	 r = num / cols;
	 c = num % cols;
      }
      else {
	 r = num % rows;
	 c = num / rows;
      }

      rowSize = &rowSizes[r];
      colSize = &colSizes[c];

/*
 * Compare widget size to known size
 */
      GetChildSize(w);
      if ( BB_prefWd(w) > *colSize ) *colSize = BB_prefWd(w);
      if ( BB_prefHt(w) > *rowSize ) *rowSize = BB_prefHt(w);

      num++;

   } /* End for each child */

/*
 *  Make rows the same size if necessary
 */
   if ( BB_uniformRows(bbw) ) {

/*
 * Find maximum row size
 */
      Dimension	maxSize = 0;
      Dimension	*rowSize = rowSizes;
      for (r=0; r<rows; r++, rowSize++) {
	 if ( *rowSize > maxSize ) maxSize = *rowSize;
      }

/*
 * Set all row sizes to the max
 */
      rowSize = rowSizes;
      for (r=0; r<rows; r++, rowSize++) *rowSize = maxSize;

   } /* End if we need uniform rows */

/*
 *  Make columns the same size if necessary
 */
   if ( BB_uniformCols(bbw) ) {

/*
 * Find maximum column size
 */
      Dimension	maxSize = 0;
      Dimension	*colSize = colSizes;
      for (c=0; c<cols; c++, colSize++) {
	 if ( *colSize > maxSize ) maxSize = *colSize;
      }

/*
 * Set all column sizes to the max
 */
      colSize = colSizes;
      for (c=0; c<cols; c++, colSize++) *colSize = maxSize;

   } /* End if we need uniform cols */

} /* End GetCellSizes */

/*---------------------------------------------------------------
 * Ask what size a child would like to be
 */

static void
GetChildSize(
	Widget			w)
{
   XtWidgetGeometry	pref;

   XtQueryGeometry(w, NULL, &pref);

   BB_prefWd(w) = (pref.request_mode & CWWidth ) ? pref.width  : XtWidth(w);
   BB_prefHt(w) = (pref.request_mode & CWHeight) ? pref.height : XtHeight(w);

} /* End GetChildSize */

/*-----------------------------------------------------------------------
 * Function to set the positions and sizes of our children
 */

static void
PlaceChildren(
	Widget		bbw,
	Dimension	wd,
	Dimension	ht)
{
   int		i;
   int		rows = 1, cols = 1;
   int		xoff, yoff;
   int		r, c;
   int		num;
   Dimension	usedWd, usedHt;
   Dimension	*rowSizes, *colSizes;
   int		count = ManagedChildCount(bbw);
   Boolean	found = False;

   if ( debuglev > 2 )
      printf("ButtonBox(%s):PlaceChildren with size %d by %d\n",
	      XtName(bbw), wd, ht);

   if ( BB_orient(bbw) == XmHORIZONTAL ) {

/*
 * Increase the number of rows until we have a fit
 */
      for (rows=1; !found && rows <= count; ) {
	 cols = count / rows;
	 if ( count % rows > 0 ) cols++;
	 found = TrySize(bbw, rows, cols, &wd, &ht);
	 if ( !found ) rows++;
      }

      if ( !found ) {
	 rows = 1;
	 cols = count;
      }

   } /* End if horizontal */

   else { /* vertical */

/*
 * Increase the number of columns until we have a fit
 */
      for (cols=1; !found && cols <= count; ) {
	 rows = count / cols;
	 if ( count % cols > 0 ) rows++;
	 found = TrySize(bbw, rows, cols, &wd, &ht);
	 if ( !found ) cols++;
      }

      if ( !found ) {
	 rows = count;
	 cols = 1;
      }

   } /* End if height is fixed */

/*
 * Calculate the size of each row and column
 */
   rowSizes = (Dimension*)XtMalloc(rows*sizeof(Dimension));
   colSizes = (Dimension*)XtMalloc(cols*sizeof(Dimension));

   GetCellSizes(bbw, rows, cols, rowSizes, colSizes);

/*
 * Calculate the space used by the buttons
 */
   usedWd = BB_colSpacing(bbw) * (cols-1);
   usedHt = BB_rowSpacing(bbw) * (rows-1);
   for (c=0; c<cols; c++) usedWd += colSizes[c];
   for (r=0; r<rows; r++) usedHt += rowSizes[r];

/*
 * Calculate the left and top offset required to get the desired alignment
 */
   if ( BB_colAlignment(bbw) == XmALIGNMENT_BEGINNING )
      xoff = BB_marginWd(bbw);
   else if ( BB_colAlignment(bbw) == XmALIGNMENT_END )
      xoff = wd - usedWd - BB_marginWd(bbw);
   else /* Centered */
      xoff = (int)(wd - usedWd) / (int)2;

   if ( BB_rowAlignment(bbw) == XmALIGNMENT_BEGINNING )
      yoff = BB_marginHt(bbw);
   else if ( BB_rowAlignment(bbw) == XmALIGNMENT_END )
      yoff = ht - usedHt - BB_marginHt(bbw);
   else /* Centered */
      yoff = (int)(ht - usedHt) / (int)2;

/*
 * Loop through widgets
 */
   count = BB_numChildren(bbw);
   num = 0;
   for (i=0; i<count; i++) {

      int		row, col;
      int		r, c;
      Dimension		*rowSize, *colSize;
      Position		x, y;
      Widget		w;

      w = BB_children(bbw)[i];
      if ( !XtIsManaged(w) ) continue;

/*
 * Calculate row and column number
 */
      if ( BB_orient(bbw) == XmHORIZONTAL ) {
	 col = num % cols;
	 row = num / cols;
      }
      else {
	 row = num % rows;
	 col = num / rows;
      }

/*
 * Calculate x position of child
 */
      x = xoff;
      for (c=0; c<col; c++) {
	 x += colSizes[c];
	 x += BB_colSpacing(bbw);
      }

/*
 * Calculate y position of child
 */
      y = yoff;
      for (r=0; r<row; r++) {
	 y += rowSizes[r];
	 y += BB_rowSpacing(bbw);
      }

/*
 * Set child geometry
 */
      XmDropSiteStartUpdate(w);
      XtConfigureWidget(w, x, y, colSizes[col], rowSizes[row],
      			XtBorderWidth(w));
      XmDropSiteEndUpdate(w);

      num++;

   } /* End for each child */

   BB_lastWd(bbw) = wd;
   BB_lastHt(bbw) = ht;

/*
 * Return allocated memory
 */
   XtFree((char*)rowSizes);
   XtFree((char*)colSizes);

} /* End PlaceChildren */

/*-----------------------------------------------------------------------
 * Function to initialize child constraint settings
 */

static void
ConstraintInitialize(
        Widget		wreq,
        Widget		wnew,
        ArgList		argv,
        Cardinal	*argc)
{
   BB_wasManaged(wnew) = False;
   BB_prefWd(wnew)     = 10;
   BB_prefHt(wnew)     = 10;
   BB_acceptGeo(wnew)  = False;
}

/*-----------------------------------------------------------------------
 * Function to modify child constraint settings
 */

static Boolean
ConstraintSetValues(
	Widget		wold,
	Widget		wreq,
	Widget		wnew,
	ArgList		argv,
	Cardinal	*argc)
{
   int	oldIndex = BB_posIndex(wold);
   int	newIndex = BB_posIndex(wnew);
   if ( oldIndex != newIndex ) {

      int	inc, i;
      Widget	tmp;
      Widget	bbw = XtParent(wnew);

      if ( newIndex == XmLAST_POSITION || newIndex < 1 ||
	   newIndex >= BB_numChildren(bbw) ) {
	 newIndex = BB_numChildren(bbw);
      }

      oldIndex--;
      newIndex--;

/*
 *  Move the widget from its old position to its new one.  Shift all other
 *     widgets accordingly
 */
      tmp = BB_children(bbw)[oldIndex];
      inc = (newIndex < oldIndex) ? -1 : 1;

      for (i = oldIndex; i != newIndex; i += inc) {
	 Widget	cnew = BB_children(bbw)[i+inc];
	 BB_children(bbw)[i] = cnew;
	 BB_posIndex(cnew) = i+1;
      }

      BB_children(bbw)[newIndex] = tmp;
      BB_posIndex(wnew)          = newIndex+1;

      if ( XtIsManaged(wnew) ) {

/*
 * Save geometry in case it changes
 */
	 XtWidgetGeometry	geo;
	 geo.x      = XtX(wnew);
	 geo.y      = XtY(wnew);
	 geo.width  = XtWidth(wnew);
	 geo.height = XtHeight(wnew);

/*
 * Update the display
 */
	 if ( BB_exposed(bbw) )
	    Refresh(bbw);

/*
 * Request a redisplay if the geometry has changed
 */
	 if ( geo.x      != XtX(wnew)     ||
	      geo.y      != XtY(wnew)     ||
	      geo.width  != XtWidth(wnew) ||
	      geo.height != XtHeight(wnew) ) {
	    BB_acceptGeo(wnew) = True;
	    return True;
	 }

      } /* End if child is managed */

   } /* End if position index has changed */

   return False; /* Don't need redisplay */

} /* End ConstraintSetValues */

/*---------------------------------------------------------------
 * Handle child geometry requests
 */

static XtGeometryResult
GeometryManager(
        Widget			w,
        XtWidgetGeometry	*req,
        XtWidgetGeometry	*reply)
{
   Widget	bbw = XtParent(w);

/*
 * Accept the size change
 */
   if ( (req->request_mode & CWX) && (req->x >= 0) ) w->core.x      = req->x;
   if ( (req->request_mode & CWY) && (req->y >= 0) ) w->core.y      = req->y;
   if ( (req->request_mode & CWWidth)  && (req->width  > 0) )
						   w->core.width  = req->width;
   if ( (req->request_mode & CWHeight) && (req->height > 0) )
						   w->core.height = req->height;

/*
 * First treat the special case resulting from a change in positionIndex
 */
   if ( BB_acceptGeo(w) ) { /* set in ConstraintSetValues */

      BB_acceptGeo(w) = False;
      return XtGeometryYes;
   }

/*
 * Calculate the positions and sizes again
 */
   if ( BB_exposed(bbw) )
      Refresh(bbw);

   return XtGeometryYes;

} /* End GeometryManager */

/*---------------------------------------------------------------
 * Someone wants to know how big we want to be.
 */

static XtGeometryResult
QueryGeometry(
        Widget			bbw,
        XtWidgetGeometry	*ask,
        XtWidgetGeometry	*answer)
{
   Dimension	wd, ht;
   Boolean	acceptWd, acceptHt;

   GetPreferredSize(bbw, &wd, &ht);
   answer->width  = wd;
   answer->height = ht;

   answer->request_mode = 0;
   if ( BB_resizeWidth(bbw)  ) answer->request_mode |= CWWidth;
   if ( BB_resizeHeight(bbw) ) answer->request_mode |= CWHeight;

   if ( ask->request_mode & CWWidth  )
      acceptWd = (ask->width == answer->width);
   else
      acceptWd = True;

   if ( ask->request_mode & CWHeight )
      acceptHt = (ask->height == answer->height);
   else
      acceptHt = True;

   if ( acceptWd && acceptHt )
      return XtGeometryYes;

   if ( (!acceptWd && !acceptHt) ||
        (answer->width == XtWidth(bbw) && answer->height == XtWidth(bbw)) )
      return XtGeometryNo;

   return XtGeometryAlmost;

} /* End QueryGeometry */

/*---------------------------------------------------------------
 * Process the first exposure
 */

static void
HandleExpose(
        Widget bbw,
        XEvent *ev,
        Region reg)
{
   if ( !BB_exposed(bbw) ) {
      Refresh(bbw);
      BB_exposed(bbw) = True;
   }
}

/*---------------------------------------------------------------
 * Do resize in a timer proc so that we can adjust our non-fixed axis
 */

static void
FinishResize(
        XtPointer data,
        XtIntervalId *id)
{
   ButtonBoxCallbackStruct	cb;
   Widget			bbw = (Widget)data;

   BB_resizeTimer(bbw) = 0;
   Refresh(bbw);

   cb.reason = XmCR_RESIZE;
   cb.event  = NULL;
   cb.window = XtWindow(bbw);

   XtCallCallbackList(bbw, BB_resizeCallback(bbw), &cb);

   return;
}

/*---------------------------------------------------------------
 * Our size has changed
 */

static void
Resize(
        Widget bbw)
{
   if ( debuglev > 2 ) printf("ButtonBox(%s):Resize\n", XtName(bbw));

/*
 * Handle the resize in a timer proc so we can adjust our non-constrained
 *    axis.
 */
   if ( (XtWidth(bbw) != BB_lastWd(bbw) || XtHeight(bbw) != BB_lastHt(bbw)) &&
	!BB_resizeTimer(bbw) )
      BB_resizeTimer(bbw) = XtAppAddTimeOut(XtWidgetToApplicationContext(bbw),
					    0, FinishResize, (XtPointer)bbw);

} /* End Resize */

/*---------------------------------------------------------------
 * Check for changes.  Ask for expose event if necessary.
 */

static Boolean
SetValues(
        Widget		wold,	/* Values before call */
        Widget		wreq,	/* Values user requested */
        Widget		wnew,	/* Values after base class processing */
        ArgList		argv,
        Cardinal	*argc)
{
   Boolean	redraw;

   if ( XtWidth (wreq) == 0 ) XtWidth(wnew)  = XtWidth(wold);
   if ( XtHeight(wreq) == 0 ) XtHeight(wnew) = XtHeight(wold);

/*
 * If the size has changed, call the Resize function.  Any other changes
 *    will get picked up at this time.
 */
   if ( XtWidth(wnew) != XtWidth(wold) ||
        XtWidth(wnew) != XtWidth(wold) ) {

      Resize(wnew);
      return False;
   }

/*
 * If there is any other type of change, ask for a redisplay.
 */
   redraw = (BB_marginWd(wnew)     != BB_marginWd(wold)		||
	     BB_marginHt(wnew)     != BB_marginWd(wold)		||
	     BB_resizeWidth(wnew)  != BB_resizeWidth(wold)	||
	     BB_resizeHeight(wnew) != BB_resizeHeight(wold)	||
	     BB_rowAlignment(wnew) != BB_rowAlignment(wold)	||
	     BB_colAlignment(wnew) != BB_colAlignment(wold)	||
	     BB_orient(wnew)       != BB_orient(wold)		||
	     BB_uniformRows(wnew)  != BB_uniformRows(wold)	||
	     BB_uniformCols(wnew)  != BB_uniformCols(wold)	||
	     BB_rowSpacing(wnew)   != BB_rowSpacing(wold)	||
	     BB_colSpacing(wnew)   != BB_colSpacing(wold));

   if ( redraw )
      BB_exposed(wnew) = False;	/* So HandleExpose will process this one */

   return redraw;

} /* End SetValues */

/*---------------------------------------------------------------
 * Convenience function to create a ButtonBox widget
 */

Widget
CreateButtonBox(
        Widget		par,
        String		name,
        ArgList		argv,
        Cardinal	argc)
{
    return XtCreateWidget(name, buttonBoxWidgetClass, par, argv, argc);
}
