/*
 *  $Id: MailFile.C,v 1.4 2000/05/07 12:26:12 fnevgeny Exp $
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

#include "Fork.h"
#include "SafeSystem.h"

#include <hgl/HalAppC.h>
#include <hgl/StringC.h>
#include <hgl/StringListC.h>
#include <hgl/CallbackC.h>
#include <hgl/SysErr.h>

#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

#ifdef HAVE_SYSEXITS_H
# include <sysexits.h>
#else
# define EX_USAGE       64      /* command line usage error */
# define EX_DATAERR     65      /* data format error */
# define EX_NOINPUT     66      /* cannot open input */
# define EX_NOUSER      67      /* addressee unknown */
# define EX_NOHOST      68      /* host name unknown */
# define EX_UNAVAILABLE 69      /* service unavailable */
# define EX_SOFTWARE    70      /* internal software error */
# define EX_OSERR       71      /* system error (e.g., can't fork) */
# define EX_OSFILE      72      /* critical OS file missing */
# define EX_CANTCREAT   73      /* can't create (user) output file */
# define EX_IOERR       74      /* input/output error */
# define EX_TEMPFAIL    75      /* temp failure; user is invited to retry */
# define EX_PROTOCOL    76      /* remote error in protocol */
# define EX_NOPERM      77      /* permission denied */
#endif

extern int	debuglev;

/*---------------------------------------------------------------
 *  Function to return status string for given sendmail error
 */

StringC
SendmailErrorMessage(int error)
{
   char	*cs;
   switch(error) {
      case EX_USAGE:		cs = "Command line usage error";	break;
      case EX_DATAERR:		cs = "Data format error";		break;
      case EX_NOINPUT:		cs = "Cannot open input";		break;
      case EX_NOUSER:		cs = "Addressee unknown";		break;
      case EX_NOHOST:		cs = "Host name unknown";		break;
      case EX_UNAVAILABLE:	cs = "Service unavailable";		break;
      case EX_SOFTWARE:		cs = "Internal software error";		break;
      case EX_OSERR:		cs = "System error (e.g., can't fork)";	break;
      case EX_OSFILE:		cs = "Critical OS file missing";	break;
      case EX_CANTCREAT:	cs = "Can't create (user) output file";	break;
      case EX_IOERR:		cs = "Input/output error";		break;
      case EX_TEMPFAIL:	cs = "Temp failure; user is invited to retry";	break;
      case EX_PROTOCOL:		cs = "Remote error in protocol";	break;
      case EX_NOPERM:		cs = "Permission denied";		break;
      default:
      {
	 StringC	msg("Unknown error: ");
	 msg += error;
	 return msg;
      }

   } // End switch error

   return cs;

} // End SendmailErrorMessage

/*---------------------------------------------------------------
 *  Callback used when file send has completed
 */

typedef struct {
   pid_t	pid;
   StringC	file1;
   StringC	file2;
   StringC	cmd;
} SendDataT;

void
SendComplete(int status, SendDataT *data)
{
   if ( debuglev > 0 ) cout <<"SendComplete: " <<status <<endl;
   if ( status != 0 ) {

      StringC msg = data->cmd;
      msg += " reported the following error:\n";

      if ( status < 0 )
	 msg += SystemErrorMessage(status);
      else {
         if ( WIFEXITED(status) ) {
	    status = WEXITSTATUS(status);
	    msg += SendmailErrorMessage(status);
	 }
	 else {
	    msg += status;
	 }
      }

      halApp->PopupMessage(msg);
   }
#if 0
      halApp->PopupMessage(ForkStatusMsg(data->cmd, status, data->pid));
#endif

   if ( data->file1.size() > 0 ) {
       if ( debuglev > 0 ) cout <<"   unlinking " <<data->file1 <<endl;
       unlink(data->file1);
    }
   if ( data->file2.size() > 0 ) {
       if ( debuglev > 0 ) cout <<"   unlinking " <<data->file2 <<endl;
       unlink(data->file2);
   }

   delete data;
}

/*---------------------------------------------------------------
 *  Function to run the specified command
 */

static Boolean
RunCommand(StringC& cmdStr, char *sendmailCmd, Boolean wait,
	   char *file1, char *file2, Boolean del1, Boolean del2)
{
   if ( debuglev > 0 ) cout <<cmdStr <<endl;

   if ( wait ) {

      int	status = SafeSystem(cmdStr);

      if ( status != 0 ) {
	 if ( debuglev > 0 ) cout <<"status: " <<status <<endl;

	 StringC msg = sendmailCmd;
	 msg += " reported the following error:\n";
	 msg += SendmailErrorMessage(status);
	 halApp->PopupMessage(msg);
	 return False;
      }

      if ( del1 && file1 ) {
	  if ( debuglev > 0 ) cout <<"   unlinking " <<file1 <<endl;
	  unlink(file1);
      }
      if ( del2 && file2 ) {
	  if ( debuglev > 0 ) cout <<"   unlinking " <<file2 <<endl;
	  unlink(file2);
      }

   } // End if we should wait on completion

   else {

      SendDataT	*data = new SendDataT;
      data->cmd  = cmdStr;
      if ( del1 && file1 ) data->file1 = file1;
      if ( del2 && file2 ) data->file2 = file2;
      CallbackC    doneCb((CallbackFn*)SendComplete, data);
      data->pid = ForkIt(cmdStr, &doneCb);

      if ( data->pid < 0 ) {
	 SendComplete((int)data->pid, data);
	 return False;
      }

   } // End if we're not waiting

   return True;

} // End RunCommand

/*---------------------------------------------------------------
 *  Method to mail the given temp file to a single recipient
 */

Boolean
MailFile(char *file, char *recipient, char *sendmailCmd, Boolean wait,
	 Boolean del)
{
   StringC	cmdStr("cat ");
   cmdStr += file;
   cmdStr += " | ";
   cmdStr += sendmailCmd;
   cmdStr += " -oi -oem ";
   if ( wait ) cmdStr += "-odi ";
   cmdStr += '\'';
   cmdStr += recipient;
   cmdStr += '\'';

   return RunCommand(cmdStr, sendmailCmd, wait, file, NULL, del, False);

} // End MailFile

/*---------------------------------------------------------------
 *  Method to mail the given temp file to several recipients
 */

Boolean
MailFile(char *file, StringListC& list, char *sendmailCmd, Boolean wait,
	 Boolean del)
{
   StringC	cmdStr("cat ");
   cmdStr += file;
   cmdStr += " | ";
   cmdStr += sendmailCmd;
   cmdStr += " -oi -oem ";
   if ( wait ) cmdStr += "-odi ";

   u_int	count = list.size();
   for (u_int i=0; i<count; i++) {
      if ( i > 0 ) cmdStr += ' ';
      cmdStr += '\'';
      cmdStr += *list[i];
      cmdStr += '\'';
   }

   return RunCommand(cmdStr, sendmailCmd, wait, file, NULL, del, False);

} // End MailFile

/*---------------------------------------------------------------
 *  Method to mail several temp files to several recipients
 */

Boolean
MailFiles(char *file1, char *file2, StringListC& list, char *sendmailCmd,
	  Boolean wait, Boolean del1, Boolean del2)
{
   StringC	cmdStr("cat ");

   cmdStr += file1;
   cmdStr += ' ';
   cmdStr += file2;
   cmdStr += " | ";
   cmdStr += sendmailCmd;
   cmdStr += " -oi -oem ";
   if ( wait ) cmdStr += "-odi ";

//
// Add recipient names
//
   u_int	count = list.size();
   for (int	i=0; i<count; i++) {
      if ( i > 0 ) cmdStr += ' ';
      cmdStr += '\'';
      cmdStr += *list[i];
      cmdStr += '\'';
   }

//
// Send message
//
   return RunCommand(cmdStr, sendmailCmd, wait, file1, file2, del1, del2);

} // End MailFiles

