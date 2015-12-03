//
// ExpElemListC.h
//

#ifndef _ExpElemListC_h_
#define _ExpElemListC_h_

#include "ExpElemC.h"

/*
 * PListC.clas -- class definition file for class PListC
 *
 *   This type of list stores only pointers to its members
 *   OListC stores the members themselves
 */

class ExpElemListC {

protected:

   ExpElemC			**_list;
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

   ExpElemListC();
   ExpElemListC(unsigned ga);
   ExpElemListC(const ExpElemListC&);
   ~ExpElemListC();

/* Append another list to this one */
   ExpElemListC& operator+=(const ExpElemListC&);

/* Copy another list to this one */
   ExpElemListC& operator=(const ExpElemListC&);

/* Return the position of the specified object.  Object must be in list */

   int		indexOf(const ExpElemC&) const;
   int		indexOf(const ExpElemC*) const;

/* Return the sorted position of the specified object.  Object does not */
/*    have to be in the list */

   unsigned		orderOf(const ExpElemC&) const;
   unsigned		orderOf(const ExpElemC*) const;

/* Return non-zero if the specified object is in the list */

   int		includes(const ExpElemC&) const;
   int		includes(const ExpElemC*) const;

/* Add the specified object to the list in the requested position */
/*    The position is ignored if this list is sorted */

   ExpElemC		*insert (const ExpElemC*, unsigned);
   ExpElemC		*replace(const ExpElemC*, unsigned);
   ExpElemC		*append (const ExpElemC*);
   ExpElemC		*prepend(const ExpElemC*);
   ExpElemC		*add    (const ExpElemC*);

/* Remove the specified object or index from the list */

   void		remove(unsigned);
   void		remove(const ExpElemC&);
   void		remove(const ExpElemC*);
   void		removeNulls();
   void		removeAll();
   void		removeLast();

/* Return a pointer to the previous object in the list */

   ExpElemC		*before(const ExpElemC*) const;

/* Functions to support sorting */

   static int	compare(const void*, const void*);
   void		sort();
   void		sort(int (*)(const void*, const void*));
   void		SetSorted(char);

   inline unsigned	size()		const { return _count; }
   inline unsigned	capacity()	const { return _space; }
   inline ExpElemC		**start()	const { return _list; }
   void         	AllowDuplicates(char);
   void			AutoShrink(char);
   void			SetCapacity(unsigned);

   ExpElemC			*operator[](unsigned) const;
   ExpElemC			*First() const;
   ExpElemC			*Last() const;

   ostream&		printOn(ostream& strm=cout) const;
};

/*-------------------------------------------------------------*/
/* PRINTING */

inline ostream&
operator<<(ostream& strm, const ExpElemListC& c)
{
   return c.printOn(strm);
}

#endif // _ExpElemListC_h_
