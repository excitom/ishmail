/*
 * $Id: WArgList.h,v 1.3 2000/08/13 13:25:32 evgeny Exp $
 *
 * Copyright (c) 1991 HaL Computer Systems, Inc.  All rights reserved.
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

#ifndef WArgList_h
#define WArgList_h

#ifndef _Xm_h
#include <Xm/Xm.h>
#endif

#ifndef _XmDragDrop_h
#include <Xm/DragDrop.h>
#endif

#include <stdarg.h>

// Shorthand
#define	ARGS	args.Args(), args.NumArgs()

// Allow setting of resources by name in arg list
#define RSRC_SET(RESOURCE,TYPE,RNAME) \
   inline WArgList&	RESOURCE(TYPE val) { return Add(RNAME, (XtArgVal)val); }


// Use this one to comment out call
#define DONT_SET(RESOURCE,TYPE,RNAME)

class WArgList {

protected :

   ArgList	args;
   Cardinal	num_args;
   Cardinal	alloc_args;

   Arg&		Grow(Cardinal n);

public :

#if 0
   WArgList(String name, XtArgVal value, ...);
#endif
   inline	WArgList() {
      args       = NULL;
      num_args   =
      alloc_args = 0;
   }

   inline	WArgList(Cardinal len) {
      args       = new Arg[len];
      num_args   = 0;
      alloc_args = len;
   }

   inline	WArgList(ArgList a, Cardinal n) {
      args       = a;
      num_args   = n;
      alloc_args = 0;
   }

   inline	~WArgList() {
      if ( alloc_args ) {
		delete [] args;
      }
   }

   inline ArgList	Args() const	{ return args; }
   inline Cardinal	NumArgs() const { return num_args; }
   inline void		Reset()		{ num_args = 0; }
   inline Arg&		operator[] (Cardinal n);
   inline WArgList&	Add(String name, XtArgVal value);
   inline WArgList&	Add(String name, void* value) {
      return Add(name, (XtArgVal)value);
   }

#ifdef XmString_h
   inline WArgList&	Add(String name, const WXmString& value) {
      return Add(name, (XmString)value);
   }
#endif

// The following methods allow setting of resources by name in arg list

   RSRC_SET(Accelerator,	char*,		"accelerator")
   RSRC_SET(AcceleratorText,	XmString,	"acceleratorText")
   RSRC_SET(Accelerators,	XtTranslations,	"accelerators")
   RSRC_SET(AdjustLast,		Boolean,	"adjustLast")
   RSRC_SET(AdjustMargin,	Boolean,	"adjustMargin")
   RSRC_SET(Alignment,		unsigned char,	"alignment")
   RSRC_SET(AllowOverlap,	Boolean,	"allowOverlap")
   RSRC_SET(AllowResize,	Boolean,	"allowResize")
   RSRC_SET(AllowShellResize,	Boolean,	"allowShellResize")
   RSRC_SET(AncestorSensitive,	Boolean,	"ancestorSensitive")
   RSRC_SET(AnimationMask,	Pixmap,		"animationMask")
   RSRC_SET(AnimationPixmap,	Pixmap,		"animationPixmap")
   RSRC_SET(AnimationPixmapDepth,	int,	"animationPixmapDepth")
   RSRC_SET(AnimationStyle,	unsigned char,	"animationStyle")
   RSRC_SET(ApplyLabelString,	XmString,	"applyLabelString")
   RSRC_SET(Argc,		int,		"argc")
   RSRC_SET(Argv,		void*,		"argv")
   RSRC_SET(ArmColor,		Pixel,		"armColor")
   RSRC_SET(ArmPixmap,		Pixmap,		"armPixmap")
   RSRC_SET(ArrowDirection,	unsigned char,	"arrowDirection")
   RSRC_SET(Attachment,		unsigned char,	"attachment")
   RSRC_SET(AutoShowCursorPosition,	Boolean,	"autoShowCursorPosition")
   RSRC_SET(AutoUnmanage,	Boolean,	"autoUnmanage")
   RSRC_SET(AutomaticSelection,	Boolean,	"automaticSelection")
   RSRC_SET(Background,		Pixel,		"background")
   RSRC_SET(BackgroundPixmap,	Pixmap,		"backgroundPixmap")
   RSRC_SET(BlendModel,		unsigned char,	"blendModel")
   RSRC_SET(BlinkRate,		int,		"blinkRate")
   RSRC_SET(BorderColor,	Pixel,		"borderColor")
   RSRC_SET(BorderPixmap,	Pixmap,		"borderPixmap")
   RSRC_SET(BorderWidth,	Dimension,	"borderWidth")
   RSRC_SET(BottomAttachment,	unsigned char,	"bottomAttachment")
   RSRC_SET(BottomOffset,	int,		"bottomOffset")
   RSRC_SET(BottomPosition,	int,		"bottomPosition")
   RSRC_SET(BottomShadowColor,	Pixel,		"bottomShadowColor")
   RSRC_SET(BottomShadowPixmap,	Pixmap,		"bottomShadowPixmap")
   RSRC_SET(BottomWidget,	Widget,		"bottomWidget")
   RSRC_SET(ButtonFontList,	XmFontList,	"buttonFontList")
   RSRC_SET(CancelButton,	Widget,		"cancelButton")
   RSRC_SET(CancelLabelString,	XmString,	"cancelLabelString")
   RSRC_SET(CascadePixmap,	Pixmap,		"cascadePixmap")
   RSRC_SET(ClientData,		XtPointer,	"clientData")
   RSRC_SET(ClipWindow,		Widget,		"clipWindow")
   RSRC_SET(Colormap,		void*,		"colormap")
   RSRC_SET(Columns,		short,		"columns")
   RSRC_SET(Command,		XmString,	"command")
   RSRC_SET(CommandWindow,	Widget,		"commandWindow")
   RSRC_SET(ConvertProc,	XtConvertSelectionIncrProc,	"convertProc")
   DONT_SET(CreatePopupChildProc,	XtProc,	"createPopupChildProc")
   RSRC_SET(CursorBackground,	Pixel,		"cursorBackground")
   RSRC_SET(CursorForeground,	Pixel,		"cursorForeground")
   RSRC_SET(CursorPosition,	XmTextPosition,	"cursorPosition")
   RSRC_SET(CursorPositionVisible,	Boolean,"cursorPositionVisible")
   RSRC_SET(DarkThreshold,	int,		"darkThreshold")
   RSRC_SET(DecimalPoints,	short,		"decimalPoints")
   RSRC_SET(DefaultButton,	Widget,		"defaultButton")
   RSRC_SET(DefaultButtonShadowThickness, int,	"defaultButtonShadowThickness")
   RSRC_SET(DefaultButtonType,	unsigned char,	"defaultButtonType")
   RSRC_SET(DefaultCopyCursorIcon,	Widget,	"defaultCopyCursorIcon")
   RSRC_SET(DefaultInvalidCursorIcon,	Widget,	"defaultInvalidCursorIcon")
   RSRC_SET(DefaultLinkCursorIcon,	Widget,	"defaultLinkCursorIcon")
   RSRC_SET(DefaultMoveCursorIcon,	Widget,	"defaultMoveCursorIcon")
   RSRC_SET(DefaultNoneCursorIcon,	Widget,	"defaultNoneCursorIcon")
   RSRC_SET(DefaultPosition,	Boolean,	"defaultPosition")
   RSRC_SET(DefaultSourceCursorIcon,	Widget,	"defaultSourceCursorIcon")
   RSRC_SET(DefaultValidCursorIcon,	Widget,	"defaultValidCursorIcon")
   RSRC_SET(DeleteResponse,	unsigned char,	"deleteResponse")
   RSRC_SET(Depth,		int,		"depth")
   RSRC_SET(DialogStyle,	unsigned char,	"dialogStyle")
   RSRC_SET(DialogTitle,	XmString,	"dialogTitle")
   RSRC_SET(DialogType,		unsigned char,	"dialogType")
   RSRC_SET(Directory,		XmString,	"directory")
   RSRC_SET(DirMask,		XmString,	"dirMask")
   RSRC_SET(DirSpec,		XmString,	"dirSpec")
   RSRC_SET(DoubleClickInterval,	int,	"doubleClickInterval")
   RSRC_SET(DragDropFinishCallback, XtCallbackList, "dragDropFinishCallback")
   RSRC_SET(DragInitiatorProtocolStyle,unsigned char,"dragInitiatorProtocolStyle")
   RSRC_SET(DragMotionCallback,	XtCallbackList,	"dragMotionCallback")
   RSRC_SET(DragOperations,	unsigned char,	"dragOperations")
   RSRC_SET(DragProc,		XtCallbackProc,	"dragProc")
   RSRC_SET(DragReceiverProtocolStyle,unsigned char,"dragReceiverProtocolStyle")
   RSRC_SET(DropFinishCallback,	XtCallbackList,	"dropFinishCallback")
   RSRC_SET(DropProc,		XtCallbackProc,	"dropProc")
   RSRC_SET(DropRectangles,	XRectangle*,	"dropRectangles")
   RSRC_SET(DropSiteActivity,	unsigned char,	"dropSiteActivity")
   RSRC_SET(DropSiteEnterCallback,	XtCallbackList,	"dropSiteEnterCallback")
   RSRC_SET(DropSiteLeaveCallback,	XtCallbackList,	"dropSiteLeaveCallback")
   RSRC_SET(DropSiteOperations,	unsigned char,	"dropSiteOperations")
   RSRC_SET(DropSiteType,	unsigned char,	"dropSiteType")
   RSRC_SET(DropStartCallback,	XtCallbackList,	"dropStartCallback")
   RSRC_SET(DropTransfers,	XmDropTransferEntryRec*,	"dropTransfers")
   RSRC_SET(EditMode,		unsigned char,	"editMode")
   RSRC_SET(Editable,		Boolean,	"editable")
   RSRC_SET(EntryAlignment,	unsigned char,	"entryAlignment")
   RSRC_SET(EntryBorder,	Dimension,	"entryBorder")
   RSRC_SET(EntryCallback,	XtCallbackList,	"entryCallback")
   RSRC_SET(EntryClass,		int,		"entryClass")
   RSRC_SET(EntryVerticalAlignment, unsigned char, "entryVerticalAlignment")
   RSRC_SET(ExportTargets,	Atom*,		"exportTargets")
   DONT_SET(FileSearchProc,	XtProc,		"fileSearchProc")
   RSRC_SET(FillOnArm,		Boolean,	"fillOnArm")
   RSRC_SET(FillOnSelect,	Boolean,	"fillOnSelect")
   RSRC_SET(FilterLabelString,	XmString,	"filterLabelString")
   RSRC_SET(Font,		XFontStruct*,	"font")
   RSRC_SET(FontList,		XmFontList,	"fontList")
   RSRC_SET(Foreground,		Pixel,		"foreground")
   RSRC_SET(ForegroundThreshold,	int,	"foregroundThreshold")
   RSRC_SET(FractionBase,	int,		"fractionBase")
   RSRC_SET(Geometry,		char*,		"geometry")
   RSRC_SET(Height,		Dimension,	"height")
   RSRC_SET(HeightInc,		int,		"heightInc")
   RSRC_SET(HelpLabelString,	XmString,	"helpLabelString")
   RSRC_SET(HighlightColor,	Pixel,		"highlightColor")
   RSRC_SET(HighlightOnEnter,	Boolean,	"highlightOnEnter")
   RSRC_SET(HighlightPixmap,	Pixmap,		"highlightPixmap")
   RSRC_SET(HighlightThickness,	short,		"highlightThickness")
   RSRC_SET(HistoryItemCount,	int,		"historyItemCount")
   RSRC_SET(HistoryItems,	XmStringTable,	"historyItems")
   RSRC_SET(HistoryMaxItems,	int,		"historyMaxItems")
   RSRC_SET(HistoryVisibleItemCount,	int,	"historyVisibleItemCount")
   RSRC_SET(HorizontalFontUnit,	int,		"horizontalFontUnit")
   RSRC_SET(HorizontalScrollBar,	Widget,	"horizontalScrollBar")
   RSRC_SET(HorizontalSpacing,	int,		"horizontalSpacing")
   RSRC_SET(HotX,		Position,	"hotX")
   RSRC_SET(HotY,		Position,	"hotY")
   RSRC_SET(IconMask,		Pixmap,		"iconMask")
   RSRC_SET(IconName,		char*,		"iconName")
   RSRC_SET(IconPixmap,		Pixmap,		"iconPixmap")
   RSRC_SET(IconWindow,		Widget,		"iconWindow")
   RSRC_SET(IconX,		int,		"iconX")
   RSRC_SET(IconY,		int,		"iconY")
   RSRC_SET(Iconic,		Boolean,	"iconic")
   RSRC_SET(Increment,		int,		"increment")
   RSRC_SET(Incremental,	Boolean,	"incremental")
   RSRC_SET(IndicatorOn,	Boolean,	"indicatorOn")
   RSRC_SET(IndicatorSize,	Dimension,	"indicatorSize")
   RSRC_SET(IndicatorType,	unsigned char,	"indicatorType")
   RSRC_SET(InitialDelay,	int,		"initialDelay")
   RSRC_SET(InitialState,	int,		"initialState")
   RSRC_SET(Input,		Boolean,	"input")
   DONT_SET(InputCreate,	XtProc,		"inputCreate")
   RSRC_SET(InsertPosition,	XtOrderProc,	"insertPosition")
   RSRC_SET(InvalidCursorForeground,	Pixel,	"invalidCursorForeground")
   RSRC_SET(ImportTargets,	Atom*,		"importTargets")
   RSRC_SET(IsAligned,		Boolean,	"isAligned")
   RSRC_SET(IsHomogeneous,	Boolean,	"isHomogeneous")
   RSRC_SET(ItemCount,		int,		"itemCount")
   RSRC_SET(Items,		XmStringTable,	"items")
   RSRC_SET(KeyboardFocusPolicy,	unsigned char,	"keyboardFocusPolicy")
   RSRC_SET(LabelFontList,	XmFontList,	"labelFontList")
   RSRC_SET(LabelInsensitivePixmap,	Pixmap,	"labelInsensitivePixmap")
   RSRC_SET(LabelPixmap,	Pixmap,		"labelPixmap")
   RSRC_SET(LabelString,	XmString,	"labelString")
   RSRC_SET(LabelType,		unsigned char,	"labelType")
   RSRC_SET(LeftAttachment,	unsigned char,	"leftAttachment")
   RSRC_SET(LeftOffset,		int,		"leftOffset")
   RSRC_SET(LeftPosition,	int,		"leftPosition")
   RSRC_SET(LeftWidget,		Widget,		"leftWidget")
   RSRC_SET(LightThreshold,	int,		"lightThreshold")
   RSRC_SET(ListItemCount,	int,		"listItemCount")
   RSRC_SET(ListItems,		XmStringTable,	"listItems")
   RSRC_SET(ListLabelString,	XmString,	"listLabelString")
   RSRC_SET(ListMarginHeight,	short,		"listMarginHeight")
   RSRC_SET(ListMarginWidth,	short,		"listMarginWidth")
   RSRC_SET(ListSizePolicy,	unsigned char,	"listSizePolicy")
   RSRC_SET(ListSpacing,	short,		"listSpacing")
   RSRC_SET(ListUpdated,	Boolean,	"listUpdated")
   RSRC_SET(ListVisibleItemCount,	int,	"listVisibleItemCount")
   RSRC_SET(MainWindowMarginHeight,	short,	"mainWindowMarginHeight")
   RSRC_SET(MainWindowMarginWidth,	short,	"mainWindowMarginWidth")
   RSRC_SET(MapCallback,	XtCallbackList,	"mapCallback")
   RSRC_SET(MappedWhenManaged,	Boolean,	"mappedWhenManaged")
   RSRC_SET(MappingDelay,	int,		"mappingDelay")
   RSRC_SET(Margin,		short,		"margin")
   RSRC_SET(MarginBottom,	short,		"marginBottom")
   RSRC_SET(MarginHeight,	Dimension,	"marginHeight")
   RSRC_SET(MarginLeft,		short,		"marginLeft")
   RSRC_SET(MarginRight,	short,		"marginRight")
   RSRC_SET(MarginTop,		short,		"marginTop")
   RSRC_SET(MarginWidth,	Dimension,	"marginWidth")
   RSRC_SET(Mask,		Pixmap,		"mask")
   RSRC_SET(MaxAspectX,		int,		"maxAspectX")
   RSRC_SET(MaxAspectY,		int,		"maxAspectY")
   RSRC_SET(MaxHeight,		int,		"maxHeight")
   RSRC_SET(MaxLength,		int,		"maxLength")
   RSRC_SET(MaxWidth,		int,		"maxWidth")
   RSRC_SET(Maximum,		int,		"maximum")
   RSRC_SET(MenuAccelerator,	char*,		"menuAccelerator")
   RSRC_SET(MenuBar,		Widget,		"menuBar")
   RSRC_SET(MenuCursor,		String,		"menuCursor")
   RSRC_SET(MenuHelpWidget,	Widget,		"menuHelpWidget")
   RSRC_SET(MenuHistory,	Widget,		"menuHistory")
   RSRC_SET(MenuPost,		String,		"menuPost")
   RSRC_SET(MessageAlignment,	unsigned char,	"messageAlignment")
   RSRC_SET(MessageString,	XmString,	"messageString")
   RSRC_SET(MessageWindow,	Widget,		"messageWindow")
   RSRC_SET(MinAspectX,		int,		"minAspectX")
   RSRC_SET(MinAspectY,		int,		"minAspectY")
   RSRC_SET(MinHeight,		int,		"minHeight")
   RSRC_SET(MinWidth,		int,		"minWidth")
   RSRC_SET(MinimizeButtons,	Boolean,	"minimizeButtons")
   RSRC_SET(Minimum,		int,		"minimum")
   RSRC_SET(Mnemonic,		char,		"mnemonic")
   RSRC_SET(MnemonicCharSet,	String,		"mnemonicCharSet")
   RSRC_SET(MultiClick,		unsigned char,	"multiClick")
   RSRC_SET(MustMatch,		Boolean,	"mustMatch")
   RSRC_SET(MwmDecorations,	int,		"mwmDecorations")
   RSRC_SET(MwmFunctions,	int,		"mwmFunctions")
   RSRC_SET(MwmInputMode,	int,		"mwmInputMode")
   RSRC_SET(MwmMenu,		char*,		"mwmMenu")
   RSRC_SET(NavigationType,	XmNavigationType,	"navigationType")
   RSRC_SET(NoneCursorForeground,	Pixel,	"noneCursorForeground")
   RSRC_SET(NoResize,		Boolean,	"noResize")
   RSRC_SET(NumColumns,		short,		"numColumns")
   RSRC_SET(NumDropRectangles,	Cardinal,	"numDropRectangles")
   RSRC_SET(NumDropTransfers,	Cardinal,	"numDropTransfers")
   RSRC_SET(NumExportTargets,	Cardinal,	"numExportTargets")
   RSRC_SET(NumImportTargets,	Cardinal,	"numImportTargets")
   RSRC_SET(OffsetX,		Position,	"offsetX")
   RSRC_SET(OffsetY,		Position,	"offsetY")
   RSRC_SET(OkLabelString,	XmString,	"okLabelString")
   RSRC_SET(OperationChangedCallback, XtCallbackList,"operationChangedCallback")
   RSRC_SET(OperationCursorIcon,	Widget,	"operationCursorIcon")
   RSRC_SET(Orientation,	unsigned char,	"orientation")
   DONT_SET(OutputCreate,	XtProc,		"outputCreate")
   RSRC_SET(OverrideRedirect,	Boolean,	"overrideRedirect")
   RSRC_SET(Packing,		unsigned char,	"packing")
   RSRC_SET(PageIncrement,	int,		"pageIncrement")
   RSRC_SET(PaneMaximum,	Dimension,	"paneMaximum")
   RSRC_SET(PaneMinimum,	Dimension,	"paneMinimum")
   RSRC_SET(Pattern,		XmString,	"pattern")
   RSRC_SET(PendingDelete,	Boolean,	"pendingDelete")
   RSRC_SET(pixmap,		Pixmap,		"pixmap")
   RSRC_SET(PopupEnabled,	Boolean,	"popupEnabled")
   RSRC_SET(PositionIndex,	short,		"positionIndex")
   RSRC_SET(ProcessingDirection,	unsigned char,	"processingDirection")
   RSRC_SET(PromptString,	XmString,	"promptString")
   RSRC_SET(PushButtonEnabled,	Boolean,	"pushButtonEnabled")
   RSRC_SET(RadioAlwaysOne,	Boolean,	"radioAlwaysOne")
   RSRC_SET(RadioBehavior,	Boolean,	"radioBehavior")
   RSRC_SET(RecomputeSize,	Boolean,	"recomputeSize")
   RSRC_SET(RefigureMode,	Boolean,	"refigureMode")
   RSRC_SET(RepeatDelay,	int,		"repeatDelay")
   RSRC_SET(Resizable,		Boolean,	"resizable")
   RSRC_SET(ResizeHeight,	Boolean,	"resizeHeight")
   RSRC_SET(ResizePolicy,	unsigned char,	"resizePolicy")
   RSRC_SET(ResizeWidth,	Boolean,	"resizeWidth")
   RSRC_SET(RightAttachment,	unsigned char,	"rightAttachment")
   RSRC_SET(RightOffset,	int,		"rightOffset")
   RSRC_SET(RightPosition,	int,		"rightPosition")
   RSRC_SET(RightWidget,	Widget,		"rightWidget")
   RSRC_SET(RowColumnType,	unsigned char,	"rowColumnType")
   RSRC_SET(Rows,		short,		"rows")
   RSRC_SET(RubberPositioning,	Boolean,	"rubberPositioning")
   RSRC_SET(SashHeight,		Dimension,	"sashHeight")
   RSRC_SET(SashIndent,		Position,	"sashIndent")
   RSRC_SET(SashShadowThickness,	int,	"sashShadowThickness")
   RSRC_SET(SashWidth,		Dimension,	"sashWidth")
   RSRC_SET(SaveUnder,		Boolean,	"saveUnder")
   RSRC_SET(ScaleHeight,	Dimension,	"scaleHeight")
   RSRC_SET(ScaleWidth,		Dimension,	"scaleWidth")
   RSRC_SET(ScrollBarDisplayPolicy,	unsigned char,	"scrollBarDisplayPolicy")
   RSRC_SET(ScrollBarPlacement,	unsigned char,	"scrollBarPlacement")
   RSRC_SET(ScrollHorizontal,	Boolean,	"scrollHorizontal")
   RSRC_SET(ScrollLeftSide,	Boolean,	"scrollLeftSide")
   RSRC_SET(ScrollTopSide,	Boolean,	"scrollTopSide")
   RSRC_SET(ScrollVertical,	Boolean,	"scrollVertical")
   RSRC_SET(ScrolledWindowMarginHeight,	short,	"scrolledWindowMarginHeight")
   RSRC_SET(ScrolledWindowMarginWidth,	short,	"scrolledWindowMarginWidth")
   RSRC_SET(ScrollingPolicy,	unsigned char,	"scrollingPolicy")
   RSRC_SET(SelectColor,	Pixel,		"selectColor")
   RSRC_SET(SelectInsensitivePixmap,	Pixmap,	"selectInsensitivePixmap")
   RSRC_SET(SelectPixmap,	Pixmap,		"selectPixmap")
   RSRC_SET(SelectThreshold,	int,		"selectThreshold")
   RSRC_SET(SelectedItemCount,	int,		"selectedItemCount")
   RSRC_SET(SelectedItems,	XmStringTable,	"selectedItems")
   RSRC_SET(SelectionArray,	XtPointer,	"selectionArray")
   RSRC_SET(SelectionArrayCount,	int,	"selectionArrayCount")
   RSRC_SET(SelectionLabelString,	XmString,	"selectionLabelString")
   RSRC_SET(SelectionPolicy,	unsigned char,	"selectionPolicy")
   RSRC_SET(Sensitive,		Boolean,	"sensitive")
   RSRC_SET(SeparatorOn,	Boolean,	"separatorOn")
   RSRC_SET(SeparatorType,	unsigned char,	"separatorType")
   RSRC_SET(Set,		Boolean,	"set")
   RSRC_SET(ShadowThickness,	short,		"shadowThickness")
   RSRC_SET(ShadowType,		unsigned char,	"shadowType")
   RSRC_SET(ShellUnitType,	unsigned char,	"shellUnitType")
   RSRC_SET(ShowArrows,		Boolean,	"showArrows")
   RSRC_SET(ShowAsDefault,	short,		"showAsDefault")
   RSRC_SET(ShowSeparator,	Boolean,	"showSeparator")
   RSRC_SET(ShowValue,		Boolean,	"showValue")
   RSRC_SET(SkipAdjust,		Boolean,	"skipAdjust")
   RSRC_SET(SliderSize,		int,		"sliderSize")
   DONT_SET(Source,		XmTextSource,	"source")
   RSRC_SET(SourceCursorIcon,	Widget,		"sourceCursorIcon")
   RSRC_SET(SourcePixmapIcon,	Widget,		"sourcePixmapIcon")
   RSRC_SET(Spacing,		Dimension,	"spacing")
   RSRC_SET(StateCursorIcon,	Widget,		"stateCursorIcon")
   RSRC_SET(StringDirection,	unsigned char,	"stringDirection")
   RSRC_SET(SubMenuId,		Widget,		"subMenuId")
   RSRC_SET(SymbolPixmap,	Pixmap,		"symbolPixmap")
   RSRC_SET(TearOffMenuActivateCallback, XtCallbackList, "tearOffMenuActivateCallback")
   RSRC_SET(TearOffMenuDeactivateCallback, XtCallbackList, "tearOffMenuDeactivateCallback")
   RSRC_SET(TearOffModel,	unsigned char,	"tearOffModel")
   RSRC_SET(TextAccelerators,	XtTranslations,	"textAccelerators")
   RSRC_SET(TextColumns,	short,		"textColumns")
   RSRC_SET(TextFontList,	XmFontList,	"textFontList")
   RSRC_SET(TextString,		XmString,	"textString")
   RSRC_SET(TextTranslations,	XtTranslations,	"textTranslations")
   RSRC_SET(Title,		char*,		"title")
   RSRC_SET(TitleString,	XmString,	"titleString")
   RSRC_SET(TopAttachment,	unsigned char,	"topAttachment")
   RSRC_SET(TopCharacter,	XmTextPosition,	"topCharacter")
   RSRC_SET(TopItemPosition,	int,		"topItemPosition")
   RSRC_SET(TopLevelEnterCallback,	XtCallbackList,	"topLevelEnterCallback")
   RSRC_SET(TopLevelLeaveCallback,	XtCallbackList,	"topLevelLeaveCallback")
   RSRC_SET(TopOffset,		int,		"topOffset")
   RSRC_SET(TopPosition,	int,		"topPosition")
   RSRC_SET(TopShadowColor,	Pixel,		"topShadowColor")
   RSRC_SET(TopShadowPixmap,	Pixmap,		"topShadowPixmap")
   RSRC_SET(TopWidget,		Widget,		"topWidget")
   RSRC_SET(TransferProc,	XtSelectionCallbackProc,	"transferProc")
   RSRC_SET(TransferStatus,	unsigned char,	"transferStatus")
   RSRC_SET(Transient,		Boolean,	"transient")
   RSRC_SET(Translations,	XtTranslations,	"translations")
   RSRC_SET(TraversalOn,	Boolean,	"traversalOn")
   RSRC_SET(UnitType,		unsigned char,	"unitType")
   RSRC_SET(UnmapCallback,	XtCallbackList,	"unmapCallback")
   RSRC_SET(UnpostBehavior,	unsigned char,	"unpostBehavior")
   RSRC_SET(UserData,		XtPointer,	"userData")
   RSRC_SET(ValidCursorForeground,	Pixel,	"validCursorForeground")
   RSRC_SET(Value,		char*,		"value")
   RSRC_SET(Value,		int,		"value")
   RSRC_SET(VerifyBell,		Boolean,	"verifyBell")
   RSRC_SET(VerticalFontUnit,	int,		"verticalFontUnit")
   RSRC_SET(VerticalScrollBar,	Widget,		"verticalScrollBar")
   RSRC_SET(VerticalSpacing,	int,		"verticalSpacing")
   RSRC_SET(VisibleItemCount,	int,		"visibleItemCount")
   RSRC_SET(VisibleWhenOff,	Boolean,	"visibleWhenOff")
   RSRC_SET(VisualPolicy,	unsigned char,	"visualPolicy")
   RSRC_SET(Waitforwm,		Boolean,	"waitforwm")
   RSRC_SET(WhichButton,	unsigned int,	"whichButton")
   RSRC_SET(Width,		Dimension,	"width")
   RSRC_SET(WidthInc,		int,		"widthInc")
   RSRC_SET(WindowGroup,	Widget,		"windowGroup")
   RSRC_SET(WmTimeout,		int,		"wmTimeout")
   RSRC_SET(WordWrap,		Boolean,	"wordWrap")
   RSRC_SET(WorkWindow,		Widget,		"workWindow")
   RSRC_SET(X,			Position,	"x")
   RSRC_SET(Y,			Position,	"y")
   RSRC_SET(_Screen,		void*,		"screen")

#if XmVersion >= 1002
// Constraint Resources -- Valid only in a frame
   RSRC_SET(ChildType,		      unsigned char, "childType")
   RSRC_SET(ChildHorizontalAlignment, unsigned char, "childHorizontalAlignment")
   RSRC_SET(ChildHorizontalSpacing,   Dimension,     "childHorizontalSpacing")
   RSRC_SET(ChildVerticalAlignment,   unsigned char, "childVerticalAlignment")
#endif

   inline WArgList& LeftAttachment(unsigned char to_what, Widget w) {
      LeftAttachment(to_what);
      LeftWidget(w);
      return *this;
   }
   inline WArgList& LeftAttachment(unsigned char to_what, Widget w,int off) {
      LeftAttachment(to_what);
      LeftWidget(w);
      LeftOffset(off);
      return *this;
   }
   inline WArgList& LeftAttachment(unsigned char to_what, int posoff) {
      LeftAttachment(to_what);
      if( to_what == XmATTACH_POSITION ) LeftPosition(posoff);
      else				 LeftOffset(posoff);
      return *this;
   }
   inline WArgList& LeftAttachment(unsigned char to_what, int pos, int off) {
      LeftAttachment(to_what);
      LeftPosition(pos);
      LeftOffset(off);
      return *this;
   }

   inline WArgList& RightAttachment(unsigned char to_what, Widget w) {
      RightAttachment(to_what);
      RightWidget(w);
      return *this;
   }
   inline WArgList& RightAttachment(unsigned char to_what, Widget w,int off) {
      RightAttachment(to_what);
      RightWidget(w);
      RightOffset(off);
      return *this;
   }
   inline WArgList& RightAttachment(unsigned char to_what, int posoff) {
      RightAttachment(to_what);
      if( to_what == XmATTACH_POSITION ) RightPosition(posoff);
      else				 RightOffset(posoff);
      return *this;
   }
   inline WArgList& RightAttachment(unsigned char to_what, int pos, int off) {
      RightAttachment(to_what);
      RightPosition(pos);
      RightOffset(off);
      return *this;
   }

   inline WArgList& TopAttachment(unsigned char to_what, Widget w) {
      TopAttachment(to_what);
      TopWidget(w);
      return *this;
   }
   inline WArgList& TopAttachment(unsigned char to_what, Widget w,int off) {
      TopAttachment(to_what);
      TopWidget(w);
      TopOffset(off);
      return *this;
   }
   inline WArgList& TopAttachment(unsigned char to_what, int posoff) {
      TopAttachment(to_what);
      if( to_what == XmATTACH_POSITION ) TopPosition(posoff);
      else				 TopOffset(posoff);
      return *this;
   }
   inline WArgList& TopAttachment(unsigned char to_what, int pos, int off) {
      TopAttachment(to_what);
      TopPosition(pos);
      TopOffset(off);
      return *this;
   }

   inline WArgList& BottomAttachment(unsigned char to_what, Widget w) {
      BottomAttachment(to_what);
      BottomWidget(w);
      return *this;
   }
   inline WArgList& BottomAttachment(unsigned char to_what, Widget w,int off) {
      BottomAttachment(to_what);
      BottomWidget(w);
      BottomOffset(off);
      return *this;
   }
   inline WArgList& BottomAttachment(unsigned char to_what, int posoff) {
      BottomAttachment(to_what);
      if( to_what == XmATTACH_POSITION ) BottomPosition(posoff);
      else				 BottomOffset(posoff);
      return *this;
   }
   inline WArgList& BottomAttachment(unsigned char to_what, int pos, int off) {
      BottomAttachment(to_what);
      BottomPosition(pos);
      BottomOffset(off);
      return *this;
   }
};

inline Arg&
WArgList::Grow (Cardinal n)
{
   // Return a reference to the requested arg if it is present
   if (n >= alloc_args) {

      // Allocate more if requested arg is not present
      int	new_alloc_args = n + 16;
      ArgList new_args = new Arg[new_alloc_args];

      // Copy any existing args to the new area
      if (args && num_args) {

         memcpy (new_args, args, num_args * sizeof (Arg));

         // Delete existing args if allocated
         if (alloc_args) {
	    delete args;
         }
      }

      // Store new information
      alloc_args = new_alloc_args;
      args = new_args;

   }
      // Return reference to requested argument
   return args[n];
}

inline Arg&
WArgList::operator [] (Cardinal n)
{
   if (n < num_args|| n < alloc_args) return args[n];
   else				      return Grow(n);
}

inline WArgList&
WArgList::Add (String name, XtArgVal value)
{
// First look to see if this arg is already in the list.
//    Use that one if it is

   Arg		*a = NULL;
   Boolean	found = False;

// Loop til a matching name is found or we get to the end of the array
   for (int i=0; !found && i<num_args; i++) {
      a = &(*this)[i];
      if ( strcmp(a->name, name) == 0 ) {
	 found = True;
      }
   }

// If the arg was found, then "a" is already set.  If not, set it here

   if ( !found ) {
      a = &(*this)[num_args];
      num_args++;
   }

   a->name  = name;
   a->value = value;

   return (*this); 
}

#if 0
inline WArgList::WArgList (String name, XtArgVal value, ...)
   : args (NULL), num_args (0), alloc_args (0)
{
   String str;

   Add (name, value);
   va_list argv;
   va_start (argv, value);
   while (str = va_arg(argv, String))
      Add (str, va_arg (argv, XtArgVal));
   va_end (argv);
}
#endif

#endif // WArgList_h
