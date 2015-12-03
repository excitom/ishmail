//
// SignalDictC.h
//

#ifndef _SignalDictC_h_
#define _SignalDictC_h_

#include "SignalCallsC.h"

/*
 *  ODictCEntry.clas -- class definition for an entry in an ODictC
 *
 *  This type of dictionary stores the actual keys and values
 */

class SignalDictCEntry {

public:

   int	key;
   SignalCallsC	val;

   inline SignalDictCEntry () {}
   inline SignalDictCEntry (const int& kobj, const SignalCallsC& vobj)
			: key(kobj), val(vobj) {}
   inline SignalDictCEntry (const SignalDictCEntry& e) { *this = e; }

   inline SignalDictCEntry&	operator= (const SignalDictCEntry& e) {
      if ( this != &e ) {
	 key = e.key;
	 val = e.val;
      }
      return (*this);
   }

   inline int	operator== (const SignalDictCEntry& e) const {
      return (key == e.key && val == e.val);
   }
   inline int	operator!= (const SignalDictCEntry& e) const {
      return (!(*this==e));
   }
   inline int	compare  (const SignalDictCEntry&) const { return 0; }
   inline int	operator<(const SignalDictCEntry&) const { return 0; }
   inline int	operator>(const SignalDictCEntry&) const { return 0; }

   inline ostream&  printOn(ostream& strm=cout) const {
      strm << key << " = " << val;
      return strm;
   }
};

inline ostream& operator<<(ostream& strm, SignalDictCEntry& e)
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

class SignalDictCEntryList {

protected:

   SignalDictCEntry		*_list;
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

   SignalDictCEntryList();
   SignalDictCEntryList(unsigned);
   SignalDictCEntryList(const SignalDictCEntryList&);
   virtual ~SignalDictCEntryList();

// Append another list to this one

   SignalDictCEntryList&	operator+=(const SignalDictCEntryList&);

// Copy another list to this one

   SignalDictCEntryList&	operator= (const SignalDictCEntryList&);

// Return the position of the specified object.	 Object must be in list

   int		indexOf(const SignalDictCEntry&) const;

// Return the sorted position of the specified object.	Object does not
//    have to be in the list

   unsigned	orderOf(const SignalDictCEntry&) const;

//
// Return non-zero if the specified object is in the list
//
   int		includes(const SignalDictCEntry&) const;

//
// Add the specified object to the list in the requested position
//    The position is ignored if this list is sorted
//
   SignalDictCEntry		*insert (const SignalDictCEntry&, unsigned);
   SignalDictCEntry		*append (const SignalDictCEntry&);
   SignalDictCEntry		*prepend(const SignalDictCEntry&);
   SignalDictCEntry		*add    (const SignalDictCEntry&);

//
// Remove the specified object or index from the list
//
   void		remove(unsigned);
   void		remove(const SignalDictCEntry&);
   void		removeAll();
   void		removeLast();

//
// Return a pointer to the previous object in the list
//
   SignalDictCEntry		*before(const SignalDictCEntry&) const;

//
// Functions to support sorting
//
   static int	compare(const void*, const void*);
   void		sort();
   void		sort(int (*)(const void*, const void*));
   void		SetSorted(char);

   inline unsigned	size()		const { return _count; }
   inline unsigned	capacity()	const { return _space; }
   inline SignalDictCEntry		*start()	const { return _list; }

   void		AllowDuplicates(char);
   void		AutoShrink(char);
   void		SetCapacity(unsigned);

   SignalDictCEntry		*operator[](unsigned) const;

   SignalDictCEntry		*First() const;
   SignalDictCEntry		*Last() const;

   ostream&	printOn(ostream& strm=cout) const;
};

//
// PRINTING
//

inline ostream& operator<<(ostream& strm, const SignalDictCEntryList& c)
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

class SignalDictC : public SignalDictCEntryList {

public:

/* Copy one dictionary to another */

   SignalDictC&	operator=(const SignalDictC&);

/* Lookup up entries */

   int		indexOf (const int&) const;
   int		indexOf (const SignalDictCEntry&) const;
   int		includes(const int&) const;
   int		includes(const SignalDictCEntry&) const;

   SignalCallsC	*definitionOf(const int& key) const;

   int	&keyOf(const int index) const;
   SignalCallsC	&valOf(const int index) const;

   SignalDictCEntry	*entryOf(const int& key) const;

/* Changing entries */

   SignalDictCEntry	*add   (const int& key, const SignalCallsC& val);
   SignalDictCEntry	*modify(const int& key, const SignalCallsC& val);
   SignalDictCEntry	*modify(unsigned index,     const SignalCallsC& val);

/* Deleting entries */

   void		remove(int&);
   void		remove(unsigned);
   void		remove(SignalDictCEntry&);
};

inline ostream& operator<<(ostream& strm, const SignalDictC& d)
{
   return d.printOn(strm);
}

#endif // _SignalDictC_h_
