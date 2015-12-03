//
// FolderListC.h
//

#ifndef _FolderListC_h_
#define _FolderListC_h_

#include "FolderC.h"

/*
 * PListC.clas -- class definition file for class PListC
 *
 *   This type of list stores only pointers to its members
 *   OListC stores the members themselves
 */

class FolderListC {

protected:

   FolderC			**_list;
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

   FolderListC();
   FolderListC(unsigned ga);
   FolderListC(const FolderListC&);
   ~FolderListC();

/* Append another list to this one */
   FolderListC& operator+=(const FolderListC&);

/* Copy another list to this one */
   FolderListC& operator=(const FolderListC&);

/* Return the position of the specified object.  Object must be in list */

   int		indexOf(const FolderC&) const;
   int		indexOf(const FolderC*) const;

/* Return the sorted position of the specified object.  Object does not */
/*    have to be in the list */

   unsigned		orderOf(const FolderC&) const;
   unsigned		orderOf(const FolderC*) const;

/* Return non-zero if the specified object is in the list */

   int		includes(const FolderC&) const;
   int		includes(const FolderC*) const;

/* Add the specified object to the list in the requested position */
/*    The position is ignored if this list is sorted */

   FolderC		*insert (const FolderC*, unsigned);
   FolderC		*replace(const FolderC*, unsigned);
   FolderC		*append (const FolderC*);
   FolderC		*prepend(const FolderC*);
   FolderC		*add    (const FolderC*);

/* Remove the specified object or index from the list */

   void		remove(unsigned);
   void		remove(const FolderC&);
   void		remove(const FolderC*);
   void		removeNulls();
   void		removeAll();
   void		removeLast();

/* Return a pointer to the previous object in the list */

   FolderC		*before(const FolderC*) const;

/* Functions to support sorting */

   static int	compare(const void*, const void*);
   void		sort();
   void		sort(int (*)(const void*, const void*));
   void		SetSorted(char);

   inline unsigned	size()		const { return _count; }
   inline unsigned	capacity()	const { return _space; }
   inline FolderC		**start()	const { return _list; }
   void         	AllowDuplicates(char);
   void			AutoShrink(char);
   void			SetCapacity(unsigned);

   FolderC			*operator[](unsigned) const;
   FolderC			*First() const;
   FolderC			*Last() const;

   ostream&		printOn(ostream& strm=cout) const;
};

/*-------------------------------------------------------------*/
/* PRINTING */

inline ostream&
operator<<(ostream& strm, const FolderListC& c)
{
   return c.printOn(strm);
}

#endif // _FolderListC_h_
