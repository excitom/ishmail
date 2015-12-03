//
// RowOrColListC.h
//

#ifndef _RowOrColListC_h_
#define _RowOrColListC_h_

#include "RowOrColC.h"

/*
 * PListC.clas -- class definition file for class PListC
 *
 *   This type of list stores only pointers to its members
 *   OListC stores the members themselves
 */

class RowOrColListC {

protected:

   RowOrColC			**_list;
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

   RowOrColListC();
   RowOrColListC(unsigned ga);
   RowOrColListC(const RowOrColListC&);
   ~RowOrColListC();

/* Append another list to this one */
   RowOrColListC& operator+=(const RowOrColListC&);

/* Copy another list to this one */
   RowOrColListC& operator=(const RowOrColListC&);

/* Return the position of the specified object.  Object must be in list */

   int		indexOf(const RowOrColC&) const;
   int		indexOf(const RowOrColC*) const;

/* Return the sorted position of the specified object.  Object does not */
/*    have to be in the list */

   unsigned		orderOf(const RowOrColC&) const;
   unsigned		orderOf(const RowOrColC*) const;

/* Return non-zero if the specified object is in the list */

   int		includes(const RowOrColC&) const;
   int		includes(const RowOrColC*) const;

/* Add the specified object to the list in the requested position */
/*    The position is ignored if this list is sorted */

   RowOrColC		*insert (const RowOrColC*, unsigned);
   RowOrColC		*replace(const RowOrColC*, unsigned);
   RowOrColC		*append (const RowOrColC*);
   RowOrColC		*prepend(const RowOrColC*);
   RowOrColC		*add    (const RowOrColC*);

/* Remove the specified object or index from the list */

   void		remove(unsigned);
   void		remove(const RowOrColC&);
   void		remove(const RowOrColC*);
   void		removeNulls();
   void		removeAll();
   void		removeLast();

/* Return a pointer to the previous object in the list */

   RowOrColC		*before(const RowOrColC*) const;

/* Functions to support sorting */

   static int	compare(const void*, const void*);
   void		sort();
   void		sort(int (*)(const void*, const void*));
   void		SetSorted(char);

   inline unsigned	size()		const { return _count; }
   inline unsigned	capacity()	const { return _space; }
   inline RowOrColC		**start()	const { return _list; }
   void         	AllowDuplicates(char);
   void			AutoShrink(char);
   void			SetCapacity(unsigned);

   RowOrColC			*operator[](unsigned) const;
   RowOrColC			*First() const;
   RowOrColC			*Last() const;

   ostream&		printOn(ostream& strm=cout) const;
};

/*-------------------------------------------------------------*/
/* PRINTING */

inline ostream&
operator<<(ostream& strm, const RowOrColListC& c)
{
   return c.printOn(strm);
}

#endif // _RowOrColListC_h_
