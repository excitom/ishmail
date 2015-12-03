//
// SortKeyListC.h
//

#ifndef _SortKeyListC_h_
#define _SortKeyListC_h_

#include "SortKeyC.h"

/*
 * PListC.clas -- class definition file for class PListC
 *
 *   This type of list stores only pointers to its members
 *   OListC stores the members themselves
 */

class SortKeyListC {

protected:

   SortKeyC			**_list;
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

   SortKeyListC();
   SortKeyListC(unsigned ga);
   SortKeyListC(const SortKeyListC&);
   ~SortKeyListC();

/* Append another list to this one */
   SortKeyListC& operator+=(const SortKeyListC&);

/* Copy another list to this one */
   SortKeyListC& operator=(const SortKeyListC&);

/* Return the position of the specified object.  Object must be in list */

   int		indexOf(const SortKeyC&) const;
   int		indexOf(const SortKeyC*) const;

/* Return the sorted position of the specified object.  Object does not */
/*    have to be in the list */

   unsigned		orderOf(const SortKeyC&) const;
   unsigned		orderOf(const SortKeyC*) const;

/* Return non-zero if the specified object is in the list */

   int		includes(const SortKeyC&) const;
   int		includes(const SortKeyC*) const;

/* Add the specified object to the list in the requested position */
/*    The position is ignored if this list is sorted */

   SortKeyC		*insert (const SortKeyC*, unsigned);
   SortKeyC		*replace(const SortKeyC*, unsigned);
   SortKeyC		*append (const SortKeyC*);
   SortKeyC		*prepend(const SortKeyC*);
   SortKeyC		*add    (const SortKeyC*);

/* Remove the specified object or index from the list */

   void		remove(unsigned);
   void		remove(const SortKeyC&);
   void		remove(const SortKeyC*);
   void		removeNulls();
   void		removeAll();
   void		removeLast();

/* Return a pointer to the previous object in the list */

   SortKeyC		*before(const SortKeyC*) const;

/* Functions to support sorting */

   static int	compare(const void*, const void*);
   void		sort();
   void		sort(int (*)(const void*, const void*));
   void		SetSorted(char);

   inline unsigned	size()		const { return _count; }
   inline unsigned	capacity()	const { return _space; }
   inline SortKeyC		**start()	const { return _list; }
   void         	AllowDuplicates(char);
   void			AutoShrink(char);
   void			SetCapacity(unsigned);

   SortKeyC			*operator[](unsigned) const;
   SortKeyC			*First() const;
   SortKeyC			*Last() const;

   ostream&		printOn(ostream& strm=cout) const;
};

/*-------------------------------------------------------------*/
/* PRINTING */

inline ostream&
operator<<(ostream& strm, const SortKeyListC& c)
{
   return c.printOn(strm);
}

#endif // _SortKeyListC_h_
