/*
 * $Id: RowColP.h,v 1.2 2000/04/30 14:07:08 fnevgeny Exp $
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

#ifndef _RowColP_h_
#define _RowColP_h_

#include <Xm/ManagerP.h>
#include "RowCol.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Constraint part record for RowCol widget */

typedef struct _RowColConstraintPart
{
   Dimension	prefWd;		/* Preferred width */
   Dimension	prefHt;		/* Preferred height */

} RowColConstraintPart;

typedef struct _RowColConstraintRec
{
   XmManagerConstraintPart	manager;
   RowColConstraintPart		rowcol;

} RowColConstraintRec;

/*  New fields for the RowCol widget class record  */

typedef struct
{
   XtPointer extension;   /* Pointer to extension record */
} RowColClassPart;


/* Full class record declaration */

typedef struct _RowColClassRec
{
	CoreClassPart		core_class;
	CompositeClassPart	composite_class;
	ConstraintClassPart	constraint_class;
	XmManagerClassPart	manager_class;
	RowColClassPart		rowcol_class;
} RowColClassRec;

externalref RowColClassRec rowColClassRec;


/* New fields for the RowCol widget record */

typedef struct
{
   Dimension		marginWd;
   Dimension		marginHt;

   XtCallbackList	resizeCallback;
   XtCallbackList	exposeCallback;
   XtCallbackList	inputCallback;
   XtCallbackList	geometryCallback;
   XtCallbackList	childGeometryCallback;

   RcResizeT		wResizePolicy;
   RcResizeT		hResizePolicy;
   RcAlignT		rowAlignment;
   RcAlignT		colAlignment;
   RcAdjustT		rowAdjust;
   RcAdjustT		colAdjust;
   RcOrientT		orient;
   Boolean		rowResizeOk;
   Boolean		colResizeOk;
   Boolean		uniformRows;
   Boolean		uniformCols;
   int			rowCount;
   int			colCount;
   Dimension		rowSpacing;
   Dimension		colSpacing;

   Boolean		inResize;
#if 0
   Dimension		width;
   Dimension		height;
#endif
   Dimension		setWidth;	/* Width  requested with SetValues */
   Dimension		setHeight;	/* Height requested with SetValues */
   Boolean		needPrefSizes;

} RowColPart;


/****************************************************************
 *
 * Full instance record declaration
 *
 ****************************************************************/

typedef struct _RowColRec
{
   CorePart		core;
   CompositePart	composite;
   ConstraintPart	constraint;
   XmManagerPart	manager;
   RowColPart		rowcol;

} RowColRec;



/********    Private Function Declarations    ********/

extern void _RowColInput( 
                        Widget wid,
                        XEvent *event,
                        String *params,
                        Cardinal *num_params) ;

#define GetRowColConstraint(w) \
        (&((RowColConstraintRec*)(w)->core.constraints)->rowcol)
#define GetRowColPart(w) (&((RowColWidget)(w))->rowcol)

/********    End Private Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _RowColP_h_ */
