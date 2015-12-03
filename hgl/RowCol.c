/*
 * $Id: RowCol.c,v 1.4 2000/05/07 12:26:11 fnevgeny Exp $
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

#include "RowColP.h"
#include <Xm/TransltnsP.h>

/* Missing in Motif-2.* declarations */
#ifndef HAVE__XMINPUTFORGADGET_DECL
XmGadget _XmInputForGadget(Widget cw, int x, int y);
#endif
#ifndef HAVE__XMGETFOCUSPOLICY_DECL
unsigned char _XmGetFocusPolicy(Widget w);
#endif
#ifndef HAVE__XMSHELLISEXCLUSIVE_DECL
Boolean _XmShellIsExclusive(Widget wid);
#endif
/* This is not declared in 1.2 either */
XmNavigability _XmGetNavigability(Widget w);

#define	MARGIN_DEFAULT		10

#define defaultTranslations	_XmDrawingA_defaultTranslations
#define traversalTranslations	_XmDrawingA_traversalTranslations

/********    Static Function Declarations    ********/

static void		ChangeManaged(Widget wid);
static void		ClassInitialize(void);
static void		ClassPartInitialize(WidgetClass);
static void		ConstraintInitialize(Widget, Widget, ArgList,
					     Cardinal*);
static XtGeometryResult GeometryManager(Widget, XtWidgetGeometry*,
					XtWidgetGeometry*);
static void		Initialize(Widget, Widget, ArgList, Cardinal*);
static XtGeometryResult	QueryGeometry(Widget, XtWidgetGeometry*,
				      XtWidgetGeometry*);
static void		Redisplay(Widget, XEvent*, Region);
static void		Resize(Widget);
static Boolean		SetValues(Widget, Widget, Widget, ArgList, Cardinal*);
static XmNavigability	WidgetNavigable(Widget);

/********    End Static Function Declarations    ********/


static XtActionsRec actionsList[] =
{
   { "DrawingAreaInput", _RowColInput },
};


/*  Resource definitions for RowCol
 */

static XmSyntheticResource syn_resources[] =
{
#if XmVersion < 2000
	{	XmNmarginWidth,
		sizeof (Dimension),
		XtOffsetOf( struct _RowColRec, rowcol.marginWd),
		_XmFromHorizontalPixels,
		_XmToHorizontalPixels
	},

	{	XmNmarginHeight,
		sizeof (Dimension),
		XtOffsetOf( struct _RowColRec, rowcol.marginHt),
		_XmFromVerticalPixels,
		_XmToVerticalPixels
	}
#endif
};


static XtResource resources[] =
{
#if XmVersion < 2000
	{	XmNmarginWidth,
		XmCMarginWidth, XmRHorizontalDimension, sizeof (Dimension),
		XtOffsetOf(RowColRec, rowcol.marginWd),
		XmRImmediate, (XtPointer) MARGIN_DEFAULT
	},

	{	XmNmarginHeight,
		XmCMarginHeight, XmRVerticalDimension, sizeof (Dimension),
		XtOffsetOf(RowColRec, rowcol.marginHt),
		XmRImmediate, (XtPointer) MARGIN_DEFAULT
	},
#endif

	{	XmNresizeCallback,
		XmCCallback, XmRCallback, sizeof (XtCallbackList),
		XtOffsetOf(RowColRec, rowcol.resizeCallback),
		XmRImmediate, (XtPointer) NULL
	},

	{	XmNexposeCallback,
		XmCCallback, XmRCallback, sizeof (XtCallbackList),
		XtOffsetOf(RowColRec, rowcol.exposeCallback),
		XmRImmediate, (XtPointer) NULL
	},

	{	XmNinputCallback,
		XmCCallback, XmRCallback, sizeof (XtCallbackList),
		XtOffsetOf(RowColRec, rowcol.inputCallback),
		XmRImmediate, (XtPointer) NULL
	},

	{	RcNgeometryCallback,
		XmCCallback, XmRCallback, sizeof(XtCallbackList),
		XtOffsetOf(RowColRec, rowcol.geometryCallback),
		XmRImmediate, (XtPointer)NULL
	},

	{	RcNchildGeometryCallback,
		XmCCallback, XmRCallback, sizeof(XtCallbackList),
		XtOffsetOf(RowColRec, rowcol.childGeometryCallback),
		XmRImmediate, (XtPointer)NULL
	},

	{	RcNwidthResizePolicy,
		RcCWidthResizePolicy, XmRResizePolicy, sizeof(unsigned char),
		XtOffsetOf(RowColRec, rowcol.wResizePolicy),
		XmRImmediate, (XtPointer)XmRESIZE_ANY
	},

	{	RcNheightResizePolicy,
		RcCHeightResizePolicy, XmRResizePolicy, sizeof(unsigned char),
		XtOffsetOf(RowColRec, rowcol.hResizePolicy),
		XmRImmediate, (XtPointer)XmRESIZE_ANY
	},

	{	RcNrowAlignment,
		RcCRowAlignment, XmRAlignment, sizeof(unsigned char),
		XtOffsetOf(RowColRec, rowcol.rowAlignment),
		XmRImmediate, (XtPointer)XmALIGNMENT_CENTER
	},

	{	RcNcolAlignment,
		RcCColAlignment, XmRAlignment, sizeof(unsigned char),
		XtOffsetOf(RowColRec, rowcol.colAlignment),
		XmRImmediate, (XtPointer)XmALIGNMENT_CENTER
	},

	{	RcNrowHeightAdjust,
		RcCRowHeightAdjust, RcRAdjust, sizeof(RcAdjustT),
		XtOffsetOf(RowColRec, rowcol.rowAdjust),
		XmRImmediate, (XtPointer)RcADJUST_NONE
	},

	{	RcNcolWidthAdjust,
		RcCColWidthAdjust, RcRAdjust, sizeof(RcAdjustT),
		XtOffsetOf(RowColRec, rowcol.colAdjust),
		XmRImmediate, (XtPointer)RcADJUST_NONE
	},

	{	RcNallowRowResize,
		RcCAllowRowResize, XmRBoolean, sizeof(Boolean),
		XtOffsetOf(RowColRec, rowcol.rowResizeOk),
		XmRImmediate, (XtPointer)True
	},

	{	RcNallowColResize,
		RcCAllowColResize, XmRBoolean, sizeof(Boolean),
		XtOffsetOf(RowColRec, rowcol.colResizeOk),
		XmRImmediate, (XtPointer)True
	},

	{	XmNorientation,
		XmCOrientation, RcROrient, sizeof(RcOrientT),
		XtOffsetOf(RowColRec, rowcol.orient),
		XmRImmediate, (XtPointer)RcROW_MAJOR
	},

	{	RcNrowCount,
		RcCRowCount, XmRInt, sizeof(int),
		XtOffsetOf(RowColRec, rowcol.rowCount),
		XmRImmediate, (XtPointer)1
	},

	{	RcNcolCount,
		RcCColCount, XmRInt, sizeof(int),
		XtOffsetOf(RowColRec, rowcol.colCount),
		XmRImmediate, (XtPointer)1
	},

	{	RcNuniformRows,
		RcCUniformRows, XmRBoolean, sizeof(Boolean),
		XtOffsetOf(RowColRec, rowcol.uniformRows),
		XmRImmediate, (XtPointer)False
	},

	{	RcNuniformCols,
		RcCUniformCols, XmRBoolean, sizeof(Boolean),
		XtOffsetOf(RowColRec, rowcol.uniformCols),
		XmRImmediate, (XtPointer)False
	},

	{	RcNrowSpacing,
		RcCRowSpacing, XmRDimension, sizeof(Dimension),
		XtOffsetOf(RowColRec, rowcol.rowSpacing),
		XmRImmediate, (XtPointer)0
	},

	{	RcNcolSpacing,
		RcCColSpacing, XmRDimension, sizeof(Dimension),
		XtOffsetOf(RowColRec, rowcol.colSpacing),
		XmRImmediate, (XtPointer)0
	},

};



/****************************************************************
 *
 * Full class record constant
 *
 ****************************************************************/

static XmBaseClassExtRec baseClassExtRec = {
    NULL,
    NULLQUARK,
    XmBaseClassExtVersion,
    sizeof(XmBaseClassExtRec),
    NULL,				/* InitializePrehook	*/
    NULL,				/* SetValuesPrehook	*/
    NULL,				/* InitializePosthook	*/
    NULL,				/* SetValuesPosthook	*/
    NULL,				/* secondaryObjectClass	*/
    NULL,				/* secondaryCreate	*/
    NULL,               		/* getSecRes data	*/
    { 0 },      			/* fastSubclass flags	*/
    NULL,				/* getValuesPrehook	*/
    NULL,				/* getValuesPosthook	*/
    NULL,                               /* classPartInitPrehook */
    NULL,                               /* classPartInitPosthook*/
    NULL,                               /* ext_resources        */
    NULL,                               /* compiled_ext_resources*/
    0,                                  /* num_ext_resources    */
    FALSE,                              /* use_sub_resources    */
    WidgetNavigable,                    /* widgetNavigable      */
    NULL                                /* focusChange          */
};

externaldef( xmrowcolclassrec) RowColClassRec rowColClassRec =
{
   {			/* core_class fields      */
      (WidgetClass) &xmManagerClassRec,		/* superclass         */
      "RowCol",				/* class_name         */
      sizeof(RowColRec),			/* widget_size        */
      ClassInitialize,	        		/* class_initialize   */
      ClassPartInitialize,			/* class_part_init    */
      FALSE,					/* class_inited       */
      Initialize,       			/* initialize         */
      NULL,					/* initialize_hook    */
      XtInheritRealize,				/* realize            */
      actionsList,				/* actions	      */
      XtNumber(actionsList),			/* num_actions	      */
      resources,				/* resources          */
      XtNumber(resources),			/* num_resources      */
      NULLQUARK,				/* xrm_class          */
      TRUE,					/* compress_motion    */
      XtExposeCompressMaximal | XtExposeGraphicsExposeMerged,
      						/* compress_exposure  */
      TRUE,					/* compress_enterlv   */
      FALSE,					/* visible_interest   */
      NULL,			                /* destroy            */
      Resize,           			/* resize             */
      Redisplay,	        		/* expose             */
      SetValues,                		/* set_values         */
      NULL,					/* set_values_hook    */
      XtInheritSetValuesAlmost,	        	/* set_values_almost  */
      NULL,					/* get_values_hook    */
      NULL,					/* accept_focus       */
      XtVersion,				/* version            */
      NULL,					/* callback_private   */
      defaultTranslations,			/* tm_table           */
      QueryGeometry,                    	/* query_geometry     */
      NULL,             	                /* display_accelerator*/
      (XtPointer)&baseClassExtRec,              /* extension          */
   },
   {		/* composite_class fields */
      GeometryManager,    	                /* geometry_manager   */
      ChangeManaged,	                	/* change_managed     */
      XtInheritInsertChild,			/* insert_child       */
      XtInheritDeleteChild,     		/* delete_child       */
      NULL,                                     /* extension          */
   },

   {		/* constraint_class fields */
      NULL,					/* resource list        */   
      0,					/* num resources        */   
      sizeof(RowColConstraintRec),		/* constraint size      */   
      ConstraintInitialize,			/* init proc            */   
      NULL,					/* destroy proc         */   
      NULL,					/* set values proc      */   
      NULL,                                     /* extension            */
   },

   {		/* manager_class fields */
      traversalTranslations,			/* translations           */
      syn_resources,				/* syn_resources      	  */
      XtNumber (syn_resources),			/* num_get_resources 	  */
      NULL,					/* syn_cont_resources     */
      0,					/* num_get_cont_resources */
      XmInheritParentProcess,                   /* parent_process         */
      NULL,					/* extension           */    
   },

   {		/* RowCol class */     
      (XtPointer) NULL,				/* extension pointer */
   }	
};

externaldef( xmrowcolwidgetclass) WidgetClass rowColWidgetClass
                                       = (WidgetClass) &rowColClassRec;

/*---------------------------------------------------------------
 * Type converter for RcAdjustT
 */

static Boolean
_CvtStringToAdjust(
        Display *display,
        XrmValue *args,
        Cardinal *num_args,
        XrmValue *from,
        XrmValue *to,
        XtPointer *converter_data)
{
   static RcAdjustT	fallback;

   if ( !to->addr ) {
      to->addr = (XPointer)&fallback;
      to->size = sizeof(RcAdjustT);
   }

   if ( to->size < sizeof(RcAdjustT) ) {
      to->size = sizeof(RcAdjustT);
      return False;
   }

   if ( strcasecmp(from->addr, "none")        == 0 ||
	strcasecmp(from->addr, "adjustnone")  == 0 ||
	strcasecmp(from->addr, "adjust_none") == 0 )
      *(RcAdjustT*)to->addr = RcADJUST_NONE;

   else if ( strcasecmp(from->addr, "equal")        == 0 ||
	     strcasecmp(from->addr, "adjustequal")  == 0 ||
	     strcasecmp(from->addr, "adjust_equal") == 0 )
      *(RcAdjustT*)to->addr = RcADJUST_EQUAL;

   else if ( strcasecmp(from->addr, "attach")        == 0 ||
	     strcasecmp(from->addr, "adjustattach")  == 0 ||
	     strcasecmp(from->addr, "adjust_attach") == 0 )
      *(RcAdjustT*)to->addr = RcADJUST_ATTACH;

   else {
      XtStringConversionWarning((char *)from->addr, RcRAdjust);
      return False;
   }

   return True;
}

/*---------------------------------------------------------------
 * Type converter for RcOrientT
 */

static Boolean
_CvtStringToOrient(
        Display *display,
        XrmValue *args,
        Cardinal *num_args,
        XrmValue *from,
        XrmValue *to,
        XtPointer *converter_data)
{
   static RcOrientT	fallback;

   if ( !to->addr ) {
      to->addr = (XPointer)&fallback;
      to->size = sizeof(RcOrientT);
   }

   if ( to->size < sizeof(RcOrientT) ) {
      to->size = sizeof(RcOrientT);
      return False;
   }

   if ( strcasecmp(from->addr, "row")      == 0 ||
	strcasecmp(from->addr, "rowmajor") == 0 ||
	strcasecmp(from->addr, "row_major") )
      *(RcOrientT*)to->addr = RcROW_MAJOR;

   else if ( strcasecmp(from->addr, "col")      == 0 ||
	     strcasecmp(from->addr, "colmajor") == 0 ||
	     strcasecmp(from->addr, "col_major") )
      *(RcOrientT*)to->addr = RcCOL_MAJOR;

   else {
      XtStringConversionWarning((char *)from->addr, RcROrient);
      return False;
   }

   return True;
}

/****************************************************************/
static void 
ClassInitialize( void )
{   
  baseClassExtRec.record_type = XmQmotif;
  XtSetTypeConverter(XmRString, RcRAdjust, _CvtStringToAdjust, NULL, 0,
		     XtCacheAll, NULL);
  XtSetTypeConverter(XmRString, RcROrient, _CvtStringToOrient, NULL, 0,
		     XtCacheAll, NULL);
}

/****************************************************************/
static void 
ClassPartInitialize(
        WidgetClass w_class )
{   
   _XmFastSubclassInit(w_class, XmDRAWING_AREA_BIT);
   return;
}

/****************************************************************
 * Let pass thru zero size, we'll catch them in Realize
 ****************/
static void 
Initialize(
        Widget wreq,
        Widget wnew,
        ArgList args,
        Cardinal *num_args )
{
   RowColPart	*rc = &((RowColWidget)wnew)->rowcol;
   rc->inResize = False;
   rc->setWidth  = 0;
   rc->setHeight = 0;
#if XmVersion >= 2000
   rc->marginWd  = MARGIN_DEFAULT;
   rc->marginHt  = MARGIN_DEFAULT;
#endif
   return;
}

/****************************************************************
 * General redisplay function called on exposure events.
 ****************/
static void 
Redisplay(
        Widget wid,
        XEvent *event,
        Region region )
{
   RowColWidget rc = (RowColWidget) wid;
   RowColCallbackStruct cb;
   
#ifdef DEBUG0
   printf("RowCol(%s) - Redisplay\n", wid->core.name);
#endif

   cb.reason = XmCR_EXPOSE;
   cb.event = event;
   cb.window = XtWindow (rc);

   XtCallCallbackList ((Widget) rc, rc->rowcol.exposeCallback, &cb);

   _XmRedisplayGadgets( (Widget) rc, event, region);
   return;
}

/****************************************************************
 * Invoke the application resize callbacks.
 ****************/
static void 
Resize(
        Widget wid )
{
   RowColPart	*rc = &((RowColWidget)wid)->rowcol;
   RowColCallbackStruct cb;

   if ( rc->inResize ) return;

#if 0
   if ( wid->core.width == rc->width && wid->core.height == rc->height )
      return;
#endif

#ifdef DEBUG
   printf("RowCol(%s) - Resize\n", wid->core.name);
   printf("   new geometry is %d by %d at %d %d\n",
	       wid->core.width, wid->core.height, wid->core.x, wid->core.y);
#endif

   rc->inResize = True;

   rc->setWidth  = wid->core.width;
   rc->setHeight = wid->core.height;

   cb.reason = XmCR_RESIZE;
   cb.event = NULL;
   cb.window = XtWindow(wid);

   XtCallCallbackList(wid, rc->resizeCallback, &cb);
   rc->inResize = False;

#if 0
   rc->width  = wid->core.width;
   rc->height = wid->core.height;
#endif

   return;
}

/************************************************************************
 *  ConstraintInitialize
 ************************************************************************/

static void 
ConstraintInitialize(
        Widget wreq,
        Widget wnew,
        ArgList args,
        Cardinal * num_args )
{
   RowColConstraintPart	*cpart = GetRowColConstraint(wnew);
    
   cpart->prefWd = 0;
   cpart->prefHt = 0;
}

/****************************************************************
 * This function processes key and button presses and releases
 *   belonging to the RowCol..
 ****************/
void 
_RowColInput(
        Widget wid,
        XEvent *event,
        String *params,
        Cardinal *num_params )
{   
            RowColWidget rc = (RowColWidget) wid;
            RowColCallbackStruct cb;
            int x, y;
            Boolean button_event, input_on_gadget, focus_explicit;

    if ((event->type == ButtonPress) || 
	(event->type == ButtonRelease)) {
	x = event->xbutton.x;
	y = event->xbutton.y;
	button_event = True;
    }
    else if (event->type == MotionNotify) {
	x = event->xmotion.x;
	y = event->xmotion.y;
	button_event = True;
    }
    else if ((event->type == KeyPress) || 
	(event->type == KeyRelease)) {
	x = event->xkey.x;
	y = event->xkey.y;
	button_event = False;
    }
    else return; 
	    /* Unrecognized event (cannot determine x, y of pointer).*/
	
    input_on_gadget = (_XmInputForGadget((Widget)rc, x, y) != NULL);
	    
    focus_explicit = ((_XmGetFocusPolicy((Widget)rc) == XmEXPLICIT) &&
		      (rc->composite.num_children != 0));

    if (!input_on_gadget) {
	if ((!focus_explicit) || (button_event)) {
	    cb.reason = XmCR_INPUT;
	    cb.event = event;
	    cb.window = XtWindow( rc);
	    XtCallCallbackList ((Widget) rc,
 				rc->rowcol.inputCallback, &cb);

	}
    }
    return;
}

/*---------------------------------------------------------------
 * Allow all geometry requests.  If we don't like them, we'll change them
 *    later anyway.
 */

static XtGeometryResult 
GeometryManager(
        Widget w,
        XtWidgetGeometry *request,
        XtWidgetGeometry *reply)
{
   RowColPart		*rpart = &((RowColWidget)XtParent(w))->rowcol;
   RowColConstraintPart	*cpart = GetRowColConstraint(w);

#ifdef DEBUG
   printf("RowCol - GeometryManager\n");
   printf("   %s wants to be %d by %d\n",
	   w->core.name, request->width, request->height);
#endif

/*
 * Save the preferred size, but refuse the request
 */
   if ( !(request->request_mode & XtCWQueryOnly) ) {

      RowColGeometryCallbackStruct	cb;
      cb.needWd = (request->request_mode & CWWidth);
      cb.needHt = (request->request_mode & CWHeight);

      if ( cb.needWd || cb.needHt ) {

	 cb.widget = w;

	 if ( cb.needWd ) cb.desiredWd = request->width;
	 else		  cb.desiredWd = w->core.width;

	 if ( cb.needHt ) cb.desiredHt = request->height;
	 else		  cb.desiredHt = w->core.height;

	 XtCallCallbackList(XtParent(w), rpart->childGeometryCallback, &cb);
      }

   } /* End if not a query */

   return XtGeometryNo;
}

/*---------------------------------------------------------------
 * Handle change in children
 */

static void
ChangeManaged(
	Widget wid)
{
   RowColWidget	rcw = (RowColWidget)wid;

   _XmNavigChangeManaged(wid);
   return;
}

/*---------------------------------------------------------------
 * Check for changes.  Ask for expose event if necessary.
 */

static Boolean 
SetValues(
        Widget wold,	/* Values before call */
        Widget wreq,	/* Values user requested */
        Widget wnew,	/* Values after base class processing */
        ArgList args,
        Cardinal *num_args )
{
   RowColPart	*rold = &((RowColWidget)wold)->rowcol;
   RowColPart	*rnew = &((RowColWidget)wnew)->rowcol;
   Boolean	otherChange;

#ifdef DEBUG
   printf("RowCol - SetValues\n");
#endif

/*
 * If the size has changed, call the Resize function.  Any other changes
 *    will get picked up at this time.
 */
   if ( wnew->core.width  != wold->core.width ||
        wnew->core.height != wold->core.height ) {

      Resize(wnew);

      return False;
   }

/*
 * If there is any other type of change, ask for a redisplay.
 */
   return (rnew->marginWd      != rold->marginWd	||
	   rnew->marginHt      != rold->marginHt	||
	   rnew->wResizePolicy != rold->wResizePolicy	||
	   rnew->hResizePolicy != rold->hResizePolicy	||
	   rnew->rowAlignment  != rold->rowAlignment	||
	   rnew->colAlignment  != rold->colAlignment	||
	   rnew->rowAdjust     != rold->rowAdjust	||
	   rnew->colAdjust     != rold->colAdjust	||
	   rnew->orient        != rold->orient		||
	   rnew->rowResizeOk   != rold->rowResizeOk	||
	   rnew->colResizeOk   != rold->colResizeOk	||
	   rnew->uniformRows   != rold->uniformRows	||
	   rnew->uniformCols   != rold->uniformCols	||
	   rnew->rowCount      != rold->rowCount	||
	   rnew->colCount      != rold->colCount	||
	   rnew->rowSpacing    != rold->rowSpacing	||
	   rnew->colSpacing    != rold->colSpacing);

} /* End SetValues */

/*---------------------------------------------------------------
 * Someone wants to know how big we want to be.
 */

static XtGeometryResult 
QueryGeometry(
        Widget wid,
        XtWidgetGeometry *ask,
        XtWidgetGeometry *answer )
{
   RowColGeometryCallbackStruct	cb;
   RowColPart	*rpart = &((RowColWidget)wid)->rowcol;
   Boolean	needWd, needHt;
   Boolean	acceptWd, acceptHt;

#ifdef DEBUG
   printf("RowCol(%s) - QueryGeometry with mode %d\n",
   				wid->core.name, ask->request_mode);
#endif
    
   *answer = *ask;

/*
 * Let the RowColC class calculate the desired size
 */
   needWd = (ask->request_mode & CWWidth)  == CWWidth;
   needHt = (ask->request_mode & CWHeight) == CWHeight;

   cb.widget    = wid;
   cb.needWd    = needWd;
   cb.needHt    = needHt;
   cb.desiredWd = ask->width;
   cb.desiredHt = ask->height;

#ifdef DEBUG
   if ( needWd ) printf("   requested width  %d\n", ask->width);
   if ( needHt ) printf("   requested height %d\n", ask->height);
#endif

   XtCallCallbackList(wid, rpart->geometryCallback, &cb);

/*
 * Report the calculated sizes
 */
   answer->request_mode = (CWWidth | CWHeight);
   answer->width        = cb.desiredWd;
   answer->height       = cb.desiredHt;

#ifdef DEBUG
   printf("   our preferred size is %d by %d\n", answer->width, answer->height);
#endif

/*
 * Use the calculated sizes
 */
   acceptWd = !needWd || (ask->width  == answer->width);
   acceptHt = !needHt || (ask->height == answer->height);

   if ( acceptWd && acceptHt ) {
#ifdef DEBUG
      printf("   we're accepting both width and height\n");
#endif
      return XtGeometryYes;
   }

   if ( answer->width  == wid->core.width &&
	answer->height == wid->core.height ) {
#ifdef DEBUG
      printf("   we're already the size we want to be\n");
#endif
      return XtGeometryNo;
   }

#ifdef DEBUG
   if ( !acceptWd ) printf("   we're not accepting the width\n");
   if ( !acceptHt ) printf("   we're not accepting the height\n");
#endif

   return XtGeometryAlmost;
}

static XmNavigability
WidgetNavigable(
        Widget wid)
{   
  /* If a RowCol has no navigable children, then it is being
   *  used as an X Window work area, so allow navigation to it.
   */
  if(    wid->core.sensitive
     &&  wid->core.ancestor_sensitive
     &&  ((XmManagerWidget) wid)->manager.traversal_on    )
    {   
      XmNavigationType nav_type = ((XmManagerWidget) wid)
	                                            ->manager.navigation_type;
      Widget *children = ((XmManagerWidget) wid)->composite.children;
      unsigned idx = 0;

      while(    idx < ((XmManagerWidget) wid)->composite.num_children    )
	{
#ifndef AIX
  	if(    _XmGetNavigability( children[idx])    )
	    {
#endif
	      if(    (nav_type == XmSTICKY_TAB_GROUP)
		 ||  (nav_type == XmEXCLUSIVE_TAB_GROUP)
		 ||  (    (nav_type == XmTAB_GROUP)
		      &&  !_XmShellIsExclusive( wid))    )
		{
		  return XmDESCENDANTS_TAB_NAVIGABLE;
		}
	      return XmDESCENDANTS_NAVIGABLE;
#ifndef AIX
	    }
#endif
	  ++idx;
	}
      if(    (nav_type == XmSTICKY_TAB_GROUP)
	 ||  (nav_type == XmEXCLUSIVE_TAB_GROUP)
	 ||  (    (nav_type == XmTAB_GROUP)
	      &&  !_XmShellIsExclusive( wid))    )
	{
	  return XmTAB_NAVIGABLE;
	}
      return XmCONTROL_NAVIGABLE;
    }
  return XmNOT_NAVIGABLE;
}

/****************************************************************
 * This function creates and returns a RowCol widget.
 ****************/
Widget 
CreateRowCol(
        Widget p,
        String name,
        ArgList args,
        Cardinal n )
{
    return XtCreateWidget(name, rowColWidgetClass, p, args, n);
}
