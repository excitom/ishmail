//
// ButtonEntryListC.h
//

#ifndef _ButtonEntryListC_h_
#define _ButtonEntryListC_h_

#include "ButtonEntryC.h"

/*
 * PListC.clas -- class definition file for class PListC
 *
 *   This type of list stores only pointers to its members
 *   OListC stores the members themselves
 */

class ButtonEntryListC {

protected:

   ButtonEntryC			**_list;
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

   ButtonEntryListC();
   ButtonEntryListC(unsigned ga);
   ButtonEntryListC(const ButtonEntryListC&);
   ~ButtonEntryListC();

/* Append another list to this one */
   ButtonEntryListC& operator+=(const ButtonEntryListC&);

/* Copy another list to this one */
   ButtonEntryListC& operator=(const ButtonEntryListC&);

/* Return the position of the specified object.  Object must be in list */

   int		indexOf(const ButtonEntryC&) const;
   int		indexOf(const ButtonEntryC*) const;

/* Return the sorted position of the specified object.  Object does not */
/*    have to be in the list */

   unsigned		orderOf(const ButtonEntryC&) const;
   unsigned		orderOf(const ButtonEntryC*) const;

/* Return non-zero if the specified object is in the list */

   int		includes(const ButtonEntryC&) const;
   int		includes(const ButtonEntryC*) const;

/* Add the specified object to the list in the requested position */
/*    The position is ignored if this list is sorted */

   ButtonEntryC		*insert (const ButtonEntryC*, unsigned);
   ButtonEntryC		*replace(const ButtonEntryC*, unsigned);
   ButtonEntryC		*append (const ButtonEntryC*);
   ButtonEntryC		*prepend(const ButtonEntryC*);
   ButtonEntryC		*add    (const ButtonEntryC*);

/* Remove the specified object or index from the list */

   void		remove(unsigned);
   void		remove(const ButtonEntryC&);
   void		remove(const ButtonEntryC*);
   void		removeNulls();
   void		removeAll();
   void		removeLast();

/* Return a pointer to the previous object in the list */

   ButtonEntryC		*before(const ButtonEntryC*) const;

/* Functions to support sorting */

   static int	compare(const void*, const void*);
   void		sort();
   void		sort(int (*)(const void*, const void*));
   void		SetSorted(char);

   inline unsigned	size()		const { return _count; }
   inline unsigned	capacity()	const { return _space; }
   inline ButtonEntryC		**start()	const { return _list; }
   void         	AllowDuplicates(char);
   void			AutoShrink(char);
   void			SetCapacity(unsigned);

   ButtonEntryC			*operator[](unsigned) const;
   ButtonEntryC			*First() const;
   ButtonEntryC			*Last() const;

   ostream&		printOn(ostream& strm=cout) const;
};

/*-------------------------------------------------------------*/
/* PRINTING */

inline ostream&
operator<<(ostream& strm, const ButtonEntryListC& c)
{
   return c.printOn(strm);
}

#endif // _ButtonEntryListC_h_
