/*
 *  $Id: ButtonBoxP.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
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
#ifndef _ButtonBoxP_h_
#define _ButtonBoxP_h_

#include "ButtonBox.h"
#include <Xm/ManagerP.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  Data specific to the ButtonBox widget
 */

typedef struct _ButtonBoxPart {

/*
 * Data
 */

   Dimension		lastWd;
   Dimension		lastHt;
   Boolean		exposed;
   XtIntervalId		resizeTimer;

/*
 * User settable resources
 */

   XtCallbackList	resizeCallback;
   Dimension		marginWd;
   Dimension		marginHt;
   Boolean		resizeWidth;
   Boolean		resizeHeight;
   unsigned char	rowAlignment;
   unsigned char	colAlignment;
   unsigned char	orient;
   Dimension		rowSpacing;
   Dimension		colSpacing;
   Boolean		uniformRows;
   Boolean		uniformCols;

} ButtonBoxPart;

/*
 * Full widget record for ButtonBox
 */

typedef struct _ButtonBoxRec {

   CorePart		core;
   CompositePart	composite;
   ConstraintPart	constraint;
   XmManagerPart	manager;
   ButtonBoxPart	bbox;

} ButtonBoxRec;


/*
 * data specific to the ButtonBox class
 */

typedef struct _ButtonBoxClassPart {

   XtPointer extension;   /* Pointer to extension record */

} ButtonBoxClassPart;


/*
 * Full class record for ButtonBox
 */

typedef struct _ButtonBoxClassRec {

   CoreClassPart	core_class;
   CompositeClassPart	composite_class;
   ConstraintClassPart	constraint_class;
   XmManagerClassPart	manager_class;
   ButtonBoxClassPart	bbox_class;

} ButtonBoxClassRec;

externalref ButtonBoxClassRec buttonBoxClassRec;

/*
 * data specific to ButtonBox children
 */

typedef struct _ButtonBoxConstraintPart {

   short	posIndex;
   Boolean	wasManaged;
   Dimension	prefWd;
   Dimension	prefHt;
   Boolean	acceptGeo;	/* Used to force acceptance */

} ButtonBoxConstraintPart;

/*
 * Full constraint record for ButtonBox child
 */

typedef struct _ButtonBoxConstraintRec {

   XmManagerConstraintPart	manager;
   ButtonBoxConstraintPart	bbox;

} ButtonBoxConstraintRec;

/********    Private Function Declarations    ********/

#define BB_Part(w)		(((ButtonBoxWidget)(w))->bbox)

#define BB_lastWd(w)		BB_Part(w).lastWd
#define BB_lastHt(w)		BB_Part(w).lastHt
#define BB_exposed(w)		BB_Part(w).exposed
#define BB_resizeTimer(w)	BB_Part(w).resizeTimer
#define BB_marginWd(w)		BB_Part(w).marginWd
#define BB_marginHt(w)		BB_Part(w).marginHt
#define BB_resizeCallback(w)	BB_Part(w).resizeCallback
#define BB_resizeWidth(w)	BB_Part(w).resizeWidth
#define BB_resizeHeight(w)	BB_Part(w).resizeHeight
#define BB_rowAlignment(w)	BB_Part(w).rowAlignment
#define BB_colAlignment(w)	BB_Part(w).colAlignment
#define BB_orient(w)		BB_Part(w).orient
#define BB_rowSpacing(w)	BB_Part(w).rowSpacing
#define BB_colSpacing(w)	BB_Part(w).colSpacing
#define BB_uniformRows(w)	BB_Part(w).uniformRows
#define BB_uniformCols(w)	BB_Part(w).uniformCols

#define BB_ConstraintPart(w) \
	(((ButtonBoxConstraintRec*)((w)->core.constraints))->bbox)

#define BB_posIndex(w)		BB_ConstraintPart(w).posIndex
#define BB_wasManaged(w)	BB_ConstraintPart(w).wasManaged
#define BB_prefWd(w)		BB_ConstraintPart(w).prefWd
#define BB_prefHt(w)		BB_ConstraintPart(w).prefHt
#define BB_acceptGeo(w)		BB_ConstraintPart(w).acceptGeo

#define BB_CompositePart(w)	(((ButtonBoxWidget)(w))->composite)

#define BB_numChildren(w)	BB_CompositePart(w).num_children
#define BB_children(w)		BB_CompositePart(w).children

/********    End Private Function Declarations    ********/

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _ButtonBoxP_h_ */
