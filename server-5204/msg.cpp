#include "server.h"
#include "link.h"
#include "msg.h"
#include "net.h"
#include "messages.h"
#include "kvstorelib.h"

rstatus_t req_recv(NetworkServer *proxy, Link *conn)
{
	Buffer *msg = conn->msg_read();

	if (msg == NULL) {
		//fprintf(stderr, "fd: %d, read failed, delete link", conn->fd());
		conn->mark_error();
		Fdevents *fdes = proxy->get_fdes();
		fdes->del(conn->fd());
		return CO_ERR;
	}


	// Get all the sent messages
	// and respond back with a value that is just the key repeated twice
	struct send_message *msgs = (struct send_message *)msg->data();
	uint16_t num_msgs = msg->size() / sizeof(struct send_message);
	for (size_t i = 0; i < num_msgs; i++)
		kvstore_process_packet((char*)&msgs[i]);
	// populate the response buffer queue
	Buffer *rsp = new Buffer((char*)msgs, sizeof(struct response_message) * num_msgs);

	conn->omsg_q.push_back(rsp);
	Fdevents *fdes = proxy->get_fdes();
	fdes->set(conn->fd(), FDEVENT_OUT, 1, conn);

	return CO_OK;
}

rstatus_t rsp_send(NetworkServer *proxy, Link *conn)
{

	// pop the response buffer queue
	// send response back
	Buffer *smsg;

	if (!conn->omsg_q.empty()) {
		smsg = conn->omsg_q.front(); conn->omsg_q.pop_front();
	}else {
		Fdevents *fdes = proxy->get_fdes();
		fdes->clr(conn->fd(), FDEVENT_OUT);     // yue: nothing to send so del ev
		return CO_OK;                           // yue: nothing to send
	}
	if (smsg == NULL)
		return CO_OK;  // yue: nothing to send
	int len = conn->msg_write(smsg);
	delete smsg;
	if (len <= 0) {
		fprintf(stderr, "fd: %d, write: %d, delete link", conn->fd(), len);
		conn->mark_error();
		return CO_ERR;
	}
	return CO_OK;
}
