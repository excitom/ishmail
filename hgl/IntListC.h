//
// IntListC.h
//

#ifndef _IntListC_h_
#define _IntListC_h_

#include <stream.h>

/*
 * OListC.clas -- class definition for class OListC
 *
 *   This type of list store the members themselves
 *   PListC stores only pointers to the members
 */

#include <stream.h>

class IntListC {

protected:

   int		*_list;
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

   IntListC();
   IntListC(unsigned);
   IntListC(const IntListC&);
   virtual ~IntListC();

// Append another list to this one

   IntListC&	operator+=(const IntListC&);

// Copy another list to this one

   IntListC&	operator= (const IntListC&);

// Return the position of the specified object.	 Object must be in list

   int		indexOf(const int&) const;

// Return the sorted position of the specified object.	Object does not
//    have to be in the list

   unsigned	orderOf(const int&) const;

//
// Return non-zero if the specified object is in the list
//
   int		includes(const int&) const;

//
// Add the specified object to the list in the requested position
//    The position is ignored if this list is sorted
//
   int		*insert (const int&, unsigned);
   int		*append (const int&);
   int		*prepend(const int&);
   int		*add    (const int&);

//
// Remove the specified object or index from the list
//
   void		remove(unsigned);
   void		remove(const int&);
   void		removeAll();
   void		removeLast();

//
// Return a pointer to the previous object in the list
//
   int		*before(const int&) const;

//
// Functions to support sorting
//
   static int	compare(const void*, const void*);
   void		sort();
   void		sort(int (*)(const void*, const void*));
   void		SetSorted(char);

   inline unsigned	size()		const { return _count; }
   inline unsigned	capacity()	const { return _space; }
   inline int		*start()	const { return _list; }

   void		AllowDuplicates(char);
   void		AutoShrink(char);
   void		SetCapacity(unsigned);

   int		*operator[](unsigned) const;

   int		*First() const;
   int		*Last() const;

   ostream&	printOn(ostream& strm=cout) const;
};

//
// PRINTING
//

inline ostream& operator<<(ostream& strm, const IntListC& c)
{
   return c.printOn(strm);
}

#endif // _IntListC_h_
