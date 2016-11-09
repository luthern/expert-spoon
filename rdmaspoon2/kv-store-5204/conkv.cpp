#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unordered_map>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <event.h>
#include <pthread.h>
#include <sys/un.h>
#include <sysexits.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <glog/logging.h>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
//#include <iostream>
#include <infiniband/arch.h>
#include <rdma/rdma_cma.h>
#include <unistd.h>
#include <inttypes.h>

#include "conkv.h"
#include "link.h"

#include "ckv_proto.pb.h"

#define ITEMS_PER_ALLOC 64

#define USE_STL_MAP








struct settings settings;
unsigned int hashpower = HASHPOWER_DEFAULT;

static struct event_base *main_base;
static LIBEVENT_THREAD *threads;
static LIBEVENT_THREAD dispatcher;

static int init_count = 0;
static pthread_mutex_t init_lock;
static pthread_cond_t init_cond;

#ifdef USE_STL_MAP
std::unordered_map<std::string, std::string> *hashmap;
#else
static item **hashtable = 0;
#endif

//static Link *conn;

static std::deque<CQ_ITEM *> cqi_freelist;
pthread_mutex_t cqi_freelist_lock;

static void settings_init(void);
static void ht_init(unsigned int hashtable_init);
static void slabs_init(const size_t limit, const double factor, const bool prealloc);
static int try_read_network(Link *c);
static bool update_event(Link *c, const int new_flags);
static void compose_reply(Link *c, protoSpec::Reply &reply, const std::string &type);
void event_handler(const int fd, const short which, void *arg);
static void handle_request(Link *c);
static void conn_close(Link *c);
static void process(Link *c);
static void kvs_thread_init(int num_threads);
static void setup_thread(LIBEVENT_THREAD *me);
static void wait_for_thread_reg(int nthreads);
static int server_socket(int port);
static Link *register_conn(int fd, conn_t type, const int event_flags,
		struct event_base *base);
static CQ_ITEM *cqi_new();
static void cqi_free(CQ_ITEM *item);
static void cq_push(LIBEVENT_THREAD *t, CQ_ITEM *item);
static CQ_ITEM *cq_pop(LIBEVENT_THREAD *t);

static void settings_init(void) {
	settings.maxbytes = 64 * 1024 * 1024;
	settings.inter = NULL;
	settings.port = 11211;
	settings.num_threads = 1;
	settings.hashpower_init = 0;
	settings.use_slabs = false;
	settings.factor = 1.25;
}









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



struct threadctx{
	int *buffer;
	struct rdma_cm_id	       *cm_id;
	struct pdata			*client_pdata;
	struct ibv_mr		       *mr;
	struct ibv_cq *cq;
	struct globalPollThread *global;
	struct clientRequest *request;
};


struct connectThread{
	struct rdma_event_channel      *cm_channel;
	struct rdma_cm_id	       *cm_id;
	struct globalConnectThread *global;
};
	
struct globalConnectThread{
	int first;
	//struct ibv_pd		       *pd;
	//struct ibv_mr		       *mr;
	//int *buf;
};

struct globalPollThread{
	int array[10];

};

void *setupThread(void *buffer){
	//struct rdma_cm_event	       *event;
	//struct rdma_conn_param		conn_param = { };


	//struct ibv_comp_channel	       *comp_chan;
	//struct ibv_cq		       *cq;
	//struct ibv_cq		       *evt_cq;

	//struct ibv_qp_init_attr		qp_attr = { };
	//struct connectThread *connectData = (struct connectThread *)buffer;
	//struct rdma_event_channel cm_channel = connectData->cm_channel;
	//struct rdma_cm_id cm_id = connectData->cm_id;
	
	//if(connectData->global->first==1){
	//	connectData->global->first-=1;
	//	connectData->global->pd = ibv_alloc_pd(cm_id->verbs);
	//	connectData->global->buf = (uint32_t *) calloc(2, sizeof (uint32_t));
	//	if (!connectData->global->buf)
	//		return 1;

	//	connectData->global->mr = ibv_reg_mr(connectData->global->pd, connectData->global->buf, 2 * sizeof (uint32_t),
	//			IBV_ACCESS_LOCAL_WRITE |
	//			IBV_ACCESS_REMOTE_READ |
	//			IBV_ACCESS_REMOTE_WRITE);
	//	if (!mr)
	//		return 1;
	return NULL;

	
}






void *inspectArray(void *array){
	struct ibv_sge			sge;
	struct ibv_send_wr		send_wr = { };
	struct ibv_send_wr	       *bad_send_wr;
	struct ibv_wc			wc;
	struct threadctx *ctx = (struct threadctx *)array;
	int q;

	//Continually checks if clientRequest struct has been written to
	//When it has the KV store makes a response and RDMA sends it to the server
	while(1){
		if(ctx->request[0].state==1){
			ibv_poll_cq(ctx->cq, 50, &wc);
			if(ctx->request[0].requestType=='r'){
				printf("\nReading from key %s\n", ctx->request[0].key);
				if (hashmap->find(ctx->request[0].key) != hashmap->end()) {
			  		std::string value = hashmap->at(ctx->request[0].key);;
					const char *valuecstring = value.c_str();
					for(q=0;q<(int)value.length();q++){
						ctx->request[0].value[q] = valuecstring[q];
					}
					ctx->request[0].state = 2;
				} else {
					ctx->request[0].value[0] = 'z';
					ctx->request[0].value[0] = '\0';
					ctx->request[0].state = 2;
				}
			}
			if(ctx->request[0].requestType=='w'){
				printf("\nWriting value %s to key %s\n", ctx->request[0].value, ctx->request[0].key);

				auto item = hashmap->find(ctx->request[0].key);
				if (item != hashmap->end()) { // yue: if kv pair exists
					item->second = ctx->request[0].value;
					ctx->request[0].state = 2;
				} 
				else { // yue: if kv pair does not exist
					hashmap->insert(std::make_pair(ctx->request[0].key, ctx->request[0].value));
					ctx->request[0].state = 2;
				}
			}

			//Uses RDMA send to write response to clientRequest struct on the server
			sge.addr   = (uintptr_t) &ctx->request[0];
			sge.length = sizeof (struct clientRequest);
			sge.lkey   = ctx->mr->lkey;

			send_wr.wr_id		    = 1;
			send_wr.opcode		    = IBV_WR_SEND;
			send_wr.send_flags          = IBV_SEND_SIGNALED;
			send_wr.sg_list		    = &sge;
			send_wr.num_sge		    = 1;

			if (ibv_post_send(ctx->cm_id->qp, &send_wr, &bad_send_wr))
				return NULL;

			

			ctx->request[0].state=0;
		}
	}
			
}

//
void * rdmaserver(void *args)
{
	//int connectthreadnum = 0;
	//int pollthreadnum = 0;
	struct clientRequest *request;
	struct threadctx		threadstuff[100];
	//struct connectThread		connectThreadStuff[50];
	struct pdata			client_pdata;
	struct pdata			rep_pdata;
	pthread_t thread;
	struct rdma_event_channel      *cm_channel;
	struct rdma_cm_id	       *listen_id;
	struct rdma_cm_id	       *cm_id;
	struct rdma_cm_event	       *event;
	struct rdma_conn_param		conn_param = { };

	struct ibv_pd		       *pd;
	struct ibv_comp_channel	       *comp_chan;
	struct ibv_cq		       *cq;
	struct ibv_mr		       *mr;
	struct ibv_qp_init_attr		qp_attr = { };

	struct sockaddr_in		sin;

	uint32_t		       *buf;

	int				err;

	/* Set up RDMA CM structures */
	cm_channel = rdma_create_event_channel();
	if (!cm_channel)
		return NULL;

	err = rdma_create_id(cm_channel, &listen_id, NULL, RDMA_PS_TCP);
	if (err)
		return NULL;//err;

	sin.sin_family	    = AF_INET;
	sin.sin_port	    = htons(20079);
	sin.sin_addr.s_addr = INADDR_ANY;

	/* Bind to local port and listen for connection request */

	err = rdma_bind_addr(listen_id, (struct sockaddr *) &sin);
	if (err)
		return NULL;

	//while(1){
		err = rdma_listen(listen_id, 1);
		if (err)
			return NULL;

		err = rdma_get_cm_event(cm_channel, &event);
		if (err)
			return NULL;//err;

		if (event->event != RDMA_CM_EVENT_CONNECT_REQUEST)
			return NULL;

		cm_id = event->id;

		rdma_ack_cm_event(event);
	//	connectThreadStuff[connectthreadnum].cm_id = cm_id;
	//	connectThreadStuff[connectthreadnum].cm_channel = cm_channel;
	//	connectThreadStuff[connectthreadnum].global = &globalConnect;
	//	pthread_create(&connectThread[connectthreadnum], NULL, (void *)&setupThread, (void *)&connectThreadStuff[connectthreadnum]);
		
	//}
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
	//printf("Hi\n");
	buf = (uint32_t *) calloc(2, sizeof (uint32_t));
	request = (struct clientRequest *) calloc(10, sizeof(struct clientRequest));
	if (!buf)
		return NULL;

	mr = ibv_reg_mr(pd, request, sizeof (struct clientRequest),
			IBV_ACCESS_LOCAL_WRITE |
			IBV_ACCESS_REMOTE_READ |
			IBV_ACCESS_REMOTE_WRITE);
	if (!mr)
		return NULL;

	qp_attr.cap.max_send_wr	 = 1;
	qp_attr.cap.max_send_sge = 1;
	qp_attr.cap.max_recv_wr	 = 1;
	qp_attr.cap.max_recv_sge = 1;

	qp_attr.send_cq		 = cq;
	qp_attr.recv_cq		 = cq;

	qp_attr.qp_type		 = IBV_QPT_RC;

	err = rdma_create_qp(cm_id, pd, &qp_attr);
	if (err)
		return NULL;//err;


	rep_pdata.buf_va   = htonll((uintptr_t) request);
	rep_pdata.buf_rkey = htonl(mr->rkey);

	conn_param.responder_resources = 1;
	conn_param.private_data	       = &rep_pdata;
	conn_param.private_data_len    = sizeof rep_pdata;

	/* Accept connection */

	err = rdma_accept(cm_id, &conn_param);
	if (err)
		return NULL;

	err = rdma_get_cm_event(cm_channel, &event);
	if (err)
		return NULL;//err;

	if (event->event != RDMA_CM_EVENT_ESTABLISHED)
		return NULL;

	rdma_ack_cm_event(event);

	threadstuff[0].cm_id = cm_id;
	threadstuff[0].client_pdata = &client_pdata;
	threadstuff[0].mr = mr;
	threadstuff[0].cq = cq;
	threadstuff[0].request = request;

	//Creates a thread that repeatedly checks if a clientRequest has been written by the server
	//Code could be updated to create multiple threads that check different ranges of clientRequests
	//or threads that read/write to different specific sections of the KV store
	pthread_create(&thread, NULL, inspectArray, (void *)&threadstuff[0]);
	
	//while(1){
	//	usleep(1000);
	//}

	return NULL;
}




/*
 * yue: hash table initialization
 */
static void ht_init(unsigned int hashtable_init) {
#ifdef USE_STL_MAP
	hashmap = new std::unordered_map<std::string, std::string>();
	CHECK(hashmap != NULL);
#else
	if (hashtable_init) {
		hashpower = hashtable_init;
	}
	hashtable = (item **)calloc(hashsize(hashpower), sizeof(void *));
	if (!hashtable) {
		fprintf(stderr, "Failed to init hashtable\n");
		exit(EXIT_FAILURE);
	}
#endif
}

/*
 * yue: so far I am thinking to just use malloc
 */
static void slabs_init(const size_t limit, const double factor, const bool prealloc) {
	;
}

static void thread_ev_proc(int fd, short which, void *arg) {
	char buf[1];
	if (::read(fd, buf, 1) != 1) {
		fprintf(stderr, "thread_ev_proc: failed in dumb read\n");
		return;
	}

	LIBEVENT_THREAD *me = (LIBEVENT_THREAD *)arg;
	CQ_ITEM *item = cq_pop(me);
	if (item != NULL) {
		Link *c = register_conn(item->sfd, item->type, item->event_flags, me->base);
		if (c == NULL) {
			fprintf(stderr, "Worker-%lu: Can't listen for events on sfd %d\n", 
					me->thread_id, item->sfd);
			close(item->sfd);
		} else {
			c->thread = me;
		}
		cqi_free(item);
	}
}

static void setup_thread(LIBEVENT_THREAD *me) {
	me->base = event_init();
	if (!me->base) {
		fprintf(stderr, "Can't allocate event base for thread\n");
		exit(EXIT_FAILURE);
	}
	event_set(&me->notify_event, me->notify_receive_fd, 
			EV_READ|EV_PERSIST, thread_ev_proc, me);
	event_base_set(me->base, &me->notify_event);
	if (event_add(&me->notify_event, 0) == -1) {
		fprintf(stderr, "Can't monitor libevent notify pipe\n");
		exit(EXIT_FAILURE);
	}

	me->new_conn_q = new std::deque<CQ_ITEM *>();
	pthread_mutex_init(&me->cq_lock, NULL);
}

static void *worker_loop(void *arg) {
	LIBEVENT_THREAD *me = (LIBEVENT_THREAD *)arg;
	pthread_mutex_lock(&init_lock);
	init_count++;
	pthread_cond_signal(&init_cond);
	pthread_mutex_unlock(&init_lock);

	event_base_loop(me->base, 0);
	return NULL;
}

static void create_worker(void *(*func)(void *), void *arg) {
	LIBEVENT_THREAD *me = (LIBEVENT_THREAD *)arg;
	pthread_attr_t attr;
	int ret;
	pthread_attr_init(&attr);

	if ((ret = pthread_create(&me->thread_id, &attr, func, arg)) != 0) {
		fprintf(stderr, "Can't create worker thread: %s\n", strerror(ret));
		exit(EXIT_FAILURE);
	}
}

static void wait_for_thread_reg(int nthreads) {
	while (init_count < nthreads) {
		pthread_cond_wait(&init_cond, &init_lock);
	}
}

static void kvs_thread_init(int nthreads) {
	int i;
	//int power;
	pthread_mutex_init(&cqi_freelist_lock, NULL);

	dispatcher.base = main_base;
	dispatcher.thread_id = pthread_self();

	threads = (LIBEVENT_THREAD *)malloc(nthreads * sizeof(LIBEVENT_THREAD));
	for (i = 0; i < nthreads; i++) {
		int fds[2];
		if (pipe(fds)) {
			perror("Can't create notify pipe");
			exit(EXIT_FAILURE);
		}
		threads[i].notify_receive_fd = fds[0];
		threads[i].notify_send_fd = fds[1];
		setup_thread(&threads[i]);
	}
	for (i = 0; i < nthreads; i++) {
		create_worker(worker_loop, &threads[i]);
	}
	pthread_mutex_lock(&init_lock);
	wait_for_thread_reg(nthreads);
	pthread_mutex_unlock(&init_lock);
}

static int new_socket(struct addrinfo *ai) {
	int sfd, flags;
	if ((sfd = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol)) == -1) {
		return -1;
	}
	if ((flags = fcntl(sfd, F_GETFL, 0)) < 0 || 
			fcntl(sfd, F_SETFL, flags | O_NONBLOCK) < 0) {
		perror("setting O_NONBLOCK");
		close(sfd);
		return -1;
	}
	return sfd;
}

static CQ_ITEM *cqi_new() {
	CQ_ITEM *item = NULL;
	pthread_mutex_lock(&cqi_freelist_lock);
	if (!cqi_freelist.empty()) {
		item = cqi_freelist.front();
		cqi_freelist.pop_front();
		pthread_mutex_unlock(&cqi_freelist_lock);
		CHECK(item != NULL);
		return item;
	}
	pthread_mutex_unlock(&cqi_freelist_lock);

	CQ_ITEM *items = NULL;
	int i;
	items = (CQ_ITEM *)malloc(sizeof(CQ_ITEM) * ITEMS_PER_ALLOC);
	if (items == NULL)	return NULL;
	pthread_mutex_lock(&cqi_freelist_lock);
	for (i = 1; i < ITEMS_PER_ALLOC; i++) {
		cqi_freelist.push_back(&items[i]);
	}
	pthread_mutex_unlock(&cqi_freelist_lock);
	fprintf(stderr, "freelist size=%lu\n", cqi_freelist.size());
	return &items[0];
}

static void cqi_free(CQ_ITEM *item) {
	pthread_mutex_lock(&cqi_freelist_lock);
	cqi_freelist.push_back(item);
	pthread_mutex_unlock(&cqi_freelist_lock);
}

static void cq_push(LIBEVENT_THREAD *t, CQ_ITEM *item) {
	fprintf(stderr, "dispatch one conn...\n");
	pthread_mutex_lock(&t->cq_lock);
	t->new_conn_q->push_back(item);
	pthread_mutex_unlock(&t->cq_lock);
	CHECK(t->new_conn_q->size() > 0);
}

static CQ_ITEM *cq_pop(LIBEVENT_THREAD *t) {
	CQ_ITEM *ret;
	//fprintf(stderr, "deque to conn q\n");
	pthread_mutex_lock(&t->cq_lock);
	if (!t->new_conn_q->empty()) {
		ret = t->new_conn_q->front();
		t->new_conn_q->pop_front();
	}
	pthread_mutex_unlock(&t->cq_lock);
	return ret;
}

static int last_thread = -1;

static void dispatch_conn_new(int sfd, conn_t type, int event_flags) {
	CQ_ITEM *item = cqi_new();
	if (item == NULL) {
		close(sfd);
		fprintf(stderr, "Failed to allocate memory for connection object\n");
		return;
	}
	int tid = (last_thread + 1) % settings.num_threads;
	LIBEVENT_THREAD *thread = threads + tid;
	last_thread = tid;

	item->sfd = sfd;
	item->type = type;
	item->event_flags = event_flags;
	fprintf(stderr, "dispatch sfd=%d\n", sfd);

	cq_push(thread, item);
	char buf[1];
	if (write(thread->notify_send_fd, buf, 1) != 1) {
		perror("Writing to thread notify pipe");
	}
}

static int try_read_network(Link *c) {
	int res = c->msg_read();
	//fprintf(stderr, "try_read_network:bytes=%d\n", res);
	return res;
}

static bool update_event(Link *c, const int new_flags) {
	CHECK(c != NULL);
	struct event_base *base = c->event.ev_base;
	if (c->get_ev_flags() == new_flags)	return true;
	if (event_del(&c->event) == -1)	return false;
	event_set(&c->event, c->get_sfd(), new_flags, event_handler, (void *)c);
	event_base_set(base, &c->event);
	c->set_ev_flags(new_flags);
	if (event_add(&c->event, 0) == -1)	return false;
	return true;
}

static void compose_reply(Link *c, protoSpec::Reply &msg, const std::string &t) {
	std::string data = msg.SerializeAsString();

	protoSpec::Request req;
	t = req.GetTypeName();
	//std::string type = this->write_buf.GetTypeName();
	//std::string type = getOpType(this->write_buf);
	size_t dataLen = data.length();
	size_t typeLen = t.length();
	size_t totalLen = (typeLen + sizeof(typeLen) +
			               dataLen + sizeof(dataLen) +
			               sizeof(totalLen) + 
										 sizeof(uint32_t));

	int old_sz = c->output->size(); // yue: remember the existing size
	//*((uint32_t *)ptr) = MAGIC;
	//ptr += sizeof(uint32_t);
	c->output->append(MAGIC);
	assert(c->output->size()-old_sz < (int)totalLen);

	//*((size_t *)ptr) = totalLen;
	//ptr += sizeof(size_t);
	c->output->append(totalLen);
	assert(c->output->size()-old_sz < (int)totalLen);

	//*((size_t *)ptr) = typeLen;
	//ptr += sizeof(size_t);
	c->output->append(typeLen);
	assert(c->output->size()-old_sz < (int)totalLen);
	//memcpy(ptr, type.c_str(), typeLen);
	//ptr += typeLen;
	//fprintf(stderr, "compose_reply for %s\n", t.c_str());
	c->output->append(t.c_str(), (int)typeLen);
	assert(c->output->size()-old_sz < (int)totalLen);

	//*((size_t *)ptr) = dataLen;
	//ptr += sizeof(size_t);
	c->output->append(dataLen);
	assert(c->output->size()-old_sz < (int)totalLen);
	//memcpy(ptr, data.c_str(), dataLen);
	//ptr += dataLen;
	c->output->append(data.c_str(), dataLen);
	assert(c->output->size()-old_sz == (int)totalLen);

	//fprintf(stderr, "composed reply size=%lu\n", totalLen);
}

static void handle_request(Link *c) {
	static protoSpec::GetMessage get;
	static protoSpec::PutMessage put;
	static protoSpec::DelMessage del;
	
	std::vector<Bytes> msg = c->get_read_buf();
	CHECK(msg.size() == 2);
	protoSpec::Request req;
	std::string t = msg[0].String();
	std::string payload = msg[1].String();
	req.ParseFromString(payload);
	protoSpec::Reply reply;
	if (get.GetTypeName() == t) {  // yue: serve GET
		//fprintf(stderr, "process GET...\n");
		std::string key = req.mutable_get()->key();
#ifdef USE_STL_MAP
		if (hashmap->find(key) != hashmap->end()) {
	  	std::string	value = hashmap->at(key);
			reply.set_status(0);
			reply.set_value(value);
		} else {
			//reply.set_value(key);
			reply.mutable_value()->append("Key not found");
			reply.set_status(0);
		}
#endif

	} else if (put.GetTypeName() == t) {  // yue: serve PUT
		//fprintf(stderr, "%d: process PUT: key=%s, val=%s\n", 
		//		settings.port, req.mutable_put()->key().c_str(), req.mutable_put()->value().c_str());
		//fprintf(stderr, "put: key=%s, val=%s\n", 
		//		req.mutable_put()->key().c_str(), 
		//		req.mutable_put()->value().c_str());
#ifdef USE_STL_MAP
		auto item = hashmap->find(req.mutable_put()->key());
		if (item != hashmap->end()) { // yue: if kv pair exists
			item->second = req.mutable_put()->value();
			reply.set_value("Found the key");
			reply.mutable_value()->append(item->first);
		} else { // yue: if kv pair does not exist
			reply.set_value("Added new key");
			//reply.mutable_value()->append(item->first);
			hashmap->insert(std::make_pair(req.mutable_put()->key(), req.mutable_put()->value()));
		}
		reply.set_status(0);
#endif

	} else if (del.GetTypeName() == t) {  // yue: serve DEL

		fprintf(stderr, "process DEL...\n");
		/* yue: FIXME: dummy reply for testing... */
		reply.set_status(0);

	}
	compose_reply(c, reply, t);
	int send_status = c->msg_write();
	CHECK(send_status > 0);
}

static void conn_close(Link *c) {
	c->close();
	delete c;
}

/**
 * yue: thread routine;
 * dispatcher calls dispatch_conn_new();
 * worker goes to the other branch.
 */
static void process(Link *c) {
	if (c->get_type() == conn_listening) {
		int sfd = c->accept();
		if (sfd == -1) {
			fprintf(stderr, "Failed to accept\n");
			return;
		}
		dispatch_conn_new(sfd, conn_slave, EV_READ|EV_PERSIST);
	} else {
		int len = try_read_network(c);
		if (len <= 0) {
			/* 
			 * yue: FIXME: we mark it as failed and should destroy this link 	
			 */
			//c->mark_error();
			fprintf(stderr, "try_read_network failed: %s\n", strerror(errno));
			conn_close(c);
			return;
		}

		/*
		 * yue: TODO: we should add a feature here to correctly handle batch requests
		 *
		 * Logic can be: a while loop looping around the read bytes at
		 * per request granularity until the read bytes are fully consumed...
		 */
		const std::vector<Bytes> *req = c->msg_parse();
		if (req == NULL) {
			fprintf(stderr, "Failed to parse message: sfd: %d\n", c->get_sfd());
			/* yue: TODO: add logic to handle exception HERE */
		}

		handle_request(c);
	}
}

void event_handler(const int fd, const short which, void *arg) {
	Link *c = (Link *)arg;
	CHECK(c != NULL);
	c->which = which;
	if (fd != c->get_sfd()) {
		fprintf(stderr, "Catastrophic: event fd doesn't match conn fd!\n");
		c->close();
		delete c;
		return;
	}
	//static uint32_t _counter = 0;
	//fprintf(stderr, "event_handler: %u\n", _counter++);
	process(c);
	return;
}

static Link *register_conn(int sfd, conn_t type, const int event_flags, 
		struct event_base *base) {
	Link *c = new Link();
	if (c == NULL) {
		fprintf(stderr, "Failed to init listening conn\n");
		exit(EXIT_FAILURE);
	}
	c->set_sfd(sfd);
	c->set_type(type);
	event_set(&c->event, sfd, event_flags, event_handler, (void *)c);
	event_base_set(base, &c->event);
	c->set_ev_flags(event_flags);
	if (event_add(&c->event, 0) == -1) {
		perror("event_add");
		return NULL;
	}
	return c;
}

static int server_socket(int port) {
	int sfd, error;
	int flags = 1;
	int success = 0;
	struct linger ling = {0, 0};
	struct addrinfo *ai;
	struct addrinfo *next;
	struct addrinfo hints = { .ai_flags = AI_PASSIVE, .ai_family = AF_UNSPEC };
	char port_buf[NI_MAXSERV];
	//hints.ai_socktype = IS_UDP(transport) ? SOCK_DGRAM : SOCK_STREAM;
	hints.ai_socktype = SOCK_STREAM;
	snprintf(port_buf, sizeof(port_buf), "%d", port);
	error = getaddrinfo(settings.inter, port_buf, &hints, &ai);
	if (error != 0) {
		if (error != EAI_SYSTEM) {
			fprintf(stderr, "getaddrinfo(): %s\n", gai_strerror(error));
		} else {
			perror("getaddrinfo()");
		}
		return 1;
	}
	for (next = ai; next; next = next->ai_next) {
		Link *listen_conn;
		if ((sfd = new_socket(next)) == -1) {
			if (errno == EMFILE) {
				perror("server_socket");
				exit(EX_OSERR);
			}
			continue;
		}
		setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, (void *)&flags, sizeof(flags));
		error = setsockopt(sfd, SOL_SOCKET, SO_KEEPALIVE, (void *)&flags, sizeof(flags));
		if (error != 0)
			perror("setsockopt trial 1");
		error = setsockopt(sfd, SOL_SOCKET, SO_LINGER, (void *)&ling, sizeof(ling));
		if (error != 0)
			perror("setsockopt trial 2");
		//error = setsockopt(sfd, SOL_SOCKET, TCP_NODELAY, (void *)&flags, sizeof(flags));
		error = setsockopt(sfd, IPPROTO_TCP, TCP_NODELAY, (void *)&flags, sizeof(flags));
		if (error != 0)
			perror("setsockopt trial 3");
		if (bind(sfd, next->ai_addr, next->ai_addrlen) == -1) {
			if (errno != EADDRINUSE) {
				perror("bind()");
				close(sfd);
				freeaddrinfo(ai);
				return 1;
			}
		} else {
			success++;
			if (listen(sfd, 1024) == -1) {
				perror("listen()");
				close(sfd);
				freeaddrinfo(ai);
				return 1;
			}
#ifdef PORTFILE
			if (next->ai_addr->sa_family == AF_INET || 
					next->ai_addr->sa_family == AF_INET6) {
				union {
					struct sockaddr_in in;
					struct sockaddr_in6 in6;
				} my_sockaddr;
				socklen_t len = sizeof(my_sockaddr);
				if (getsockname(sfd, (struct sockaddr *)&my_sockaddr, &len)==0) {
					if (next->ai_addr->sa_family == AF_INET) {
						fprintf(stderr, "TCP INET: %u\n", ntohs(my_sockaddr.in.sin_port));
					} else {
						fprintf(stderr, "TCP INET6: %u\n", ntohs(my_sockaddr.in6.sin6_port));
					}
				}
			}
#endif
		}
		fprintf(stderr, "parent thread sfd=%d\n", sfd);
		if (!(listen_conn = register_conn(sfd, conn_listening, EV_READ|EV_PERSIST, main_base))) {
			fprintf(stderr, "Failed to create listening connection\n");
			exit(EXIT_FAILURE);
		}
	}
	freeaddrinfo(ai);
	return success==0;
}

static void usage() {
	printf("Simple kvstore datalet: \n"
			   "  -m <num>  max memory to use for items in megabytes (default: 64 MB)\n"
			   "  -l <addr> interface to listen on\n"
				 "  -p <num>  TCP port number to listen on\n"
				 "  -t <num>  number of threads to use\n"
				 "  -h        print usage info\n"
				 );
}

int main(int argc, char **argv) {
	int c;
	int ret = 0;
	bool preallocate = false;

	settings_init();

	while (-1 != (c = getopt(argc, argv,
					"m:"  /* yue: max memory to use for items in megabytes */
					"l:"  /* yue: interface to liston on */
					"p:"  /* yue: TCP port number to listen on */
					"t:"  /* yue: # threads */
					"h"   /* yue: help info */
					))) {
		switch(c) {
		case 'm':
			settings.maxbytes = ((size_t)atoi(optarg)) * 1024 * 1024;
			break;
		case 'l':
			if (settings.inter != NULL) {
				if (strstr(settings.inter, optarg) != NULL) {
					break;
				}
				size_t len = strlen(settings.inter) + strlen(optarg) + 2;
				char *p = (char *)malloc(len);
				if (p == NULL) {
					fprintf(stderr, "Failed to allocat mem\n");
					return 1;
				}
				snprintf(p, len, "%s,%s", settings.inter, optarg);
				free(settings.inter);
				settings.inter = p;
			} else {
				settings.inter = strdup(optarg);
			}
			break;
		case 'p':
			settings.port = atoi(optarg);
			break;
		case 't':
			settings.num_threads = atoi(optarg);
			if (settings.num_threads <= 0) {
				fprintf(stderr, "Number of threads must be greater than 0\n");
				return 1;
			}
			break;
		case 'h':
			usage();
			exit(EXIT_SUCCESS);
		}
	}


	//Thread that initializes RDMA connection with server
	pthread_t rdmathread;

	//Might not actually need to be a thread, could just call rdmaserver
	pthread_create(&rdmathread, NULL, rdmaserver, NULL);



	main_base = event_init();
	ht_init(settings.hashpower_init);
	if (settings.use_slabs)
		slabs_init(settings.maxbytes, settings.factor, preallocate);
	kvs_thread_init(settings.num_threads);
	if (settings.port && server_socket(settings.port)) {
		fprintf(stderr, "Port %d: ", settings.port);
		perror("Failed to listen on TCP port");
		exit(EX_OSERR);
	}
	//return 0;
	/* yue: one moment please */
	usleep(1000);
	/* yue: enter the event loop */
	if (event_base_loop(main_base, 0x04) != 0) {
		ret = EXIT_FAILURE;
	}
	if (settings.inter)	free(settings.inter);
	return ret;
}