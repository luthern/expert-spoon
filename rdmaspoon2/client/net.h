#ifndef _NET_H_
#define _NET_H_

#include "server.h"
#include "link.h"
#include "fde.h"

class Link;

class NetworkServer {
	public:
		NetworkServer();
		rstatus_t main_loop();
		Link *proxy_listen();
		Link *connect_to_server();
		Link *accept_conn();
		rstatus_t send(const Fdevent *fde);
		rstatus_t recv(const Fdevent *fde);
		Fdevents *get_fdes();
		rstatus_t proc_info(Link *link);
		Link *client_conn;
	private:
		Fdevents *fdes;

		Link *server_conn;
		int conn_count;
};

#endif
