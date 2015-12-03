/*
 *  $Id: ImapMisc.C,v 1.4 2000/12/13 17:15:49 evgeny Exp $
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
#include "AppPrefC.h"
#include "FolderPrefC.h"
#include "ImapMisc.h"
#include "Imap.h"

#include <hgl/CharC.h>
#include <hgl/RegexC.h>

static RegexC	*imapPat    = NULL;
static RegexC	*imapPat2   = NULL;
static RegexC	*imapUrlPat = NULL;

#define	SERV_PART	1
#define	PATH_PART	2

/*---------------------------------------------------------------
 *  Function to build the regular expression
 */

void
BuildImapPat()
{
   if ( !imapPat ) {
      imapPat    = new RegexC("{\\([^{}]+\\)}\\(.*\\)");
      imapPat2   = new RegexC("\\(INBOX.*\\)");
      imapUrlPat = new RegexC("\\(imap\\|imaps\\)://\\(.+@\\)\\(.+\\)/\\(.*\\)");
   }
}

Boolean
IsImapUrl(CharC name)
{
   if ( name.StartsWith("imap://") || name.StartsWith("imaps://") ) {
      return True;
   } else {
      return False;
   }
}

/*---------------------------------------------------------------
 *  Function to determine if a name is an IMAP name: "{server}path"
 */
Boolean
IsImapName(CharC name)
{
//
// If we're not using local folders at all, return True
//
   if ( !ishApp->folderPrefs->UsingLocal() )
      return True;

//
// If we're using both IMAP and local folders, look for the pattern
//
   
// Try URL syntax first
   if ( IsImapUrl(name) ) {
      return True;
   }   
   
   if ( imapPat->match(name) )
      return True;

//
// If we're using IMAP folders, try this loosy check as a last resort:
//
   if ( ishApp->appPrefs->usingImap ) {
      return imapPat2->match(name);
   } else {
      return False;
   }

} // End IsImap

/*---------------------------------------------------------------
 *  Function to extract the server part from an IMAP name: "{server}path"
 */

CharC
ImapServerPart(CharC name)
{
   if ( imapPat->match(name) )
      return name((*imapPat)[SERV_PART]);
   else
      return ishApp->appPrefs->imapServer;
}

/*---------------------------------------------------------------
 *  Function to extract the path part from an IMAP name: "{server}path"
 */

CharC
ImapPathPart(CharC name)
{
   if ( imapPat->match(name) )
      return name((*imapPat)[PATH_PART]);
   else
      return name;
}

Boolean
ParseImapUrl(CharC name,
    StringC &server, long *port, Boolean *imaps, StringC &user, StringC &path)
{
   if ( imapUrlPat->match(name) ) {
      CharC protoName = name((*imapUrlPat)[1]);
      if ( protoName.Equals("imaps") ) {
         *imaps = True;
         *port  = IMAPS_PORT;
      } else {
         *imaps = False;
         *port  = IMAP_PORT;
      }
      
      server = name((*imapUrlPat)[3]);
      user   = name((*imapUrlPat)[2]);
      user.CutEnd(1); // remove trailing '@'
      path   = name((*imapUrlPat)[4]);
      
      // *port = ;
      
      return True;
   } else {
      return False;
   }
}

Boolean
ParseImapName(CharC name,
    StringC &server, long *port, Boolean *imaps, StringC &user, StringC &path)
{
   if ( ParseImapUrl(name, server, port, imaps, user, path) ) {
      return True;
   } else {
      *imaps = False;
      *port  = IMAP_PORT;
      user   = ishApp->userId;
      server = ImapServerPart(name);
      path   = ImapPathPart(name);
   }
}
