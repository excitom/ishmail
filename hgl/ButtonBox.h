/*
 *  $Id: ButtonBox.h,v 1.2 2000/04/30 14:07:08 fnevgeny Exp $
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
#ifndef _ButtonBox_h_
#define _ButtonBox_h_

#include <Xm/Xm.h>

#ifdef __cplusplus
extern "C" {
#endif

/*  Class record constants */

externalref WidgetClass buttonBoxWidgetClass;

typedef struct _ButtonBoxClassRec * ButtonBoxWidgetClass;
typedef struct _ButtonBoxRec      * ButtonBoxWidget;


#ifndef IsButtonBox
#define IsButtonBox(w)  (XtIsSubclass (w, ButtonBoxWidgetClass))
#endif

#define BbNrowAlignment		"rowAlignment"
#define BbCRowAlignment		"RowAlignment"
#define BbNcolAlignment		"colAlignment"
#define BbCColAlignment		"ColAlignment"
#define BbNrowSpacing		"rowSpacing"
#define BbCRowSpacing		"RowSpacing"
#define BbNcolSpacing		"colSpacing"
#define BbCColSpacing		"ColSpacing"
#define BbNuniformRows		"uniformRows"
#define BbCUniformRows		"UniformRows"
#define BbNuniformCols		"uniformCols"
#define BbCUniformCols		"UniformCols"

/********    Public Function Declarations    ********/
extern Widget CreateButtonBox(
                        Widget p,
                        String name,
                        ArgList args,
                        Cardinal n
);

/********    End Public Function Declarations    ********/

typedef struct
{
   int		reason;
   XEvent	*event;
   Window	window;

} ButtonBoxCallbackStruct;

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _ButtonBox_h_ */
/* DON'T ADD ANYTHING AFTER THIS #endif */
