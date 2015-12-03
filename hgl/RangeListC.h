//
// RangeListC.h
//

#ifndef _RangeListC_h_
#define _RangeListC_h_

#include "RangeC.h"

/*
 * OListC.clas -- class definition for class OListC
 *
 *   This type of list store the members themselves
 *   PListC stores only pointers to the members
 */

#include <stream.h>

class RangeListC {

protected:

   RangeC		*_list;
   unsigned	_count;
   unsigned	_space;
   char		allow_duplicates;
   char		sorted;
   char		adjustGrowth;
   char		autoShrink;
   void		grow(unsigned size=0);
   void		shrink();

public:

   unsigned		GROWTH_AMOUNT;
   const int		NULL_INDEX;
   const unsigned	MAX_GROWTH;

   RangeListC();
   RangeListC(unsigned);
   RangeListC(const RangeListC&);
   virtual ~RangeListC();

// Append another list to this one

   RangeListC&	operator+=(const RangeListC&);

// Copy another list to this one

   RangeListC&	operator= (const RangeListC&);

// Return the position of the specified object.	 Object must be in list

   int		indexOf(const RangeC&) const;

// Return the sorted position of the specified object.	Object does not
//    have to be in the list

   unsigned	orderOf(const RangeC&) const;

//
// Return non-zero if the specified object is in the list
//
   int		includes(const RangeC&) const;

//
// Add the specified object to the list in the requested position
//    The position is ignored if this list is sorted
//
   RangeC		*insert (const RangeC&, unsigned);
   RangeC		*append (const RangeC&);
   RangeC		*prepend(const RangeC&);
   RangeC		*add    (const RangeC&);

//
// Remove the specified object or index from the list
//
   void		remove(unsigned);
   void		remove(const RangeC&);
   void		removeAll();
   void		removeLast();

//
// Return a pointer to the previous object in the list
//
   RangeC		*before(const RangeC&) const;

//
// Functions to support sorting
//
   static int	compare(const void*, const void*);
   void		sort();
   void		sort(int (*)(const void*, const void*));
   void		SetSorted(char);

   inline unsigned	size()		const { return _count; }
   inline unsigned	capacity()	const { return _space; }
   inline RangeC		*start()	const { return _list; }

   void		AllowDuplicates(char);
   void		AutoShrink(char);
   void		SetCapacity(unsigned);

   RangeC		*operator[](unsigned) const;

   RangeC		*First() const;
   RangeC		*Last() const;

   ostream&	printOn(ostream& strm=cout) const;
};

//
// PRINTING
//

inline ostream& operator<<(ostream& strm, const RangeListC& c)
{
   return c.printOn(strm);
}

#endif // _RangeListC_h_
