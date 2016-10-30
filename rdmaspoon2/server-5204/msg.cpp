#include "server.h"
#include "link.h"
#include "msg.h"
#include "net.h"

rstatus_t req_recv(NetworkServer *proxy, Link *conn) {
	Buffer *msg = conn->msg_read();
	if (msg == NULL) {
		fprintf(stderr, "fd: %d, read failed, delete link", conn->fd());
		conn->mark_error();
		Fdevents *fdes = proxy->get_fdes();
		fdes->del(conn->fd());
		return CO_ERR;
	}

	// When the server receives a message from a client
	//It uses the rdmarespond function to get a reponse from the KV store using RDMA write
	//and sends a response back to the client as a message
	char *data = proxy->rdmarespond(msg->data());
	Buffer *rsp = new Buffer(data, sizeof(char)*strlen(data));

	//The steps to send a message to the client
	conn->omsg_q.push_back(rsp);
	Fdevents *fdes = proxy->get_fdes();
	fdes->set(conn->fd(), FDEVENT_OUT, 1, conn);

	return CO_OK;
}

//Unedited from original nonrdma server
rstatus_t rsp_send(NetworkServer *proxy, Link *conn) {

	// pop the response buffer queue
	// send response back
	Buffer *smsg;
	if (!conn->omsg_q.empty()) {
	smsg = conn->omsg_q.front(); conn->omsg_q.pop_front();
	}
	else{
		Fdevents *fdes = proxy->get_fdes();
		fdes->clr(conn->fd(), FDEVENT_OUT);  // yue: nothing to send so del ev
		return CO_OK;  // yue: nothing to send
	}
	if (smsg == NULL) {
		return CO_OK;  // yue: nothing to send
	}
	int len = conn->msg_write(smsg);
	if (len <= 0) {
		fprintf(stderr, "fd: %d, write: %d, delete link", conn->fd(), len);
		conn->mark_error();
		return CO_ERR;
	}

	return CO_OK;
}

