//
// PartialMsgDictC.h
//

#ifndef _PartialMsgDictC_h_
#define _PartialMsgDictC_h_

#include <hgl/StringC.h>
#include "PartialMsgC.h"

/*
 *  ODictCEntry.clas -- class definition for an entry in an ODictC
 *
 *  This type of dictionary stores the actual keys and values
 */

class PartialMsgDictCEntry {

public:

   StringC	key;
   PartialMsgPtr	val;

   inline PartialMsgDictCEntry () {}
   inline PartialMsgDictCEntry (const StringC& kobj, const PartialMsgPtr& vobj)
			: key(kobj), val(vobj) {}
   inline PartialMsgDictCEntry (const PartialMsgDictCEntry& e) { *this = e; }

   inline PartialMsgDictCEntry&	operator= (const PartialMsgDictCEntry& e) {
      if ( this != &e ) {
	 key = e.key;
	 val = e.val;
      }
      return (*this);
   }

   inline int	operator== (const PartialMsgDictCEntry& e) const {
      return (key == e.key && val == e.val);
   }
   inline int	operator!= (const PartialMsgDictCEntry& e) const {
      return (!(*this==e));
   }
   inline int	compare  (const PartialMsgDictCEntry&) const { return 0; }
   inline int	operator<(const PartialMsgDictCEntry&) const { return 0; }
   inline int	operator>(const PartialMsgDictCEntry&) const { return 0; }

   inline ostream&  printOn(ostream& strm=cout) const {
      strm << key << " = " << val;
      return strm;
   }
};

inline ostream& operator<<(ostream& strm, PartialMsgDictCEntry& e)
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

class PartialMsgDictCEntryList {

protected:

   PartialMsgDictCEntry		*_list;
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

   PartialMsgDictCEntryList();
   PartialMsgDictCEntryList(unsigned);
   PartialMsgDictCEntryList(const PartialMsgDictCEntryList&);
   virtual ~PartialMsgDictCEntryList();

// Append another list to this one

   PartialMsgDictCEntryList&	operator+=(const PartialMsgDictCEntryList&);

// Copy another list to this one

   PartialMsgDictCEntryList&	operator= (const PartialMsgDictCEntryList&);

// Return the position of the specified object.	 Object must be in list

   int		indexOf(const PartialMsgDictCEntry&) const;

// Return the sorted position of the specified object.	Object does not
//    have to be in the list

   unsigned	orderOf(const PartialMsgDictCEntry&) const;

//
// Return non-zero if the specified object is in the list
//
   int		includes(const PartialMsgDictCEntry&) const;

//
// Add the specified object to the list in the requested position
//    The position is ignored if this list is sorted
//
   PartialMsgDictCEntry		*insert (const PartialMsgDictCEntry&, unsigned);
   PartialMsgDictCEntry		*append (const PartialMsgDictCEntry&);
   PartialMsgDictCEntry		*prepend(const PartialMsgDictCEntry&);
   PartialMsgDictCEntry		*add    (const PartialMsgDictCEntry&);

//
// Remove the specified object or index from the list
//
   void		remove(unsigned);
   void		remove(const PartialMsgDictCEntry&);
   void		removeAll();
   void		removeLast();

//
// Return a pointer to the previous object in the list
//
   PartialMsgDictCEntry		*before(const PartialMsgDictCEntry&) const;

//
// Functions to support sorting
//
   static int	compare(const void*, const void*);
   void		sort();
   void		sort(int (*)(const void*, const void*));
   void		SetSorted(char);

   inline unsigned	size()		const { return _count; }
   inline unsigned	capacity()	const { return _space; }
   inline PartialMsgDictCEntry		*start()	const { return _list; }

   void		AllowDuplicates(char);
   void		AutoShrink(char);
   void		SetCapacity(unsigned);

   PartialMsgDictCEntry		*operator[](unsigned) const;

   PartialMsgDictCEntry		*First() const;
   PartialMsgDictCEntry		*Last() const;

   ostream&	printOn(ostream& strm=cout) const;
};

//
// PRINTING
//

inline ostream& operator<<(ostream& strm, const PartialMsgDictCEntryList& c)
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

class PartialMsgDictC : public PartialMsgDictCEntryList {

public:

/* Copy one dictionary to another */

   PartialMsgDictC&	operator=(const PartialMsgDictC&);

/* Lookup up entries */

   int		indexOf (const StringC&) const;
   int		indexOf (const PartialMsgDictCEntry&) const;
   int		includes(const StringC&) const;
   int		includes(const PartialMsgDictCEntry&) const;

   PartialMsgPtr	*definitionOf(const StringC& key) const;

   StringC	&keyOf(const int index) const;
   PartialMsgPtr	&valOf(const int index) const;

   PartialMsgDictCEntry	*entryOf(const StringC& key) const;

/* Changing entries */

   PartialMsgDictCEntry	*add   (const StringC& key, const PartialMsgPtr& val);
   PartialMsgDictCEntry	*modify(const StringC& key, const PartialMsgPtr& val);
   PartialMsgDictCEntry	*modify(unsigned index,     const PartialMsgPtr& val);

/* Deleting entries */

   void		remove(StringC&);
   void		remove(unsigned);
   void		remove(PartialMsgDictCEntry&);
};

inline ostream& operator<<(ostream& strm, const PartialMsgDictC& d)
{
   return d.printOn(strm);
}

#endif // _PartialMsgDictC_h_
