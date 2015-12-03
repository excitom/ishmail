//
// StringListC.h
//

#ifndef _StringListC_h_
#define _StringListC_h_

#include "StringC.h"

/*
 * OListC.clas -- class definition for class OListC
 *
 *   This type of list store the members themselves
 *   PListC stores only pointers to the members
 */

#include <stream.h>

class StringListC {

protected:

   StringC		*_list;
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

   StringListC();
   StringListC(unsigned);
   StringListC(const StringListC&);
   virtual ~StringListC();

// Append another list to this one

   StringListC&	operator+=(const StringListC&);

// Copy another list to this one

   StringListC&	operator= (const StringListC&);

// Return the position of the specified object.	 Object must be in list

   int		indexOf(const StringC&) const;

// Return the sorted position of the specified object.	Object does not
//    have to be in the list

   unsigned	orderOf(const StringC&) const;

//
// Return non-zero if the specified object is in the list
//
   int		includes(const StringC&) const;

//
// Add the specified object to the list in the requested position
//    The position is ignored if this list is sorted
//
   StringC		*insert (const StringC&, unsigned);
   StringC		*append (const StringC&);
   StringC		*prepend(const StringC&);
   StringC		*add    (const StringC&);

//
// Remove the specified object or index from the list
//
   void		remove(unsigned);
   void		remove(const StringC&);
   void		removeAll();
   void		removeLast();

//
// Return a pointer to the previous object in the list
//
   StringC		*before(const StringC&) const;

//
// Functions to support sorting
//
   static int	compare(const void*, const void*);
   void		sort();
   void		sort(int (*)(const void*, const void*));
   void		SetSorted(char);

   inline unsigned	size()		const { return _count; }
   inline unsigned	capacity()	const { return _space; }
   inline StringC		*start()	const { return _list; }

   void		AllowDuplicates(char);
   void		AutoShrink(char);
   void		SetCapacity(unsigned);

   StringC		*operator[](unsigned) const;

   StringC		*First() const;
   StringC		*Last() const;

   ostream&	printOn(ostream& strm=cout) const;
};

//
// PRINTING
//

inline ostream& operator<<(ostream& strm, const StringListC& c)
{
   return c.printOn(strm);
}

#endif // _StringListC_h_
