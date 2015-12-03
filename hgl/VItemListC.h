//
// VItemListC.h
//

#ifndef _VItemListC_h_
#define _VItemListC_h_

#include "VItemC.h"

/*
 * PListC.clas -- class definition file for class PListC
 *
 *   This type of list stores only pointers to its members
 *   OListC stores the members themselves
 */

class VItemListC {

protected:

   VItemC			**_list;
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

   VItemListC();
   VItemListC(unsigned ga);
   VItemListC(const VItemListC&);
   ~VItemListC();

/* Append another list to this one */
   VItemListC& operator+=(const VItemListC&);

/* Copy another list to this one */
   VItemListC& operator=(const VItemListC&);

/* Return the position of the specified object.  Object must be in list */

   int		indexOf(const VItemC&) const;
   int		indexOf(const VItemC*) const;

/* Return the sorted position of the specified object.  Object does not */
/*    have to be in the list */

   unsigned		orderOf(const VItemC&) const;
   unsigned		orderOf(const VItemC*) const;

/* Return non-zero if the specified object is in the list */

   int		includes(const VItemC&) const;
   int		includes(const VItemC*) const;

/* Add the specified object to the list in the requested position */
/*    The position is ignored if this list is sorted */

   VItemC		*insert (const VItemC*, unsigned);
   VItemC		*replace(const VItemC*, unsigned);
   VItemC		*append (const VItemC*);
   VItemC		*prepend(const VItemC*);
   VItemC		*add    (const VItemC*);

/* Remove the specified object or index from the list */

   void		remove(unsigned);
   void		remove(const VItemC&);
   void		remove(const VItemC*);
   void		removeNulls();
   void		removeAll();
   void		removeLast();

/* Return a pointer to the previous object in the list */

   VItemC		*before(const VItemC*) const;

/* Functions to support sorting */

   static int	compare(const void*, const void*);
   void		sort();
   void		sort(int (*)(const void*, const void*));
   void		SetSorted(char);

   inline unsigned	size()		const { return _count; }
   inline unsigned	capacity()	const { return _space; }
   inline VItemC		**start()	const { return _list; }
   void         	AllowDuplicates(char);
   void			AutoShrink(char);
   void			SetCapacity(unsigned);

   VItemC			*operator[](unsigned) const;
   VItemC			*First() const;
   VItemC			*Last() const;

   ostream&		printOn(ostream& strm=cout) const;
};

/*-------------------------------------------------------------*/
/* PRINTING */

inline ostream&
operator<<(ostream& strm, const VItemListC& c)
{
   return c.printOn(strm);
}

#endif // _VItemListC_h_
