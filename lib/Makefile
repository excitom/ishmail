TOP = ..

include $(TOP)/Make.conf


DATA = \
Ishmail \
Ishmail-mono \
flagdown \
flagup \
mailcap \
welcome \
Ishmail.colors \
Ishmail.colors-mono \
Ishmail.fonts \
Ishmail.help \
Ishmail.hlp \
Ishmail.lang \
thumb_down.mask \
thumb_up.mask \
Ishmail.misc \
mime.types \
display-0.xbm \
pin-in.xbm \
pin-out.xbm \
retrieval-0.xbm \
audio.xpm \
thumb_down.xbm \
thumb_up.xbm \
thumb_down2.xbm \
thumb_up2.xbm \
binary.xpm \
box.xpm \
edit-0.xpm \
edit-1.xpm \
edit-2.xpm \
edit-3.xpm \
edit-4.xpm \
edit-5.xpm \
edit-6.xpm \
edit-7.xpm \
gif.xpm \
heart-0.xpm \
heart-1.xpm \
ishmail.xpm \
jpg.xpm \
mpg.xpm \
new_mail.xpm \
postscript.xpm \
richtext.xpm \
system-closed-no-mail.xpm \
system-open-new-mail.xpm \
system-open-no-mail.xpm \
text.xpm \
truck-0.xpm \
truck-1.xpm \
truck-2.xpm \
truck-3.xpm \
truck-4.xpm \
truck-5.xpm \
truck-6.xpm \
truck-7.xpm \
truck-8.xpm


all : $(DATA)

install : $(DATA)
	$(MKINSTALLDIRS) $(ISHHOME)/lib
	for i in $?; do $(INSTALL_DATA) $$i $(ISHHOME)/lib; done

clean : dummy
	
distclean : clean

dummy :
