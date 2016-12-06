#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <getopt.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ev.h>
#include <sys/un.h>
#include <errno.h>
#include <sys/ioctl.h>

#include <sys/fcntl.h>

#include "messages.h"
#include "utils.h"


// The struct we are using to store the results
// What will be written from the child processes to the parent
struct benchmark_results {
	struct timespec tstart;
	struct timespec tend;
	uint32_t messages_sent;
};


// TODO
// disconnect and reconnect on each newline

// Nasty globals for now
// feel free to fork this example and clean it up
EV_P;
ev_io file_watcher;
ev_io remote_w;
ev_io send_w;
int remote_fd;
FILE *file_in;
char* line = NULL;
size_t len = 0;
int counter = 0;
int total;
int lines_written = 0;
int lines_read_from_file = 0;

void usage(char *name)
{
	printf("%s -i <ip_addr> -p <port> -c <num_of_connections> -m <msg_num> -n <req_num> -d <milliseconds> -v -h\n", name);
	printf("Example: %s -i 0.0.0.0 -p 12345 -c 12 -m 100 -d 0 -n 100000\n", name);
}

void help(char * name)
{
	usage(name);
	printf("\n-i: The IP address of the server you are connecting to\n");
	printf("-p: The port of the server you are connecting to\n");
	printf("-c(default=1): The number of socket connections you want to send messages from\n");
	printf("-m(default=1): The number of messages that are sent per request\n");
	printf("-d(default=0): Time to delay in milliseconds after each request\n");
	printf("-n: The number of requests you want to send per socket in total\n");
	printf("-t(default=test): The name of the test files to be read (If -t test is used then it will read from test0.txt, test1.txt, etc. depending on the value of the -c argument");
	printf("-v: Run the program in verbose mode\n");
	printf("-h: Displays this message\n");

}


//Parses a line from a file to create a send_message struct
struct send_message *parseLine(std::string line, struct send_message *request)
{
	//The opcode is now a number that is the first character of each line (0-4)
	std::string opstr = "";
	opstr.push_back(line[0]);
	request->operation = stoi(opstr);
	request->keepalive = 1;

	int index = 2;
	int keyindex = 0;
	int valueindex = 0;

	//Fills in the send message key char array with the chars from the file
	//The : acts as a seperator between the operator, key, and value
	while (line[index] != ':' and keyindex < 16) {
		request->key[keyindex] = line[index];
		index++;
		keyindex++;
	}
	//Fills all remaining spaces in the array with '\0'
	while (keyindex < 16) {
		request->key[keyindex] = '\0';
		keyindex++;
	}

	index++;
	//Fills in the send message value char array with the chars from the file
	while (line[index] != ':' and valueindex < 32) {
		request->value[valueindex] = line[index];
		index++;
		valueindex++;
	}
	//Fills all remaining spaces in the array with '\0'
	while (valueindex < 32) {
		request->value[valueindex] = '\0';
		valueindex++;
	}
	return request;
}

static void filein_cb(EV_P_ ev_io *w, int revents)
{
	//puts("stdin written to, reading...");
	int ret = getline(&line, &len, file_in);

	if (ret == -1) {
		ev_io_stop(EV_A_ & file_watcher);
		ev_io_stop(EV_A_ & send_w);
		ev_io_set(&send_w, remote_fd,  EV_WRITE);
		ev_io_start(EV_A_ & send_w);
		return;
	}
	lines_read_from_file++;
	//printf("%s\n", line);
	ev_io_stop(EV_A_ & file_watcher);
	ev_io_stop(EV_A_ & send_w);
	ev_io_set(&send_w, remote_fd, EV_READ | EV_WRITE);
	ev_io_start(EV_A_ & send_w);
}

int total_bytes_written = 0;

static void send_cb(EV_P_ ev_io *w, int revents)
{
	if (revents & EV_WRITE) {

		if (lines_written >= total) {
			ev_io_stop(EV_A_ & send_w);
			ev_io_set(&send_w, remote_fd, EV_READ);
			ev_io_start(EV_A_ & send_w);
		}

		struct send_message *msg = (struct send_message *)malloc(sizeof(struct send_message));
		parseLine(std::string(line), msg);
		//puts("remote ready for writing...");
		// for (int i = 0; i < 32; i++)
		//      msg->value[i] = 0x00;
		// for (int i = 0; i < 16; i++)
		//      msg->key[i] = 0xFF;
		if (lines_written == total - 1)
			msg->keepalive = 0;

		int ret = send(remote_fd, msg, sizeof(struct send_message), 0);
		total_bytes_written += ret;
		printf("total bytes written %d\n", total_bytes_written);
		//assert(ret == 50);
		lines_written++;
		//printf("send returned %d\n", ret);
		//print_send_message(msg);
		//free(msg);
		if (-1 == ret) {
			perror("echo send");
			exit(EXIT_FAILURE);
		}
		//printf("counter = %d\n", counter);
		//printf("total = %d\n", total);
		// once the data is sent, stop notifications that
		// data can be sent until there is actually more
		// data to send
		uint32_t amount_we_can_read;
		ioctl(remote_fd, FIONREAD, &amount_we_can_read);
		//printf("n = %d\n", n);
		if (lines_written % 995 == 0) {
			ev_io_stop(EV_A_ & send_w);
			ev_io_set(&send_w, remote_fd, EV_READ);
			ev_io_start(EV_A_ & send_w);
			//printf("READING\n");
		} else{
			ev_io_stop(EV_A_ & send_w);
			ev_io_set(&send_w, remote_fd, EV_READ | EV_WRITE);
			ev_io_start(EV_A_ & send_w);
		}
	}else if (revents & EV_READ) {
		char str[100];

		int to_read;
		int bytes_read;
		//printf("[r,remote]");
		//printf("WE ARE HERE\n" );
		while (recv(remote_fd, str, 50, 0) == 50) {
			printf("counter = %d\n", counter);
			counter++;
			//print_response_message((struct response_message *)str);
			if (counter >= total) {
				close(remote_fd);
				//printf("DONE!\n");
				ev_io_stop(EV_A_ & send_w);
				ev_io_stop(EV_A_ & remote_w);
				ev_io_stop(EV_A_ & file_watcher);
				return;
			}
		}
		if (lines_read_from_file != total) {
			ev_io_stop(EV_A_ & file_watcher);
			ev_io_set(&file_watcher, fileno(file_in), EV_READ);
			ev_io_start(EV_A_ & file_watcher);
		}else {
			ev_io_stop(EV_A_ & send_w);
			ev_io_set(&send_w, remote_fd, EV_READ);
			ev_io_start(EV_A_ & send_w);
		}

	}
}


static void remote_cb(EV_P_ ev_io *w, int revents)
{
	puts("connected, now watching stdin");
	// Once the connection is established, listen to stdin
	ev_io_start(EV_A_ & file_watcher);
	// Once we're connected, that 's the end of that
	ev_io_stop(EV_A_ & remote_w);
}


// Simply adds O_NONBLOCK to the file descriptor of choice
int setnonblock(int fd)
{
	int flags;

	flags = fcntl(fd, F_GETFL);
	flags |= O_NONBLOCK;
	return fcntl(fd, F_SETFL, flags);
}

static void connection_new(EV_P_ uint32_t ip_addr, uint16_t port)
{
	int socket_desc;
	struct sockaddr_in server;

	//Create socket
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1) {
		printf("Could not create socket\n");
		exit(-1);
	}

	server.sin_addr.s_addr = ip_addr;
	server.sin_family = AF_INET;
	server.sin_port = port;

	//Connect to remote server
	if (connect(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0) {
		printf("Connect error\n");
		exit(-1);
	}
	setnonblock(socket_desc);

	// this should be initialized before the connect() so
	// that no packets are dropped when initially sent?
	// http://cr.yp.to/docs/connect.html

	// initialize the connect callback so that it starts the stdin asap
	remote_fd = socket_desc;
	ev_io_init(&remote_w, remote_cb, socket_desc, EV_WRITE);
	ev_io_start(EV_A_ & remote_w);
	// initialize the send callback, but wait to start until there is data to write
	ev_io_init(&send_w, send_cb, socket_desc, EV_READ);
	ev_io_start(EV_A_ & send_w);

}

int garbage_init(uint32_t ip_addr, uint16_t port, uint16_t num_of_connections,
		 uint16_t msgs_per_request, uint32_t num_of_msgs_per_connection,
		 uint32_t delay, std::string filename)
{
	loop = EV_DEFAULT;
	// initialise an io watcher, then start it
	// this one will watch for stdin to become readable
	file_in = fopen(filename.c_str(), "r");
	if (file_in == NULL) {
		printf("Error opening file!\n");
		exit(EXIT_FAILURE);
	}
	setnonblock(fileno(file_in));

	ev_io_init(&file_watcher, filein_cb, fileno(file_in), EV_READ);

	connection_new(EV_A_ ip_addr, port);

	// now wait for events to arrive
	struct benchmark_results br;
	clock_gettime(CLOCK_MONOTONIC, &br.tstart);
	ev_loop(EV_A_ 0);
	clock_gettime(CLOCK_MONOTONIC, &br.tend);
	struct timespec latest_end =  br.tend;
	struct timespec earliest_start = br.tstart;
	double total_time;
	total_time = ((double)latest_end.tv_sec + 1.0e-9 * latest_end.tv_nsec) -
		     ((double)earliest_start.tv_sec + 1.0e-9 * earliest_start.tv_nsec);

	printf("\n\nTotal Time Elapsped: %lf \n", total_time);
	printf("Total Messages Sent: %d\n", total);
	printf("Messages per Second: %lf\n", total / total_time);

	// unloop was called, so exit
	return 0;
}


// Sets up all the arguments that were passed to the program
// and then starts the benchmark
int main(int argc, char *argv[])
{
	srand(time(NULL));
	int option;
	uint16_t port;
	uint32_t ip_addr;
	uint16_t num_of_connections = 1;
	uint16_t msgs_per_request = 1;
	uint32_t num_of_msgs_per_connection = 1024;
	uint8_t req_args = 0b11;
	uint32_t delay = 0;
	std::string filename = "test";
	while ((option = getopt(argc, argv, "vhi:p:c:m:d:n:t:")) != -1) {
		switch (option) {
		case 'i':
			ip_addr = inet_addr(optarg);
			req_args &= 0b10;
			break;
		case 'p':
			port = htons(atoi(optarg));
			req_args &= 0b01;
			break;
		case 'c':
			num_of_connections = atoi(optarg);
			break;
		case 'm':
			msgs_per_request = atoi(optarg);
			break;
		case 'n':
			num_of_msgs_per_connection = atoi(optarg);
			total = num_of_msgs_per_connection;
			break;
		case 'd':
			delay = atoi(optarg);
			break;
		case 't':
			filename = std::string(optarg);
			break;
		case 'v':
			//g_verbose = true;
			break;
		case 'h':
			help(argv[0]);
			exit(EXIT_FAILURE);
			break;
		default:
			help(argv[0]);
			exit(EXIT_FAILURE);
		}
	}
	if (req_args != 0) {
		help(argv[0]);
		exit(EXIT_FAILURE);
	}
	garbage_init(ip_addr, port, num_of_connections, msgs_per_request,
		     num_of_msgs_per_connection, delay, filename);
	return 0;
}
