/*
 *  $Id: SumPrefC.C,v 1.2 2000/05/07 12:26:13 fnevgeny Exp $
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
#include "SumPrefC.h"
#include "SumPrefWinC.h"

#include <hgl/HalAppC.h>
#include <hgl/rsrc.h>
#include <hgl/StringListC.h>
#include <hgl/FieldViewC.h>

/*---------------------------------------------------------------
 *  Constructor
 */

SumPrefC::SumPrefC() : PrefC()
{
   prefWin = NULL;

//
// Read resources
//
   showPixmaps = get_boolean(*halApp, "showPixmaps", True);
   dateFormat  = get_string (*halApp, "dateFormat");

   SumFieldC	*field = &sumFieldList[SumFieldC::MSG_NUM];
   field->type  = SumFieldC::MSG_NUM;
   field->pos   = get_int    (*halApp, "msgNumColumnPos",      0);
   field->min   = get_int    (*halApp, "msgNumColumnMinWidth", 0);
   field->max   = get_int    (*halApp, "msgNumColumnMaxWidth", 0);
   field->title = get_string (*halApp, "msgNumColumnTitle",    "Number");
   field->show  = get_boolean(*halApp, "msgNumColumnVisible",  True);

   field = &sumFieldList[SumFieldC::STATUS];
   field->type  = SumFieldC::STATUS;
   field->pos   = get_int    (*halApp, "msgStatColumnPos",      0);
   field->min   = get_int    (*halApp, "msgStatColumnMinWidth", 0);
   field->max   = get_int    (*halApp, "msgStatColumnMaxWidth", 0);
   field->title = get_string (*halApp, "msgStatColumnTitle",    " ");
   field->show  = get_boolean(*halApp, "msgStatColumnVisible",  True);

   field = &sumFieldList[SumFieldC::FROM];
   field->type  = SumFieldC::FROM;
   field->pos   = get_int    (*halApp, "fromColumnPos",      0);
   field->min   = get_int    (*halApp, "fromColumnMinWidth", 0);
   field->max   = get_int    (*halApp, "fromColumnMaxWidth", 0);
   field->title = get_string (*halApp, "fromColumnTitle",    "From");
   field->show  = get_boolean(*halApp, "fromColumnVisible",  True);

   field = &sumFieldList[SumFieldC::SUBJECT];
   field->type  = SumFieldC::SUBJECT;
   field->pos   = get_int    (*halApp, "subjectColumnPos",      0);
   field->min   = get_int    (*halApp, "subjectColumnMinWidth", 0);
   field->max   = get_int    (*halApp, "subjectColumnMaxWidth", 0);
   field->title = get_string (*halApp, "subjectColumnTitle",    "Subject");
   field->show  = get_boolean(*halApp, "subjectColumnVisible",  True);

   field = &sumFieldList[SumFieldC::DATE];
   field->type  = SumFieldC::DATE;
   field->pos   = get_int    (*halApp, "dateColumnPos",      0);
   field->min   = get_int    (*halApp, "dateColumnMinWidth", 0);
   field->max   = get_int    (*halApp, "dateColumnMaxWidth", 0);
   field->title = get_string (*halApp, "dateColumnTitle",    "Date");
   field->show  = get_boolean(*halApp, "dateColumnVisible",  True);

   field = &sumFieldList[SumFieldC::LINES];
   field->type  = SumFieldC::LINES;
   field->pos   = get_int    (*halApp, "linesColumnPos",      0);
   field->min   = get_int    (*halApp, "linesColumnMinWidth", 0);
   field->max   = get_int    (*halApp, "linesColumnMaxWidth", 0);
   field->title = get_string (*halApp, "linesColumnTitle",    "Lines");
   field->show  = get_boolean(*halApp, "linesColumnVisible",  False);

   field = &sumFieldList[SumFieldC::BYTES];
   field->type  = SumFieldC::BYTES;
   field->pos   = get_int    (*halApp, "bytesColumnPos",      0);
   field->min   = get_int    (*halApp, "bytesColumnMinWidth", 0);
   field->max   = get_int    (*halApp, "bytesColumnMaxWidth", 0);
   field->title = get_string (*halApp, "bytesColumnTitle",    "Bytes");
   field->show  = get_boolean(*halApp, "bytesColumnVisible",  False);

   SortFields();

} // End constructor

/*---------------------------------------------------------------
 *  destructor
 */

SumPrefC::~SumPrefC()
{
   delete prefWin;
}

/*---------------------------------------------------------------
 *  Method to write resources to resource database
 */

Boolean
SumPrefC::WriteDatabase()
{
   SumFieldC	*field = &sumFieldList[sumColumn[SumFieldC::MSG_NUM]];
   Store("msgNumColumnPos",		field->pos);
   Store("msgNumColumnMinWidth",	field->min);
   Store("msgNumColumnMaxWidth",	field->max);
   Store("msgNumColumnTitle",		field->title);
   Store("msgNumColumnVisible",		field->show);

   field = &sumFieldList[sumColumn[SumFieldC::STATUS]];
   Store("msgStatColumnPos",		field->pos);
   Store("msgStatColumnMinWidth",	field->min);
   Store("msgStatColumnMaxWidth",	field->max);
   Store("msgStatColumnTitle",		field->title);
   Store("msgStatColumnVisible",	field->show);

   field = &sumFieldList[sumColumn[SumFieldC::FROM]];
   Store("fromColumnPos",		field->pos);
   Store("fromColumnMinWidth",		field->min);
   Store("fromColumnMaxWidth",		field->max);
   Store("fromColumnTitle",		field->title);
   Store("fromColumnVisible",		field->show);

   field = &sumFieldList[sumColumn[SumFieldC::SUBJECT]];
   Store("subjectColumnPos",		field->pos);
   Store("subjectColumnMinWidth",	field->min);
   Store("subjectColumnMaxWidth",	field->max);
   Store("subjectColumnTitle",		field->title);
   Store("subjectColumnVisible",	field->show);

   field = &sumFieldList[sumColumn[SumFieldC::DATE]];
   Store("dateColumnPos",		field->pos);
   Store("dateColumnMinWidth",		field->min);
   Store("dateColumnMaxWidth",		field->max);
   Store("dateColumnTitle",		field->title);
   Store("dateColumnVisible",		field->show);

   field = &sumFieldList[sumColumn[SumFieldC::LINES]];
   Store("linesColumnPos",		field->pos);
   Store("linesColumnMinWidth",		field->min);
   Store("linesColumnMaxWidth",		field->max);
   Store("linesColumnTitle",		field->title);
   Store("linesColumnVisible",		field->show);

   field = &sumFieldList[sumColumn[SumFieldC::BYTES]];
   Store("bytesColumnPos",		field->pos);
   Store("bytesColumnMinWidth",		field->min);
   Store("bytesColumnMaxWidth",		field->max);
   Store("bytesColumnTitle",		field->title);
   Store("bytesColumnVisible",		field->show);

   Store("showPixmaps",			showPixmaps);
   Store("dateFormat",			dateFormat);

   return True;

} // End WriteDatabase

/*---------------------------------------------------------------
 *  Method to write resources to a file
 */

Boolean
SumPrefC::WriteFile()
{
   StringListC	lineList;
   ReadResFile(lineList);

   SumFieldC	*field = &sumFieldList[sumColumn[SumFieldC::MSG_NUM]];
   Update(lineList, "msgNumColumnPos",		field->pos);
   Update(lineList, "msgNumColumnMinWidth",	field->min);
   Update(lineList, "msgNumColumnMaxWidth",	field->max);
   Update(lineList, "msgNumColumnTitle",	field->title);
   Update(lineList, "msgNumColumnVisible",	field->show);

   field = &sumFieldList[sumColumn[SumFieldC::STATUS]];
   Update(lineList, "msgStatColumnPos",		field->pos);
   Update(lineList, "msgStatColumnMinWidth",	field->min);
   Update(lineList, "msgStatColumnMaxWidth",	field->max);
   Update(lineList, "msgStatColumnTitle",	field->title);
   Update(lineList, "msgStatColumnVisible",	field->show);

   field = &sumFieldList[sumColumn[SumFieldC::FROM]];
   Update(lineList, "fromColumnPos",		field->pos);
   Update(lineList, "fromColumnMinWidth",	field->min);
   Update(lineList, "fromColumnMaxWidth",	field->max);
   Update(lineList, "fromColumnTitle",		field->title);
   Update(lineList, "fromColumnVisible",	field->show);

   field = &sumFieldList[sumColumn[SumFieldC::SUBJECT]];
   Update(lineList, "subjectColumnPos",		field->pos);
   Update(lineList, "subjectColumnMinWidth",	field->min);
   Update(lineList, "subjectColumnMaxWidth",	field->max);
   Update(lineList, "subjectColumnTitle",	field->title);
   Update(lineList, "subjectColumnVisible",	field->show);

   field = &sumFieldList[sumColumn[SumFieldC::DATE]];
   Update(lineList, "dateColumnPos",		field->pos);
   Update(lineList, "dateColumnMinWidth",	field->min);
   Update(lineList, "dateColumnMaxWidth",	field->max);
   Update(lineList, "dateColumnTitle",		field->title);
   Update(lineList, "dateColumnVisible",	field->show);

   field = &sumFieldList[sumColumn[SumFieldC::LINES]];
   Update(lineList, "linesColumnPos",		field->pos);
   Update(lineList, "linesColumnMinWidth",	field->min);
   Update(lineList, "linesColumnMaxWidth",	field->max);
   Update(lineList, "linesColumnTitle",		field->title);
   Update(lineList, "linesColumnVisible",	field->show);

   field = &sumFieldList[sumColumn[SumFieldC::BYTES]];
   Update(lineList, "bytesColumnPos",		field->pos);
   Update(lineList, "bytesColumnMinWidth",	field->min);
   Update(lineList, "bytesColumnMaxWidth",	field->max);
   Update(lineList, "bytesColumnTitle",		field->title);
   Update(lineList, "bytesColumnVisible",	field->show);

   Update(lineList, "showPixmaps",		showPixmaps);
   Update(lineList, "dateFormat",		dateFormat);

//
// Write the information back to the file
//
   return WriteResFile(lineList);

} // End WriteFile

/*------------------------------------------------------------------------
 * Sort the field columns by position
 */

void
SumPrefC::SortFields()
{
   qsort((void *)sumFieldList, (size_t)SumFieldC::SUMMARY_FIELD_COUNT,
	 (size_t)sizeof(SumFieldC),
	 (int (*)(const void*, const void*))SumFieldC::ComparePositions);

//
// Update the column numbers
//
   for (int i=0; i<SumFieldC::SUMMARY_FIELD_COUNT; i++) {
      SumFieldC	*field = &sumFieldList[i];
      sumColumn[field->type] = i;
   }

} // End SortFields

/*------------------------------------------------------------------------
 * Method to update summary field characteristics for a specified field view
 */

void
SumPrefC::UpdateFields(FieldViewC *fv)
{
//
// Set the new column titles.  Set these first as that determines the
//   number of columns
//
   StringListC	titleList;
   titleList.AllowDuplicates(TRUE);

//
// Loop through the columns in sorted order
//
   int	i;
   for (i=0; i<SumFieldC::SUMMARY_FIELD_COUNT; i++) {
      SumFieldC	*field = &sumFieldList[i];
      titleList.append(field->title);
   }

   fv->SetTitles(titleList);

//
// Now set the rest of the info
//
   for (i=0; i<SumFieldC::SUMMARY_FIELD_COUNT; i++) {

      SumFieldC	*field = &sumFieldList[i];

//
// Set the justification
//
      if ( field->type == SumFieldC::FROM    ||
	   field->type == SumFieldC::SUBJECT ||
	   field->type == SumFieldC::DATE )
         fv->JustifyColumn(i, JUSTIFY_LEFT);
      else
         fv->JustifyColumn(i, JUSTIFY_RIGHT);

//
// Set the max width
//
      fv->SetColumnMinWidth(i, field->min);
      fv->SetColumnMaxWidth(i, field->max);

//
// Set the visibility
//
      if ( field->show ) fv->ShowColumn(i);
      else		 fv->HideColumn(i);

   } // End for each sorted field

} // End UpdateFields

/*------------------------------------------------------------------------
 * Method to edit the preferences
 */

void
SumPrefC::Edit(Widget parent)
{
   halApp->BusyCursor(True);

   if ( !prefWin ) prefWin = new SumPrefWinC(parent);
   prefWin->Show();

   halApp->BusyCursor(False);
}

