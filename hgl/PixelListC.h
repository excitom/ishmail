//
// PixelListC.h
//

#ifndef _PixelListC_h_
#define _PixelListC_h_

#include <X11/Intrinsic.h>

/*
 * OListC.clas -- class definition for class OListC
 *
 *   This type of list store the members themselves
 *   PListC stores only pointers to the members
 */

#include <stream.h>

class PixelListC {

protected:

   Pixel		*_list;
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

   PixelListC();
   PixelListC(unsigned);
   PixelListC(const PixelListC&);
   virtual ~PixelListC();

// Append another list to this one

   PixelListC&	operator+=(const PixelListC&);

// Copy another list to this one

   PixelListC&	operator= (const PixelListC&);

// Return the position of the specified object.	 Object must be in list

   int		indexOf(const Pixel&) const;

// Return the sorted position of the specified object.	Object does not
//    have to be in the list

   unsigned	orderOf(const Pixel&) const;

//
// Return non-zero if the specified object is in the list
//
   int		includes(const Pixel&) const;

//
// Add the specified object to the list in the requested position
//    The position is ignored if this list is sorted
//
   Pixel		*insert (const Pixel&, unsigned);
   Pixel		*append (const Pixel&);
   Pixel		*prepend(const Pixel&);
   Pixel		*add    (const Pixel&);

//
// Remove the specified object or index from the list
//
   void		remove(unsigned);
   void		remove(const Pixel&);
   void		removeAll();
   void		removeLast();

//
// Return a pointer to the previous object in the list
//
   Pixel		*before(const Pixel&) const;

//
// Functions to support sorting
//
   static int	compare(const void*, const void*);
   void		sort();
   void		sort(int (*)(const void*, const void*));
   void		SetSorted(char);

   inline unsigned	size()		const { return _count; }
   inline unsigned	capacity()	const { return _space; }
   inline Pixel		*start()	const { return _list; }

   void		AllowDuplicates(char);
   void		AutoShrink(char);
   void		SetCapacity(unsigned);

   Pixel		*operator[](unsigned) const;

   Pixel		*First() const;
   Pixel		*Last() const;

   ostream&	printOn(ostream& strm=cout) const;
};

//
// PRINTING
//

inline ostream& operator<<(ostream& strm, const PixelListC& c)
{
   return c.printOn(strm);
}

#endif // _PixelListC_h_
