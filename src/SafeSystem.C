/*
 *  $Id: SafeSystem.C,v 1.5 2000/08/04 23:41:48 kherron Exp $
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

#include "PidListC.h"

#include <hgl/SignalRegistry.h>
#include <hgl/StringC.h>
#include <hgl/StringListC.h>
#include <hgl/IntListC.h>
#include <hgl/SysErr.h>

#include <X11/Intrinsic.h>

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

#ifdef HAVE_VFORK_H
# include <vfork.h>
#endif

extern int	debuglev;

static PidListC	*pidList    = NULL;
static IntListC	*statusList = NULL;

/*---------------------------------------------------------------
 *  This data is passed to the SafeDone callback
 */

typedef struct {

   pid_t		pid;
   int			status;
   volatile Boolean	done;

} SafeDataT;

/*-----------------------------------------------------------------------
 *  Callback to handle death of child process
 */

static void
SafeDone(SignalDataT *sig, SafeDataT *safe)
{
   if ( debuglev > 1 )
      cout <<"SafeDone: signal = " <<sig->signum <<", pid = "
				   <<sig->pid <<endl;

//
// Make sure this is the right signal
//
   if ( sig->signum != SIGCHLD ) return;

//
// If the child pid doesn't match the signal data pid, add it to the list
//    of terminated children.  It may have died so fast we haven't set the
//    signal data pid yet.
//
   if ( sig->pid != safe->pid ) {
      pidList->add(sig->pid);
      statusList->add(sig->status);
      return;
   }

//
// Remove this handler
//
   RemoveSignalCallback(SIGCHLD, (CallbackFn*)SafeDone, safe);

   safe->status = sig->status;
   safe->done   = True;

} // End SafeDone

/*---------------------------------------------------------------
 *  Method to build an arg list suitable for execvp from the given
 *     command line
 */

static void
BuildArgList(const char *cmdline, StringListC& argList)
{
   static char	*cs = NULL;
   if ( !cs ) cs = getenv("SHELL");
   if ( !cs ) cs = "/bin/sh";

   StringC	shell(cs);
   argList.add(shell);

//
// Add flags to preserve environments
//
   if ( shell.EndsWith("bash") ) {
      argList.add("-norc");
      argList.add("-c");
   }
   else if ( shell.EndsWith("csh") )
      argList.add("-fbc");
   else
      argList.add("-c");

   argList.add(cmdline);

} // End BuildArgList

/*-----------------------------------------------------------------------
 *  Method to make a system() call with the real gid rather than the
 *     effective gid.  We don't want to run any child processes with
 *     the setgid permissions on.
 */

int
SafeSystem(const char *cmd, StringC *output)
{
   if ( debuglev > 1 ) cout <<"SafeSystem: cmd = " <<cmd <<endl;

   char	*tmpFile = NULL;
   if ( output )
      tmpFile = tempnam(NULL, "safe.");

   if ( !pidList ) {

      pidList = new PidListC;
      pidList->AllowDuplicates(TRUE);
      pidList->SetSorted(FALSE);

      statusList = new IntListC;
      statusList->AllowDuplicates(TRUE);
      statusList->SetSorted(FALSE);
   }

   SafeDataT	*data = new SafeDataT;
   data->pid    = 0;
   data->status = 0;
   data->done   = False;
   AddSignalCallback(SIGCHLD, (CallbackFn*)SafeDone, data);

   data->pid = vfork();

//
// Check the result of the fork
//
   if ( data->pid == -1 ) {	// error

      int	err = errno;
      if ( debuglev > 0 )
	 cout <<"SafeSystem: fork error = " <<SystemErrorMessage(err) <<endl;
      RemoveSignalCallback(SIGCHLD, (CallbackFn*)SafeDone, data);
      delete data;
      return err;
   }

   else if ( data->pid == 0 ) {	// child

//
// Extract the arguments for the command
//
      StringListC	argList;
      BuildArgList(cmd, argList);

      int	argc = argList.size();
      char	**argv = new char*[argc+1];
      for (int i=0; i<argc; i++) argv[i] = (char*)*argList[i];
      argv[argc] = NULL;

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

      pid_t	gpid = setsid();

//
// Make stdout point to the temporary file
//
      if ( output ) {
	 FILE      *ofp = fopen(tmpFile, "w+");
	 if ( ofp ) {
	    close(1);      // stdout
	    fcntl(fileno(ofp), F_DUPFD, 1);
	    fclose(ofp);
	 }
      }

//
// Execute the command
//
      execvp(argv[0], (char *const *)argv);

//
// If we reached this point, there was a problem
//
      int	err = errno;
      if ( debuglev > 0 ) {
	 cerr <<"SafeSystem(child): execvp failed, errno = " <<err <<endl;
	 perror("SafeSystem(child)");
      }

      _exit(err);

   } // End if this is the child process

//
// If we get here, this is the parent
//
   if ( debuglev > 1 ) cout <<"SafeSystem: pid = " <<data->pid <<endl;

   int	status = 0;

//
// See if the child already terminated
//
   if ( pidList->includes(data->pid) ) {
      int	index = pidList->indexOf(data->pid);
      data->status = *(*statusList)[index];
      pidList->remove((u_int)index);
      statusList->remove((u_int)index);
      data->done = True;
      RemoveSignalCallback(SIGCHLD, (CallbackFn*)SafeDone, data);
   }

//
// Wait for child to terminate
//
   while ( !data->done ) sleep(1);
   status = data->status;
   delete data;

   if ( WIFEXITED(status) ) status = WEXITSTATUS(status);

//
// Read the temp file if necessary
//
   if ( output && tmpFile ) {
      output->Clear();
      status = !output->ReadFile(tmpFile);
      unlink(tmpFile);
      free(tmpFile);
   }

   if ( debuglev > 1 ) cout <<"SafeSystem: status = " <<status <<endl;

   return status;
}
