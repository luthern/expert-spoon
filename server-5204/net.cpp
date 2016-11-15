

#include "server.h"
#include "net.h"
#include "resp.h"

NetworkServer::NetworkServer(void) {
	fdes = new Fdevents();

	client_conn = Link::listen("127.0.0.1", 12345);
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
