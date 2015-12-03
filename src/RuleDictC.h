//
// RuleDictC.h
//

#ifndef _RuleDictC_h_
#define _RuleDictC_h_

#include <hgl/RegexC.h>
#include <hgl/StringC.h>

/*
 *  ODictCEntry.clas -- class definition for an entry in an ODictC
 *
 *  This type of dictionary stores the actual keys and values
 */

class RuleDictCEntry {

public:

   RegexC	key;
   StringC	val;

   inline RuleDictCEntry () {}
   inline RuleDictCEntry (const RegexC& kobj, const StringC& vobj)
			: key(kobj), val(vobj) {}
   inline RuleDictCEntry (const RuleDictCEntry& e) { *this = e; }

   inline RuleDictCEntry&	operator= (const RuleDictCEntry& e) {
      if ( this != &e ) {
	 key = e.key;
	 val = e.val;
      }
      return (*this);
   }

   inline int	operator== (const RuleDictCEntry& e) const {
      return (key == e.key && val == e.val);
   }
   inline int	operator!= (const RuleDictCEntry& e) const {
      return (!(*this==e));
   }
   inline int	compare  (const RuleDictCEntry&) const { return 0; }
   inline int	operator<(const RuleDictCEntry&) const { return 0; }
   inline int	operator>(const RuleDictCEntry&) const { return 0; }

   inline ostream&  printOn(ostream& strm=cout) const {
      strm << key << " = " << val;
      return strm;
   }
};

inline ostream& operator<<(ostream& strm, RuleDictCEntry& e)
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

class RuleDictCEntryList {

protected:

   RuleDictCEntry		*_list;
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

   RuleDictCEntryList();
   RuleDictCEntryList(unsigned);
   RuleDictCEntryList(const RuleDictCEntryList&);
   virtual ~RuleDictCEntryList();

// Append another list to this one

   RuleDictCEntryList&	operator+=(const RuleDictCEntryList&);

// Copy another list to this one

   RuleDictCEntryList&	operator= (const RuleDictCEntryList&);

// Return the position of the specified object.	 Object must be in list

   int		indexOf(const RuleDictCEntry&) const;

// Return the sorted position of the specified object.	Object does not
//    have to be in the list

   unsigned	orderOf(const RuleDictCEntry&) const;

//
// Return non-zero if the specified object is in the list
//
   int		includes(const RuleDictCEntry&) const;

//
// Add the specified object to the list in the requested position
//    The position is ignored if this list is sorted
//
   RuleDictCEntry		*insert (const RuleDictCEntry&, unsigned);
   RuleDictCEntry		*append (const RuleDictCEntry&);
   RuleDictCEntry		*prepend(const RuleDictCEntry&);
   RuleDictCEntry		*add    (const RuleDictCEntry&);

//
// Remove the specified object or index from the list
//
   void		remove(unsigned);
   void		remove(const RuleDictCEntry&);
   void		removeAll();
   void		removeLast();

//
// Return a pointer to the previous object in the list
//
   RuleDictCEntry		*before(const RuleDictCEntry&) const;

//
// Functions to support sorting
//
   static int	compare(const void*, const void*);
   void		sort();
   void		sort(int (*)(const void*, const void*));
   void		SetSorted(char);

   inline unsigned	size()		const { return _count; }
   inline unsigned	capacity()	const { return _space; }
   inline RuleDictCEntry		*start()	const { return _list; }

   void		AllowDuplicates(char);
   void		AutoShrink(char);
   void		SetCapacity(unsigned);

   RuleDictCEntry		*operator[](unsigned) const;

   RuleDictCEntry		*First() const;
   RuleDictCEntry		*Last() const;

   ostream&	printOn(ostream& strm=cout) const;
};

//
// PRINTING
//

inline ostream& operator<<(ostream& strm, const RuleDictCEntryList& c)
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

class RuleDictC : public RuleDictCEntryList {

public:

/* Copy one dictionary to another */

   RuleDictC&	operator=(const RuleDictC&);

/* Lookup up entries */

   int		indexOf (const RegexC&) const;
   int		indexOf (const RuleDictCEntry&) const;
   int		includes(const RegexC&) const;
   int		includes(const RuleDictCEntry&) const;

   StringC	*definitionOf(const RegexC& key) const;

   RegexC	&keyOf(const int index) const;
   StringC	&valOf(const int index) const;

   RuleDictCEntry	*entryOf(const RegexC& key) const;

/* Changing entries */

   RuleDictCEntry	*add   (const RegexC& key, const StringC& val);
   RuleDictCEntry	*modify(const RegexC& key, const StringC& val);
   RuleDictCEntry	*modify(unsigned index,     const StringC& val);

/* Deleting entries */

   void		remove(RegexC&);
   void		remove(unsigned);
   void		remove(RuleDictCEntry&);
};

inline ostream& operator<<(ostream& strm, const RuleDictC& d)
{
   return d.printOn(strm);
}

#endif // _RuleDictC_h_
