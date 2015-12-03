/*
 * $Id: MsgFindExp.h,v 1.1.1.1 2000/04/25 13:49:02 fnevgeny Exp $
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

#ifndef _MsgFindExp_h_
#define _MsgFindExp_h_

#include "MsgStatus.h"

#include <hgl/BoolExpC.h>
#include <hgl/RegexC.h>

/*--------------------------------------------------------------------------
 * Terminal expression classes
 */

class MsgFromExpC : public TermExpC {

   RegexC	pat;

public:

   MsgFromExpC(const char*);

   Widget	CreateWidget(Widget, ArgList argv=NULL, Cardinal argc=0);
   Boolean	Match(void *);
};

//--------------------------------------------------------------------------

class MsgToExpC : public TermExpC {

   RegexC	pat;

public:

   MsgToExpC(const char*);

   Widget	CreateWidget(Widget, ArgList argv=NULL, Cardinal argc=0);
   Boolean	Match(void *);
};

//--------------------------------------------------------------------------

class MsgSubjExpC : public TermExpC {

   RegexC	pat;

public:

   MsgSubjExpC(const char*);

   Widget	CreateWidget(Widget, ArgList argv=NULL, Cardinal argc=0);
   Boolean	Match(void *);
};

//--------------------------------------------------------------------------

class MsgHeadExpC : public TermExpC {

   RegexC	pat;

public:

   MsgHeadExpC(const char*);

   Widget	CreateWidget(Widget, ArgList argv=NULL, Cardinal argc=0);
   Boolean	Match(void *);
};

//--------------------------------------------------------------------------

class MsgBodyExpC : public TermExpC {

   RegexC	pat;

public:

   MsgBodyExpC(const char*);

   Widget	CreateWidget(Widget, ArgList argv=NULL, Cardinal argc=0);
   Boolean	Match(void *);
};

//--------------------------------------------------------------------------

class MsgMsgExpC : public TermExpC {

   RegexC	pat;

public:

   MsgMsgExpC(const char*);

   Widget	CreateWidget(Widget, ArgList argv=NULL, Cardinal argc=0);
   Boolean	Match(void *);
};

//--------------------------------------------------------------------------

class MsgDateExpC : public TermExpC {

public:

   enum MsgDateOp { LT, LE, EQ, GE, GT, NE };

private:

   MsgDateOp	op;
   StringC	dateStr;
   time_t	val;

public:

   MsgDateExpC(MsgDateOp, const char*);

   Widget	CreateWidget(Widget, ArgList argv=NULL, Cardinal argc=0);
   Boolean	Match(void *);
};

//--------------------------------------------------------------------------

class MsgStatExpC : public TermExpC {

public:

private:

   MsgStatusT	stat;

public:

   MsgStatExpC(MsgStatusT);

   Widget	CreateWidget(Widget, ArgList argv=NULL, Cardinal argc=0);
   Boolean	Match(void *);
};

//--------------------------------------------------------------------------

class MsgNumExpC : public TermExpC {

public:

   enum MsgNumOp { LT, LE, EQ, GE, GT, NE };

private:

   MsgNumOp	op;
   unsigned	val;

public:

   MsgNumExpC(MsgNumOp, unsigned);

   Widget	CreateWidget(Widget, ArgList argv=NULL, Cardinal argc=0);
   Boolean	Match(void *);
};

//--------------------------------------------------------------------------

class MsgLineExpC : public TermExpC {

public:

   enum MsgLineOp { LT, LE, EQ, GE, GT, NE };

private:

   MsgLineOp	op;
   unsigned	val;

public:

   MsgLineExpC(MsgLineOp, unsigned);

   Widget	CreateWidget(Widget, ArgList argv=NULL, Cardinal argc=0);
   Boolean	Match(void *);
};

//--------------------------------------------------------------------------

class MsgByteExpC : public TermExpC {

public:

   enum MsgByteOp { LT, LE, EQ, GE, GT, NE };

private:

   MsgByteOp	op;
   unsigned	val;

public:

   MsgByteExpC(MsgByteOp, unsigned);

   Widget	CreateWidget(Widget, ArgList argv=NULL, Cardinal argc=0);
   Boolean	Match(void *);
};

#endif // _MsgFindExp_h_
