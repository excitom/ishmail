//
// PtrListC.h
//

#ifndef _PtrListC_h_
#define _PtrListC_h_

#include "PtrT.h"

/*
 * OListC.clas -- class definition for class OListC
 *
 *   This type of list store the members themselves
 *   PListC stores only pointers to the members
 */

#include <stream.h>

class PtrListC {

protected:

   PtrT		*_list;
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

   PtrListC();
   PtrListC(unsigned);
   PtrListC(const PtrListC&);
   virtual ~PtrListC();

// Append another list to this one

   PtrListC&	operator+=(const PtrListC&);

// Copy another list to this one

   PtrListC&	operator= (const PtrListC&);

// Return the position of the specified object.	 Object must be in list

   int		indexOf(const PtrT&) const;

// Return the sorted position of the specified object.	Object does not
//    have to be in the list

   unsigned	orderOf(const PtrT&) const;

//
// Return non-zero if the specified object is in the list
//
   int		includes(const PtrT&) const;

//
// Add the specified object to the list in the requested position
//    The position is ignored if this list is sorted
//
   PtrT		*insert (const PtrT&, unsigned);
   PtrT		*append (const PtrT&);
   PtrT		*prepend(const PtrT&);
   PtrT		*add    (const PtrT&);

//
// Remove the specified object or index from the list
//
   void		remove(unsigned);
   void		remove(const PtrT&);
   void		removeAll();
   void		removeLast();

//
// Return a pointer to the previous object in the list
//
   PtrT		*before(const PtrT&) const;

//
// Functions to support sorting
//
   static int	compare(const void*, const void*);
   void		sort();
   void		sort(int (*)(const void*, const void*));
   void		SetSorted(char);

   inline unsigned	size()		const { return _count; }
   inline unsigned	capacity()	const { return _space; }
   inline PtrT		*start()	const { return _list; }

   void		AllowDuplicates(char);
   void		AutoShrink(char);
   void		SetCapacity(unsigned);

   PtrT		*operator[](unsigned) const;

   PtrT		*First() const;
   PtrT		*Last() const;

   ostream&	printOn(ostream& strm=cout) const;
};

//
// PRINTING
//

inline ostream& operator<<(ostream& strm, const PtrListC& c)
{
   return c.printOn(strm);
}

#endif // _PtrListC_h_
