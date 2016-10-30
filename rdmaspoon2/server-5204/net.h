#ifndef _NET_H_
#define _NET_H_
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
//#include <iostream>

#include <infiniband/arch.h>
#include <rdma/rdma_cma.h>
#include "server.h"
#include "link.h"
#include "fde.h"

void * rdmaclient(void *args);


enum {
	RESOLVE_TIMEOUT_MS	= 5000,
};

struct pdata {
	uint64_t	buf_va;
	uint32_t	buf_rkey;
};

struct clientRequest{
	int state;
	char requestType;
	int index;
	char key[16];
	char value[32];
	int answernum;
};



struct rdmaParams{

	struct clientRequest *request;
	struct pdata			server_pdata;
	struct pdata			rep_pdata;
	struct rdma_event_channel      *cm_channel;
	struct rdma_cm_id	       *cm_id;
	struct rdma_conn_param		conn_param = { };
	struct ibv_pd		       *pd;
	struct ibv_comp_channel	       *comp_chan;
	struct ibv_cq		       *cq;
	struct ibv_mr		       *mr;
	void			       *cq_context;
};




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
		struct rdmaParams rdma;
		char * rdmarespond(char *msg);
	private:
		Fdevents *fdes;
		Link *client_conn;
		Link *server_conn;
		int conn_count;
};

#endif
