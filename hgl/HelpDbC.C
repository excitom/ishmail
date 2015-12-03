/*
 * $Id: HelpDbC.C,v 1.2 2000/05/07 12:26:10 fnevgeny Exp $
 *
 * Copyright (c) 1994 HAL Computer Systems International, Ltd.
 * 
 *          HAL COMPUTER SYSTEMS INTERNATIONAL, LTD.
 *                  1315 Dell Avenue
 *                  Campbell, CA  95008
 *
 * Author: Greg Hilton
 * Contributors: Tom Lang, Frank Bieser, and others
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * http://www.gnu.org/copyleft/gpl.html
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <config.h>

#include "HalAppC.h"
#include "HelpDbC.h"
#include "MemMap.h"
#include "RegexC.h"
#include "SysErr.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

extern int	debuglev;

/*-------------------------------------------------------------------------
 * Constructor
 */

HelpDbC::HelpDbC(const char *name)
{
   loaded = False;
   dbMap  = NULL;

   char	*path = NULL;

   if ( debuglev > 0 ) cout <<"Looking for help database file: " <<name <<endl;

//
// Try using the HELPCARD_PATH variable
//
   const char *helpcard_path = getenv("HELPCARD_PATH");
   if ( helpcard_path ) {
      if ( debuglev > 1 ) cout <<"Looking in path: " <<helpcard_path <<endl;
      path = XtResolvePathname(halApp->display, "help", name, NULL,
			       helpcard_path, NULL, 0, NULL);
   }

//
// If we don't have a path, try the default search path
//
   if ( !path ) {
      if ( debuglev > 1 ) cout <<"Trying default search path." <<endl;
      path = XtResolvePathname(halApp->display, "help", name, NULL, NULL, NULL,
			       0, NULL);
   }

//
// If we still didn't find it, see if we can just access it directly
//
   if ( !path && access(name, R_OK) == 0 ) {
      if ( debuglev > 1 ) cout <<"Trying direct access." <<endl;
      path = (char*)name;
   }

//
// If we still didn't find it, punt
//
   if ( !path ) {
      if ( debuglev > 0 ) cout <<"Could not find help file." <<endl;
      return;
   }

   if ( debuglev > 0 ) cout <<"Found help file: " <<path <<endl;

   file = path;
   if ( path != name ) XtFree(path);

//
// Now let's try to map the file
//
   dbMap = MapFile(file);
   if ( !dbMap ) {
      cerr <<"Could not map help file: " <<name <<endl;
      return;
   }

//
// See which format is used.  If the first line starts with [CARD] it's the new
//    format
//
   if ( debuglev > 0 ) cout <<"Reading help cards" <<endl;

   if ( dbMap->data.StartsWith("[CARD] ") || dbMap->data.Length() == 0 )
      ReadNewDb();
   else
      ReadOldDb();

   UnmapFile(dbMap);
   dbMap = NULL;

//
// Sort the cards by title
//
   if ( debuglev > 0 ) cout <<"Sorting help cards" <<endl;

   cardList.sort((int (*)(const void*, const void*))CompareTitles);

   if ( debuglev > 0 ) cout <<"Finished with " <<file <<endl;

   loaded = True;

} // End constructor

/*-------------------------------------------------------------------------
 * Method to read a new format helpcard data file
 */

void
HelpDbC::ReadNewDb()
{
   if ( debuglev > 0 ) cout <<"Reading new format help database." <<endl;
   CharC	data = dbMap->data;

//
// Loop through cards
//
   int		pos = data.PosOf("[CARD] ");
   int		nlPos;
   while ( pos >= 0 ) {

//
// Create a new card and add it to the list
//
      HelpCardC	*card = new HelpCardC;
      void	*tmp = (void*)card;
      cardList.add(tmp);
      card->db = this;

//
// Read the card name
//
      pos += strlen("[CARD] ");
      nlPos = data.PosOf('\n', (u_int)pos);
      if ( nlPos <= pos ) {
	 cerr <<"Missing helpcard name at offset " <<pos
	      <<" in file " <<file <<endl;
	 goto NEXT_CARD;
      }

      card->name = data(pos, nlPos-pos);

//
// Read the card title
//
      pos = nlPos + 1;
      if ( !data.StartsWith("[TITLE] ", (u_int)pos) ) {
	 cerr <<"Missing helpcard title at offset " <<pos
	      <<" in file " <<file <<endl;
	 goto NEXT_CARD;
      }

      pos += strlen("[TITLE] ");
      nlPos = data.PosOf('\n', (u_int)pos);
      if ( nlPos <= pos ) {
	 cerr <<"Missing helpcard title at offset " <<pos
	      <<" in file " <<file <<endl;
	 goto NEXT_CARD;
      }

      card->title = data(pos, nlPos-pos);

//
// Read the locator if present
//
      pos = nlPos + 1;
      if ( data.StartsWith("[LOCATOR] ", (u_int)pos) ) {

	 pos += strlen("[LOCATOR] ");
	 nlPos = data.PosOf('\n', (u_int)pos);
	 if ( nlPos <= pos ) {
	    cerr <<"Missing locator at offset " <<pos
		 <<" in file " <<file <<endl;
	    goto NEXT_CARD;
	 }

	 card->locator = data(pos, nlPos-pos);
	 pos = nlPos + 1;
      }

NEXT_CARD:

//
// Mark the location of the text
//
      card->offset = pos;
      if ( debuglev > 1 )
	 cout <<"Found " <<card->name <<" at " <<card->offset <<endl;

      int	nextPos = data.PosOf("[CARD] ", (u_int)pos);
      if ( nextPos > pos )
	 card->length = nextPos - pos;
      else
	 card->length = data.Length() - pos;

      pos = nextPos;

   } // End for each [CARD] tag

} // End ReadNewDb

/*-------------------------------------------------------------------------
 * Method to read an old format helpcard data file
 */

void
HelpDbC::ReadOldDb()
{
   if ( debuglev > 0 ) cout <<"Reading old format help database." <<endl;
//
// Look for the TABLE-OFFSET line at the end of the file
//
   int		pos = dbMap->data.RevPosOf("[TABLE-OFFSET]:");
   long		tableOffset = 0;
   StringC	tmpStr;
   if ( pos < 0 ) {
      cerr <<"Could not find [TABLE-OFFSET] tag in helpcard data file: " <<file
	   <<endl;
   }

   else {
      tmpStr = dbMap->data.NextWord(pos+15);
      tableOffset = (u_int)atol(tmpStr);
      if ( tableOffset < 0 ) {
	 cerr <<"Invalid [TABLE-OFFSET] tag \"" <<tmpStr
	      <<"\" in helpcard data file: " <<file <<endl;
	 tableOffset = 0;
      }
   }

//
// Look for the [NUM-HELPCARDS] line
//
   pos = dbMap->data.PosOf("[NUM-HELPCARDS]:", (u_int)tableOffset);
   if ( pos < 0 && tableOffset > 0 )
      pos = dbMap->data.PosOf("[NUM-HELPCARDS]:");

   int	cardCount = 0;
   if ( pos < 0 ) {
      cerr <<"Could not find [NUM-HELPCARDS] tag in helpcard data file: " <<file
	   <<endl;
   }
   else {
      tmpStr = dbMap->data.NextWord(pos+16);
      int	cardCount = atoi(tmpStr);
      if ( cardCount < 0 ) {
	 cerr <<"Invalid [NUM-HELPCARDS] tag \"" <<tmpStr
	      <<"\" in helpcard data file: " <<file <<endl;
	 cardCount = 0;
      }
   }

//
// Look for the [BEGIN-TABLE] line
//
   pos = dbMap->data.PosOf("[BEGIN-TABLE]\n", (u_int)pos);
   if ( pos < 0 ) pos = dbMap->data.PosOf("[BEGIN-TABLE]\n");

   if ( pos < 0 ) {
      cerr <<"Bogus helpcard data file: " <<file
	   <<"\nCould not find [BEGIN-TABLE] tag." <<endl;
   }
   else
      pos += 14;

//
// Read in helpcard data.  Format is "Name:Offset:Locator:Length"
//
   CharC	line;
   RegexC	tablePat("\\(.*\\):\\(.*\\):\\(.*\\):\\(.*\\)");
   Boolean	done = False;

   if ( cardCount > 0 ) cardList.GROWTH_AMOUNT = cardCount;
   int	i = 0;
   while ( !done ) {

//
// Read the next line
//
      line = dbMap->data.NextWord(pos, '\n');
      pos += line.Length() + 1;

      if ( line.Length() <= 0 ) {
	 cerr <<"Bogus helpcard data file: " <<file
	      <<"\nUnexpected EOF while trying to read helpcard number "
	      <<i <<endl;
	 done = True;
	 continue;
      }

//
// Watch for end of list
//
      if ( line.Equals("[END-TABLE]") ) {
	 done = True;
	 continue;
      }

      if ( !tablePat.match(line) ) {
	 cerr <<"Bogus helpcard data file: " <<file
	      <<"\nUnexpected line while trying to read helpcard number " <<i
	      <<":\n" <<line <<endl;
	 done = True;
	 continue;
      }

//
// Create a new card
//
      HelpCardC	*card = new HelpCardC;

      card->db      = this;
      card->name    = line(tablePat[1]);
      tmpStr        = line(tablePat[2]);
      card->offset  = atol(tmpStr);
      card->locator = line(tablePat[3]);
      tmpStr        = line(tablePat[4]);
      card->length  = atol(tmpStr);
      if ( debuglev > 1 )
	 cout <<"Found " <<card->name <<" at " <<card->offset <<endl;

//
// Add this card to the list
//
      void	*tmp = (void*)card;
      cardList.add(tmp);

      i++;

   } // End for each card

//
// Look for the [BEGIN-INDEX] line
//
   pos = dbMap->data.PosOf("[BEGIN-INDEX]\n", (u_int)pos);
   if ( pos < 0 ) pos = dbMap->data.PosOf("[BEGIN-INDEX]\n");

   if ( pos < 0 ) {
      cerr <<"Bogus helpcard data file: " <<file
	   <<"\nCould not find [BEGIN-INDEX] tag." <<endl;
   }
   else
      pos += 14;

//
// Read index entries.  Format is "Title:Name"
//
   RegexC	indexPat("\\(.*\\):\\(.*\\)");
   CharC	title;
   CharC	cname;

   done = False;
   i = 0;
   while ( !done ) {

//
// Read the next line
//
      line = dbMap->data.NextWord(pos, '\n');
      pos += line.Length() + 1;

      if ( line.Length() <= 0 ) {
	 cerr <<"Bogus helpcard data file: " <<file
	      <<"\nUnexpected EOF while trying to read index entry "
	      <<i <<endl;
	 done = True;
	 continue;
      }

//
// Watch for end of list
//
      if ( line.Equals("[END-INDEX]") ) {
	 done = True;
	 continue;
      }

      if ( !indexPat.match(line) ) {
	 cerr <<"Bogus helpcard data file: " <<file
	      <<"\nUnexpected line while trying to read index entry " <<i
	      <<":\n" <<line <<endl;
	 done = True;
	 continue;
      }

//
// Read title and card name
//
      title = line(indexPat[1]);
      cname = line(indexPat[2]);

//
// Look up card and store title
//
      HelpCardC	*card = FindCard(cname);
      if ( card ) card->title = title;

   } // End for each index entry

} // End ReadOldDb

/*-------------------------------------------------------------------------
 * Destructor
 */

HelpDbC::~HelpDbC()
{
   DeleteCards();
   if ( dbMap ) UnmapFile(dbMap);
}

/*-------------------------------------------------------------------------
 * Method to delete helpcard records
 */

void
HelpDbC::DeleteCards()
{
   unsigned	count = cardList.size();
   for (int i=0; i<count; i++) {
      HelpCardC	*card = (HelpCardC*)*cardList[i];
      delete card;
   }

   cardList.removeAll();
}

/*-----------------------------------------------------------------------
 *  Compare function for help cards
 */

int
HelpDbC::CompareTitles(const void *a, const void *b)
{
   HelpCardC       *hca = *(HelpCardC **)a;
   HelpCardC       *hcb = *(HelpCardC **)b;

   return ( hca->title.compare(hcb->title) );
}

/*-------------------------------------------------------------------------
 * Method to look for the card with the specified name
 */

HelpCardC*
HelpDbC::FindCard(const CharC name) const
{
//
// Loop through the card list
//
   HelpCardC	*card = NULL;
   unsigned	count = cardList.size();
   for (int i=0; !card && i<count; i++) {
      HelpCardC	*card = (HelpCardC*)*cardList[i];
      if ( card->name == name ) return card;
   }

   return NULL;
}

/*-------------------------------------------------------------------------
 * Method to return the text for this card
 */

Boolean
HelpCardC::GetText(StringC& text) const
{
   text.Clear();

   if ( debuglev > 1 ) {
      cout <<"Getting text for helpcard " <<name <<endl;
      cout <<"at offset " <<offset <<" in file " <<db->file <<endl;
   }

   FILE	*fp = fopen(db->file, "r");
   if ( !fp ) {
      text = "No database for help file: ";
      text += db->file;
      text += '\n';
      text += SystemErrorMessage(errno);
      return False;
   }

   if ( fseek(fp, offset, SEEK_SET) != 0 ) {
      text = "Could not seek to offset ";
      text += (int)offset;
      text += " in help file: ";
      text += db->file;
      text += '\n';
      text += SystemErrorMessage(errno);
      fclose(fp);
      return False;
   }

//
// Read the first line
//
   if ( !text.GetLine(fp) ) {
      text = "Could not read help file: ";
      text += db->file;
      text += '\n';
      text += SystemErrorMessage(errno);
      fclose(fp);
      return False;
   }

//
// Look for "[TITLE]:".  If it's there this is an old-style card.
//
   if ( text.StartsWith("[TITLE]:") ) {

//
// Skip [BEGIN-TEXT]
//
      if ( !text.GetLine(fp) ) {
	 text = "Could not read help file: ";
	 text += db->file;
	 text += '\n';
	 text += SystemErrorMessage(errno);
	 fclose(fp);
	 return False;
      }

//
// Read up to [END-TEXT]
//
      text.Clear();
      StringC	line;
      while ( line != "[END-TEXT]" && text.size() <= length ) {

	 line.Clear();
	 if ( !line.GetLine(fp) ) {
	    text = "Could not read help file: ";
	    text += db->file;
	    text += '\n';
	    text += SystemErrorMessage(errno);
	    fclose(fp);
	    return False;
	 }

	 if ( line != "[END-TEXT]" ) {
	    text += line;
	    text += '\n';
	 }
      }

   } // End if this is an old-style card

   else {

      text += '\n';
      if ( length > text.size() ) {
	 u_int	len = (u_int)length - text.size();
	 if ( !text.AppendFile(fp, len) ) {
	    text = "Could not read help file: ";
	    text += db->file;
	    text += '\n';
	    text += SystemErrorMessage(errno);
	    fclose(fp);
	    return False;
	 }
      }
      else {
	 text.Clear((int)length);
      }

   } // End if this is a new-style card

   fclose(fp);
   return True;

} // End GetText
