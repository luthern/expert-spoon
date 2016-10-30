
#include "server.h"
#include "net.h"
#include "resp.h"
#include "unistd.h"
#include <time.h>  
#include <pthread.h>

//Instead of listening for connections connects to specific address
//with Link::connect(char *ip, int port)
NetworkServer::NetworkServer(void) {
	fdes = new Fdevents();
	client_conn = Link::connect("127.0.0.1", 12345);
	client_conn->set_nw_func();
	//client_conn = Link::listen("127.0.0.1", 12345);
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

//Function that allows a thread to send messages at regular intervals
void * messageSender(void *args){

	NetworkServer *proxy = (NetworkServer *)args;
	
	while(1){
		//Adds some delay between messages
		usleep(1000000);


		//Prepares a message sending event
		//Copied from server's req_recv in msg.cpp
		char *data = (char *) calloc(60, sizeof(char));

		//First char of message determines if it is for reading or writing
		if(rand()%2==0){
			data[0] = 'r';
		}
		else{
			data[0] = 'w';
		}

		//Randomly generates the rest of message
		//Uses syntax mode:key:value: (just a random example protocol I made up can be redesigned to whatever)
		//where mode is r or w (for read or write)
		//key is string up to 16 bits (the key to read or write to)
		//value is string up to 32 bits (the value to write into the key, if mode is w)
		//but this line of code just makes both key and value 
		//a char of a number from 0 to 9
		snprintf(&data[1], sizeof(char)*59, ":%d:%d:", rand()%10, rand()%10);

		//Pushes response buffer onto the message queue
		//And sets up a send event like the server does in msg.cpp
		Buffer *rsp = new Buffer(data, sizeof(char)*strlen(data));
		proxy->client_conn->omsg_q.push_back(rsp);
		proxy->get_fdes()->set(proxy->client_conn->fd(), FDEVENT_OUT, 1, proxy->client_conn);
	}
	return NULL;
}



int NetworkServer::main_loop(void) {
	srand (time(NULL));
	pthread_t sendThread;
	const Fdevents::events_t *events;

	//set client connections to be accepted (leftover from server)
	fdes->set(client_conn->fd(), FDEVENT_IN, 0, client_conn);

	//Start thread that periodically sends a randomized message to the server
	pthread_create(&sendThread, NULL, messageSender, this);

	//While loop checks for send and recv events forever
	while (true) {
		events = fdes->wait(50); // yue: so far timeout hardcoded
		if (events == NULL) {
			return CO_ERR;
		}
		for (int i = 0; i < (int)events->size(); i++) {
			const Fdevent *fde = events->at(i);

			if (fde->events & FDEVENT_IN) { // ali: where we read data from client
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

