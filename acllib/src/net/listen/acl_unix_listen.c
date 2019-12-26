/* System interfaces. */
#include "StdAfx.h"
#ifndef ACL_PREPARE_COMPILE

#include "stdlib/acl_define.h"

#ifdef ACL_BCB_COMPILER
#pragma hdrstop
#endif

#endif

#ifdef	ACL_UNIX
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <stdio.h>

/* Utility library. */

#include "stdlib/acl_sys_patch.h"
#include "stdlib/acl_msg.h"
#include "stdlib/acl_iostuff.h"
#include "net/acl_sane_inet.h"
#include "net/acl_listen.h"

/* acl_unix_listen - create UNIX-domain listener */

ACL_SOCKET acl_unix_listen(const char *addr, int backlog, unsigned flag)
{
#undef sun
	struct sockaddr_un sun;
	int    len = (int) strlen(addr);
	ACL_SOCKET sock;

	/*
	 * Translate address information to internal form.
	 */
	if (len >= (int) sizeof(sun.sun_path)) {
		acl_msg_error("%s(%d), %s: unix-domain name too long: %s",
			__FILE__, __LINE__, __FUNCTION__, addr);
		return ACL_SOCKET_INVALID;
	}

	memset((char *) &sun, 0, sizeof(sun));
	sun.sun_family = AF_UNIX;
#ifdef HAS_SUN_LEN
	sun.sun_len = len + 1;
#endif
	memcpy(sun.sun_path, addr, len + 1);

	/*
	 * Create a listener socket. Do whatever we can so we don't run into
	 * trouble when this process is restarted after crash.
	 */
	if ((sock = socket(AF_UNIX, SOCK_STREAM, 0)) == ACL_SOCKET_INVALID) {
		acl_msg_error("%s(%d), %s: socket: %s",
			__FILE__, __LINE__, __FUNCTION__, acl_last_serror());
		return ACL_SOCKET_INVALID;
	}

	(void) unlink(addr);

	if (bind(sock, (struct sockaddr *) & sun, sizeof(sun)) < 0) {
		acl_msg_error("%s(%d), %s: bind: %s: %s", __FILE__, __LINE__,
			__FUNCTION__, addr, acl_last_serror());
		return ACL_SOCKET_INVALID;
	}
#ifdef FCHMOD_UNIX_SOCKETS
	if (fchmod(sock, 0666) < 0)
		acl_msg_warn("%s(%d), %s: fchmod socket %s: %s", __FILE__,
			__LINE__, __FUNCTION__, addr, acl_last_serror());
#else
	if (chmod(addr, 0666) < 0)
		acl_msg_warn("%s(%d), %s: chmod socket %s: %s", __FILE__,
			__LINE__, __FUNCTION__, addr, acl_last_serror());
#endif
	acl_non_blocking(sock, flag & ACL_INET_FLAG_NBLOCK ?
		ACL_NON_BLOCKING : ACL_BLOCKING);

	if (listen(sock, backlog) < 0) {
		acl_msg_error("%s(%d), %s: listen %s error %s", __FILE__,
			__LINE__, __FUNCTION__, addr, acl_last_serror());
		acl_socket_close(sock);
		return ACL_SOCKET_INVALID;
	}

	return sock;
}

/* acl_unix_accept - accept connection */

ACL_SOCKET acl_unix_accept(ACL_SOCKET fd)
{
	return acl_sane_accept(fd, (struct sockaddr *) 0, (socklen_t *) 0);
}

#endif
