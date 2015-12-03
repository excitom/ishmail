//
// RowColChildListC.h
//

#ifndef _RowColChildListC_h_
#define _RowColChildListC_h_

#include "RowColChildC.h"

/*
 * PListC.clas -- class definition file for class PListC
 *
 *   This type of list stores only pointers to its members
 *   OListC stores the members themselves
 */

class RowColChildListC {

protected:

   RowColChildC			**_list;
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

   RowColChildListC();
   RowColChildListC(unsigned ga);
   RowColChildListC(const RowColChildListC&);
   ~RowColChildListC();

/* Append another list to this one */
   RowColChildListC& operator+=(const RowColChildListC&);

/* Copy another list to this one */
   RowColChildListC& operator=(const RowColChildListC&);

/* Return the position of the specified object.  Object must be in list */

   int		indexOf(const RowColChildC&) const;
   int		indexOf(const RowColChildC*) const;

/* Return the sorted position of the specified object.  Object does not */
/*    have to be in the list */

   unsigned		orderOf(const RowColChildC&) const;
   unsigned		orderOf(const RowColChildC*) const;

/* Return non-zero if the specified object is in the list */

   int		includes(const RowColChildC&) const;
   int		includes(const RowColChildC*) const;

/* Add the specified object to the list in the requested position */
/*    The position is ignored if this list is sorted */

   RowColChildC		*insert (const RowColChildC*, unsigned);
   RowColChildC		*replace(const RowColChildC*, unsigned);
   RowColChildC		*append (const RowColChildC*);
   RowColChildC		*prepend(const RowColChildC*);
   RowColChildC		*add    (const RowColChildC*);

/* Remove the specified object or index from the list */

   void		remove(unsigned);
   void		remove(const RowColChildC&);
   void		remove(const RowColChildC*);
   void		removeNulls();
   void		removeAll();
   void		removeLast();

/* Return a pointer to the previous object in the list */

   RowColChildC		*before(const RowColChildC*) const;

/* Functions to support sorting */

   static int	compare(const void*, const void*);
   void		sort();
   void		sort(int (*)(const void*, const void*));
   void		SetSorted(char);

   inline unsigned	size()		const { return _count; }
   inline unsigned	capacity()	const { return _space; }
   inline RowColChildC		**start()	const { return _list; }
   void         	AllowDuplicates(char);
   void			AutoShrink(char);
   void			SetCapacity(unsigned);

   RowColChildC			*operator[](unsigned) const;
   RowColChildC			*First() const;
   RowColChildC			*Last() const;

   ostream&		printOn(ostream& strm=cout) const;
};

/*-------------------------------------------------------------*/
/* PRINTING */

inline ostream&
operator<<(ostream& strm, const RowColChildListC& c)
{
   return c.printOn(strm);
}

#endif // _RowColChildListC_h_
