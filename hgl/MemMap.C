/*
 *  $Id: MemMap.C,v 1.4 2000/06/05 17:00:41 evgeny Exp $
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
#include "MemMap.h"
#include "HalAppC.h"
#include "SysErr.h"

#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#ifdef AIX
# include <sys/ipc.h>
# include <sys/shm.h>
#else
extern "C" {
# include <sys/mman.h>
}
#endif

extern int	debuglev;
extern Boolean	memMapOk;

/*--------------------------------------------------------------------------
 * FAKE Memory map the given file
 */

static MappedFileC
*FakeMapFile(const char *name, int mode, MappedFileC *mf)
{
   if ( debuglev>=2 ) cout <<"Fake MapFile(" <<name <<")" <<endl;

//
// Create a string for the file
//
   StringC	*buffer = new StringC;
   if ( !buffer ) {
      StringC	errmsg = "Could not create buffer for mapping file: ";
      errmsg += name;
      errmsg += "\n";
      errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
      return NULL;
   }

//
// Read the file
//
   if ( !buffer->ReadFile((char*)name) ) {
      if ( errno != ENOENT ) {
	 StringC	errmsg = "Could not read file: ";
	 errmsg += name;
	 errmsg += "\n";
	 errmsg += SystemErrorMessage(errno);
	 halApp->PopupMessage(errmsg);
      }
      return NULL;
   }

//
// Create the return structure
//
   if ( !mf ) mf = new MappedFileC;
   if ( !mf ) {
      delete buffer;
      StringC	errmsg = "Could not create data structure for mapping file: ";
      errmsg += name;
      errmsg += "\n";
      errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
      return NULL;
   }

   mf->name       = name;
   mf->mode       = mode;
   mf->fakeBuffer = buffer;
   mf->data       = *buffer;

   if ( debuglev>=2 ) {
      cout <<"   data structure at " <<mf <<endl;
      cout <<"   " <<mf->data.Length() <<" byte buffer at "
      		   <<(int*)mf->data.Addr() <<endl;
   }
   return mf;

} // End FakeMapFile

/*--------------------------------------------------------------------------
 * FAKE Un-memory map a mapped file
 */

static Boolean
FakeUnmapFile(MappedFileC *mf, Boolean del)
{
   if ( debuglev>=2 ) cout <<"Fake UnmapFile(" <<mf <<")" <<endl;
   if ( !mf ) return False;

//
// If the mode is O_RDWR, we have to write the memory contents back to the
//    file before deleting.
// If del is False, this is a Remap and we don't want to do the write.
//
   if ( mf->mode == O_RDWR && del ) {
      if ( debuglev>=2 ) cout <<"   updating file: " <<mf->name <<endl;
      if ( !mf->data.WriteFile(mf->name) ) {
	 StringC errmsg = "Could not update file: ";
	 errmsg += mf->name;
	 errmsg += " when unmapping.\n";
	 errmsg += SystemErrorMessage(errno);
	 halApp->PopupMessage(errmsg);
      }
   }

   StringC	*buffer = mf->fakeBuffer;
   if ( debuglev>=2 )
      cout <<"   deleting " <<mf->data.Length() <<" byte buffer at "
			    <<(int*)mf->data.Addr() <<endl;
   delete buffer;

   if ( del ) {
      if ( debuglev>=2 ) cout <<"   deleting data structure" <<endl;
      delete mf;
   }

   return True;

} // End FakeUnmapFile

/*--------------------------------------------------------------------------
 * Memory map the given file
 */

MappedFileC
*MapFile(const char *name, int mode, MappedFileC *mf)
{
   if ( !memMapOk ) return FakeMapFile(name, mode, mf);

   if ( debuglev>=2 ) cout <<"MapFile(" <<name <<")" <<endl;

//
// Open the file using the requested mode
//
   int	fd = open(name, mode);
   if ( fd == -1 ) {

//
// If the mode was O_RDWR, try it read only
//
      if ( mode == O_RDWR ) {
	 mode = O_RDONLY;
	 fd = open(name, mode);
      }

      if ( fd == -1 ) {
	 if ( errno != ENOENT ) {
	    StringC errmsg = "Could not open file: ";
	    errmsg += name;
	    errmsg += " for mapping.\n";
	    errmsg += SystemErrorMessage(errno);
	    halApp->PopupMessage(errmsg);
	 }
	 return NULL;
      }
   }

//
// Get the file size
//
   off_t	len = lseek(fd, (off_t)0, SEEK_END);
   if ( len == (off_t)-1 ) {
      StringC errmsg = "Could not seek in file: ";
      errmsg += name;
      errmsg += " while mapping.\n";
      errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
      close(fd);
      return NULL;
   }

//
// Map the file
//
#ifdef AIX

   int		prot = 0;
   if ( mode == O_RDONLY ) prot = SHM_RDONLY;
   void	*pa = shmat(fd, NULL, SHM_MAP|prot);
   if ( pa == (void*)-1 && len > 0 ) {
      StringC errmsg = "Could not map file: ";
      errmsg += name;
      errmsg += '\n';
      errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
      close(fd);
      return NULL;
   }

   if ( debuglev>=2 ) cout <<"   file descriptor is " <<fd <<endl;

#else

   int	flags = (mode == O_RDWR ? MAP_SHARED : MAP_PRIVATE);
#ifdef HPUX
   flags |= MAP_FILE | MAP_VARIABLE;
#endif

   int		prot = PROT_READ;
   if      ( mode == O_WRONLY ) prot  = PROT_WRITE;
   else if ( mode == O_RDWR   ) prot |= PROT_WRITE;

   caddr_t	pa = (caddr_t)mmap(NULL, (size_t)len, prot, flags, fd, (off_t)0);
   if ( pa == (caddr_t)-1 && len > 0 ) {
      StringC errmsg = "Could not map file: ";
      errmsg += name;
      errmsg += '\n';
      errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
      close(fd);
      return NULL;
   }
   close(fd);
   fd = -1;
#endif

   if ( debuglev>=2 ) cout <<"   memory address is " <<(int*)pa <<endl;

//
// Create the return structure
//
   if ( !mf ) mf = new MappedFileC;
   if ( !mf ) {
      if ( debuglev>=2 ) perror("   Could not create data structure");
      StringC errmsg = "Could not create data structure while mapping file: ";
      errmsg += name;
      errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
      return NULL;
   }

   mf->name = name;
   mf->mode = mode;
   mf->fd   = fd;
   mf->data.Set((const char*)pa, (u_int)len);

   if ( debuglev>=2 ) cout <<"   data structure at " <<mf <<endl;
   return mf;

} // End MapFile

/*--------------------------------------------------------------------------
 * Un-memory map a mapped file
 */

Boolean
UnmapFile(MappedFileC *mf, Boolean del)
{
   if ( !memMapOk ) return FakeUnmapFile(mf, del);

   if ( !mf ) return False;

   if ( debuglev>=2 ) cout <<"UnmapFile(" <<mf->name <<")" <<endl;

   errno = 0;

#ifdef AIX
   if ( debuglev>=2 ) cout <<"   closing file " <<mf->fd <<endl;
   close(mf->fd);
#else
   if ( mf->Length() > 0 ) {
      if ( debuglev>=2 )
	 cout <<"   unmapping address " <<(int*)mf->Addr() <<endl;
      munmap((caddr_t)mf->Addr(), (size_t)mf->Length());
   }
#endif
   
   if ( del && errno == 0 ) {
      if ( debuglev>=2 ) cout <<"   deleting data structure " <<mf <<endl;
      delete mf;
   }
   return (errno == 0);
}

/*--------------------------------------------------------------------------
 * Re-memory map the given file
 */

Boolean
RemapFile(MappedFileC *mf)
{
   if ( debuglev>=2 ) {
      cout <<"RemapFile(" <<mf <<")" <<endl;
      cout <<"   name is " <<mf->name <<endl;
   }

   StringC	name = mf->name;

   if ( !UnmapFile(mf, False/*don't delete*/) )
      return False;

   return (MapFile(name, mf->mode, mf) != NULL);

} // End Remap
