TOP = ..
MAKECONF = $(TOP)/Make.conf
include $(MAKECONF)

DEPENDFILE = Make.dep

.SUFFIXES : .c .C $(O)

#----------------------------------------------------------------------------
# Suffix rules
#

.C$(O) :
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -I$(TOP) $(GUI_FLAGS) -c $< -o $@
.c$(O) :
	$(CC)  $(CFLAGS)   $(CPPFLAGS) -I$(TOP) $(GUI_FLAGS) -c $< -o $@

CSRC =	\
	BoolExpC.C	\
	BoolExpWinC.C	\
	CallbackC.C	\
	CharC.C		\
	ExpElemC.C	\
	FieldViewC.C	\
	FiltDialogC.C	\
	FindDialogC.C	\
	FontChooserC.C	\
	FontDataC.C	\
	ForceDialog.C	\
	HalAppC.C	\
	HalDialogC.C	\
	HalMainWinC.C	\
	HalShellC.C	\
	HalTopLevelC.C	\
	HelpDbC.C	\
	HelpResWinC.C	\
	HelpC.C		\
	IconC.C		\
	IconDataC.C	\
	IconViewC.C	\
	JoyStickC.C	\
	LgIconViewC.C	\
	ListBoxC.C	\
	MemMap.C	\
	MimeRichTextDraw.C	\
	MimeRichTextEdit.C	\
	MimeRichTextInit.C	\
	MimeRichTextSelect.C	\
	PixmapC.C	\
	PtrDictC.C	\
	PtrList2.C	\
	RegexC.C	\
	RichSearchWinC.C	\
	RowColC.C	\
	RowColChildC.C	\
	RowOrColC.C	\
	Shadow.C	\
	SignalRegistry.C	\
	SmIconViewC.C	\
	SortDialogC.C	\
	SortExpWinC.C	\
	StrCase.C	\
	StringC.C	\
	SysErr.C	\
	System.C	\
	TBoxC.C		\
	TextMisc.C	\
	VBoxC.C		\
	VItemC.C	\
	ViewC.C		\
	WorkingBoxC.C	\
	rsrc.C


cSRC =	\
	RowCol.c	\
	ButtonBox.c	\
	regex.c

#
# Template files
#

CTEMPLATES = \
	CallbackListC.C	\
	ColumnListC.C	\
	ExpElemListC.C	\
	FieldListC.C	\
	IconDataDictC.C	\
	IntListC.C	\
	ItemDataDictC.C	\
	LabelDataListC.C	\
	PixelListC.C	\
	PixmapDataDictC.C	\
	PixmapNameDictC.C	\
	PtrListC.C	\
	RangeListC.C	\
	RowColChildListC.C	\
	RowOrColListC.C	\
	SignalDictC.C	\
	SortElemListC.C	\
	SortKeyListC.C	\
	StringDictC.C	\
	StringListC.C	\
	VItemListC.C	\
	ViewListC.C	\
	WidgetListC.C

HTEMPLATES = \
	CallbackListC.h	\
	ColumnListC.h	\
	ExpElemListC.h	\
	FieldListC.h	\
	IconDataDictC.h	\
	IntListC.h	\
	ItemDataDictC.h	\
	LabelDataListC.h	\
	PixelListC.h	\
	PixmapDataDictC.h	\
	PixmapNameDictC.h	\
	PtrListC.h	\
	RangeListC.h	\
	RowColChildListC.h	\
	RowOrColListC.h	\
	SignalDictC.h	\
	SortElemListC.h	\
	SortKeyListC.h	\
	StringDictC.h	\
	StringListC.h	\
	VItemListC.h	\
	ViewListC.h	\
	WidgetListC.h

TEMPLATES = $(CTEMPLATES) $(HTEMPLATES)

SRC = $(CSRC) $(CTEMPLATES) $(cSRC)


OBJS =	\
	BoolExpWinC$(O)		\
	BoolExpC$(O)		\
	ExpElemListC$(O)	\
	ExpElemC$(O)		\
	FieldViewC$(O)		\
	ColumnListC$(O)		\
	FieldListC$(O)		\
	FontChooserC$(O)	\
	HalMainWinC$(O)		\
	HalTopLevelC$(O)	\
	IconC$(O)		\
	ItemDataDictC$(O)	\
	ListBoxC$(O)		\
	RangeListC$(O)		\
	RowColC$(O)		\
	RowCol$(O)		\
	RowOrColListC$(O)	\
	RowOrColC$(O)		\
	RowColChildListC$(O)	\
	RowColChildC$(O)	\
	StringDictC$(O)		\
	Shadow$(O)		\
	TBoxC$(O)		\
	WidgetListC$(O)		\
	HalAppC$(O)		\
	HelpC$(O)		\
	HelpDbC$(O)		\
	HelpResWinC$(O)		\
	SignalRegistry$(O)	\
	SignalDictC$(O)		\
	WorkingBoxC$(O)		\
	VBoxC$(O)		\
	FiltDialogC$(O)		\
	FindDialogC$(O)		\
	LgIconViewC$(O)		\
	SmIconViewC$(O)		\
	IconViewC$(O)		\
	IconDataC$(O)		\
	IconDataDictC$(O)	\
	LabelDataListC$(O)	\
	PixmapC$(O)		\
	PixmapDataDictC$(O)	\
	PixmapNameDictC$(O)	\
	SortDialogC$(O)		\
	VItemListC$(O)		\
	VItemC$(O)		\
	ViewC$(O)		\
	ViewListC$(O)		\
	SortExpWinC$(O)		\
	SortElemListC$(O)	\
	SortKeyListC$(O)	\
	RichSearchWinC$(O)	\
	HalDialogC$(O)		\
	HalShellC$(O)		\
	ButtonBox$(O)		\
	ForceDialog$(O)		\
	TextMisc$(O)		\
	MemMap$(O)		\
	SysErr$(O)		\
	MimeRichTextSelect$(O)	\
	System$(O)		\
	JoyStickC$(O)		\
	MimeRichTextInit$(O)	\
	FontDataC$(O)		\
	MimeRichTextEdit$(O)	\
	RegexC$(O)		\
	StrCase$(O)		\
	regex$(O)		\
	rsrc$(O)		\
	IntListC$(O)		\
	MimeRichTextDraw$(O)	\
	PixelListC$(O)		\
	PtrDictC$(O)		\
	PtrListC$(O)		\
	PtrList2$(O)		\
	StringListC$(O)		\
	StringC$(O)		\
	CharC$(O)		\
	CallbackC$(O)		\
	CallbackListC$(O)

LIB    = libhgl.a

all : $(LIB)

$(LIB) : $(OBJS)
	$(RM) $(LIB)
	$(AR) $(AR_FLAGS) $@ $(OBJS)
	$(RANLIB) $@

templates : $(TEMPLATES)

install : $(LIB)

clean :
	$(RM) *.o $(LIB) *.bak *~

distclean : clean
	$(RM) $(TEMPLATES) tags

tags : $(SRC)
	ctags $(SRC)

srcdistro : $(TEMPLATES) $(DEPENDFILE)

#----------------------------------------------------------------------------
# template build rules
#

CallbackListC.h :	PListC.clas
	$(BUILD_LIST_H) P CallbackListC CallbackC '"CallbackC.h"'
ColumnListC.h :	PListC.clas
	$(BUILD_LIST_H) P ColumnListC ColumnC '"ColumnC.h"'
ExpElemListC.h :	PListC.clas
	$(BUILD_LIST_H) P ExpElemListC ExpElemC '"ExpElemC.h"'
FieldListC.h :	PListC.clas
	$(BUILD_LIST_H) P FieldListC FieldC '"FieldC.h"'
IntListC.h :	OListC.clas
	$(BUILD_LIST_H) O IntListC int '<stream.h>'
LabelDataListC.h :	OListC.clas
	$(BUILD_LIST_H) O LabelDataListC LabelDataC '"LabelDataC.h"'
PixelListC.h :	OListC.clas
	$(BUILD_LIST_H) O PixelListC Pixel '<X11/Intrinsic.h>'
PtrListC.h :	OListC.clas
	$(BUILD_LIST_H) O PtrListC PtrT '"PtrT.h"'
RangeListC.h :	OListC.clas
	$(BUILD_LIST_H) O RangeListC RangeC '"RangeC.h"'
RowColChildListC.h :	PListC.clas
	$(BUILD_LIST_H) P RowColChildListC RowColChildC '"RowColChildC.h"'
RowOrColListC.h :	PListC.clas
	$(BUILD_LIST_H) P RowOrColListC RowOrColC '"RowOrColC.h"'
SortElemListC.h :	PListC.clas
	$(BUILD_LIST_H) P SortElemListC SortElemC '"SortElemC.h"'
SortKeyListC.h :	PListC.clas
	$(BUILD_LIST_H) P SortKeyListC SortKeyC '"SortKeyC.h"'
StringListC.h :	OListC.clas
	$(BUILD_LIST_H) O StringListC StringC '"StringC.h"'
VItemListC.h :	PListC.clas
	$(BUILD_LIST_H) P VItemListC VItemC '"VItemC.h"'
ViewListC.h :	PListC.clas
	$(BUILD_LIST_H) P ViewListC ViewC '"ViewC.h"'
WidgetListC.h :	OListC.clas
	$(BUILD_LIST_H) O WidgetListC Widget '<X11/Intrinsic.h>'

IconDataDictC.h :	PDictC.clas
	$(BUILD_DICT_H) P IconDataDictC VItemC IconDataC '"VItemC.h"' '"IconDataC.h"'
ItemDataDictC.h :	PDictC.clas
	$(BUILD_DICT_H) P ItemDataDictC VItemC ItemDataC '"VItemC.h"' '"ItemDataC.h"'
PixmapDataDictC.h :	ODictC.clas
	$(BUILD_DICT_H) O PixmapDataDictC PtrT PixmapPtrT '"PtrT.h"' '"PixmapC.h"'
PixmapNameDictC.h :	ODictC.clas
	$(BUILD_DICT_H) O PixmapNameDictC StringC PixmapPtrT '"StringC.h"' '"PixmapC.h"'
SignalDictC.h :	ODictC.clas
	$(BUILD_DICT_H) O SignalDictC int SignalCallsC '"SignalCallsC.h"'
StringDictC.h :	ODictC.clas
	$(BUILD_DICT_H) O StringDictC StringC StringC '"StringC.h"'

CallbackListC.C :	PListC.meth
	$(BUILD_LIST_C) P CallbackListC CallbackC
ColumnListC.C :	PListC.meth
	$(BUILD_LIST_C) P ColumnListC ColumnC
ExpElemListC.C :	PListC.meth
	$(BUILD_LIST_C) P ExpElemListC ExpElemC
FieldListC.C :	PListC.meth
	$(BUILD_LIST_C) P FieldListC FieldC
IntListC.C :	OListC.meth
	$(BUILD_LIST_C) O IntListC int
LabelDataListC.C :	OListC.meth
	$(BUILD_LIST_C) O LabelDataListC LabelDataC
PixelListC.C :	OListC.meth
	$(BUILD_LIST_C) O PixelListC Pixel
PtrListC.C :	OListC.meth
	$(BUILD_LIST_C) O PtrListC PtrT
RangeListC.C :	OListC.meth
	$(BUILD_LIST_C) O RangeListC RangeC
RowColChildListC.C :	PListC.meth
	$(BUILD_LIST_C) P RowColChildListC RowColChildC
RowOrColListC.C :	PListC.meth
	$(BUILD_LIST_C) P RowOrColListC RowOrColC
SortElemListC.C :	PListC.meth
	$(BUILD_LIST_C) P SortElemListC SortElemC
SortKeyListC.C :	PListC.meth
	$(BUILD_LIST_C) P SortKeyListC SortKeyC
StringListC.C :	OListC.meth
	$(BUILD_LIST_C) O StringListC StringC
VItemListC.C :	PListC.meth
	$(BUILD_LIST_C) P VItemListC VItemC
ViewListC.C :	PListC.meth
	$(BUILD_LIST_C) P ViewListC ViewC
WidgetListC.C :	OListC.meth
	$(BUILD_LIST_C) O WidgetListC Widget

IconDataDictC.C :	PDictC.meth
	$(BUILD_DICT_C) P IconDataDictC VItemC IconDataC
ItemDataDictC.C :	PDictC.meth
	$(BUILD_DICT_C) P ItemDataDictC VItemC ItemDataC
PixmapDataDictC.C :	ODictC.meth
	$(BUILD_DICT_C) O PixmapDataDictC PtrT PixmapPtrT
PixmapNameDictC.C :	ODictC.meth
	$(BUILD_DICT_C) O PixmapNameDictC StringC PixmapPtrT
SignalDictC.C :	ODictC.meth
	$(BUILD_DICT_C) O SignalDictC int SignalCallsC
StringDictC.C :	ODictC.meth
	$(BUILD_DICT_C) O StringDictC StringC StringC


#----------------------------------------------------------------------------
# Dependencies
#

$(OBJS) : $(TOP)/config.h

depend : $(SRC) $(HTEMPLATES)
	echo "# Generated automatically by \`make depend'" > $(DEPENDFILE)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -I$(TOP) $(GUI_FLAGS) -MM $(SRC) | sed 's/\.o:/\$$(O)\ :/g' >> $(DEPENDFILE)

include $(DEPENDFILE)
