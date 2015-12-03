/*
 *  $Id: HelpResWinC.C,v 1.3 2000/08/07 10:58:16 evgeny Exp $
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

#include "HelpResWinC.h"
#include "WArgList.h"
#include "WXmString.h"
#include "TextMisc.h"
#include "rsrc.h"
#include "HalAppC.h"
#include "MemMap.h"
#include "FieldViewC.h"
#include "VItemC.h"
#include "ButtonBox.h"
#include "HelpC.h"
#include "HelpDbC.h"
#include "SysErr.h"
#include "red.xpm"
#include "green.xpm"
#include "yellow.xpm"

#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/PushB.h>
#include <Xm/Separator.h>
#include <Xm/TextF.h>
#include <Xm/Frame.h>
#include <Xm/RowColumn.h>
#include <Xm/ArrowB.h>
#include <Xm/MessageB.h>
#include <Xm/SelectioB.h>
#include <Xm/Protocols.h>
#include <Xm/AtomMgr.h>

#include <X11/cursorfont.h>
#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

extern int debuglev;

static HelpResWinC	*win = NULL;

#define BIG_DIST	(1<<30)
#define QUICK_FIELD	0
#define CONT_FIELD	1
#define HELP_FIELD	2

/*---------------------------------------------------------------
 *  Function to build a default name for the widget
 */

static void
GetShortName(Widget w, StringC& name)
{
//
// Build name of the form "*shell*parent.widget"
//

   Widget	par   = XtParent(w);
   Widget	shell = par;
   while ( shell && !XtIsShell(shell) ) shell = XtParent(shell);

   name.Clear();

   if ( shell ) {

      if ( shell != (Widget)*halApp )
	 name += "*";

      if ( shell != par ) {
	 name += XtName(shell);
	 name += '*';
      }
   }

   if ( par ) {
      name += XtName(par);
      name += '.';
   }

   name += XtName(w);

} // End GetShortName

/*---------------------------------------------------------------
 *  Function to build the full name for the widget
 */

static void
GetLongName(Widget w, StringC& name)
{
   name = XtName(w);
   Widget	par = XtParent(w);
   CharC	dot(".");
   while ( par ) {
      name = XtName(par) + dot + name;
      par = XtParent(par);
   }

} // End GetLongName

/*---------------------------------------------------------------
 *  Class to represent down a line in a resource file
 */

class ResEntryC {
public:

   StringC	full;		// Complete line in resource file
   CharC	rspec;		// First part of line including resource
   CharC	wspec;		// First part of line without resource
   CharC	res;		// Resource part
   CharC	val;		// Second part of line
   char		binding;	// dot or star

   ResEntryC() {}

   ResEntryC(const ResEntryC& re) {
      *this = re;
   }

   ResEntryC(CharC c) {
      *this = c;
   }

   ResEntryC& operator=(const ResEntryC& re) {
      full = re.full;
      Parse();
      return *this;
   }

   ResEntryC& operator=(CharC c) {
      full = c;
      Parse();
      return *this;
   }

   void		SetVal(CharC v) {
      StringC	tmp = rspec;
      tmp += ":\t";
      tmp += v;
      full = tmp;
      Parse();
   }

   void		SetWspec(CharC w) {
      StringC	tmp = w;
      tmp += binding;
      tmp += res;
      tmp += ":\t";
      tmp += val;
      full = tmp;
      Parse();
   }

   void		SetRspec(CharC r) {
      StringC	tmp = r;
      tmp += ":\t";
      tmp += val;
      full = tmp;
      Parse();
   }

   void		Parse();
   Boolean	AppliesTo(Widget w, Widget *wsrc=NULL, int *dist=NULL);
};

/*---------------------------------------------------------------
 *  Class to show how a resource entry applies to a specific widget
 */

class EntryDataC {
public:

   ResEntryC	*entry;	// Pointer to resource entry
   Widget	w;	// Widget that entry corresponds to (could be ancestor)
   int		dist;	// Generations to ancestor

   EntryDataC() {
      Clear();
   }

   EntryDataC(const EntryDataC& ed) {
      *this = ed;
   }

   EntryDataC& operator=(const EntryDataC& ed) {
      entry = ed.entry;
      w     = ed.w;
      dist  = ed.dist;
      return *this;
   }

   void	Clear() {
      entry = NULL;
      w     = NULL;
      dist  = BIG_DIST;
   }

   void	Reset() {

//
// Remove entry from any lists
//
      win->quickList2.remove(entry);
      win->contList2.remove(entry);
      win->helpList2.remove(entry);

//
// Delete entry
//
      delete entry;
      Clear();
   }

   void	Init(Widget wid, PtrList2& list);
};

/*---------------------------------------------------------------
 *  Class to show how all resource entries apply to a specific widget
 */

class RecDataC {
public:

   EntryDataC	quick;	// Info for quickHelp resource
   EntryDataC	cont;	// Info for contextHelp resource
   EntryDataC	help;	// Info for helpcard resource

   RecDataC() {}

   RecDataC(const RecDataC& rd) {
      *this = rd;
   }

   RecDataC& operator=(const RecDataC& rd) {
      quick = rd.quick;
      cont  = rd.cont;
      help  = rd.help;
      return *this;
   }
};

/*---------------------------------------------------------------
 *  Class to represent a widget in the interface
 */

class WidgetRecC {
public:

   Widget	w;		// This widget
   StringC	shortName;	// *shell*parent.widget
   StringC	longName;	// All ancestors
   VItemC	*item;		// View item in view box
   RecDataC	orig;		// As found in Xrm database
   RecDataC	edit;		// As modified by user

   WidgetRecC() {
      w    = NULL;
      item = NULL;
   }

   WidgetRecC(const WidgetRecC& wr) {
      item = NULL;
      *this = wr;
   }

   WidgetRecC(Widget w) {
      item = NULL;
      *this = w;
   }

   WidgetRecC& operator=(const WidgetRecC& wr) {
      w		= wr.w;
      shortName = wr.shortName;
      longName  = wr.longName;
      orig	= wr.orig;
      edit	= wr.edit;
      return *this;
   }

   WidgetRecC& operator=(Widget wid) {
      w	= wid;
      GetShortName(w, shortName);
      GetLongName (w, longName);
      FindEntries();
      UpdateItem();
      return *this;
   }

   void		FindEntries();
   void		UpdateItem();

   ResEntryC	*QuickEntry() {
      return (edit.quick.entry ? edit.quick.entry : orig.quick.entry);
   }
   EntryDataC	*QuickData() {
      return (edit.quick.entry ? &edit.quick : &orig.quick);
   }

   ResEntryC	*ContEntry() {
      return (edit.cont.entry ? edit.cont.entry : orig.cont.entry);
   }
   EntryDataC	*ContData() {
      return (edit.cont.entry ? &edit.cont : &orig.cont);
   }

   ResEntryC	*HelpEntry() {
      return (edit.help.entry ? edit.help.entry : orig.help.entry);
   }
   EntryDataC	*HelpData() {
      return (edit.help.entry ? &edit.help : &orig.help);
   }

   Boolean	Modified() {
      return (edit.quick.entry || edit.cont.entry || edit.help.entry);
   }

   void		Reset() {
      edit.quick.Reset();
      edit.cont.Reset();
      edit.help.Reset();
   }
};

/*---------------------------------------------------------------
 *  Parse a resource line into its components
 */

void
ResEntryC::Parse()
{
//
// Break at colon
//
   int	pos = full.PosOf(':');
   rspec = full(0,pos);
   val   = full(pos+1, full.size());
   val.Trim();

//
// Remove resource part after last dot or star
//
   int	dpos = rspec.RevPosOf('.');
   int	spos = rspec.RevPosOf('*');
   if ( dpos >= 0 && spos >= 0 ) pos = MAX(dpos, spos);
   else if ( dpos >= 0 )         pos = dpos;
   else			         pos = spos;

   if ( pos >= 0 ) {
      binding = rspec[pos];
      res     = rspec(pos+1,rspec.Length());
      wspec   = rspec(0,pos);
   }
   else {
      wspec = rspec;
      binding = '.';
   }

} // End Parse

/*---------------------------------------------------------------
 *  See if a resource line applies to a widget.  If it does, return the
 *     widget from which it was inherited and the number of generations
 *     of separation
 */

Boolean
ResEntryC::AppliesTo(Widget w, Widget *wsrc, int *dist)
{
   if ( wsrc ) *wsrc = NULL;
   if ( dist ) *dist = 0;

   char	*name = XtName(w);

   if ( binding == '.' ) {

//
// Must end with widget name
//
      if ( !wspec.EndsWith(name) ) return False;
   }

   else if ( binding == '*' ) {

//
// Must end with name of an ancestor
//
      while ( w && !wspec.EndsWith(name) ) {
	 if ( dist ) (*dist)++;
	 w = XtParent(w);
	 if ( w ) name = XtName(w);
      }

      if ( !w ) return False;
   }

   else
      return False;

   if ( wsrc ) *wsrc = w;

//
// Now examine rest of spec to make sure it matches
//
   CharC	spec = wspec;
   spec.CutEnd(strlen(name));

   while ( spec.Length() > 0 ) {

      char	sep = spec.LastChar();
      if ( sep == '.' ) {

	 spec.CutEnd(1);

//
// Next component must be immediate parent
//
	 w = XtParent(w);
	 if ( !w ) return False;

	 name = XtName(w);
	 if ( !spec.EndsWith(name) ) return False;

	 spec.CutEnd(strlen(name));
      }

      else if ( sep == '*' ) {

	 spec.CutEnd(1);

//
// Next component must be any ancestor or NULL
//
	 if ( spec.Length() == 0 ) return True;

//
// Extract next component
//
	 int	dpos = spec.RevPosOf('.');
	 int	spos = spec.RevPosOf('*');
	 int	pos  = dpos;
	 if ( pos >= 0 ) pos = MAX(dpos,spos);
	 else		 pos = spos;

	 CharC	comp;
	 if ( pos < 0 ) comp = spec;
	 else		comp = spec(pos+1,spec.Length());

//
// Look for an ancestor widget with this name
//
	 Widget	par = XtParent(w);
	 while ( par && XtName(par) != comp ) par = XtParent(par);
	 if ( !par ) return False;

	 w = par;
	 spec.CutEnd(comp.Length());

      } // End if separator is '*'

      else
	 return False;

   } // End for each element in widget spec

   return True;

} // End AppliesTo

/*---------------------------------------------------------------
 *  Look for matching resources for a widget in a resource list
 */

void
EntryDataC::Init(Widget wid, PtrList2& list)
{
   Clear();

//
// Loop through resource entries
//
   u_int	count   = list.size();
   for (int i=0; i<count; i++) {

      ResEntryC	*ent = (ResEntryC*)list[i];
      int	wdist;
      Widget	wsrc;

//
// See if this entry applies
//
      if ( ent->AppliesTo(wid, &wsrc, &wdist) && wdist < dist ) {
	 entry = ent;
	 w     = wsrc;
	 dist  = wdist;
      }
   }

} // End EntryDataC::Init

/*---------------------------------------------------------------
 *  Find all resources for our widget
 */

void
WidgetRecC::FindEntries()
{
//
// Look up original entries.  If there is a matching entry in the 2 list,
//   it could come from this widget or from an ancestor.  If it comes from
//   an ancestor, it is not an edit.
//
   edit.quick.Init(w, win->quickList2);
   orig.quick.Init(w, win->quickList);
   if ( edit.quick.w != w ) {
      if ( edit.quick.dist < orig.quick.dist ) orig.quick = edit.quick;
      edit.quick.Clear();
   }

   edit.cont.Init(w, win->contList2);
   orig.cont.Init(w, win->contList);
   if ( edit.cont.w != w ) {
      if ( edit.cont.dist < orig.cont.dist ) orig.cont = edit.cont;
      edit.cont.Clear();
   }

   edit.help.Init(w, win->helpList2);
   orig.help.Init(w, win->helpList);
   if ( edit.cont.w != w ) {
      if ( edit.cont.dist < orig.cont.dist ) orig.cont = edit.cont;
      edit.cont.Clear();
   }

} // End FindEntries

/*---------------------------------------------------------------
 *  Update the fields in the view item
 */

void
WidgetRecC::UpdateItem()
{
   if ( !item ) return;

   item->SetFieldTag(QUICK_FIELD, edit.quick.entry ? "bold" : "plain");
   item->SetFieldTag(CONT_FIELD,  edit.cont.entry  ? "bold" : "plain");
   item->SetFieldTag(HELP_FIELD,  edit.help.entry  ? "bold" : "plain");

   static char	*no  = "No";
   static char	*na  = "N/A";
   static char	*yes = "Yes";
   static char	*in  = "Inherit";

   XpmT	pm = green_xpm;
   char	*tmp = no;
   EntryDataC	*data = QuickData();
   if ( data->entry ) {
      if ( data->w == w ) tmp = yes;
      else	          tmp = in;
   }
   item->Field(QUICK_FIELD, tmp);
   if ( tmp == no ) pm = red_xpm;

   tmp = no;
   data = ContData();
   if ( data->entry ) {
      if ( data->w == w ) tmp = yes;
      else	          tmp = in;
   }
   item->Field(CONT_FIELD, tmp);
   if ( tmp == no ) pm = red_xpm;

   tmp = no;
   data = HelpData();
   if ( data->entry ) {
      if ( data->w == w ) tmp = yes;
      else	          tmp = in;
   }
   else if ( !data->w || !XmIsPushButton(data->w) ) {
      tmp = na;
   }
   item->Field(HELP_FIELD, tmp);

//
// This is used so rarely that it isn't very useful to highlight where it is
//    missing
//   if ( tmp == no ) pm = red_xpm;

   if ( Modified() ) pm = yellow_xpm;
   item->SetPixmaps(NULL, pm);

} // End UpdateItem

/*-----------------------------------------------------------------
 * Constructor
 */

HelpResWinC::HelpResWinC(Widget parent) : HalTopLevelC("helpResWin", parent)
{
   win = this;

   WArgList	args;

   widgetList.AllowDuplicates(FALSE);
   quickList.AllowDuplicates(FALSE);
   contList.AllowDuplicates(FALSE);
   helpList.AllowDuplicates(FALSE);
   quickList2.AllowDuplicates(FALSE);
   contList2.AllowDuplicates(FALSE);
   helpList2.AllowDuplicates(FALSE);

   curWidget = NULL;
   changed   = False;
   selfMod   = False;

//
// appForm
//
// Form		shortNameForm
// Form		longNameForm
// Separator	quickSep
// Form		quickForm
// Separator	contSep
// Form		contForm
// Separator	helpSep
// Form		helpForm
// Separator	cmdSep
// Frame	cmdFrame
// Separator	widgetSep
// VBoxC	widgetBox
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_NONE);
   Widget	shortNameForm = XmCreateForm(appForm, "shortNameForm", ARGS);

   args.TopAttachment(XmATTACH_WIDGET, shortNameForm);
   Widget	longNameForm = XmCreateForm(appForm, "longNameForm", ARGS);

   args.TopAttachment(XmATTACH_WIDGET, longNameForm);
   Widget	quickSep = XmCreateSeparator(appForm, "quickSep", ARGS);

   args.TopAttachment(XmATTACH_WIDGET, quickSep);
   Widget	quickForm = XmCreateForm(appForm, "quickForm", ARGS);

   args.TopAttachment(XmATTACH_WIDGET, quickForm);
   Widget	contSep = XmCreateSeparator(appForm, "contSep", ARGS);

   args.TopAttachment(XmATTACH_WIDGET, contSep);
   Widget	contForm = XmCreateForm(appForm, "contForm", ARGS);

   args.TopAttachment(XmATTACH_WIDGET, contForm);
   Widget	helpSep = XmCreateSeparator(appForm, "helpSep", ARGS);

   args.TopAttachment(XmATTACH_WIDGET, helpSep);
   Widget	helpForm = XmCreateForm(appForm, "helpForm", ARGS);

   args.TopAttachment(XmATTACH_WIDGET, helpForm);
   Widget	cmdSep = XmCreateSeparator(appForm, "cmdSep", ARGS);

   args.TopAttachment(XmATTACH_WIDGET, cmdSep);
   Widget	cmdFrame = XmCreateFrame(appForm, "cmdFrame", ARGS);

   args.TopAttachment(XmATTACH_WIDGET, cmdFrame);
   Widget	widgetSep = XmCreateSeparator(appForm, "widgetSep", ARGS);

   args.TopAttachment(XmATTACH_WIDGET, widgetSep);
   args.BottomAttachment(XmATTACH_FORM);
   widgetBox = new VBoxC(appForm, "widgetBox", ARGS);

   args.Reset();
   args.ShadowThickness(0);
   args.MarginWidth(0);
   args.MarginHeight(0);
   XtSetValues(cmdFrame, ARGS);

//
// widgetBox
//
// FieldViewC	widgetView
//
   widgetView     = new FieldViewC(widgetBox);
   widgetViewType = widgetBox->AddView(*widgetView);
   //widgetBox->HideStatus();

//
// shortNameForm
//
// Label	shortNameLabel
// TextField	shortNameTF
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   Widget shortNameLabel = XmCreateLabel(shortNameForm, "shortNameLabel", ARGS);

   args.LeftAttachment(XmATTACH_WIDGET, shortNameLabel);
   args.RightAttachment(XmATTACH_FORM);
   args.Editable(False);
   shortNameTF = CreateTextField(shortNameForm, "shortNameTF", ARGS);

   XtManageChild(shortNameLabel);
   XtManageChild(shortNameTF);

//
// longNameForm
//
// Label	longNameLabel
// TextField	longNameTF
// ArrowButton	longNameAB
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   Widget longNameLabel = XmCreateLabel(longNameForm, "longNameLabel", ARGS);

   args.LeftAttachment(XmATTACH_NONE);
   args.RightAttachment(XmATTACH_FORM);
   Widget longNameAB = XmCreateArrowButton(longNameForm, "longNameAB", ARGS);

   args.LeftAttachment(XmATTACH_WIDGET, longNameLabel);
   args.RightAttachment(XmATTACH_WIDGET, longNameAB);
   args.Editable(False);
   longNameTF = CreateTextField(longNameForm, "longNameTF", ARGS);

   XtManageChild(longNameLabel);
   XtManageChild(longNameAB);
   XtManageChild(longNameTF);

   XtAddCallback(longNameAB, XmNactivateCallback, (XtCallbackProc)DoParentLoad,
   		 this);

//
// quickForm
//
// Form	quickHelpForm
// Form	quickNameForm
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_NONE);
   Widget quickHelpForm = XmCreateForm(quickForm, "quickHelpForm", ARGS);

   args.TopAttachment(XmATTACH_WIDGET, quickHelpForm);
   Widget quickNameForm = XmCreateForm(quickForm, "quickNameForm", ARGS);

//
// quickHelpForm
//
// Label	quickHelpLabel
// TextField	quickHelpTF
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   Widget quickHelpLabel = XmCreateLabel(quickHelpForm, "quickHelpLabel", ARGS);

   args.LeftAttachment(XmATTACH_WIDGET, quickHelpLabel);
   args.RightAttachment(XmATTACH_FORM);
   quickHelpTF = CreateTextField(quickHelpForm, "quickHelpTF", ARGS);

   XtManageChild(quickHelpLabel);
   XtManageChild(quickHelpTF);

//
// quickNameForm
//
// Label	quickNameLabel
// TextField	quickNameTF
// PushButton	quickLoadPB
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   Widget quickNameLabel = XmCreateLabel(quickNameForm, "quickNameLabel",
   					   ARGS);

   args.LeftAttachment(XmATTACH_NONE);
   args.RightAttachment(XmATTACH_FORM);
   quickLoadPB = XmCreatePushButton(quickNameForm,"quickLoadPB", ARGS);
   XtAddCallback(quickLoadPB, XmNactivateCallback,
   		 (XtCallbackProc)DoQuickLoad, this);

   args.LeftAttachment(XmATTACH_WIDGET, quickNameLabel);
   args.RightAttachment(XmATTACH_WIDGET, quickLoadPB);
   args.Editable(False);
   quickNameTF = CreateTextField(quickNameForm, "quickNameTF", ARGS);

   XtManageChild(quickNameLabel);
   XtManageChild(quickLoadPB);
   XtManageChild(quickNameTF);

   XtManageChild(quickHelpForm);
   XtManageChild(quickNameForm);

//
// contForm
//
// Form	contCardForm
// Form	contNameForm
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_NONE);
   Widget contCardForm = XmCreateForm(contForm, "contCardForm", ARGS);

   args.TopAttachment(XmATTACH_WIDGET, contCardForm);
   Widget contNameForm = XmCreateForm(contForm, "contNameForm", ARGS);

//
// contCardForm
//
// Label	contCardLabel
// TextField	contCardTF
// PushButton	contCardPB
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   Widget contCardLabel = XmCreateLabel(contCardForm, "contCardLabel", ARGS);

   args.LeftAttachment(XmATTACH_NONE);
   args.RightAttachment(XmATTACH_FORM);
   contCardPB = XmCreatePushButton(contCardForm, "contCardPB", ARGS);
   XtAddCallback(contCardPB, XmNactivateCallback,
   		 (XtCallbackProc)DoContCard, this);

   args.LeftAttachment(XmATTACH_WIDGET, contCardLabel);
   args.RightAttachment(XmATTACH_WIDGET, contCardPB);
   contCardTF = CreateTextField(contCardForm, "contCardTF", ARGS);

   XtManageChild(contCardLabel);
   XtManageChild(contCardPB);
   XtManageChild(contCardTF);

//
// contNameForm
//
// Label	contNameLabel
// TextField	contNameTF
// PushButton	contLoadPB
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   Widget contNameLabel = XmCreateLabel(contNameForm, "contNameLabel",
   					   ARGS);

   args.LeftAttachment(XmATTACH_NONE);
   args.RightAttachment(XmATTACH_FORM);
   contLoadPB = XmCreatePushButton(contNameForm,"contLoadPB", ARGS);
   XtAddCallback(contLoadPB, XmNactivateCallback,
   		 (XtCallbackProc)DoContLoad, this);

   args.LeftAttachment(XmATTACH_WIDGET, contNameLabel);
   args.RightAttachment(XmATTACH_WIDGET, contLoadPB);
   args.Editable(False);
   contNameTF = CreateTextField(contNameForm, "contNameTF", ARGS);

   XtManageChild(contNameLabel);
   XtManageChild(contLoadPB);
   XtManageChild(contNameTF);

   XtManageChild(contCardForm);
   XtManageChild(contNameForm);

//
// helpForm
//
// Form	helpCardForm
// Form	helpNameForm
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_NONE);
   Widget helpCardForm = XmCreateForm(helpForm, "helpCardForm", ARGS);

   args.TopAttachment(XmATTACH_WIDGET, helpCardForm);
   Widget helpNameForm = XmCreateForm(helpForm, "helpNameForm", ARGS);

//
// helpCardForm
//
// Label	helpCardLabel
// TextField	helpCardTF
// PushButton	helpCardPB
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   Widget helpCardLabel = XmCreateLabel(helpCardForm, "helpCardLabel", ARGS);

   args.LeftAttachment(XmATTACH_NONE);
   args.RightAttachment(XmATTACH_FORM);
   helpCardPB = XmCreatePushButton(helpCardForm, "helpCardPB", ARGS);
   XtAddCallback(helpCardPB, XmNactivateCallback,
   		 (XtCallbackProc)DoHelpCard, this);

   args.LeftAttachment(XmATTACH_WIDGET, helpCardLabel);
   args.RightAttachment(XmATTACH_WIDGET, helpCardPB);
   helpCardTF = CreateTextField(helpCardForm, "helpCardTF", ARGS);

   XtManageChild(helpCardLabel);
   XtManageChild(helpCardPB);
   XtManageChild(helpCardTF);

//
// helpNameForm
//
// Label	helpNameLabel
// TextField	helpNameTF
// PushButton	helpLoadPB
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_FORM);
   Widget helpNameLabel = XmCreateLabel(helpNameForm, "helpNameLabel",
   					   ARGS);

   args.LeftAttachment(XmATTACH_NONE);
   args.RightAttachment(XmATTACH_FORM);
   helpLoadPB = XmCreatePushButton(helpNameForm,"helpLoadPB", ARGS);
   XtAddCallback(helpLoadPB, XmNactivateCallback,
   		 (XtCallbackProc)DoHelpLoad, this);

   args.LeftAttachment(XmATTACH_WIDGET, helpNameLabel);
   args.RightAttachment(XmATTACH_WIDGET, helpLoadPB);
   args.Editable(False);
   helpNameTF = CreateTextField(helpNameForm, "helpNameTF", ARGS);

   XtManageChild(helpNameLabel);
   XtManageChild(helpLoadPB);
   XtManageChild(helpNameTF);

   XtManageChild(helpCardForm);
   XtManageChild(helpNameForm);

//
// Add change detection callbacks
//
   XtAddCallback(quickHelpTF, XmNvalueChangedCallback,
		 (XtCallbackProc)QuickTextChanged, this);
   XtAddCallback(contCardTF, XmNvalueChangedCallback,
		 (XtCallbackProc)ContTextChanged, this);
   XtAddCallback(helpCardTF, XmNvalueChangedCallback,
		 (XtCallbackProc)HelpTextChanged, this);

//
// Align labels
//
   Dimension	wd, max;
   XtVaGetValues(shortNameLabel, XmNwidth, &max, NULL);
   XtVaGetValues(longNameLabel,  XmNwidth, &wd,  NULL); if ( wd>max ) max = wd;
   XtVaGetValues(quickHelpLabel, XmNwidth, &wd,  NULL); if ( wd>max ) max = wd;
   XtVaGetValues(quickNameLabel, XmNwidth, &wd,  NULL); if ( wd>max ) max = wd;
   XtVaGetValues(contCardLabel,  XmNwidth, &wd,  NULL); if ( wd>max ) max = wd;
   XtVaGetValues(contNameLabel,  XmNwidth, &wd,  NULL); if ( wd>max ) max = wd;
   XtVaGetValues(helpCardLabel,  XmNwidth, &wd,  NULL); if ( wd>max ) max = wd;
   XtVaGetValues(helpNameLabel,  XmNwidth, &wd,  NULL); if ( wd>max ) max = wd;

   args.Reset();
   args.Width(max);
   args.Resizable(False);
   XtSetValues(shortNameLabel, ARGS);
   XtSetValues(longNameLabel,  ARGS);
   XtSetValues(quickHelpLabel, ARGS);
   XtSetValues(quickNameLabel, ARGS);
   XtSetValues(contCardLabel,  ARGS);
   XtSetValues(contNameLabel,  ARGS);
   XtSetValues(helpCardLabel,  ARGS);
   XtSetValues(helpNameLabel,  ARGS);

//
// Align buttons
//
   XtVaGetValues(quickLoadPB,  XmNwidth, &max, NULL);
   XtVaGetValues(contCardPB,   XmNwidth, &wd,  NULL); if (wd>max) max = wd;
   XtVaGetValues(contLoadPB,   XmNwidth, &wd,  NULL); if (wd>max) max = wd;
   XtVaGetValues(helpCardPB,   XmNwidth, &wd,  NULL); if (wd>max) max = wd;
   XtVaGetValues(helpLoadPB,   XmNwidth, &wd,  NULL); if (wd>max) max = wd;

   args.Reset();
   args.Width(max);
   args.Resizable(False);
   XtSetValues(quickLoadPB,  ARGS);
   XtSetValues(contCardPB,   ARGS);
   XtSetValues(contLoadPB,   ARGS);
   XtSetValues(helpCardPB,   ARGS);
   XtSetValues(helpLoadPB,   ARGS);

//
// cmdFrame
//
//    RowColumn		cmdRC
//       Label		modLabel1
//       PushButton	pickPB
//       PushButton	nextPB
//       PushButton	prevPB
//       PushButton	resetPB
//       PushButton	delPB
//       Label		modLabel2
//
   args.Reset();
   args.Orientation(XmHORIZONTAL);
   args.ChildType(XmFRAME_TITLE_CHILD);
   args.ChildHorizontalAlignment(XmALIGNMENT_CENTER);
   args.ChildHorizontalSpacing(0);
   args.ChildVerticalAlignment(XmALIGNMENT_WIDGET_TOP);
   args.EntryAlignment(XmALIGNMENT_CENTER);
   Widget	cmdRC = XmCreateRowColumn(cmdFrame, "cmdRC", ARGS);

   modLabel1 = XmCreateLabel(cmdRC, "modLabel", 0,0);
   XtManageChild(modLabel1);

   Widget pickPB = XmCreatePushButton(cmdRC, "pickPB", 0,0);
   XtAddCallback(pickPB, XmNactivateCallback, (XtCallbackProc)DoWidgetPick,
   		 this);
   XtManageChild(pickPB);

   Widget nextPB = XmCreatePushButton(cmdRC, "nextPB", 0,0);
   XtAddCallback(nextPB, XmNactivateCallback, (XtCallbackProc)DoWidgetNext,
   		 this);
   XtManageChild(nextPB);

   Widget prevPB = XmCreatePushButton(cmdRC, "prevPB", 0,0);
   XtAddCallback(prevPB, XmNactivateCallback, (XtCallbackProc)DoWidgetPrev,
   		 this);
   XtManageChild(prevPB);

   Widget resetPB = XmCreatePushButton(cmdRC, "resetPB", 0,0);
   XtAddCallback(resetPB, XmNactivateCallback, (XtCallbackProc)DoWidgetReset,
   		 this);
   XtManageChild(resetPB);

#if 0
   Widget delPB = XmCreatePushButton(cmdRC, "delPB", 0,0);
   XtAddCallback(delPB, XmNactivateCallback, (XtCallbackProc)DoWidgetDel,
   		 this);
   XtManageChild(delPB);
#endif

   modLabel2 = XmCreateLabel(cmdRC, "modLabel", 0,0);
   XtManageChild(modLabel2);

   XtManageChild(cmdRC);

   XtManageChild(shortNameForm);
   XtManageChild(longNameForm);
   XtManageChild(quickSep);
   XtManageChild(quickForm);
   XtManageChild(contSep);
   XtManageChild(contForm);
   XtManageChild(helpSep);
   XtManageChild(helpForm);
   XtManageChild(cmdSep);
   XtManageChild(cmdFrame);
   XtManageChild(widgetSep);
   XtManageChild(*widgetBox);

   AddButtonBox();

#if 0
   Widget	loadSomePB = XmCreatePushButton(buttonRC, "loadSomePB", 0,0);
   XtManageChild(loadSomePB);
   XtAddCallback(loadSomePB, XmNactivateCallback, (XtCallbackProc)DoLoadSome,
   		 this);

   Widget	loadAllPB = XmCreatePushButton(buttonRC, "loadAllPB", 0,0);
   XtManageChild(loadAllPB);
   XtAddCallback(loadAllPB, XmNactivateCallback, (XtCallbackProc)DoLoadAll,
   		 this);
#endif

   Widget	savePB = XmCreatePushButton(buttonRC, "savePB", 0,0);
   XtManageChild(savePB);
   XtAddCallback(savePB, XmNactivateCallback, (XtCallbackProc)DoSave, this);

   Widget	donePB = XmCreatePushButton(buttonRC, "donePB", 0,0);
   XtManageChild(donePB);
   XtAddCallback(donePB, XmNactivateCallback, (XtCallbackProc)DoDone, this);

   ShowInfoMsg();
   HandleHelp();

   pickCursor = XCreateFontCursor(halApp->display, XC_hand1);

   XtSetSensitive(quickLoadPB, False);
   XtSetSensitive(contCardPB,  False);
   XtSetSensitive(contLoadPB,  False);
   XtSetSensitive(helpCardPB,  False);
   XtSetSensitive(helpLoadPB,  False);

   PixelValue(modLabel1, "yellow", &modBg);
   XtVaGetValues(modLabel1, XmNbackground, &regBg, NULL);

   XtAddCallback(*this, XmNpopupCallback, (XtCallbackProc)DoPopup, this);

} // End constructor

/*-----------------------------------------------------------------
 * Destructor
 */

HelpResWinC::~HelpResWinC()
{
   if ( halApp->xRunning ) {

      Boolean	needSave;
      QuerySave(&needSave, /*cancelOk=*/False);
      if ( needSave ) DoSave(NULL, this, NULL);

      XFreeCursor(halApp->display, pickCursor);
   }

   Clear();

   u_int	count = quickList.size();
   int	i;
   for (i=0; i<count; i++) {
      ResEntryC	*ent = (ResEntryC*)quickList[i];
      delete ent;
   }

   count = contList.size();
   for (i=0; i<count; i++) {
      ResEntryC	*ent = (ResEntryC*)contList[i];
      delete ent;
   }

   count = helpList.size();
   for (i=0; i<count; i++) {
      ResEntryC	*ent = (ResEntryC*)helpList[i];
      delete ent;
   }

   delete widgetBox;
   delete widgetView;

} // End destructor

/*-----------------------------------------------------------------
 * Clear all widgets and modifications
 */

void
HelpResWinC::Clear()
{
   VItemListC&	items = widgetBox->Items();
   u_int	count = items.size();
   int	i;
   for (i=0; i<count; i++) delete items[i];

   count = widgetList.size();
   for (i=0; i<count; i++) {
      WidgetRecC	*rec = (WidgetRecC*)widgetList[i];
      delete rec;
   }
   widgetList.removeAll();

   curWidget = NULL;

} // End Clear

/*-----------------------------------------------------------------
 * Final setup
 */

void
HelpResWinC::DoPopup(Widget, HelpResWinC *This, XtPointer)
{
   XtRemoveCallback(*This, XmNpopupCallback, (XtCallbackProc)DoPopup, This);

   StringListC	titleList;
   titleList.add("quickHelp");
   titleList.add("contextHelp");
   titleList.add("helpcard");
   titleList.add("Widget Type");
   titleList.add("Short Name");
   titleList.add("Long Name");
   This->widgetView->SetTitles(titleList);

   This->widgetBox->ViewType(This->widgetViewType);

//
// Hide "modified" flag
//
   Pixel	bg;
   XtVaGetValues(This->modLabel1, XmNbackground, &bg, NULL);
   XtVaSetValues(This->modLabel1, XmNforeground,  bg, NULL);
   XtVaSetValues(This->modLabel2, XmNforeground,  bg, NULL);

   XtAppAddTimeOut(halApp->context, 0, (XtTimerCallbackProc)FinishInit, This);

} // End DoPopup

/*-----------------------------------------------------------------
 * Initialization after window is displayed
 */

void
HelpResWinC::FinishInit(HelpResWinC *This, XtIntervalId*)
{
   This->BusyCursor(True);

//
// Load the resource database
//
   This->Message("Reading resource database");
   char		*dbFile = tempnam(NULL, "db.");
   XrmDatabase	db      = XtDatabase(halApp->display);
   XrmPutFileDatabase(db, dbFile);

   MappedFileC	*mf = MapFile(dbFile);
   if ( mf ) {
      This->GetResLines(mf->data);
      UnmapFile(mf);
   }

   unlink(dbFile);
   free(dbFile);

   This->ClearMessage();
   This->BusyCursor(False);

} // End FinishInit

/*---------------------------------------------------------------
 *  Function to read the help resource lines from the given text data
 */

void
HelpResWinC::GetResLines(CharC data)
{
   StringC	base("Reading resource database: ");
   StringC	msg;
   int		count = 0;

   StringC	line;
   CharC	word;
   u_int	offset = 0;
   int		pos = data.PosOf('\n', offset);
   while ( pos >= 0 ) {

      word = data(offset, (u_int)pos-offset);
      offset = pos + 1;

//
// If the newline is escaped, read some more.
//
      if ( word.EndsWith("\\") ) {
	 word.CutEnd(1);
	 line += word;
      }
      else {

	 line += word;

	 if ( line.Contains("quickHelp:") ||
	      line.Contains("contextHelp:") ||
	      line.Contains("helpcard:") ) {

	    ResEntryC	*ent = new ResEntryC(line);
	    count++;
	    msg = base;
	    msg += count;
	    Message(msg);

	    if      ( line.Contains("quickHelp:")   ) quickList.add(ent);
	    else if ( line.Contains("contextHelp:") ) contList.add(ent);
	    else				      helpList.add(ent);
	 }

	 line.Clear();
      }

      pos = data.PosOf('\n', offset);

   } // End for each line in file

} // End GetResLines

/*---------------------------------------------------------------
 *  Method to add a widget item for the specified widget
 */

void
HelpResWinC::LoadWidget(Widget w, Boolean all)
{
   if ( !w || !XtIsWidget(w) ) return;

   if ( debuglev > 1 ) cout <<"Checking " <<w <<" (" <<XtName(w) <<")" <<endl;

   if ( all || XmIsPrimitive(w) || XmIsDrawingArea(w) ) {

      StringC	msg("Found widget: ");
      msg += XtName(w);
      Message(msg);

      WidgetRecC	*rec = new WidgetRecC(w);
      widgetList.add(rec);

//
// Create a view item and set fields
//
      rec->item = new VItemC(rec->longName);
      rec->item->SetUserData(rec);
      rec->item->AddOpenCallback((CallbackFn*)OpenWidget, this);

      StringListC	fieldList;
      fieldList.AllowDuplicates(TRUE);

      StringC	tmp("");
      fieldList.add(tmp);
      fieldList.add(tmp);
      fieldList.add(tmp);

      WidgetClass	wc = XtClass(w);
      tmp = wc->core_class.class_name;
      fieldList.add(tmp);

      fieldList.add(rec->shortName);
      fieldList.add(rec->longName);
      rec->item->FieldList(fieldList);

      rec->UpdateItem();

      widgetBox->AddItem(*rec->item);

   } // End if widget should be displayed

//
// If this is a composite, loop through the children and check them.
//
   if ( XtIsComposite(w) ) {

      WidgetList	list;
      Cardinal		count;
      XtVaGetValues(w, XmNnumChildren, &count, XmNchildren, &list, NULL);
      if ( debuglev > 1 ) cout <<"   has " <<count <<" children" <<endl;

//
// Loop through children.  Don't go into other shells.
//
      for (int i=0; i<count; i++) {
	 Widget	w = list[i];
	 if ( !XtIsShell(w) ) LoadWidget(w, all);
      }

   } // End if this is a composite widget
   
#if 0
//
// If there are any popups associated with this widget, check them
//
   if ( w->core.num_popups > 0 ) {

      if ( debuglev > 1 )
	 cout <<"   has " <<w->core.num_popups <<" popups" <<endl;
      for (int i=0; i<w->core.num_popups; i++)
	 LoadWidget(w->core.popup_list[i], all);

   } // End if there are any popups
#endif

} // End LoadWidget

/*---------------------------------------------------------------
 *  Callback to handle double-click on widget item
 */

void
HelpResWinC::OpenWidget(VItemC *item, HelpResWinC *This)
{
   WidgetRecC	*rec = (WidgetRecC*)item->UserData();
   This->ShowWidgetInfo(rec);
}

/*---------------------------------------------------------------
 *  Method to find the record associated with a widget
 */

WidgetRecC*
HelpResWinC::GetWidgetRec(Widget w)
{
//
// Look up widget rec
//
   u_int	count = widgetList.size();
   for (int i=0; i<count; i++) {
      WidgetRecC	*rec = (WidgetRecC*)widgetList[i];
      if ( rec->w == w ) return rec;
   }

   return NULL;
}

/*---------------------------------------------------------------
 *  Method to display resources for the specified widget
 */

void
HelpResWinC::ShowWidgetInfo(Widget w)
{
//
// Look up record
//
   WidgetRecC	*rec = GetWidgetRec(w);
   ShowWidgetInfo(rec);
}

/*---------------------------------------------------------------
 *  Method to display resources for the specified widget
 */

void
HelpResWinC::ShowWidgetInfo(WidgetRecC *rec)
{
   selfMod = True;

//
// Save current info if it has been changed
//
   if ( curWidget && changed ) {

//
// Update widget view box
//
      UpdateWidget(curWidget->w);
      ClearMessage();

   } // End if there is a current widget and it has changed

   if ( rec ) {

      XmTextFieldSetString(shortNameTF, rec->shortName);
      XmTextFieldSetString(longNameTF,  rec->longName);

      EntryDataC*	data = rec->QuickData();
      if ( data->entry ) {
	 TextFieldSetString(quickHelpTF, data->entry->val);
	 TextFieldSetString(quickNameTF, data->entry->wspec);
      }
      else {
	 XmTextFieldSetString(quickHelpTF, "");
	 XmTextFieldSetString(quickNameTF, "");
      }
      XtSetSensitive(quickLoadPB, data->w && data->w != rec->w);

      data = rec->ContData();
      if ( data->entry ) {
	 TextFieldSetString(contCardTF, data->entry->val);
	 TextFieldSetString(contNameTF, data->entry->wspec);
	 XtSetSensitive(contCardPB, data->entry->val.Length() > 0);
      }
      else {
	 XmTextFieldSetString(contCardTF, "");
	 XmTextFieldSetString(contNameTF, "");
	 XtSetSensitive(contCardPB, False);
      }
      XtSetSensitive(contLoadPB, data->w && data->w != rec->w);

      if ( XmIsPushButton(rec->w) ) {
	 data = rec->HelpData();
	 if ( data->entry ) {
	    TextFieldSetString(helpCardTF, data->entry->val);
	    TextFieldSetString(helpNameTF, data->entry->wspec);
	    XtSetSensitive(helpCardPB, data->entry->val.Length() > 0);
	 }
	 else {
	    XmTextFieldSetString(helpCardTF, "");
	    XmTextFieldSetString(helpNameTF, "");
	    XtSetSensitive(helpCardPB, False);
	 }
	 XtSetSensitive(helpLoadPB, data->w && data->w != rec->w);
	 XtVaSetValues(helpCardTF, XmNeditable, True, NULL);
      }
      else {
	 XmTextFieldSetString(helpCardTF, "N/A");
	 XmTextFieldSetString(helpNameTF, "N/A");
	 XtSetSensitive(helpCardPB, False);
	 XtSetSensitive(helpLoadPB, False);
	 XtVaSetValues(helpCardTF, XmNeditable, False, NULL);
      }

      widgetBox->SelectItemOnly(*rec->item);
      widgetView->ScrollToItem(*rec->item);

//
// Update "modified" label
//
      Pixel	bg = rec->Modified() ? modBg : regBg;
      XtVaSetValues(modLabel1, XmNforeground, bg, NULL);
      XtVaSetValues(modLabel2, XmNforeground, bg, NULL);

   } // End if there is a current record

   else {

      XmTextFieldSetString(shortNameTF, "");
      XmTextFieldSetString(longNameTF,  "");
      XmTextFieldSetString(quickHelpTF, "");
      XmTextFieldSetString(quickNameTF, "");
      XtSetSensitive(quickLoadPB, False);

      XmTextFieldSetString(contCardTF, "");
      XmTextFieldSetString(contNameTF, "");
      XtSetSensitive(contCardPB, False);
      XtSetSensitive(contLoadPB, False);

      XmTextFieldSetString(helpCardTF, "N/A");
      XmTextFieldSetString(helpNameTF, "N/A");
      XtSetSensitive(helpCardPB, False);
      XtSetSensitive(helpLoadPB, False);
      XtVaSetValues(helpCardTF, XmNeditable, False, NULL);

      widgetBox->DeselectAllItems();

//
// Update "modified" label
//
      XtVaSetValues(modLabel1, XmNforeground, regBg, NULL);
      XtVaSetValues(modLabel2, XmNforeground, regBg, NULL);

   } // End if no new record

   XmTextFieldShowPosition(shortNameTF,XmTextFieldGetLastPosition(shortNameTF));
   XmTextFieldShowPosition(longNameTF, XmTextFieldGetLastPosition(longNameTF));
   XmTextFieldShowPosition(quickNameTF,XmTextFieldGetLastPosition(quickNameTF));
   XmTextFieldShowPosition(contNameTF, XmTextFieldGetLastPosition(contNameTF));
   XmTextFieldShowPosition(helpNameTF, XmTextFieldGetLastPosition(helpNameTF));

   curWidget = rec;
   changed   = False;
   selfMod   = False;

} // End ShowWidgetInfo

/*---------------------------------------------------------------
 *  Method to update the fields for a widget item
 */

void
HelpResWinC::UpdateWidget(Widget w)
{
   if ( !w || !XtIsWidget(w) ) return;

   StringC	msg("Updating widget: ");
   msg += XtName(w);
   Message(msg);

//
// Look up widget rec
//
   WidgetRecC	*rec = GetWidgetRec(w);
   if ( rec ) {
      rec->FindEntries();
      rec->UpdateItem();
   }

//
// If this is a composite, loop through the children and check them.
//
   if ( XtIsComposite(w) ) {

      WidgetList	list;
      Cardinal		count;
      XtVaGetValues(w, XmNnumChildren, &count, XmNchildren, &list, NULL);
      if ( debuglev > 1 ) cout <<"   has " <<count <<" children" <<endl;

//
// Loop through children.  Don't go into other shells.
//
      for (int i=0; i<count; i++) {
	 Widget	w = list[i];
	 if ( !XtIsShell(w) ) UpdateWidget(w);
      }

   } // End if this is a composite widget
   
} // End UpdateWidget

/*-----------------------------------------------------------------
 * Callback to detect text changes to quick help text
 */

void
HelpResWinC::QuickTextChanged(Widget, HelpResWinC *This, XtPointer)
{
   if ( This->selfMod ) return;

   WidgetRecC	*rec = This->curWidget;
   if ( !rec ) return;

//
// Get new value
//
   char	*cs = XmTextFieldGetString(This->quickHelpTF);

//
// Compare the new value to the original value.  Update the edit entry if
//    necessary
//
   This->CheckResVal(rec, "quickHelp", cs, &rec->orig.quick, &rec->edit.quick,
		     This->quickList2);

   XtFree(cs);

//
// Update "modified" label
//
   Pixel	bg = rec->Modified() ? This->modBg : This->regBg;
   XtVaSetValues(This->modLabel1, XmNforeground, bg, NULL);
   XtVaSetValues(This->modLabel2, XmNforeground, bg, NULL);

//
// Update widget name if string has changed
//
   cs = XmTextFieldGetString(This->quickNameTF);
   ResEntryC	*ent = rec->QuickEntry();
   if ( ent && ent->wspec != cs )
      TextFieldSetString(This->quickNameTF, ent->wspec);
   XtFree(cs);

   This->changed = True;

} // End QuickTextChanged

/*-----------------------------------------------------------------
 * Callback to detect text changes to context help text
 */

void
HelpResWinC::ContTextChanged(Widget, HelpResWinC *This, XtPointer)
{
   if ( This->selfMod ) return;

   WidgetRecC	*rec = This->curWidget;
   if ( !rec ) return;

//
// Get new value
//
   char	*cs = XmTextFieldGetString(This->contCardTF);

//
// Compare the new value to the original value.  Update the edit entry if
//    necessary
//
   This->CheckResVal(rec, "contextHelp", cs, &rec->orig.cont, &rec->edit.cont,
		     This->contList2);

   XtFree(cs);

//
// Update "modified" label
//
   Pixel	bg = rec->Modified() ? This->modBg : This->regBg;
   XtVaSetValues(This->modLabel1, XmNforeground, bg, NULL);
   XtVaSetValues(This->modLabel2, XmNforeground, bg, NULL);

//
// Update widget name if string has changed
//
   cs = XmTextFieldGetString(This->contNameTF);
   ResEntryC	*ent = rec->ContEntry();
   if ( ent && ent->wspec != cs )
      TextFieldSetString(This->contNameTF, ent->wspec);
   XtFree(cs);

   This->changed = True;

} // End ContTextChanged

/*-----------------------------------------------------------------
 * Callback to detect text changes to helpcard help text
 */

void
HelpResWinC::HelpTextChanged(Widget, HelpResWinC *This, XtPointer)
{
   if ( This->selfMod ) return;

   WidgetRecC	*rec = This->curWidget;
   if ( !rec ) return;

//
// Get new value
//
   char	*cs = XmTextFieldGetString(This->helpCardTF);

//
// Compare the new value to the original value.  Update the edit entry if
//    necessary
//
   This->CheckResVal(rec, "helpcard", cs, &rec->orig.help, &rec->edit.help,
		     This->helpList2);

   XtFree(cs);

//
// Update "modified" label
//
   Pixel	bg = rec->Modified() ? This->modBg : This->regBg;
   XtVaSetValues(This->modLabel1, XmNforeground, bg, NULL);
   XtVaSetValues(This->modLabel2, XmNforeground, bg, NULL);

//
// Update widget name if string has changed
//
   cs = XmTextFieldGetString(This->helpNameTF);
   ResEntryC	*ent = rec->HelpEntry();
   if ( ent && ent->wspec != cs )
      TextFieldSetString(This->helpNameTF, ent->wspec);
   XtFree(cs);

   This->changed = True;

} // End HelpTextChanged

/*-----------------------------------------------------------------
 * Callback to detect text changes to quick help text
 */

void
HelpResWinC::CheckResVal(WidgetRecC *rec, CharC res, CharC val,
			 EntryDataC *orig, EntryDataC *edit, PtrList2& list2)
{
//
// See if the new value is different than the original value
//
   CharC	origVal;
   if ( orig->entry ) origVal = orig->entry->val;
   if ( val != origVal ) {

//
// If an edit entry exists, update it.
//
      if ( edit->entry ) {
	 edit->entry->SetVal(val);
      }

//
// If no edit entry exists, create one
//
      else {

	 StringC	line;

//
// If the original entry belongs to this widget, use the original resource spec
//
	 if ( orig->entry && orig->w == rec->w )
	    line = orig->entry->rspec;

//
// If the original entry belongs to another widget, create a new resource spec
//
	 else {
	    line = rec->shortName;
	    if ( XtIsComposite(rec->w) ) line += '*';
	    else			 line += '.';
	    line += res;
	 }

//
// Add new value
//
	 line += ":\t";
	 line += val;

//
// Create a new entry and add it to the alternate list
//
	 edit->entry = new ResEntryC(line);
	 edit->w     = rec->w;
	 list2.add(edit->entry);

      } // End if no edit entry exists

   } // End if new value is different than original

//
// If the new value is the same as the original value and an edit entry exists,
//    delete it
//
   else if ( edit->entry ) {
      list2.remove(edit->entry);
      delete edit->entry;
      edit->entry = NULL;
   }

} // End CheckResVal

/*-----------------------------------------------------------------
 * Callback to pick a widget and load the resources
 */

void
HelpResWinC::DoWidgetPick(Widget, HelpResWinC *This, XtPointer)
{
   XEvent ev;
   Widget w = XmTrackingEvent(*This, This->pickCursor, False, &ev);
   XmUpdateDisplay(*This);
   XFlush(halApp->display);
   XSync(halApp->display, False);

   if ( !w ) return;

   halApp->BusyCursor(True);

//
// If this widget isn't in the list, generate a list for the shell containing
//    the widget.
//
   WidgetRecC	*rec = This->GetWidgetRec(w);
   if ( !rec ) {

      Widget	shell = XtParent(w);
      while ( shell && !XtIsShell(shell) ) shell = XtParent(shell);

      This->Clear();
      This->widgetBox->RemoveAllItems();

      This->LoadWidget(shell, /*all=*/True);
      This->ClearMessage();
   }

//
// Display the info for this widget
//
   This->ShowWidgetInfo(w);

   halApp->BusyCursor(False);

} // End DoWidgetPick

/*-----------------------------------------------------------------
 * Callback to load the parent of the current widget
 */

void
HelpResWinC::DoParentLoad(Widget, HelpResWinC *This, XtPointer)
{
   if ( !This->curWidget ) return;

   Widget	par = XtParent(This->curWidget->w);
   if ( !par ) return;

   This->ShowWidgetInfo(par);
}

/*-----------------------------------------------------------------
 * Callback to load the widget specified in the quick help source widget field
 */

void
HelpResWinC::DoQuickLoad(Widget, HelpResWinC *This, XtPointer)
{
   EntryDataC*	data = This->curWidget->QuickData();
   This->ShowWidgetInfo(data->w);
}

/*-----------------------------------------------------------------
 * Callback to load the widget specified in the context help source widget field
 */

void
HelpResWinC::DoContLoad(Widget, HelpResWinC *This, XtPointer)
{
   EntryDataC*	data = This->curWidget->ContData();
   This->ShowWidgetInfo(data->w);
}

/*-----------------------------------------------------------------
 * Callback to load the widget specified in the help button source widget field
 */

void
HelpResWinC::DoHelpLoad(Widget, HelpResWinC *This, XtPointer)
{
   EntryDataC*	data = This->curWidget->HelpData();
   This->ShowWidgetInfo(data->w);
}

/*-----------------------------------------------------------------
 * Callback to view the helpcard specified in the context helpcard field
 */

void
HelpResWinC::DoContCard(Widget, HelpResWinC *This, XtPointer)
{
   This->BusyCursor(True);

   char	*cs = XmTextFieldGetString(This->contCardTF);
   StringC	name(cs);
   XtFree(cs);

   HelpCardC	*card = halApp->HelpWin()->FindCard(name);
   halApp->HelpWin()->ShowCard(card, *This);

   This->BusyCursor(False);
}

/*-----------------------------------------------------------------
 * Callback to view the helpcard specified in the help button helpcard field
 */

void
HelpResWinC::DoHelpCard(Widget, HelpResWinC *This, XtPointer)
{
   This->BusyCursor(True);

   char	*cs = XmTextFieldGetString(This->helpCardTF);
   StringC	name(cs);
   XtFree(cs);

   HelpCardC	*card = halApp->HelpWin()->FindCard(name);
   halApp->HelpWin()->ShowCard(card, *This);

   This->BusyCursor(False);
}

/*-----------------------------------------------------------------
 * Callback to show resources for next widget in list
 */

void
HelpResWinC::DoWidgetNext(Widget, HelpResWinC *This, XtPointer)
{
   VItemListC&	items = This->widgetBox->VisItems();
   u_int	icount = items.size();
   if ( icount == 0 ) return;

   VItemC	*item    = NULL;
   VItemListC&	selItems = This->widgetBox->SelItems();
   u_int	scount   = selItems.size();
   if ( scount == 0 ) {
      item = items[0];
   }

   else {
      item = selItems[scount-1];
      int	index = items.indexOf(item) + 1;
      if ( index >= icount ) index = 0;
      item = items[index];
   }

   if ( item ) {
      WidgetRecC	*rec = (WidgetRecC*)item->UserData();
      This->ShowWidgetInfo(rec);
   }
}

/*-----------------------------------------------------------------
 * Callback to show resources for previous widget in list
 */

void
HelpResWinC::DoWidgetPrev(Widget, HelpResWinC *This, XtPointer)
{
   VItemListC&	items = This->widgetBox->VisItems();
   u_int	icount = items.size();
   if ( icount == 0 ) return;

   VItemC	*item    = NULL;
   VItemListC&	selItems = This->widgetBox->SelItems();
   u_int	scount   = selItems.size();
   if ( scount == 0 ) {
      item = items[0];
   }

   else {
      item = selItems[0];
      int	index = items.indexOf(item) - 1;
      if ( index < 0 ) index = icount-1;
      item = items[index];
   }

   if ( item ) {
      WidgetRecC	*rec = (WidgetRecC*)item->UserData();
      This->ShowWidgetInfo(rec);
   }
}

/*-----------------------------------------------------------------
 * Callback to reset resources for current widget
 */

void
HelpResWinC::DoWidgetReset(Widget, HelpResWinC *This, XtPointer)
{
   if ( This->curWidget ) {
      This->curWidget->Reset();
      This->changed = True;
      This->ShowWidgetInfo(This->curWidget);
   }
}

#if 0
/*-----------------------------------------------------------------
 * Callback to delete resources for current widget
 */

void
HelpResWinC::DoWidgetDel(Widget, HelpResWinC *This, XtPointer)
{
}
#endif

/*---------------------------------------------------------------
 *  Callback routine to handle response in a question dialog
 */

static void
AnswerQuery(Widget, int *answer, XmAnyCallbackStruct *cbs)
{
   *answer = cbs->reason;
}

/*------------------------------------------------------------------------
 * Trap window manager close
 */

static void
WmClose(Widget, int *answer, XtPointer)
{
   *answer = XmCR_CANCEL;
}

/*---------------------------------------------------------------
 *  Method to save resources
 */

void
HelpResWinC::DoSave(Widget, HelpResWinC *This, XtPointer)
{
//
// Prompt for a file name
//
   WArgList	args;
   args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
   if ( halApp->questionPM ) args.SymbolPixmap(halApp->questionPM);
   Widget	dialog = XmCreatePromptDialog(*This, "fileNameWin", ARGS);
   int		answer;

   XtAddCallback(dialog, XmNokCallback, (XtCallbackProc)AnswerQuery,
		 (XtPointer)&answer);
   XtAddCallback(dialog, XmNcancelCallback, (XtCallbackProc)AnswerQuery,
		 (XtPointer)&answer);

   XtUnmanageChild(XmSelectionBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));

//
// Trap window manager close function
//
   XmAddWMProtocolCallback(XtParent(dialog), halApp->delWinAtom,
			   (XtCallbackProc)WmClose, (caddr_t)&answer);

//
// Display dialog
//
   XtManageChild(dialog);

   StringC	file;
   while ( file.size() == 0 ) {

//
// Wait for answer
//
      answer = XmCR_NONE;
      while ( answer == XmCR_NONE ) {
	 XtAppProcessEvent(halApp->context, XtIMXEvent);
	 XSync(halApp->display, False);
      }

//
// Read file name from text field
//
      if ( answer != XmCR_CANCEL ) {

	 XmString	str;
	 XtVaGetValues(dialog, XmNtextString, &str, NULL);
	 WXmString	wstr(str);
	 char	*cs = (char*)wstr;

	 file = cs;
	 file.Trim();

	 XtFree(cs);
	 XmStringFree(str);

	 if ( file.size() == 0 ) {
	    This->PopupMessage("Please enter a file name");
	    set_invalid(XmSelectionBoxGetChild(dialog, XmDIALOG_TEXT),
	    		True, True);
	 }
      }
   }

   XtUnmanageChild(dialog);
   XtDestroyWidget(dialog);
   XSync(halApp->display, False);

   if ( answer == XmCR_CANCEL ) return;

//
// Create file
//
   FILE	*fp = fopen(file, "w");
   if ( !fp ) {
      StringC	errmsg = "Could not create ";
      errmsg += file;
      This->PopupMessage(errmsg);
      return;
   }

//
// Write out quick help
//
   fprintf(fp, "\n");
   fprintf(fp, "!--------------------------------------------------\n");
   fprintf(fp, "! Quick Help\n");
   fprintf(fp, "!--------------------------------------------------\n");
   fprintf(fp, "\n");

   Boolean	error = False;
   error = !This->WriteResources(fp, This->quickList, This->quickList2);
   
//
// Write out context help
//
   if ( !error ) {
      fprintf(fp, "\n");
      fprintf(fp, "!--------------------------------------------------\n");
      fprintf(fp, "! Context Help\n");
      fprintf(fp, "!--------------------------------------------------\n");
      fprintf(fp, "\n");

      error = !This->WriteResources(fp, This->contList, This->contList2);
   }
   
//
// Write out helpcards
//
   if ( !error ) {
      fprintf(fp, "\n");
      fprintf(fp, "!--------------------------------------------------\n");
      fprintf(fp, "! Help Button Helpcards\n");
      fprintf(fp, "!--------------------------------------------------\n");
      fprintf(fp, "\n");

      error = !This->WriteResources(fp, This->helpList, This->helpList2);
   }
   
   fclose(fp);
   error = (errno != 0);

   if ( error ) {
      StringC	errmsg = "Could not write ";
      errmsg += file;
      errmsg += '\n';
      errmsg += SystemErrorMessage(errno);
      This->PopupMessage(errmsg);
      return;
   }

//
// Write new resources to the database and move them to the original list
//
   This->MergeLists(This->quickList, This->quickList2);
   This->MergeLists(This->contList,  This->contList2);
   This->MergeLists(This->helpList,  This->helpList2);

//
// Update widget rec entry pointers
//
   u_int	count = This->widgetList.size();
   for (int i=0; i<count; i++) {

      WidgetRecC	*rec = (WidgetRecC*)This->widgetList[i];

      if ( rec->edit.quick.entry ) {
	 rec->orig.quick.entry = rec->edit.quick.entry;
	 rec->edit.quick.entry = NULL;
      }
      if ( rec->edit.cont.entry ) {
	 rec->orig.cont.entry = rec->edit.cont.entry;
	 rec->edit.cont.entry = NULL;
      }
      if ( rec->edit.help.entry ) {
	 rec->orig.help.entry = rec->edit.help.entry;
	 rec->edit.help.entry = NULL;
      }
   }

} // End DoSave

/*---------------------------------------------------------------
 *  Method to close window
 */

void
HelpResWinC::DoDone(Widget, HelpResWinC *This, XtPointer)
{
   Boolean	needSave;
   if ( !This->QuerySave(&needSave, /*cancelOk=*/True) ) return;

   if ( needSave ) DoSave(NULL, This, NULL);

   This->Hide();
}

/*------------------------------------------------------------------------
 * Check for changes that need to be saved
 */

Boolean
HelpResWinC::QuerySave(Boolean *doSave, Boolean cancelOk)
{
   *doSave = False;

   if ( quickList2.size() == 0 &&
      	 contList2.size() == 0 &&
	 helpList2.size() == 0 )
      return True;

//
// Ask about saving changes
//
   WArgList	args;
   args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
   if ( halApp->questionPM ) args.SymbolPixmap(halApp->questionPM);
   Widget	dialog = XmCreateQuestionDialog(*this, "saveChangesWin", ARGS);
   int		answer;

   Widget	noPB = XmCreatePushButton(dialog, "noPB", 0,0);
   XtManageChild(noPB);

   XtAddCallback(dialog, XmNokCallback, (XtCallbackProc)AnswerQuery,
		 (XtPointer)&answer);
   XtAddCallback(noPB, XmNactivateCallback, (XtCallbackProc)AnswerQuery,
		 (XtPointer)&answer);

   if ( cancelOk )
      XtAddCallback(dialog, XmNcancelCallback, (XtCallbackProc)AnswerQuery,
		    (XtPointer)&answer);
   else
      XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_CANCEL_BUTTON));

   XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));

//
// Trap window manager close function
//
   XmAddWMProtocolCallback(XtParent(dialog), halApp->delWinAtom,
			   (XtCallbackProc)WmClose, (caddr_t)&answer);

//
// Display dialog
//
   XtManageChild(dialog);

//
// Wait for answer
//
   answer = XmCR_NONE;
   while ( answer == XmCR_NONE ) {
      XtAppProcessEvent(halApp->context, XtIMXEvent);
      XSync(halApp->display, False);
   }

   XtUnmanageChild(dialog);
   XtDestroyWidget(dialog);
   XSync(halApp->display, False);

   if ( answer == XmCR_CANCEL )
      return False;

   *doSave = (answer == XmCR_OK);
   return True;

} // End QuerySave

/*------------------------------------------------------------------------
 * Write out resources from original and edit lists
 */

Boolean
HelpResWinC::WriteResources(FILE *fp, PtrList2& origList, PtrList2& editList)
{
   ResEntryC	*ent;
   CharC	nl("\n");

//
// Write original entries if they don't appear in edit list.
//
   u_int	count = origList.size();
   Boolean	error = False;
   int	i;
   for (i=0; !error && i<count; i++) {

      ent = (ResEntryC*)origList[i];

      if ( EntryIndex(ent, editList) == -1 )
	 error = (!ent->full.WriteFile(fp) || !nl.WriteFile(fp));
   }

//
// Write edit entries
//
   if ( !error ) {
      CharC	marker("!--------------------------------------------------\n");
      error = !marker.WriteFile(fp);
   }

   count = editList.size();
   for (i=0; !error && i<count; i++) {

      ent = (ResEntryC*)editList[i];
      error = (!ent->full.WriteFile(fp) || !nl.WriteFile(fp));
   }

   return !error;

} // End WriteResources

/*------------------------------------------------------------------------
 * See if an exact match for this entry exists in the list
 */

int
HelpResWinC::EntryIndex(ResEntryC *thisEnt, PtrList2& list)
{
   u_int	count = list.size();
   for (int i=0; i<count; i++) {
      ResEntryC	*thatEnt = (ResEntryC*)list[i];
      if ( thatEnt->wspec == thisEnt->wspec ) return i;
   }

   return -1;

} // End EntryIndex

/*------------------------------------------------------------------------
 * Merge the edits into the orignal list
 */

void
HelpResWinC::MergeLists(PtrList2& origList, PtrList2& editList)
{
   XrmDatabase	db    = XtDatabase(halApp->display);
   u_int	count = editList.size();
   for (int i=0; i<count; i++) {

      ResEntryC	*editEnt = (ResEntryC*)editList[i];
      int	index = EntryIndex(editEnt, origList);
      if ( index >= 0 ) {
	 ResEntryC	*origEnt = (ResEntryC*)origList[index];
	 origList.replace(editEnt, index);
	 delete origEnt;
      }
      else
         origList.append(editEnt);

      XrmPutLineResource(&db, editEnt->full);
   }

   editList.removeAll();

} // End MergeLists
