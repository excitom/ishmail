/*
 * $Id: BoolExpC.C,v 1.2 2000/05/07 12:26:10 fnevgeny Exp $
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

#include <config.h>

#include "BoolExpC.h"

#include <Xm/Label.h>

/*-------------------------------------------------------------------------
 * Binary expression constructor taking two expressions and an operator
 */

BoolExpC::BoolExpC(BoolExpC *a, BoolOpT o, BoolExpC *b)
{
   expA = a;
   expB = b;
   op   = o;
   term = NULL;
   type = BINARY;
}

/*-------------------------------------------------------------------------
 * Unary expression constructor taking one expressions and an operator
 */

BoolExpC::BoolExpC(BoolExpC::BoolOpT o, BoolExpC *b)
{
   expA = NULL;
   expB = b;
   op   = o;
   term = NULL;
   type = UNARY;
}

/*-------------------------------------------------------------------------
 * Terminal expression constructor
 */

BoolExpC::BoolExpC(TermExpC *t)
{
   expA = NULL;
   expB = NULL;
   term = t;
   type = TERMINAL;
}

/*-------------------------------------------------------------------------
 * Expression destructor
 */

BoolExpC::~BoolExpC()
{
   delete expA;
   delete expB;
}

/*-------------------------------------------------------------------------
 * Expression evalator
 */

Boolean
BoolExpC::Match(void *subject)
{
   switch (type) {
      case (BINARY):
	 switch (op) {
	    case (AND): return expA->Match(subject) && expB->Match(subject);
	    case (OR ): return expA->Match(subject) || expB->Match(subject);
	    default: break;
	 }
	 break;
      case (UNARY):
	 switch (op) {
	    case (NOT): return !expB->Match(subject);
	    default: break;
	 }
	 break;
      default:
	 return term->Match(subject);
   }

   return False;

} // End BoolExpC Match

/*-------------------------------------------------------------------------
 * Built-in terminals
 */

Widget
TrueExpC::CreateWidget(Widget parent, ArgList argv, Cardinal argc)
{
   return XmCreateLabel(parent, "trueLabel", argv, argc);
}

Widget
FalseExpC::CreateWidget(Widget parent, ArgList argv, Cardinal argc)
{
   return XmCreateLabel(parent, "falseLabel", argv, argc);
}
