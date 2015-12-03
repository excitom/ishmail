/*
 *  $Id: ShellExp.C,v 1.5 2000/06/07 16:10:01 evgeny Exp $
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
#include "SafeSystem.h"
#include "Misc.h"
#include "ImapServerC.h"
#include "ImapMisc.h"

#include <hgl/SysErr.h>
#include <hgl/StringC.h>
#include <hgl/StringListC.h>
#include <hgl/CharC.h>

#include <X11/Intrinsic.h>

#include <errno.h>

#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef HAVE_VFORK_H
# include <vfork.h>
#endif

#ifdef HAVE_SYS_SELECT_H
# include <sys/select.h>
#endif

extern int	debuglev;

/*------------------------------------------------------------------------
 * Function to start process that will be used to expand shell variables
 */

static int	shex_po;             // Parent output
static int	shex_pi;             // Parent input
static int	shex_co;             // Child output
static int	shex_ci;             // Child input
static Boolean	shex_ok = False;

void
InitShellExpand()
{
//
// Create two pipes for communicating
//
   int	pipe1[2];	// Pipe to   the shell.  Read 0, write 1
   int	pipe2[2];	// Pipe from the shell.  Read 0, write 1

   if ( pipe(pipe1) == -1 || pipe(pipe2) == -1 ) {
      cerr <<"InitShellExpand: error = " <<SystemErrorMessage(errno) <<endl;
      return;
   }

   shex_ci = pipe1[0];
   shex_po = pipe1[1];
   shex_pi = pipe2[0];
   shex_co = pipe2[1];

//
// Create a child process for executing the command
//
   pid_t	pid = vfork();

//
// Check the result of the fork
//
   if ( pid == -1 ) {	// error

      int	err = errno;
      cerr <<"InitShellExpand: error = " <<SystemErrorMessage(err) <<endl;
      close(shex_po);
      close(shex_pi);
      close(shex_co);
      close(shex_ci);
      return;
   }

   else if ( pid == 0 ) {	// child

//
// Close the parent sides
//
      close(shex_po);
      close(shex_pi);

//
// Set the effective group id to the real group id.  We don't want to
//    run any child processes with the setgid permissions on.
//
#ifdef HAVE_SETEGID
      setegid(getgid());
#else
      gid_t	gid = getgid();
      setresgid(gid, gid, gid);
#endif

//
// Attach the child input pipe to standard input
//
      close(0);	// stdin
      fcntl(shex_ci, F_DUPFD, 0);

//
// Make stdout and stderr point to the child output pipe
//
      close(1);	// stdout
      close(2);	// stderr
      fcntl(shex_co, F_DUPFD, 1);
      fcntl(shex_co, F_DUPFD, 2);

      pid_t	gpid = setsid();

      char	*cs = getenv("SHELL");
      if ( !cs ) cs = "/bin/sh";

      int	argc = 1;
      char	*argv[2];
      argv[0] = cs;
      argv[argc] = NULL;

//
// Execute the command
//
      execvp(argv[0], (char *const *)argv);

//
// If we reached this point, there was a problem
//
      int	err = errno;
      cerr <<"InitShellExpand(child): execvp failed, errno = "
	   <<SystemErrorMessage(err) <<endl;
      _exit(err);

   } // End if this is the child process

//
// Close the child sides
//
   close(shex_co);
   close(shex_ci);

   if ( debuglev > 0 ) cout <<"ShellExpand pid: " <<pid <<endl;
   shex_ok = True;

} // End InitShellExpand

/*------------------------------------------------------------------------
 * Function to see if there is more data to be read for a file descriptor
 */

static Boolean
MoreData(int fd)
{
   fd_set		fds;
   FD_ZERO(&fds);
   FD_SET(fd, &fds);

   struct timeval	timeout;
   timeout.tv_sec  = 0;
   timeout.tv_usec = 0;

   int	status = select(FD_SETSIZE, &fds, NULL, NULL, &timeout);

   if ( status > 0 ) return True;

   if ( status != 0 && debuglev > 0 ) {
      int	err = errno;
      cout <<"ShellExpand: select failed: " <<SystemErrorMessage(err) <<endl;
   }

   return False;

} // End MoreData

/*------------------------------------------------------------------------
 * Function to remove all but the first word
 */

static void
Truncate(StringC& string)
{
   string.Trim();
   int	pos = string.PosOf(' ');
   if ( pos > 0 ) string.Clear(pos);
}

/*------------------------------------------------------------------------
 * Function to expand shell variables in strings
 */

void
ShellExpand(StringC& string, Boolean oneWord=False)
{
   if ( string.size() == 0 ) return;

   static StringC	*orig = NULL;
   if ( !orig ) orig = new StringC;

//
// See if we're expanding an IMAP name
//
   if ( IsImapName(string) ) {

      if ( debuglev > 0 ) *orig = string;

//
// Substitute for + or =
//
      CharC	path = ImapPathPart(string);
      char	first = path[0];
      if ( first == '+' || first == '=' ) {

	// If using both IMAP and local folders, don't assume the
	// folder directory applies to the IMAP server. It applies in 
	// this case to the local folders. Just throw away the + or =.

	int	pos = (int)(path.Addr() - (char*)string);
	if (ishApp->folderPrefs->UsingLocal()) 
	   string(pos,1) = "";

	// If using only IMAP folders, assume the folder directory
	// should be applied.

	else
	   string(pos,1) = ishApp->appPrefs->FolderDir() + "/";
      }

//
// See if the name contains any characters worthy of expansion
//
      if ( strpbrk((char*)string, "$*?~^!`") ) {

//
// This name needs expanding
//
	 StringC	serverName = ImapServerPart(string);
	 ImapServerC	*server    = FindImapServer(serverName);
	 StringListC	list;
	 StringListC	output;
	 if ( server->ListMailboxes(string, list, output) && list.size() > 0 ) {

	    if ( oneWord ) string = *list[0];
	    else {
	       string.Clear();
	       u_int	count = list.size();
	       for (int i=0; i<count; i++) {
		  if ( i > 0 ) string += ' ';
		  string += *list[i];
	       }
	    }

	 } // End if got listing

      } // End if there are any wildcard chars

      if ( oneWord ) Truncate(string);
      if ( debuglev > 0 )
	 cout <<"\"" <<*orig <<"\" expands to \"" <<string <<"\"" <<endl;
      return;

   } // End if we need to use the IMAP server to expand the name

   else {
   //
   // Substitute for + or =
   //
       char	first = string[0];
       if ( first == '+' || first == '=' ) {
	    string(0,1) = ishApp->appPrefs->FolderDir() + "/";
       }
   }

//
// Using local shell expansion from here on
//

//
// See if the name contains any characters worthy of expansion
//
   if ( !strpbrk((char*)string, "$*?~^!`") ) {
      if ( oneWord ) Truncate(string);
      return;
   }

   static StringC	*cmd  = NULL;
   if ( !cmd ) cmd = new StringC;
   *orig = string;

//
// Use the "echo" command to expand the string
//
   *cmd = "echo ";
   *cmd += string;

   if ( shex_ok ) {

      *cmd += "\n";
      string.Clear();

//
// Pass the string to the expansion pipe
//
      char	line[128];
      int	count = write(shex_po, (char*)*cmd, cmd->size());
      if ( count != cmd->size() ) {
	 if ( debuglev > 0 ) {
	    int	err = errno;
	    cout <<"ShellExpand: write failed: "
	         <<SystemErrorMessage(err) <<endl;
	 }
	 shex_ok = False;
	 string = *orig;
      }

//
// Read the results of the command
//
      else do {

	 count = read(shex_pi, line, 127);
	 if ( count <= 0 ) {
	    if ( debuglev > 0 ) {
	       int	err = errno;
	       cout <<"ShellExpand: read failed: "
		    <<SystemErrorMessage(err) <<endl;
	    }
	    shex_ok = False;
	    string = *orig;
	 }

//
// Add the results to the output string
//
	 else {
	    line[count] = 0;
	    string += line;
	 }

      } while ( count == 127 && MoreData(shex_pi) );

      if ( string.EndsWith('\n') ) string.CutEnd(1);

      if ( string.Contains("not a tty",        IGNORE_CASE) ||
	   string.Contains("not a typewriter", IGNORE_CASE) ) {
	 shex_ok = False;
	 string = *orig;
      }

   } // End if child process is available

//
// Use SafeSystem if child process is not available
//
   if ( !shex_ok ) {
      int	status = SafeSystem(*cmd, &string);
      if ( string.size() == 0 ) string = *orig;
      while ( string.EndsWith("\n") ) string.Clear(string.size()-1);
   }

   if ( string.size() == 0 || string.StartsWith("not a tty", IGNORE_CASE) )
      string = *orig;

//
// Throw out extra words if necessary
//
   if ( oneWord ) Truncate(string);
   if ( debuglev > 0 )
      cout <<"\"" <<*orig <<"\" expands to \"" <<string <<"\"" <<endl;
   return;

} // End ShellExpand

/*---------------------------------------------------------------
 *  Function to perform shell expansion on the names in a list.
 *  Any new names are added to the list.
 */

void
ExpandList(StringListC& list)
{
   StringListC	newList;
   StringC	entry;
   Boolean	changed = False;

//
// Loop through strings
//
   u_int	count = list.size();
   for (int i=0; i<count; i++) {

//
// Pass string through shell
//
      entry = *list[i];
      ShellExpand(entry);

//
// If it changed, see if it expanded to multiple names
//
      if ( entry != *list[i] ) {
	 ExtractList(entry, newList, " \t");
	 changed = True;
      }

      else
	 newList.add(entry);

   } // End for each string

//
// If there were any changes, use the new list
//
   if ( changed )
      list = newList;

} // End ExpandList

