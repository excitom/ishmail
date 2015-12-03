//
// ItemDataDictC.h
//

#ifndef _ItemDataDictC_h_
#define _ItemDataDictC_h_

#include "VItemC.h"
#include "ItemDataC.h"

/*
 *  PDictCEntry.clas -- class definition for an entry in a PDictC
 *
 *   This type of dictionary stores only pointers to the keys and values
 */

class ItemDataDictCEntry {

public:

   VItemC	*key;
   ItemDataC	*val;

   inline ItemDataDictCEntry () { key = (VItemC*)NULL; val = (ItemDataC*)NULL; }
   inline ItemDataDictCEntry (const VItemC* kp, const ItemDataC* vp)
				 { key = (VItemC *)kp; val = (ItemDataC *)vp; }
   inline ItemDataDictCEntry (const ItemDataDictCEntry& e) { *this = e; }

   inline ItemDataDictCEntry&	operator= (const ItemDataDictCEntry& e) {
      if ( this != &e ) {
	 key = e.key;
	 val = e.val;
      }
      return *this;
   }

   inline int	operator== (const ItemDataDictCEntry& e) const {
   /* Only need to check pointers here */
      return (key == e.key && val == e.val);
   }
   inline int	operator!= (const ItemDataDictCEntry& e) const {
      return !(*this==e);
   }
   inline int	compare  (const ItemDataDictCEntry&) const { return 0; }
   inline int	operator<(const ItemDataDictCEntry&) const { return 0; }
   inline int	operator>(const ItemDataDictCEntry&) const { return 0; }

   inline ostream&  printOn(ostream& strm=cout) const {
      strm <<key <<" -> " <<val;
      return strm;
   }
};

inline ostream& operator<<(ostream& strm, const ItemDataDictCEntry& e)
{
   return e.printOn(strm);
}

/*
 * PListC.clas -- class definition file for class PListC
 *
 *   This type of list stores only pointers to its members
 *   OListC stores the members themselves
 */

class ItemDataDictCEntryList {

protected:

   ItemDataDictCEntry			**_list;
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

   ItemDataDictCEntryList();
   ItemDataDictCEntryList(unsigned ga);
   ItemDataDictCEntryList(const ItemDataDictCEntryList&);
   ~ItemDataDictCEntryList();

/* Append another list to this one */
   ItemDataDictCEntryList& operator+=(const ItemDataDictCEntryList&);

/* Copy another list to this one */
   ItemDataDictCEntryList& operator=(const ItemDataDictCEntryList&);

/* Return the position of the specified object.  Object must be in list */

   int		indexOf(const ItemDataDictCEntry&) const;
   int		indexOf(const ItemDataDictCEntry*) const;

/* Return the sorted position of the specified object.  Object does not */
/*    have to be in the list */

   unsigned		orderOf(const ItemDataDictCEntry&) const;
   unsigned		orderOf(const ItemDataDictCEntry*) const;

/* Return non-zero if the specified object is in the list */

   int		includes(const ItemDataDictCEntry&) const;
   int		includes(const ItemDataDictCEntry*) const;

/* Add the specified object to the list in the requested position */
/*    The position is ignored if this list is sorted */

   ItemDataDictCEntry		*insert (const ItemDataDictCEntry*, unsigned);
   ItemDataDictCEntry		*replace(const ItemDataDictCEntry*, unsigned);
   ItemDataDictCEntry		*append (const ItemDataDictCEntry*);
   ItemDataDictCEntry		*prepend(const ItemDataDictCEntry*);
   ItemDataDictCEntry		*add    (const ItemDataDictCEntry*);

/* Remove the specified object or index from the list */

   void		remove(unsigned);
   void		remove(const ItemDataDictCEntry&);
   void		remove(const ItemDataDictCEntry*);
   void		removeNulls();
   void		removeAll();
   void		removeLast();

/* Return a pointer to the previous object in the list */

   ItemDataDictCEntry		*before(const ItemDataDictCEntry*) const;

/* Functions to support sorting */

   static int	compare(const void*, const void*);
   void		sort();
   void		sort(int (*)(const void*, const void*));
   void		SetSorted(char);

   inline unsigned	size()		const { return _count; }
   inline unsigned	capacity()	const { return _space; }
   inline ItemDataDictCEntry		**start()	const { return _list; }
   void         	AllowDuplicates(char);
   void			AutoShrink(char);
   void			SetCapacity(unsigned);

   ItemDataDictCEntry			*operator[](unsigned) const;
   ItemDataDictCEntry			*First() const;
   ItemDataDictCEntry			*Last() const;

   ostream&		printOn(ostream& strm=cout) const;
};

/*-------------------------------------------------------------*/
/* PRINTING */

inline ostream&
operator<<(ostream& strm, const ItemDataDictCEntryList& c)
{
   return c.printOn(strm);
}

/*
 *  PDictC.clas -- class definition for PDictC
 *
 *   This type of dictionary stores only pointers to the keys and values
 *   ODictC.h stores the actual keys and values
 */

/* Create ItemDataDictC as a derived class of ItemDataDictCEntryList */

class ItemDataDictC : public ItemDataDictCEntryList {

public:

   inline ItemDataDictC() {}
   virtual inline ~ItemDataDictC() {}

/* Build this dictionary from another */
   inline ItemDataDictC(const ItemDataDictC& d) { *this = d; }

/* Copy one dictionary to another */

   ItemDataDictC&	operator=(const ItemDataDictC& d);

/* Lookup up entries */

   int			indexOf (const VItemC& key) const;
   int			indexOf (const VItemC* kp) const;
   int			indexOf (const ItemDataDictCEntry& entry) const;
   int			indexOf (const ItemDataDictCEntry* entry) const;
   int			includes(const VItemC& key) const;
   int			includes(const VItemC* kp) const;
   int			includes(const ItemDataDictCEntry& entry) const;
   int			includes(const ItemDataDictCEntry* entry) const;

   ItemDataC		*definitionOf(const VItemC& key) const;
   ItemDataC		*definitionOf(const VItemC* kp) const;

   ItemDataC		*valOf(const int index) const;

   VItemC      	 *keyOf(const int index) const;

   ItemDataDictCEntry		*entryOf(const VItemC& key) const;
   ItemDataDictCEntry		*entryOf(const VItemC* kp) const;

/* Changing entries */

   ItemDataDictCEntry		*add   (const VItemC* kp, const ItemDataC* vp);
   ItemDataDictCEntry		*modify(const VItemC& key, const ItemDataC* vp);
   ItemDataDictCEntry		*modify(const VItemC* kp, const ItemDataC* vp);
   ItemDataDictCEntry		*modify(unsigned index, const ItemDataC* vp);

/* Deleting entries */

   void			remove(const VItemC& key);
   void			remove(const VItemC* kp);
   void			remove(unsigned index);
   void			remove(const ItemDataDictCEntry& entry);
   void			remove(const ItemDataDictCEntry* entry);

/* Printing */

   void			printOn(ostream& strm=cout) const;
};

inline ostream& operator<<(ostream& strm, const ItemDataDictC& d)
{
   d.printOn(strm);
   return(strm);
}

#endif // _ItemDataDictC_h_
