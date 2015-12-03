//
// MsgListC.h
//

#ifndef _MsgListC_h_
#define _MsgListC_h_

#include "MsgC.h"

/*
 * PListC.clas -- class definition file for class PListC
 *
 *   This type of list stores only pointers to its members
 *   OListC stores the members themselves
 */

class MsgListC {

protected:

   MsgC			**_list;
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

   MsgListC();
   MsgListC(unsigned ga);
   MsgListC(const MsgListC&);
   ~MsgListC();

/* Append another list to this one */
   MsgListC& operator+=(const MsgListC&);

/* Copy another list to this one */
   MsgListC& operator=(const MsgListC&);

/* Return the position of the specified object.  Object must be in list */

   int		indexOf(const MsgC&) const;
   int		indexOf(const MsgC*) const;

/* Return the sorted position of the specified object.  Object does not */
/*    have to be in the list */

   unsigned		orderOf(const MsgC&) const;
   unsigned		orderOf(const MsgC*) const;

/* Return non-zero if the specified object is in the list */

   int		includes(const MsgC&) const;
   int		includes(const MsgC*) const;

/* Add the specified object to the list in the requested position */
/*    The position is ignored if this list is sorted */

   MsgC		*insert (const MsgC*, unsigned);
   MsgC		*replace(const MsgC*, unsigned);
   MsgC		*append (const MsgC*);
   MsgC		*prepend(const MsgC*);
   MsgC		*add    (const MsgC*);

/* Remove the specified object or index from the list */

   void		remove(unsigned);
   void		remove(const MsgC&);
   void		remove(const MsgC*);
   void		removeNulls();
   void		removeAll();
   void		removeLast();

/* Return a pointer to the previous object in the list */

   MsgC		*before(const MsgC*) const;

/* Functions to support sorting */

   static int	compare(const void*, const void*);
   void		sort();
   void		sort(int (*)(const void*, const void*));
   void		SetSorted(char);

   inline unsigned	size()		const { return _count; }
   inline unsigned	capacity()	const { return _space; }
   inline MsgC		**start()	const { return _list; }
   void         	AllowDuplicates(char);
   void			AutoShrink(char);
   void			SetCapacity(unsigned);

   MsgC			*operator[](unsigned) const;
   MsgC			*First() const;
   MsgC			*Last() const;

   ostream&		printOn(ostream& strm=cout) const;
};

/*-------------------------------------------------------------*/
/* PRINTING */

inline ostream&
operator<<(ostream& strm, const MsgListC& c)
{
   return c.printOn(strm);
}

#endif // _MsgListC_h_
