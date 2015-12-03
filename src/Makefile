TOP = ..
MAKECONF = $(TOP)/Make.conf
include $(MAKECONF)

DEPENDFILE = Make.dep

.SUFFIXES : .c .C $(O)

#----------------------------------------------------------------------------
# Suffix rules
#

.C$(O) :
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -I$(TOP) -I$(HGLINCDIR) $(GUI_FLAGS) -c $< -o $@
.c$(O) :
	$(CC)  $(CFLAGS)   $(CPPFLAGS) -I$(TOP) -I$(HGLINCDIR) $(GUI_FLAGS) -c $< -o $@


PROG = ishmail$(EXE)

VERSION_C = version.c
VERSION_O = version$(O)

DATE_Y = date.y
DATE_C = date.c
DATE_O = date$(O)

HGLLIB = $(HGLSRC)/libhgl.a


CSRC	= \
	AddressC.C \
	AddrMisc.C \
	AlertPrefC.C \
	AlertPrefWinC.C \
	AliasPrefC.C \
	AliasPrefWinC.C \
	AppPrefC.C \
	AppPrefWinC.C \
	AutoFilePrefC.C \
	AutoFileWinC.C \
	Base64.C \
	ButtonEditWinC.C \
	ButtonEntryC.C \
	ButtonMgrC.C \
	ButtPrefWinC.C \
	CompPrefC.C \
	CompPrefWinC.C \
	ComplexImapFindWinC.C \
	ComplexMsgFindWinC.C \
	ConfPrefC.C \
	ConfPrefWinC.C \
	DictEditWinC.C \
	EntryEditWinC.C \
	FileChooserWinC.C \
	FileFolderC.C \
	FileMisc.C \
	FileMsgC.C \
	FilePartMsgC.C \
	FolderC.C \
	FolderPrefC.C \
	FolderPrefWinC.C \
	FontPrefC.C \
	FontPrefWinC.C \
	Fork.C \
	HeadPrefC.C \
	HeadPrefWinC.C \
	HeaderC.C \
	HeaderValC.C \
	IconPrefC.C \
	IconPrefWinC.C \
	ImapFolderC.C \
	ImapMisc.C \
	ImapMsgC.C \
	ImapServerC.C \
	IncludeWinC.C \
	IshAppC.C \
	IshAppP.C \
	LocalTextWinC.C \
	LoginWinC.C \
	Mailcap.C \
	MailcapC.C \
	MailFile.C \
	MailPrefC.C \
	MailPrefWinC.C \
	MailSortKeyC.C \
	MainButtPrefC.C \
	MainButtWinC.C \
	MainFile.C \
	MainFolder.C \
	MainMsg.C \
	MainOpt.C \
	MainWinC.C \
	MainWinP.C \
	MhFolderC.C \
	MhMsgC.C \
	MimeEncode.C \
	MimeIconC.C \
	MimeTypes.C \
	Misc.C \
	MmdfFolderC.C \
	MmdfMsgC.C \
	MsgC.C \
	MsgFindExp.C \
	MsgFindWinC.C \
	MsgItemC.C \
	MsgPartC.C \
	OptWinC.C \
	ParamC.C \
	PartialMsgC.C \
	PickAliasWinC.C \
	PipeWinC.C \
	PrefC.C \
	PrintWinC.C \
	Query.C \
	QuickMenu.C \
	QuotedP.C \
	ReadButtPrefC.C \
	ReadButtWinC.C \
	ReadFile.C \
	ReadIconC.C \
	ReadOpt.C \
	ReadPart.C \
	ReadPrefC.C \
	ReadPrefWinC.C \
	ReadReply.C \
	ReadWinC.C \
	ReadWinP.C \
	ReplyPrefC.C \
	ReplyPrefWinC.C \
	SafeSystem.C \
	SaveMgrC.C \
	SavePrefC.C \
	SavePrefWinC.C \
	SendButtPrefC.C \
	SendButtWinC.C \
	SendEdit.C \
	SendFile.C \
	SendIconC.C \
	SendInc.C \
	SendMisc.C \
	SendOpt.C \
	SendWinC.C \
	SendWinP.C \
	ShellExp.C \
	SigPrefC.C \
	SigPrefWinC.C \
	SortMgrC.C \
	SortPrefC.C \
	SortPrefWinC.C \
	SumFieldC.C \
	SumPrefC.C \
	SumPrefWinC.C \
	UndelWinC.C \
	UnixFolderC.C \
	UnixMsgC.C \
	edit.C \
	main.C

#----------------------------------------------------------------------------
# Template files
#

CTEMPLATES = \
	ButtonEntryListC.C \
	FolderListC.C \
	MailcapListC.C \
	MsgListC.C \
	PartialMsgDictC.C \
	PidListC.C \
	RuleDictC.C

HTEMPLATES = \
	ButtonEntryListC.h \
	FolderListC.h \
	MailcapListC.h \
	MsgListC.h \
	PartialMsgDictC.h \
	PidListC.h \
	RuleDictC.h


COBJ	= \
	AddressC$(O) \
	AddrMisc$(O) \
	AlertPrefC$(O) \
	AlertPrefWinC$(O) \
	AliasPrefC$(O) \
	AliasPrefWinC$(O) \
	AppPrefC$(O) \
	AppPrefWinC$(O) \
	AutoFilePrefC$(O) \
	AutoFileWinC$(O) \
	Base64$(O) \
	ButtonEditWinC$(O) \
	ButtonEntryC$(O) \
	ButtonEntryListC$(O) \
	ButtonMgrC$(O) \
	ButtPrefWinC$(O) \
	CompPrefC$(O) \
	CompPrefWinC$(O) \
	ComplexImapFindWinC$(O) \
	ComplexMsgFindWinC$(O) \
	ConfPrefC$(O) \
	ConfPrefWinC$(O) \
	DictEditWinC$(O) \
	EntryEditWinC$(O) \
	FileChooserWinC$(O) \
	FileFolderC$(O) \
	FileMisc$(O) \
	FileMsgC$(O) \
	FilePartMsgC$(O) \
	FolderC$(O) \
	FolderListC$(O) \
	FolderPrefC$(O) \
	FolderPrefWinC$(O) \
	FontPrefC$(O) \
	FontPrefWinC$(O) \
	Fork$(O) \
	HeaderC$(O) \
	HeaderValC$(O) \
	HeadPrefC$(O) \
	HeadPrefWinC$(O) \
	IconPrefC$(O) \
	IconPrefWinC$(O) \
	ImapFolderC$(O) \
	ImapMisc$(O) \
	ImapMsgC$(O) \
	ImapServerC$(O) \
	IncludeWinC$(O) \
	IshAppC$(O) \
	IshAppP$(O) \
	LocalTextWinC$(O) \
	LoginWinC$(O) \
	Mailcap$(O) \
	MailcapC$(O) \
	MailcapListC$(O) \
	MailFile$(O) \
	MailPrefC$(O) \
	MailPrefWinC$(O) \
	MailSortKeyC$(O) \
	MainButtPrefC$(O) \
	MainButtWinC$(O) \
	MainFile$(O) \
	MainFolder$(O) \
	MainMsg$(O) \
	MainOpt$(O) \
	MainWinC$(O) \
	MainWinP$(O) \
	MhFolderC$(O) \
	MhMsgC$(O) \
	MimeEncode$(O) \
	MimeIconC$(O) \
	MimeTypes$(O) \
	Misc$(O) \
	MmdfFolderC$(O) \
	MmdfMsgC$(O) \
	MsgC$(O) \
	MsgListC$(O) \
	MsgFindExp$(O) \
	MsgFindWinC$(O) \
	MsgItemC$(O) \
	MsgPartC$(O) \
	OptWinC$(O) \
	PartialMsgC$(O) \
	PartialMsgDictC$(O) \
	ParamC$(O) \
	PickAliasWinC$(O) \
	PidListC$(O) \
	PipeWinC$(O) \
	PrefC$(O) \
	PrintWinC$(O) \
	Query$(O) \
	QuickMenu$(O) \
	QuotedP$(O) \
	ReadButtPrefC$(O) \
	ReadButtWinC$(O) \
	ReadFile$(O) \
	ReadIconC$(O) \
	ReadOpt$(O) \
	ReadPart$(O) \
	ReadPrefC$(O) \
	ReadPrefWinC$(O) \
	ReadReply$(O) \
	ReadWinC$(O) \
	ReadWinP$(O) \
	ReplyPrefC$(O) \
	ReplyPrefWinC$(O) \
	RuleDictC$(O) \
	SafeSystem$(O) \
	SaveMgrC$(O) \
	SavePrefC$(O) \
	SavePrefWinC$(O) \
	SendButtPrefC$(O) \
	SendButtWinC$(O) \
	SendEdit$(O) \
	SendFile$(O) \
	SendIconC$(O) \
	SendInc$(O) \
	SendMisc$(O) \
	SendOpt$(O) \
	SendWinC$(O) \
	SendWinP$(O) \
	ShellExp$(O) \
	SigPrefC$(O) \
	SigPrefWinC$(O) \
	SortMgrC$(O) \
	SortPrefC$(O) \
	SortPrefWinC$(O) \
	SumFieldC$(O) \
	SumPrefC$(O) \
	SumPrefWinC$(O) \
	UndelWinC$(O) \
	UnixFolderC$(O) \
	UnixMsgC$(O) \
	edit$(O) \
	main$(O)

cSRC	= \
	getadate.c \
	sun2mime.c

cOBJ	= \
	sun2mime$(O) \
	getadate$(O) \
	$(DATE_O) \
	$(VERSION_O)

TEMPLATES = $(CTEMPLATES) $(HTEMPLATES)

SRC = $(CSRC) $(CTEMPLATES) $(cSRC) $(DATE_C) $(VERSION_C)
OBJ = $(COBJ) $(cOBJ)

#----------------------------------------------------------------------------
# Targets
#

all : $(PROG)


ishmail$(EXE) : $(OBJ) $(HGLLIB)
	$(CXX) $(OBJ) $(HGLLIB) $(LDFLAGS) $(GUI_LIBS) $(M_LIB) \
	$(OPENSSL_LIBS) $(DMALLOC_LIB) -o $@

templates : $(TEMPLATES)

install : $(PROG)
	$(MKINSTALLDIRS) $(bindir)
	$(INSTALL_PROGRAM) -s $? $(bindir)

clean :
	$(RM) *.o $(PROG) *.bak *~

distclean : clean
	$(RM) $(TEMPLATES) tags

tags : $(SRC)
	ctags $(SRC)

srcdistro : $(TEMPLATES) $(DATE_C) $(DEPENDFILE)

#----------------------------------------------------------------------------
# version.c
#

$(VERSION_C) : $(MAKECONF) $(CSRC) $(cSRC)
	echo "char  *version        = \"$(ISHVER)-`date +%Y%m%d`-$(host)\";"  > $(VERSION_C)
	echo "char  *versionNumber  = \"$(ISHVER)\";"  >> $(VERSION_C)
	echo "char  *versionIshHome = \"$(ISHHOME)\";" >> $(VERSION_C)
#----------------------------------------------------------------------------
# date.c
#

$(DATE_C) : $(DATE_Y)
	$(YACC) $?
	$(MV) y.tab.c $@

#----------------------------------------------------------------------------
# template build rules
#

ButtonEntryListC.h : $(HGLSRC)/PListC.clas
	$(BUILD_LIST_H) P ButtonEntryListC ButtonEntryC '"ButtonEntryC.h"'
FolderListC.h : $(HGLSRC)/PListC.clas
	$(BUILD_LIST_H) P FolderListC FolderC '"FolderC.h"'
MailcapListC.h : $(HGLSRC)/PListC.clas
	$(BUILD_LIST_H) P MailcapListC MailcapC '"MailcapC.h"'
MsgListC.h : $(HGLSRC)/PListC.clas
	$(BUILD_LIST_H) P MsgListC MsgC '"MsgC.h"'

ButtonEntryListC.C : $(HGLSRC)/PListC.meth
	$(BUILD_LIST_C) P ButtonEntryListC ButtonEntryC
FolderListC.C : $(HGLSRC)/PListC.meth
	$(BUILD_LIST_C) P FolderListC FolderC
MailcapListC.C : $(HGLSRC)/PListC.meth
	$(BUILD_LIST_C) P MailcapListC MailcapC
MsgListC.C : $(HGLSRC)/PListC.meth
	$(BUILD_LIST_C) P MsgListC MsgC

PidListC.h : $(HGLSRC)/PListC.clas
	$(BUILD_LIST_H) O PidListC pid_t '<sys/types.h>'

PidListC.C : $(HGLSRC)/PListC.meth
	$(BUILD_LIST_C) O PidListC pid_t

PartialMsgDictC.h : $(HGLSRC)/ODictC.clas
	$(BUILD_DICT_H) O PartialMsgDictC StringC PartialMsgPtr '<hgl/StringC.h>' '"PartialMsgC.h"'
RuleDictC.h : $(HGLSRC)/ODictC.clas
	$(BUILD_DICT_H) O RuleDictC RegexC StringC '<hgl/RegexC.h>' '<hgl/StringC.h>'

PartialMsgDictC.C : $(HGLSRC)/ODictC.meth
	$(BUILD_DICT_C) O PartialMsgDictC StringC PartialMsgPtr
RuleDictC.C : $(HGLSRC)/ODictC.meth
	$(BUILD_DICT_C) O RuleDictC RegexC StringC


#----------------------------------------------------------------------------
# Dependencies
#

$(OBJ) : $(TOP)/config.h

depend : $(SRC) $(HTEMPLATES)
	echo "# Generated automatically by \`make depend'" > $(DEPENDFILE)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -I$(TOP) $(GUI_FLAGS) -MM $(SRC) | sed 's/\.o:/\$$(O)\ :/g' >> $(DEPENDFILE)

include $(DEPENDFILE)
