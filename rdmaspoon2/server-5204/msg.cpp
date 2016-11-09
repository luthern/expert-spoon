#include "server.h"
#include "link.h"
#include "msg.h"
#include "net.h"
#include <iostream>
#include "ckv_proto.pb.h"

rstatus_t rdma_req_recv(NetworkServer *proxy, Link *conn) {
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
	Buffer *rsp = new Buffer(data, sizeof(struct response_message));

	//The steps to send a message to the client
	conn->omsg_q.push_back(rsp);
	Fdevents *fdes = proxy->get_fdes();
	fdes->set(conn->fd(), FDEVENT_OUT, 1, conn);

	return CO_OK;
}


rstatus_t req_recv(NetworkServer *proxy, Link *conn) {

	Buffer *msg = conn->msg_read();
	if (msg == NULL) {
		//fprintf(stderr, "fd: %d, read failed, delete link", conn->fd());
		conn->mark_error();
		Fdevents *fdes = proxy->get_fdes();
		fdes->del(conn->fd());
		return CO_ERR;
	}

    	struct send_message *msgs = (struct send_message *)msg->data();
    	struct response_message *resp_msgs;
    	uint16_t num_msgs = msg->size() / sizeof(struct send_message);

	
	for(size_t i = 0; i < num_msgs; i++){

		protoSpec::Request req;
		protoSpec::PutMessage *put;
		protoSpec::GetMessage *get;
		if(msgs[i].operation == 0){
			printf("Get\n");
			
			get = req.mutable_get();
			req.set_op(protoSpec::Request::Operation::Request_Operation_GET);
			get->set_key((const char *)msgs[i].key, (size_t)16);
			std::cout << "Key: " << get->key() << "\n";
		}
		else{
			printf("Put\n");	
			
			put = req.mutable_put();
			req.set_op(protoSpec::Request::Operation::Request_Operation_PUT);
			put->set_key((const char *)msgs[i].key, (size_t)16);
			put->set_value((const char *)msgs[i].value, (size_t)32);
			std::cout << "Key: " << put->key() << "\n";
			std::cout << "Value: " << put->value() << "\n";
		};
		std::string data = req.SerializeAsString();;;
		size_t dataLen = data.length();
		size_t typeLen;
		if(msgs[i].operation ==0){
			typeLen =  get->GetTypeName().length();
		}
		else{
			typeLen =  put->GetTypeName().length();
		}
		size_t totalLen = (typeLen + sizeof(typeLen) +
					       dataLen + sizeof(dataLen) +
					       sizeof(totalLen) + 
											 sizeof(uint32_t));
		void * ptr = malloc(totalLen);
;
		*((uint32_t *)ptr) = MAGIC;

		ptr += sizeof(uint32_t);

		*((size_t *)ptr) = totalLen;
		ptr += sizeof(size_t);

		*((size_t *)ptr) = typeLen;
		ptr += sizeof(size_t);
		if(msgs[i].operation ==0){
			memcpy(ptr,  get->GetTypeName().c_str(), typeLen);
		}
		else{
			memcpy(ptr,  put->GetTypeName().c_str(), typeLen);
		}
		ptr += typeLen;

		*((size_t *)ptr) = dataLen;
		ptr += sizeof(size_t);

		memcpy(ptr, data.c_str(), dataLen);
		ptr += dataLen;
		ptr = ptr - totalLen;


		Buffer *msg2 = new Buffer(totalLen);
		
		msg2->append(ptr, totalLen);
		proxy->kvlink->omsg_q.push_back(msg2);
		Fdevents *fdes = proxy->get_fdes();
		fdes->set(proxy->kvlink->fd(), FDEVENT_OUT, 1, proxy->kvlink);
		fdes->set(proxy->kvlink->fd(), FDEVENT_IN, 1, proxy->kvlink);
	}

	return CO_OK;

}


rstatus_t kv_req_recv(NetworkServer *proxy, Link *conn) {
	printf("KV Store received\n");
	struct response_message *resp = (struct response_message *)calloc(2, sizeof(struct response_message));
	Buffer *msg = conn->msg_read();
	char *ptr = msg->data();
	uint32_t *magic;
	magic = (uint32_t *)ptr;



	ptr += sizeof(uint32_t);

	size_t *sz  = (size_t *)ptr;
	size_t totalSize = *sz;

	ptr += sizeof(size_t);

	size_t typeLen = *((size_t *)ptr);
	ptr += sizeof(size_t);

	ptr += typeLen;

	size_t dataLen = *((size_t *)ptr);
	ptr += sizeof(size_t);

	protoSpec::Reply rep;

	std::string str;
	str.assign((char *) ptr, dataLen);

	rep.ParseFromString(str);

	const char *dummykey = "key";
	strcpy((char *)resp->key, (const char *)dummykey);
	strcpy((char *)resp->value, (const char *)rep.value().c_str());

	resp->status_code = 0;

	Buffer *rsp = new Buffer((char *)resp, sizeof(struct response_message));
;
	proxy->clientlink->omsg_q.push_back(rsp);
	Fdevents *fdes = proxy->get_fdes();
	fdes->set(proxy->clientlink->fd(), FDEVENT_OUT, 1, proxy->clientlink);
	fdes->set(proxy->clientlink->fd(), FDEVENT_IN, 1, proxy->clientlink);
	return CO_OK;
}



//Unedited from original nonrdma server
rstatus_t rsp_send(NetworkServer *proxy, Link *conn) {
	printf("Sending\n");
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
