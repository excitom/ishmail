/*
 * IMAP-specific defines
 */

#ifndef _Imap_h_
#define _Imap_h_

#define IMAP_PORT	143
#define IMAPS_PORT	993

// Known IMAP protocols
#define     IMAP_PROTOCOL_IMAP2BIS      (1L << 0)
#define     IMAP_PROTOCOL_IMAP4         (1L << 1)
#define     IMAP_PROTOCOL_IMAP4REV1     (1L << 2)

// Known authentication methods
#define     IMAP_AUTH_CLEAR_TEXT        (1L << 0) /* IMAP2 LOGIN */
#define     IMAP_AUTH_ANONYMOUS         (1L << 1) /* anonymous */
#define     IMAP_AUTH_LOGIN             (1L << 2) /* base64-encoded clear text */
#define     IMAP_AUTH_CRAM_MD5          (1L << 3) /* reasonably secure */
#define     IMAP_AUTH_GSSAPI            (1L << 4)
#define     IMAP_AUTH_KERBEROS_V4       (1L << 5)

// Known IMAP server bugs
#define     IMAP_BUG_DUPLICATE_SELECT   (1L << 0)
#define     IMAP_BUG_STARTTLS_BROKEN    (1L << 1)
// more to come ;-)

#endif  // _Imap_h_
