//
// LabelDataListC.h
//

#ifndef _LabelDataListC_h_
#define _LabelDataListC_h_

#include "LabelDataC.h"

/*
 * OListC.clas -- class definition for class OListC
 *
 *   This type of list store the members themselves
 *   PListC stores only pointers to the members
 */

#include <stream.h>

class LabelDataListC {

protected:

   LabelDataC		*_list;
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

   LabelDataListC();
   LabelDataListC(unsigned);
   LabelDataListC(const LabelDataListC&);
   virtual ~LabelDataListC();

// Append another list to this one

   LabelDataListC&	operator+=(const LabelDataListC&);

// Copy another list to this one

   LabelDataListC&	operator= (const LabelDataListC&);

// Return the position of the specified object.	 Object must be in list

   int		indexOf(const LabelDataC&) const;

// Return the sorted position of the specified object.	Object does not
//    have to be in the list

   unsigned	orderOf(const LabelDataC&) const;

//
// Return non-zero if the specified object is in the list
//
   int		includes(const LabelDataC&) const;

//
// Add the specified object to the list in the requested position
//    The position is ignored if this list is sorted
//
   LabelDataC		*insert (const LabelDataC&, unsigned);
   LabelDataC		*append (const LabelDataC&);
   LabelDataC		*prepend(const LabelDataC&);
   LabelDataC		*add    (const LabelDataC&);

//
// Remove the specified object or index from the list
//
   void		remove(unsigned);
   void		remove(const LabelDataC&);
   void		removeAll();
   void		removeLast();

//
// Return a pointer to the previous object in the list
//
   LabelDataC		*before(const LabelDataC&) const;

//
// Functions to support sorting
//
   static int	compare(const void*, const void*);
   void		sort();
   void		sort(int (*)(const void*, const void*));
   void		SetSorted(char);

   inline unsigned	size()		const { return _count; }
   inline unsigned	capacity()	const { return _space; }
   inline LabelDataC		*start()	const { return _list; }

   void		AllowDuplicates(char);
   void		AutoShrink(char);
   void		SetCapacity(unsigned);

   LabelDataC		*operator[](unsigned) const;

   LabelDataC		*First() const;
   LabelDataC		*Last() const;

   ostream&	printOn(ostream& strm=cout) const;
};

//
// PRINTING
//

inline ostream& operator<<(ostream& strm, const LabelDataListC& c)
{
   return c.printOn(strm);
}

#endif // _LabelDataListC_h_
