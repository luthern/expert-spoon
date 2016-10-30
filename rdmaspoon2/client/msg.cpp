#include "server.h"
#include "link.h"
#include "msg.h"
#include "net.h"

//Prints data of message after receiving it
rstatus_t req_recv(NetworkServer *proxy, Link *conn) {

	Buffer *msg = conn->msg_read();
	if (msg == NULL) {
		fprintf(stderr, "fd: %d, read failed, delete link", conn->fd());
		conn->mark_error();
		Fdevents *fdes = proxy->get_fdes();
		fdes->del(conn->fd());
		return CO_ERR;
	}
	printf("Received:\n");
	printf("%s\n", msg->data());
	return CO_OK;
}

//Sends a message to connection and print the data it is sending
//Exactly the same as the server's rsp_send
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
	printf("Sending:\n");
	printf("%s\n", smsg->data());
	int len = conn->msg_write(smsg);
	if (len <= 0) {
		fprintf(stderr, "fd: %d, write: %d, delete link", conn->fd(), len);
		conn->mark_error();
		return CO_ERR;
	}

	return CO_OK;
}

