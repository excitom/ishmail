/*
 * $Id: main.C,v 1.8 2000/06/05 16:33:58 evgeny Exp $
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

#include "IshAppC.h"
#include "ShellExp.h"
#include "ImapMisc.h"

#include <sys/stat.h>

#include <new.h>

//
// Our own home dir
//
StringC ishHome;

//
// 
//
static int SetEnvValue(const char *name, const char *value)
{
#ifdef HAVE_SETENV
   return setenv(name, value, /* overwrite */ 1);
#else
   char *s;
   s = (char *) malloc(sizeof(char)*(strlen(name) + strlen(value) + 2));
   sprintf(s, "%s=%s", name, value);
   return putenv(s);
// NB: s MUST remain unfreed!
#endif
}

/*---------------------------------------------------------------
 *  New handler
 */

void
OutOfMemory(void)
{
   ishApp->PopupMessage
	       ("The application has run out of memory\nand cannot continue");
   exit(1);
}

/*---------------------------------------------------------------
 *  Main program
 */

int main(int argc, char **argv)
{
//
// Print version and exit
//
   if ( argc > 1 && strcmp(argv[1], "-v") == 0 ) {
      cout <<version <<endl;
      exit(0);
   }

   set_new_handler(OutOfMemory);

//
// Start helper process for expanding shell variables
//
   BuildImapPat();
   InitShellExpand();

//
// Get our own home dir
//
   char *cs = getenv("ISHHOME");
   if ( !cs ) {
      ishHome = versionIshHome;
   } else {
      ishHome = cs;
   }
   struct stat statb;
   if (stat(ishHome, &statb) || !S_ISDIR(statb.st_mode)) {
      cerr << "Can't find installation directory ($ISHHOME), exiting." <<endl;
      exit(1);
   }

//
// Add location of our auxiliary programs to the PATH
//
   cs = getenv("PATH");
   if ( !cs ) {
      cs = "";
   }
   StringC newpath = cs;
   newpath += ":" + ishHome + "/libexec";
   SetEnvValue("PATH", newpath);


//
// Set XFILESEARCHPATH
//
   cs = getenv("XFILESEARCHPATH");
   if ( !cs ) {
      cs = "";
   }
   StringC xfspath = cs;
   xfspath += ":" + ishHome + "/lib/%N%C" + ":" + ishHome + "/lib/%N";
   SetEnvValue("XFILESEARCHPATH", xfspath);

//
// ... and XBMLANGPATH
//
   cs = getenv("XBMLANGPATH");
   if ( !cs ) {
      cs = "";
   }
   StringC xbmlpath = cs;
   xbmlpath += ":" + ishHome + "/lib/%B";
   SetEnvValue("XBMLANGPATH", xbmlpath);

//
// Create application object.  This initializes X-Windows and reads user
//    preferences.
//
   IshAppC	*app = new IshAppC(&argc, argv, "ishmail", "Ishmail");

//
// Start looping
//
   if ( debuglev > 0 ) cout <<"MainLoop" <<endl;
   app->MainLoop();		// This never returns

   exit(0);

} // End main
