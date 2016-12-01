#include "server.h"
#include "link.h"
#include "msg.h"
#include "net.h"
#include "cuckoo_kvstore.h"
#include "messages.h"


extern KVStore *kvstore;

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
	struct response_message *resp_msgs;
	uint16_t num_msgs = msg->size() / sizeof(struct send_message);
	resp_msgs = (struct response_message *)malloc(sizeof(struct response_message) * num_msgs);
	for (size_t i = 0; i < num_msgs; i++) {
		bool ret = false;
		if (msgs[i].operation == GET)
			ret = kvstore->find(KEY_CAST(msgs[i].key), &VALUE_CAST(resp_msgs[i].value));
		if (msgs[i].operation == PUT) {
			if (kvstore->contains(KEY_CAST(msgs[i].key)))
				ret = kvstore->update(KEY_CAST(msgs[i].key), VALUE_CAST(msgs[i].value));
			else
				ret = kvstore->insert(KEY_CAST(msgs[i].key), VALUE_CAST(msgs[i].key));
		}
		if (msgs[i].operation == DELETE)
			ret = kvstore->erase(KEY_CAST(msgs[i].key));

		if (ret == true)
			resp_msgs[i].status_code = OK;
		else
			resp_msgs[i].status_code = ERROR;

		memcpy(resp_msgs[i].key, msgs[i].key, sizeof(KVStore::Key));
	}
	delete msg;
	// populate the response buffer queue
	Buffer *rsp = new Buffer((char*)resp_msgs, sizeof(struct response_message) * num_msgs);

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
