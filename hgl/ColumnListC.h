//
// ColumnListC.h
//

#ifndef _ColumnListC_h_
#define _ColumnListC_h_

#include "ColumnC.h"

/*
 * PListC.clas -- class definition file for class PListC
 *
 *   This type of list stores only pointers to its members
 *   OListC stores the members themselves
 */

class ColumnListC {

protected:

   ColumnC			**_list;
   unsigned		_count;
   unsigned		_space;
   char			allow_duplicates;
   char			sorted;
   char         	adjustGrowth;
   char         	autoShrink;
   void			grow(unsigned size=0);
   void			shrink();

public:

   unsigned		GROWTH_AMOUNT;
   const int		NULL_INDEX;
   const unsigned	MAX_GROWTH;

   ColumnListC();
   ColumnListC(unsigned ga);
   ColumnListC(const ColumnListC&);
   ~ColumnListC();

/* Append another list to this one */
   ColumnListC& operator+=(const ColumnListC&);

/* Copy another list to this one */
   ColumnListC& operator=(const ColumnListC&);

/* Return the position of the specified object.  Object must be in list */

   int		indexOf(const ColumnC&) const;
   int		indexOf(const ColumnC*) const;

/* Return the sorted position of the specified object.  Object does not */
/*    have to be in the list */

   unsigned		orderOf(const ColumnC&) const;
   unsigned		orderOf(const ColumnC*) const;

/* Return non-zero if the specified object is in the list */

   int		includes(const ColumnC&) const;
   int		includes(const ColumnC*) const;

/* Add the specified object to the list in the requested position */
/*    The position is ignored if this list is sorted */

   ColumnC		*insert (const ColumnC*, unsigned);
   ColumnC		*replace(const ColumnC*, unsigned);
   ColumnC		*append (const ColumnC*);
   ColumnC		*prepend(const ColumnC*);
   ColumnC		*add    (const ColumnC*);

/* Remove the specified object or index from the list */

   void		remove(unsigned);
   void		remove(const ColumnC&);
   void		remove(const ColumnC*);
   void		removeNulls();
   void		removeAll();
   void		removeLast();

/* Return a pointer to the previous object in the list */

   ColumnC		*before(const ColumnC*) const;

/* Functions to support sorting */

   static int	compare(const void*, const void*);
   void		sort();
   void		sort(int (*)(const void*, const void*));
   void		SetSorted(char);

   inline unsigned	size()		const { return _count; }
   inline unsigned	capacity()	const { return _space; }
   inline ColumnC		**start()	const { return _list; }
   void         	AllowDuplicates(char);
   void			AutoShrink(char);
   void			SetCapacity(unsigned);

   ColumnC			*operator[](unsigned) const;
   ColumnC			*First() const;
   ColumnC			*Last() const;

   ostream&		printOn(ostream& strm=cout) const;
};

/*-------------------------------------------------------------*/
/* PRINTING */

inline ostream&
operator<<(ostream& strm, const ColumnListC& c)
{
   return c.printOn(strm);
}

#endif // _ColumnListC_h_
