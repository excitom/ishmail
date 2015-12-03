//
// PixmapNameDictC.h
//

#ifndef _PixmapNameDictC_h_
#define _PixmapNameDictC_h_

#include "StringC.h"
#include "PixmapC.h"

/*
 *  ODictCEntry.clas -- class definition for an entry in an ODictC
 *
 *  This type of dictionary stores the actual keys and values
 */

class PixmapNameDictCEntry {

public:

   StringC	key;
   PixmapPtrT	val;

   inline PixmapNameDictCEntry () {}
   inline PixmapNameDictCEntry (const StringC& kobj, const PixmapPtrT& vobj)
			: key(kobj), val(vobj) {}
   inline PixmapNameDictCEntry (const PixmapNameDictCEntry& e) { *this = e; }

   inline PixmapNameDictCEntry&	operator= (const PixmapNameDictCEntry& e) {
      if ( this != &e ) {
	 key = e.key;
	 val = e.val;
      }
      return (*this);
   }

   inline int	operator== (const PixmapNameDictCEntry& e) const {
      return (key == e.key && val == e.val);
   }
   inline int	operator!= (const PixmapNameDictCEntry& e) const {
      return (!(*this==e));
   }
   inline int	compare  (const PixmapNameDictCEntry&) const { return 0; }
   inline int	operator<(const PixmapNameDictCEntry&) const { return 0; }
   inline int	operator>(const PixmapNameDictCEntry&) const { return 0; }

   inline ostream&  printOn(ostream& strm=cout) const {
      strm << key << " = " << val;
      return strm;
   }
};

inline ostream& operator<<(ostream& strm, PixmapNameDictCEntry& e)
{
   return e.printOn(strm);
}

/*
 * OListC.clas -- class definition for class OListC
 *
 *   This type of list store the members themselves
 *   PListC stores only pointers to the members
 */

#include <stream.h>

class PixmapNameDictCEntryList {

protected:

   PixmapNameDictCEntry		*_list;
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

   PixmapNameDictCEntryList();
   PixmapNameDictCEntryList(unsigned);
   PixmapNameDictCEntryList(const PixmapNameDictCEntryList&);
   virtual ~PixmapNameDictCEntryList();

// Append another list to this one

   PixmapNameDictCEntryList&	operator+=(const PixmapNameDictCEntryList&);

// Copy another list to this one

   PixmapNameDictCEntryList&	operator= (const PixmapNameDictCEntryList&);

// Return the position of the specified object.	 Object must be in list

   int		indexOf(const PixmapNameDictCEntry&) const;

// Return the sorted position of the specified object.	Object does not
//    have to be in the list

   unsigned	orderOf(const PixmapNameDictCEntry&) const;

//
// Return non-zero if the specified object is in the list
//
   int		includes(const PixmapNameDictCEntry&) const;

//
// Add the specified object to the list in the requested position
//    The position is ignored if this list is sorted
//
   PixmapNameDictCEntry		*insert (const PixmapNameDictCEntry&, unsigned);
   PixmapNameDictCEntry		*append (const PixmapNameDictCEntry&);
   PixmapNameDictCEntry		*prepend(const PixmapNameDictCEntry&);
   PixmapNameDictCEntry		*add    (const PixmapNameDictCEntry&);

//
// Remove the specified object or index from the list
//
   void		remove(unsigned);
   void		remove(const PixmapNameDictCEntry&);
   void		removeAll();
   void		removeLast();

//
// Return a pointer to the previous object in the list
//
   PixmapNameDictCEntry		*before(const PixmapNameDictCEntry&) const;

//
// Functions to support sorting
//
   static int	compare(const void*, const void*);
   void		sort();
   void		sort(int (*)(const void*, const void*));
   void		SetSorted(char);

   inline unsigned	size()		const { return _count; }
   inline unsigned	capacity()	const { return _space; }
   inline PixmapNameDictCEntry		*start()	const { return _list; }

   void		AllowDuplicates(char);
   void		AutoShrink(char);
   void		SetCapacity(unsigned);

   PixmapNameDictCEntry		*operator[](unsigned) const;

   PixmapNameDictCEntry		*First() const;
   PixmapNameDictCEntry		*Last() const;

   ostream&	printOn(ostream& strm=cout) const;
};

//
// PRINTING
//

inline ostream& operator<<(ostream& strm, const PixmapNameDictCEntryList& c)
{
   return c.printOn(strm);
}

/*
 *  ODictC.clas -- class definition for a set of relations
 *
 *  This type of dictionary stores the actual keys and values
 *  PDictC.h stores only pointers to the keys and values
 */

/*--------------------------------------------------------------------------
 * Define dictionary class
 */

class PixmapNameDictC : public PixmapNameDictCEntryList {

public:

/* Copy one dictionary to another */

   PixmapNameDictC&	operator=(const PixmapNameDictC&);

/* Lookup up entries */

   int		indexOf (const StringC&) const;
   int		indexOf (const PixmapNameDictCEntry&) const;
   int		includes(const StringC&) const;
   int		includes(const PixmapNameDictCEntry&) const;

   PixmapPtrT	*definitionOf(const StringC& key) const;

   StringC	&keyOf(const int index) const;
   PixmapPtrT	&valOf(const int index) const;

   PixmapNameDictCEntry	*entryOf(const StringC& key) const;

/* Changing entries */

   PixmapNameDictCEntry	*add   (const StringC& key, const PixmapPtrT& val);
   PixmapNameDictCEntry	*modify(const StringC& key, const PixmapPtrT& val);
   PixmapNameDictCEntry	*modify(unsigned index,     const PixmapPtrT& val);

/* Deleting entries */

   void		remove(StringC&);
   void		remove(unsigned);
   void		remove(PixmapNameDictCEntry&);
};

inline ostream& operator<<(ostream& strm, const PixmapNameDictC& d)
{
   return d.printOn(strm);
}

#endif // _PixmapNameDictC_h_
