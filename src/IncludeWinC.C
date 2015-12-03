/*
 * $Id: IncludeWinC.C,v 1.4 2000/06/06 12:46:10 evgeny Exp $
 *
 * Copyright (c) 1993 HAL Computer Systems International, Ltd.
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

#include "IncludeWinC.h"
#include "Mailcap.h"
#include "SendIconC.h"
#include "FileMisc.h"
#include "Misc.h"
#include "MsgPartC.h"
#include "ParamC.h"

#include <hgl/HalAppC.h>
#include <hgl/WArgList.h>
#include <hgl/StringListC.h>
#include <hgl/RowColC.h>
#include <hgl/WXmString.h>
#include <hgl/StrCase.h>
#include <hgl/rsrc.h>
#include <hgl/TextMisc.h>
#include <hgl/StringDictC.h>
#include <hgl/MemMap.h>

#include <Xm/PanedW.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/TextF.h>
#include <Xm/Frame.h>
#include <Xm/Text.h>
#include <Xm/PushB.h>
#include <Xm/ToggleB.h>
#include <Xm/RowColumn.h>
#include <Xm/CascadeB.h>

#include <time.h>
#include <unistd.h>	// For access
#include <sys/stat.h>

/*---------------------------------------------------------------
 *  IncludeWinC constructor
 */

IncludeWinC::IncludeWinC(Widget parent, const char *name)
: HalDialogC(name, parent)
{
   WArgList	args;
   Widget	wlist[11];
   int		wcount;

   fileList.AllowDuplicates(FALSE);
   fileList.SetSorted(FALSE);

//
// Create appForm hierarchy
//
// appForm
//    PanedWindow	appPanes
//
   args.Reset();
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_FORM);
   args.TopAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   Widget	appPanes = XmCreatePanedWindow(appForm, "appPanes", ARGS);

//
// Create appPanes hierarchy
//
// appPanes
//    RowColC		*fileRC
//    Form		includeForm
//    Form		encodeForm
//    RowColC		*paramRC
//
   fileRC       = new RowColC (appPanes, "fileRC",       0,0);
   includeForm  = XmCreateForm(appPanes, "includeForm",  0,0);
   encodeForm   = XmCreateForm(appPanes, "encodeForm",   0,0);
   paramRC      = new RowColC (appPanes, "paramRC",      0,0);

//
// Create fileRC hierarchy
//
// fileRC
//   Label	nameLabel
//   TextField	nameTF
//   Label	descLabel
//   TextField	descTF
//   Label	typeLabel
//   OptionMenu	typeOM
//

//
// Set up 2 columns in fileRC
//
   fileRC->Defer(True);
   fileRC->SetOrientation(RcROW_MAJOR);
   fileRC->SetColCount(2);
   fileRC->SetColAlignment(XmALIGNMENT_END);
   fileRC->SetColWidthAdjust(0, RcADJUST_NONE);
   fileRC->SetColWidthAdjust(1, RcADJUST_ATTACH);
   fileRC->SetColResize(0, False);
   fileRC->SetColResize(1, True);

   Widget nameLabel = XmCreateLabel       (*fileRC, "nameLabel", 0,0);
   nameTF           = CreateTextField   (*fileRC, "nameTF",    0,0);
   Widget descLabel = XmCreateLabel       (*fileRC, "descLabel", 0,0);
   descTF           = CreateTextField   (*fileRC, "descTF",    0,0);
   Widget typeLabel = XmCreateLabel       (*fileRC, "typeLabel", 0,0);
   typePD           = XmCreatePulldownMenu(*fileRC, "typePD",    0,0);
   args.Reset();
   args.SubMenuId(typePD);
   typeOM           = XmCreateOptionMenu  (*fileRC, "typeOM",    ARGS);

   fileRC->AddChild(nameLabel);
   fileRC->AddChild(nameTF);
   fileRC->AddChild(descLabel);
   fileRC->AddChild(descTF);
   fileRC->AddChild(typeLabel);
   fileRC->AddChild(typeOM);
   fileRC->Defer(False);

//
// Create typePD hierarchy
//
// typePD
//    CascadeButton	typeAppCB
//    PulldownMenu	typeAppPD
//    CascadeButton	typeAudioCB
//    PulldownMenu	typeAudioPD
//    CascadeButton	typeImageCB
//    PulldownMenu	typeImagePD
//    CascadeButton	typeMsgCB
//    PulldownMenu	typeMsgPD
//    CascadeButton	typeMultCB
//    PulldownMenu	typeMultPD
//    CascadeButton	typeTextCB
//    PulldownMenu	typeTextPD
//    CascadeButton	typeVideoCB
//    PulldownMenu	typeVideoPD
//
   typeAppPD   = XmCreatePulldownMenu(typePD, "typeAppPD",   0,0);
   typeAudioPD = XmCreatePulldownMenu(typePD, "typeAudioPD", 0,0);
   typeImagePD = XmCreatePulldownMenu(typePD, "typeImagePD", 0,0);
   typeMsgPD   = XmCreatePulldownMenu(typePD, "typeMsgPD",   0,0);
   typeMultPD  = XmCreatePulldownMenu(typePD, "typeMultPD",  0,0);
   typeTextPD  = XmCreatePulldownMenu(typePD, "typeTextPD",  0,0);
   typeVideoPD = XmCreatePulldownMenu(typePD, "typeVideoPD", 0,0);

   args.Reset();
   args.SubMenuId(typeAppPD);
   typeAppCB   = XmCreateCascadeButton(typePD, "typeAppCB",   ARGS);
   args.SubMenuId(typeAudioPD);
   typeAudioCB = XmCreateCascadeButton(typePD, "typeAudioCB", ARGS);
   args.SubMenuId(typeImagePD);
   typeImageCB = XmCreateCascadeButton(typePD, "typeImageCB", ARGS);
   args.SubMenuId(typeMsgPD);
   typeMsgCB   = XmCreateCascadeButton(typePD, "typeMsgCB",   ARGS);
   args.SubMenuId(typeMultPD);
   typeMultCB  = XmCreateCascadeButton(typePD, "typeMultCB",  ARGS);
   args.SubMenuId(typeTextPD);
   typeTextCB  = XmCreateCascadeButton(typePD, "typeTextCB",  ARGS);
   args.SubMenuId(typeVideoPD);
   typeVideoCB = XmCreateCascadeButton(typePD, "typeVideoCB", ARGS);

   wcount = 0;
   wlist[wcount++] = typeAppCB;
   wlist[wcount++] = typeAudioCB;
   wlist[wcount++] = typeImageCB;
   wlist[wcount++] = typeMsgCB;
   wlist[wcount++] = typeMultCB;
   wlist[wcount++] = typeTextCB;
   wlist[wcount++] = typeVideoCB;
   XtManageChildren(wlist, wcount);	// typePD children

   typeList.AllowDuplicates(FALSE);
   BuildTypeList();
   BuildTypeMenu();

//
// Create includeForm hierarchy
//
// includeForm
//   Label	includeLabel
//   Frame	includeFrame
//   RadioBox	   includeRadio
//   ToggleButton      (radioButtons)
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_NONE);
   Widget includeLabel = XmCreateLabel(includeForm, "includeLabel", ARGS);

   args.TopAttachment(XmATTACH_WIDGET, includeLabel);
   args.RightAttachment(XmATTACH_FORM);
   Widget includeFrame = XmCreateFrame(includeForm, "includeFrame", ARGS);

   args.Reset();
   args.Packing(XmPACK_COLUMN);
   args.Orientation(XmVERTICAL);
   Widget includeRadio = XmCreateRadioBox(includeFrame, "includeRadio", ARGS);

   includeAsTextTB = XmCreateToggleButton(includeRadio, "includeAsTextTB", 0,0);
   includeAsFileTB = XmCreateToggleButton(includeRadio, "includeAsFileTB", 0,0);
   attachLocalTB   = XmCreateToggleButton(includeRadio, "attachLocalTB",   0,0);
   attachAnonTB    = XmCreateToggleButton(includeRadio, "attachAnonTB",    0,0);
   attachFtpTB     = XmCreateToggleButton(includeRadio, "attachFtpTB",     0,0);
   attachTftpTB    = XmCreateToggleButton(includeRadio, "attachTftpTB",    0,0);
   attachMailTB    = XmCreateToggleButton(includeRadio, "attachMailTB",    0,0);

   XtManageChild(includeAsTextTB);
   XtManageChild(includeAsFileTB);
   XtManageChild(attachLocalTB);
   XtManageChild(attachAnonTB);
   XtManageChild(attachFtpTB);
   XtManageChild(attachTftpTB);
   XtManageChild(attachMailTB);
   XtManageChild(includeRadio);

   XtManageChild(includeLabel);
   XtManageChild(includeFrame);

   XtAddCallback(includeAsTextTB, XmNvalueChangedCallback,
   		 (XtCallbackProc)IncludeTypeChanged, this);
   XtAddCallback(includeAsFileTB, XmNvalueChangedCallback,
   		 (XtCallbackProc)IncludeTypeChanged, this);
   XtAddCallback(attachLocalTB, XmNvalueChangedCallback,
   		 (XtCallbackProc)IncludeTypeChanged, this);
   XtAddCallback(attachAnonTB, XmNvalueChangedCallback,
   		 (XtCallbackProc)IncludeTypeChanged, this);
   XtAddCallback(attachFtpTB, XmNvalueChangedCallback,
   		 (XtCallbackProc)IncludeTypeChanged, this);
   XtAddCallback(attachTftpTB, XmNvalueChangedCallback,
   		 (XtCallbackProc)IncludeTypeChanged, this);
   XtAddCallback(attachMailTB, XmNvalueChangedCallback,
   		 (XtCallbackProc)IncludeTypeChanged, this);

//
// Create encodeForm hierarchy
//
// encodeForm
//   Label		encodeLabel
//   Frame		encodeFrame
//   RadioBox		   encodeRadio
//   ToggleButton	      (radioButtons)
//   ToggleButton	preEncodeTB
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_NONE);
   args.BottomAttachment(XmATTACH_NONE);
   Widget encodeLabel = XmCreateLabel(encodeForm, "encodeLabel", ARGS);

   args.TopAttachment(XmATTACH_WIDGET, encodeLabel);
   args.RightAttachment(XmATTACH_FORM);
   Widget encodeFrame = XmCreateFrame(encodeForm, "encodeFrame", ARGS);

   args.TopAttachment(XmATTACH_WIDGET, encodeFrame);
   args.RightAttachment(XmATTACH_NONE);
   preEncodeTB = XmCreateToggleButton(encodeForm, "preEncodeTB", ARGS);

   args.Reset();
   args.Packing(XmPACK_TIGHT);
   args.Orientation(XmHORIZONTAL);
   Widget encodeRadio = XmCreateRadioBox(encodeFrame, "encodeRadio", ARGS);

   encodeNoneTB   = XmCreateToggleButton(encodeRadio, "encodeNoneTB",   0,0);
   encode8bitTB   = XmCreateToggleButton(encodeRadio, "encode8bitTB",   0,0);
   encodeQpTB     = XmCreateToggleButton(encodeRadio, "encodeQpTB",     0,0);
   encode64TB     = XmCreateToggleButton(encodeRadio, "encode64TB",     0,0);
   encodeUuTB     = XmCreateToggleButton(encodeRadio, "encodeUuTB",     0,0);
   encodeBinHexTB = XmCreateToggleButton(encodeRadio, "encodeBinHexTB", 0,0);

   XtManageChild(encodeNoneTB);
   XtManageChild(encode8bitTB);
   XtManageChild(encodeQpTB);
   XtManageChild(encode64TB);
   XtManageChild(encodeUuTB);
//   XtManageChild(encodeBinHexTB);
   XtManageChild(encodeRadio);

   XtManageChild(encodeLabel);
   XtManageChild(encodeFrame);
   XtManageChild(preEncodeTB);

//
// Create paramForm hierarchy
//
//   paramRC
//      Label		   charLabel
//      Form		   charForm
//      Label		   octTypeLabel
//      TextField	   octTypeTF
//      Label		   octPaddingLabel
//      TextField	   octPaddingTF
//      Label		   otherLabel
//      Text		   otherText
//      Label		   hostLabel
//      TextField	   hostTF
//      Label		   dirLabel
//      TextField	   dirTF
//      Label		   ftpModeLabel
//      TextField	   ftpModeFrame
//      Label		   tftpModeLabel
//      Text		   tftpModeFrame
//      Label		   serverLabel
//      TextField	   serverTF
//      Label		   subjectLabel
//      TextField	   subjectTF
//      Label		   bodyLabel
//      Text		   bodyText
//      Label		   expLabel
//      TextField	   expTF
//      Label		   sizeLabel
//      TextField	   sizeTF
//      Label		   permLabel
//      TextField	   permFrame
//

//
// Set up 2 columns in paramRC
//
   paramRC->Defer(True);
   paramRC->SetOrientation(RcROW_MAJOR);
   paramRC->SetColCount(2);
   paramRC->SetColAlignment(XmALIGNMENT_END);
   paramRC->SetColWidthAdjust(0, RcADJUST_NONE);
   paramRC->SetColWidthAdjust(1, RcADJUST_ATTACH);
   paramRC->SetColResize(0, False);
   paramRC->SetColResize(1, True);

   Widget charLabel     = XmCreateLabel    (*paramRC, "charLabel",     0,0);
   Widget charForm      = XmCreateForm     (*paramRC, "charForm",      0,0);
   Widget outNameLabel  = XmCreateLabel    (*paramRC, "outNameLabel",  0,0);
   outNameTF            = CreateTextField(*paramRC, "outNameTF",     0,0);
   Widget octTypeLabel  = XmCreateLabel    (*paramRC, "octTypeLabel",  0,0);
   octTypeTF            = CreateTextField(*paramRC, "octTypeTF",     0,0);
   Widget octPadLabel   = XmCreateLabel    (*paramRC, "octPadLabel",   0,0);
   octPadTF             = CreateTextField(*paramRC, "octPadTF",      0,0);
   Widget otherLabel    = XmCreateLabel    (*paramRC, "otherLabel",    0,0);
   args.Reset();
   args.EditMode(XmMULTI_LINE_EDIT);
   otherText            = CreateText     (*paramRC, "otherText",     ARGS);
   Widget hostLabel     = XmCreateLabel    (*paramRC, "hostLabel",     0,0);
   hostTF               = CreateTextField(*paramRC, "hostTF",        0,0);
   Widget dirLabel      = XmCreateLabel    (*paramRC, "dirLabel",      0,0);
   dirTF                = CreateTextField(*paramRC, "dirTF",         0,0);
   Widget ftpModeLabel  = XmCreateLabel    (*paramRC, "ftpModeLabel",  0,0);
   Widget ftpModeFrame  = XmCreateFrame    (*paramRC, "ftpModeFrame",  0,0);
   Widget tftpModeLabel = XmCreateLabel    (*paramRC, "tftpModeLabel", 0,0);
   Widget tftpModeFrame = XmCreateFrame    (*paramRC, "tftpModeFrame", 0,0);
   Widget serverLabel   = XmCreateLabel    (*paramRC, "serverLabel",   0,0);
   serverTF             = CreateTextField(*paramRC, "serverTF",      0,0);
   Widget subjectLabel  = XmCreateLabel    (*paramRC, "subjectLabel",  0,0);
   subjectTF            = CreateTextField(*paramRC, "subjectTF",     0,0);
   Widget bodyLabel     = XmCreateLabel    (*paramRC, "bodyLabel",     0,0);
   args.Reset();
   args.EditMode(XmMULTI_LINE_EDIT);
   bodyText             = CreateText     (*paramRC, "bodyText",      ARGS);
   Widget expLabel      = XmCreateLabel    (*paramRC, "expLabel",      0,0);
   expTF                = CreateTextField(*paramRC, "expTF",         0,0);
   Widget sizeLabel     = XmCreateLabel    (*paramRC, "sizeLabel",     0,0);
   sizeTF               = CreateTextField(*paramRC, "sizeTF",        0,0);
   Widget permLabel     = XmCreateLabel    (*paramRC, "permLabel",     0,0);
   Widget permFrame     = XmCreateFrame    (*paramRC, "permFrame",     0,0);

//
// Create charForm hierarchy
//
// charForm
//    TextField		charTF
//    MenuBar		charMB
//       CascadeButton	charCB
//       PulldownMenu	charPD
//          PushButton	charAsciiPB
//          PushButton	charIso1PB
//          PushButton	charIso2PB
//          PushButton	charIso3PB
//          PushButton	charIso4PB
//          PushButton	charIso5PB
//          PushButton	charIso6PB
//          PushButton	charIso7PB
//          PushButton	charIso8PB
//          PushButton	charIso9PB
//          PushButton	charIso13PB
//
   args.Reset();
   args.TopAttachment(XmATTACH_FORM);
   args.LeftAttachment(XmATTACH_NONE);
   args.RightAttachment(XmATTACH_FORM);
   args.BottomAttachment(XmATTACH_FORM);
   Widget charMB = XmCreateMenuBar(charForm, "charMB", ARGS);

   args.LeftAttachment(XmATTACH_FORM);
   args.RightAttachment(XmATTACH_WIDGET, charMB);
   args.Value("us-ascii");
   charTF = CreateTextField(charForm, "charTF", ARGS);

   Widget charPD = XmCreatePulldownMenu(charMB, "charPD", 0,0);

   charAsciiPB = XmCreatePushButton(charPD, "us-ascii",   0,0);
   charIso1PB  = XmCreatePushButton(charPD, "iso-8859-1", 0,0);
   charIso2PB  = XmCreatePushButton(charPD, "iso-8859-2", 0,0);
   charIso3PB  = XmCreatePushButton(charPD, "iso-8859-3", 0,0);
   charIso4PB  = XmCreatePushButton(charPD, "iso-8859-4", 0,0);
   charIso5PB  = XmCreatePushButton(charPD, "iso-8859-5", 0,0);
   charIso6PB  = XmCreatePushButton(charPD, "iso-8859-6", 0,0);
   charIso7PB  = XmCreatePushButton(charPD, "iso-8859-7", 0,0);
   charIso8PB  = XmCreatePushButton(charPD, "iso-8859-8", 0,0);
   charIso9PB  = XmCreatePushButton(charPD, "iso-8859-9", 0,0);
   charIso13PB  = XmCreatePushButton(charPD, "iso-8859-13", 0,0);

   XtAddCallback(charAsciiPB, XmNactivateCallback, (XtCallbackProc)DoCharset,
   		 this);
   XtAddCallback(charIso1PB,  XmNactivateCallback, (XtCallbackProc)DoCharset,
   		 this);
   XtAddCallback(charIso2PB,  XmNactivateCallback, (XtCallbackProc)DoCharset,
   		 this);
   XtAddCallback(charIso3PB,  XmNactivateCallback, (XtCallbackProc)DoCharset,
   		 this);
   XtAddCallback(charIso4PB,  XmNactivateCallback, (XtCallbackProc)DoCharset,
   		 this);
   XtAddCallback(charIso5PB,  XmNactivateCallback, (XtCallbackProc)DoCharset,
   		 this);
   XtAddCallback(charIso6PB,  XmNactivateCallback, (XtCallbackProc)DoCharset,
   		 this);
   XtAddCallback(charIso7PB,  XmNactivateCallback, (XtCallbackProc)DoCharset,
   		 this);
   XtAddCallback(charIso8PB,  XmNactivateCallback, (XtCallbackProc)DoCharset,
   		 this);
   XtAddCallback(charIso9PB,  XmNactivateCallback, (XtCallbackProc)DoCharset,
   		 this);
   XtAddCallback(charIso13PB,  XmNactivateCallback, (XtCallbackProc)DoCharset,
   		 this);

   wlist[0] = charAsciiPB;
   wlist[1] = charIso1PB;
   wlist[2] = charIso2PB;
   wlist[3] = charIso3PB;
   wlist[4] = charIso4PB;
   wlist[5] = charIso5PB;
   wlist[6] = charIso6PB;
   wlist[7] = charIso7PB;
   wlist[8] = charIso8PB;
   wlist[9] = charIso9PB;
   wlist[10] = charIso13PB;
   XtManageChildren(wlist, 11);		// charPD children

   args.Reset();
   args.SubMenuId(charPD);
   Widget charCB = XmCreateCascadeButton(charMB, "charCB", ARGS);
   XtManageChild(charCB);

   wlist[0] = charMB;
   wlist[1] = charTF;
   XtManageChildren(wlist, 2);	// charForm children

//
// Create ftpModeFrame hierarchy
//
//    Frame			ftpModeFrame
//       RadioBox		ftpModeRadio
//          ToggleButton	   (buttons)
//
   args.Reset();
   args.Packing(XmPACK_TIGHT);
   args.Orientation(XmHORIZONTAL);
   Widget ftpModeRadio = XmCreateRadioBox(ftpModeFrame, "ftpModeRadio", ARGS);

   ftpModeAsciiTB  = XmCreateToggleButton(ftpModeRadio, "ftpModeAsciiTB",  0,0);
   ftpModeEbcdicTB = XmCreateToggleButton(ftpModeRadio, "ftpModeEbcdicTB", 0,0);
   ftpModeImageTB  = XmCreateToggleButton(ftpModeRadio, "ftpModeImageTB",  0,0);
//   ftpModeLocalTB  = XmCreateToggleButton(ftpModeRadio, "ftpModeLocalTB",  0,0);

   XtManageChild(ftpModeAsciiTB);
   XtManageChild(ftpModeEbcdicTB);
   XtManageChild(ftpModeImageTB);
//   XtManageChild(ftpModeLocalTB);
   XtManageChild(ftpModeRadio);

//
// Create tftpModeFrame hierarchy
//
//    Frame			tftpModeFrame
//       RadioBox		tftpModeRadio
//          ToggleButton	   (buttons)
//
   args.Reset();
   args.Packing(XmPACK_TIGHT);
   args.Orientation(XmHORIZONTAL);
   Widget tftpModeRadio = XmCreateRadioBox(tftpModeFrame, "tftpModeRadio",ARGS);

   tftpModeNetAsciiTB =
      XmCreateToggleButton(tftpModeRadio, "tftpModeNetAsciiTB",  0,0);
   tftpModeOctetTB = XmCreateToggleButton(tftpModeRadio, "tftpModeOctetTB",0,0);
   tftpModeMailTB  = XmCreateToggleButton(tftpModeRadio, "tftpModeMailTB", 0,0);

   XtManageChild(tftpModeNetAsciiTB);
   XtManageChild(tftpModeOctetTB);
   XtManageChild(tftpModeMailTB);
   XtManageChild(tftpModeRadio);

//
// Create permFrame hierarchy
//
//    Frame			permFrame
//       RadioBox		permRadio
//          ToggleButton	   (buttons)
//
   args.Reset();
   args.Packing(XmPACK_TIGHT);
   args.Orientation(XmHORIZONTAL);
   Widget permRadio = XmCreateRadioBox(permFrame, "permRadio", ARGS);

   permRoTB = XmCreateToggleButton(permRadio, "permRoTB", 0,0);
   permRwTB = XmCreateToggleButton(permRadio, "permRwTB", 0,0);

   XtManageChild(permRoTB);
   XtManageChild(permRwTB);
   XtManageChild(permRadio);

   XmToggleButtonSetState(permRoTB, True, True);

#if 0
//
// Add paramRC children
//
   paramRC->AddChild(charLabel);
   paramRC->AddChild(charForm);
   paramRC->AddChild(outNameLabel);
   paramRC->AddChild(outNameTF);
   paramRC->AddChild(octTypeLabel);
   paramRC->AddChild(octTypeTF);
   paramRC->AddChild(octPadLabel);
   paramRC->AddChild(octPadTF);
   paramRC->AddChild(otherLabel);
   paramRC->AddChild(otherText);
   paramRC->AddChild(hostLabel);
   paramRC->AddChild(hostTF);
   paramRC->AddChild(dirLabel);
   paramRC->AddChild(dirTF);
   paramRC->AddChild(ftpModeLabel);
   paramRC->AddChild(ftpModeFrame);
   paramRC->AddChild(tftpModeLabel);
   paramRC->AddChild(tftpModeFrame);
   paramRC->AddChild(serverLabel);
   paramRC->AddChild(serverTF);
   paramRC->AddChild(subjectLabel);
   paramRC->AddChild(subjectTF);
   paramRC->AddChild(bodyLabel);
   paramRC->AddChild(bodyText);
   paramRC->AddChild(expLabel);
   paramRC->AddChild(expTF);
   paramRC->AddChild(sizeLabel);
   paramRC->AddChild(sizeTF);
   paramRC->AddChild(permLabel);
   paramRC->AddChild(permFrame);
   paramRC->Defer(False);
#endif

   int	rowNum = 0;
   charsetRow  = rowNum++;
   outNameRow  = rowNum++;
   octTypeRow  = rowNum++;
   octPadRow   = rowNum++;
   otherRow    = rowNum++;
   hostRow     = rowNum++;
   dirRow      = rowNum++;
   ftpModeRow  = rowNum++;
   tftpModeRow = rowNum++;
   serverRow   = rowNum++;
   subjectRow  = rowNum++;
   bodyRow     = rowNum++;
   expRow      = rowNum++;
   sizeRow     = rowNum++;
   permRow     = rowNum++;

   XtManageChild(*fileRC);
   XtManageChild(includeForm);
   XtManageChild(encodeForm);
   //XtManageChild(paramForm);

   XtManageChild(appPanes);

//
// Add buttons
//
   AddButtonBox();
   Widget	okPB     = XmCreatePushButton(buttonRC, "okPB",     0,0);
   skipPB                = XmCreatePushButton(buttonRC, "skipPB",   0,0);
   Widget	cancelPB = XmCreatePushButton(buttonRC, "cancelPB", 0,0);
   Widget	helpPB   = XmCreatePushButton(buttonRC, "helpPB",   0,0);

   XtAddCallback(okPB,     XmNactivateCallback, (XtCallbackProc)DoOk,   this);
   XtAddCallback(skipPB,   XmNactivateCallback, (XtCallbackProc)DoSkip, this);
   XtAddCallback(cancelPB, XmNactivateCallback, (XtCallbackProc)DoHide, this);
   XtAddCallback(helpPB,   XmNactivateCallback, (XtCallbackProc)HalAppC::DoHelp,
   		 (char *) "helpcard");

   XtManageChild(okPB);
   XtManageChild(cancelPB);
   XtManageChild(helpPB);

   HandleHelp();

//
// Add callbacks to finish initialization
//
   XtAddCallback(shell, XmNpopupCallback, (XtCallbackProc)DoPopup,
                    (XtPointer)this);

   XtAddCallback(*fileRC, XmNexposeCallback, (XtCallbackProc)DoFileExpose,
                 this);
   XtAddEventHandler(includeForm, StructureNotifyMask, False,
		     (XtEventHandler)DoIncludeResize, this);
   XtAddEventHandler(encodeForm, StructureNotifyMask, False,
		     (XtEventHandler)DoEncodeResize, this);
   XtAddCallback(*paramRC, XmNexposeCallback, (XtCallbackProc)DoParamExpose,
                 this);

   return;

} // End constructor

/*---------------------------------------------------------------
 *  Method to build a list of file types
 */

void
IncludeWinC::BuildTypeList()
{
   typeList.SetSorted(FALSE);
   typeList.removeAll();

//
// Build a sorted list of the standard types plus any additional types found
//    in the mailcap and mime-types files.
//
   StringC	typeStr;
   typeStr = "application/octet-stream";	typeList.add(typeStr);
   typeStr = "application/postscript";		typeList.add(typeStr);
   typeStr = "audio/basic";			typeList.add(typeStr);
   typeStr = "image/gif";			typeList.add(typeStr);
   typeStr = "image/jpeg";			typeList.add(typeStr);
   typeStr = "message/rfc822";			typeList.add(typeStr);
   typeStr = "text/enriched";			typeList.add(typeStr);
   typeStr = "text/plain";			typeList.add(typeStr);
   typeStr = "video/mpeg";			typeList.add(typeStr);

//
// Loop through the mailcap tree
//
   MailcapFileC	*mfile = MailcapFiles;
   while ( mfile ) {

      u_int	count = mfile->entryList.size();
      for (int i=0; i<count; i++) {

	 MailcapC	*ent = mfile->entryList[i];
	 if ( ent->conType != "*" && ent->subType != "*" ) {

            typeList.add(ent->fullType);
#if 0
	    typeStr = ent->conType;
	    typeStr += "/";
	    typeStr += ent->subType;
	    typeStr.Trim();
	    if ( typeStr.size() > 0 ) typeList.add(typeStr);
#endif
	 }

      } // End for each mailcap entry

      mfile = mfile->next;

   } // End for each mailcap file

//
// Loop through the mime types dictionary
//
   StringDictC&	dict = MimeTypeDict();
   u_int	count = dict.size();
   for (int i=0; i<count; i++) {
      StringDictCEntry	*ent = dict[i];
      typeList.add(ent->val);
   }

//
// Sort the types
//
   typeList.SetSorted(TRUE);

//
// Mark the time this list was created
//
   typeListTime = time(0);

} // End BuildTypeList

/*---------------------------------------------------------------
 *  Method to build an option menu of file types
 */

void
IncludeWinC::BuildTypeMenu()
{
//
// Delete existing buttons
//
   unsigned	count = typePbList.size();
   int	i;
   for (i=0; i<count; i++) XtDestroyWidget(*typePbList[i]);
   typePbList.removeAll();

//
// Create new buttons
//
   WArgList	args;
   WXmString	wstr;
   count = typeList.size();
   for (i=0; i<count; i++) {

      StringC	*label = typeList[i];

//
// See which menu these are to be added to.  If there is no match, add them
//    to the top level menu
//
      Widget	parent = typePD;
      if      ( label->StartsWith("application/", IGNORE_CASE) )
	 parent = typeAppPD;
      else if ( label->StartsWith("audio/", IGNORE_CASE) )
	 parent = typeAudioPD;
      else if ( label->StartsWith("image/", IGNORE_CASE) )
	 parent = typeImagePD;
      else if ( label->StartsWith("message/", IGNORE_CASE) )
	 parent = typeMsgPD;
      else if ( label->StartsWith("multipart/", IGNORE_CASE) )
	 parent = typeMultPD;
      else if ( label->StartsWith("text/", IGNORE_CASE) )
	 parent = typeTextPD;
      else if ( label->StartsWith("video/", IGNORE_CASE) )
	 parent = typeVideoPD;

//
// Create a button
//
      wstr = (char*)*label;
      args.LabelString(wstr);
      Widget pb = XmCreatePushButton(parent, "typePB", ARGS);
      XtManageChild(pb);

      XtAddCallback(pb, XmNactivateCallback, (XtCallbackProc)FileTypeChanged,
		    this);
      typePbList.add(pb);

   } // End for each button to be added

//
// Unmanage cascade buttons with no children
//
   Cardinal	childCount;
   XtVaGetValues(typeAppPD, XmNnumChildren, &childCount, NULL);
   if ( childCount > 0 ) XtManageChild(typeAppCB);
   else			 XtUnmanageChild(typeAppCB);

   XtVaGetValues(typeAudioPD, XmNnumChildren, &childCount, NULL);
   if ( childCount > 0 ) XtManageChild(typeAudioCB);
   else			 XtUnmanageChild(typeAudioCB);

   XtVaGetValues(typeImagePD, XmNnumChildren, &childCount, NULL);
   if ( childCount > 0 ) XtManageChild(typeImageCB);
   else			 XtUnmanageChild(typeImageCB);

   XtVaGetValues(typeMsgPD, XmNnumChildren, &childCount, NULL);
   if ( childCount > 0 ) XtManageChild(typeMsgCB);
   else			 XtUnmanageChild(typeMsgCB);

   XtVaGetValues(typeMultPD, XmNnumChildren, &childCount, NULL);
   if ( childCount > 0 ) XtManageChild(typeMultCB);
   else			 XtUnmanageChild(typeMultCB);

   XtVaGetValues(typeTextPD, XmNnumChildren, &childCount, NULL);
   if ( childCount > 0 ) XtManageChild(typeTextCB);
   else			 XtUnmanageChild(typeTextCB);

   XtVaGetValues(typeVideoPD, XmNnumChildren, &childCount, NULL);
   if ( childCount > 0 ) XtManageChild(typeVideoCB);
   else			 XtUnmanageChild(typeVideoCB);

} // End BuildTypeMenus

/*---------------------------------------------------------------
 *  Callback routine to handle initial display.
 */

void
IncludeWinC::DoPopup(Widget, IncludeWinC *This, XtPointer)
{
   XtRemoveCallback(This->shell, XmNpopupCallback, (XtCallbackProc)DoPopup,
		    (XtPointer)This);

//
// Finish initializing paramRC
//
   WidgetList	wlist;
   Cardinal	wcount;
   XtVaGetValues(*This->paramRC, XmNchildren, &wlist, XmNnumChildren, &wcount,
   		 NULL);
   This->paramRC->SetChildren(wlist, wcount);
   This->paramRC->Defer(False);
}

/*-----------------------------------------------------------------------
 *  Handle initial exposure of fileRC
 */

void
IncludeWinC::DoFileExpose(Widget, IncludeWinC *This, XtPointer)
{
//
// Remove this callback
//
   XtRemoveCallback(*This->fileRC, XmNexposeCallback,
		    (XtCallbackProc)DoFileExpose, This);

//
// Fix sizes of pane
//
   Dimension	ht;
   XtVaGetValues(*This->fileRC, XmNheight, &ht, NULL);
   XtVaSetValues(*This->fileRC, XmNpaneMinimum, ht, XmNpaneMaximum, ht, NULL);

} // End DoFileExpose

/*-----------------------------------------------------------------------
 *  Handle resize of includeForm
 */

void
IncludeWinC::DoIncludeResize(Widget, IncludeWinC *This, XEvent *ev, Boolean*)
{
   if ( ev->type != ConfigureNotify ) return;

//
// Fix sizes of pane
//
   Dimension	ht;
   XtVaGetValues(This->includeForm, XmNheight, &ht, NULL);
   XtVaSetValues(This->includeForm, XmNpaneMinimum, ht, XmNpaneMaximum, ht, 0);

} // End DoIncludeResize

/*-----------------------------------------------------------------------
 *  Handle resize of encodeForm
 */

void
IncludeWinC::DoEncodeResize(Widget, IncludeWinC *This, XEvent *ev, Boolean*)
{
   if ( ev->type != ConfigureNotify ) return;

//
// Fix sizes of pane
//
   Dimension	ht;
   XtVaGetValues(This->encodeForm, XmNheight, &ht, NULL);
   XtVaSetValues(This->encodeForm, XmNpaneMinimum, ht, XmNpaneMaximum, ht, 0);

} // End DoEncodeResize

/*-----------------------------------------------------------------------
 *  Handle initial exposure of paramRC
 */

void
IncludeWinC::DoParamExpose(Widget, IncludeWinC *This, XtPointer)
{
//
// Remove this callback
//
   XtRemoveCallback(*This->paramRC, XmNexposeCallback,
		    (XtCallbackProc)DoParamExpose, This);

//
// Fix sizes of pane
//
   Dimension	ht;
   XtVaGetValues(*This->paramRC, XmNheight, &ht, NULL);
   XtVaSetValues(*This->paramRC, XmNpaneMinimum, ht, XmNpaneMaximum, ht, NULL);

} // End DoParamExpose

/*---------------------------------------------------------------
 *  IncludeWinC destructor
 */

IncludeWinC::~IncludeWinC()
{
   delete fileRC;
   delete paramRC;
}

/*---------------------------------------------------------------
 *  Method to edit data for a single file
 */

void
IncludeWinC::Show(SendIconC *file)
{
   fileList.removeAll();
   fileList.add(file->data->dataFile);
   fileIndex = 0;

   typeStr = file->data->conStr;
   typeIndex = typeList.indexOf(typeStr);

   XtUnmanageChild(skipPB);
   HalDialogC::Show();

//
// Initialize fields
//
   StringC	tmpStr;
   file->data->GetDescription(tmpStr);
   XmTextFieldSetString(descTF, tmpStr);
   XtVaSetValues(typeOM, XmNmenuHistory, *typePbList[typeIndex], NULL);

   Widget	tb = NULL;
   switch ( file->data->accType ) {
      case (AT_INLINE):		tb = includeAsFileTB;	break;
      case (AT_LOCAL_FILE):	tb = attachLocalTB;	break;
      case (AT_ANON_FTP):	tb = attachAnonTB;	break;
      case (AT_FTP):		tb = attachFtpTB;	break;
      case (AT_TFTP):		tb = attachTftpTB;	break;
      case (AT_MAIL_SERVER):	tb = attachMailTB;	break;
   }
   if ( tb ) XmToggleButtonSetState(tb, True, True);

   tb = NULL;
   switch ( file->data->encType ) {
      case (ET_NONE):		tb = encodeNoneTB;	break;
      case (ET_8BIT):		tb = encode8bitTB;	break;
      case (ET_BASE_64):	tb = encode64TB;	break;
      case (ET_BINHEX):		tb = encodeBinHexTB;	break;
      case (ET_QP):		tb = encodeQpTB;	break;
      case (ET_UUENCODE):	tb = encodeUuTB;	break;
   }
   if ( tb ) XmToggleButtonSetState(tb, True, True);
   XmToggleButtonSetState(preEncodeTB, file->data->IsEncoded(), True);

   CharC	val;
   ParamC	*param = file->data->Param("charset");
   if ( param ) val = param->val;
   else		val = "";
   TextFieldSetString(charTF, val);

   param = file->data->Param("name");
   if ( param ) val = param->val;
   else		val = file->data->dataFile;
   TextFieldSetString(nameTF, val);

   param = file->data->Param("filename");
   if ( param ) val = param->val;
   else		val = "";
   TextFieldSetString(outNameTF, val);

   param = file->data->Param("type");
   if ( param ) val = param->val;
   else		val = "";
   TextFieldSetString(octTypeTF, val);

   param = file->data->Param("padding");
   if ( param ) val = param->val;
   else		val = "";
   TextFieldSetString(octPadTF, val);

//
// Look for other content-type parameters
//
   tmpStr.Clear();
   param = file->data->conParams;
   while ( param ) {
      if ( !param->val.Equals("charset",  IGNORE_CASE) &&
	   !param->val.Equals("filename", IGNORE_CASE) &&
	   !param->val.Equals("type",     IGNORE_CASE) &&
	   !param->val.Equals("padding",  IGNORE_CASE) ) {
	 if ( tmpStr.size() > 0 ) tmpStr += "; ";
         tmpStr += param->full;
      }
      param = param->next;
   }
   TextSetString(otherText, tmpStr);

   param = file->data->Param("site");
   if ( param ) val = param->val;
   else		val = "";
   TextFieldSetString(hostTF, val);

   param = file->data->Param("directory");
   if ( param ) val = param->val;
   else		val = "";
   TextFieldSetString(dirTF, val);

   param = file->data->Param("mode");
   if ( param ) {

      val = param->val;
      tb = NULL;

      if ( file->data->IsTFTP() ) {
	 if      (val.Equals("netascii", IGNORE_CASE)) tb = tftpModeNetAsciiTB;
	 else if (val.Equals("octet"   , IGNORE_CASE)) tb = tftpModeOctetTB;
	 else if (val.Equals("mail"    , IGNORE_CASE)) tb = tftpModeMailTB;
      }
      else {
	 if      ( val.Equals    ("ascii",  IGNORE_CASE) ) tb = ftpModeAsciiTB;
	 else if ( val.Equals    ("ebcdic", IGNORE_CASE) ) tb = ftpModeEbcdicTB;
	 else if ( val.Equals    ("image",  IGNORE_CASE) ) tb = ftpModeImageTB;
//       else if ( val.StartsWith("local",  IGNORE_CASE) ) tb = ftpModeLocalTB;
      }

      if ( tb ) XmToggleButtonSetState(tb, True, True);
   }

   param = file->data->Param("server");
   if ( param ) val = param->val;
   else		val = "";
   TextFieldSetString(serverTF, val);

   param = file->data->Param("subject");
   if ( param ) val = param->val;
   else		val = "";
   TextFieldSetString(subjectTF, val);

   tmpStr.Clear();
   if ( file->data->IsMail() ) file->data->GetText(tmpStr);
   TextSetString(bodyText, tmpStr);

   param = file->data->Param("expiration");
   if ( param ) val = param->val;
   else		val = "";
   TextFieldSetString(expTF, val);

   param = file->data->Param("size");
   if ( param ) val = param->val;
   else		val = "";
   TextFieldSetString(sizeTF, val);

   param = file->data->Param("permission");
   Boolean readWrite = (param && param->val.Equals("read-write", IGNORE_CASE));
   XmToggleButtonSetState(readWrite ? permRwTB : permRoTB, True, True);

   UpdateVisibleFields();

} // End Show

/*---------------------------------------------------------------
 *  Method to get data for a list of files
 */

void
IncludeWinC::Show(StringListC *list)
{
   fileList = *list;
   typeStr.Clear();	// Force a type change

   if ( list->size() > 1 ) XtManageChild(skipPB);
   else			   XtUnmanageChild(skipPB);

   HalDialogC::Show();

   SetFile(0);

} // End Show

/*---------------------------------------------------------------
 *  Method to get data for a specific file
 */

void
IncludeWinC::SetFile(int fileNum)
{
   fileIndex = fileNum;

//
// Display the file name
//
   StringC	*name = fileList[fileIndex];
   while ( IsDir(*name) ) {

      StringC	errMsg = *name;
      errMsg += " is a directory";
      PopupMessage(errMsg);

      if ( fileIndex+1 < fileList.size() ) {
	 fileIndex++;
	 name = fileList[fileIndex];
      }

      else {
	 Hide();
	 return;
      }
   }

   BusyCursor(True);

   XmTextFieldSetString(nameTF, *name);
   CharC	baseName = BaseName(*name);
   TextSetString(outNameTF, baseName);
   XmTextFieldSetString(descTF, "");

//
// Get the file type
//
   StringC	newTypeStr;
   ::GetFileType(*name, newTypeStr);
   Boolean	typeChange = (newTypeStr != typeStr);

//
// Initialize buttons if the type changed
//
   if ( typeChange ) {

      typeStr = newTypeStr;

      if ( !typeList.includes(typeStr) ) {
	 typeList.add(typeStr);
	 BuildTypeMenu();
      }
      typeIndex = typeList.indexOf(typeStr);

//
// Display "type" push button
//
      XtVaSetValues(typeOM, XmNmenuHistory, *typePbList[typeIndex], NULL);

      if ( IsText() || ContentType() == CT_RFC822 ) {
	 XmToggleButtonSetState(includeAsTextTB, True, True);
	 XmToggleButtonSetState(encodeNoneTB,    True, True);
      }

      else {
	 XmToggleButtonSetState(includeAsFileTB, True, True);
	 XmToggleButtonSetState(encode64TB,      True, True);
      }

      UpdateVisibleFields();

   } // End if file type changed

//
// Display size if available
//
   struct stat	stats;
   if ( stat(*name, &stats) == 0 ) {
      StringC	sizeStr;
      sizeStr += (int)stats.st_size;
      XmTextFieldSetString(sizeTF, sizeStr);
   }

   BusyCursor(False);

} // End SetFile

/*---------------------------------------------------------------
 *  Method to display the fields appropriate to the current file type
 *     and include type
 */

void
IncludeWinC::UpdateVisibleFields()
{
   if ( IncludeAsText() ) {
      XtUnmanageChild(*paramRC);
   }

   else {

//
// Allow pane size to change
//
      XtVaSetValues(*paramRC, XmNpaneMinimum, (Dimension)1,
			      XmNpaneMaximum, (Dimension)10000, NULL);

//
// Determine which parameters are visible
//
      paramRC->Defer(True);
      paramRC->SetRowVisible(charsetRow, IsText() && IncludeAsFile());
      paramRC->SetRowVisible(octTypeRow, IsAppOctet());
      paramRC->SetRowVisible(octPadRow,  IsAppOctet());
      paramRC->SetRowVisible(hostRow,
      			     AttachFtp() || AttachAnon() || AttachTftp());
      paramRC->SetRowVisible(dirRow,
      			     AttachFtp() || AttachAnon() || AttachTftp());
      paramRC->SetRowVisible(ftpModeRow,  AttachFtp() || AttachAnon());
      paramRC->SetRowVisible(tftpModeRow, AttachTftp());
      paramRC->SetRowVisible(serverRow,   AttachMail());
      paramRC->SetRowVisible(subjectRow,  AttachMail());
      paramRC->SetRowVisible(bodyRow,     AttachMail());
      paramRC->Defer(False);

      XtManageChild(*paramRC);

   } // End if not including as text

//
// Encoding can only be pre-existing for attachments
//
   if ( !IncludeAsText() && !IncludeAsFile() ) {
      XmToggleButtonSetState(preEncodeTB, True, False);
      XmToggleButtonSetState(encodeNoneTB, True, True);
   }
   else
      XmToggleButtonSetState(preEncodeTB, False, False);

   if ( IsText() ) {
      XmToggleButtonSetState(ftpModeAsciiTB, True, True);
      XmToggleButtonSetState(tftpModeNetAsciiTB, True, True);
   }
   else {
      XmToggleButtonSetState(ftpModeImageTB, True, True);
      XmToggleButtonSetState(tftpModeOctetTB, True, True);
   }

} // End UpdateVisibleFields

/*---------------------------------------------------------------
 *  Callback to handle press of ok button
 */

void
IncludeWinC::DoOk(Widget, IncludeWinC *This, XtPointer)
{
//
// Check the file name field
//
   char	*cs = XmTextFieldGetString(This->nameTF);
   if ( strlen(cs) == 0 ) {
      set_invalid(This->nameTF, True, True);
      This->PopupMessage("Please enter a file name.");
      XtFree(cs);
      return;
   }

//
// Check the file accessibility
//
   if ( This->IncludeAsText() || This->IncludeAsFile() ) {

      if ( access(cs, F_OK) != 0 ) {
	 set_invalid(This->nameTF, True, True);
	 This->PopupMessage("The specified file does not exist.");
	 XtFree(cs);
	 return;
      }
      else if ( access(cs, R_OK) != 0 ) {
	 set_invalid(This->nameTF, True, True);
	 This->PopupMessage("The specified file is not readable.");
	 XtFree(cs);
	 return;
      }

//
// Check the encoding fields.  Check for 8-bit data in the file if no
//    encoding has been selected.
//
      if ( This->NoEncoding() ) {

	 MappedFileC	*mf = MapFile(cs);
	 if ( mf ) {

	    Boolean	has8 = Contains8Bit(mf->data);
	    UnmapFile(mf);

	    if ( has8 ) {
	       This->PopupMessage
	         ("This file contains 8-bit data.\nPlease select an encoding.");
	       XtFree(cs);
	       return;
	    }
	 }
      }

   } // End if we need access to the file

   XtFree(cs);

//
// Check the ftp host field
//
   if ( This->AttachFtp() || This->AttachAnon() || This->AttachTftp() ) {

      cs = XmTextFieldGetString(This->hostTF);

      if ( strlen(cs) == 0 ) {
	 set_invalid(This->hostTF, True, True);
	 This->PopupMessage("Please enter a host name or address.");
	 XtFree(cs);
	 return;
      }

      XtFree(cs);
   }

//
// Check the mail fields
//
   if ( This->AttachMail() ) {

      cs = XmTextFieldGetString(This->serverTF);

      if ( strlen(cs) == 0 ) {
	 set_invalid(This->serverTF, True, True);
	 This->PopupMessage("Please enter an address for the mail server.");
	 XtFree(cs);
	 return;
      }

      XtFree(cs);
   }

   This->BusyCursor(True);

   This->CallOkCallbacks();

//
// See if there is another file to be displayed
//
   if ( This->fileIndex+1 < This->fileList.size() )
      This->SetFile(This->fileIndex+1);
   else
      This->Hide();

   This->BusyCursor(False);

} // End DoOk

/*---------------------------------------------------------------
 *  Callback to handle press of skip button
 */

void
IncludeWinC::DoSkip(Widget, IncludeWinC *This, XtPointer)
{
   This->BusyCursor(True);

//
// See if there is another file to be displayed
//
   if ( This->fileIndex+1 < This->fileList.size() )
      This->SetFile(This->fileIndex+1);
   else
      This->Hide();

   This->BusyCursor(False);
}

/*---------------------------------------------------------------
 *  Callback to handle press of charset button
 */

void
IncludeWinC::DoCharset(Widget w, IncludeWinC *This, XtPointer)
{
   char	*cs = "us-ascii";

        if ( w == This->charIso1PB ) cs = "iso-8859-1";
   else if ( w == This->charIso2PB ) cs = "iso-8859-2";
   else if ( w == This->charIso3PB ) cs = "iso-8859-3";
   else if ( w == This->charIso4PB ) cs = "iso-8859-4";
   else if ( w == This->charIso5PB ) cs = "iso-8859-5";
   else if ( w == This->charIso6PB ) cs = "iso-8859-6";
   else if ( w == This->charIso7PB ) cs = "iso-8859-7";
   else if ( w == This->charIso8PB ) cs = "iso-8859-8";
   else if ( w == This->charIso9PB ) cs = "iso-8859-9";
   else if ( w == This->charIso13PB ) cs = "iso-8859-13";

   XmTextFieldSetString(This->charTF, cs);

} // End DoCharset

/*-----------------------------------------------------------------------
 *  Handle change of file type
 */

void
IncludeWinC::FileTypeChanged(Widget w, IncludeWinC *This, XtPointer)
{
   XtUnmanageChild(This->typePD);

   int	newIndex = This->typePbList.indexOf(w);
   if ( newIndex < 0 || newIndex == This->typeIndex ) return;

   This->typeIndex = newIndex;
   This->typeStr   = *This->typeList[newIndex];

   This->UpdateVisibleFields();
}

/*-----------------------------------------------------------------------
 *  Handle change of include type
 */

void
IncludeWinC::IncludeTypeChanged(Widget, IncludeWinC *This, XtPointer)
{
   This->UpdateVisibleFields();
}

/*---------------------------------------------------------------
 *  Query methods
 */

MimeAccessType
IncludeWinC::AccessType() const
{
   if ( AttachLocal() ) return AT_LOCAL_FILE;
   if ( AttachAnon()  ) return AT_ANON_FTP;
   if ( AttachFtp()   ) return AT_FTP;
   if ( AttachTftp()  ) return AT_TFTP;
   if ( AttachMail()  ) return AT_MAIL_SERVER;

   return AT_INLINE;
}

Boolean
IncludeWinC::AlreadyEncoded() const
{
   return XmToggleButtonGetState(preEncodeTB);
}

Boolean
IncludeWinC::AttachAnon() const
{
   return XmToggleButtonGetState(attachAnonTB);
}

Boolean
IncludeWinC::AttachFtp() const
{
   return XmToggleButtonGetState(attachFtpTB);
}

Boolean
IncludeWinC::AttachLocal() const
{
   return XmToggleButtonGetState(attachLocalTB);
}

Boolean
IncludeWinC::AttachMail() const
{
   return XmToggleButtonGetState(attachMailTB);
}

Boolean
IncludeWinC::AttachTftp() const
{
   return XmToggleButtonGetState(attachTftpTB);
}

MimeContentType
IncludeWinC::ContentType() const
{
   return ::ContentType(typeStr);
}

MimeEncodingType
IncludeWinC::EncodingType() const
{
   if ( NoEncoding() ) return ET_NONE;
   if ( Is8Bit()     ) return ET_8BIT;
   if ( IsQP()       ) return ET_QP;
   if ( IsBase64()   ) return ET_BASE_64;
   if ( IsUUencode() ) return ET_UUENCODE;
   if ( IsBinHex()   ) return ET_BINHEX;

   return ET_UNKNOWN;

} // End EncodingType

MimeGroupType
IncludeWinC::GroupType() const
{
   return ::GroupType(typeStr);
}

Boolean
IncludeWinC::NoEncoding() const
{
   return XmToggleButtonGetState(encodeNoneTB);
}

Boolean
IncludeWinC::IncludeAsFile() const
{
   return XmToggleButtonGetState(includeAsFileTB);
}

Boolean
IncludeWinC::IncludeAsText() const
{
   return XmToggleButtonGetState(includeAsTextTB);
}

Boolean
IncludeWinC::Is8Bit() const
{
   return XmToggleButtonGetState(encode8bitTB);
}

Boolean
IncludeWinC::IsAppOctet() const
{
   return (strcasecmp(typeStr, "application/octet-stream") == 0);
}

Boolean
IncludeWinC::IsBase64() const
{
   return XmToggleButtonGetState(encode64TB);
}

Boolean
IncludeWinC::IsBinHex() const
{
   return XmToggleButtonGetState(encodeBinHexTB);
}

Boolean
IncludeWinC::IsQP() const
{
   return XmToggleButtonGetState(encodeQpTB);
}

Boolean
IncludeWinC::IsReadWrite() const
{
   return XmToggleButtonGetState(permRwTB);
}

Boolean
IncludeWinC::IsText() const
{
   return (strncasecmp(typeStr, "text/", 5) == 0);
}

Boolean
IncludeWinC::IsTextEnriched() const
{
   return (strcasecmp(typeStr, "text/enriched") == 0);
}

Boolean
IncludeWinC::IsTextPlain() const
{
   return (strcasecmp(typeStr, "text/plain") == 0);
}

Boolean
IncludeWinC::IsTextRichtext() const
{
   return (strcasecmp(typeStr, "text/richtext") == 0);
}

Boolean
IncludeWinC::IsUUencode() const
{
   return XmToggleButtonGetState(encodeUuTB);
}

void
IncludeWinC::GetAppType(StringC& type) const
{
   if ( IsAppOctet() && !IncludeAsText() ) {
      char	*cs = XmTextFieldGetString(octTypeTF);
      type = cs;
      XtFree(cs);
   }
   else
      type.Clear();
}

void
IncludeWinC::GetAppPadding(StringC& pad) const
{
   if ( IsAppOctet() && !IncludeAsText() ) {
      char	*cs = XmTextFieldGetString(octPadTF);
      pad = cs;
      XtFree(cs);
   }
   else
      pad.Clear();
}

void
IncludeWinC::GetCharset(StringC& charset) const
{
   if ( IsText() && !IncludeAsText() ) {
      char	*cs = XmTextFieldGetString(charTF);
      charset = cs;
      XtFree(cs);
   }
   else
      charset.Clear();
}

void
IncludeWinC::GetDescription(StringC& desc) const
{
   char	*cs = XmTextFieldGetString(descTF);
   desc = cs;
   XtFree(cs);
}

void
IncludeWinC::GetExpiration(StringC& exp) const
{
   if ( !IncludeAsText() && !IncludeAsFile() ) {
      char	*cs = XmTextFieldGetString(expTF);
      exp = cs;
      XtFree(cs);
   }
   else
      exp.Clear();
}

void
IncludeWinC::GetFileName(StringC& name) const
{
   char	*cs = XmTextFieldGetString(nameTF);
   name = cs;
   XtFree(cs);
}

void
IncludeWinC::GetFileType(StringC& type) const
{
   type = typeStr;
}

void
IncludeWinC::GetFtpDir(StringC& dir) const
{
   if ( AttachFtp() || AttachAnon() || AttachTftp() ) {
      char	*cs = XmTextFieldGetString(dirTF);
      dir = cs;
      XtFree(cs);
   }
   else
      dir.Clear();
}

void
IncludeWinC::GetFtpHost(StringC& host) const
{
   if ( AttachFtp() || AttachAnon() || AttachTftp() ) {
      char	*cs = XmTextFieldGetString(hostTF);
      host = cs;
      XtFree(cs);
   }
   else
      host.Clear();
}

void
IncludeWinC::GetFtpMode(StringC& mode) const
{
   if ( AttachFtp() || AttachAnon() ) {
      if      ( XmToggleButtonGetState(ftpModeAsciiTB)  ) mode = "ascii";
      else if ( XmToggleButtonGetState(ftpModeEbcdicTB) ) mode = "ebcdic";
      else if ( XmToggleButtonGetState(ftpModeImageTB)  ) mode = "image";
//      else if ( XmToggleButtonGetState(ftpModeLocalTB)  ) mode = "local 8";
   }
   else
      mode = "ascii";
}

void
IncludeWinC::GetMailAddr(StringC& addr) const
{
   if ( AttachMail() ) {
      char	*cs = XmTextFieldGetString(serverTF);
      addr = cs;
      XtFree(cs);
   }
   else
      addr.Clear();
}

void
IncludeWinC::GetMailBody(StringC& body) const
{
   if ( AttachMail() ) {
      char	*cs = XmTextGetString(bodyText);
      body = cs;
      XtFree(cs);
   }
   else
      body.Clear();
}

void
IncludeWinC::GetMailSubject(StringC& subject) const
{
   if ( AttachMail() ) {
      char	*cs = XmTextFieldGetString(subjectTF);
      subject = cs;
      XtFree(cs);
   }
   else
      subject.Clear();
}

void
IncludeWinC::GetOtherParams(StringC& params) const
{
   if ( !IncludeAsText() ) {
      char	*cs = XmTextGetString(otherText);
      params = cs;
      XtFree(cs);

//
// Replace any newlines
//
      int	pos;
      cs = params;
      while ( (pos=params.PosOf(";\n")) > 0 ) cs[pos+1] = ' ';
      while ( (pos=params.PosOf('\n'))  > 0 ) params(pos,1) = "; ";
   }
   else
      params.Clear();
}

void
IncludeWinC::GetOutputName(StringC& name) const
{
   if ( !IncludeAsText() ) {
      char	*cs = XmTextFieldGetString(outNameTF);
      name = cs;
      XtFree(cs);
   }
   else
      name.Clear();
}

void
IncludeWinC::GetSize(StringC& size) const
{
   if ( !IncludeAsText() && !IncludeAsFile() ) {
      char	*cs = XmTextFieldGetString(sizeTF);
      size = cs;
      XtFree(cs);
   }
   else
      size.Clear();
}

void
IncludeWinC::GetTftpMode(StringC& mode) const
{
   if ( AttachTftp() ) {
      if      ( XmToggleButtonGetState(tftpModeNetAsciiTB) ) mode = "netascii";
      else if ( XmToggleButtonGetState(tftpModeOctetTB)    ) mode = "octet";
      else if ( XmToggleButtonGetState(tftpModeMailTB)     ) mode = "mail";
   }
   else
      mode = "netascii";
}

