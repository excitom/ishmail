//
// SortElemListC.h
//

#ifndef _SortElemListC_h_
#define _SortElemListC_h_

#include "SortElemC.h"

/*
 * PListC.clas -- class definition file for class PListC
 *
 *   This type of list stores only pointers to its members
 *   OListC stores the members themselves
 */

class SortElemListC {

protected:

   SortElemC			**_list;
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

   SortElemListC();
   SortElemListC(unsigned ga);
   SortElemListC(const SortElemListC&);
   ~SortElemListC();

/* Append another list to this one */
   SortElemListC& operator+=(const SortElemListC&);

/* Copy another list to this one */
   SortElemListC& operator=(const SortElemListC&);

/* Return the position of the specified object.  Object must be in list */

   int		indexOf(const SortElemC&) const;
   int		indexOf(const SortElemC*) const;

/* Return the sorted position of the specified object.  Object does not */
/*    have to be in the list */

   unsigned		orderOf(const SortElemC&) const;
   unsigned		orderOf(const SortElemC*) const;

/* Return non-zero if the specified object is in the list */

   int		includes(const SortElemC&) const;
   int		includes(const SortElemC*) const;

/* Add the specified object to the list in the requested position */
/*    The position is ignored if this list is sorted */

   SortElemC		*insert (const SortElemC*, unsigned);
   SortElemC		*replace(const SortElemC*, unsigned);
   SortElemC		*append (const SortElemC*);
   SortElemC		*prepend(const SortElemC*);
   SortElemC		*add    (const SortElemC*);

/* Remove the specified object or index from the list */

   void		remove(unsigned);
   void		remove(const SortElemC&);
   void		remove(const SortElemC*);
   void		removeNulls();
   void		removeAll();
   void		removeLast();

/* Return a pointer to the previous object in the list */

   SortElemC		*before(const SortElemC*) const;

/* Functions to support sorting */

   static int	compare(const void*, const void*);
   void		sort();
   void		sort(int (*)(const void*, const void*));
   void		SetSorted(char);

   inline unsigned	size()		const { return _count; }
   inline unsigned	capacity()	const { return _space; }
   inline SortElemC		**start()	const { return _list; }
   void         	AllowDuplicates(char);
   void			AutoShrink(char);
   void			SetCapacity(unsigned);

   SortElemC			*operator[](unsigned) const;
   SortElemC			*First() const;
   SortElemC			*Last() const;

   ostream&		printOn(ostream& strm=cout) const;
};

/*-------------------------------------------------------------*/
/* PRINTING */

inline ostream&
operator<<(ostream& strm, const SortElemListC& c)
{
   return c.printOn(strm);
}

#endif // _SortElemListC_h_
