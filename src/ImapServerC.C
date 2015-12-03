/*
 *  $Id: ImapServerC.C,v 1.38 2001/07/22 18:07:30 evgeny Exp $
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
#include "MainWinC.h"
#include "Misc.h"
#include "FolderPrefC.h"
#include "ImapServerC.h"
#include "ImapFolderC.h"
#include "MsgStatus.h"
#include "LoginWinC.h"
#include "Query.h"
#include "Base64.h"

#include <hgl/HalAppC.h>
#include <hgl/StringC.h>
#include <hgl/SysErr.h>
#include <hgl/PtrListC.h>
#include <hgl/StringListC.h>
#include <hgl/CharC.h>
#include <hgl/WArgList.h>
#include <hgl/WXmString.h>
#include <hgl/IntListC.h>

#include <Xm/MessageB.h>
#include <Xm/Protocols.h>
#include <Xm/AtomMgr.h>

#include <X11/Intrinsic.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#include <unistd.h>
#include <errno.h>
#include <string.h>

#ifdef HAVE_SYS_SELECT_H
# include <sys/select.h>
#endif

#include <netdb.h>

#ifdef HAVE_OPENSSL
  // A hack to find out OPENSSLDIR
# define HEADER_CRYPTLIB_H
#  include <openssl/opensslconf.h>
# undef HEADER_CRYPTLIB_H
# ifndef OPENSSLDIR
#  define OPENSSLDIR "/usr/local/ssl"
# endif

# include <openssl/hmac.h>
#endif

extern int	debuglev;

static PtrListC	*serverList = NULL;

#ifdef HAVE_OPENSSL
static Boolean sslInited = False;
#endif

/*------------------------------------------------------------------------
 * Constructor to open a connection to an imap server
 */

ImapServerC::ImapServerC(const char *hostname, const char *username,
    long portnum, Boolean imaps_flag)
{
   sock          = 0;
   connected     = False;
   TLSEnabled    = False;
   authenticated = False;
   bugs          = 0L;		// let's assume the implementation is perfect
   haveStartTLS  = False;
   dataPending   = False;	// Should be no data pending at this point

//
// save hostname, port # etc in server object
//
   name = hostname;
   port = portnum;
   if ( username ) {
      user = username;
   } else {
      user = ishApp->userId;
   }

//
// Init buffers
//
   lineList = new StringListC;
   lineList->AllowDuplicates(TRUE);
   lastBuf[0] = 0;
   
#ifndef HAVE_OPENSSL
   if ( imaps_flag ) {
      char *errmsg = "Support for secure connections is not compiled in";
      halApp->PopupMessage(errmsg);
      return;
   }
#else
   imaps = imaps_flag;
#endif

   if ( !EstablishSession() ) return;
//
// Add this server to the list
//
   if ( !serverList ) serverList = new PtrListC;

   void	*tmp = (void*)this;
   serverList->add(tmp);

} // End constructor

/*------------------------------------------------------------------------
 * Destructor
 */

ImapServerC::~ImapServerC()
{
   if ( connected ) {
      StringListC	output;
      Logout(output);
   }

   delete lineList;

//
// Remove this server from the list
//
   if ( serverList ) {
      void	*tmp = (void*)this;
      serverList->remove(tmp);
   }
   
} // End destructor


/*------------------------------------------------------------------------
 * Method to establish an authenticated session with the IMAP server
 * - this is called from the constructor, and also to re-connect if
 *   the connection drops for some reason.
 */
Boolean
ImapServerC::EstablishSession()
{
//
// Connect to the server
//
   connected = Connect();
   if ( !connected ) return False;

//
// Determine capabilities of the server
//
   if ( !Capability() ) {
      CloseSocket();
      return False;
   }
   
#ifdef HAVE_OPENSSL
   if ( !imaps && haveStartTLS && !(bugs & IMAP_BUG_STARTTLS_BROKEN) ) {
      // We MUST re-check capabilities once switched to TLS
      if ( StartTLS() && Capability() ) {
         ; // OK
      } else {
         StringC errmsg =
            "Can't start TLS; falling back to unencrypted session";
         halApp->PopupMessage(errmsg);
         // Re-try connecting, this time without TLS
         return EstablishSession();
      }
   }
#endif

//
// Log in if needed
//
   authenticated = DoLogin();
   if ( !authenticated ) {
      CloseSocket();
      return False;
   }
   
   return True;
}

/*------------------------------------------------------------------------
 * Method to connect a socket to the named IMAP server
 */
Boolean
ImapServerC::Connect()
{
   StringC	errmsg;

   authenticated = False;

   dataPending = False;		// Should be no data pending at this point

   CloseSocket();
//
// Get host name
//
   host = gethostbyname(name);
   if ( !host ) {
      int	err = errno;
      errmsg = "Could not find entry for host: ";
      errmsg += name;
      errmsg += ".\n" + SystemErrorMessage(err);
      halApp->PopupMessage(errmsg);
      return False;
   }

//
// Create socket
//
   sock = socket(AF_INET, SOCK_STREAM, 0);
   if ( sock <= 0 ) {
      int	err = errno;
      errmsg = "Could not create socket.\n";
      errmsg += SystemErrorMessage(err);
      halApp->PopupMessage(errmsg);
      return False;
   }

//
// Open connection
//
   struct sockaddr_in	sin;

   memset((char*)&sin, 0, sizeof(sin));
   sin.sin_family = AF_INET;
   sin.sin_port   = (u_short)htons(port);
   memcpy((char*)&sin.sin_addr, host->h_addr, host->h_length);
   if ( connect(sock, (struct sockaddr*)&sin, sizeof(sin)) < 0 ) {
      int	err = errno;
      errmsg = "Could not connect to host: ";
      errmsg += name;
      errmsg += ".\n" + SystemErrorMessage(err);
      halApp->PopupMessage(errmsg);
      CloseSocket();
      return False;
   }
   
   // Start right in the TLS mode if told so
   if ( imaps ) {
      if ( !EnableTLS() ) {
         CloseSocket();
         return False;
      }
   }

//
// Wait for the server's response to see if the user must log in
//
   StringC	line;
   if ( !GetLine(line) ) {
      CloseSocket();
      return False;
   }

   if ( line.StartsWith("* OK ") ) {
      // OK; will need to log in later on
      return True;
   }

   else if ( line.StartsWith("* BYE") ) { // Not accepted
      line.CutBeg(5);
      line.Trim();
      errmsg = "The IMAP server on host ";
      errmsg += name;
      errmsg += "\nwould not accept a connection:\n\n";
      errmsg += line;
      halApp->PopupMessage(errmsg);
      CloseSocket();
      return False;
   }

   else if ( line.StartsWith("* PREAUTH") ) {
      // Connection has already been authenticated by external means
      authenticated = True;
      return True;
   }

   else {
      Unexpected(line);
      CloseSocket();
      return False;
   }

   return True;
}

/*------------------------------------------------------------------------
 * Method to close socket (if it has been opened)
 */
void
ImapServerC::CloseSocket()
{
   if ( sock > 0 ) {
      close(sock);
   }
   if ( TLSEnabled ) {
      DisableTLS();
   }
   sock = 0;
   connected     = False;
   authenticated = False;
}

/*------------------------------------------------------------------------
 * Method to start TLS session
 */
Boolean
ImapServerC::StartTLS()
{
#ifdef HAVE_OPENSSL
   if ( !haveStartTLS || TLSEnabled ) {
      return False;
   }

   StringListC	output;
   if ( !RunCommand("STARTTLS", output) ) return False;
   
   return EnableTLS(TLS_VERSION_TLSV1);
#else
   return False;
#endif
}
   
#ifdef HAVE_OPENSSL

static int verify_cert(int ok, X509_STORE_CTX *ctx)
{
   X509 *serverCert;
   char *certSubjectName, *certIssuerName;
   
   // Get server's certificate
   serverCert = X509_STORE_CTX_get_current_cert(ctx);
   if ( !serverCert ) {
      halApp->PopupMessage("No server certificate presented!", XmDIALOG_ERROR);
      return False;
   }
   
   certSubjectName = X509_NAME_oneline(X509_get_subject_name(serverCert), 0, 0);
   certIssuerName  = X509_NAME_oneline(X509_get_issuer_name(serverCert), 0, 0);
   
   if ( debuglev ) {
      cout << "Server certificate:"
           << "\n\t subject: "
           << certSubjectName
           << "\n\t issuer: "
           << certIssuerName
           << endl;
   }

   if ( !ok ) {
      StringC str = "Could not verify the following certificate:";
      str += "\n   Subject: ";
      str += certSubjectName;
      str += "\n   Issued by: ";
      str += certIssuerName;
      str.Replace("/", "\n      ");
      str += "\n\nReason: ";
      str += X509_verify_cert_error_string(X509_STORE_CTX_get_error(ctx));
      str += ".\n\nAccept the certificate?";

      if ( Query(str, *halApp, False) == QUERY_YES ) {
         ok = True;

         // Certificate path (TODO: configurable)
         StringC certFile = ishApp->home + "/.ishmail.pem";

         FILE *fp = fopen(certFile, "wa");
         if (fp) {
            PEM_write_X509(fp, serverCert);
         }
      }
   }

   return ok;
}
#endif

/*------------------------------------------------------------------------
 * Method to enable TLS negotiation supporting protocols of vers
 */
Boolean
ImapServerC::EnableTLS(unsigned long vers)
{
#ifdef HAVE_OPENSSL
   if ( !sock || !(haveStartTLS || imaps) || TLSEnabled ) {
      return False;
   }
   
   // Initialization
   sslCtx      = NULL;
   ssl         = NULL;
   
   int err;
   SSL_METHOD *meth;

   if ( !sslInited ) {
      if ( debuglev ) {
         cout << "Initializing OpenSSL" << endl;
      }
      // For error messages
      SSL_load_error_strings();
      // Setup all the global SSL stuff
      SSL_library_init();
      sslInited = True;
   }
   
   Boolean sslv2_ok = vers & TLS_VERSION_SSLV2;
   Boolean sslv3_ok = vers & TLS_VERSION_SSLV3;
   Boolean tlsv1_ok = vers & TLS_VERSION_TLSV1;
   
   if ( sslv2_ok && !sslv3_ok && !tlsv1_ok ) {
      meth = SSLv2_client_method();
   } else
   if ( !sslv2_ok && sslv3_ok && tlsv1_ok ) {
      meth = SSLv3_client_method();
   } else
   if ( !sslv2_ok && !sslv3_ok && tlsv1_ok ) {
      meth = TLSv1_client_method();
   } else {
      meth = SSLv23_client_method();
   }
   
   sslCtx = SSL_CTX_new(meth);
   if ( !sslCtx ) {
      return False;
   }

   // set up CAs to look up
   // Certificate path (TODO: configurable)
   StringC certFile = ishApp->home + "/.ishmail.pem";
   StringC certPath = OPENSSLDIR;
   certPath += "/certs";
   if (!SSL_CTX_load_verify_locations(sslCtx, certFile, certPath)) {
      SSL_CTX_set_default_verify_paths(sslCtx);
   }
   
   SSL_CTX_set_verify(sslCtx, SSL_VERIFY_PEER, verify_cert);
   SSL_CTX_set_verify_depth(sslCtx, 1);

   // Start SSL negotiation
   ssl = SSL_new(sslCtx);
   if ( !ssl ) {
      DisableTLS();
      return False;
   }
   
   SSL_set_connect_state(ssl);

   SSL_set_fd(ssl, sock);
   int res = SSL_connect(ssl);
   if ( res <= 0 ) {
      err = SSL_get_error(ssl, res);
      if ( err == SSL_ERROR_SYSCALL ) {
         halApp->PopupMessage(SystemErrorMessage(errno));
      }
      // Broken STARTTLS implementation?
      bugs |= IMAP_BUG_STARTTLS_BROKEN;
      DisableTLS();
      return False;
   }

   // Get the cipher
   if ( debuglev ) {
      cout << "SSL connection using " << SSL_get_cipher(ssl) << endl;
   }

   TLSEnabled = True;
   return True;
#else
   return False;
#endif
}

/*------------------------------------------------------------------------
 * Method to end TLS negotiation and free respective structures
 */
Boolean
ImapServerC::DisableTLS()
{
#ifdef HAVE_OPENSSL
   if ( !haveStartTLS ) {
      return False;
   }
   
   if ( ssl ) {
      SSL_shutdown(ssl);
      SSL_free(ssl);
   }
   if ( sslCtx ) {
      SSL_CTX_free(sslCtx);
   }
   
   TLSEnabled = False;
   return True;
#else
   return False;
#endif
}

/*------------------------------------------------------------------------
 * Method to authenticate against the IMAP server
 */
Boolean
ImapServerC::DoLogin()
{
//
// Do nothing if already (pre)authenticated
//
   if ( authenticated ) return True;
   
   StringC	errmsg;
   
   while ( True ) {

//
// Get user id and password
//
      if ( !GetLogin(name, user, pass) ) {
	 // User cancelled
	 return False;
      }

//
// Send login command
//
      ImapCommandReturn res = Authenticate(user, pass);
      switch ( res ) {
      case ImapCommandOK:
	 return True;
         break;
      case ImapCommandNoConnect:
	 // An error message should be issued by a low-level function
         return False;
         break;
      case ImapCommandCancelled:
	 // User cancelled; just return
         return False;
         break;
      case ImapCommandNO:
	 // Wrong user/pass
         errmsg = "The IMAP server on host ";
	 errmsg += name;
	 errmsg += "\ndenied access.";
         break;
      default:
	 // ?? Not-supported protocol or our bug
         errmsg = "An authentication error ocured while connecting to server.";
	 errmsg += name;
         break;
      }
      
      InvalidateLogin(name);
      
      errmsg += "\n\nRetry?";
      if ( !RetryLogin(errmsg) ) {
	 return False;
      }

   } // End while not logged in
}

/*------------------------------------------------------------------------
 * Method to reconnect to the IMAP server
 */

Boolean
ImapServerC::ReConnect()
{
   if ( EstablishSession() ) {
      ishApp->mainWin->curFolder->Rescan();
      return True;
   } else
      return False;
}

/*------------------------------------------------------------------------
 * Method to determine the IMAP server capabilities
 */

Boolean
ImapServerC::Capability()
{
   protocols    = 0L;
   authMethods  = 0L;
   
   // STARTTLS may not be announced after switching to TLS
   if ( !TLSEnabled ) {
      haveStartTLS = False;
   }
   
   // all servers support LOGIN unless explicitly said otherwise
   Boolean loginDisabled = False; 

//
// Issue the CAPABILITY command
//
   StringListC	output;
   if ( RunCommand("CAPABILITY", output) ) {
//
// Loop through the output looking for BAD response. If found, this is
// an IMAP 2 server. Otherwise, get some valuable info (supported protocols,
// authentication schemes etc)
//
      u_int	count = output.size();
      for (int i=0; i<count; i++) {
	 StringC	*line = output[i];
	 if ( line->StartsWith("BAD", IGNORE_CASE) ) {
	    protocols = IMAP_PROTOCOL_IMAP2BIS; // Stick with IMAP2bis
            break;
         } else {
            StringListC tokens;
            
            ExtractList(*line, tokens);

            int ntokens = tokens.size();
            for (int j = 0; j < ntokens; j++) {
	       StringC *token = tokens[j];
               if ( token->Equals("IMAP4", IGNORE_CASE) ) {
	          protocols |= IMAP_PROTOCOL_IMAP4;
               }
               else if ( token->Equals("IMAP4rev1", IGNORE_CASE) ) {
	          protocols |= IMAP_PROTOCOL_IMAP4REV1;
               }
               else if ( token->Equals("AUTH=LOGIN", IGNORE_CASE) ) {
	          authMethods |= IMAP_AUTH_LOGIN;
               }
               else if ( token->Equals("AUTH=ANONYMOUS", IGNORE_CASE) ) {
	          authMethods |= IMAP_AUTH_ANONYMOUS;
               }
#ifdef HAVE_OPENSSL
               else if ( token->Equals("AUTH=CRAM-MD5", IGNORE_CASE) ) {
	          authMethods |= IMAP_AUTH_CRAM_MD5;
               }
#endif
#if 0
               else if ( token->Equals("AUTH=GSSAPI", IGNORE_CASE) ) {
	          authMethods |= IMAP_AUTH_GSSAPI;
               }
               else if ( token->Equals("AUTH=KERBEROS_V4", IGNORE_CASE) ) {
	          authMethods |= IMAP_AUTH_KERBEROS_V4;
               }
#endif
               else if ( token->Equals("LOGINDISABLED", IGNORE_CASE) ) {
	          loginDisabled = True;
               }
               else if ( token->Equals("STARTTLS", IGNORE_CASE) ) {
	          haveStartTLS = True;
               }
            }
         }
      }
   
      if ( !loginDisabled ) {
         authMethods |= IMAP_AUTH_CLEAR_TEXT;
      }
      
      if ( !protocols ) {
         // No supported protocols found
         StringC	errmsg;
         errmsg = "The IMAP server on host ";
         errmsg += name;
         errmsg += "\ndoes not support any known IMAP protocol\n";
         halApp->PopupMessage(errmsg);
         return False;
      }
      else if ( !authMethods ) {
         // No supported authentication methods
         StringC	errmsg;
         errmsg = "The IMAP server on host ";
         errmsg += name;
         errmsg += "\nfeatures no supported authentication methods\n";
         halApp->PopupMessage(errmsg);
         return False;
      } else {
         return True;
      }
   } else {
      return False;
   }

} // End Capability

/*------------------------------------------------------------------------
 * Function to find or open a connection to the specified IMAP server
 */
ImapServerC*
FindImapServer(const char *hostname, long port, Boolean imaps, const char *username)
{
//
// See if this one is already connected
//
   if ( !serverList ) serverList = new PtrListC;

   u_int	count = serverList->size();
   for (int i=0; i<count; i++) {
      ImapServerC	*server = (ImapServerC*)*(*serverList)[i];
      if ( server->name == hostname &&
           server->port == port     &&
           server->user == username ) return server;
   }

//
// Create a new server object
//
   ImapServerC	*server = new ImapServerC(hostname, username, port, imaps);

   return server;

} // End FindImapServer

/*------------------------------------------------------------------------
 * An old restricted form of the above. Should be eventually removed
 */
ImapServerC*
FindImapServer(const char *hostname)
{
   if ( !serverList ) serverList = new PtrListC;

   u_int	count = serverList->size();
   for (int i=0; i<count; i++) {
      ImapServerC	*server = (ImapServerC*)*(*serverList)[i];
      if ( server->name == hostname ) return server;
   }

   ImapServerC	*server = new ImapServerC(hostname);

   return server;

} // End FindImapServer

/*------------------------------------------------------------------------
 * Function to close the connections to all IMAP servers
 */

void
CloseImapServerConnections()
{
   if ( !serverList ) return;

   u_int	count = serverList->size();
   for (int i=count-1; i>=0; i--) {
      ImapServerC	*server = (ImapServerC*)*(*serverList)[i];
      delete server;
   }
   
   serverList->removeAll();
   delete serverList;

} // CloseImapServerConnections

/*------------------------------------------------------------------------
 * Function to return the next line from the server
 */

Boolean
ImapServerC::GetLine(StringC& line)
{
   StringC	errmsg;
   char		buf[IMAP_MAXLINE+1];
   
   buf[0]	= 0;

//
// Return the first line in the list
//
   if ( lineList->size() > 0 ) {
      line = *(*lineList)[0];
      lineList->remove((u_int)0);
      if ( debuglev > 1 ) cout <<"<< [" <<line <<"]" <<endl;
      return True;
   }

   line.Clear();

   StringC	curLine(lastBuf);
   CharC	bufStr;
   int          readCount;

#ifdef HAVE_OPENSSL
   if ( TLSEnabled ) {

      while (True) {
         readCount = SSL_read(ssl, buf, IMAP_MAXLINE);
         if ( readCount <= 0 ) {
            // FIXME: proper check
            return False;
         }
         
         buf[readCount] = 0;
         bufStr = buf;

   //
   // If there's no newline, save this buffer and read some more
   //
         int	pos = bufStr.PosOf('\n');
         if ( pos < 0 ) {
	    curLine += bufStr;
	    bufStr.CutBeg(bufStr.Length());
         }

   //
   // Extract lines from the buffer
   //
         else while ( pos >= 0 ) {

	    curLine += bufStr(0,pos);	// Copy up to newline
	    if ( curLine.EndsWith('\r') ) curLine.CutEnd(1);

   //
   // See if this line is to be returned
   //
	    if ( line.size() == 0 )
	       line = curLine;
	    else
	       lineList->add(curLine);

   //
   // Look for another line
   //
	    lastBuf[0] = 0;
	    bufStr.CutBeg(pos+1);

	    curLine.Clear();

	    pos = bufStr.PosOf('\n');

         } // End for each newline in buffer

   //
   // If we found a match, we're done
   //
         if ( line.size() > 0 ) {
	    if ( debuglev > 1 ) cout <<"<< [" <<line <<"]" <<endl;
            strcpy(lastBuf, bufStr.Addr());
	    return True;
         }

   //
   // Copy whatever's left into lastBuf
   //
         //strcpy(lastBuf, bufStr.Addr());
         if (bufStr.Length() > 0)
            curLine += bufStr(0,bufStr.Length());
      }
      
      return True;
   }
#endif

//
// Read lines until we find a match
//
   fd_set	readSet;
   FD_ZERO(&readSet);

   struct timeval	timeout;

   while ( True ) {

//
// Check the socket
//
      int rc;
      while (True) {
         FD_SET(sock, &readSet);
         timeout.tv_sec  = 10;
         timeout.tv_usec = 0;
         rc = select(sock+1/*width*/, &readSet, NULL, NULL, &timeout);
	 //
	 // data is available on some socket
	 //
	 if ( rc > 0 )
		break;

	 //
	 // error occurred
	 //
         else if ( rc < 0) {
            if ( errno == EINTR ) {
		if (debuglev > 0) cout << "Select got SIGINTR, retrying" << endl;
	    } else {
	 	StringC	errmsg;
	 	int	err = errno;
	 	errmsg = "Could not select socket ";
	 	errmsg += sock;
	 	errmsg += " for IMAP server ";
	 	errmsg += name;
	 	errmsg += ".\n" + SystemErrorMessage(err);
	 	halApp->PopupMessage(errmsg);
	 	return False;
	    }
	 }

	 //
	 // timeout
	 //
         else if ( rc == 0 ) {
	    CloseSocket();
	    errmsg = "Timeout trying to select the IMAP server\n";
	    errmsg += "\n\nRetry?";
	    if ( RetryLogin(errmsg) ) {
	       //
	       // Try to reconnect. Regardless of whether it works, return
	       // "False" here so the the operation will be re-tried from
	       // the beginning.
	       //
   	       connected = ReConnect();
	    }
	    return False;
	 }
      }

//
// See if there is data for us
//
      if ( !FD_ISSET(sock, &readSet) ) continue;

      readCount = read(sock, buf, IMAP_MAXLINE);
      if ( readCount == 0 ) {
	  CloseSocket();
	  errmsg = "Timeout trying to select the IMAP server\n";
	  errmsg += "\n\nRetry?";
	  if ( RetryLogin(errmsg) ) {
	     //
	     // Try to reconnect. Regardless of whether it works, return
	     // "False" here so the the operation will be re-tried from
	     // the beginning.
	     //
   	     connected = ReConnect();
	  }
	  return False;
      }
      else if ( readCount < 0 ) {
	 StringC	errmsg;
	 int	err = errno;
	 errmsg = "Could not read socket ";
	 errmsg += sock;
	 errmsg += " for IMAP server ";
	 errmsg += name;
	 errmsg += ".\n" + SystemErrorMessage(err);
	 halApp->PopupMessage(errmsg);
	 return False;
      }

      buf[readCount] = 0;
      if ( debuglev > 2 ) cout <<"Buffer [" <<buf <<"]" <<endl;
      bufStr = buf;

//
// If there's no newline, save this buffer and read some more
//
      int	pos = bufStr.PosOf('\n');
      if ( pos < 0 ) {
	 curLine += bufStr;
	 bufStr.CutBeg(bufStr.Length());
      }

//
// Extract lines from the buffer
//
      else while ( pos >= 0 ) {

	 curLine += bufStr(0,pos);	// Copy up to newline
	 if ( curLine.EndsWith('\r') ) curLine.CutEnd(1);

//
// See if this line is to be returned
//
	 if ( line.size() == 0 )
	    line = curLine;
	 else
	    lineList->add(curLine);

//
// Look for another line
//
	 lastBuf[0] = 0;
	 bufStr.CutBeg(pos+1);

	 curLine.Clear();

	 pos = bufStr.PosOf('\n');

      } // End for each newline in buffer

//
// If we found a match, we're done
//
      if ( line.size() > 0 ) {
	 if ( debuglev > 1 ) cout <<"<< [" <<line <<"]" <<endl;
         strcpy(lastBuf, bufStr.Addr());
	 return True;
      }

//
// Copy whatever's left into lastBuf
//
      //strcpy(lastBuf, bufStr.Addr());
      if (bufStr.Length() > 0)
         curLine += bufStr(0,bufStr.Length());

   } // End while no matching line has been read

} // End GetLine

/*------------------------------------------------------------------------
 * Function to send a command to the server
 */

Boolean
ImapServerC::PutLine(CharC line, Boolean terminate)
{
   StringC		errmsg;
   if ( debuglev > 0 ) {
      int	pos = -1;
      char	*esc;
      if ( TLSEnabled ) {
         esc = "[01;32m>> [";
      } else {
         esc = "[1m>> [";
      }
      if ( line.Contains(" LOGIN ") ) pos = line.RevPosOf(' ');
      cout <<esc << ((pos >= 0) ? line(0,pos):line) <<"][0m" <<endl;
   }
   
   StringC linebuf(line);
//
// Fix line ending if necessary
//
   if ( terminate ) {
      if ( linebuf.EndsWith('\n') ) linebuf.CutEnd(1);
      linebuf += "\r\n";
   }

#ifdef HAVE_OPENSSL
   if ( TLSEnabled ) {
      void *buf = (char *) linebuf;
      int nbytes = SSL_write(ssl, buf, linebuf.length());
      if (nbytes <= 0) {
         // FIXME: proper check
         // err = SSL_get_error(ssl, nbytes);
         if ( Query("Connection dropped. Retry?", *halApp, False) == QUERY_YES ) {
            ReConnect();
         }
         return False;
      } else {
         return True;
      }
   }
#endif

   fd_set	writeSet;
   FD_ZERO(&writeSet);

   struct timeval timeout;

   while ( linebuf.length() > 0 ) {

//
// See if the socket is available
//
      int rc;
      while (True) {
         FD_SET(sock, &writeSet);
         timeout.tv_sec  = 5;
         timeout.tv_usec = 0;
         rc = select(sock+1, NULL, &writeSet, NULL, &timeout);
	 //
	 // a socket is available for writing
	 //
	 if ( rc > 0 )
		break;

	 //
	 // error occurred
	 //
         else if ( rc < 0) {
            if ( errno == EINTR ) {
		if (debuglev > 0) cout << "Select got SIGINTR, retrying" << endl;
            } else if ( errno == EBADF ) {
                ReConnect();
	    } else {
	 	int	err = errno;
	 	errmsg = "Could not select socket ";
	 	errmsg += sock;
	 	errmsg += " for writing to the IMAP server ";
	 	errmsg += name;
	 	errmsg += ".\n" + SystemErrorMessage(err);
	 	halApp->PopupMessage(errmsg);
	 	return False;
	    }
	 }

	 //
	 // timeout
	 //
         else if ( rc == 0 ) {
	     errmsg = "Timeout trying to select the IMAP server\n";
	     errmsg += "\n\nRetry?";
	     if ( !RetryLogin(errmsg) ) {
	       CloseSocket();
	       return False;
	     }
   	     if ( !(connected = ReConnect()) ) {
	       return False;
	     }
	 }
      }

      int writeCount;
      while ( True ) {
         writeCount = write(sock, (const char*) linebuf, linebuf.length());
         if ( writeCount <= 0 ) {
            if ( errno == EPIPE) {
		errmsg = "Lost connection to the IMAP server\n";
	 	errmsg += "\n\nTry to re-connect ?";
	 	if ( !RetryLogin(errmsg) ) {
		  CloseSocket();
		  return False;
	 	}
   		if ( !(connected = ReConnect()) ) {
		  return False;
		}
	    } else {
	 	int	err = errno;
	 	errmsg = "Could not write socket ";
	 	errmsg += sock;
	 	errmsg += " for IMAP server ";
	 	errmsg += name;
	 	errmsg += ".\n" + SystemErrorMessage(err);
	 	halApp->PopupMessage(errmsg);
	 	return False;
	   }
        } else {
	   break;	// write succeeded
	}
      }
      linebuf.CutBeg(writeCount);

   } // End while all bytes not written

   return True;

} // End PutLine

/*------------------------------------------------------------------------
 * Method to generate a unique tag
 */

void
ImapServerC::GenTag(StringC& tag)
{
   tag = "ISH";
   tag += tagCount++;
   tag += " ";
}

/*------------------------------------------------------------------------
 * Method to display an error message about an unexpected reply
 */

void
ImapServerC::Unexpected(CharC str)
{
   StringC	errmsg = "Unexpected reply from IMAP server ";
   errmsg += name;
   errmsg += ":\n\"";
   errmsg += str;
   errmsg += "\"\n";
#if 0
   halApp->PopupMessage(errmsg);
#endif
   // Rather than hang the program waiting for a user response,
   // just splat these messages to standard error.

   cerr << errmsg;
}

/*------------------------------------------------------------------------
 * Function to run a command and wait for the response
 */

Boolean
ImapServerC::RunCommand(CharC cmd, StringListC& output)
{
   StringC	tag;
   GenTag(tag);

   StringC	cmdline(tag);
   cmdline += cmd;

   if ( !PutLine(cmdline) ) return False;

   if ( !GetLine(cmdline) ) return False;
   while ( !cmdline.StartsWith(tag) ) {
      output.add(cmdline);
      if ( !GetLine(cmdline) ) return False;
   }

   cmdline.CutBeg(tag.size());
   output.add(cmdline);

   return True;

} // End RunCommand


/*------------------------------------------------------------------------
 * Function to run a command and let the supplied hook function process
 * the response, including dealing with continuation requests from the
 * server. Any data can be passed to the callback function, if needed
 */

ImapCommandReturn
ImapServerC::RunCommand2(CharC cmd, RunCommandHook hook, void *cdata)
{
   Boolean 		ok = True;
   ImapCommandReturn 	ret;
   StringListC 		info, data;
   StringC		tag;
   
   GenTag(tag);
   StringC	cmdline(tag);
   cmdline += cmd;

   if ( !PutLine(cmdline) ) {
      return ImapCommandNoConnect;
   }

   while ( True ) {
      StringC		response;
      ImapResponse	type;
      
      if ( !GetLine(response) ) {
         return ImapCommandNoConnect;
      }
      
      if ( response.StartsWith(tag) ) {
         response.CutBeg(tag.size());
         if ( response.StartsWith("OK", IGNORE_CASE) ) {
            type = ImapResponseOK;
            ret  = ImapCommandOK;
            response.CutBeg(2);
         } else
         if (response.StartsWith("NO", IGNORE_CASE) ) {
            type = ImapResponseNO;
            ret  = ImapCommandNO;
            response.CutBeg(2);
         } else
         if (response.StartsWith("BAD", IGNORE_CASE) ) {
            type = ImapResponseBAD;
            ret  = ImapCommandBAD;
            response.CutBeg(3);
         } else {
            // Unknown response
            return ImapCommandFailed;
         }
         if ( hook && !hook(type, response, info, data, &cmdline, cdata) ) {
            ok = False;
         }
         if ( ok ) {
            return ret;
         } else {
            return ImapCommandFailed;
         }
      } else if ( response.StartsWith("* ") ) {
         response.CutBeg(2);
         info.add(response);
         // TODO: check for BYE here
      } else if ( response.StartsWith("+ ") ) {
         type = ImapResponseCont;
         response.CutBeg(2);
         if ( !hook || !hook(type, response, info, data, &cmdline, cdata) ) {
            ok = False;
         }
         if ( !PutLine(cmdline) ) {
            return ImapCommandNoConnect;
         }
      } else {
         data.add(response);
      }
   }
   
   // Should never reach this point
   return ImapCommandFailed;

} // End RunCommand

/*------------------------------------------------------------------------
 * Function to send NOOP command and wait for response.
 */

Boolean
ImapServerC::Noop(StringListC& output)
{
   return RunCommand("NOOP", output);
}

// Anonymous login
static Boolean
authenticationAnonymousHook(
   ImapResponse type,
   CharC resp,
   StringListC info,
   StringListC data,
   StringC *reply,
   void *cdata
)
{
   if ( type == ImapResponseCont ) {
      // Ignore the server's challenge; it may be an empty string
      
      AuthData *ad = (AuthData *) cdata;

      reply->Clear();

      // Respond with the password (which is the user's email, probably)
      if ( !TextToText64(ad->pass, reply, False, False) ) {
         return False;
      }

      return True;
   } else {
      // No special action on the command completion
      return True;
   }
}

// Base64-encoded plain text login; should be used as the last resort
static Boolean
authenticationLoginHook(
   ImapResponse type,
   CharC resp,
   StringListC info,
   StringListC data,
   StringC *reply,
   void *cdata
)
{
   if ( type == ImapResponseCont ) {
      StringC buf;
      // Decode the response; it should be either "User Name" or "Password"
      if ( !Text64ToText(resp, &buf) ) {
         return False;
      } else {
         AuthData *ad = (AuthData *) cdata;
         
         reply->Clear();

         // NB: below, StartsWith() is used instead of Equals() because the
         // base64 strings are explicitly 0-terminated
         if ( buf.StartsWith("User Name", IGNORE_CASE) ) {
            // Respond with the username
            if ( !TextToText64(ad->user, reply, False, False) ) {
               return False;
            }
         } else if ( buf.StartsWith("Password", IGNORE_CASE) ) {
            // Respond with the password
            if ( !TextToText64(ad->pass, reply, False, False) ) {
               return False;
            }
         } else {
            // Unexpected response
            return False;
         }
         
         return True;
      }
   } else {
      // No special action on the command completion
      return True;
   }
}

#ifdef HAVE_OPENSSL

// Print data in hexadecimals
static char *
hexpt(unsigned char *md)
{
   int i;
   static char buf[2*MD5_DIGEST_LENGTH + 1];

   for (i = 0; i < MD5_DIGEST_LENGTH; i++) {
      sprintf(&(buf[2*i]), "%02x", md[i]);
   }
   
   buf[2*MD5_DIGEST_LENGTH] = '\0';
   
   return(buf);
}

/*------------------------------------------------------------------------
 * CRAM-MD5 authentication scheme (RFC 2195)
 * Based on the keyed-hashing MD5 digest algorithm (aka HMAC) (RFC 2104)
 * All the hard work is done by the OpenSSL crypto library
 */
static Boolean
authenticationHmacHook(
   ImapResponse type,
   CharC resp,
   StringListC info,
   StringListC data,
   StringC *reply,
   void *cdata
)
{
   if ( type == ImapResponseCont ) {
      StringC buf;
      // Decode the response; its a "challenge"
      if ( !Text64ToText(resp, &buf) ) {
         return False;
      } else {
         AuthData *ad = (AuthData *) cdata;
         
         reply->Clear();
         
         // Produce the digest & print it in hexadecimals
         const char *d = buf;
         const char *p;
         p = hexpt(HMAC(EVP_md5(),
                  ad->pass.Addr(), ad->pass.Length(),
                  (const unsigned char *) d, buf.length(),
                  NULL, NULL));
         // Prepend username
         buf = ad->user;
         buf += " ";
         buf += p;

         // Base64 encode everything
         if ( !TextToText64(buf, reply, False, False) ) {
               return False;
         }
         
         return True;
      }
   } else {
      // No special action on the command completion
      return True;
   }
}
#endif

/*------------------------------------------------------------------------
 * Function to send LOGIN (for IMAP2) or AUTHENTICATE (IMAP4) command and
 * process response.
 */

ImapCommandReturn
ImapServerC::Authenticate(CharC user, CharC pass)
{
   ImapCommandReturn 	res;
   StringC		cmd;
   RunCommandHook 	hook;
   Boolean		secure;
   AuthData 		ad;
   ad.user = user;
   ad.pass = pass;
   
   if ( (authMethods & IMAP_AUTH_ANONYMOUS) &&
        user.Equals("anonymous", IGNORE_CASE) ) {
      // Anonymous login; not really secure, but who cares? ;-)
      secure = True;
      cmd = "AUTHENTICATE ANONYMOUS";
      hook = authenticationAnonymousHook;
   } else
#ifdef HAVE_OPENSSL
   if ( authMethods & IMAP_AUTH_CRAM_MD5 ) {
      // MD5 digest
      secure = True;
      cmd = "AUTHENTICATE CRAM-MD5";
      hook = authenticationHmacHook;
   } else
#endif
   if ( authMethods & IMAP_AUTH_LOGIN ) {
      // using base64-encoded username & password
      secure = False;
      cmd = "AUTHENTICATE LOGIN";
      hook = authenticationLoginHook;
   } else
   if ( authMethods & IMAP_AUTH_CLEAR_TEXT ) {
      // IMAP2 clear text login
      secure = False;
      cmd = "LOGIN ";
      cmd += user;
      cmd += " \"";
      cmd += pass;
      cmd += "\"";
      hook = NULL;
   } else {
      // should never happen
      return ImapCommandFailed;
   }
   
   // If the whole session is encrypted, don't worry about secure authentication
   if ( TLSEnabled ) {
      secure = True;
   }
   
   if ( !secure ) {
      // If the authentication method is insecure, ask user
      // whether (s)he agrees to use a clear text login
      StringC str;
      str  = "No (supported) secure authentication mechanisms available.";
      str += "\n\nDo you want to use a clear text login?";
      if ( Query(str, *halApp, False) != QUERY_YES ) {
         return ImapCommandCancelled;
      }
   }
   
   return RunCommand2(cmd, hook, (void *) &ad);
}

/*------------------------------------------------------------------------
 * Function to send LOGOUT command and wait for response.
 */

Boolean
ImapServerC::Logout(StringListC& output)
{
   if ( !connected ) {
      return True;
   } else {
      return RunCommand("LOGOUT", output);
   }
}

/*------------------------------------------------------------------------
 * Function to send CREATE command and wait for response.
 */

Boolean
ImapServerC::Create(CharC mailbox, StringListC& output)
{
   StringC	cmd = "CREATE ";
   cmd += mailbox;

   return RunCommand(cmd, output);
}

/*------------------------------------------------------------------------
 * Function to send DELETE command and wait for response.
 */

Boolean
ImapServerC::Delete(CharC mailbox, StringListC& output)
{
   // If the folder is currently open, close it first
   if ( folder == mailbox ) {
      StringListC output;
      if ( !Close(output) ) return False;
   }

   StringC	cmd = "DELETE ";
   cmd += mailbox;

   return RunCommand(cmd, output);
}

/*------------------------------------------------------------------------
 * Function to send RENAME command and wait for response.
 */

Boolean
ImapServerC::Rename(CharC oldname, CharC newname, StringListC& output)
{
   StringC	cmd = "RENAME ";
   cmd += oldname;
   cmd += " ";
   cmd += newname;

   Boolean	success = RunCommand(cmd, output);
   if ( success && oldname == folder ) folder = newname;

   return success;
}

/*------------------------------------------------------------------------
 * Function to send SELECT command and wait for response.
 */

Boolean
ImapServerC::Select(CharC mailbox, StringListC& output)
{
   // If the folder has been opened, and the server doesn't answer
   // duplicate SELECT correctly (UW, for one) close the folder first
   if ( bugs & IMAP_BUG_DUPLICATE_SELECT && folder == mailbox ) {
      StringListC output;
      if ( !Close(output) ) return False;
   }
   
   StringC	cmd = "SELECT ";
   cmd += mailbox;

   Boolean	success = RunCommand(cmd, output);
   if ( success ) folder = mailbox;

   return success;
}

/*------------------------------------------------------------------------
 * Function to send CLOSE command and wait for response.
 */

Boolean
ImapServerC::Close(StringListC& output)
{
   return RunCommand("CLOSE", output);
}

/*------------------------------------------------------------------------
 * Function to send EXPUNGE command and wait for response.
 */

Boolean
ImapServerC::Expunge(StringListC& output)
{
   return RunCommand("EXPUNGE", output);
}

/*------------------------------------------------------------------------
 * Function to send FETCH command and wait for response.
 */

Boolean
ImapServerC::Fetch(int msgnum, ImapFolderC* fld, CharC key, char **output, char **flags,
		   StringC& response)
{
   *output = NULL;
   *flags  = NULL;
   response.Clear();

   StringC	saveFolderName = folder;
   StringC	folderName = fld->name;
   
//
// May need to select the named folder first
//
   if ( folderName != saveFolderName ) {
      StringListC output;
      if ( !Select(folderName, output) ) return False;
   }
   
//
// Build the command we will send
//
   StringC	tag;
   GenTag(tag);

   StringC	cmd(tag);
   cmd += "FETCH ";
   cmd += msgnum;
   cmd += " ";
   cmd += key;

//
// Send the command and wait for the response
//
   if ( !PutLine(cmd) ) return False;
   if ( !GetLine(response) ) return False;

   StringC	expect("* ");
   expect += msgnum;
   expect += " FETCH ";
   while ( !response.StartsWith(expect, IGNORE_CASE) &&
      	   !response.StartsWith(tag) ) {

      Unexpected(response);

      if ( !GetLine(response) ) return False;
      if ( response.StartsWith(tag) ) {
	 response.CutBeg(tag.size());
	 return False;
      }
   }

//
// Parse the response
//
   if ( response.StartsWith(expect, IGNORE_CASE) )
       response.CutBeg(expect.size());
   else if ( response.StartsWith(tag) )
       response.CutBeg(tag.size()); 
   if ( response.StartsWith('(') ) response.CutBeg(1);
   response.Trim();

//
// These commands send the total size in the first line
//
   if ( response.StartsWith("INTERNALDATE ",	IGNORE_CASE) ||
	response.StartsWith("RFC822.HEADER ",	IGNORE_CASE) ||
	response.StartsWith("RFC822.SIZE ",	IGNORE_CASE) ||
	response.StartsWith("UID ",		IGNORE_CASE) ||
	response.StartsWith("BODY[",		IGNORE_CASE) ||
	response.StartsWith("RFC822 ",		IGNORE_CASE) ||
	response.StartsWith("RFC822.TEXT ",	IGNORE_CASE) ) {

//
// See if there is a length present
//
      int	pos = response.PosOf(' ');
      if ( pos >= 0 ) response.CutBeg(pos+1);
      response.Trim();

      if ( response.StartsWith('{') ) {

	 char	*lenp = response;
	 char	*cp = ++lenp;
	 while ( *cp != '}' ) cp++;
	 *cp = 0;          // Temporary
	 int	len = atoi(lenp);

//
// Allocate buffer of specified length
//
	 *output = new char[len+1];
	 **output = 0;

//
// Read until buffer is filled
//
	 int	readlen = 0;
         char *output_p = *output;
	 while ( readlen < len && GetLine(response) &&
	         !response.StartsWith(tag) ) {
	    int	space = len - readlen;
            int response_size = response.size();
	    if ( response_size >= space ) {
	       strncpy(output_p, response, space);
               output_p += space;
	       *output_p = '\0';
	       response.CutBeg(space);
	       readlen = len;
	    }
	    else {
	       strcpy(output_p, response);
               output_p += response_size;
	       strcpy(output_p, "\n");
               output_p += 1;
	       readlen += response_size + 2;	// CRLF was stripped
	    }
	 }

//
// There can also be FLAGS
//
	 response.Trim();
	 if ( response.StartsWith("FLAGS ", IGNORE_CASE) ) {
	    response.CutBeg(6);
	    if ( response.EndsWith(')') ) response.CutEnd(1);
	    *flags = new char[response.size()+1];
	    **flags = 0;
	    strcpy(*flags, response);
	    response.Clear();
	 }

      } // End if length is known
      
//
// If the length is not known, the length is whatever's left in the response
//   line.
//
      else if ( !response.StartsWith("NIL") ) {
	 if ( response.EndsWith(')') ) response.CutEnd(1);
	 *output = new char[response.size()+1];
	 **output = 0;
	 strcpy(*output, response);
	 response.Clear();
      }

   } // End if the command is recognized

//
// These commands can contain many sized literals
//
   else if ( response.StartsWith("BODY ",		IGNORE_CASE) ||
	     response.StartsWith("BODYSTRUCTURE ",	IGNORE_CASE) ||
	     response.StartsWith("ENVELOPE ",		IGNORE_CASE) ) {

      int	pos = response.PosOf(' ');
      response.CutBeg(pos+1);
      response.Trim();

//
// Read lines until we get the tag
//
      StringC	buffer = response;
      if ( !GetLine(response) ) return False;
      while ( !response.StartsWith(tag) ) {

//
// Check for FLAGS
//
	 response.Trim();
	 if ( response.StartsWith("FLAGS ", IGNORE_CASE) ) {
	    response.CutBeg(6);
	    if ( response.EndsWith(')') ) response.CutEnd(1);
	    *flags = new char[response.size()+1];
	    **flags = 0;
	    strcpy(*flags, response);
	    response.Clear();
	 }
	 else
	    buffer += response;

	 if ( !GetLine(response) ) return False;

      } // End for each line up to tag

      *output = new char[buffer.size()+1];
      **output = 0;
      strcpy(*output, buffer);

   } // End if BODYSTRUCTURE or ENVELOPE command

//
// Check for flags only
//
   else if ( response.StartsWith("FLAGS ", IGNORE_CASE) ) {

      response.CutBeg(6);
      if ( response.EndsWith(')') ) response.CutEnd(1);
      *flags = new char[response.size()+1];
      **flags = 0;
      strcpy(*flags, response);
      response.Clear();
   }

   else {
      Unexpected(response);
   }

//
// Continue reading until we get the tag
//
   while ( !response.StartsWith(tag) ) {
      if ( !GetLine(response) ||
           response.StartsWith("NO ", IGNORE_CASE) ) return False;
   }

   response.CutBeg(tag.size());

//
// Restore the original folder, if any
//
   if ( saveFolderName.IsNull() && folderName != saveFolderName ) {
      StringListC output;
      if ( !Select(saveFolderName, output) ) return False;
   }

   return True;

} // End Fetch

/*------------------------------------------------------------------------
 * Function to send FETCH command to get a sequence of 1 or more 
 * message headers.
 */

Boolean
ImapServerC::FetchHdrs(int msgnum, ImapFolderC *fld, int *bytes,
		char **output, char **flags, StringC& response)
{
   int pos;
   *output = NULL;
   *flags  = NULL;
   response.Clear();

//
// First time through, build the FETCH command we will send.
// Save the "tag" prefix in the folder object, so that after the
// last message is fetched we can find the trailing tag.
//
   if ( fld->fetchNeeded ) {

      fld->fetchNeeded = False;
      GenTag(fld->fetchTag);

      StringC	cmd(fld->fetchTag);
      cmd += "FETCH ";
      cmd += msgnum;
      cmd += ":";
      cmd += fld->msgCount;
      cmd += " (FLAGS RFC822.SIZE";

//
// For max compatibility with function of other folder types, we want
// all the headers. However, the tradeoff is performance. Decision: The
// Message-ID and Received headers are least useful, and can take up a 
// lot of lines, so don't fetch them. Get everything else, though.
//
      //cmd += " RFC822.HEADER.LINES.NOT (MESSAGE-ID RECEIVED))";
      cmd += " RFC822.HEADER)";

//
// Send the command and wait for the response
//
      if ( !PutLine(cmd) ) return False;
   }

   if ( !GetLine(response) ) return False;

   StringC	expect("* ");
   expect += msgnum;
   expect += " FETCH ";
   while ( !response.StartsWith(expect, IGNORE_CASE) &&
      	   !response.StartsWith(fld->fetchTag) ) {

      Unexpected(response);

      if ( !GetLine(response) ) return False;
      if ( response.StartsWith(fld->fetchTag) ) {
	 response.CutBeg(fld->fetchTag.size());
	 return False;
      }
   }

// Got a response, set the data pending flag on the server object
   dataPending = True;

//
// Parse the response
//
   if ( response.StartsWith(expect, IGNORE_CASE) )
       response.CutBeg(expect.size());
   else if ( response.StartsWith(fld->fetchTag) )
       response.CutBeg(fld->fetchTag.size()); 
   if ( response.StartsWith('(') ) response.CutBeg(1);
   response.Trim();

//
// The first thing in the response line should be the flags
//
    if ( response.StartsWith("FLAGS ", IGNORE_CASE) ) {
      response.CutBeg(6);

      StringC flgs(response);
      pos = flgs.PosOf(')');
      if ( pos >=0 ) flgs.CutEnd(flgs.size()-pos-1);
      *flags = new char[flgs.size()+1];
      **flags = 0;
      strcpy(*flags, flgs);

      //
      // Skip over the flags
      //
      pos = response.PosOf(')');
      if ( pos >= 0 ) response.CutBeg(pos+1);
      response.Trim();
   } else {
      Unexpected(response);
      return False;
   }

//
// Next should be the size of the message
//
   if ( response.StartsWith("RFC822.SIZE", IGNORE_CASE) ) {
      pos = response.PosOf(' ');
      if ( pos >= 0 ) response.CutBeg(pos+1);
      response.Trim();

      StringC msgSize(response);
      pos = msgSize.PosOf(' ');
      if ( pos >= 0 ) msgSize.CutEnd(msgSize.size()-pos-1);
      msgSize.Trim();
      *bytes = atoi(msgSize);

      //
      // Skip over the size info
      //
      pos = response.PosOf(' ');
      if ( pos >= 0 ) response.CutBeg(pos+1);
      response.Trim();
   } else {
      Unexpected(response);
      return False;
   }

//
// Next should be the header info, with length of headers enclosed
// in curly braces. The headers will follow on subsequent lines.
//
   if ( response.StartsWith("RFC822.HEADER", IGNORE_CASE)) {
      pos = response.PosOf(' ');
      if ( pos >= 0 ) response.CutBeg(pos+1);
      response.Trim();
   } else {
      Unexpected(response);
      return False;
   }

//
// See if there is a length present
//
   Boolean seenEnd = False;	// flag: closing parenthesis found

   if ( response.StartsWith('{') ) {

      char	*lenp = response;
      char	*cp = ++lenp;
      while ( *cp != '}' ) cp++;
      *cp = 0;          // Temporary
      int	len = atoi(lenp);

//
// Allocate buffer of specified length
//
      *output = new char[len+1];
      **output = 0;

//
// Read until buffer is filled
//
      int	readlen = 0;
      while ( readlen < len && GetLine(response) ) {

	 //
	 // This means that the header length was too big - seems
	 // to happen sometimes...
	 //
         if (response.StartsWith(')') ) {
	    // Got all the data, clear the data pending flag
	    dataPending = False;
	    seenEnd = True;
	    break;
	 }

	 //
	 // Shouldn't see these yet...
	 //
	 if (response.StartsWith('*')  ||
	     response.StartsWith(fld->fetchTag) ) {
		Unexpected(response);
		return False;
	 }

         int	space = len - readlen;
         if ( response.size() >= space ) {
            strncat(*output, response, space);
            (*output)[len] = 0;
            response.CutBeg(space);
            readlen = len;
         }
         else {
            strcat(*output, response);
            strcat(*output, "\n");
            readlen += response.size() + 2;	// CRLF was stripped
         }
      }

  } else {
      Unexpected(response);
      return False;
  }
      
//
// The last response should have ended with a close parenthesis, indicating
// end of the FETCH data. If we haven't seen it yet, it should be on the
// next line.
//
  if (!seenEnd) {
    response.Trim();
    if (!response.EndsWith(')')) {
	GetLine(response);
	if (!response.EndsWith(')')) {
	  Unexpected(response);
	  return False;
	}
    }
    // Got it all, clear the data pending flag on the server object
    dataPending = False;
  }
  return True;

} // End FetchHdrs

/*------------------------------------------------------------------------
 * Function to flush trailing output from a FETCH command, after
 * processing one or more headers.
 */

void
ImapServerC::FetchFlush( StringC& tag )
{

//
// If last message in the folder, continue reading until we get the tag
//
    StringC response;
    while ( !response.StartsWith(tag) ) {
      if (!GetLine(response)) return;
    }
    return;

} // End FetchFlush


/*------------------------------------------------------------------------
 * Function to send STORE FLAGS command and wait for response.
 */

Boolean
ImapServerC::SetFlags(int msgnum, u_int state)
{
   if ( dataPending ) {
      // A command has been issued which has left data in transit currently!!
      // XXX - It seems rude, but maybe the solution is to open a second socket
      // to the IMAP server to set the flag(s)?
      cerr << "Avoiding logic flaw at " << __FILE__ << ":" <<
	      __LINE__ << " - status of message " << msgnum <<
	      " will not be changed\n";
      return False;
   }
   StringC	cmd = "STORE ";
   cmd += msgnum;
   cmd += " FLAGS (";

   Boolean	needSpace = False;
   if ( state & MSG_READ ) {
      if ( needSpace ) cmd += ' ';
      cmd += "\\Seen";
      needSpace = True;
   }
   if ( state & MSG_DELETED ) {
      if ( needSpace ) cmd += ' ';
      cmd += "\\Deleted";
      needSpace = True;
   }
   if ( state & MSG_REPLIED ) {
      if ( needSpace ) cmd += ' ';
      cmd += "\\Answered";
      needSpace = True;
   }
   if ( state & MSG_FLAGGED ) {
      if ( needSpace ) cmd += ' ';
      cmd += "\\Flagged";
      needSpace = True;
   }

#if 0
   if ( state & MSG_NEW ) {
      if ( needSpace ) cmd += ' ';
      cmd += "\\Recent";
      needSpace = True;
   }
   if ( state & MSG_SAVED ) {
      if ( needSpace ) cmd += ' ';
      cmd += "Saved";
      needSpace = True;
   }
   if ( state & MSG_FORWARDED ) {
      if ( needSpace ) cmd += ' ';
      cmd += "Forwarded";
      needSpace = True;
   }
   if ( state & MSG_RESENT ) {
      if ( needSpace ) cmd += ' ';
      cmd += "Resent";
      needSpace = True;
   }
   if ( state & MSG_PRINTED ) {
      if ( needSpace ) cmd += ' ';
      cmd += "Printed";
      needSpace = True;
   }
   if ( state & MSG_FILTERED ) {
      if ( needSpace ) cmd += ' ';
      cmd += "Filtered";
      needSpace = True;
   }
#endif

   // Send a "STORE <msgnum> -FLAGS (\Seen)" if the flags are empty
   if (cmd.EndsWith("FLAGS (")) {
      cmd = "STORE ";
      cmd += msgnum;
	  cmd += " -FLAGS (\\Seen";
   }

   cmd += ')';

   StringListC	output;
   return RunCommand(cmd, output);

} // End SetFlags

/*------------------------------------------------------------------------
 * Function to send COPY command and wait for response.
 */

Boolean
ImapServerC::Copy(int msgnum, CharC mailbox, StringListC& output)
{
   StringC	cmd = "COPY ";
   cmd += msgnum;
   cmd += ' ';
   cmd += mailbox;

   return RunCommand(cmd, output);
}

/*------------------------------------------------------------------------
 * Function to send APPEND command and wait for response.
 */

Boolean
ImapServerC::Append(CharC mailbox, CharC data, StringListC& output)
{
   StringC	tag;
   GenTag(tag);

   StringC	cmd = tag;
   cmd += "APPEND ";
   cmd += mailbox;
   cmd += " {";
   cmd += (int)data.Length();
   cmd += "}";

   if ( !PutLine(cmd) ) return False;

   if ( !GetLine(cmd) ) return False;
   while ( !cmd.StartsWith('+') && !cmd.StartsWith(tag) ) {
      output.add(cmd);
      if ( !GetLine(cmd) ) return False;
   }

//
// See if server is ready for data
//
   if ( !cmd.StartsWith('+') ) {
      cmd.CutBeg(tag.size());
      output.add(cmd);
      return False;
   }

//
// Send data
//
   if ( !PutLine(data) ) return False;

//
// Wait for tagged response
//
   if ( !GetLine(cmd) ) return False;
   while ( !cmd.StartsWith(tag) ) {
      output.add(cmd);
      if ( !GetLine(cmd) ) return False;
   }
   cmd.CutBeg(tag.size());
   output.add(cmd);

   // Parse output and make sure it worked!
   u_int       count = output.size();
   int i;
   for (i=0; i<count; i++) {
      CharC    resp = *output[i];
      if ( resp.StartsWith("NO ") || resp.StartsWith("BAD ") ) return False;
   }

   return True;

} // End Append

/*------------------------------------------------------------------------
 * Function to send APPEND command and wait for response.
 */

Boolean
ImapServerC::Append(CharC mailbox, StringListC& headList, char *bodyFile,
		    StringListC& output)
{
//
// Open body file
//
   FILE	*fp = fopen(bodyFile, "r");
   if ( !fp ) {
      StringC	errmsg("Could not open file \"");
      errmsg += bodyFile;
      errmsg += "\"\n";
      errmsg += SystemErrorMessage(errno);
      halApp->PopupMessage(errmsg);
      return False;
   }

//
// Get size of data
//
   int		size = 0;
   u_int	count = headList.size();
   StringC	*headStr;
   int	i;
   for (i=0; i<count; i++) {
      headStr = headList[i];
      size += headStr->size() + 1;  // Add one for the CR that will be added
   }
   size += 2;	// Blank line (including CR that will be added)

   fseek(fp, 0, SEEK_END);
   size += (int)ftell(fp);
   fseek(fp, 0, SEEK_SET);

//
// Build command
//
   StringC	tag;
   GenTag(tag);

   StringC	cmd = tag;
   cmd += "APPEND ";
   cmd += mailbox;
   cmd += " {";
   cmd += size;
   cmd += "}";

   Boolean	error = !PutLine(cmd);

   if ( !error ) error = !GetLine(cmd);
   while ( !error && !cmd.StartsWith('+') && !cmd.StartsWith(tag) ) {
      output.add(cmd);
      error = !GetLine(cmd);
   }

//
// See if server is ready for data
//
   if ( !error && !cmd.StartsWith('+') ) {
      cmd.CutBeg(tag.size());
      output.add(cmd);
      error = True;
   }

//
// Send headers
//
   count = headList.size();
   for (i=0; !error && i<count; i++) {
      headStr = headList[i];
      error = !PutLine(*headStr, /*terminate=*/True);
   }

//
// Send blank line
//
   if ( !error ) error = !PutLine("\n", /*terminate=*/True);

//
// Send body
//
#define BUFLEN	1024

   char	buffer[BUFLEN];
   buffer[0] = 0;

   if ( !error ) count = fread(buffer, 1, BUFLEN-1, fp);
   while ( !error && count > 0 ) {
      buffer[count] = 0;
      error = !PutLine(buffer, /*terminate=*/False);
      if ( !error ) count = fread(buffer, 1, BUFLEN-1, fp);
   }

   fclose(fp);

//
// Send termination
//
   if ( !error ) error = !PutLine("\n");

//
// Wait for tagged response
//
   if ( !error ) error = !GetLine(cmd);
   while ( !error && !cmd.StartsWith(tag) ) {
      output.add(cmd);
      error = !GetLine(cmd);
   }
   cmd.CutBeg(tag.size());
   output.add(cmd);

   // Parse output and make sure it worked!
   count = output.size();
   for (i=0; i<count; i++) {
      CharC    resp = *output[i];
      if ( resp.StartsWith("NO ") || resp.StartsWith("BAD ") ) return False;
   }

   return !error;

} // End Append

/*------------------------------------------------------------------------
 * Function to send FIND MAILBOXES command and wait for response.
 */

Boolean
ImapServerC::ListMailboxes(CharC pattern, StringListC& mailboxes,
					  StringListC& output)
{
   StringC	cmd = "FIND ALL.MAILBOXES ";
   cmd += pattern;

   Boolean	success = RunCommand(cmd, output);
   if ( !success ) return False;

//
// Loop through output and get names
//
   u_int	count = output.size();
   StringC	tmp;
   int	i;
   for (i=0; i<count; i++) {
      CharC	name = *output[i];
      if ( name.StartsWith("* MAILBOX ", IGNORE_CASE) ) {
	 name.CutBeg(10); 
	 name.Trim();
	 tmp = name;
	 mailboxes.add(tmp);
      }
   }

//
// Remove names from output
//
   count = output.size();
   for (i=count-1; i>=0; i--) {
      CharC	name = *output[i];
      if ( name.StartsWith("* MAILBOX ", IGNORE_CASE) )
	 output.remove(i);
   }

   return True;

} // End ListMailboxes

/*------------------------------------------------------------------------
 * Function to see if a mailbox exists
 */

Boolean
ImapServerC::Exists(CharC name)
{
   StringC	cmd = "FIND ALL.MAILBOXES ";
   cmd += name;

   StringListC	output;
   Boolean	success = RunCommand(cmd, output);
   if ( !success ) return False;

//
// Loop through output and check for name
//
   u_int	count = output.size();
   int	i;
   for (i=0; i<count; i++) {
      CharC	line = *output[i];
      if ( line.StartsWith("* MAILBOX ", IGNORE_CASE) ) {
	 line.CutBeg(10); 
	 line.Trim();
	 if ( line.Equals(name, IGNORE_CASE) ) return True;
      }
   }

   return False;

} // End Exists

/*---------------------------------------------------------------
 *  Method to ask user if they want to retry the login
 */

Boolean
ImapServerC::RetryLogin(char *msgStr)
{
   static QueryAnswerT	answer;

//
// Create the dialog if necessary
//
   static Widget	dialog = NULL;
   if ( !dialog ) {

      halApp->BusyCursor(True);

      WArgList	args;
      args.DialogStyle(XmDIALOG_FULL_APPLICATION_MODAL);
      if ( halApp->questionPM ) args.SymbolPixmap(halApp->questionPM);
      dialog = XmCreateQuestionDialog(*halApp, "retryLoginWin", ARGS);

      XtAddCallback(dialog, XmNokCallback, (XtCallbackProc)AnswerQuery,
		    (XtPointer)&answer);
      XtAddCallback(dialog, XmNcancelCallback,
		    (XtCallbackProc)AnswerQuery, (XtPointer)&answer);

//
// Trap window manager close function
//
      XmAddWMProtocolCallback(XtParent(dialog), halApp->delWinAtom,
			      (XtCallbackProc)WmClose,
			      (caddr_t)&answer);

      halApp->BusyCursor(False);

   } // End if query dialog not created

//
// Display the message
//
   WXmString    wstr = msgStr;
   XtVaSetValues(dialog, XmNmessageString, (XmString)wstr, NULL);

//
// Show the dialog
//
   XtManageChild(dialog);
   XMapRaised(halApp->display, XtWindow(XtParent(dialog)));

//
// Simulate the main event loop and wait for the answer
//
   answer = QUERY_NONE;
   while ( answer == QUERY_NONE ) {
      XtAppProcessEvent(halApp->context, XtIMXEvent);
      XSync(halApp->display, False);
   }

   XtUnmanageChild(dialog);
   XSync(halApp->display, False);
   XmUpdateDisplay(dialog);

   return (answer == QUERY_YES);

} // End RetryLogin

/*---------------------------------------------------------------
 *  Method to perform wildcard expansions on the names in a list.
 *  Any new names are added to the list.
 */

void
ImapServerC::ExpandList(StringListC& list, StringListC& output)
{
//
// Loop through strings
//
   u_int	count = list.size();
   for (int i=0; i<count; i++) {

      StringC	*name = list[i];

//
// If there are any wildcards in the name, ask the server to expand them
//
      if ( name->Contains('*') ||
	   name->Contains('%') ||
	   name->Contains('?') ) {

	 StringListC	nameList;
	 if ( ListMailboxes(*name, nameList, output) && nameList.size() > 0 ) {

//
// Add these names to the end of the list and remove the original pattern.
//
	    list.remove(i);
	    list += nameList;
	    count = list.size();
	    i--; // Since pattern was removed
	 }

      } // End if the name is a pattern

   } // End for each name

} // End ExpandList

/*------------------------------------------------------------------------

 * Function to send SEARCH command and wait for response.
 */

Boolean
ImapServerC::Search(CharC pattern, IntListC& numbers, StringListC& output)
{
   StringC	cmd = "SEARCH ";
   cmd += pattern;

   Boolean	success = RunCommand(cmd, output);
   if ( !success ) return False;

//
// Loop through output and get numbers
//
   u_int	count = output.size();
   StringC	tmp;
   char		numStr[16];
   int		num;
   int	i;
   for (i=0; i<count; i++) {

      CharC	line = *output[i];
      if ( line.StartsWith("* SEARCH ", IGNORE_CASE) ) {

	 line.CutBeg(9); 
	 line.Trim();

//
// Loop through numbers and add to list
//
	 u_int		offset = 0;
	 CharC		word = line.NextWord(offset);
	 while ( word.Length() > 0 ) {

	    strncpy(numStr, word.Addr(), word.Length());
	    numStr[word.Length()] = 0;

	    num = atoi(numStr);
	    numbers.add(num);

	    offset = word.Addr() - line.Addr() + word.Length();
	    word   = line.NextWord(offset);
	 }

      } // End if line is search result

   } // End for each output line

//
// Remove numbers from output
//
   count = output.size();
   for (i=count-1; i>=0; i--) {
      CharC	line = *output[i];
      if ( line.StartsWith("* SEARCH ", IGNORE_CASE) )
	 output.remove(i);
   }

   return True;

} // End Search

