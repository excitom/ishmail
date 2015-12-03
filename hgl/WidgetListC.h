//
// WidgetListC.h
//

#ifndef _WidgetListC_h_
#define _WidgetListC_h_

#include <X11/Intrinsic.h>

/*
 * OListC.clas -- class definition for class OListC
 *
 *   This type of list store the members themselves
 *   PListC stores only pointers to the members
 */

#include <stream.h>

class WidgetListC {

protected:

   Widget		*_list;
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

   WidgetListC();
   WidgetListC(unsigned);
   WidgetListC(const WidgetListC&);
   virtual ~WidgetListC();

// Append another list to this one

   WidgetListC&	operator+=(const WidgetListC&);

// Copy another list to this one

   WidgetListC&	operator= (const WidgetListC&);

// Return the position of the specified object.	 Object must be in list

   int		indexOf(const Widget&) const;

// Return the sorted position of the specified object.	Object does not
//    have to be in the list

   unsigned	orderOf(const Widget&) const;

//
// Return non-zero if the specified object is in the list
//
   int		includes(const Widget&) const;

//
// Add the specified object to the list in the requested position
//    The position is ignored if this list is sorted
//
   Widget		*insert (const Widget&, unsigned);
   Widget		*append (const Widget&);
   Widget		*prepend(const Widget&);
   Widget		*add    (const Widget&);

//
// Remove the specified object or index from the list
//
   void		remove(unsigned);
   void		remove(const Widget&);
   void		removeAll();
   void		removeLast();

//
// Return a pointer to the previous object in the list
//
   Widget		*before(const Widget&) const;

//
// Functions to support sorting
//
   static int	compare(const void*, const void*);
   void		sort();
   void		sort(int (*)(const void*, const void*));
   void		SetSorted(char);

   inline unsigned	size()		const { return _count; }
   inline unsigned	capacity()	const { return _space; }
   inline Widget		*start()	const { return _list; }

   void		AllowDuplicates(char);
   void		AutoShrink(char);
   void		SetCapacity(unsigned);

   Widget		*operator[](unsigned) const;

   Widget		*First() const;
   Widget		*Last() const;

   ostream&	printOn(ostream& strm=cout) const;
};

//
// PRINTING
//

inline ostream& operator<<(ostream& strm, const WidgetListC& c)
{
   return c.printOn(strm);
}

#endif // _WidgetListC_h_
