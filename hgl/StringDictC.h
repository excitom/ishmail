//
// StringDictC.h
//

#ifndef _StringDictC_h_
#define _StringDictC_h_

#include "StringC.h"

/*
 *  ODictCEntry.clas -- class definition for an entry in an ODictC
 *
 *  This type of dictionary stores the actual keys and values
 */

class StringDictCEntry {

public:

   StringC	key;
   StringC	val;

   inline StringDictCEntry () {}
   inline StringDictCEntry (const StringC& kobj, const StringC& vobj)
			: key(kobj), val(vobj) {}
   inline StringDictCEntry (const StringDictCEntry& e) { *this = e; }

   inline StringDictCEntry&	operator= (const StringDictCEntry& e) {
      if ( this != &e ) {
	 key = e.key;
	 val = e.val;
      }
      return (*this);
   }

   inline int	operator== (const StringDictCEntry& e) const {
      return (key == e.key && val == e.val);
   }
   inline int	operator!= (const StringDictCEntry& e) const {
      return (!(*this==e));
   }
   inline int	compare  (const StringDictCEntry&) const { return 0; }
   inline int	operator<(const StringDictCEntry&) const { return 0; }
   inline int	operator>(const StringDictCEntry&) const { return 0; }

   inline ostream&  printOn(ostream& strm=cout) const {
      strm << key << " = " << val;
      return strm;
   }
};

inline ostream& operator<<(ostream& strm, StringDictCEntry& e)
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

class StringDictCEntryList {

protected:

   StringDictCEntry		*_list;
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

   StringDictCEntryList();
   StringDictCEntryList(unsigned);
   StringDictCEntryList(const StringDictCEntryList&);
   virtual ~StringDictCEntryList();

// Append another list to this one

   StringDictCEntryList&	operator+=(const StringDictCEntryList&);

// Copy another list to this one

   StringDictCEntryList&	operator= (const StringDictCEntryList&);

// Return the position of the specified object.	 Object must be in list

   int		indexOf(const StringDictCEntry&) const;

// Return the sorted position of the specified object.	Object does not
//    have to be in the list

   unsigned	orderOf(const StringDictCEntry&) const;

//
// Return non-zero if the specified object is in the list
//
   int		includes(const StringDictCEntry&) const;

//
// Add the specified object to the list in the requested position
//    The position is ignored if this list is sorted
//
   StringDictCEntry		*insert (const StringDictCEntry&, unsigned);
   StringDictCEntry		*append (const StringDictCEntry&);
   StringDictCEntry		*prepend(const StringDictCEntry&);
   StringDictCEntry		*add    (const StringDictCEntry&);

//
// Remove the specified object or index from the list
//
   void		remove(unsigned);
   void		remove(const StringDictCEntry&);
   void		removeAll();
   void		removeLast();

//
// Return a pointer to the previous object in the list
//
   StringDictCEntry		*before(const StringDictCEntry&) const;

//
// Functions to support sorting
//
   static int	compare(const void*, const void*);
   void		sort();
   void		sort(int (*)(const void*, const void*));
   void		SetSorted(char);

   inline unsigned	size()		const { return _count; }
   inline unsigned	capacity()	const { return _space; }
   inline StringDictCEntry		*start()	const { return _list; }

   void		AllowDuplicates(char);
   void		AutoShrink(char);
   void		SetCapacity(unsigned);

   StringDictCEntry		*operator[](unsigned) const;

   StringDictCEntry		*First() const;
   StringDictCEntry		*Last() const;

   ostream&	printOn(ostream& strm=cout) const;
};

//
// PRINTING
//

inline ostream& operator<<(ostream& strm, const StringDictCEntryList& c)
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

class StringDictC : public StringDictCEntryList {

public:

/* Copy one dictionary to another */

   StringDictC&	operator=(const StringDictC&);

/* Lookup up entries */

   int		indexOf (const StringC&) const;
   int		indexOf (const StringDictCEntry&) const;
   int		includes(const StringC&) const;
   int		includes(const StringDictCEntry&) const;

   StringC	*definitionOf(const StringC& key) const;

   StringC	&keyOf(const int index) const;
   StringC	&valOf(const int index) const;

   StringDictCEntry	*entryOf(const StringC& key) const;

/* Changing entries */

   StringDictCEntry	*add   (const StringC& key, const StringC& val);
   StringDictCEntry	*modify(const StringC& key, const StringC& val);
   StringDictCEntry	*modify(unsigned index,     const StringC& val);

/* Deleting entries */

   void		remove(StringC&);
   void		remove(unsigned);
   void		remove(StringDictCEntry&);
};

inline ostream& operator<<(ostream& strm, const StringDictC& d)
{
   return d.printOn(strm);
}

#endif // _StringDictC_h_
