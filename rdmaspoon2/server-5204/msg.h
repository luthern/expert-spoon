#ifndef _MSG_H_
#define _MSG_H_

#include "server.h"
#include "net.h"
#include "link.h"
#include "fde.h"
#include "util/bytes.h"


rstatus_t req_recv(NetworkServer *proxy, Link *conn);
rstatus_t rsp_send(NetworkServer *proxy, Link *conn);

#endif
