//
// CallbackListC.h
//

#ifndef _CallbackListC_h_
#define _CallbackListC_h_

#include "CallbackC.h"

/*
 * PListC.clas -- class definition file for class PListC
 *
 *   This type of list stores only pointers to its members
 *   OListC stores the members themselves
 */

class CallbackListC {

protected:

   CallbackC			**_list;
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

   CallbackListC();
   CallbackListC(unsigned ga);
   CallbackListC(const CallbackListC&);
   ~CallbackListC();

/* Append another list to this one */
   CallbackListC& operator+=(const CallbackListC&);

/* Copy another list to this one */
   CallbackListC& operator=(const CallbackListC&);

/* Return the position of the specified object.  Object must be in list */

   int		indexOf(const CallbackC&) const;
   int		indexOf(const CallbackC*) const;

/* Return the sorted position of the specified object.  Object does not */
/*    have to be in the list */

   unsigned		orderOf(const CallbackC&) const;
   unsigned		orderOf(const CallbackC*) const;

/* Return non-zero if the specified object is in the list */

   int		includes(const CallbackC&) const;
   int		includes(const CallbackC*) const;

/* Add the specified object to the list in the requested position */
/*    The position is ignored if this list is sorted */

   CallbackC		*insert (const CallbackC*, unsigned);
   CallbackC		*replace(const CallbackC*, unsigned);
   CallbackC		*append (const CallbackC*);
   CallbackC		*prepend(const CallbackC*);
   CallbackC		*add    (const CallbackC*);

/* Remove the specified object or index from the list */

   void		remove(unsigned);
   void		remove(const CallbackC&);
   void		remove(const CallbackC*);
   void		removeNulls();
   void		removeAll();
   void		removeLast();

/* Return a pointer to the previous object in the list */

   CallbackC		*before(const CallbackC*) const;

/* Functions to support sorting */

   static int	compare(const void*, const void*);
   void		sort();
   void		sort(int (*)(const void*, const void*));
   void		SetSorted(char);

   inline unsigned	size()		const { return _count; }
   inline unsigned	capacity()	const { return _space; }
   inline CallbackC		**start()	const { return _list; }
   void         	AllowDuplicates(char);
   void			AutoShrink(char);
   void			SetCapacity(unsigned);

   CallbackC			*operator[](unsigned) const;
   CallbackC			*First() const;
   CallbackC			*Last() const;

   ostream&		printOn(ostream& strm=cout) const;
};

/*-------------------------------------------------------------*/
/* PRINTING */

inline ostream&
operator<<(ostream& strm, const CallbackListC& c)
{
   return c.printOn(strm);
}

#endif // _CallbackListC_h_
