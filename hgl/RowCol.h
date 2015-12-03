/*
 * $Id: RowCol.h,v 1.2 2000/04/30 14:07:08 fnevgeny Exp $
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

#ifndef _RowCol_h_
#define _RowCol_h_

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Class record constants */

externalref WidgetClass rowColWidgetClass;

typedef struct _RowColClassRec * RowColWidgetClass;
typedef struct _RowColRec      * RowColWidget;


#ifndef IsRowCol
#define IsRowCol(w)  (XtIsSubclass (w, rowColWidgetClass))
#endif

#define RcNwidthResizePolicy	"widthResizePolicy"
#define RcCWidthResizePolicy	"WidthResizePolicy"
#define RcNheightResizePolicy	"heightResizePolicy"
#define RcCHeightResizePolicy	"HeightResizePolicy"
#define RcNrowAlignment		"rowAlignment"
#define RcCRowAlignment		"RowAlignment"
#define RcNcolAlignment		"colAlignment"
#define RcCColAlignment		"ColAlignment"
#define RcNrowHeightAdjust	"rowHeightAdjust"
#define RcCRowHeightAdjust	"RowHeightAdjust"
#define RcNcolWidthAdjust	"colWidthAdjust"
#define RcCColWidthAdjust	"ColWidthAdjust"
#define RcNallowRowResize	"allowRowResize"
#define RcCAllowRowResize	"AllowRowResize"
#define RcNallowColResize	"allowColResize"
#define RcCAllowColResize	"AllowColResize"
#define RcNrowCount		"rowCount"
#define RcCRowCount		"RowCount"
#define RcNcolCount		"colCount"
#define RcCColCount		"ColCount"
#define RcNuniformRows		"uniformRows"
#define RcCUniformRows		"UniformRows"
#define RcNuniformCols		"uniformCols"
#define RcCUniformCols		"UniformCols"
#define RcNrowSpacing		"rowSpacing"
#define RcCRowSpacing		"RowSpacing"
#define RcNcolSpacing		"colSpacing"
#define RcCColSpacing		"ColSpacing"
#define RcNgeometryCallback	"geometryCallback"
#define RcNchildGeometryCallback	"childGeometryCallback"

#define RcRAdjust		"Adjust"
#define RcROrient		"Orientation"

typedef unsigned char	RcAlignT;
typedef unsigned char	RcResizeT;

typedef enum {
   RcADJUST_NONE,
   RcADJUST_EQUAL,
   RcADJUST_ATTACH
} RcAdjustT;

typedef enum {
   RcROW_MAJOR,
   RcCOL_MAJOR
} RcOrientT;

/********    Public Function Declarations    ********/

extern Widget CreateRowCol( 
                        Widget p,
                        String name,
                        ArgList args,
                        Cardinal n) ;

/********    End Public Function Declarations    ********/


typedef struct
{
   int		reason;
   XEvent	*event;
   Window	window;

} RowColCallbackStruct;

typedef struct
{
   Widget	widget;
   Boolean	needWd;
   Boolean	needHt;
   Dimension	desiredWd;
   Dimension	desiredHt;

} RowColGeometryCallbackStruct;

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _RowCol_h_ */
/* DON'T ADD ANYTHING AFTER THIS #endif */
