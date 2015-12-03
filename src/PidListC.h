//
// PidListC.h
//

#ifndef _PidListC_h_
#define _PidListC_h_

#include <sys/types.h>

/*
 * OListC.clas -- class definition for class OListC
 *
 *   This type of list store the members themselves
 *   PListC stores only pointers to the members
 */

#include <stream.h>

class PidListC {

protected:

   pid_t		*_list;
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

   PidListC();
   PidListC(unsigned);
   PidListC(const PidListC&);
   virtual ~PidListC();

// Append another list to this one

   PidListC&	operator+=(const PidListC&);

// Copy another list to this one

   PidListC&	operator= (const PidListC&);

// Return the position of the specified object.	 Object must be in list

   int		indexOf(const pid_t&) const;

// Return the sorted position of the specified object.	Object does not
//    have to be in the list

   unsigned	orderOf(const pid_t&) const;

//
// Return non-zero if the specified object is in the list
//
   int		includes(const pid_t&) const;

//
// Add the specified object to the list in the requested position
//    The position is ignored if this list is sorted
//
   pid_t		*insert (const pid_t&, unsigned);
   pid_t		*append (const pid_t&);
   pid_t		*prepend(const pid_t&);
   pid_t		*add    (const pid_t&);

//
// Remove the specified object or index from the list
//
   void		remove(unsigned);
   void		remove(const pid_t&);
   void		removeAll();
   void		removeLast();

//
// Return a pointer to the previous object in the list
//
   pid_t		*before(const pid_t&) const;

//
// Functions to support sorting
//
   static int	compare(const void*, const void*);
   void		sort();
   void		sort(int (*)(const void*, const void*));
   void		SetSorted(char);

   inline unsigned	size()		const { return _count; }
   inline unsigned	capacity()	const { return _space; }
   inline pid_t		*start()	const { return _list; }

   void		AllowDuplicates(char);
   void		AutoShrink(char);
   void		SetCapacity(unsigned);

   pid_t		*operator[](unsigned) const;

   pid_t		*First() const;
   pid_t		*Last() const;

   ostream&	printOn(ostream& strm=cout) const;
};

//
// PRINTING
//

inline ostream& operator<<(ostream& strm, const PidListC& c)
{
   return c.printOn(strm);
}

#endif // _PidListC_h_
