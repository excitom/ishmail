/*
 * $Id: Fork.C,v 1.6 2000/06/07 16:18:22 evgeny Exp $
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

#include "Fork.h"
#include "Misc.h"

#include <hgl/StringListC.h>
#include <hgl/CallbackC.h>
#include <hgl/SignalRegistry.h>
#include <hgl/HalAppC.h>
#include <hgl/SysErr.h>

#include <X11/Intrinsic.h>

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/wait.h>

#ifdef HAVE_VFORK_H
# include <vfork.h>
#endif

extern int debuglev;

/*---------------------------------------------------------------
 *  Where temp files are created
 */

static char	*ForkItTmpDir = NULL;

#ifndef P_tmpdir
# define P_tmpdir	"/var/tmp/"
#endif

/*---------------------------------------------------------------
 *  This data is passed to the ForkItDone callback
 */

typedef struct {

   pid_t	pid;
   CallbackC	userCb;
   int		status;

} ForkDataT;

/*---------------------------------------------------------------
 *  Method to build an arg list suitable for execvp from the given
 *     command line
 */

static void
BuildArgList(StringC cmdline, StringListC& argList)
{
   char	*cs = NULL; // getenv("SHELL");
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
 *  TimeOut used to finish processing in X thread.  Can't do this stuff in
 *     signal handler due to the fact that there may be X calls in the user
 *     callback.
 */

static void
ForkDone2(ForkDataT *fd, XtIntervalId*)
{
//
// Call user callback
//
   fd->userCb((void*)fd->status);

#ifdef HAVE_UNLINK
//
// Delete temporary files
//
   StringC	prefix(ForkItTmpDir);
   if ( prefix.LastChar() != '/' ) prefix += "/";
   prefix += (int)fd->pid;
//   unlink(prefix + ".err");
   unlink(prefix + ".out");
#endif

//
// Delete fork data
//
   delete fd;

   return;

} // End ForkDone2

/*-----------------------------------------------------------------------
 *  Callback to handle death of forked process
 */

static void
ForkDone(SignalDataT *sd, ForkDataT *fd)
{
//
// Make sure this is the right signal
//
   if ( sd->signum != SIGCHLD || sd->pid != fd->pid ) return;

//
// Remove this handler
//
   RemoveSignalCallback(SIGCHLD, (CallbackFn*)ForkDone, fd);

   fd->status = sd->status;

//
// Use a timeout in case there are any X calls in the user callback.  X calls
//    cannot be made from a signal handler.
//
   XtAppAddTimeOut(halApp->context, 0, (XtTimerCallbackProc)ForkDone2,
		   (XtPointer)fd);
}

/*---------------------------------------------------------------
 *  Method to fork/exec the specified command.  Returns the pid of the child
 *  process on success, -1 on failure.
 */

pid_t
ForkIt(char *cmdline, CallbackC *doneCb)
{
   if ( debuglev > 0 ) cout <<"ForkIt: cmdline = " <<cmdline <<endl;

//
// Find temp dir if necessary
//
   if ( !ForkItTmpDir ) {
      getenv("TMPDIR");
      if ( !ForkItTmpDir && (access(P_tmpdir, W_OK) == 0) )
	 ForkItTmpDir = P_tmpdir;
      if ( !ForkItTmpDir && (access("/tmp/",  W_OK) == 0) )
	 ForkItTmpDir = "/tmp/";
   }

//
// Create a pipe for receiving the return status
//
   int	pfd[2];
   if (pipe(pfd) == -1) return(-1);

   fcntl(pfd[0], F_SETFD, 1);
   fcntl(pfd[1], F_SETFD, 1);

//
// Add a callback to detect process completion
//
   static CallbackC	*nullCb = NULL;
   if ( !nullCb ) nullCb = new CallbackC;
   ForkDataT	*data = new ForkDataT;
   if ( doneCb ) data->userCb = *doneCb;
   else		 data->userCb = *nullCb;
   AddSignalCallback(SIGCHLD, (CallbackFn*)ForkDone, data);

//
// Create a child process for executing the command
//
   int		execstat = 0;
   data->pid = vfork();

//
// Check the result of the fork
//
   if ( data->pid == -1 ) {	// error

      int	err = errno;
      if ( debuglev > 0 )
	 cout <<"ForkIt: error = " <<SystemErrorMessage(err) <<endl;
      close(pfd[0]);
      close(pfd[1]);
      RemoveSignalCallback(SIGCHLD, (CallbackFn*)ForkDone, data);
      delete data;
      return (-err);
   }

   else if ( data->pid == 0 ) {	// child

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
      if ( debuglev > 0 ) cout <<"ForkIt: Effective gid in child is "
      			 <<getegid() <<endl;

//
// Extract the arguments for the command
//
      StringListC	argList;
      BuildArgList(cmdline, argList);

      int	argc = argList.size();
      char	**argv = new char*[argc+1];
      if ( debuglev > 0 ) cout <<"ForkIt: arguments are: " <<endl;
      int i=0; for (i=0; i<argc; i++) {
	 argv[i] = (char*)*argList[i];
	 if ( debuglev > 0 ) cout <<"\t" <<argv[i] <<endl;
      }
      argv[argc] = NULL;

//
// Make stdout and stderr point to temporary files
//
      pid_t	mypid = getpid();
      StringC	prefix(ForkItTmpDir);
      if ( prefix.LastChar() != '/' ) prefix += "/";
      prefix += (int)mypid;

      StringC	file = prefix + ".out";
      FILE	*ofp = fopen(file, "w+");
      if ( ofp ) {
	 close(1);	// stdout
	 close(2);	// stderr
	 fcntl(fileno(ofp), F_DUPFD, 1);
	 fcntl(fileno(ofp), F_DUPFD, 2);
	 fclose(ofp);
      }

      pid_t	gpid = setsid();
/*
      write(pfd[1], &gpid, sizeof(gpid));
*/

//
// Execute the command
//
      execstat = execvp(argv[0], (char *const *)argv);

//
// If we reached this point, there was a problem
//
      int	err = errno;
      if ( debuglev > 0 ) {
	 cout <<"ForkIt(child): execvp failed, errno = " <<err <<endl;
	 perror("ForkIt(child)");
      }

      execstat = err;
      write(pfd[1], (char*)&execstat, sizeof(execstat));
      _exit(err);

   } // End if this is the child process

//
// If we get here, this is the parent
//
   if ( debuglev > 0 ) cout <<"ForkIt: pid = " <<data->pid <<endl;

   close(pfd[1]);

//
// See if the child ever got started.  If we are able to read here, then
//   the child was able to write which is not a good sign.
//
   int	readstat = read(pfd[0], (char*)&execstat, sizeof(execstat));
   if ( readstat > 0 ) {
      RemoveSignalCallback(SIGCHLD, (CallbackFn*)ForkDone, data);
      delete data;
      data->pid = -execstat;
   }

//
// Close the pipe
//
   close(pfd[0]);

   return data->pid;

} // End ForkIt

/*---------------------------------------------------------------
 *  Create an error message for the given status code
 */

StringC
ForkStatusMsg(const char *cmdline, int code, pid_t pid)
{
   if ( debuglev > 0 )
      cout <<"Status code is " <<code <<" for command " <<cmdline <<endl;

   StringC	msg(cmdline);
   msg += "\n";

//
// If less than zero, it's an errno
//
   if ( code < 0 ) {
      msg += SystemErrorMessage(-code);
   }

//
// If greater than zero, it's an exit code
//
   else if ( code > 0 ) {

//
// Look for an error output file
//
      StringC	prefix(ForkItTmpDir);
      if ( prefix.LastChar() != '/' ) prefix += "/";
      prefix += (int)pid;
//      StringC	errfile = prefix + ".err";
      StringC	outfile = prefix + ".out";
      StringC	str;
      str.ReadFile(outfile);

//
// If there is still no information, try to interpret the code
//
      if ( str.size() == 0 ) {

	 str = "exited with status: ";
	 if ( WIFEXITED(code) ) {
	    char	ccode = (char)WEXITSTATUS(code);
	    code = ccode;
	 }
	 str += code;
      }

      msg += str;

   } // End if code > 0
      
   else {
      msg += "completed successfully.";
   }

   return msg;

} // End ForkStatusMsg

/*---------------------------------------------------------------
 *  Display the output message for the given process
 */

StringC
ForkOutput(pid_t pid)
{
//
// Look for an output file
//
   StringC	outfile(ForkItTmpDir);
   if ( outfile.LastChar() != '/' ) outfile += "/";
   outfile += (int)pid;
   outfile += ".out";

   StringC	str;
   str.ReadFile(outfile);

   return str;

} // End ForkOutput
