#include "server.h"
#include "link.h"
#include "msg.h"
#include "net.h"

struct response_message{
    uint8_t status_code;
    char key[16];
    char value[32];
};

struct send_message {
    uint8_t operation;
    char key[16];
};


rstatus_t req_recv(NetworkServer *proxy, Link *conn) {
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
    struct response_message *resp_msgs;
    uint16_t num_msgs = msg->size() / sizeof(struct send_message);
    resp_msgs = (struct response_message *)malloc(sizeof(struct response_message) * num_msgs);
    for(size_t i = 0; i < num_msgs; i++){
        resp_msgs[i].status_code = 0;
        memcpy(resp_msgs[i].key, msgs[i].key, 16);
        memcpy(resp_msgs[i].value, msgs[i].key, 16);
        memcpy(resp_msgs[i].value + 16, msgs[i].key, 16);
    }
	// populate the response buffer queue
	Buffer *rsp = new Buffer((char *)resp_msgs, sizeof(struct response_message) * num_msgs);

	conn->omsg_q.push_back(rsp);
	Fdevents *fdes = proxy->get_fdes();
	fdes->set(conn->fd(), FDEVENT_OUT, 1, conn);

	return CO_OK;
}

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
