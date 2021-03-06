/*
 * $Id: BoolExpC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
 *
 * Copyright (c) 1992 HAL Computer Systems International, Ltd.
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

#ifndef _BoolExpC_h_
#define _BoolExpC_h_

#include <X11/Intrinsic.h>

class TermExpC {
public:
   virtual Boolean	Match(void*) = 0;
   virtual Widget	CreateWidget(Widget parent, ArgList argv=NULL,
				     Cardinal argc=0) = 0;
};

//----------------------------------------------------------------------
// Built-in terminal that always returns True

class TrueExpC : public TermExpC {
   inline Boolean	Match(void*) { return True; }
   Widget	CreateWidget(Widget, ArgList argv=NULL, Cardinal argc=0);
};

//----------------------------------------------------------------------
// Built-in terminal that always returns False

class FalseExpC : public TermExpC {
   inline Boolean	Match(void*) { return False; }
   Widget	CreateWidget(Widget, ArgList argv=NULL, Cardinal argc=0);
};

//----------------------------------------------------------------------

class BoolExpC {

public:

   enum BoolOpT {
      AND,
      OR,
      NOT
   };

private:

   enum BoolExpT {
      BINARY,
      UNARY,
      TERMINAL
   };

   BoolExpT		type;
   BoolOpT		op;
   BoolExpC		*expA;
   BoolExpC		*expB;
   TermExpC		*term;

public:

   BoolExpC(BoolExpC*, BoolOpT, BoolExpC*);
   BoolExpC(BoolOpT, BoolExpC*);
   BoolExpC(TermExpC*);
   ~BoolExpC();

   Boolean	Match(void*);
};

#endif // _BoolExpC_h_
