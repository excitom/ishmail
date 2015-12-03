TOP = ..

include $(TOP)/Make.conf


MANPAGES = man1/ishmail.1

all : $(MANPAGES)

install : $(MANPAGES)
	$(MKINSTALLDIRS) $(mandir)
	$(MKINSTALLDIRS) $(mandir)/man1
	for i in $?; do $(INSTALL_DATA) $$i $(mandir)/man1; done

clean : dummy
	
distclean : clean

dummy :
