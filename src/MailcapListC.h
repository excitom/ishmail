//
// MailcapListC.h
//

#ifndef _MailcapListC_h_
#define _MailcapListC_h_

#include "MailcapC.h"

/*
 * PListC.clas -- class definition file for class PListC
 *
 *   This type of list stores only pointers to its members
 *   OListC stores the members themselves
 */

class MailcapListC {

protected:

   MailcapC			**_list;
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

   MailcapListC();
   MailcapListC(unsigned ga);
   MailcapListC(const MailcapListC&);
   ~MailcapListC();

/* Append another list to this one */
   MailcapListC& operator+=(const MailcapListC&);

/* Copy another list to this one */
   MailcapListC& operator=(const MailcapListC&);

/* Return the position of the specified object.  Object must be in list */

   int		indexOf(const MailcapC&) const;
   int		indexOf(const MailcapC*) const;

/* Return the sorted position of the specified object.  Object does not */
/*    have to be in the list */

   unsigned		orderOf(const MailcapC&) const;
   unsigned		orderOf(const MailcapC*) const;

/* Return non-zero if the specified object is in the list */

   int		includes(const MailcapC&) const;
   int		includes(const MailcapC*) const;

/* Add the specified object to the list in the requested position */
/*    The position is ignored if this list is sorted */

   MailcapC		*insert (const MailcapC*, unsigned);
   MailcapC		*replace(const MailcapC*, unsigned);
   MailcapC		*append (const MailcapC*);
   MailcapC		*prepend(const MailcapC*);
   MailcapC		*add    (const MailcapC*);

/* Remove the specified object or index from the list */

   void		remove(unsigned);
   void		remove(const MailcapC&);
   void		remove(const MailcapC*);
   void		removeNulls();
   void		removeAll();
   void		removeLast();

/* Return a pointer to the previous object in the list */

   MailcapC		*before(const MailcapC*) const;

/* Functions to support sorting */

   static int	compare(const void*, const void*);
   void		sort();
   void		sort(int (*)(const void*, const void*));
   void		SetSorted(char);

   inline unsigned	size()		const { return _count; }
   inline unsigned	capacity()	const { return _space; }
   inline MailcapC		**start()	const { return _list; }
   void         	AllowDuplicates(char);
   void			AutoShrink(char);
   void			SetCapacity(unsigned);

   MailcapC			*operator[](unsigned) const;
   MailcapC			*First() const;
   MailcapC			*Last() const;

   ostream&		printOn(ostream& strm=cout) const;
};

/*-------------------------------------------------------------*/
/* PRINTING */

inline ostream&
operator<<(ostream& strm, const MailcapListC& c)
{
   return c.printOn(strm);
}

#endif // _MailcapListC_h_
