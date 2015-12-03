/*
 *  $Id: IshAppC.h,v 1.6 2001/07/28 18:26:03 evgeny Exp $
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
#ifndef _IshAppC_h
#define _IshAppC_h

#include <hgl/HalAppC.h>
#include <hgl/PtrListC.h>

class IshAppP;
class AppPrefC;
class CompPrefC;
class MailPrefC;
class ReplyPrefC;
class SigPrefC;
class ReadPrefC;
class HeadPrefC;
class ConfPrefC;
class AliasPrefC;
class AlertPrefC;
class IconPrefC;
class SavePrefC;
class AutoFilePrefC;
class MainButtPrefC;
class ReadButtPrefC;
class SendButtPrefC;
class SumPrefC;
class FolderPrefC;
class FontPrefC;
class SortPrefC;
class MainWinC;
class FolderC;
class UndelWinC;
class SaveMgrC;
class VItemListC;
class MsgC;
class ReadWinC;
class PixmapC;
class SendWinC;

class IshAppC : public HalAppC {

   friend class IshAppP;
   friend class PrefC;

   IshAppP	*priv;	// Private data and methods

public:

//
// User preferences
//
   AppPrefC		*appPrefs;
   CompPrefC		*compPrefs;
   MailPrefC		*mailPrefs;
   ReplyPrefC		*replyPrefs;
   SigPrefC		*sigPrefs;
   ReadPrefC		*readPrefs;
   HeadPrefC		*headPrefs;	// Header preferences
   ConfPrefC		*confPrefs;	// Confirmation preferences
   AliasPrefC		*aliasPrefs;
   AlertPrefC		*alertPrefs;
   IconPrefC		*iconPrefs;
   SavePrefC		*savePrefs;
   AutoFilePrefC	*autoFilePrefs;
   MainButtPrefC	*mainButtPrefs;
   ReadButtPrefC	*readButtPrefs;
   SendButtPrefC	*sendButtPrefs;
   SumPrefC		*sumPrefs;
   FolderPrefC		*folderPrefs;
   FontPrefC		*fontPrefs;
   SortPrefC		*sortPrefs;

//
// User variables
//
   StringC		userId;
   StringC		userName;
   StringC		host;
   StringC		domain;
   StringC		userAtHost;
   StringC		userAtDomain;
   StringC		userAtHostDomain;
   StringC		home;
   StringC		idxDir;
   StringC		startupDir;
   StringC		resFile;

//
// Application variables
//
   XrmDatabase		resdb;
   Atom			msgAtom;
   Boolean		sleeping;
   Boolean		exiting;
   PixmapC		*ishmailPM;

//
// Folders
//
   FolderC		*systemFolder;

//
// Windows
//
   MainWinC		*mainWin;
   PtrListC		sendWinList;
   PtrListC		readWinList;
   UndelWinC		*undelWin;

//
// Support classes
//
   SaveMgrC		*saveMgr;

//
// Global methods
//
    IshAppC(int*, char**, const char*, const char*);
   ~IshAppC();

   void		AbbreviateFolderName(StringC&);
   void		Broadcast(const char*);
   void		CheckForNewMail();
   void		Compose();
   void		DisplayMessage(MsgC*, ReadWinC* =NULL);
   void		ExpandFolderName(StringC&, const char *folderDir=NULL);
   SendWinC	*GetSendWin();
   void		VerifyMailCheck(Boolean restart=False);
};

//
// Our home
//
extern  StringC ishHome;


extern IshAppC	*ishApp;
extern int	debuglev;
extern "C" char	*version;
extern "C" char	*versionNumber;
extern "C" char	*versionIshHome;

#define AddActivate(W,C,D) \
  XtAddCallback(W, XmNactivateCallback,     (XtCallbackProc)(C), (XtPointer)(D))
#define AddCascading(W,C,D) \
  XtAddCallback(W, XmNcascadingCallback,    (XtCallbackProc)(C), (XtPointer)(D))
#define AddValueChanged(W,C,D) \
  XtAddCallback(W, XmNvalueChangedCallback, (XtCallbackProc)(C), (XtPointer)(D))

#endif // _IshAppC_h
