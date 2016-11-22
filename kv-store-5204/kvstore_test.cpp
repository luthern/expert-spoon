#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>

#include "cuckoo_kvstore.h"
#include "ckv_proto.pb.h"
#include <google/protobuf/io/zero_copy_stream_impl.h>

// Does all the tcp socket initialization that must be done
// and returns the fd
int init_tcp_connection(uint32_t ip_addr, uint16_t port)
{
	int socket_desc;
	struct sockaddr_in server;

	//Create socket
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1) {
		printf("Failed to create socket\n");
		exit(-1);
	}

	server.sin_addr.s_addr = ip_addr;
	server.sin_family = AF_INET;
	server.sin_port = port;

	//Connect to remote server
	if (connect(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0) {
		printf("Failed to connect to server\n");
		exit(-1);
	}
	return socket_desc;
}

int main(int argc, char const *argv[])
{
	protoSpec::Request req;
	protoSpec::Reply reply;

	protoSpec::GetMessage *get = req.mutable_get();;
	req.set_op(protoSpec::Request_Operation_GET);
	get->set_key("TES1TES1TES1TES1");
	int s = init_tcp_connection(inet_addr("127.0.0.1"), htons(atoi("11211")));
	google::protobuf::io::FileInputStream *s_zero_copy = new google::protobuf::io::FileInputStream(s);
	printf("STARTING\n");
	req.SerializeToFileDescriptor(s);
	uint32_t len;
	int err = recv(s, &len, 4, 0);
	printf("err = %d\n", err);
	printf("len= %d\n", len);
	reply.ParseFromBoundedZeroCopyStream(s_zero_copy, len);
	printf("status = %d\n", reply.status());

}
