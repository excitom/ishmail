/*
 * $Id: IncludeWinC.h,v 1.2 2000/06/06 12:46:10 evgeny Exp $
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

#ifndef _IncludeWinC_h_
#define _IncludeWinC_h_

#include "MimeTypes.h"

#include <hgl/HalDialogC.h>
#include <hgl/CallbackC.h>
#include <hgl/StringListC.h>
#include <hgl/WidgetListC.h>

#include <time.h>

class RowColC;
class SendIconC;

class IncludeWinC : public HalDialogC {

public:

protected:

//
// Widgets
//
   RowColC		*fileRC;
   Widget		   nameTF;
   Widget		   descTF;
   Widget		   typePD;
   Widget		      typeAppCB;
   Widget		      typeAppPD;
   Widget		      typeAudioCB;
   Widget		      typeAudioPD;
   Widget		      typeImageCB;
   Widget		      typeImagePD;
   Widget		      typeMsgCB;
   Widget		      typeMsgPD;
   Widget		      typeMultCB;
   Widget		      typeMultPD;
   Widget		      typeTextCB;
   Widget		      typeTextPD;
   Widget		      typeVideoCB;
   Widget		      typeVideoPD;
   Widget		   typeOM;
   Widget		includeForm;
   Widget		   includeAsTextTB;
   Widget		   includeAsFileTB;
   Widget		   attachLocalTB;
   Widget		   attachAnonTB;
   Widget		   attachFtpTB;
   Widget		   attachTftpTB;
   Widget		   attachMailTB;
   Widget		encodeForm;
   Widget		   encodeNoneTB;
   Widget		   encode8bitTB;
   Widget		   encodeQpTB;
   Widget		   encode64TB;
   Widget		   encodeUuTB;
   Widget		   encodeBinHexTB;
   Widget		   preEncodeTB;
   RowColC		*paramRC;
   Widget		   charTF;
   Widget		   charAsciiPB;
   Widget		   charIso1PB;
   Widget		   charIso2PB;
   Widget		   charIso3PB;
   Widget		   charIso4PB;
   Widget		   charIso5PB;
   Widget		   charIso6PB;
   Widget		   charIso7PB;
   Widget		   charIso8PB;
   Widget		   charIso9PB;
   Widget		   charIso13PB;
   Widget		   outNameTF;
   Widget		   octTypeTF;
   Widget		   octPadTF;
   Widget		   otherText;
   Widget		   hostTF;
   Widget		   dirTF;
   Widget		   ftpModeAsciiTB;
   Widget		   ftpModeEbcdicTB;
   Widget		   ftpModeImageTB;
   Widget		   ftpModeLocalTB;
   Widget		   tftpModeNetAsciiTB;
   Widget		   tftpModeOctetTB;
   Widget		   tftpModeMailTB;
   Widget		   serverTF;
   Widget		   subjectTF;
   Widget		   bodyText;
   Widget		   expTF;
   Widget		   sizeTF;
   Widget		   permRoTB;
   Widget		   permRwTB;
   Widget		skipPB;

//
// Data
//
   StringListC		typeList;
   time_t		typeListTime;
   WidgetListC		typePbList;
   StringListC		fileList;	// List of files to be displayed
   int			fileIndex;	// Current file being displayed
   StringC		typeStr;	// Current file type
   int			typeIndex;	// Index into type list

   int			charsetRow;	// Row numbers in paramRC
   int			outNameRow;
   int			octTypeRow;
   int			octPadRow;
   int			otherRow;
   int			hostRow;
   int			dirRow;
   int			ftpModeRow;
   int			tftpModeRow;
   int			serverRow;
   int			subjectRow;
   int			bodyRow;
   int			expRow;
   int			sizeRow;
   int			permRow;

//
// Callbacks
//
   CallbackListC	okCalls;

   static void		DoCharset         (Widget, IncludeWinC*, XtPointer);
   static void		DoFileExpose      (Widget, IncludeWinC*, XtPointer);
   static void		DoOk              (Widget, IncludeWinC*, XtPointer);
   static void		DoParamExpose     (Widget, IncludeWinC*, XtPointer);
   static void		DoPopup           (Widget, IncludeWinC*, XtPointer);
   static void		DoSkip            (Widget, IncludeWinC*, XtPointer);
   static void		FileTypeChanged   (Widget, IncludeWinC*, XtPointer);
   static void		IncludeTypeChanged(Widget, IncludeWinC*, XtPointer);

   static void		DoEncodeResize(Widget, IncludeWinC*, XEvent*, Boolean*);
   static void		DoIncludeResize(Widget, IncludeWinC*, XEvent*,Boolean*);

//
// Private methods
//
   void			BuildTypeList();
   void			BuildTypeMenu();
   void			SetFile(int);
   void			UpdateVisibleFields();

public:

// Methods

   IncludeWinC(Widget, const char*);
   ~IncludeWinC();

//
// Handle callbacks
//
   inline void	AddOkCallback(CallbackFn *fn, void *data)
      { AddCallback(okCalls, fn, data); }
   inline void	RemoveOkCallback(CallbackFn *fn, void *data)
      { RemoveCallback(okCalls, fn, data); }
   inline void	CallOkCallbacks()
      { CallCallbacks(okCalls, this); }

//
// Get values
//
   MimeAccessType	AccessType()		const;
   Boolean		AlreadyEncoded()	const;
   Boolean		AttachAnon()		const;
   Boolean		AttachFtp()		const;
   Boolean		AttachLocal()		const;
   Boolean		AttachMail()		const;
   Boolean		AttachTftp()		const;
   MimeContentType	ContentType()		const;
   StringC&		ContentTypeStr() { return typeStr; }
   MimeEncodingType	EncodingType()		const;
   MimeGroupType	GroupType()		const;
   Boolean		NoEncoding()		const;
   Boolean		IncludeAsFile()		const;
   Boolean		IncludeAsText()		const;
   Boolean		Is8Bit()		const;
   Boolean		IsAppOctet()		const;
   Boolean		IsBase64()		const;
   Boolean		IsBinHex()		const;
   Boolean		IsQP()			const;
   Boolean		IsReadWrite()		const;
   Boolean		IsText()		const;
   Boolean		IsTextEnriched()	const;
   Boolean		IsTextPlain()		const;
   Boolean		IsTextRichtext()	const;
   Boolean		IsUUencode()		const;

   void			GetAppType(StringC&)		const;
   void			GetAppPadding(StringC&)		const;
   void			GetCharset(StringC&)		const;
   void			GetDescription(StringC&)	const;
   void			GetExpiration(StringC&)		const;
   void			GetFileName(StringC&)		const;
   void			GetFileType(StringC&)		const;
   void			GetFtpDir(StringC&)		const;
   void			GetFtpHost(StringC&)		const;
   void			GetFtpMode(StringC&)		const;
   void			GetMailAddr(StringC&)		const;
   void			GetMailBody(StringC&)		const;
   void			GetMailSubject(StringC&)	const;
   void			GetOtherParams(StringC&)	const;
   void			GetOutputName(StringC&)		const;
   void			GetSize(StringC&)		const;
   void			GetTftpMode(StringC&)		const;

//
// Set values
//
   void		Show(StringListC*);
   void		Show(SendIconC*);
   inline void	Show() { HalDialogC::Show(); }
};

#endif // _IncludeWinC_h_
