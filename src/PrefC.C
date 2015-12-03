/*
 * $Id: PrefC.C,v 1.2 2000/05/07 12:26:12 fnevgeny Exp $
 *
 * Copyright (c) 1993 HAL Computer Systems International, Ltd.
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

#include "PrefC.h"
#include "IshAppC.h"
#include "IshAppP.h"
#include "RuleDictC.h"

#include <hgl/StringListC.h>
#include <hgl/RegexC.h>
#include <hgl/CharC.h>

/*---------------------------------------------------------------
 *  Constructor
 */

PrefC::PrefC()
{
} // End constructor

/*---------------------------------------------------------------
 *  Destructor
 */

PrefC::~PrefC()
{
}

/*---------------------------------------------------------------
 *  Methods to store resources in the resource database
 */

void
PrefC::Store(const char *res)
{
   resStr = "Ishmail*";
   resStr += res;
   XrmPutStringResource(&ishApp->resdb, resStr, valStr);
}

void
PrefC::Store(const char *res, const char *val)
{
   valStr = val;
   AddEscapes(valStr);
   Store(res);
}

void
PrefC::Store(const char *res, Boolean val)
{
   valStr = (val ? "True" : "False");
   Store(res);
}

void
PrefC::Store(const char *res, int val)
{
   valStr.Clear();
   valStr += val;
   Store(res);
}

void
PrefC::Store(const char *res, float val)
{
   char      fs[128];
   sprintf(fs, "%f", val);

   valStr.Clear();
   valStr += fs;
   Store(res);
}

void
PrefC::Store(const char *res, StringListC& list)
{
   valStr.Clear();

   StringC	str;
   u_int	count = list.size();
   for (int i=0; i<count; i++) {
      str = *list[i];
      AddEscapes(str);
      if ( i>0 ) valStr += "\t";
      valStr += str;
   }

   Store(res);
}

void
PrefC::Store(const char *res, RuleDictC& dict)
{
   valStr.Clear();

   StringC	str;
   unsigned     count = dict.size();
   for (int i=0; i<count; i++) {

      str = dict[i]->key;
      AddEscapes(str);
      valStr += str;
      valStr += "\t:: ";

      str = dict[i]->val;
      AddEscapes(str);
      valStr += str;
      valStr += ";";
   }

   Store(res);
}

void
PrefC::StoreGravity(const char *res, int val)
{
   switch (val) {
      case (NorthGravity):	valStr = "North";	break;
      case (NorthEastGravity):	valStr = "NorthEast";	break;
      case (NorthWestGravity):	valStr = "NorthWest";	break;
      case (SouthEastGravity):	valStr = "SouthEast";	break;
      case (SouthWestGravity):	valStr = "SouthWest";	break;
      case (EastGravity):	valStr = "East";	break;
      case (WestGravity):	valStr = "West";	break;
      case (SouthGravity):
      default:			valStr = "South";	break;
   }

   Store(res);
}

/*----------------------------------------------------------------------
 * Replace '\' with "\\"
 */

void
PrefC::AddEscapes(StringC& str)
{
   int	pos = 0;
   StringC	newStr(str.size()*2);
   while ( pos < str.size() ) {
      char	c = str[pos];
      if      ( c == '\\' ) newStr += "\\\\";
      else if ( c == '\n' ) newStr += "\\n\\\n";
      else		    newStr += c;
      pos++;
   }

   str = newStr;

} // End AddEscapes

/*---------------------------------------------------------------
 *  Methods to update a resource in a list
 */

void
PrefC::Update(StringListC& list, const char *res)
{
   if ( debuglev > 1 ) cout <<"Update " <<res <<" " <<valStr <<endl;

   resStr.Clear();
   if ( strncmp(res, "Ishmail*", 8) != 0 ) resStr = "Ishmail*";
   resStr += res;
   resStr += ":\t";
   resStr += valStr;

   StringC	*lineP = Find(list, res);
   if ( lineP ) {
      if ( debuglev > 2 ) cout <<"   Modifying line: " <<*lineP <<endl;
      *lineP = resStr;
      if ( debuglev > 2 ) cout <<"   New line is   : " <<*lineP <<endl;
   }
   else	{
      list.add(resStr);
      if ( debuglev > 2 ) cout <<"   Added new line" <<endl;
   }
}

void
PrefC::Update(StringListC& list, const char *res, const char *val)
{
   valStr = val;
   AddEscapes(valStr);
   Update(list, res);
}

void
PrefC::Update(StringListC& list, const char *res, Boolean val)
{
   valStr = (val ? "True" : "False");
   Update(list, res);
}

void
PrefC::Update(StringListC& list, const char *res, int val)
{
   valStr.Clear();
   valStr += val;
   Update(list, res);
}

void
PrefC::Update(StringListC& list, const char *res, float val)
{
   char      fs[128];
   sprintf(fs, "%f", val);

   valStr.Clear();
   valStr += fs;
   Update(list, res);
}

void
PrefC::Update(StringListC& list, const char *res, StringListC& valList)
{
   valStr.Clear();

   StringC	str;
   u_int	count = valList.size();
   for (int i=0; i<count; i++) {
      str = *valList[i];
      AddEscapes(str);
      if ( i>0 ) valStr += "\t";
      valStr += str;
   }

   Update(list, res);
}

void
PrefC::Update(StringListC& list, const char *res, RuleDictC& dict)
{
   valStr.Clear();

   StringC	str;
   unsigned     count = dict.size();
   for (int i=0; i<count; i++) {

      str = dict[i]->key;
      AddEscapes(str);
      valStr += str;
      valStr += "\t:: ";

      str = dict[i]->val;
      AddEscapes(str);
      valStr += str;
      valStr += ";";
   }

   Update(list, res);
}

void
PrefC::UpdateGravity(StringListC& list, const char *res, int val)
{
   switch (val) {
      case (NorthGravity):	valStr = "North";	break;
      case (NorthEastGravity):	valStr = "NorthEast";	break;
      case (NorthWestGravity):	valStr = "NorthWest";	break;
      case (SouthEastGravity):	valStr = "SouthEast";	break;
      case (SouthWestGravity):	valStr = "SouthWest";	break;
      case (EastGravity):	valStr = "East";	break;
      case (WestGravity):	valStr = "West";	break;
      case (SouthGravity):
      default:			valStr = "South";	break;
   }

   Update(list, res);
}

/*---------------------------------------------------------------
 *  Method to search for a resource in a list
 */

StringC*
PrefC::Find(StringListC& list, const char *res)
{
//
// Look for the resource in the list.  Return it if found.
//
   static RegexC	*resPat = NULL;
   if ( !resPat ) resPat = new RegexC("^Ishmail\\*\\([^:]+\\):");

   if ( strncmp(res, "Ishmail*", 8) == 0 ) res += 8;

   unsigned	count = list.size();
   for (int i=0; i<count; i++) {
      StringC	*line = list[i];
      if ( resPat->match(*line) && (*line)((*resPat)[1]) == res )
	 return line;
   }

   return NULL;

} // End Find

/*---------------------------------------------------------------
 *  Method to read the resource file
 */

void
PrefC::ReadResFile(StringListC& lineList)
{
   StringC	data;
   if ( !data.ReadFile(ishApp->resFile) ) return;

   ishApp->priv->GetResLines(data, lineList, False/*removeDups*/);
}

/*---------------------------------------------------------------
 *  Method to write the resource file
 */

Boolean
PrefC::WriteResFile(StringListC& lineList)
{
   return ishApp->priv->WriteResFile(lineList);
}

/*---------------------------------------------------------------
 *  Method to extract rules from a string
 */

void
PrefC::ExtractRules(StringC ruleStr, RuleDictC& ruleDict)
{
//
// Extract lines of the following format:
//   pattern :: value;
//
   RegexC	entry("[^;]+;");
   RegexC	pair("\\(.+\\)::\\(.+\\);");
   StringC	line;
   StringC	left;
   StringC	right;
   int		startPos = 0;

   while ( startPos < ruleStr.size() && entry.search(ruleStr,startPos) >= 0 ) {

      line = ruleStr(entry[0]);
      startPos += entry[0].length();
      //cout <<"Rule line is: <" <<line <<">" <<endl;

      if ( pair.match(line) ) {
	 left  = line(pair[1]);
	 right = line(pair[2]);
	 left.Trim();
	 right.Trim();
	 //cout <<"Adding rule: " <<left <<" :: " <<right <<endl;
	 ruleDict.add(RegexC(left), right);
      }
   }

//
// Get that last line without a comma
//
   //cout <<"Final line is: <" <<str <<">" <<endl;
   if ( startPos < ruleStr.size() && pair.match(ruleStr) ) {
      left  = ruleStr(pair[1]);
      right = ruleStr(pair[2]);
      left.Trim();
      right.Trim();
      //cout <<"Adding rule: " <<left <<" :: " <<right <<endl;
      ruleDict.add(RegexC(left), right);
   }

} // End ExtractRules

