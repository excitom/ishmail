//
// PixmapDataDictC.h
//

#ifndef _PixmapDataDictC_h_
#define _PixmapDataDictC_h_

#include "PtrT.h"
#include "PixmapC.h"

/*
 *  ODictCEntry.clas -- class definition for an entry in an ODictC
 *
 *  This type of dictionary stores the actual keys and values
 */

class PixmapDataDictCEntry {

public:

   PtrT	key;
   PixmapPtrT	val;

   inline PixmapDataDictCEntry () {}
   inline PixmapDataDictCEntry (const PtrT& kobj, const PixmapPtrT& vobj)
			: key(kobj), val(vobj) {}
   inline PixmapDataDictCEntry (const PixmapDataDictCEntry& e) { *this = e; }

   inline PixmapDataDictCEntry&	operator= (const PixmapDataDictCEntry& e) {
      if ( this != &e ) {
	 key = e.key;
	 val = e.val;
      }
      return (*this);
   }

   inline int	operator== (const PixmapDataDictCEntry& e) const {
      return (key == e.key && val == e.val);
   }
   inline int	operator!= (const PixmapDataDictCEntry& e) const {
      return (!(*this==e));
   }
   inline int	compare  (const PixmapDataDictCEntry&) const { return 0; }
   inline int	operator<(const PixmapDataDictCEntry&) const { return 0; }
   inline int	operator>(const PixmapDataDictCEntry&) const { return 0; }

   inline ostream&  printOn(ostream& strm=cout) const {
      strm << key << " = " << val;
      return strm;
   }
};

inline ostream& operator<<(ostream& strm, PixmapDataDictCEntry& e)
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

class PixmapDataDictCEntryList {

protected:

   PixmapDataDictCEntry		*_list;
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

   PixmapDataDictCEntryList();
   PixmapDataDictCEntryList(unsigned);
   PixmapDataDictCEntryList(const PixmapDataDictCEntryList&);
   virtual ~PixmapDataDictCEntryList();

// Append another list to this one

   PixmapDataDictCEntryList&	operator+=(const PixmapDataDictCEntryList&);

// Copy another list to this one

   PixmapDataDictCEntryList&	operator= (const PixmapDataDictCEntryList&);

// Return the position of the specified object.	 Object must be in list

   int		indexOf(const PixmapDataDictCEntry&) const;

// Return the sorted position of the specified object.	Object does not
//    have to be in the list

   unsigned	orderOf(const PixmapDataDictCEntry&) const;

//
// Return non-zero if the specified object is in the list
//
   int		includes(const PixmapDataDictCEntry&) const;

//
// Add the specified object to the list in the requested position
//    The position is ignored if this list is sorted
//
   PixmapDataDictCEntry		*insert (const PixmapDataDictCEntry&, unsigned);
   PixmapDataDictCEntry		*append (const PixmapDataDictCEntry&);
   PixmapDataDictCEntry		*prepend(const PixmapDataDictCEntry&);
   PixmapDataDictCEntry		*add    (const PixmapDataDictCEntry&);

//
// Remove the specified object or index from the list
//
   void		remove(unsigned);
   void		remove(const PixmapDataDictCEntry&);
   void		removeAll();
   void		removeLast();

//
// Return a pointer to the previous object in the list
//
   PixmapDataDictCEntry		*before(const PixmapDataDictCEntry&) const;

//
// Functions to support sorting
//
   static int	compare(const void*, const void*);
   void		sort();
   void		sort(int (*)(const void*, const void*));
   void		SetSorted(char);

   inline unsigned	size()		const { return _count; }
   inline unsigned	capacity()	const { return _space; }
   inline PixmapDataDictCEntry		*start()	const { return _list; }

   void		AllowDuplicates(char);
   void		AutoShrink(char);
   void		SetCapacity(unsigned);

   PixmapDataDictCEntry		*operator[](unsigned) const;

   PixmapDataDictCEntry		*First() const;
   PixmapDataDictCEntry		*Last() const;

   ostream&	printOn(ostream& strm=cout) const;
};

//
// PRINTING
//

inline ostream& operator<<(ostream& strm, const PixmapDataDictCEntryList& c)
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

class PixmapDataDictC : public PixmapDataDictCEntryList {

public:

/* Copy one dictionary to another */

   PixmapDataDictC&	operator=(const PixmapDataDictC&);

/* Lookup up entries */

   int		indexOf (const PtrT&) const;
   int		indexOf (const PixmapDataDictCEntry&) const;
   int		includes(const PtrT&) const;
   int		includes(const PixmapDataDictCEntry&) const;

   PixmapPtrT	*definitionOf(const PtrT& key) const;

   PtrT	&keyOf(const int index) const;
   PixmapPtrT	&valOf(const int index) const;

   PixmapDataDictCEntry	*entryOf(const PtrT& key) const;

/* Changing entries */

   PixmapDataDictCEntry	*add   (const PtrT& key, const PixmapPtrT& val);
   PixmapDataDictCEntry	*modify(const PtrT& key, const PixmapPtrT& val);
   PixmapDataDictCEntry	*modify(unsigned index,     const PixmapPtrT& val);

/* Deleting entries */

   void		remove(PtrT&);
   void		remove(unsigned);
   void		remove(PixmapDataDictCEntry&);
};

inline ostream& operator<<(ostream& strm, const PixmapDataDictC& d)
{
   return d.printOn(strm);
}

#endif // _PixmapDataDictC_h_
