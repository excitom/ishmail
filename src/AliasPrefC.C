/*
 *  $Id: AliasPrefC.C,v 1.3 2000/05/07 12:26:11 fnevgeny Exp $
 *  
 *  Copyright (c) 1994 HAL Computer Systems International, Ltd.
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
#include "IshAppC.h"
#include "AliasPrefC.h"
#include "AliasPrefWinC.h"
#include "ShellExp.h"
#include "Misc.h"
#include "CompPrefC.h"
#include "AddressC.h"

#include <hgl/rsrc.h>
#include <hgl/CharC.h>
#include <hgl/RegexC.h>
#include <hgl/RangeC.h>

#include <stdio.h>

/*---------------------------------------------------------------
 *  Constructor
 */

AliasPrefC::AliasPrefC() : PrefC()
{
   aliasWin = NULL;
   groupWin = NULL;

//
// Example mailrc alias:
//	alias ishmail support@ishmail.com
//
   aliasPat = new RegexC("^alias[ \t]+\\([^ \t]+\\)[ \t]+\\(.*\\)");

//
// Example elm alias:
//	ishmail = Ishmail Technical Support = support@ishmail.com
//
   elmAliasPat = new RegexC("^\\([^= \t]+\\)[ \t]*=[ \t]*\\([^=]*\\)[ \t]*=[ \t]*\\(.*\\)");

//
// Example pine alias:
//	ishmail Ishmail Technical Support (support@ishmail.com)
//
   pineAliasPat = new RegexC("^\\([^\t]+\\)\t\\([^\t]*\\)\t\\((?[^)]*)?\\)");

//
// Used for converting aliases to mailrc format and back
//
   namePat  = new RegexC("\\([ \t]*\\)\\([^,]+\\),?");

        aliasDict.AllowDuplicates(TRUE);
        aliasDict.SetSorted(FALSE);
   groupAliasDict.AllowDuplicates(TRUE);
   groupAliasDict.SetSorted(FALSE);
   otherAliasDict.AllowDuplicates(TRUE);
   otherAliasDict.SetSorted(FALSE);

//
// Read resources
//
   mailrcFile        = ishApp->home + "/" + ".mailrc";
   orig.mailrcFile   = get_string(*halApp, "mailrcFile", mailrcFile);

   groupMailrcFile   = ishHome + "/lib/" + "mailrc";
   orig.groupMailrcFile = get_string(*halApp, "groupMailrcFile",
				     groupMailrcFile);

   orig.otherAliasStr = get_string(*halApp, "otherAliasFiles");

   mailrcFile      = orig.mailrcFile;		ShellExpand(mailrcFile);
   groupMailrcFile = orig.groupMailrcFile;	ShellExpand(groupMailrcFile);

   ExtractList(orig.otherAliasStr, otherAliasFiles);
   ExpandList(otherAliasFiles);

//
// Read mailrc files
//
   ReadAliasFile(mailrcFile,      aliasDict,      False/*no others*/ );
   ReadAliasFile(groupMailrcFile, groupAliasDict, False/*no others*/ );

//
// Read other alias files
//
   unsigned count = otherAliasFiles.size();
   for (int i=0; i<count; i++)
      ReadAliasFile(*otherAliasFiles[i], otherAliasDict, True/*allow others*/);

//
// Read list of aliases that are not to be expanded
//
   StringC	expandStr = get_string(*halApp, "hideAddresses");
   ExtractList(expandStr, hideAddrList);

} // End constructor

/*---------------------------------------------------------------
 *  destructor
 */

AliasPrefC::~AliasPrefC()
{
   delete aliasPat;
   delete elmAliasPat;
   delete pineAliasPat;
   delete namePat;
   delete aliasWin;
   delete groupWin;

} // End destructor

/*---------------------------------------------------------------
 *  Method to write resources to resource database
 */

Boolean
AliasPrefC::WriteDatabase()
{
   Store("mailrcFile",		orig.mailrcFile);
   Store("groupMailrcFile",	orig.groupMailrcFile);
   Store("otherAliasFiles",	orig.otherAliasStr);
   Store("sortAliases",		sortAliases);
   Store("hideAddresses",	hideAddrList);

   return True;

} // End WriteDatabase

/*---------------------------------------------------------------
 *  Method to write resources to a file
 */

Boolean
AliasPrefC::WriteFile()
{
   if ( !WriteAliases(mailrcFile, aliasDict) )
      return False;

   if ( !WriteAliases(groupMailrcFile, groupAliasDict) )
      return False;

   StringListC	lineList;
   ReadResFile(lineList);

   Update(lineList, "mailrcFile",	orig.mailrcFile);
   Update(lineList, "groupMailrcFile",	orig.groupMailrcFile);
   Update(lineList, "otherAliasFiles",	orig.otherAliasStr);
   Update(lineList, "sortAliases",	sortAliases);
   Update(lineList, "hideAddresses",	hideAddrList);

//
// Write the information back to the file
//
   return WriteResFile(lineList);

} // End WriteFile

/*---------------------------------------------------------------
 *  Method to read aliases out of a file
 */

void
AliasPrefC::ReadAliasFile(const char *aFile, StringDictC& aDict,
			  Boolean otherFmt)
{
   if ( debuglev > 0 ) cout <<"Reading aliases from " <<aFile <<endl;

   StringC	line; line.AutoShrink(False);
   StringC	key;
   StringC	val;

   FILE	*fp = fopen(aFile, "r");
   if ( !fp ) return;	// if file unreadable, don't worry about it

//
// Process lines in the file
//
   while ( line.GetLine(fp) != EOF ) {

      line.Trim();	// Remove whitspace

//
// If the newline is escaped, read some more
//
      if ( line.EndsWith("\\") ) {
	 line.Clear(line.size()-1);
	 continue;
      }

//
// See if this is an alias line
//
      if ( aliasPat->match(line) ) {

	 key = line((*aliasPat)[1]);
	 val = line((*aliasPat)[2]);
	 ConvertMailrcToAlias(val);

	 aDict.add(key, val);
	 if ( debuglev > 1 )
	    cout <<"   Found mailrc alias: " <<key <<": " <<val <<endl;

      } // End if an alias was found

//
// Conditionally see if this is an elm alias line
//
      else if ( otherFmt && elmAliasPat->match(line) ) {

	 key = line((*elmAliasPat)[1]);
	 val = line((*elmAliasPat)[3]);
	 RangeC	r2 = (*elmAliasPat)[2];
	 if ( r2.length() > 0 ) val += " (" + line(r2) + ")";

	 aDict.add(key, val);
	 if ( debuglev > 1 )
	    cout <<"   Found elm alias: " <<key <<": " <<val <<endl;

      } // End if elm alias found

//
// Conditionally see if this is a pine alias line
//
      else if ( otherFmt && pineAliasPat->match(line) ) {

	 key = line((*pineAliasPat)[1]);
	 val = line((*pineAliasPat)[3]);

//
// If there is a left paren, make sure we've read the right paren
//
	 val.Trim();
	 if ( val.StartsWith('(') && !val.EndsWith(')') )
	    continue;

	 if ( val.StartsWith('(') && val.EndsWith(')') )
	    val.CutBoth(1);

	 RangeC	r2 = (*pineAliasPat)[2];
	 if ( r2.length() > 0 ) val += " (" + line(r2) + ")";

	 aDict.add(key, val);
	 if ( debuglev > 1 )
	    cout <<"   Found pine alias: " <<key <<": " <<val <<endl;

      } // End if pine alias found

      line.Clear();

   } // End while not EOF

   fclose(fp);

} // End ReadAliasFile

/*---------------------------------------------------------------
 *  Method to remove unnecessary quotes from alias strings and to add commas
 */

void
AliasPrefC::ConvertMailrcToAlias(StringC& val)
{
//
// Add commas
//
   AddAliasCommas(val);

//
// Loop through addresses
//
   CharC	name;
   int		findPos, startPos = 0;
   RangeC	range0, range2;
   while ( (findPos=namePat->search(val,startPos)) >= 0 ) {

      range0 = (*namePat)[0];
      range2 = (*namePat)[2];
      startPos = findPos + range0.length();
      name = val(range2);

//
// If this alias starts and ends with quotes, remove them
//
      if ( (name.StartsWith('"')  && name.EndsWith('"')) ||
           (name.StartsWith('\'') && name.EndsWith('\'')) ) {
	 name.CutBoth(1);
	 val(range2) = name;
	 startPos -= 2;
      }

   } // End for each address in the alias
   
} // End ConvertMailrcToAlias

/*---------------------------------------------------------------
 *  Method to add necessary commas to alias string
 *     address
 *    <address>
 *   "<address>  comment"
 *   "<address> (comment)"
 *    "comment  <address>"
 *   "(comment) <address>"
 *    "comment  <address>  comment"
 *    "comment  <address> (comment)"
 *   "(comment) <address>  comment"
 *   "(comment) <address> (comment)"
 *    "address  (comment)"
 *   "(comment)  address"
 */

void
AliasPrefC::AddAliasCommas(StringC& inStr)
{
   StringC	outStr;
   unsigned	count = inStr.size();
   int		comment = 0;
   Boolean	gotSpace = False;
   char		lastChar = 0;
   Boolean	squoted = False;
   Boolean	dquoted = False;

   for (int i=0; i<count; i++ ) {

      char	c = inStr[i];
      switch (c) {

         case '(':
	    comment++;
	    if ( gotSpace ) {
	       outStr += (char)' ';
	       gotSpace = False;
	    }
	    outStr += c;
	    lastChar = c;
	    break;

	 case ')':
	    if ( comment>0 ) {
	       comment--;
	       outStr += c;
	       lastChar = c;
	    }
	    gotSpace = False;
	    break;

	 case ' ':
	 case '\t':
	    if ( comment==0 && !squoted && !dquoted ) gotSpace = True;
	    else {
	       outStr += c;
	       lastChar = c;
	    }
	    break;

	 default:
	    if ( gotSpace ) {
	       if ( lastChar != 0 ) {
		  if ( lastChar != ',' ) outStr += (char)',';
		  outStr += (char)' ';
	       }
	       gotSpace = False;
	    }
	    if      ( c == '"'  ) dquoted = !dquoted;
	    else if ( c == '\'' ) squoted = !squoted;
	    outStr += c;
	    lastChar = c;

      } // End switch current character

   } // End for each character

//
// Check for space in last character
//
   if ( gotSpace ) outStr += (char)' ';

//
// Return the resulting string
//
   inStr = outStr;

} // End AddAliasCommas

/*---------------------------------------------------------------
 *  Method to write resources to a file
 */

Boolean
AliasPrefC::WriteAliases(const char *file, StringDictC& dict)
{
   StringListC	lineList;

//
// Try to open file for reading and writing
//
   if ( debuglev > 0 ) cout <<"Opening mailrc file: " <<file <<endl;
   FILE	*fp = fopen(file, "r+");

//
// If the file was opened, read the lines into a list
//
   if ( fp ) {

      StringC	line;
      lineList.AllowDuplicates(TRUE);
      while ( line.GetLine(fp) != EOF ) {

//
// If the newline is escaped, read some more.
//
	 if ( line.EndsWith("\\") )
	    line.Clear(line.size()-1);
	 else {
	    lineList.add(line);
	    line.Clear();
	 }
      }
      fclose(fp);

   } // End if file found

   UpdateAliases(lineList, dict);

//
// Write the information back to the file
//
   fp = fopen(file, "w");
   if ( !fp ) return False;

   unsigned	count = lineList.size();
   for (int i=0; i<count; i++)
      fprintf(fp, "%s\n", (char *)(*lineList[i]));

   fclose(fp);
   return True;

} // End WriteAliases

/*---------------------------------------------------------------
 *  Method to update the resource line list
 */

void
AliasPrefC::UpdateAliases(StringListC& lineList, StringDictC& dict)
{
//
// Remove any alias lines that aren't in the dictionary
//
   unsigned	lcount = lineList.size();
   StringC	key;
   for (int l=lcount-1; l>=0; l--) {

      StringC	*line = lineList[l];
      StringC	*prev = (l>0) ? lineList[l-1] : (StringC*)NULL;

      if ( aliasPat->match(*line) ) {

	 key = (*line)((*aliasPat)[1]);
	 if ( !dict.includes(key) ) {

	    lineList.remove(l);

	 } // End if alias not in dictionary
      } // End if line is an alias

   } // End for each line

//
// Modify any lines that have changed or add any that are missing
//
   unsigned	acount = dict.size();
   StringC	val;
   for (int a=0; a<acount; a++) {

      StringC&	key = dict[a]->key;
      val = dict[a]->val;

//
// Add quotes to value where needed and remove commas
//
      ConvertAliasToMailrc(val);

//
// Look at each line
//
      lcount = lineList.size();
      Boolean	found = False;
      for (int l=0; !found && l<lcount; l++) {

	 StringC	*lineP = lineList[l];
	 if ( aliasPat->match(*lineP) && (*lineP)((*aliasPat)[1]) == key ) {

//
// Found a match
//
	    if ( debuglev > 0 ) cout <<"   Modifying line: " <<*lineP <<endl;
	    *lineP = "alias " + key + "\t" + val;
	    if ( debuglev > 0 ) cout <<"   New line is   : " <<*lineP <<endl;
	    found = True;
	 }

      } // End for each line

//
// Add a new line if none was found
//
      if ( !found ) {

	 StringC	line = "alias " + key + "\t" + val;
	 lineList.add(line);
	 if ( debuglev > 0 ) cout <<"   Added new line" <<endl;
      }

   } // End for each alias

} // End UpdateAliases

/*---------------------------------------------------------------
 *  Function to add quotes to alias strings and to remove commas
 */

void
AliasPrefC::ConvertAliasToMailrc(StringC& val)
{
//
// Look through names
//
   StringC	name;
   int		findPos, startPos = 0;
   int		commaPos;
   RangeC	range0;
   RangeC	range1;
   RangeC	range2;
   while ( (findPos=namePat->search(val,startPos)) >= 0 ) {

      range0 = (*namePat)[0];
      range1 = (*namePat)[1];
      range2 = (*namePat)[2];

      startPos = findPos + range0.length();
      commaPos = range0.lastIndex();
      name = val(range2);

//
// If the name contains whitespace, it must be quoted.  Doing this will also
//    remove any commas
//
      if ( (strchr((char *)name, ' ') || strchr((char *)name, '\t')) && name[0] != '"' ) {
	 val(range0) = val(range1) + "\"" + name + "\"";
	 startPos += 2;
      }

//
// Remove the comma if present
//
      else if ( val[commaPos] == ',' ) {

//
// Make sure there's whitespace after
//
	 if ( commaPos+1 < val.size() && val[commaPos+1] == ' ' ) {
	    val(commaPos,1) = "";
	    startPos--;
	 }
	 else
	    val(commaPos,1) = " ";
      }

   } // End for each address in the alias

} // End ConvertAliasToMailrc

/*------------------------------------------------------------------------
 * Method to edit the aliases
 */

void
AliasPrefC::Edit(Widget parent)
{
   halApp->BusyCursor(True);

   if ( !aliasWin ) aliasWin = new AliasPrefWinC(parent, aliasDict);
   aliasWin->Show();

   halApp->BusyCursor(False);
}

/*------------------------------------------------------------------------
 * Method to edit the group aliases
 */

void
AliasPrefC::EditGroup(Widget parent)
{
   halApp->BusyCursor(True);

   if ( !groupWin ) groupWin = new AliasPrefWinC(parent, groupAliasDict);
   groupWin->Show();

   halApp->BusyCursor(False);
}

/*---------------------------------------------------------------
 *  Function to expand aliases with an argument to tell whether or
 *     not to expand hidden aliases.
 */

StringC
AliasPrefC::ExpandAddress(StringC& alias, int depth, Boolean expandHidden)
{
   if ( debuglev > 0 )
      cout <<depth <<" Expanding alias string: " <<alias <<endl;

   if ( alias.size() < 1 || depth <= 0 )
      return alias;

   StringC	output;

   if ( !expandHidden && hideAddrList.includes(alias) ) {
      output = alias;
      output += ":;";	// RFC822 section 6.2.6 "named list" syntax
      return output;
   }

//
// Loop through addresses
//
   AddressC	addrs(alias);
   AddressC	*addr = &addrs;
   StringC	*val;
   StringC	addrStr;

   while ( addr ) {

//
// If there's a comment don't expand this one
//
      if ( addr->name ) {

//
// Remove comments if spaces are used as address separators
//
	 if ( ishApp->compPrefs->spaceEndsAddr )
	    addrStr = addr->addr;
	 else
	    addrStr = addr->full;
      }

//
// Try to expand the address
//
      else {

	 addrStr = addr->addr;
	 val = aliasDict.definitionOf(addrStr);
	 if ( !val ) val = groupAliasDict.definitionOf(addrStr);
	 if ( !val ) val = otherAliasDict.definitionOf(addrStr);

	 if ( val ) {
            if ( depth>1 )
	       addrStr = ExpandAddress(*val, depth-1, expandHidden);
            else {
//
// Remove comments if spaces are used as address separators
//
	       if ( ishApp->compPrefs->spaceEndsAddr ) {
		  AddressC	valAddr(*val);
		  addrStr = valAddr.addr;
	       }
	       else
		  addrStr = *val;
	    }

	 } // End if there is a new value

      } // End if we can expand

//
// Add the result to the output
//
      if ( addrStr.size() > 0 ) {
	 if ( output.size() > 0 ) output += ", ";
	 output += addrStr;
      }

      addr = addr->next;

   } // End for each address

   if ( debuglev > 0 ) cout <<depth <<" Expanded string: " <<output <<endl;
   return output;

} // End ExpandAddress

