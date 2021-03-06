/*
 *  $Id: ReadIconC.C,v 1.2 2000/05/07 12:26:12 fnevgeny Exp $
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
#include "ReadIconC.h"
#include "MsgPartC.h"

#include <hgl/rsrc.h>

/*---------------------------------------------------------------
 *  Constructor
 */

ReadIconC::ReadIconC(MsgPartC *msgPart, MimeRichTextC *rich)
: MimeIconC (msgPart->conStr, rich)
{
   part        = msgPart;
   textWin     = NULL;
   msgWin      = NULL;

//
// Read resources
//
   char		*cl   = "ReadIconC";
   char		*name = part->conStr;
   Widget	par   = rt->TextArea();
   showStr = get_string(cl, name, "showLabel", par, "Show");
   hideStr = get_string(cl, name, "hideLabel", par, "Hide");

} // End constructor

/*---------------------------------------------------------------
 *  Destructor
 */

ReadIconC::~ReadIconC()
{
}

/*---------------------------------------------------------------
 *  Method to return the name of a good pixmap file
 */

void
ReadIconC::GetPixmapFile(StringC& file)
{
//
// If this is an image part, see if we can create a thumbnail
//
   if ( part->IsImage() && (part->subStr.Contains("xpm", IGNORE_CASE) ||
      			    part->subStr.Contains("xbm", IGNORE_CASE)) &&
	part->CreateDataFile() )
      file = part->dataFile;
   else
      file = pixmapFile;

} // End GetPixmapFile

