/*
 *  $Id: IshAppC.C,v 1.15 2001/07/28 18:26:03 evgeny Exp $
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
#include "IshAppC.h"
#include "IshAppP.h"
#include "AppPrefC.h"
#include "CompPrefC.h"
#include "MailPrefC.h"
#include "ReplyPrefC.h"
#include "SigPrefC.h"
#include "ReadPrefC.h"
#include "HeadPrefC.h"
#include "ConfPrefC.h"
#include "AliasPrefC.h"
#include "AlertPrefC.h"
#include "IconPrefC.h"
#include "SavePrefC.h"
#include "AutoFilePrefC.h"
#include "MainButtPrefC.h"
#include "ReadButtPrefC.h"
#include "SendButtPrefC.h"
#include "SumPrefC.h"
#include "FolderPrefC.h"
#include "FontPrefC.h"
#include "SortPrefC.h"
#include "ShellExp.h"
#include "FileMisc.h"
#include "MailFile.h"
#include "Mailcap.h"
#include "MainWinC.h"
#include "UnixFolderC.h"
#include "ImapFolderC.h"
#include "ImapServerC.h"
#include "SaveMgrC.h"
#include "UndelWinC.h"
#include "ImapMisc.h"
#include "ReadWinC.h"
#include "MsgC.h"
#include "MsgItemC.h"
#include "SendWinC.h"

#include <hgl/rsrc.h>
#include <hgl/PixmapC.h>
#include <hgl/SysErr.h>
#include <hgl/VBoxC.h>
#include <hgl/FieldViewC.h>

#include <Xm/AtomMgr.h>

#include <pwd.h>	// For passwd
#include <unistd.h>	// For getenv, getlogin, getuid, access
#include <fcntl.h>	// open
#include <errno.h>
#include <dirent.h>

#include <netdb.h>
#ifndef MAXHOSTNAMELEN
#	define MAXHOSTNAMELEN (256)
#endif

IshAppC	*ishApp;

/*----------------------------------------------------------------------
 * Constructor
 */

IshAppC::IshAppC(int *argc, char **argv, const char *name, const char *clss)
: HalAppC(argc, argv, name, clss)
{
   ishApp	= this;
   priv		= new IshAppP(this);
   undelWin	= NULL;
   ishmailPM	= NULL;
   systemFolder	= NULL;

   sendWinList.SetSorted(FALSE);
   readWinList.SetSorted(FALSE);

   sleeping = FALSE;
   exiting = FALSE;

//
// Get user login id.  Try LOGNAME, USER, getuid and getlogname in that order.
//
   struct passwd	*pw = NULL;
   char *cs = getenv("LOGNAME");
   if ( !cs ) cs = getenv("USER");
   if ( !cs ) {
      pw = getpwuid(getuid());
      if ( pw ) {
	 userId = pw->pw_name;
      }
      else {
	 cs = getlogin();
	 if ( cs ) userId = cs;
	 else	   userId = "Unknown";
      }
   }
   else {
      userId = cs;
   }

   if ( debuglev > 0 ) cout <<"User id is: " <<userId <<endl;

//
// Get user name
//
   cs = getenv("NAME");
   if ( cs ) userName = cs;
   else {
      if ( !pw ) pw = getpwuid(getuid());
      if ( pw ) {

	// In some systems, convention is to load the gecos field
	// with extra info after the name, separated by a comma.
	// Truncate the extra info, with possible loss of stuff
	// like the ", Jr." in "Ben Dover, Jr.".
	// Setting $NAME can always override this...

	char *comma = strchr(pw->pw_gecos, ',');
	if (comma != NULL) *comma = '\0';
	userName = pw->pw_gecos;
      }
      else	userName = userId;
   }

   if ( debuglev > 0 ) cout <<"User name is: " <<userName <<endl;

//
// Get host and domain info
//
   char	tmp[MAXHOSTNAMELEN];
   if ( gethostname(tmp, sizeof(tmp)) == 0 ) {
      tmp[sizeof(tmp) - 1] = 0;		// Be sure it's 0-terminated
      struct hostent	*he = gethostbyname(tmp);
      if ( he ) host = he->h_name;
      else	host = tmp;
   }

   int	pos = host.PosOf('.');
   if ( pos > 0 ) {
      domain = host(pos+1, host.size());
      host.Clear(pos);
   }
#ifdef HAVE_GETDOMAINNAME_DECL
   else if ( getdomainname(tmp, sizeof(tmp)) == 0 ) {
      domain = tmp;
   }
#endif

//
// If we don't have a domain name, try /etc/resolv.conf
//
   if ( domain.size() == 0 || domain.Equals("(none)", IGNORE_CASE) ) {

      if ( debuglev > 0 )
	 cout <<"Trying /etc/resolv.conf for domain name" <<endl;

      StringC	resolvStr;
      resolvStr.ReadFile("/etc/resolv.conf");

      int	pos;
      if ( resolvStr.StartsWith("domain", IGNORE_CASE) &&
	   isspace(resolvStr[6]) )
	 pos = 0;
      else
	 pos = resolvStr.PosOf("\ndomain", IGNORE_CASE);
	 while ( pos > 0 && !isspace(resolvStr[pos+6]) )
	    pos = resolvStr.PosOf("\ndomain", (u_int)pos+6, IGNORE_CASE);

      if ( pos >= 0 )
	 domain = resolvStr.NextWord(pos+6);

   } // End if no domain found

   if ( debuglev > 0 ) {
      cout <<"Host   name is: " <<host <<endl;
      cout <<"Domain name is: " <<domain <<endl;
   }

   userAtHost = userId;
   userAtHost += '@';
   userAtHost += host;

   userAtDomain = userId;
   userAtDomain += '@';
   userAtDomain += domain;

   userAtHostDomain = userAtHost;
   userAtHostDomain += '.';
   userAtHostDomain += domain;

//
// Get home directory
//
   cs = getenv("HOME");
   if ( cs && strlen(cs)>0 ) home = cs;
   else                      home = ".";
   if ( debuglev > 0 ) cout <<"Home directory is: " <<home <<endl;

//
// Check if there is a specific directory for index files
// If not, the default is to use the same directory as the associated
// folder. This sometimes is a problem for the InBox, as users don't
// have privilege to create files in the system mail folder directory (e.g.
// /var/spool/mail).
//
   cs = getenv("ISHIDX");
   if ( cs && strlen(cs)>0 ) idxDir = cs;
   else                      idxDir = "";
   if ( debuglev > 0 ) cout <<"Index file directory is: " <<idxDir <<endl;

//
// Get name of user preferences file
//
   cs = getenv("ISHMAILRC");
   if ( cs && strlen(cs) > 0 ) resFile = cs;
   else {
      resFile = home;
      resFile += "/";
      resFile += ".ishmailrc";
   }

//
// See if "Ishmail" needs to be added to beginning of resources.  Prior to
//    version 1.1 we thought we didn't need it, but it looks like we really
//    do.
// Also check for duplicate lines.  If there are any, keep the last.
//   
   priv->FixIshmailrc();

//
// Read resources
//
   resdb = XtDatabase(display);
   XrmCombineFileDatabase(resFile, &resdb, /*Override*/True);

#if 0
   unlink("/tmp/ishdb");
   XrmPutFileDatabase(resdb, "/tmp/ishdb");
#endif

   QuickHelp(get_boolean(appShell, "showQuickHelp", True));

//
// Read resources.  Create these two first since ShellExpand and
//    ExpandFolderName refer to them.
//
   appPrefs    = new AppPrefC;
   folderPrefs = new FolderPrefC;

//
// Now we can safely expand the values
//
   appPrefs->ExpandValues();
   folderPrefs->ExpandValues();

//
// Read remaining resources
//
   compPrefs	 = new CompPrefC;
   mailPrefs	 = new MailPrefC;
   replyPrefs	 = new ReplyPrefC;
   sigPrefs	 = new SigPrefC;
   readPrefs	 = new ReadPrefC;
   headPrefs	 = new HeadPrefC;
   confPrefs	 = new ConfPrefC;
   aliasPrefs	 = new AliasPrefC;
   alertPrefs	 = new AlertPrefC;
   iconPrefs	 = new IconPrefC;
   savePrefs	 = new SavePrefC;
   autoFilePrefs = new AutoFilePrefC;
   mainButtPrefs = new MainButtPrefC;
   readButtPrefs = new ReadButtPrefC;
   sendButtPrefs = new SendButtPrefC;
   sumPrefs	 = new SumPrefC;
   fontPrefs	 = new FontPrefC;
   sortPrefs	 = new SortPrefC;

//
// Get current directory
//
   char	sdir[MAXPATHLEN+2];
   if ( getcwd(sdir, MAXPATHLEN+2) ) {
      startupDir = sdir;
   }
   else {
      cs = getenv("PWD");
      if ( cs ) startupDir = cs;
      else {
	 startupDir = ".";
	 startupDir = FullPathname(startupDir);
      }
   }

   if ( startupDir.StartsWith(appPrefs->AutomountRoot()) )
      startupDir.CutBeg(appPrefs->AutomountRoot().size());

//
// Create message dialog pixmaps
//
   priv->CreatePixmaps();
   messagePM  = priv->messagePM->reg;
   questionPM = priv->questionPM->reg;
   warningPM  = priv->warningPM->reg;
   errorPM    = priv->errorPM->reg;

   if ( debuglev > 0 ) cout <<"Reading mailcap files" <<endl;
   if ( InitMailcap() != 0 ) {
      StringC	msg = "Can't initialize mailcap subsystem";
      msg += SystemErrorMessage(errno);
      PopupMessage(msg);
   }

//
// See if this is a first time user
//
   //Boolean	firstTime = False;
   if ( access(resFile, F_OK) != 0 ) {

//
// Create a blank rc file
//
      close(open(resFile, O_WRONLY|O_CREAT|O_TRUNC, 0600));

//
// Send a welcome message to the user
//
      StringC	welcome = ishHome + "/lib/welcome";
      MailFile(welcome, userId, mailPrefs->SendmailCmd(),
	       False/*Don't wait*/, False/*Don't delete*/);

      //firstTime = True;

   } // End if this is a first time user

//
// See if this is the first time the user has run this version
//
   CharC	thisVer = versionNumber;
   StringC	lastVer = get_string(appShell, "versionNumber");
   if ( thisVer > lastVer )
      appPrefs->WriteVersion();

//
// Version 1.2 was the first version where IMAP and POP support was available
//
   if ( lastVer < "1.2" ) {
      priv->QueryImap();
      if ( !appPrefs->usingImap ) priv->QueryPop();
      priv->QueryFolderType();
   }

//
// Create application-specific atoms
//
   msgAtom = XmInternAtom(display, "IshmailMessage", False);

//
// Create support classes
//
   saveMgr = new SaveMgrC;

//
// Create and display main window object.
//
   if ( debuglev > 0 ) cout <<"Creating main window" <<endl;
   mainWin = new MainWinC(appShell);

   mainWin->Show();

//
// Open the in-box
//
   if ( debuglev > 0 ) cout <<"Opening in-box" <<endl;
   if ( IsImapName(appPrefs->inBox) )
      systemFolder = new ImapFolderC(appPrefs->inBox);
   else
      systemFolder = new UnixFolderC(appPrefs->inBox);

//
// Display messages in in-box
//
   mainWin->ShowFolder(systemFolder);
   mainWin->ActivateSystemFolder();

//
// Open initial folders
//
   u_int	count = folderPrefs->InitialFolders().size();
   StringC	fname;
   int i=0; for (i=0; i<count; i++) {
      fname = *folderPrefs->InitialFolders()[i];
      mainWin->AddInitialFolder(fname);
   }

//
// Open command line folders
//
   CharC	arg;
   StringListC	folderList;
   for (i=0; i<argCount; i++) {

      arg = argVec[i];

      if ( !arg.StartsWith('-') ) {
	 if ( debuglev > 0 ) cout <<" Adding folder " <<arg <<endl;
	 folderList.add(arg);
      }

   } // End for each arg

   if ( folderList.size() > 0 ) {

      ExpandList(folderList);	// Expand wildcards

      count = folderList.size();
      for (i=0; i<count; i++) {
	 fname = *folderList[i];
	 mainWin->AddInitialFolder(fname);
      }
   }

   if ( systemFolder->HasNewMessages() )
      mainWin->GetNewMail(systemFolder);
   else
      mainWin->UpdateTitle();

//
// Start looking for mail
//
   if ( debuglev > 0 ) cout <<"starting mail check" NL;
   CheckForNewMail();

//
// Look for composition checkpoint files.  If any exist, they are probably
//    left from a crash.
//
   if ( compPrefs->AutoSaveDir().size() > 0 ) {

//
// Open the auto-save directory
//
      StringC		file;
      DIR		*dirp = opendir(compPrefs->AutoSaveDir());
      if ( dirp ) {

//
// See if there are any entries
//
	 struct dirent	*dp;
	 while ( (dp=readdir(dirp)) != NULL ) {

	    file = dp->d_name;

//
// Add all but dot names
//
	    if ( !file.StartsWith('.') )
	       priv->compFileList.add(file);

	 } // End for each directory entry

	 closedir(dirp);

//
// See if any files were found
//
	 if ( priv->compFileList.size() > 0 )
	    priv->EditSavedCompositions();

      } // End if directory opened

      else if ( compPrefs->autoSave && errno != ENOENT ) {
	 StringC
	    errmsg("I could not read your composition auto-save directory:\n");
	 errmsg += compPrefs->AutoSaveDir();
	 errmsg += '\n';
	 errmsg += SystemErrorMessage(errno);
	 mainWin->PopupMessage(errmsg);
      }

   } // End if there is an auto save dir

   mainWin->FolderVBox().Refresh();

} // End constructor

/*----------------------------------------------------------------------
 * Destructor
 */

IshAppC::~IshAppC()
{
   if ( xRunning ) {
      BusyCursor(True);
   }

   delete systemFolder;

   u_int     count = folderPrefs->OpenFolders().size();
   int i=0; for (i=0; i<count; i++) delete folderPrefs->OpenFolders()[i];

   count = folderPrefs->FileFolders().size();
   for (i=0; i<count; i++) delete folderPrefs->FileFolders()[i];

   CloseImapServerConnections();

   if ( xRunning ) {

      delete mainWin;
      delete undelWin;

      count = readWinList.size();
      for (i=0; i<count; i++) {
	 ReadWinC	*readWin = (ReadWinC*)*readWinList[i];
	 delete readWin;
      }

      count = sendWinList.size();
      for (i=0; i<count; i++) {
	 SendWinC	*sendWin = (SendWinC*)*sendWinList[i];
	 delete sendWin;
      }
   }

   delete appPrefs;
   delete compPrefs;
   delete mailPrefs;
   delete replyPrefs;
   delete sigPrefs;
   delete readPrefs;
   delete headPrefs;
   delete confPrefs;
   delete aliasPrefs;
   delete alertPrefs;
   delete iconPrefs;
   delete savePrefs;
   delete autoFilePrefs;
   delete mainButtPrefs;
   delete readButtPrefs;
   delete sendButtPrefs;
   delete sumPrefs;
   delete folderPrefs;
   delete fontPrefs;
   delete sortPrefs;

   delete saveMgr;
   delete ishmailPM;

   delete priv;

} // End destructor

/*---------------------------------------------------------------
 *  Method to expand folder abbreviations
 */

void
IshAppC::ExpandFolderName(StringC& name, const char *folderDir)
{
   static StringC	*tmp = NULL;

   if ( debuglev > 1 ) {
      if ( !tmp ) tmp = new StringC;
      *tmp = name;
   }

   ShellExpand(name, True/*oneWord*/);

//
// Always place relative names in the base directory.  Make them type
//    "./" to use the current directory.
// If name starts with "{" it's a fully qualified IMAP server name, so
//    don't mess with it.
//
   if ( !name.StartsWith('/') &&
	   !name.StartsWith("./") &&
	   !name.StartsWith("../") &&
	   !name.StartsWith("{") ) {

      if (!IsImapName(name)) {
	 if ( !folderDir ) folderDir = appPrefs->FolderDir();
	 name = folderDir + ("/" + name);
      }

   } // End if name is not a full pathname

//
// Abbreviations seem to work better without this
//
#if 0
//
// Now get the full pathname
//
      name = FullPathname(name);
#endif

   if ( debuglev > 1 ) cout <<*tmp <<" expands to " <<name <<endl;

} // End ExpandFolderName

/*---------------------------------------------------------------
 *  Method to abbreviate folder names
 */

void
IshAppC::AbbreviateFolderName(StringC& name)
{
   static StringC	*tmp = NULL;
   if ( !tmp ) tmp = new StringC;

   if ( debuglev > 1 ) *tmp = name;

//
// Initialize automounter equivalents if necessary
//
   static StringC *afpath = NULL;
   static StringC *aspath = NULL;

   if ( !afpath ) {

      char	result[MAXPATHLEN+2];

      realpath(appPrefs->FolderDir(), result);
      afpath = new StringC(result);
      if ( !afpath->EndsWith('/') ) *afpath += '/';

      realpath(startupDir, result);
      aspath = new StringC(result);
      if ( !aspath->EndsWith('/') ) *aspath += '/';

      if ( debuglev > 1 ) {
	 cout <<"   auto folder  path is: " <<*afpath <<endl;
	 cout <<"   auto startup path is: " <<*aspath <<endl;
      }
   }
   CharC	afdir(*afpath); afdir.CutEnd(1);
   CharC	asdir(*aspath); asdir.CutEnd(1);

//
// If we're only using IMAP folders, see if we can remove the server name.
// If the folder directory contains the server name, skip this step.
//
   StringC&	fdir  = appPrefs->FolderDir();
   StringC	fpath = fdir; fpath += '/';
   StringC	spath = startupDir; spath += '/';
   Boolean	isImap = IsImapName(name);
   if ( isImap && !folderPrefs->UsingLocal() && !fdir.StartsWith('{') ) {
      CharC	server = ImapServerPart(name);
      if ( server == appPrefs->imapServer && name.StartsWith(server, (u_int)1) )
	 name.CutBeg(server.Length() + 2);
   }

//
// Check for folder directory in name
//
   if      ( name.Equals(fdir) || name.Equals(fpath) ) name = "+";
   else if ( name.StartsWith(fpath) ) name.Replace(fpath, "+");
   else if ( isImap ) {
      if      ( name.EndsWith(fdir)  ) name.Replace(fdir,  "+");
      else if ( name.Contains(fpath) ) name.Replace(fpath, "+");
   }

//
// Check for automounter folder directory in name
//
   else if ( name.Equals(afdir) || name.Equals(*afpath) ) name = "+";
   else if ( name.StartsWith(*afpath) ) name.Replace(*afpath, "+");
   else if ( isImap ) {
      if      ( name.EndsWith(afdir)   ) name.Replace(afdir,   "+");
      else if ( name.Contains(*afpath) ) name.Replace(*afpath, "+");
   }

//
// Check for startup directory in name
//
   else if ( name.Equals(startupDir) ) name = ".";
   else if ( name.StartsWith(spath) ) name.Replace(startupDir, ".");
   else if ( isImap ) {
      if      ( name.EndsWith(startupDir) ) name.Replace(startupDir, ".");
      else if ( name.Contains(spath)      ) name.Replace(startupDir, ".");
   }

//
// Check for automounter startup directory in name
//
   else if ( name.Equals(asdir) ) name = ".";
   else if ( name.StartsWith(*aspath) ) name.Replace(asdir, ".");
   else if ( isImap ) {
      if      ( name.EndsWith(asdir)   ) name.Replace(asdir, ".");
      else if ( name.Contains(*aspath) ) name.Replace(asdir, ".");
   }

   if ( debuglev > 1 ) cout <<*tmp <<" abbreviates to " <<name <<endl;

} // End AbbreviateFolderName


/*------------------------------------------------------------------------
 * Method to check for new mail
 */

void
IshAppC::CheckForNewMail()
{
   if ( priv->checkTimer ) XtRemoveTimeOut(priv->checkTimer);
   priv->CheckForNewMail(priv, NULL);
}

/*---------------------------------------------------------------
 * Display a message in all primary windows
 */

void
IshAppC::Broadcast(const char *msg)
{
   if ( !xRunning && strlen(msg) > 0 ) {
      cerr <<msg <<endl;
      return;
   }

   mainWin->Message(msg);

   u_int	count = readWinList.size();
   int i=0; for (i=0; i<count; i++) {
      ReadWinC	*readWin = (ReadWinC*)*readWinList[i];
      if ( readWin->IsShown() ) readWin->Message(msg);
   }

   count = sendWinList.size();
   for (i=0; i<count; i++) {
      SendWinC	*sendWin = (SendWinC*)*sendWinList[i];
      if ( sendWin->IsShown() ) sendWin->Message(msg);
   }

} // End Broadcast

/*------------------------------------------------------------------------
 * Method to make sure we still have mail checking.  For some reason, X
 *    will quit calling the TimeOut proc on occasion.
 */

void
IshAppC::VerifyMailCheck(Boolean restart)
{
   if ( appPrefs->checkInterval <= 0 ) return;

   time_t	t = time(0);
   if ( restart || (t - priv->lastMailCheck > appPrefs->checkInterval) ) {

      if ( debuglev > 0 ) {
	 if ( !restart ) cout <<"Mail hasn't been checked in "
	    		      <<(int)(t-priv->lastMailCheck) <<" seconds.  ";
	 cout <<"Restarting mail check timer proc." <<endl;
      }

      if ( restart ) {
	 if ( priv->checkTimer ) XtRemoveTimeOut(priv->checkTimer);
	 priv->checkTimer =
	    XtAppAddTimeOut(context, appPrefs->checkInterval*1000,
			    (XtTimerCallbackProc)IshAppP::CheckForNewMail,
			    (XtPointer)priv);
      }

      else
	 CheckForNewMail();
   }

} // End VerifyMailCheck

/*---------------------------------------------------------------
 *  Method to display the requested message in a reading window.  Use
 *     the one passed if it is not NULL.
 */

void
IshAppC::DisplayMessage(MsgC *msg, ReadWinC *readWin)
{
   if ( !msg ) return;

   if ( !readWin ) {

//
// See if this message is already displayed somewhere
//
      if ( msg->IsViewed() ) {

//
// Find its window and pop it up
//
	 u_int	count = readWinList.size();
	 for (int i=0; i<count; i++) {

	    ReadWinC	*readWin = (ReadWinC *)*readWinList[i];
	    if ( msg == readWin->msg ) {
	       if ( !readWin->IsIconified() ) readWin->Show();
	       return;
	    }
	 }
      }

   } // End if no reading window is supplied

   BusyCursor(True);

//
// Get a new reading window if necessary
//
   Boolean	newWin = (readWin == NULL);
   if ( !readWin ) readWin = priv->GetReadWin();

   MsgC		*oldMsg = readWin->msg;
   VBoxC&	vbox    = mainWin->MsgVBox();

//
// Reset old message
//
   MsgItemC	*item;
   if ( oldMsg ) {
      item = oldMsg->icon;
      if ( oldMsg->folder->active && !mainWin->readingSelected )
	 vbox.DeselectItem(*item);
      oldMsg->ClearViewed();
      if ( item ) mainWin->curMsgList->remove(item);
   }

//
// Update new message
//
   item = msg->icon;
   if ( item ) {
      if ( msg->folder->active && !mainWin->readingSelected )
	 vbox.SelectItem(*item);
      mainWin->curMsgList->add(item);
      mainWin->FieldView().ScrollToItem(*item);
   }

   msg->SetViewed();

//
// Display message
//
   if ( newWin && !readWin->IsIconified() ) readWin->Show();
   readWin->SetMessage(msg);

   mainWin->Refresh();

   Broadcast("");
   BusyCursor(False);

} // End DisplayMessage

/*---------------------------------------------------------------
 *  Method to return a usable composition window
 */

SendWinC*
IshAppC::GetSendWin()
{
   BusyCursor(True);

//
// First look for a displayed, empty window
//
   SendWinC	*sendWin = NULL;
   unsigned	count = sendWinList.size();
   Boolean	found = False;
   int i=0; for (i=0; !found && i<count; i++) {

      sendWin = (SendWinC*)*sendWinList[i];

      if ( !sendWin->IsShown() || sendWin->IsEmpty() )
	 found = True;

   } // End for each existing composition window

//
// If we didn't find a window, we need to create a new one
//
   if ( !found ) {

      if ( debuglev > 0 ) cout <<"Creating composition window" <<endl;
      sendWin = new SendWinC("sendWin", *halApp);

      void	*tmp = (void*)sendWin;
      sendWinList.add(tmp);

   }

   sendWin->SetTo("");
   sendWin->SetSubject("");
   sendWin->SetCc("");
   sendWin->SetBcc("");
   sendWin->SetOther("");
   sendWin->SetBody("");

   BusyCursor(False);

   return sendWin;

} // End GetSendWin

