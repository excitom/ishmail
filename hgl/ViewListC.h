//
// ViewListC.h
//

#ifndef _ViewListC_h_
#define _ViewListC_h_

#include "ViewC.h"

/*
 * PListC.clas -- class definition file for class PListC
 *
 *   This type of list stores only pointers to its members
 *   OListC stores the members themselves
 */

class ViewListC {

protected:

   ViewC			**_list;
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

   ViewListC();
   ViewListC(unsigned ga);
   ViewListC(const ViewListC&);
   ~ViewListC();

/* Append another list to this one */
   ViewListC& operator+=(const ViewListC&);

/* Copy another list to this one */
   ViewListC& operator=(const ViewListC&);

/* Return the position of the specified object.  Object must be in list */

   int		indexOf(const ViewC&) const;
   int		indexOf(const ViewC*) const;

/* Return the sorted position of the specified object.  Object does not */
/*    have to be in the list */

   unsigned		orderOf(const ViewC&) const;
   unsigned		orderOf(const ViewC*) const;

/* Return non-zero if the specified object is in the list */

   int		includes(const ViewC&) const;
   int		includes(const ViewC*) const;

/* Add the specified object to the list in the requested position */
/*    The position is ignored if this list is sorted */

   ViewC		*insert (const ViewC*, unsigned);
   ViewC		*replace(const ViewC*, unsigned);
   ViewC		*append (const ViewC*);
   ViewC		*prepend(const ViewC*);
   ViewC		*add    (const ViewC*);

/* Remove the specified object or index from the list */

   void		remove(unsigned);
   void		remove(const ViewC&);
   void		remove(const ViewC*);
   void		removeNulls();
   void		removeAll();
   void		removeLast();

/* Return a pointer to the previous object in the list */

   ViewC		*before(const ViewC*) const;

/* Functions to support sorting */

   static int	compare(const void*, const void*);
   void		sort();
   void		sort(int (*)(const void*, const void*));
   void		SetSorted(char);

   inline unsigned	size()		const { return _count; }
   inline unsigned	capacity()	const { return _space; }
   inline ViewC		**start()	const { return _list; }
   void         	AllowDuplicates(char);
   void			AutoShrink(char);
   void			SetCapacity(unsigned);

   ViewC			*operator[](unsigned) const;
   ViewC			*First() const;
   ViewC			*Last() const;

   ostream&		printOn(ostream& strm=cout) const;
};

/*-------------------------------------------------------------*/
/* PRINTING */

inline ostream&
operator<<(ostream& strm, const ViewListC& c)
{
   return c.printOn(strm);
}

#endif // _ViewListC_h_
