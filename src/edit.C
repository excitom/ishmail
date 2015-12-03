/*
 * $Id: edit.C,v 1.2 2000/05/07 12:26:13 fnevgeny Exp $
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

#include "Fork.h"
#include "IshAppC.h"
#include "CompPrefC.h"

#include <hgl/StringC.h>
#include <hgl/CallbackC.h>
#include <hgl/CharC.h>

#include <sys/stat.h>
#include <sys/wait.h>

extern int	debug1;

/*---------------------------------------------------------------
 *  Class used to store data about each edit process
 */

class EditDataC {
public:

   pid_t		pid;		// Editor process id
   StringC		cmdStr;		// Command being run
   StringC		fileName;	// Name of file being edited
   time_t		mod_time;	// Time of last modification
   CallbackC		modifyCall;	// Call this when done and modified
   CallbackC		doneCall;	// Call this when done

//
// Assignment from EditDataC
//
   EditDataC&	operator=(const EditDataC&);

//
// Constructors
//
   EditDataC();
   EditDataC(const char*, const char*, const CallbackC&, const CallbackC&);
   EditDataC(const EditDataC& e) { *this = e; }
};

/*---------------------------------------------------------------
 *  Constructors for EditDataC class
 */

EditDataC::EditDataC()
{
   pid      = 0;
   mod_time = 0;
}

EditDataC::EditDataC(const char *cmd, const char *file, const CallbackC& mCall,
						        const CallbackC& dCall)
{
   pid        = 0;
   cmdStr     = cmd;
   fileName   = file;
   modifyCall = mCall;
   doneCall   = dCall;

//
// Substitute file name
//
   cmdStr.Replace("$FILE", file);

//
// Get the status of the file
//
   struct stat	statBuf;
   if ( stat(fileName, &statBuf) != 0 ) // Not able to stat
      mod_time = 0;

//
// Save the modification time
//
   else
      mod_time = statBuf.st_mtime;
}

EditDataC&
EditDataC::operator=(const EditDataC& e)
{
   if ( this != &e ) {
      pid	 = e.pid;
      cmdStr     = e.cmdStr;
      fileName   = e.fileName;
      mod_time   = e.mod_time;
      modifyCall = e.modifyCall;
      doneCall   = e.doneCall;
   }
   return (*this);
}

/*---------------------------------------------------------------
 *  TimeOut used to finish processing since there may be X calls in ther
 *     user callbacks.
 */

void
Edit_Done(int status, EditDataC *ed)
{
   if ( debug1 ) cout <<"Exit status is " <<status <<endl;

//
// Make sure the child exited cleanly
//
   if ( status == 0 ) {

      if ( debug1 ) cout <<"It was a clean exit" <<endl;

//
// Get the status of the file
//
      struct stat	statBuf;
      if ( stat(ed->fileName, &statBuf) == 0 ) {

	 if ( debug1 ) cout <<"Got file status" <<endl;

//
// See if the file has been modified
//
	 if ( statBuf.st_mtime != ed->mod_time ) {

	    if ( debug1 ) cout <<"File was modified" <<endl;

//
// Call modification callback
//
	    ed->modifyCall((char *)ed->fileName);
	 }
	 else if ( debug1 ) cout <<"File was NOT modified" <<endl;

      } // End if file could be stat'd

      else if ( debug1 ) cout <<"DID NOT get file status" <<endl;

   } // End if process exited cleanly

   else {
      if ( debug1 ) cout <<"It was NOT a clean exit" <<endl;
      StringC	errmsg = "The edit process reported the following:\n";
      errmsg += ForkStatusMsg(ed->cmdStr, status, ed->pid);
      halApp->PopupMessage(errmsg);
   }

//
// Call finished callback
//
   ed->doneCall((char *)ed->fileName);

   delete ed;

   return;

} // End Edit_Done

/*---------------------------------------------------------------
 *  Routine to run the specified command on a file
 */

pid_t
FilterFile(const char *cmdStr, const char *path, const CallbackC& modCall,
						 const CallbackC& doneCall)
{
//
// We have to make temporary copies because for some reason we get a BUS
//   ERROR otherwise
//
   CallbackC	mod  = modCall;
   CallbackC	done = doneCall;

//
// Run the command in the background
//
   EditDataC	*ed = new EditDataC(cmdStr, path, mod, done);
   CallbackC	doneCb((CallbackFn*)Edit_Done, ed);
   ed->pid = ForkIt(ed->cmdStr, &doneCb);

   return ed->pid;

} // End FilterFile

