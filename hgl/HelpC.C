/*
 * $Id: HelpC.C,v 1.2 2000/05/07 12:26:10 fnevgeny Exp $
 *
 * Copyright (c) 1994 HAL Computer Systems International, Ltd.
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

#include "HalAppC.h"
#include "HelpC.h"
#include "HelpDbC.h"
#include "StringC.h"
#include "rsrc.h"
#include "WArgList.h"
#include "WXmString.h"
#include "MimeRichTextC.h"
#include "TextMisc.h"
#include "SysErr.h"
#include "HelpResWinC.h"

#include <Xm/MessageB.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>
#include <Xm/PushB.h>
#include <Xm/List.h>
#include <Xm/ScrolledW.h>
#include <Xm/RowColumn.h>
#include <Xm/CascadeB.h>
#include <Xm/Protocols.h>
#include <Xm/SelectioB.h>
#include <Xm/AtomMgr.h>

#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>

#include <errno.h>
#include <unistd.h>

extern int	debuglev;

/*-----------------------------------------------------------------------
 * Constructor
 */

HelpC::HelpC(Boolean editOk)
{
   helpWin    = NULL;
   glossWin   = NULL;
   indexWin   = NULL;
   indexList  = NULL;
   curCard    = NULL;
   helpResWin = NULL;
   showOlias  = True;
   enriched   = False;
   edit       = editOk;
   changed    = False;

   if ( debuglev > 0 ) {
      if ( Editable() ) cout <<"Helpcards can be edited" <<endl;
      else	        cout <<"Helpcards cannot be edited" <<endl;
   }

   StringC	defname = halApp->name + ".hlp";
   StringC	files   = get_string(*halApp, "helpcardDbFiles", defname);

//
// Create a database for each file
//
   char	*name = strtok(files, " ");
   while (name) {

      HelpDbC	*db = new HelpDbC(name);
      if ( db->loaded ) {
	 if ( debuglev > 1 ) cout <<"Loaded help db: " <<db <<endl;
	 void	*tmp = (void*)db;
	 dbList.add(tmp);
      }
      else
	 delete db;

      name = strtok(NULL, " ");
   }

   isActive = (dbList.size() > 0);

} // End Constructor

/*-----------------------------------------------------------------------
 * Destructor
 */

HelpC::~HelpC()
{
#if EDIT_OK
   if ( CheckChanges(/*cancelOk=*/False) )
      DoFileSave(NULL, this, NULL);
#endif

   unsigned	count = dbList.size();
   for (int i=0; i<count; i++) {
      HelpDbC	*db = (HelpDbC*)*dbList[i];
      delete db;
   }

   delete helpResWin;

} // End Destructor

/*-----------------------------------------------------------------------
 * Method to display a helpcard for the specified widget
 */

void
HelpC::ShowCard(const Widget w, const char *res, Widget parent)
{
   if ( !isActive ) {
      StringC	msg = "There are no help files available.";
      ShowMessage(msg, parent);
      return;
   }

   StringC	name = CardName(w, res);
   if ( name.size() == 0 ) {
      StringC	msg = "I could not find a \"";
      msg += res;
      msg += "\" resource for the widget \"";
      msg += XtName(w);
      msg += "\"";
      ShowMessage(msg, parent);
      return;
   }

//
// Look through each database for the card
//
   unsigned	count = dbList.size();
   int	i;
   for (i=0; i<count; i++) {
      HelpDbC	*db = (HelpDbC*)*dbList[i];
      if ( debuglev > 1 ) cout <<"Checking help db: " <<db <<endl;
      HelpCardC	*card = db->FindCard(name);
      if ( card ) {
	 if ( card->IsGlossary() )
	    ShowGlossary(card, parent);
	 else
	    ShowCard(card, parent);
	 return;
      }
   }

   StringC	msg = "I could not find the help card \"";
   msg += name;
   msg += "\" for the widget \"";
   msg += XtName(w);
   msg += "\"\n";

   if ( dbList.size() > 1 ) {
      msg += "in any of the help files:\n";
      count = dbList.size();
      for (i=0; i<count; i++) {
	 HelpDbC	*db = (HelpDbC*)*dbList[i];
	 msg += "   " + db->file + "\n";
      }
   }
   else {
      HelpDbC	*db = (HelpDbC*)*dbList[0];
      msg += "in the help file: " + db->file;
   }

   ShowMessage(msg, parent);

} // End ShowCard

/*-----------------------------------------------------------------------
 * Method to create the helpcard window
 */

void
HelpC::CreateHelpWin()
{
   if ( helpWin ) return;

   WArgList	args;
   args.AutoUnmanage(False);
   if ( Editable() ) {
      args.DefaultButtonType(XmDIALOG_NONE);
      args.MinimizeButtons(False);
      args.DefaultButton(NULL);
   }
   if ( halApp->messagePM ) args.SymbolPixmap(halApp->messagePM);
   helpWin = XmCreateTemplateDialog(*halApp, "helpWin", ARGS);

   XtAddCallback(helpWin, XmNokCallback,      (XtCallbackProc)DoHelpIndex,this);
   XtAddCallback(helpWin, XmNhelpCallback,    (XtCallbackProc)DoHelpHelp, this);

// Get the defaultButtonShadowThickness from the cancel button
   Dimension defShadow;
   Widget okPB = XmMessageBoxGetChild(helpWin, XmDIALOG_OK_BUTTON);
   XtVaGetValues(okPB, XmNdefaultButtonShadowThickness, &defShadow, NULL);

   args.Reset();
   args.DefaultButtonShadowThickness(defShadow);
   Widget donePB = XmCreatePushButton(helpWin, "donePB", ARGS);
   XtAddCallback(donePB,  XmNactivateCallback,(XtCallbackProc)DoHelpDone, this);

   showOlias = get_boolean(helpWin, "showOliasButton", False);
   if ( showOlias )
      XtAddCallback(helpWin, XmNcancelCallback,
		    (XtCallbackProc)DoHelpOlias, this);
   else {
      Widget oliasPB = XmMessageBoxGetChild(helpWin, XmDIALOG_CANCEL_BUTTON);
      if ( oliasPB ) XtUnmanageChild(oliasPB);
   }
   XtVaSetValues(helpWin, XmNdefaultButton, donePB, NULL);

   enriched = get_boolean(helpWin, "enrichedText", False);

   XtManageChild(donePB);

   Widget	form = XmCreateForm(helpWin, "helpForm", 0,0);
   if ( Editable() ) {

#if EDIT_OK
      args.Reset();
      args.TopAttachment(XmATTACH_FORM);
      args.LeftAttachment(XmATTACH_FORM);
      args.RightAttachment(XmATTACH_NONE);
      args.BottomAttachment(XmATTACH_NONE);
      Widget	nameLabel = XmCreateLabel(form, "nameLabel", ARGS);
      XtManageChild(nameLabel);

      args.Reset();
      args.TopAttachment(XmATTACH_OPPOSITE_WIDGET, nameLabel);
      args.LeftAttachment(XmATTACH_WIDGET, nameLabel);
      args.RightAttachment(XmATTACH_FORM);
      args.BottomAttachment(XmATTACH_NONE);
      nameTF = CreateTextField(form, "nameTF", ARGS);
      XtManageChild(nameTF);
      XtAddCallback(nameTF, XmNmodifyVerifyCallback,
      		    (XtCallbackProc)ModifyName, this);
      XtAddCallback(nameTF, XmNactivateCallback,
      		    (XtCallbackProc)EnterName, this);

      args.Reset();
      args.TopAttachment(XmATTACH_WIDGET, nameTF);
      args.LeftAttachment(XmATTACH_FORM);
      args.RightAttachment(XmATTACH_NONE);
      args.BottomAttachment(XmATTACH_NONE);
      Widget	locLabel = XmCreateLabel(form, "locLabel", ARGS);
      XtManageChild(locLabel);

      args.Reset();
      args.TopAttachment(XmATTACH_OPPOSITE_WIDGET, locLabel);
      args.LeftAttachment(XmATTACH_WIDGET, locLabel);
      args.RightAttachment(XmATTACH_FORM);
      args.BottomAttachment(XmATTACH_NONE);
      locTF = CreateTextField(form, "locTF", ARGS);
      XtManageChild(locTF);
      XtAddCallback(locTF, XmNvalueChangedCallback,
      		    (XtCallbackProc)TextChanged, this);

      args.Reset();
      args.TopAttachment(XmATTACH_WIDGET, locTF);
      args.LeftAttachment(XmATTACH_FORM);
      args.RightAttachment(XmATTACH_NONE);
      args.BottomAttachment(XmATTACH_NONE);
      Widget	titleLabel = XmCreateLabel(form, "titleLabel", ARGS);
      XtManageChild(titleLabel);

      args.Reset();
      args.TopAttachment(XmATTACH_OPPOSITE_WIDGET, titleLabel);
      args.LeftAttachment(XmATTACH_WIDGET, titleLabel);
      args.RightAttachment(XmATTACH_FORM);
      args.BottomAttachment(XmATTACH_NONE);
      helpTitle = CreateTextField(form, "helpTitle", ARGS);
      XtManageChild(helpTitle);
      XtAddCallback(helpTitle, XmNvalueChangedCallback,
      		    (XtCallbackProc)TextChanged, this);

      Dimension	maxWd, wd;
      XtVaGetValues(nameLabel,  XmNwidth, &maxWd, NULL);
      XtVaGetValues(locLabel,   XmNwidth, &wd, NULL); if (wd>maxWd) maxWd = wd;
      XtVaGetValues(titleLabel, XmNwidth, &wd, NULL); if (wd>maxWd) maxWd = wd;

      args.Reset();
      args.Alignment(XmALIGNMENT_END);
      args.Width(maxWd);
      args.Resizable(False);
      XtSetValues(nameLabel, ARGS);
      XtSetValues(locLabel, ARGS);
      XtSetValues(titleLabel, ARGS);
#endif
   }

   else {
      helpTitle = XmCreateLabel(form, "helpTitle", 0,0);
      XtManageChild(helpTitle);
   }

   args.Reset();
   args.TopAttachment(XmATTACH_WIDGET, helpTitle);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);

   if ( enriched ) {
      richText = new MimeRichTextC(form, "richText", ARGS);
      richText->SetTextType(TT_ENRICHED);
      richText->SetEditable(Editable());
      richText->ResizeWidth(False);
      richText->AddLinkAccessCallback((CallbackFn*)DoGlossaryCard, (void*)this);
      XtManageChild(richText->MainWidget());
   }

   else {
      args.Editable(Editable());
      args.EditMode(XmMULTI_LINE_EDIT);
      args.ScrollHorizontal(False);
      args.CursorPositionVisible(False);
      args.WordWrap(True);
      args.ResizeWidth(False);
      helpText = CreateScrolledText(form, "helpText", ARGS);
      XtManageChild(helpText);
   }

#if EDIT_OK
   if ( Editable() ) {

      args.Reset();

      Widget	menuBar = XmCreateMenuBar(helpWin, "menuBar", 0,0);
      XtManageChild(menuBar);

//
// File menu
//
      Widget	filePD = XmCreatePulldownMenu(menuBar, "filePD", 0,0);
      args.SubMenuId(filePD);
      Widget	fileCB = XmCreateCascadeButton(menuBar, "fileCB", ARGS);
      XtManageChild(fileCB);

#if 0
      Widget	fileNewPB  = XmCreatePushButton(filePD, "fileNewPB",  0,0);
      XtManageChild(fileNewPB);
      XtAddCallback(fileNewPB, XmNactivateCallback, (XtCallbackProc)DoFileNew,
      		    this);

      Widget	fileDelPB  = XmCreatePushButton(filePD, "fileDelPB",  0,0);
      XtManageChild(fileDelPB);
      XtAddCallback(fileDelPB, XmNactivateCallback, (XtCallbackProc)DoFileDel,
      		    this);
#endif

      Widget	fileSavePB = XmCreatePushButton(filePD, "fileSavePB", 0,0);
      XtManageChild(fileSavePB);
      XtAddCallback(fileSavePB, XmNactivateCallback, (XtCallbackProc)DoFileSave,
      		    this);

      if ( enriched ) {

	 richText->AddTextChangeCallback((CallbackFn*)RichTextChanged,
					 (void*)this);

//
// Edit menu
//
	 Widget	editPD = XmCreatePulldownMenu(menuBar, "editPD", 0,0);
	 args.SubMenuId(editPD);
	 Widget	editCB = XmCreateCascadeButton(menuBar, "editCB", ARGS);
	 XtManageChild(editCB);

	 Widget	editPlainPB  = XmCreatePushButton(editPD, "editPlainPB",  0,0);
	 XtManageChild(editPlainPB);
	 XtAddCallback(editPlainPB, XmNactivateCallback,
	 			   (XtCallbackProc)DoEditPlain, this);

	 Widget	editBoldPB  = XmCreatePushButton(editPD, "editBoldPB",  0,0);
	 XtManageChild(editBoldPB);
	 XtAddCallback(editBoldPB, XmNactivateCallback,
	 			   (XtCallbackProc)DoEditBold, this);

	 Widget	editItalicPB = XmCreatePushButton(editPD, "editItalicPB",  0,0);
	 XtManageChild(editItalicPB);
	 XtAddCallback(editItalicPB, XmNactivateCallback,
	 			   (XtCallbackProc)DoEditItalic, this);

	 Widget	editFixedPB  = XmCreatePushButton(editPD, "editFixedPB",  0,0);
	 XtManageChild(editFixedPB);
	 XtAddCallback(editFixedPB, XmNactivateCallback,
	 			   (XtCallbackProc)DoEditFixed, this);

	 Widget	editUnderPB  = XmCreatePushButton(editPD, "editUnderPB",  0,0);
	 XtManageChild(editUnderPB);
	 XtAddCallback(editUnderPB, XmNactivateCallback,
	 			   (XtCallbackProc)DoEditUnder, this);

	 Widget	editBigPB = XmCreatePushButton(editPD, "editBigPB",  0,0);
	 XtManageChild(editBigPB);
	 XtAddCallback(editBigPB, XmNactivateCallback,
	 			   (XtCallbackProc)DoEditBig, this);

	 Widget	editSmallPB = XmCreatePushButton(editPD, "editSmallPB", 0,0);
	 XtManageChild(editSmallPB);
	 XtAddCallback(editSmallPB, XmNactivateCallback,
	 			   (XtCallbackProc)DoEditSmall, this);

	 Widget	editUndelPB = XmCreatePushButton(editPD, "editUndelPB",  0,0);
	 XtManageChild(editUndelPB);
	 XtAddCallback(editUndelPB, XmNactivateCallback,
	 			   (XtCallbackProc)DoEditUndel, this);

//
// Justify menu
//
	 Widget	justPD = XmCreatePulldownMenu(menuBar, "justPD", 0,0);
	 args.SubMenuId(justPD);
	 Widget	justCB = XmCreateCascadeButton(menuBar, "justCB", ARGS);
	 XtManageChild(justCB);

	 Widget	justLeftPB = XmCreatePushButton(justPD, "justLeftPB", 0,0);
	 XtManageChild(justLeftPB);
	 XtAddCallback(justLeftPB, XmNactivateCallback,
	 			   (XtCallbackProc)DoJustLeft, this);

	 Widget	justRightPB = XmCreatePushButton(justPD, "justRightPB", 0,0);
	 XtManageChild(justRightPB);
	 XtAddCallback(justRightPB, XmNactivateCallback,
	 			   (XtCallbackProc)DoJustRight, this);

	 Widget	justCenterPB = XmCreatePushButton(justPD, "justCenterPB", 0,0);
	 XtManageChild(justCenterPB);
	 XtAddCallback(justCenterPB, XmNactivateCallback,
	 			   (XtCallbackProc)DoJustCenter, this);

//
// Indent menu
//
	 Widget	indentPD = XmCreatePulldownMenu(menuBar, "indentPD", 0,0);
	 args.SubMenuId(indentPD);
	 Widget	indentCB = XmCreateCascadeButton(menuBar, "indentCB", ARGS);
	 XtManageChild(indentCB);

	 Widget	indentLeftMorePB = XmCreatePushButton(indentPD,
	 					      "indentLeftMorePB", 0,0);
	 XtManageChild(indentLeftMorePB);
	 XtAddCallback(indentLeftMorePB, XmNactivateCallback,
	 			   (XtCallbackProc)DoIndentLeftMore, this);

	 Widget	indentLeftLessPB = XmCreatePushButton(indentPD,
	 					      "indentLeftLessPB", 0,0);
	 XtManageChild(indentLeftLessPB);
	 XtAddCallback(indentLeftLessPB, XmNactivateCallback,
	 			   (XtCallbackProc)DoIndentLeftLess, this);

	 Widget	indentRightMorePB = XmCreatePushButton(indentPD,
	 					      "indentRightMorePB", 0,0);
	 XtManageChild(indentRightMorePB);
	 XtAddCallback(indentRightMorePB, XmNactivateCallback,
	 			   (XtCallbackProc)DoIndentRightMore, this);

	 Widget	indentRightLessPB = XmCreatePushButton(indentPD,
	 					      "indentRightLessPB", 0,0);
	 XtManageChild(indentRightLessPB);
	 XtAddCallback(indentRightLessPB, XmNactivateCallback,
	 			   (XtCallbackProc)DoIndentRightLess, this);

//
// Color menu
//
	 Widget	colorPD = XmCreatePulldownMenu(menuBar, "colorPD", 0,0);
	 args.SubMenuId(colorPD);
	 Widget	colorCB = XmCreateCascadeButton(menuBar, "colorCB", ARGS);
	 XtManageChild(colorCB);

	 Widget	colorRedPB = XmCreatePushButton(colorPD, "colorRedPB", 0,0);
	 XtManageChild(colorRedPB);
	 XtAddCallback(colorRedPB, XmNactivateCallback,
	 			   (XtCallbackProc)DoColorRed, this);

	 Widget	colorGreenPB = XmCreatePushButton(colorPD, "colorGreenPB", 0,0);
	 XtManageChild(colorGreenPB);
	 XtAddCallback(colorGreenPB, XmNactivateCallback,
	 			   (XtCallbackProc)DoColorGreen, this);

	 Widget	colorBluePB = XmCreatePushButton(colorPD, "colorBluePB", 0,0);
	 XtManageChild(colorBluePB);
	 XtAddCallback(colorBluePB, XmNactivateCallback,
	 			   (XtCallbackProc)DoColorBlue, this);

	 Widget	colorYellowPB = XmCreatePushButton(colorPD, "colorYellowPB",
	 					   0,0);
	 XtManageChild(colorYellowPB);
	 XtAddCallback(colorYellowPB, XmNactivateCallback,
	 			   (XtCallbackProc)DoColorYellow, this);

	 Widget	colorMagentaPB = XmCreatePushButton(colorPD, "colorMagentaPB",
						    0,0);
	 XtManageChild(colorMagentaPB);
	 XtAddCallback(colorMagentaPB, XmNactivateCallback,
	 			   (XtCallbackProc)DoColorMagenta, this);

	 Widget	colorCyanPB = XmCreatePushButton(colorPD, "colorCyanPB", 0,0);
	 XtManageChild(colorCyanPB);
	 XtAddCallback(colorCyanPB, XmNactivateCallback,
	 			   (XtCallbackProc)DoColorCyan, this);

	 Widget	colorBlackPB = XmCreatePushButton(colorPD, "colorBlackPB", 0,0);
	 XtManageChild(colorBlackPB);
	 XtAddCallback(colorBlackPB, XmNactivateCallback,
	 			   (XtCallbackProc)DoColorBlack, this);

	 Widget	colorWhitePB = XmCreatePushButton(colorPD, "colorWhitePB", 0,0);
	 XtManageChild(colorWhitePB);
	 XtAddCallback(colorWhitePB, XmNactivateCallback,
	 			   (XtCallbackProc)DoColorWhite, this);

	 Widget	colorNonePB = XmCreatePushButton(colorPD, "colorNonePB", 0,0);
	 XtManageChild(colorNonePB);
	 XtAddCallback(colorNonePB, XmNactivateCallback,
	 			   (XtCallbackProc)DoColorNone, this);

//
// Modified label
//
	 modLabel = XmCreateCascadeButton(menuBar, "modLabel", 0,0);
	 XtManageChild(modLabel);
	 XtVaSetValues(menuBar, XmNmenuHelpWidget, modLabel, NULL);

      } // End if enriched

      else {
	    XtAddCallback(helpText, XmNvalueChangedCallback,
			  (XtCallbackProc)TextChanged, this);
      }

   } // End if card is editable
#endif

   XtManageChild(form);

} // End CreateHelpWin

/*-----------------------------------------------------------------------
 * Method to display the helpcard window
 */

void
HelpC::PopupHelpWin(Widget parent)
{
//
// Set the dialog's parent so it pops up in the desired place
//
   if ( !parent ) parent = halApp->appShell;
   Widget	dShell = XtParent(helpWin);

//
// Get the parent shell of the parent.  If the parent shell is the shell of
// the Help Window, do not change the parent. (DJL 07/11/95)
//
   Widget parentShell = parent;
   while ( !XtIsShell(parentShell) ) parentShell = XtParent(parentShell);
  
   if ( parentShell != dShell ) dShell->core.parent = parent;

   XtManageChild(helpWin);
   XMapRaised(halApp->display, XtWindow(XtParent(helpWin)));

} // End PopupHelpWin

/*-----------------------------------------------------------------------
 * Method to display the specified message in the helpcard window
 */

void
HelpC::ShowMessage(const char *msg, Widget parent)
{
//
// Create the dialog if necessary
//
   CreateHelpWin();

//
// Fill in the information
//
   if ( enriched ) {
      richText->Defer(True);
      richText->Clear();
      richText->AddStringPlain(msg);
      richText->ScrollTop();
      richText->Defer(False);
   }

   else
      XmTextSetString(helpText, (char*)msg);

   if ( Editable() ) {
#if EDIT_OK
      XmTextSetString(helpTitle, "");
#endif
   }
   else {
      WXmString	wstr = "";
      XtVaSetValues(helpTitle, XmNlabelString, (XmString)wstr, NULL);
   }

   curCard = NULL;

   PopupHelpWin(parent);

} // End ShowMessage

/*-----------------------------------------------------------------------
 * Method to display the specified helpcard
 */

void
HelpC::ShowCard(const HelpCardC *card, Widget parent)
{
#if EDIT_OK
   if ( helpWin && XtIsManaged(helpWin) && !CheckChanges() ) return;
#endif

//
// Create the dialog if necessary
//
   CreateHelpWin();

//
// Fill in the information
//
   StringC	cardText;
   card->GetText(cardText);

   if ( enriched ) {
      richText->SetString(cardText);
      richText->ScrollTop();
   }
   else
      XmTextSetString(helpText, cardText);

   SetChanged(False);

   if ( Editable() ) {
#if EDIT_OK
      XmTextFieldSetString(nameTF,    card->name);
      XmTextFieldSetString(locTF,     card->locator);
      XmTextFieldSetString(helpTitle, card->title);
#endif
   }
   else {
      WXmString	wstr = (const char*)card->title;
      XtVaSetValues(helpTitle, XmNlabelString, (XmString)wstr, NULL);
   }

   curCard = (HelpCardC*)card;

   if ( parent != (Widget)-1 )
      PopupHelpWin(parent);

   SetChanged(False);

} // End ShowCard

/*-----------------------------------------------------------------------
 * Method to display the specified glossary card
 */

void
HelpC::DoGlossaryCard(void *arg, HelpC *This)
{
  StringC *cardName = (StringC*)arg;

//
// Look through each database for the card
//
   unsigned	count = This->dbList.size();
   int	i;
   for (i=0; i<count; i++) {
      HelpDbC	*db = (HelpDbC*)(*This->dbList[i]);
      HelpCardC	*card = db->FindCard(*cardName);
      if ( card ) {
	 Widget dShell = XtParent(This->helpWin);
	 This->ShowGlossary(card, dShell->core.parent);
	 return;
      }
   }
  
   StringC	msg = "I could not find the glossary help card \"";
   msg += *cardName;
   msg += "\"\n";

   if ( This->dbList.size() > 1 ) {
      msg += "in any of the help files:\n";
      count = This->dbList.size();
      for (i=0; i<count; i++) {
	 HelpDbC	*db = (HelpDbC*)(*This->dbList[i]);
	 msg += "   " + db->file + "\n";
      }
   }
   else {
      HelpDbC	*db = (HelpDbC*)*This->dbList[0];
      msg += "in the help file: " + db->file;
   }

   halApp->PopupMessage(msg, XtParent(This->helpWin));

} // End HelpC DoGlossaryCard

/*-----------------------------------------------------------------------
 * Method to display the specified glossary card
 */

void
HelpC::ShowGlossary(const HelpCardC *card, Widget parent)
{
//
// Create the dialog if necessary
//
   if ( !glossWin ) {

      WArgList	args;
      args.AutoUnmanage(False);
      if ( halApp->messagePM )
	 args.SymbolPixmap(halApp->messagePM);
      glossWin = XmCreateTemplateDialog(*halApp, "glossWin", ARGS);

      XtAddCallback(glossWin, XmNokCallback, (XtCallbackProc)DoGlossaryIndex,
		    this);
      XtAddCallback(glossWin, XmNhelpCallback, (XtCallbackProc)DoGlossaryHelp,
		    this);

// Get the defaultButtonShadowThickness from the cancel button
      Dimension defShadow;
      Widget okPB = XmMessageBoxGetChild(helpWin, XmDIALOG_OK_BUTTON);
      XtVaGetValues(okPB, XmNdefaultButtonShadowThickness, &defShadow, NULL);

      args.Reset();
      args.DefaultButtonShadowThickness(defShadow);
      Widget donePB = XmCreatePushButton(helpWin, "donePB", ARGS);
      XtAddCallback(donePB, XmNactivateCallback,(XtCallbackProc)DoGlossaryDone,
		    this);

      showOlias = get_boolean(glossWin, "showOliasButton", False);
      if ( showOlias )
	 XtAddCallback(glossWin, XmNcancelCallback,
		       (XtCallbackProc)DoHelpOlias, this);
      else {
	 Widget	oliasPB = XmMessageBoxGetChild(glossWin, XmDIALOG_CANCEL_BUTTON);
	 if ( oliasPB ) XtUnmanageChild(oliasPB);
      }
      XtVaSetValues(helpWin, XmNdefaultButton, donePB, NULL);

      enriched = get_boolean(glossWin, "enrichedText", False);

      XtManageChild(donePB);

//      args.Reset();
//      args.ResizePolicy(XmRESIZE_NONE);
      Widget	form  = XmCreateForm(glossWin, "glossForm", 0,0);
      glossTitle = XmCreateLabel(form, "glossTitle", 0,0);
      XtManageChild(glossTitle);

      args.Reset();
      args.TopAttachment(XmATTACH_WIDGET, glossTitle);
      args.LeftAttachment(XmATTACH_FORM);
      args.RightAttachment(XmATTACH_FORM);
      args.BottomAttachment(XmATTACH_FORM);

      if ( enriched ) {
	 glossRichText = new MimeRichTextC(form, "glossRichText", ARGS);
	 glossRichText->SetTextType(TT_ENRICHED);
	 glossRichText->SetEditable(False);
	 glossRichText->ResizeWidth(False);
	 XtManageChild(glossRichText->MainWidget());
      }

      else {
	 args.Editable(False);
	 args.EditMode(XmMULTI_LINE_EDIT);
	 args.ScrollHorizontal(False);
	 args.CursorPositionVisible(False);
	 args.WordWrap(True);
	 args.ResizeWidth(False);
	 glossText = CreateScrolledText(form, "glossText", ARGS);
	 XtManageChild(glossText);
      }

      XtManageChild(form);

   } // End if window not created

//
// Fill in the information
//
   StringC	cardText;
   card->GetText(cardText);

   if ( enriched ) {
      glossRichText->SetString(cardText);
      glossRichText->ScrollTop();
   }
   else
      XmTextSetString(glossText, cardText);

   WXmString	wstr = (const char*)card->title;
   XtVaSetValues(glossTitle, XmNlabelString, (XmString)wstr, NULL);

   curCard = (HelpCardC*)card;

//
// Set the dialog's parent so it pops up in the desired place
//
   if ( !parent ) parent = halApp->appShell;
   Widget	dShell = XtParent(glossWin);

//
// Get the parent shell of the parent.  If the parent shell is the shell of
// the Help Window, do not change the parent. (DJL 07/11/95)
//
   Widget parentShell = parent;
   while (!XtIsShell(parentShell)) {
     parentShell = XtParent(parentShell);
   }
  
   if (parentShell != dShell) {
     dShell->core.parent = parent;
   }

   XtManageChild(glossWin);
   XMapRaised(halApp->display, XtWindow(XtParent(glossWin)));

} // End ShowCard

/*-----------------------------------------------------------------------
 * Method to display the helpcard index
 */

void
HelpC::ShowIndex(Widget parent)
{
   if ( !isActive ) {
      StringC	msg = "There are no help files available.";
      halApp->PopupMessage(msg, parent);
      return;
   }

//
// Create the dialog if necessary
//
   if ( !indexWin ) {

//
// Count the number of index entries
//
      unsigned	itemCount = 0;
      unsigned	count = dbList.size();
      int	i;
      for (i=0; i<count; i++) {
	 HelpDbC	*db = (HelpDbC*)*dbList[i];
	 itemCount += db->cardList.size();
      }

//
// Create a list of entries
//
      XmString	*items = new XmString[itemCount];
      count = dbList.size();
      int	k = 0;
      StringC	tmpStr(80);
      for (i=0; i<count; i++) {
	 HelpDbC	*db = (HelpDbC*)*dbList[i];
	 unsigned	ccount = db->cardList.size();
	 for (int j=0; j<ccount; j++) {
	    HelpCardC	*card = (HelpCardC*)*db->cardList[j];
	    tmpStr = card->title;
	    items[k] = XmStringCreateLocalized(tmpStr);
	    k++;
	 }
      }

      WArgList	args;
      args.AutoUnmanage(False);
      if ( halApp->messagePM )
	 args.SymbolPixmap(halApp->messagePM);
      indexWin = XmCreateTemplateDialog(*halApp, "helpIndexWin", ARGS);

      viewPB = XmMessageBoxGetChild(indexWin, XmDIALOG_OK_BUTTON);
      nextPB = XmCreatePushButton(indexWin, "nextPB", 0,0);
      prevPB = XmCreatePushButton(indexWin, "prevPB", 0,0);
      Widget	donePB = XmCreatePushButton(indexWin, "donePB", 0,0);

      XtAddCallback(indexWin, XmNokCallback, (XtCallbackProc)DoIndexView,
		    this);
      XtAddCallback(nextPB, XmNactivateCallback,(XtCallbackProc)DoIndexNext,
		    this);
      XtAddCallback(prevPB, XmNactivateCallback,(XtCallbackProc)DoIndexPrev,
		    this);
      XtAddCallback(donePB, XmNactivateCallback,(XtCallbackProc)DoIndexDone,
		    this);
      XtAddCallback(indexWin, XmNhelpCallback, (XtCallbackProc)DoIndexHelp,
		    this);

      XtManageChild(nextPB);
      XtManageChild(prevPB);
      XtManageChild(donePB);

      args.Reset();
      args.ItemCount(itemCount);
      args.Items(items);
      args.SelectionPolicy(XmBROWSE_SELECT);
      indexList = XmCreateScrolledList(indexWin, "indexList", ARGS);
      XtManageChild(indexList);

      XtAddCallback(indexList, XmNbrowseSelectionCallback,
      		    (XtCallbackProc)DoIndexSelect, this);
      XtAddCallback(indexList, XmNdefaultActionCallback,
      		    (XtCallbackProc)DoIndexOpen, this);

//
// Release strings
//
      for (i=0; i<itemCount; i++) XmStringFree(items[i]);
      delete [] items;

   } // End if window not created

//
// Select the current card
//
   int	pos = 1;
   if ( curCard ) {
      unsigned	count = dbList.size();
      for (int i=0; i<count; i++) {
	 HelpDbC	*db = (HelpDbC*)*dbList[i];
	 void	*tmp = (void*)curCard;
	 int	index = db->cardList.indexOf(tmp);
	 if ( index != db->cardList.NULL_INDEX )
	    pos += index;
	 else {
	    pos += db->cardList.size();;
	    i = count;
	 }
      }

      XmListSelectPos(indexList, pos, True);
   }

   else
      XmListDeselectAllItems(indexList);

   int	top;
   int	vis;
   XtVaGetValues(indexList, XmNtopItemPosition, &top,
   			    XmNvisibleItemCount, &vis, NULL);
   int	bot = top + vis - 1;
   if ( pos < top )
      XmListSetPos(indexList, pos);
   else if ( pos > bot )
      XmListSetBottomPos(indexList, pos);

//
// Set the dialog's parent so it pops up in the desired place
//
   if ( !parent ) parent = halApp->appShell;
   Widget	dShell = XtParent(indexWin);
   dShell->core.parent = parent;

   XtManageChild(indexWin);
   XMapRaised(halApp->display, XtWindow(XtParent(indexWin)));

} // End ShowIndex

/*-----------------------------------------------------------------------
 * Method to remove the Olias button
 */

void
HelpC::removeOliasButton()
{
   if ( !showOlias ) return;

   showOlias = False;
   if ( helpWin ) {
      Widget	oliasPB = XmMessageBoxGetChild(helpWin, XmDIALOG_CANCEL_BUTTON);
      if ( oliasPB ) XtUnmanageChild(oliasPB);
   }

   if ( glossWin ) {
      Widget	oliasPB = XmMessageBoxGetChild(glossWin, XmDIALOG_CANCEL_BUTTON);
      if ( oliasPB ) XtUnmanageChild(oliasPB);
   }

} // End removeOliasButton

/*-----------------------------------------------------------------------
 * Method to find the helpcard name for the specified widget
 */

StringC
HelpC::CardName(const Widget w, const char *res)
{
   if ( !res ) res = "helpcard";
   StringC	name = get_string(w, res);

//
// If no card was found, try the parent
//
   if ( name.size() == 0 && XtParent(w) != NULL )
      name = CardName(XtParent(w), res);

   return name;

} // End CardName

/*-----------------------------------------------------------------------
 * Callback to close helpcard
 */

void
HelpC::DoHelpDone(Widget, HelpC *This, XtPointer)
{
#if EDIT_OK
   if ( !This->CheckChanges() ) return;
   DoFileSave(NULL, This, NULL);
#endif

   This->SetChanged(False);
   XtUnmanageChild(This->helpWin);
}

void
HelpC::DoGlossaryDone(Widget, HelpC *This, XtPointer)
{
   XtUnmanageChild(This->glossWin);
}

/*-----------------------------------------------------------------------
 * Callback to display help for the helpcard
 */

Widget helpHelpWin = NULL;

void
HelpC::DoHelpHelp(Widget, HelpC *This, XtPointer)
{
   if (helpHelpWin == NULL) {
      StringC	text = get_string(This->helpWin, "helpString", "helpString");
      if ( This->showOlias ) {
         StringC  otext = get_string(This->helpWin, "helpOliasString",
				     "helpOliasString");
         text += "\n" + otext;
      }

      XmString tcs = XmStringCreateLtoR((char*)text, XmFONTLIST_DEFAULT_TAG);
      WArgList args;
      args.DialogType(XmDIALOG_INFORMATION);
      args.MessageString(tcs);
      if ( halApp->messagePM ) args.SymbolPixmap(halApp->messagePM);
      helpHelpWin = XmCreateMessageDialog(This->helpWin, "helpHelpWin", ARGS);
      XtUnmanageChild(XmMessageBoxGetChild(helpHelpWin, XmDIALOG_OK_BUTTON));
      XtUnmanageChild(XmMessageBoxGetChild(helpHelpWin, XmDIALOG_HELP_BUTTON));
      XmStringFree(tcs);
   }
   XtManageChild(helpHelpWin);
} // End DoHelpHelp

Widget glossHelpWin = NULL;

void
HelpC::DoGlossaryHelp(Widget, HelpC *This, XtPointer)
{
   if (glossHelpWin) {
      StringC  text = get_string(This->glossWin, "glossString", "glossString");
      if ( This->showOlias ) {
         StringC otext = get_string(This->glossWin, "helpOliasString",
				    "helpOliasString");
         text += "\n" + otext;
      }

      XmString tcs = XmStringCreateLtoR((char*)text, XmFONTLIST_DEFAULT_TAG);
      WArgList args;
      args.DialogType(XmDIALOG_INFORMATION);
      args.MessageString(tcs);
      if ( halApp->messagePM ) args.SymbolPixmap(halApp->messagePM);
      glossHelpWin = XmCreateMessageDialog(This->helpWin, "glossHelpWin", ARGS);
      XtUnmanageChild(XmMessageBoxGetChild(glossHelpWin,
                                           XmDIALOG_CANCEL_BUTTON));
      XtUnmanageChild(XmMessageBoxGetChild(glossHelpWin,
                                           XmDIALOG_HELP_BUTTON));
      XmStringFree(tcs);
   }
   XtManageChild(glossHelpWin);
} // End DoGlossaryHelp

/*-----------------------------------------------------------------------
 * Callback to display the helpcard index
 */

void
HelpC::DoHelpIndex(Widget, HelpC *This, XtPointer)
{
   Widget	dShell = XtParent(This->helpWin);
   This->ShowIndex(dShell->core.parent);
}

void
HelpC::DoGlossaryIndex(Widget, HelpC *This, XtPointer)
{
   Widget	dShell = XtParent(This->glossWin);
   This->ShowIndex(dShell->core.parent);
}

/*-----------------------------------------------------------------------
 * Callback to invoke Olias for this helpcard
 */

#ifdef OLIAS
#include "olias.h"
#endif

void
HelpC::DoHelpOlias(Widget, HelpC *This, XtPointer)
{
#ifdef OLIAS
   static OliasDisplayEvent	event;
   event.type     = OLIAS_DISPLAY_EVENT;
   event.infobase = "baseD";
   event.locator  = This->curCard->locator;

   int	status = olias_send_event(This->helpWin, (OliasEvent *)&event);
   if (status != OLIAS_SUCCESS) {
      StringC	msg = get_string(This->helpWin, "oliasErrorString",
				 "oliasErrorString");
      msg += ' ';
      msg += status;
      halApp->PopupMessage(msg, This->helpWin);
   }
#endif
}

/*-----------------------------------------------------------------------
 * Callback to close index window
 */

void
HelpC::DoIndexDone(Widget, HelpC *This, XtPointer)
{
   XtUnmanageChild(This->indexWin);
}

/*-----------------------------------------------------------------------
 * Callback to selection of a list item
 */

void
HelpC::DoIndexSelect(Widget, HelpC *This, XmListCallbackStruct *cb)
{
   if ( cb->selected_item_count != 1 ) {
      XtSetSensitive(This->viewPB, False);
      XtSetSensitive(This->nextPB, False);
      XtSetSensitive(This->prevPB, False);
   }

   else {
      int	itemCount;
      XtVaGetValues(This->indexList, XmNitemCount, &itemCount, NULL);
      XtSetSensitive(This->nextPB, cb->item_position < itemCount );
      XtSetSensitive(This->prevPB, cb->item_position > 1);
      XtSetSensitive(This->viewPB, True);
      This->indexPos = cb->item_position;
   }

} // End DoIndexSelect

/*-----------------------------------------------------------------------
 * Callback to handle press of view button in index window
 */

void
HelpC::DoIndexView(Widget, HelpC *This, XtPointer)
{
   This->ShowCardFromIndex(This->indexPos);
}

/*-----------------------------------------------------------------------
 * Callback to handle press of next button in index window
 */

void
HelpC::DoIndexNext(Widget, HelpC *This, XtPointer)
{
   XmListSelectPos(This->indexList, This->indexPos+1, True);

   int	top;
   int	vis;
   XtVaGetValues(This->indexList, XmNtopItemPosition, &top,
				  XmNvisibleItemCount, &vis, NULL);
   int	bot = top + vis - 1;
   if ( This->indexPos > bot )
      XmListSetBottomPos(This->indexList, This->indexPos);

   This->ShowCardFromIndex(This->indexPos);
}

/*-----------------------------------------------------------------------
 * Callback to handle press of prev button in index window
 */

void
HelpC::DoIndexPrev(Widget, HelpC *This, XtPointer)
{
   XmListSelectPos(This->indexList, This->indexPos-1, True);

   int	top;
   int	vis;
   XtVaGetValues(This->indexList, XmNtopItemPosition, &top,
				  XmNvisibleItemCount, &vis, NULL);
   if ( This->indexPos < top )
      XmListSetPos(This->indexList, This->indexPos);

   This->ShowCardFromIndex(This->indexPos);
}

/*-----------------------------------------------------------------------
 * Callback to handle double-click on a list item
 */

void
HelpC::DoIndexOpen(Widget, HelpC *This, XmListCallbackStruct *cb)
{
   XmListSelectPos(This->indexList, cb->item_position, True);
   This->ShowCardFromIndex(This->indexPos);
}

/*-----------------------------------------------------------------------
 * Callback to handle press of help button in index window
 */

Widget indexHelpWin = NULL;

void
HelpC::DoIndexHelp(Widget, HelpC *This, XtPointer)
{
   if (indexHelpWin == NULL) {
      StringC text = get_string(This->indexWin, "helpString", "helpString");
      XmString tcs = XmStringCreateLtoR((char*)text, XmFONTLIST_DEFAULT_TAG);
      WArgList args;
      args.DialogType(XmDIALOG_INFORMATION);
      args.MessageString(tcs);
      if ( halApp->messagePM ) args.SymbolPixmap(halApp->messagePM);
      indexHelpWin = XmCreateMessageDialog(This->helpWin, "indexHelpWin", ARGS);
      XtUnmanageChild(XmMessageBoxGetChild(indexHelpWin, XmDIALOG_OK_BUTTON));
      XtUnmanageChild(XmMessageBoxGetChild(indexHelpWin, XmDIALOG_HELP_BUTTON));
      XmStringFree(tcs);
   }
   XtManageChild(indexHelpWin);
}

/*-----------------------------------------------------------------------
 * Method to display the helpcard corresponding to the given index position
 */

void
HelpC::ShowCardFromIndex(int pos)
{
//
// Find the help card using the index position
//
   unsigned	count = dbList.size();
   for (int i=0; i<count; i++) {
      HelpDbC	*db = (HelpDbC*)*dbList[i];
      if ( pos > db->cardList.size() ) pos -= db->cardList.size();
      else {
	 HelpCardC	*card = (HelpCardC*)*db->cardList[pos-1];
	 Widget	dShell = XtParent(indexWin);
	 if ( card->IsGlossary() )
	    ShowGlossary(card, dShell->core.parent);
	 else
	    ShowCard(card, dShell->core.parent);
	 return;
      }
   }

} // End ShowCardFromIndex

void
HelpC::ReparentWindows(Widget parent)
{
  Widget trans_parent, dShell;
  if (helpWin) {
    dShell = XtParent(helpWin);
    trans_parent = dShell->core.parent;
    while (!XtIsShell(trans_parent))
      trans_parent = XtParent(trans_parent);

    if (trans_parent == parent) {
      dShell->core.parent = *halApp;
    }
  }

  if (glossWin) {
    dShell = XtParent(glossWin);
    trans_parent = dShell->core.parent;
    while (!XtIsShell(trans_parent))
      trans_parent = XtParent(trans_parent);

    if (trans_parent == parent) {
      dShell->core.parent = *halApp;
    }
  }

  if (indexWin) {
    dShell = XtParent(indexWin);
    trans_parent = dShell->core.parent;
    while (!XtIsShell(trans_parent))
      trans_parent = XtParent(trans_parent);

    if (trans_parent == parent) {
      dShell->core.parent = *halApp;
    }
  }
}

#if EDIT_OK
/*------------------------------------------------------------------------
 * Record for modified helpcard
 */

class CardRecC {

public:

   StringC	name;
   StringC	title;
   StringC	locator;
   StringC	text;
   HelpDbC	*db;
   StringC	file;

   CardRecC() { db = NULL; }
};
#endif

/*-----------------------------------------------------------------------
 * Method to write new help database format for all databases
 */

void
HelpC::WriteNewDbFormat()
{
//
// Loop though databases
//
   HelpDbC	*db;
   StringC	newName;

   u_int	dcount = dbList.size();
   for (int d=0; d<dcount; d++) {

      db = (HelpDbC*)*dbList[d];
      cout <<"Converting help file " <<db->file <<endl;

      if ( db->file.EndsWith('2') ) {
	 cout <<"   file already converted." <<endl;
	 continue;
      }

      newName = db->file;
      newName += '2';

      if ( !WriteDbFile(db, newName) ) {
	 unlink(newName);
	 StringC	errstr("Could not write file: ");
	 errstr += newName;
	 errstr += '\n';
	 errstr += SystemErrorMessage(errno);
	 halApp->PopupMessage(errstr);
	 continue;
      }
   }

} // End WriteNewDbFormat

/*-----------------------------------------------------------------------
 * Method to write a help database to the specified file.  The new
 *     format is used.
 */

Boolean
HelpC::WriteDbFile(HelpDbC *db, char *newName)
{
   HelpCardC	*card;
   FILE		*fp;
   CharC	nl("\n");
   StringC	text;
   Boolean	needWrite;
   Boolean	error;

   fp = fopen(newName, "w");
   if ( !fp ) return False;

//
// Loop through cards
//
   u_int	ccount = db->cardList.size();
   for (int c=0; c<ccount; c++) {

      card = (HelpCardC*)*db->cardList[c];
      needWrite = True;

#if EDIT_OK
//
// See if this card has been modified
//
      CardRecC	*rec;
      u_int	mcount = modList.size();
      for (int m=0; needWrite && m<mcount; m++) {

	 rec = (CardRecC*)*modList[m];
	 if ( rec->db == db && rec->name == card->name ) {

	    error = (fprintf(fp, "[CARD] ") == EOF);
	    if ( !error ) error = !rec->name.WriteFile(fp);
	    if ( !error ) error = !nl.WriteFile(fp);

	    if ( !error ) error = (fprintf(fp, "[TITLE] ") == EOF);
	    if ( !error ) error = !rec->title.WriteFile(fp);
	    if ( !error ) error = !nl.WriteFile(fp);

	    if ( rec->locator.size() > 0 ) {
	       if ( !error ) error = (fprintf(fp, "[LOCATOR] ") == EOF);
	       if ( !error ) error = !rec->locator.WriteFile(fp);
	       if ( !error ) error = !nl.WriteFile(fp);
	    }

	    if ( !error ) error = !rec->text.WriteFile(fp);
	    if ( !error && !rec->text.EndsWith('\n') )
	       error = !nl.WriteFile(fp);

	    if ( error ) {
	       fclose(fp);
	       return False;
	    }

	    needWrite = False;
	    modList.remove(m);

	 } // End if card was modified

      } // End for each modified card
#endif

      if ( needWrite ) {

	 error = !card->GetText(text);

	 if ( !error ) error = (fprintf(fp, "[CARD] ") == EOF);
	 if ( !error ) error = !card->name.WriteFile(fp);
	 if ( !error ) error = !nl.WriteFile(fp);

	 if ( !error ) error = (fprintf(fp, "[TITLE] ") == EOF);
	 if ( !error ) error = !card->title.WriteFile(fp);
	 if ( !error ) error = !nl.WriteFile(fp);

	 if ( card->locator.size() > 0 ) {
	    if ( !error ) error = (fprintf(fp, "[LOCATOR] ") == EOF);
	    if ( !error ) error = !card->locator.WriteFile(fp);
	    if ( !error ) error = !nl.WriteFile(fp);
	 }

	 if ( !error ) error = !text.WriteFile(fp);
	 if ( !error && !text.EndsWith('\n') ) error = !nl.WriteFile(fp);

	 if ( error ) {
	    fclose(fp);
	    return False;
	 }

      } // End card needs to be written

   } // End for each help card

#if EDIT_OK
//
// See if any cards have been added
//
   u_int	acount = addList.size();
   for (int a=acount-1; a>=0; a--) {

      CardRecC	*rec = (CardRecC*)*addList[a];
      if ( rec->db == db || rec->file == db->file ) {

	 error = (fprintf(fp, "[CARD] ") == EOF);
	 if ( !error ) error = !rec->name.WriteFile(fp);
	 if ( !error ) error = !nl.WriteFile(fp);

	 if ( !error ) error = (fprintf(fp, "[TITLE] ") == EOF);
	 if ( !error ) error = !rec->title.WriteFile(fp);
	 if ( !error ) error = !nl.WriteFile(fp);

	 if ( rec->locator.size() > 0 ) {
	    if ( !error ) error = (fprintf(fp, "[LOCATOR] ") == EOF);
	    if ( !error ) error = !rec->locator.WriteFile(fp);
	    if ( !error ) error = !nl.WriteFile(fp);
	 }

	 if ( !error ) error = !rec->text.WriteFile(fp);
	 if ( !error && !rec->text.EndsWith('\n') ) error = !nl.WriteFile(fp);

	 if ( error ) {
	    fclose(fp);
	    return False;
	 }

	 addList.remove(a);
      }

   } // End for each added card
#endif

   error = (fclose(fp) == EOF);
   if ( !error && debuglev > 0 )
      cout <<"Created new help file: " <<newName <<endl;

   return !error;

} // End WriteDbFile

void
HelpC::SetChanged(Boolean val)
{
#if EDIT_OK
   if ( val == changed ) return;

   changed = val;

   Pixel	bg;
   if ( changed ) PixelValue(modLabel, "red", &bg);
   else		  XtVaGetValues(modLabel, XmNbackground, &bg, NULL);

   XtVaSetValues(modLabel, XmNforeground,  bg, NULL);
#endif
}

#if EDIT_OK
#if 0
void
HelpC::DoFileNew(Widget, HelpC *This, XtPointer)
{
   if ( !This->CheckChanges() ) return;

   XmTextFieldSetString(This->nameTF,    "");
   XmTextFieldSetString(This->locTF,     "");
   XmTextFieldSetString(This->helpTitle, "");
   This->richText->Clear();
   This->curCard = NULL;
}

void
HelpC::DoFileDel(Widget, HelpC *This, XtPointer)
{
}
#endif

/*-----------------------------------------------------------------------
 * Method to update database files with any changes.
 */

void
HelpC::DoFileSave(Widget, HelpC *This, XtPointer)
{
   if ( This->modList.size() == 0 && This->addList.size() == 0 ) return;

   if ( This->indexWin ) {
      XtUnmanageChild(This->indexWin);
      XtDestroyWidget(This->indexWin);
      This->indexWin  = NULL;
      This->indexList = NULL;
   }

//
// Loop though databases
//
   HelpDbC	*db;
   StringC	file;

   u_int	dcount = This->dbList.size();
   for (int d=0; d<dcount; d++) {

      db = (HelpDbC*)*This->dbList[d];
      if ( debuglev > 0 ) cout <<"Saving help file " <<db->file <<endl;

//
// Update file
//
      file = db->file;
      file += ".new";
      if ( !This->WriteDbFile(db, file) ) {
	 unlink(file);
	 StringC	errstr("Could not write file: ");
	 errstr += file;
	 errstr += '\n';
	 errstr += SystemErrorMessage(errno);
	 halApp->PopupMessage(errstr);
	 continue;
      }

      unlink(db->file);
      rename(file, db->file);
      file = db->file;

//
// Reload database 
//
      delete db;
      db = new HelpDbC(file);
      if ( db->loaded ) {
	 void	*tmp = (void*)db;
	 This->dbList.remove(d);
	 This->dbList.insert(tmp, d);
      }
      else
	 delete db;

   } // End for each database

//
// Create new database files for any entries still in the add list
//
   FILE		*fp;
   CharC	nl("\n");
   CharC	text;
   u_int	acount = This->addList.size();
   for (int a=0; a<acount; a++) {

      CardRecC	*rec = (CardRecC*)*This->addList[a];

      fp = fopen(rec->file, "a");
      if ( !fp ) {
	 perror(rec->file);
	 continue;
      }

      fprintf(fp, "[CARD] ");
      rec->name.WriteFile(fp);
      nl.WriteFile(fp);

      fprintf(fp, "[TITLE] ");
      rec->title.WriteFile(fp);
      nl.WriteFile(fp);

      if ( rec->locator.size() > 0 ) {
	 fprintf(fp, "[LOCATOR] ");
	 rec->locator.WriteFile(fp);
	 nl.WriteFile(fp);
      }

      rec->text.WriteFile(fp);
      if ( !rec->text.EndsWith('\n') ) nl.WriteFile(fp);

      fclose(fp);

   } // End for each added card

   This->addList.removeAll();

   This->SetChanged(False);

} // End DoFileSave

/*-----------------------------------------------------------------------
 * Method to check for changes before a new card name is entered
 */

void
HelpC::ModifyName(Widget, HelpC *This, XmTextVerifyCallbackStruct *cb)
{
   cb->doit = This->CheckChanges();
}

/*-----------------------------------------------------------------------
 * Method to enter a new helpcard name
 */

void
HelpC::EnterName(Widget, HelpC *This, XtPointer)
{
   if ( !This->CheckChanges() ) return;

   halApp->BusyCursor(True);

   char	*cs = XmTextFieldGetString(This->nameTF);
   StringC	nameStr(cs);
   XtFree(cs);
   nameStr.Trim();

//
// Look through each database looking for the name
//
   u_int	count = This->dbList.size();
   for (int i=0; i<count; i++) {

      HelpDbC	*db   = (HelpDbC*)*This->dbList[i];
      HelpCardC	*card = db->FindCard(nameStr);

      if ( card ) {
	 This->ShowCard(card, (Widget)-1/*No popup necessary*/);
	 halApp->BusyCursor(False);
	 return;
      }
   }

//
// Clear fields if name not found
//
   XmTextFieldSetString(This->locTF,     "");
   XmTextFieldSetString(This->helpTitle, "");
   This->richText->Clear();
   This->curCard = NULL;
   This->SetChanged(False);

   halApp->BusyCursor(False);

} // End EnterName

void
HelpC::DoEditPlain(Widget, HelpC *This, XtPointer)
{
   This->richText->ChangeFont(FC_PLAIN);
}

void
HelpC::DoEditBold(Widget, HelpC *This, XtPointer)
{
   This->richText->ChangeFont(FC_BOLD);
}

void
HelpC::DoEditItalic(Widget, HelpC *This, XtPointer)
{
   This->richText->ChangeFont(FC_ITALIC);
}

void
HelpC::DoEditFixed(Widget, HelpC *This, XtPointer)
{
   This->richText->ChangeFont(FC_FIXED);
}

void
HelpC::DoEditUnder(Widget, HelpC *This, XtPointer)
{
   This->richText->ChangeFont(FC_UNDERLINE);
}

void
HelpC::DoEditBig(Widget, HelpC *This, XtPointer)
{
   This->richText->ChangeFont(FC_BIGGER);
}

void
HelpC::DoEditSmall(Widget, HelpC *This, XtPointer)
{
   This->richText->ChangeFont(FC_SMALLER);
}

void
HelpC::DoEditUndel(Widget, HelpC *This, XtPointer)
{
   This->richText->Undelete();
}

void
HelpC::DoJustLeft(Widget, HelpC *This, XtPointer)
{
   This->richText->ChangeJust(JC_LEFT);
}

void
HelpC::DoJustRight(Widget, HelpC *This, XtPointer)
{
   This->richText->ChangeJust(JC_RIGHT);
}

void
HelpC::DoJustCenter(Widget, HelpC *This, XtPointer)
{
   This->richText->ChangeJust(JC_CENTER);
}

void
HelpC::DoIndentLeftMore(Widget, HelpC *This, XtPointer)
{
   This->richText->ChangeMargin(MC_LEFT_IN);
}

void
HelpC::DoIndentLeftLess(Widget, HelpC *This, XtPointer)
{
   This->richText->ChangeMargin(MC_LEFT_OUT);
}

void
HelpC::DoIndentRightMore(Widget, HelpC *This, XtPointer)
{
   This->richText->ChangeMargin(MC_RIGHT_IN);
}

void
HelpC::DoIndentRightLess(Widget, HelpC *This, XtPointer)
{
   This->richText->ChangeMargin(MC_RIGHT_OUT);
}

void
HelpC::DoColorRed(Widget, HelpC *This, XtPointer)
{
   This->richText->ChangeColor("Red");
}

void
HelpC::DoColorGreen(Widget, HelpC *This, XtPointer)
{
   This->richText->ChangeColor("Green");
}

void
HelpC::DoColorBlue(Widget, HelpC *This, XtPointer)
{
   This->richText->ChangeColor("Blue");
}

void
HelpC::DoColorYellow(Widget, HelpC *This, XtPointer)
{
   This->richText->ChangeColor("Yellow");
}

void
HelpC::DoColorMagenta(Widget, HelpC *This, XtPointer)
{
   This->richText->ChangeColor("Magenta");
}

void
HelpC::DoColorCyan(Widget, HelpC *This, XtPointer)
{
   This->richText->ChangeColor("Cyan");
}

void
HelpC::DoColorBlack(Widget, HelpC *This, XtPointer)
{
   This->richText->ChangeColor("Black");
}

void
HelpC::DoColorWhite(Widget, HelpC *This, XtPointer)
{
   This->richText->ChangeColor("White");
}

void
HelpC::DoColorNone(Widget, HelpC *This, XtPointer)
{
   This->richText->ChangeColor("None");
}

void
HelpC::TextChanged(Widget, HelpC *This, XtPointer)
{
   This->SetChanged(True);
}

void
HelpC::RichTextChanged(void*, HelpC *This)
{
   This->SetChanged(True);
}

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

/*------------------------------------------------------------------------
 * Check for changes to current information
 */

Boolean
HelpC::CheckChanges(Boolean cancelOk)
{
   if ( !changed ) return True;

//
// Ask about saving changes
//
   WArgList	args;
   args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
   if ( halApp->questionPM ) args.SymbolPixmap(halApp->questionPM);
   Widget	dialog = XmCreateQuestionDialog(helpWin, "saveChangesWin",ARGS);
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

   switch (answer) {

      case XmCR_ACTIVATE:
	 // Discard current
	 SetChanged(False);
         return True;

      case XmCR_CANCEL:
	 // Cancel operation
	 return False;

      default:
	 // Save current
         break;
   }

//
// If there is a  current card, create a record for the "mod" list.
// If there is no current card, create a record for the "add" list.
// We need a database or filename for new cards.
//
   CardRecC	*rec = new CardRecC;

   char	*cs = XmTextFieldGetString(nameTF);
   rec->name = cs;
   XtFree(cs);
   rec->name.Trim();

   cs = XmTextFieldGetString(locTF);
   rec->locator = cs;
   XtFree(cs);
   rec->locator.Trim();

   cs = XmTextFieldGetString(helpTitle);
   rec->title = cs;
   XtFree(cs);
   rec->title.Trim();

   if ( enriched ) {
      richText->GetString(rec->text, TT_ENRICHED);
   }
   else {
      cs = XmTextGetString(helpText);
      rec->text = cs;
      XtFree(cs);
   }

//
// Add entry to appropriate list
//
   void	*tmp = (void*)rec;
   if ( curCard ) {

      rec->db = curCard->db;
      modList.add(tmp);

//
// If the title has changed, update the index
//
      if ( indexList && rec->title != curCard->title ) {
      }
   }

   else {

      if ( dbList.size() == 1 ) {
	 rec->db = (HelpDbC*)*dbList[0];
      }

      else if ( !GetDbFileName(rec->file) ) {
	 delete rec;
	 return False;
      }

//
// Add this card to the index
//
      if ( indexList ) {
      }

      addList.add(tmp);
   }

   SetChanged(False);
   return True;

} // End CheckChanges

/*------------------------------------------------------------------------
 * Get name of database file for new card
 */

Boolean
HelpC::GetDbFileName(StringC& file)
{
//
// Prompt for file name
//
   WArgList	args;
   args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
   if ( halApp->questionPM ) args.SymbolPixmap(halApp->questionPM);
   Widget	dialog = XmCreatePromptDialog(helpWin, "dbFileNameWin",ARGS);
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
	    halApp->PopupMessage("Please enter a file name");
	    set_invalid(XmSelectionBoxGetChild(dialog, XmDIALOG_TEXT),
	    		True, True);
	 }
      }
   }

   XtUnmanageChild(dialog);
   XtDestroyWidget(dialog);
   XSync(halApp->display, False);

   return (answer != XmCR_CANCEL);

} // End GetDbFileName
#endif

/*-----------------------------------------------------------------------
 * Method to display a helpcard for the specified widget
 */

HelpCardC*
HelpC::FindCard(const Widget w, const char *res)
{
   if ( !isActive ) return NULL;

   StringC  name = CardName(w, res);
   return FindCard(name);
}

/*-----------------------------------------------------------------------
 * Method to find the help card by name.
 */

HelpCardC*
HelpC::FindCard(StringC& name)
{
   if ( name.size() > 0 ) {
//
//    Look through each database for the card name.
//
      u_int	count = dbList.size();
      for (int i=0; i<count; i++) {
         HelpDbC    *db   = (HelpDbC*)*dbList[i];
         HelpCardC  *card = db->FindCard((char*)name);
         if ( card ) return card;
      }
   }

   return NULL;
}

Boolean
HelpC::Editable() const
{
#if EDIT_OK
   return edit;
#else
   return False;
#endif
}

HelpResWinC*
HelpC::HelpResWin()
{
#if EDIT_OK
   if ( !helpResWin ) helpResWin = new HelpResWinC(*halApp);
#endif
   return helpResWin;
}
