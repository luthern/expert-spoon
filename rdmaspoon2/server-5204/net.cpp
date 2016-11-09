
#include "server.h"
#include "net.h"
#include "resp.h"







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






//Initializes rdma connection with KV store
//Based on code from https://github.com/simonzhangsm/SoftiWarp/tree/master/rdma-tutorial-2007
void * rdmaclient(void *args)
{
	
	struct clientRequest *request;
	struct pdata			server_pdata;
	struct pdata			rep_pdata;
	struct rdma_event_channel      *cm_channel;
	struct rdma_cm_id	       *cm_id;
	struct rdma_cm_event	       *event;
	struct rdma_conn_param		conn_param = { };

	struct ibv_pd		       *pd;
	struct ibv_comp_channel	       *comp_chan;
	struct ibv_cq		       *cq;
	struct ibv_mr		       *mr;
	struct ibv_qp_init_attr		qp_attr = { };;

	struct addrinfo		       *res, *t;
	struct addrinfo 		hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	int				n;

	int				err;

	/* Set up RDMA CM structures */

	cm_channel = rdma_create_event_channel();
	if (!cm_channel)
		return NULL;

	err = rdma_create_id(cm_channel, &cm_id, NULL, RDMA_PS_TCP);
	if (err)
		return NULL;//err;

	n = getaddrinfo("127.0.0.1", "20079", &hints, &res);
	if (n < 0)
		return NULL;

	/* Resolve server address and route */

	for (t = res; t; t = t->ai_next) {
		err = rdma_resolve_addr(cm_id, NULL, t->ai_addr,
					RESOLVE_TIMEOUT_MS);
		if (!err)
			break;
	}
	if (err)
		return NULL;//err;

	err = rdma_get_cm_event(cm_channel, &event);
	if (err)
		return NULL;//err;

	if (event->event != RDMA_CM_EVENT_ADDR_RESOLVED)
		return NULL;

	rdma_ack_cm_event(event);

	err = rdma_resolve_route(cm_id, RESOLVE_TIMEOUT_MS);
	if (err)
		return NULL;//err;

	err = rdma_get_cm_event(cm_channel, &event);
	if (err)
		return NULL;//err;

	if (event->event != RDMA_CM_EVENT_ROUTE_RESOLVED)
		return NULL;

	rdma_ack_cm_event(event);;
	/* Create verbs objects now that we know which device to use */

	pd = ibv_alloc_pd(cm_id->verbs);
	if (!pd)
		return NULL;
	comp_chan = ibv_create_comp_channel(cm_id->verbs);
	if (!comp_chan)
		return NULL;
	cq = ibv_create_cq(cm_id->verbs, 20, NULL, comp_chan, 0);
	if (!cq)
		return NULL;
	if (ibv_req_notify_cq(cq, 0))
		return NULL;
	request = (struct clientRequest *) calloc(10, sizeof(struct clientRequest));
	mr = ibv_reg_mr(pd, request, 10 * sizeof (struct clientRequest), IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE | IBV_ACCESS_REMOTE_READ);
	if (!mr)
		return NULL;
	qp_attr.cap.max_send_wr	 = 2;
	qp_attr.cap.max_send_sge = 1;
	qp_attr.cap.max_recv_wr	 = 1;
	qp_attr.cap.max_recv_sge = 1;

	qp_attr.send_cq		 = cq;
	qp_attr.recv_cq		 = cq;

	qp_attr.qp_type		 = IBV_QPT_RC;

	err = rdma_create_qp(cm_id, pd, &qp_attr);
	if (err)
		return NULL;//err;

	conn_param.initiator_depth = 1;
	conn_param.retry_count	   = 7;
	conn_param.responder_resources = 1;
	/* Connect to server */

	err = rdma_connect(cm_id, &conn_param);
	if (err)
		return NULL;//err;

	err = rdma_get_cm_event(cm_channel, &event);
	if (err)
		return NULL;//err;

	if (event->event != RDMA_CM_EVENT_ESTABLISHED)
		return NULL;

	memcpy(&server_pdata, event->param.conn.private_data,
	       sizeof server_pdata);

	rdma_ack_cm_event(event);


	//
	struct rdmaParams * rdmaargs = (struct rdmaParams *) args;

	rdmaargs->request = request;
	rdmaargs->server_pdata = server_pdata;
	rdmaargs->rep_pdata = rep_pdata;
	rdmaargs->cm_channel = cm_channel;
	rdmaargs->cm_id = cm_id;
	rdmaargs->conn_param = conn_param;
	rdmaargs->pd = pd;
	rdmaargs->comp_chan = comp_chan;
	rdmaargs->cq = cq;
	rdmaargs->mr = mr;




	printf("RDMA ready!!!\n");
	//while(1){
	//	usleep(100000);
	//}




	return NULL;
}




//Parses msg and uses RDMA to get reponse from KV store
char * NetworkServer::rdmarespond(char *msg){
	memset(this->rdma.request[0].key, 0, 16);
	memset(this->rdma.request[0].value, 0, 32);
	struct ibv_sge			sge;
	struct ibv_send_wr		send_wr = { };
	struct ibv_send_wr	       *bad_send_wr;
	struct ibv_recv_wr		recv_wr = { };
	struct ibv_recv_wr	       *bad_recv_wr;
	struct ibv_wc			wc;

	struct send_message *recv_msg = (struct send_message *) msg;
	
	//Uses message contents to fill out a clientRequest struct
	//This struct is the memory region that will be written into the KV store
	//memory with RDMA
	//The message looks like this mode:key:value:
	//Like r:cow:hamburger: or w:pig:bacon:
	
	if(recv_msg->operation == 0){
		this->rdma.request[0].requestType = 'r';
	}
	else{
		this->rdma.request[0].requestType = 'w';
	}
	memcpy(this->rdma.request[0].key, recv_msg->key, 16);
	memcpy(this->rdma.request[0].value, recv_msg->value, 32);

	sge.addr   = (uintptr_t) this->rdma.request;
	sge.length = sizeof (struct clientRequest);
	sge.lkey   = this->rdma.mr->lkey;

	recv_wr.wr_id   = 0;
	recv_wr.sg_list = &sge;
	recv_wr.num_sge = 1;

	//Preposts an RDMA recv that designates where the clientRequest will be written
	//to by the KV store's RDMA send
	if (ibv_post_recv(this->rdma.cm_id->qp, &recv_wr, &bad_recv_wr))
		return NULL;

	//Changes request to state 1, indicating a request has been made
	this->rdma.request[0].state = 1;
	sge.addr   = (uintptr_t) &this->rdma.request[0];
	sge.length = sizeof (struct clientRequest);
	sge.lkey   = this->rdma.mr->lkey;
	send_wr.wr_id		    = 1;
	send_wr.opcode		    = IBV_WR_RDMA_WRITE;
	send_wr.sg_list		    = &sge;
	send_wr.num_sge		    = 1;

	//Rkey is used to identify the remote memory region on the KV store being written to
	send_wr.wr.rdma.rkey	    = ntohl(this->rdma.server_pdata.buf_rkey);

	//This is the remote memory address being written to on the KV store
	send_wr.wr.rdma.remote_addr = ntohll(this->rdma.server_pdata.buf_va);

	if (ibv_post_send(this->rdma.cm_id->qp, &send_wr, &bad_send_wr))
		return NULL;

	//Waits until the clientRequest struct has a state change
	//That would indicate that the KV store had responded to the request
	//with an RDMA send
	//Currently it just changes state to 2, but it could do different things to indicate
	//errors or something
	while(this->rdma.request[0].state==1){
	}

	struct response_message *resp = (struct response_message *)calloc(1, sizeof(struct response_message));
	resp->status_code = (int)rdma.request[0].requestType;
	strcpy((char *)resp->key, (char *)rdma.request[0].key);
	strcpy((char *)resp->value, (char *)rdma.request[0].value);

	//Changes the request struct state to 0, indicating it is free to be used for requests
	this->rdma.request[0].state=0;

	//Polls completion queue because if it fills up that's bad
	//The CQ is where work requests go after they were processed by queue pairs
	ibv_poll_cq(this->rdma.cq, 50, &wc);


	return (char *)resp;
	//Writes message back to client based on contents of clientRequest struct
	/*char *returnbuffer = (char *) calloc(100, sizeof(char));
	if(this->rdma.request[0].requestType == 'w')
	snprintf(returnbuffer, 100, "Wrote value %s to key %s\n", this->rdma.request[0].value, this->rdma.request[0].key);
	else{
	snprintf(returnbuffer, 100, "Read value %s from key %s\n", this->rdma.request[0].value, this->rdma.request[0].key);}*/

}





NetworkServer::NetworkServer(void) {
	fdes = new Fdevents();

	client_conn = Link::listen("127.0.0.1", 12345);
	kvlink = (Link *)malloc(sizeof(Link));
	if(client_conn == NULL){
		fprintf(stderr, "error opening server socket! %s\n", strerror(errno));
		exit(1);
	}
	conn_count = 0;
}

Link *NetworkServer::accept_conn() {
	Link *conn = client_conn->accept();
	if (conn == NULL) {
		fprintf(stderr, "accept failed: %s", strerror(errno));
		return NULL;
	}

	conn->nodelay();
	conn->noblock();
	conn->create_time = millitime();
	conn->active_time = conn->create_time;

	conn->set_nw_func();

	return conn;
}

int NetworkServer::main_loop(void) {


	pthread_t rdmathread;
	//Creates thread that initializes RDMA
	if(isrdma=='1'){
		pthread_create(&rdmathread, NULL, rdmaclient, &(this->rdma));
	}
	else{
		this->kvlink = Link::connect("127.0.0.1", 11211);
		kvlink->set_kv_nw_func();
		fdes->set(kvlink->fd(), FDEVENT_IN, 0, client_conn);
	}

	const Fdevents::events_t *events;

	//set client connections to be accepted
	fdes->set(client_conn->fd(), FDEVENT_IN, 0, client_conn);
	

	while (true) {
		events = fdes->wait(50); // yue: so far timeout hardcoded
		if (events == NULL) {
			return CO_ERR;
		}
		for (int i = 0; i < (int)events->size(); i++) {
			const Fdevent *fde = events->at(i);
			if (fde->data.ptr == client_conn) {
				Link *conn = accept_conn();
				if(isrdma=='1'){
					conn->set_rdma_nw_func();
				}
				else{
					conn->set_nw_func();
					clientlink = conn;
				}
				if (conn) {
					conn_count++;
					fdes->set(conn->fd(), FDEVENT_IN, 1, conn);
				}
			} else if (fde->events & FDEVENT_IN) { // ali: where we read data from client
				recv(fde);
			} else if (fde->events & FDEVENT_OUT) { // ali: where we send data to client
				send(fde);
			}
		}
	}

	return CO_OK;
}

/*
 * ali: wrapper calling registered funcs
 */
rstatus_t NetworkServer::recv(const Fdevent *fde) {
	Link *conn = (Link *)fde->data.ptr;
	if (conn->error())	return CO_ERR;

	rstatus_t ret = conn->recv_nw(this, conn);
	if (ret != CO_OK) {
		fprintf(stderr, "recv on %d failed: %s", conn->fd(), strerror(errno));
	}

	return ret;
}

/*
 * ali: wrapper calling registered funcs
 */
rstatus_t NetworkServer::send(const Fdevent *fde) {
	Link *conn = (Link *)fde->data.ptr;
	if (conn->error())	return CO_ERR;

	rstatus_t ret = conn->send_nw(this, conn);
	if (ret != CO_OK) {
		fprintf(stderr, "send on %d failed: %s", conn->fd(), strerror(errno));
	}

	return ret;
}

Fdevents *NetworkServer::get_fdes() {
	return fdes;
}

