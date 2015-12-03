/*
 *  $Id: TextMisc.C,v 1.2 2000/05/07 12:26:11 fnevgeny Exp $
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

#include <config.h>
#include "TextMisc.h"
#include "CharC.h"
#include "MemMap.h"
#include "StringListC.h"
#include "rsrc.h"

#include <Xm/Text.h>
#include <Xm/TextF.h>

#include <unistd.h>

#define BUFLEN  1023
static char	buffer[BUFLEN+1];
static char	null = 0;

static XtTranslations  delLeftTextTrans   = NULL;
static XtTranslations  delRightTextTrans  = NULL;
static XtTranslations  emacsTextTrans     = NULL;

static XtTranslations  delLeftFieldTrans  = NULL;
static XtTranslations  delRightFieldTrans = NULL;
static XtTranslations  emacsFieldTrans    = NULL;

/*--------------------------------------------------------------------------
 *  Create translation tables for custom translations
 */

static void
ParseTranslations()
{
//
// These translations handle the behavior of the "delete" key.  People are
//   so particular...
//
      delRightTextTrans = XtParseTranslationTable(
"~Ctrl ~Shift   <Key>osfDelete	:	kill-next-character()		\n\
 !Ctrl	        <Key>osfDelete	:	kill-next-word()		\n\
 !Shift	        <Key>osfDelete	:	kill-next-character()		\n\
 !Ctrl Shift	<Key>osfDelete	:	kill-to-end-of-line()");

      delLeftTextTrans = XtParseTranslationTable(
"~Ctrl ~Shift   <Key>osfDelete	:	kill-previous-character()	\n\
 !Ctrl	        <Key>osfDelete	:	kill-previous-word()		\n\
 !Shift	        <Key>osfDelete	:	kill-previous-character()	\n\
 !Ctrl Shift	<Key>osfDelete	:	kill-to-start-of-line()");

//
// XmTextField doesn't have the kill- actions
//
      delRightFieldTrans = XtParseTranslationTable(
"~Ctrl ~Shift   <Key>osfDelete	:	delete-next-character()		\n\
 !Ctrl	        <Key>osfDelete	:	delete-next-word()		\n\
 !Shift	        <Key>osfDelete	:	delete-next-character()		\n\
 !Ctrl Shift	<Key>osfDelete	:	delete-to-end-of-line()");

      delLeftFieldTrans = XtParseTranslationTable(
"~Ctrl ~Shift   <Key>osfDelete	:	delete-previous-character()	\n\
 !Ctrl	        <Key>osfDelete	:	delete-previous-word()		\n\
 !Shift	        <Key>osfDelete	:	delete-previous-character()	\n\
 !Ctrl Shift	<Key>osfDelete	:	delete-to-start-of-line()");

//
// These translations mimic emacs and override any default translations
//
      emacsTextTrans = XtParseTranslationTable(
"!Ctrl		<Key>b		:	backward-character()		\n\
 !Mod1		<Key>b		:	backward-word()			\n\
 !Ctrl 		<Key>f		:	forward-character()		\n\
 !Mod1		<Key>f		:	forward-word()			\n\
 !Ctrl 		<Key>a		:	beginning-of-line()		\n\
 !Ctrl		<Key>e		:	end-of-line()			\n\
 !Ctrl		<Key>p		:	previous-line()			\n\
 !Mod1		<Key>bracketleft:	backward-paragraph()		\n\
 !Ctrl		<Key>n		:	next-line()			\n\
 !Mod1		<Key>bracketright:	forward-paragraph()		\n\
 !Mod1		<Key>comma	:	beginning-of-file()		\n\
 !Mod1		<Key>period	:	end-of-file()			\n\
 !Ctrl		<Key>z		:	scroll-one-line-up()		\n\
 !Mod1		<Key>z		:	scroll-one-line-down()		\n\
 !Ctrl		<Key>d		:	kill-next-character()		\n\
 !Ctrl		<Key>k		:	kill-to-end-of-line()		\n\
 !Ctrl Shift	<Key>b		:	key-select(left)		\n\
 !Mod1 Shift	<Key>b		:	backward-word(extend)		\n\
 !Ctrl Shift 	<Key>f		:	key-select(right)		\n\
 !Mod1 Shift	<Key>f		:	forward-word(extend)		\n\
 !Ctrl Shift	<Key>a		:	beginning-of-line(extend)	\n\
 !Ctrl Shift	<Key>e		:	end-of-line(extend)		\n\
 !Mod1 Shift	<Key>bracketleft:	backward-paragraph(extend)	\n\
 !Mod1 Shift	<Key>bracketright:	forward-paragraph(extend)	\n\
 !Mod1 Shift	<Key>comma	:	beginning-of-file(extend)	\n\
 !Mod1 Shift	<Key>period	:	end-of-file(extend)		\n\
 !Ctrl		<Key>o		:	newline-and-backup()		\n\
 !Ctrl		<Key>j		:	newline-and-indent()		\n\
 !Ctrl		<Key>w		:	key-select() kill-selection()	\n\
 !Ctrl		<Key>g		:	process-cancel()		\n\
 !Ctrl		<Key>space	:	set-anchor()			\n\
 !Ctrl	        <Key>y		:	unkill()");

//
// TextField translations
//
      emacsFieldTrans = XtParseTranslationTable(
"!Ctrl		<Key>b		:	backward-character()		\n\
 !Mod1		<Key>b		:	backward-word()			\n\
 !Ctrl 		<Key>f		:	forward-character()		\n\
 !Mod1		<Key>f		:	forward-word()			\n\
 !Ctrl 		<Key>a		:	beginning-of-line()		\n\
 !Ctrl		<Key>e		:	end-of-line()			\n\
 !Ctrl		<Key>p		:	beginning-of-line()		\n\
 !Mod1		<Key>bracketleft:	beginning-of-line()		\n\
 !Ctrl		<Key>n		:	end-of-line()			\n\
 !Mod1		<Key>bracketright:	end-of-line()			\n\
 !Mod1		<Key>comma	:	beginning-of-line()		\n\
 !Mod1		<Key>period	:	end-of-line()			\n\
 !Ctrl		<Key>d		:	delete-next-character()		\n\
 !Ctrl		<Key>k		:	delete-to-end-of-line()		\n\
 !Ctrl Shift	<Key>b		:	key-select(left)		\n\
 !Mod1 Shift	<Key>b		:	backward-word(extend)		\n\
 !Ctrl Shift 	<Key>f		:	key-select(right)		\n\
 !Mod1 Shift	<Key>f		:	forward-word(extend)		\n\
 !Ctrl Shift	<Key>a		:	beginning-of-line(extend)	\n\
 !Ctrl Shift	<Key>e		:	end-of-line(extend)		\n\
 !Mod1 Shift	<Key>bracketleft:	beginning-of-line(extend)	\n\
 !Mod1 Shift	<Key>bracketright:	end-of-line(extend)		\n\
 !Mod1 Shift	<Key>comma	:	beginning-of-line(extend)	\n\
 !Mod1 Shift	<Key>period	:	end-of-line(extend)		\n\
 !Ctrl		<Key>w		:	key-select() delete-selection()	\n\
 !Ctrl		<Key>g		:	process-cancel()		\n\
 !Ctrl		<Key>space	:	set-anchor()");

} // End ParseTranslations

/*--------------------------------------------------------------------------
 *  Create a text field widget and look for the emacsMode and
 *     deleteMeansBackspace resources
 */

Widget
CreateTextField(Widget parent, String name, ArgList argv, Cardinal argc)
{
   Widget	tf = XmCreateTextField(parent, name, argv, argc);

   if ( !emacsTextTrans ) ParseTranslations();

//
// Apply delete key functionality
//
   Boolean delMeansBs = get_boolean("XmTextField", tf, "deleteMeansBackspace",
				    False);
   if ( delMeansBs ) XtOverrideTranslations(tf, delLeftFieldTrans);
   else		     XtOverrideTranslations(tf, delRightFieldTrans);

//
// Apply emacs overrides if necessary
//
   Boolean emacsMode = get_boolean("XmTextField", tf, "emacsMode", False);
   if ( emacsMode ) XtOverrideTranslations(tf, emacsFieldTrans);

//
// Apply user overrides if necessary
//
   StringC userTrans = get_string("XmTextField", tf, "translations");
   userTrans.Trim();

   if ( userTrans.size() > 0 ) {

      if ( userTrans.StartsWith("#override") )
	 XtOverrideTranslations(tf, XtParseTranslationTable(userTrans));
      else if ( userTrans.StartsWith("#augment") )
	 XtAugmentTranslations(tf, XtParseTranslationTable(userTrans));
      else
	 XtVaSetValues(tf, XmNtranslations, XtParseTranslationTable(userTrans),
		       NULL);
   }

   return tf;

} // End CreateTextField

/*--------------------------------------------------------------------------
 *  Create a text widget and look for the emacsMode and
 *     deleteMeansBackspace resources
 */

Widget
CreateText(Widget parent, String name, ArgList argv, Cardinal argc)
{
   Widget	text = XmCreateText(parent, name, argv, argc);

   if ( !emacsTextTrans ) ParseTranslations();

//
// Apply delete key functionality
//
   Boolean delMeansBs = get_boolean("XmText", text, "deleteMeansBackspace",
   				    False);
   if ( delMeansBs ) XtOverrideTranslations(text, delLeftTextTrans);
   else		     XtOverrideTranslations(text, delRightTextTrans);

//
// Apply emacs overrides if necessary
//
   Boolean emacsMode = get_boolean("XmText", text, "emacsMode", False);
   if ( emacsMode ) XtOverrideTranslations(text, emacsTextTrans);

//
// Apply user overrides if necessary
//
   StringC userTrans = get_string("XmText", text, "translations");
   userTrans.Trim();

   if ( userTrans.size() > 0 ) {

      if ( userTrans.StartsWith("#override") )
	 XtOverrideTranslations(text, XtParseTranslationTable(userTrans));
      else if ( userTrans.StartsWith("#augment") )
	 XtAugmentTranslations(text, XtParseTranslationTable(userTrans));
      else
	 XtVaSetValues(text, XmNtranslations,
		       XtParseTranslationTable(userTrans), NULL);
   }

   return text;

} // End CreateText

/*--------------------------------------------------------------------------
 *  Create a scrolled text widget and look for the emacsMode and
 *     deleteMeansBackspace resources
 */

Widget
CreateScrolledText(Widget parent, String name, ArgList argv, Cardinal argc)
{
   Widget	text = XmCreateScrolledText(parent, name, argv, argc);

   if ( !emacsTextTrans ) ParseTranslations();

//
// Apply delete key functionality
//
   Boolean delMeansBs = get_boolean("XmText", text, "deleteMeansBackspace",
   				    False);
   if ( delMeansBs ) XtOverrideTranslations(text, delLeftTextTrans);
   else		     XtOverrideTranslations(text, delRightTextTrans);

//
// Apply emacs overrides if necessary
//
   Boolean emacsMode = get_boolean("XmText", text, "emacsMode", False);
   if ( emacsMode ) XtOverrideTranslations(text, emacsTextTrans);

//
// Apply user overrides if necessary
//
   StringC userTrans = get_string("XmText", text, "translations");
   userTrans.Trim();

   if ( userTrans.size() > 0 ) {

      if ( userTrans.StartsWith("#override") )
	 XtOverrideTranslations(text, XtParseTranslationTable(userTrans));
      else if ( userTrans.StartsWith("#augment") )
	 XtAugmentTranslations(text, XtParseTranslationTable(userTrans));
      else
	 XtVaSetValues(text, XmNtranslations,
		       XtParseTranslationTable(userTrans), NULL);
   }

   return text;

} // End CreateText

/*--------------------------------------------------------------------------
 *  Set Text widget from character array
 */

void
TextSetString(Widget w, CharC c)
{
   if ( c.Length() == 0 ) {
      if ( XmIsTextField(w) ) XmTextFieldSetString(w, "");
      else		      XmTextSetString(w, "");
      return;
   }

//
// Use the existing string if possible
//
   if ( c.FollowedByNull() ) {
      if ( XmIsTextField(w) ) XmTextFieldSetString(w, (char*)c.Addr());
      else		      XmTextSetString(w, (char*)c.Addr());
      return;
   }

//
// Use the internal buffer if possible
//
   if ( c.Length() <= BUFLEN ) {
      strncpy(buffer, c.Addr(), c.Length());
      buffer[c.Length()] = 0;
      if ( XmIsTextField(w) ) XmTextFieldSetString(w, buffer);
      else		      XmTextSetString(w, buffer);
      return;
   }

//
// If we reached this point, we will try to write the data to a
//   temporary file so we can null-terminate it.
//
   Boolean	error = False;
   char	*tmpFile = tempnam(NULL, NULL);
   if ( !tmpFile ) error = True;

//
// Open the file
//
   int	fd;
   if ( !error ) {
      fd = open(tmpFile, O_RDWR|O_CREAT|O_TRUNC, 0600);
      if ( fd < 0 ) {
	 free(tmpFile);
	 error = True;
      }
   }

//
// Write the file and the null
//
   if ( !error ) {
      if ( !c.WriteFile(fd) || write(fd, &null, 1) != 1 ) {
	 unlink(tmpFile);
	 free(tmpFile);
	 error = True;
      }
      close(fd);
   }

//
// Map the file
//
   MappedFileC	*mf;
   if ( !error ) {
      mf = MapFile(tmpFile);
      if ( !mf ) {
	 unlink(tmpFile);
	 free(tmpFile);
	 error = True;
      }
   }

//
// Use the data from the mapped file
//
   if ( !error ) {
      if ( XmIsTextField(w) ) XmTextFieldSetString(w, (char*)mf->Addr());
      else		      XmTextSetString(w, (char*)mf->Addr());
      UnmapFile(mf);
      unlink(tmpFile);
      free(tmpFile);
      return;
   }

//
// If we reached this point, we have to use the 1K at a time method
//
   if ( XmIsText(w) ) XmTextDisableRedisplay(w);

   if ( XmIsTextField(w) ) XmTextFieldSetString(w, "");
   else			   XmTextSetString(w, "");

   const char	*str = c.Addr();
   int		rem = c.Length();
   while ( rem > 0 ) {
      int	len = MIN(rem, BUFLEN);
      strncpy(buffer, str, len);
      buffer[len] = 0;
      if ( XmIsTextField(w) )
	 XmTextFieldInsert(w, XmTextGetLastPosition(w), buffer);
      else
	 XmTextInsert(w, XmTextGetLastPosition(w), buffer);
      str += len;
      rem -= len;
   }

   if ( XmIsTextField(w) ) XmTextFieldShowPosition(w, 0);
   else			   XmTextShowPosition(w, 0);
   if ( XmIsText(w) ) XmTextEnableRedisplay(w);

} // End TextSetString

/*--------------------------------------------------------------------------
 *  Insert character array in text widget
 */

void
TextInsert(Widget w, XmTextPosition pos, CharC c)
{
   if ( c.Length() == 0 ) return;

//
// Use the existing string if possible
//
   if ( c.FollowedByNull() ) {
      if ( XmIsTextField(w) ) XmTextFieldInsert(w, pos, (char*)c.Addr());
      else		      XmTextInsert(w, pos, (char*)c.Addr());
      return;
   }

//
// Use the internal buffer if possible
//
   if ( c.Length() <= BUFLEN ) {
      strncpy(buffer, c.Addr(), c.Length());
      buffer[c.Length()] = 0;
      if ( XmIsTextField(w) ) XmTextFieldInsert(w, pos, buffer);
      else		      XmTextInsert(w, pos, buffer);
      return;
   }

//
// If we reached this point, we will try to write the data to a
//   temporary file so we can null-terminate it.
//
   Boolean	error = False;
   char	*tmpFile = tempnam(NULL, NULL);
   if ( !tmpFile ) error = True;

//
// Open the file
//
   int	fd;
   if ( !error ) {
      fd = open(tmpFile, O_RDWR|O_CREAT|O_TRUNC, 0600);
      if ( fd < 0 ) {
	 free(tmpFile);
	 error = True;
      }
   }

//
// Write the file and the null
//
   if ( !error ) {
      if ( !c.WriteFile(fd) || write(fd, &null, 1) != 1 ) {
	 unlink(tmpFile);
	 free(tmpFile);
	 error = True;
      }
      close(fd);
   }

//
// Map the file
//
   MappedFileC	*mf;
   if ( !error ) {
      mf = MapFile(tmpFile);
      if ( !mf ) {
	 unlink(tmpFile);
	 free(tmpFile);
	 error = True;
      }
   }

//
// Use the data from the mapped file
//
   if ( !error ) {
      if ( XmIsTextField(w) ) XmTextFieldInsert(w, pos, (char*)mf->Addr());
      else		      XmTextInsert(w, pos, (char*)mf->Addr());
      UnmapFile(mf);
      unlink(tmpFile);
      free(tmpFile);
      return;
   }

//
// If we reached this point, we have to use the 1K at a time method
//
   if ( XmIsText(w) ) XmTextDisableRedisplay(w);

   const char	*str = c.Addr();
   int		rem = c.Length();
   while ( rem > 0 ) {
      int	len = MIN(rem, BUFLEN);
      strncpy(buffer, str, len);
      buffer[len] = 0;
      if ( XmIsTextField(w) ) XmTextFieldInsert(w, pos, buffer);
      else		      XmTextInsert(w, pos, buffer);
      pos += len;
      str += len;
      rem -= len;
   }

   if ( XmIsText(w) ) XmTextEnableRedisplay(w);

} // End TextInsert

/*--------------------------------------------------------------------------
 *  Set Text widget from string list
 */

void
TextSetList(Widget w, const StringListC& list)
{
   StringC	tmp;
   unsigned	count = list.size();
   for (int i=0; i<count; i++) {
      if ( i > 0 ) tmp += '\n';
      tmp += *list[i];
   }

   XmTextSetString(w, tmp);

} // End TextSetList

/*--------------------------------------------------------------------------
 *  Set string list from text widget
 */

void
TextGetList(Widget w, StringListC& list)
{
   char		*cs = XmTextGetString(w);
   CharC	src(cs);

   StringC	tmp;
   u_int	offset = 0;
   CharC	entry = src.NextWord(offset, "\n");
   while ( entry.Length() > 0 ) {
      tmp = entry;
      list.add(tmp);
      offset = entry.Addr() - src.Addr() + 1;
      entry = src.NextWord(offset, "\n");
   }

   XtFree(cs);

} // End TextSetList
