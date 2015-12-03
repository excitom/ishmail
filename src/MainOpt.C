/*
 *  $Id: MainOpt.C,v 1.2 2000/05/07 12:26:12 fnevgeny Exp $
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
#include "MainWinC.h"
#include "MainWinP.h"
#include "IshAppC.h"
#include "AppPrefC.h"
#include "ReadPrefC.h"
#include "CompPrefC.h"
#include "MailPrefC.h"
#include "SigPrefC.h"
#include "ReplyPrefC.h"
#include "ConfPrefC.h"
#include "HeadPrefC.h"
#include "AliasPrefC.h"
#include "AlertPrefC.h"
#include "IconPrefC.h"
#include "SavePrefC.h"
#include "AutoFilePrefC.h"
#include "FontPrefC.h"
#include "MainButtPrefC.h"
#include "SumPrefC.h"
#include "FolderPrefC.h"
#include "SortPrefC.h"

/*---------------------------------------------------------------
 *  Callback to handle Options->Application
 */

void
MainWinP::DoOptApp(Widget, MainWinP *This, XtPointer)
{
   ishApp->appPrefs->Edit(*This->pub);
}

/*---------------------------------------------------------------
 *  Callback to handle Options->Reading
 */

void
MainWinP::DoOptRead(Widget, MainWinP *This, XtPointer)
{
   ishApp->readPrefs->Edit(*This->pub);
}

/*---------------------------------------------------------------
 *  Callback to handle Options->Composition
 */

void
MainWinP::DoOptSend(Widget, MainWinP *This, XtPointer)
{
   ishApp->compPrefs->Edit(*This->pub);
}

/*---------------------------------------------------------------
 *  Callback to handle Options->Mail
 */

void
MainWinP::DoOptMail(Widget, MainWinP *This, XtPointer)
{
   ishApp->mailPrefs->Edit(*This->pub);
}

/*---------------------------------------------------------------
 *  Callback to handle Options->Signature
 */

void
MainWinP::DoOptSig(Widget, MainWinP *This, XtPointer)
{
   ishApp->sigPrefs->Edit(*This->pub);
}

/*---------------------------------------------------------------
 *  Callback to handle Options->Reply
 */

void
MainWinP::DoOptReply(Widget, MainWinP *This, XtPointer)
{
   ishApp->replyPrefs->Edit(*This->pub);
}

/*---------------------------------------------------------------
 *  Callback to handle Options->Confirmations
 */

void
MainWinP::DoOptConf(Widget, MainWinP *This, XtPointer)
{
   ishApp->confPrefs->Edit(*This->pub);
}

/*---------------------------------------------------------------
 *  Callback to handle Options->Headers
 */

void
MainWinP::DoOptHead(Widget, MainWinP *This, XtPointer)
{
   ishApp->headPrefs->Edit(*This->pub);
}

/*---------------------------------------------------------------
 *  Callback to handle Options->Aliases
 */

void
MainWinP::DoOptAlias(Widget, MainWinP *This, XtPointer)
{
   ishApp->aliasPrefs->Edit(*This->pub);
}

/*---------------------------------------------------------------
 *  Callback to handle Options->Group Aliases
 */

void
MainWinP::DoOptGroup(Widget, MainWinP *This, XtPointer)
{
   ishApp->aliasPrefs->EditGroup(*This->pub);
}

/*---------------------------------------------------------------
 *  Callback to handle Options->Alerts
 */

void
MainWinP::DoOptAlert(Widget, MainWinP *This, XtPointer)
{
   ishApp->alertPrefs->Edit(*This->pub);
}

/*---------------------------------------------------------------
 *  Callback to handle Options->Icons
 */

void
MainWinP::DoOptIcon(Widget, MainWinP *This, XtPointer)
{
   ishApp->iconPrefs->Edit(*This->pub);
}

/*---------------------------------------------------------------
 *  Callback to handle Options->Save Patterns
 */

void
MainWinP::DoOptSave(Widget, MainWinP *This, XtPointer)
{
   ishApp->savePrefs->Edit(*This->pub);
}

/*---------------------------------------------------------------
 *  Callback to handle Options->Automatic Filing
 */

void
MainWinP::DoOptAuto(Widget, MainWinP *This, XtPointer)
{
   ishApp->autoFilePrefs->Edit(*This->pub);
}

/*---------------------------------------------------------------
 *  Callback to handle Options->Fonts
 */

void
MainWinP::DoOptFont(Widget, MainWinP *This, XtPointer)
{
   ishApp->fontPrefs->Edit(*This->pub);
}

/*---------------------------------------------------------------
 *  Callback to handle Options->Buttons
 */

void
MainWinP::DoOptButt(Widget, MainWinP *This, XtPointer)
{
   ishApp->mainButtPrefs->Edit(*This->pub);
}

/*---------------------------------------------------------------
 *  Callback to handle Options->Message list
 */

void
MainWinP::DoOptSumm(Widget, MainWinP *This, XtPointer)
{
   ishApp->sumPrefs->Edit(*This->pub);
}

/*---------------------------------------------------------------
 *  Callback to handle Options->Folder
 */

void
MainWinP::DoOptFolder(Widget, MainWinP *This, XtPointer)
{
   ishApp->folderPrefs->Edit(*This->pub);
}

/*---------------------------------------------------------------
 *  Callback to handle Options->Sort
 */

void
MainWinP::DoOptSort(Widget, MainWinP *This, XtPointer)
{
   ishApp->sortPrefs->Edit(*This->pub);
}
