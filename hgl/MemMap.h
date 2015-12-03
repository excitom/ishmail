/*
 *  $Id: MemMap.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
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
#ifndef _MemMap_h_
#define _MemMap_h_

#include "CharC.h"
#include "StringC.h"
#include <X11/Intrinsic.h>	// For Boolean
#include <fcntl.h>		// For O_RDONLY

//
// Structure to hold mapped file data
//

class MappedFileC {

public:

   StringC	name;
   int		mode;
   int		fd;
   StringC	*fakeBuffer;
   CharC	data;

    MappedFileC() {}
   ~MappedFileC() {}

   inline const char	*Addr()  const { return data.Addr();   }
   inline u_int		Len()    const { return data.Length(); }
   inline u_int		Length() const { return data.Length(); }
};

//
// Calls to map and unmap files
//
extern MappedFileC	*MapFile(const char*, int mode=O_RDONLY,
				 MappedFileC *mf=NULL);
extern Boolean		UnmapFile(MappedFileC*, Boolean del=True);
extern Boolean		RemapFile(MappedFileC*);

#endif // _MemMap_h_
