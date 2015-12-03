//
// FieldListC.h
//

#ifndef _FieldListC_h_
#define _FieldListC_h_

#include "FieldC.h"

/*
 * PListC.clas -- class definition file for class PListC
 *
 *   This type of list stores only pointers to its members
 *   OListC stores the members themselves
 */

class FieldListC {

protected:

   FieldC			**_list;
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

   FieldListC();
   FieldListC(unsigned ga);
   FieldListC(const FieldListC&);
   ~FieldListC();

/* Append another list to this one */
   FieldListC& operator+=(const FieldListC&);

/* Copy another list to this one */
   FieldListC& operator=(const FieldListC&);

/* Return the position of the specified object.  Object must be in list */

   int		indexOf(const FieldC&) const;
   int		indexOf(const FieldC*) const;

/* Return the sorted position of the specified object.  Object does not */
/*    have to be in the list */

   unsigned		orderOf(const FieldC&) const;
   unsigned		orderOf(const FieldC*) const;

/* Return non-zero if the specified object is in the list */

   int		includes(const FieldC&) const;
   int		includes(const FieldC*) const;

/* Add the specified object to the list in the requested position */
/*    The position is ignored if this list is sorted */

   FieldC		*insert (const FieldC*, unsigned);
   FieldC		*replace(const FieldC*, unsigned);
   FieldC		*append (const FieldC*);
   FieldC		*prepend(const FieldC*);
   FieldC		*add    (const FieldC*);

/* Remove the specified object or index from the list */

   void		remove(unsigned);
   void		remove(const FieldC&);
   void		remove(const FieldC*);
   void		removeNulls();
   void		removeAll();
   void		removeLast();

/* Return a pointer to the previous object in the list */

   FieldC		*before(const FieldC*) const;

/* Functions to support sorting */

   static int	compare(const void*, const void*);
   void		sort();
   void		sort(int (*)(const void*, const void*));
   void		SetSorted(char);

   inline unsigned	size()		const { return _count; }
   inline unsigned	capacity()	const { return _space; }
   inline FieldC		**start()	const { return _list; }
   void         	AllowDuplicates(char);
   void			AutoShrink(char);
   void			SetCapacity(unsigned);

   FieldC			*operator[](unsigned) const;
   FieldC			*First() const;
   FieldC			*Last() const;

   ostream&		printOn(ostream& strm=cout) const;
};

/*-------------------------------------------------------------*/
/* PRINTING */

inline ostream&
operator<<(ostream& strm, const FieldListC& c)
{
   return c.printOn(strm);
}

#endif // _FieldListC_h_
