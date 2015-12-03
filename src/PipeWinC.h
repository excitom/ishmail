/*
 * $Id: PipeWinC.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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

#ifndef _PipeWinC_h_
#define _PipeWinC_h_

#include <hgl/HalDialogC.h>
#include <hgl/StringC.h>

class PipeWinC;

typedef struct {

   pid_t	pid;
   int		msgnum;
   PipeWinC	*win;
   char		*tmpFile;
   StringC	cmdStr;
   int		status;

} PipeProcT;

class VItemListC;
class MsgC;

class PipeWinC : public HalDialogC {

//
// Widgets
//
   Widget		hdrFrame;
   Widget		hdrAllTB;
   Widget		hdrDispTB;
   Widget		hdrNoneTB;
   Widget		ordFrame;
   Widget		ordParallelTB;
   Widget		ordSerialTB;
   Widget		cmdTF;

//
// Callbacks
//
   static void		DoPipe(Widget, PipeWinC*, XtPointer);
   static void		ForkDone(int, PipeProcT*);

public:

   HalShellC		*errorShell;

// Methods

   PipeWinC(Widget);
   ~PipeWinC();

   void			ShowOrder();
   void			HideOrder();
   Boolean		PipeParallel(VItemListC&, StringC&);
   Boolean		PipeSerial(VItemListC&, StringC&);
   Boolean		Pipe(MsgC*, StringC&);
   Boolean		PipeFile(char*, StringC&, int msgnum=-1);
};

#endif // _PipeWinC_h_
