/*
 * $Id: SysErr.C,v 1.3 2000/05/07 12:26:11 fnevgeny Exp $
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

#include "StringC.h"
#include <errno.h>

/*---------------------------------------------------------------
 *  Function to return status string for given system error
 */

StringC
SystemErrorMessage(int error)
{
#ifdef HAVE_STRERROR
   return strerror(error);
#else
   char	*cs;
   switch(error) {
      case EPERM:	cs = "Not owner";			break;
      case ENOENT:	cs = "No such file or directory";	break;
      case ESRCH:	cs = "No such process";			break;
      case EINTR:	cs = "Interrupted system call";		break;
      case EIO:		cs = "I/O error";			break;
      case ENXIO:	cs = "No such device or address";	break;
      case E2BIG:	cs = "Arg list too long";		break;
      case ENOEXEC:	cs = "Exec format error";		break;
      case EBADF:	cs = "Bad file number";			break;
      case ECHILD:	cs = "No children";			break;
      case EAGAIN:	cs = "No more processes";		break;
      case ENOMEM:	cs = "Not enough core";			break;
      case EACCES:	cs = "Permission denied";		break;
      case EFAULT:	cs = "Bad address";			break;
      case ENOTBLK:	cs = "Block device required";		break;
      case EBUSY:	cs = "Mount device busy";		break;
      case EEXIST:	cs = "File exists";			break;
      case EXDEV:	cs = "Cross-device link";		break;
      case ENODEV:	cs = "No such device";			break;
      case ENOTDIR:	cs = "Not a directory";			break;
      case EISDIR:	cs = "Is a directory";			break;
      case EINVAL:	cs = "Invalid argument";		break;
      case ENFILE:	cs = "File table overflow";		break;
      case EMFILE:	cs = "Too many open files";		break;
      case ENOTTY:	cs = "Not a typewriter";		break;
      case ETXTBSY:	cs = "Text file busy";			break;
      case EFBIG:	cs = "File too large";			break;
      case ENOSPC:	cs = "No space left on device";		break;
      case ESPIPE:	cs = "Illegal seek";			break;
      case EROFS:	cs = "Read-only file system";		break;
      case EMLINK:	cs = "Too many links";			break;
      case EPIPE:	cs = "Broken pipe";			break;
      case EDOM:	cs = "Argument too large";		break;
      case ERANGE:	cs = "Result too large";		break;
      case EWOULDBLOCK:	cs = "Operation would block";		break;
      case EINPROGRESS:	cs = "Operation now in progress";	break;
      case EALREADY:	cs = "Operation already in progress";	break;
      case ENOTSOCK:	cs = "Socket operation on non-socket";	break;
      case EDESTADDRREQ:cs = "Destination address required";	break;
      case EMSGSIZE:	cs = "Message too long";		break;
      case EPROTOTYPE:	cs = "Protocol wrong type for socket";	break;
      case ENOPROTOOPT:	cs = "Protocol not available";		break;
      case EPROTONOSUPPORT: cs = "Protocol not supported";	break;
      case ESOCKTNOSUPPORT: cs = "Socket type not supported";	break;
      case EOPNOTSUPP:  cs = "Operation not supported on socket";break;
      case EPFNOSUPPORT:cs = "Protocol family not supported";	break;
      case EAFNOSUPPORT:
	 cs = "Address family not supported by protocol family";break;
      case EADDRINUSE:	cs = "Address already in use";		break;
      case EADDRNOTAVAIL:cs = "Can't assign requested address";	break;
      case ENETDOWN:	cs = "Network is down";			break;
      case ENETUNREACH:	cs = "Network is unreachable";		break;
      case ENETRESET:   cs ="Network dropped connection on reset";break;
      case ECONNABORTED:cs = "Software caused connection abort";break;
      case ECONNRESET:	cs = "Connection reset by peer";	break;
      case ENOBUFS:	cs = "No buffer space available";	break;
      case EISCONN:	cs = "Socket is already connected";	break;
      case ENOTCONN:	cs = "Socket is not connected";		break;
      case ESHUTDOWN:   cs = "Can't send after socket shutdown";break;
      case ETOOMANYREFS:cs = "Too many references: can't splice";break;
      case ETIMEDOUT:	cs = "Connection timed out";		break;
      case ECONNREFUSED:cs = "Connection refused";		break;
      case ELOOP:	cs = "Too many levels of symbolic links";break;
      case ENAMETOOLONG:cs = "File name too long";		break;
      case EHOSTDOWN:	cs = "Host is down";			break;
      case EHOSTUNREACH:cs = "No route to host";		break;
      case ENOTEMPTY:	cs = "Directory not empty";		break;
      case EPROCLIM:	cs = "Unknown user or Too many processes";	break;
      case EUSERS:	cs = "Too many users";			break;
      case EDQUOT:	cs = "Disc quota exceeded";		break;
      case ESTALE:	cs = "Stale NFS file handle";		break;
      case EREMOTE:     cs = "Too many levels of remote in path";break;
      case ENOSTR:	cs = "Device is not a stream";		break;
      case ETIME:	cs = "Timer expired";			break;
      case ENOSR:	cs = "Out of streams resources";	break;
      case ENOMSG:	cs = "No message of desired type";	break;
      case EBADMSG:     cs = "Trying to read unreadable message";break;
      case EIDRM:	cs = "Identifier removed";		break;
      case EDEADLK:	cs = "Deadlock condition.";		break;
      case ENOLCK:	cs = "No record locks available.";	break;
      case ENONET:	cs = "Machine is not on the network";	break;
      case ERREMOTE:	cs = "Object is remote";		break;
      case ENOLINK:	cs = "the link has been severed";	break;
      case EADV:	cs = "advertise error";			break;
      case ESRMNT:	cs = "srmount error";			break;
      case ECOMM:	cs = "Communication error on send";	break;
      case EPROTO:	cs = "Protocol error";			break;
      case EMULTIHOP:	cs = "multihop attempted";		break;
      case EDOTDOT:    	cs = "Cross mount point (not an error)";break;
      case EREMCHG:	cs = "Remote address changed";		break;
      case ENOSYS:	cs = "function not implemented";	break;
      default:
      {
	 StringC	msg("system error: ");
	 msg += error;
	 return msg;
      }

   } // End switch errno
   return cs;
#endif

} // End SystemErrorMessage

