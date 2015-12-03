TOP = ..

include $(TOP)/Make.conf


DATA = \
ishdecrypt \
ishencrypt \
showaudio \
showextern \
showimage \
showpgp \
showpgpsign \
statrun \
xtermexec \
customize.pl \
statChk.pl


all : $(DATA)

install : $(DATA)
	$(MKINSTALLDIRS) $(ISHHOME)/libexec
	for i in $?; do $(INSTALL) $$i $(ISHHOME)/libexec; done

clean : dummy
	
distclean : clean

dummy :
