/*
 * $Id: MatrixC.h,v 1.1.1.1 2000/04/25 13:49:01 fnevgeny Exp $
 *
 * Copyright (c) 1992 HaL Computer Systems, Inc.  All rights reserved.
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

#ifndef _MatrixC_h_
#define _MatrixC_h_

#include "Base.h"

#include <math.h>

#define DEG_TO_RAD	(3.14159265358979323846264338327950288/180.0)

/*
 *  This is a 3x3 matrix used to perform a 2d transformation.  The third
 *     column is always [0,0,1] and is ignored in these calculations.
 *
 *  	xx	yx	0
 *	xy	yy	0
 *	xt	yt	1
 */

class MatrixC {

   float	xx, yx;
   float	xy, yy;
   float	xt, yt;
   int		Round(float) const;

public:

   MatrixC();
   MatrixC(const MatrixC&);
   ~MatrixC() {}

   void 	Identity();

// Compute and return inverse matrix
   MatrixC	Inverse();

// Invert this matrix in place
   void		Invert();

// Add a rotation transformation to this matrix
   void		Rotate(float);
   void		Rotate(int);
   void		Rotate(long);

// Add a scale transformation to this matrix
   void		Scale(float, float);
   void		Scale(int, int);
   void		Scale(long, long);

// Add a translate transformation to this matrix
   void		Translate(float, float);
   void		Translate(int, int);
   void		Translate(long, long);

// Transform coords in place
   void		Transform(int *, int *) const;
   void		Transform(long *, long *) const;
   void		Transform(float *, float *) const;

// Transform a float value to an int
   int		TransformX(float) const;
   int		TransformY(float) const;

// Transform an int value in place
   void		TransformX(int *) const;
   void		TransformY(int *) const;

// Copy one matrix to another
   MatrixC&	operator=(const MatrixC&);

// Postmultiply another matrix onto this one
   void		operator*=(const MatrixC&);

// Print matrix
   void		printOn(ostream& strm) const;
};

inline void
MatrixC::Identity()
{
   xx = yy = 1.0;
   yx = xy = xt = yt = 0.0;
}

inline
MatrixC::MatrixC()
{
   Identity();
}

inline int
MatrixC::Round(float f) const
{
   return (int)((f > 0.0) ? (f + 0.5) : (f - 0.5));
}

inline void
MatrixC::Rotate(float angle)
{
   float	r = angle * DEG_TO_RAD;
   float	s = sin(r);
   float	c = cos(r);

   float	nxx, nxy, nxt;
   float	nyx, nyy, nyt;

   nxx = xx * c - yx * s;
   nxy = xy * c - yy * s;
   nxt = xt * c - yt * s;
   nyx = xx * s + yx * c;
   nyy = xy * s + yy * c;
   nyt = xt * s + yt * c;

   xx = nxx;
   xy = nxy;
   xt = nxt;
   yx = nyx;
   yy = nyy;
   yt = nyt;
}

inline void
MatrixC::Rotate(int a)
{
   Rotate((float)a);
}

inline void
MatrixC::Rotate(long a)
{
   Rotate((float)a);
}

inline void
MatrixC::Scale(float x, float y)
{
   xx *= x;
   xy *= x;
   xt *= x;
   yx *= y;
   yy *= y;
   yt *= y;
}

inline void
MatrixC::Scale(int x, int y)
{
   Scale((float)x, (float)y);
}

inline void
MatrixC::Scale(long x, long y)
{
   Scale((float)x, (float)y);
}

inline void
MatrixC::Translate(float x, float y)
{
   xt += x;
   yt += y;
}

inline void
MatrixC::Translate(int x, int y)
{
   Translate((float)x, (float)y);
}

inline void
MatrixC::Translate(long x, long y)
{
   Translate((float)x, (float)y);
}

inline void
MatrixC::Transform(float *x, float *y) const
{
   float	nx, ny;
   nx = *x * xx + *y * xy + xt;
   ny = *x * yx + *y * yy + yt;
   *x = nx;
   *y = ny;
}

inline void
MatrixC::Transform(int *x, int *y) const
{
   float	nx = *x;
   float	ny = *y;
   Transform(&nx, &ny);
   *x = Round(nx);
   *y = Round(ny);
}

inline void
MatrixC::Transform(long *x, long *y) const
{
   float	nx = *x;
   float	ny = *y;
   Transform(&nx, &ny);
   *x = Round(nx);
   *y = Round(ny);
}

inline int
MatrixC::TransformX(float x) const
{
   float	nx;
   nx = x * xx + xt;
   return Round(nx);
}

inline int
MatrixC::TransformY(float y) const
{
   float	ny;
   ny = y * yy + yt;
   return Round(ny);
}

inline void
MatrixC::TransformX(int *x) const
{
   float	nx;
   nx = *x * xx + xt;
   *x = Round(nx);
}

inline void
MatrixC::TransformY(int *y) const
{
   float	ny;
   ny = *y * yy + yt;
   *y = Round(ny);
}

inline MatrixC&
MatrixC::operator=(const MatrixC& m)
{
   if ( this != &m ) {
      xx = m.xx;
      xy = m.xy;
      yx = m.yx;
      yy = m.yy;
      xt = m.xt;
      yt = m.yt;
   }
   return *this;
}

inline
MatrixC::MatrixC(const MatrixC& m)
{
   *this = m;
}

inline void
MatrixC::operator*=(const MatrixC& m)
{
   MatrixC	o;
   o.xx = xx * m.xx + yx * m.xy;
   o.yx = xx * m.yx + yx * m.yy;
   o.xy = xy * m.xx + yy * m.xy;
   o.yy = xy * m.yx + yy * m.yy;
   o.xt = xt * m.xx + yt * m.xy + m.xt;
   o.yt = xt * m.yx + yt * m.yy + m.yt;

   *this = o;
}

inline MatrixC
MatrixC::Inverse()
{
   float	det;	// Determinant of upper left 2x2
   det = xx * yy - xy * yx;
   if ( det != 0.0 ) det = 1.0 / det;

   MatrixC	inv;
   inv.xx =  yy * det;
   inv.xy = -xy * det;
   inv.xt = (xy * yt - xt * yy) * det;

   inv.yx = -yx * det;
   inv.yy =  xx * det;
   inv.yt = (xt * yx - xx * yt) * det;

   return (inv);
}

inline void
MatrixC::Invert()
{
   *this = Inverse();
}

inline void
MatrixC::printOn(ostream& strm=cout) const {
   strm << xx SP yx SP 0 NL;
   strm << xy SP yy SP 0 NL;
   strm << xt SP yt SP 1 NL;
}

inline ostream&
operator<<(ostream& strm, const MatrixC& m)
{
   m.printOn(strm);
   return(strm);
}

#endif // _MatrixC_h_
