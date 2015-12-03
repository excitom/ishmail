TOP = .

include $(TOP)/Make.conf

SUBDIRS = hgl src utils libexec lib man doc

subdirs :
	@set -e; for i in $(SUBDIRS); do (cd $$i; $(MAKE)) || exit 1; done

all : subdirs

install : subdirs
	@set -e; for i in $(SUBDIRS); do (cd $$i; $(MAKE) install) || exit 1; done

clean :
	@set -e; for i in $(SUBDIRS); do (cd $$i; $(MAKE) clean) || exit 1; done

distclean :
	@set -e; for i in $(SUBDIRS); do (cd $$i; $(MAKE) distclean) || exit 1; done
	$(RM) config.log config.cache config.status config.h Make.conf

configure : ac-tools/configure.in ac-tools/aclocal.m4
	autoconf ac-tools/configure.in > configure
	chmod +x configure

Make.conf : ac-tools/Make.conf.in configure
	@echo
	@echo 'Please re-run ./configure'
	@echo
	@exit 1

ChangeLog : 
	$(SCRIPTS)/cvs2cl.pl -F trunk

dummy :

