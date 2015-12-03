TOP = ..
include $(TOP)/Make.conf

.SUFFIXES : .c $(O)

#
# Suffix rules
#
.c$(O) :
	$(CC) $(CFLAGS) -c $< -o $@

PROGS = rt2ps et2ps

PSPROLOG = prolog.h

all : $(PROGS)

install : $(PROGS)
	$(MKINSTALLDIRS) $(bindir)
	for i in $?; do $(INSTALL_PROGRAM) -s $$i $(bindir); done

clean :
	$(RM) $(PROGS) *.o *.bak junk *~

distclean : clean
	$(RM) $(TEMPLATES) $(PSPROLOG) paginate.ps

srcdistro : $(PSPROLOG)

#----------------------------------------------------------------------------
# paginate.ps.verbose is the PostScript source code for the et2ps and rt2ps
# filters. When it is modified, the pstrip command "strips" it, to make it
# much smaller and (almost) impossible for a human to read. Then, the mkincl
# command turns it into a static data structure in an include file, prolog.h,
# which is compiled into rt2ps and et2ps.
#

$(PSPROLOG) : paginate.ps	
	$(SCRIPTS)/mkincl < paginate.ps > $@

paginate.ps : paginate.ps.verbose
	$(SCRIPTS)/pstrip < paginate.ps.verbose > $@

#----------------------------------------------------------------------------
# et2ps and rt2ps
#

rt2ps : rt2ps$(O)
	$(CC) $(CFLAGS) $? -o $@

et2ps : et2ps$(O)
	$(CC) $(CFLAGS) $? -o $@

rt2ps$(O) : $(PSPROLOG)

et2ps$(O) : $(PSPROLOG)

