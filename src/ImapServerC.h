/*
 *  $Id: ImapServerC.h,v 1.16 2001/04/04 09:39:04 evgeny Exp $
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
#ifndef _ImapServerC_h_
#define _ImapServerC_h_

#include <hgl/CharC.h>
#include <hgl/StringC.h>

#include "Imap.h"

#ifdef HAVE_OPENSSL
# include <openssl/crypto.h>
# include <openssl/x509.h>
# include <openssl/pem.h>
# include <openssl/ssl.h>
# include <openssl/err.h>
#endif

#define IMAP_MAXLINE	255

class CharC;
class StringListC;
class IntListC;
class ImapFolderC;


typedef struct _AuthData
{
   CharC user;
   CharC pass;
} AuthData;

// Return codes of RunCommand2()
typedef enum {
   ImapCommandOK,		// OK :-)
   ImapCommandNO,		// Could not be performed by the server
   ImapCommandBAD,		// Unrecognized by the server
   ImapCommandBYE,		// Server closed connection
   ImapCommandFailed,		// Protocol error and/or our internal bug
   ImapCommandCancelled,	// Cancelled by the user
   ImapCommandNoConnect		// Connection impossible (no network etc)
} ImapCommandReturn;

// SSL versions
#define   TLS_VERSION_SSLV2        (1L << 0)
#define   TLS_VERSION_SSLV3        (1L << 1)
#define   TLS_VERSION_TLSV1        (1L << 2)

// Server response types as passed to RunCommand2's hook
typedef enum {
   ImapResponseOK,
   ImapResponseNO,
   ImapResponseBAD,
   ImapResponseCont
} ImapResponse;

// Hook function to be supplied for RunCommand2
typedef Boolean (*RunCommandHook)(
   ImapResponse type,
   CharC response,
   StringListC info,
   StringListC data,
   StringC *reply,
   void *data
);

class ImapServerC {

private:

   int		tagCount;

   StringListC	*lineList;
   char		lastBuf[IMAP_MAXLINE+1];
   
   Boolean	RetryLogin(char*);
   Boolean	Capability();
   void		CloseSocket();
   Boolean      DoLogin();

public:

   // Server name, port etc
   StringC	name;
   long		port;
   Boolean	imaps;
   struct hostent	*host;
   
   // Server capabilities & properties
   long		protocols;
   long		authMethods;
   Boolean	haveStartTLS;
   long		bugs;
   
   // Connection properties & flags
   int		sock;
   Boolean	connected;
   Boolean	TLSEnabled;
   Boolean	authenticated;
   Boolean	dataPending;

   // SSL stuff
#ifdef HAVE_OPENSSL
   SSL_CTX* sslCtx;
   SSL*     ssl;
#endif
   
   // Authentication data
   StringC	user;
   StringC	pass;
   
   // Currently SELECT'ed folder
   StringC	folder;

   ImapServerC(const char *hostname, const char *username=NULL,
               long portnum=IMAP_PORT, Boolean imaps_flag=False);
   ~ImapServerC();

   Boolean	Append(CharC, CharC, StringListC&);
   Boolean	Append(CharC, StringListC&, char*, StringListC&);
   ImapCommandReturn	Authenticate(CharC, CharC);
   Boolean      Close(StringListC&);
   Boolean      Connect();
   Boolean	Copy(int, CharC, StringListC&);
   Boolean	Create(CharC, StringListC&);
   Boolean	Delete(CharC, StringListC&);
   Boolean      EnableTLS(unsigned long vers=
                   TLS_VERSION_SSLV2|TLS_VERSION_SSLV3|TLS_VERSION_TLSV1);
   Boolean	EstablishSession();
   Boolean	Exists(CharC);
   Boolean	Expunge(StringListC&);
   void		ExpandList(StringListC&, StringListC&);
   Boolean      DisableTLS();
   Boolean	FetchHdrs(int, ImapFolderC*, int*, char**, char**, StringC&);
   void		FetchFlush( StringC& );
   Boolean	Fetch(int, ImapFolderC*, CharC, char**, char**, StringC&);
   void		GenTag(StringC&);
   Boolean	GetLine(StringC&);
   Boolean	ListMailboxes(CharC, StringListC&, StringListC&);
   Boolean	Logout(StringListC&);
   Boolean	Noop(StringListC&);
   Boolean	PutLine(CharC, Boolean terminate=True);
   Boolean      ReConnect();
   Boolean	Rename(CharC, CharC, StringListC&);
   Boolean	RunCommand(CharC, StringListC&);
   ImapCommandReturn	RunCommand2(CharC, RunCommandHook, void *);
   Boolean	Search(CharC, IntListC&, StringListC&);
   Boolean	Select(CharC, StringListC&);
   Boolean	SetFlags(int, u_int);
   Boolean      StartTLS();
   void		Unexpected(CharC);
};

extern ImapServerC	*FindImapServer(const char*, long, Boolean, const char *);
extern ImapServerC	*FindImapServer(const char*);
extern void		 CloseImapServerConnections();

#endif // _ImapServerC_h_
