//
// IconDataDictC.h
//

#ifndef _IconDataDictC_h_
#define _IconDataDictC_h_

#include "VItemC.h"
#include "IconDataC.h"

/*
 *  PDictCEntry.clas -- class definition for an entry in a PDictC
 *
 *   This type of dictionary stores only pointers to the keys and values
 */

class IconDataDictCEntry {

public:

   VItemC	*key;
   IconDataC	*val;

   inline IconDataDictCEntry () { key = (VItemC*)NULL; val = (IconDataC*)NULL; }
   inline IconDataDictCEntry (const VItemC* kp, const IconDataC* vp)
				 { key = (VItemC *)kp; val = (IconDataC *)vp; }
   inline IconDataDictCEntry (const IconDataDictCEntry& e) { *this = e; }

   inline IconDataDictCEntry&	operator= (const IconDataDictCEntry& e) {
      if ( this != &e ) {
	 key = e.key;
	 val = e.val;
      }
      return *this;
   }

   inline int	operator== (const IconDataDictCEntry& e) const {
   /* Only need to check pointers here */
      return (key == e.key && val == e.val);
   }
   inline int	operator!= (const IconDataDictCEntry& e) const {
      return !(*this==e);
   }
   inline int	compare  (const IconDataDictCEntry&) const { return 0; }
   inline int	operator<(const IconDataDictCEntry&) const { return 0; }
   inline int	operator>(const IconDataDictCEntry&) const { return 0; }

   inline ostream&  printOn(ostream& strm=cout) const {
      strm <<key <<" -> " <<val;
      return strm;
   }
};

inline ostream& operator<<(ostream& strm, const IconDataDictCEntry& e)
{
   return e.printOn(strm);
}

/*
 * PListC.clas -- class definition file for class PListC
 *
 *   This type of list stores only pointers to its members
 *   OListC stores the members themselves
 */

class IconDataDictCEntryList {

protected:

   IconDataDictCEntry			**_list;
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

   IconDataDictCEntryList();
   IconDataDictCEntryList(unsigned ga);
   IconDataDictCEntryList(const IconDataDictCEntryList&);
   ~IconDataDictCEntryList();

/* Append another list to this one */
   IconDataDictCEntryList& operator+=(const IconDataDictCEntryList&);

/* Copy another list to this one */
   IconDataDictCEntryList& operator=(const IconDataDictCEntryList&);

/* Return the position of the specified object.  Object must be in list */

   int		indexOf(const IconDataDictCEntry&) const;
   int		indexOf(const IconDataDictCEntry*) const;

/* Return the sorted position of the specified object.  Object does not */
/*    have to be in the list */

   unsigned		orderOf(const IconDataDictCEntry&) const;
   unsigned		orderOf(const IconDataDictCEntry*) const;

/* Return non-zero if the specified object is in the list */

   int		includes(const IconDataDictCEntry&) const;
   int		includes(const IconDataDictCEntry*) const;

/* Add the specified object to the list in the requested position */
/*    The position is ignored if this list is sorted */

   IconDataDictCEntry		*insert (const IconDataDictCEntry*, unsigned);
   IconDataDictCEntry		*replace(const IconDataDictCEntry*, unsigned);
   IconDataDictCEntry		*append (const IconDataDictCEntry*);
   IconDataDictCEntry		*prepend(const IconDataDictCEntry*);
   IconDataDictCEntry		*add    (const IconDataDictCEntry*);

/* Remove the specified object or index from the list */

   void		remove(unsigned);
   void		remove(const IconDataDictCEntry&);
   void		remove(const IconDataDictCEntry*);
   void		removeNulls();
   void		removeAll();
   void		removeLast();

/* Return a pointer to the previous object in the list */

   IconDataDictCEntry		*before(const IconDataDictCEntry*) const;

/* Functions to support sorting */

   static int	compare(const void*, const void*);
   void		sort();
   void		sort(int (*)(const void*, const void*));
   void		SetSorted(char);

   inline unsigned	size()		const { return _count; }
   inline unsigned	capacity()	const { return _space; }
   inline IconDataDictCEntry		**start()	const { return _list; }
   void         	AllowDuplicates(char);
   void			AutoShrink(char);
   void			SetCapacity(unsigned);

   IconDataDictCEntry			*operator[](unsigned) const;
   IconDataDictCEntry			*First() const;
   IconDataDictCEntry			*Last() const;

   ostream&		printOn(ostream& strm=cout) const;
};

/*-------------------------------------------------------------*/
/* PRINTING */

inline ostream&
operator<<(ostream& strm, const IconDataDictCEntryList& c)
{
   return c.printOn(strm);
}

/*
 *  PDictC.clas -- class definition for PDictC
 *
 *   This type of dictionary stores only pointers to the keys and values
 *   ODictC.h stores the actual keys and values
 */

/* Create IconDataDictC as a derived class of IconDataDictCEntryList */

class IconDataDictC : public IconDataDictCEntryList {

public:

   inline IconDataDictC() {}
   virtual inline ~IconDataDictC() {}

/* Build this dictionary from another */
   inline IconDataDictC(const IconDataDictC& d) { *this = d; }

/* Copy one dictionary to another */

   IconDataDictC&	operator=(const IconDataDictC& d);

/* Lookup up entries */

   int			indexOf (const VItemC& key) const;
   int			indexOf (const VItemC* kp) const;
   int			indexOf (const IconDataDictCEntry& entry) const;
   int			indexOf (const IconDataDictCEntry* entry) const;
   int			includes(const VItemC& key) const;
   int			includes(const VItemC* kp) const;
   int			includes(const IconDataDictCEntry& entry) const;
   int			includes(const IconDataDictCEntry* entry) const;

   IconDataC		*definitionOf(const VItemC& key) const;
   IconDataC		*definitionOf(const VItemC* kp) const;

   IconDataC		*valOf(const int index) const;

   VItemC      	 *keyOf(const int index) const;

   IconDataDictCEntry		*entryOf(const VItemC& key) const;
   IconDataDictCEntry		*entryOf(const VItemC* kp) const;

/* Changing entries */

   IconDataDictCEntry		*add   (const VItemC* kp, const IconDataC* vp);
   IconDataDictCEntry		*modify(const VItemC& key, const IconDataC* vp);
   IconDataDictCEntry		*modify(const VItemC* kp, const IconDataC* vp);
   IconDataDictCEntry		*modify(unsigned index, const IconDataC* vp);

/* Deleting entries */

   void			remove(const VItemC& key);
   void			remove(const VItemC* kp);
   void			remove(unsigned index);
   void			remove(const IconDataDictCEntry& entry);
   void			remove(const IconDataDictCEntry* entry);

/* Printing */

   void			printOn(ostream& strm=cout) const;
};

inline ostream& operator<<(ostream& strm, const IconDataDictC& d)
{
   d.printOn(strm);
   return(strm);
}

#endif // _IconDataDictC_h_
