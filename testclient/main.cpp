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
#include "messages.h"
#include "utils.h"

bool g_verbose = false;

// The struct we are using to store the results
// What will be written from the child processes to the parent
struct benchmark_results {
    struct timespec tstart;
    struct timespec tend;
    uint32_t messages_sent;
};


int init_connection(uint32_t ip_addr, uint16_t port){
    int socket_desc;
    struct sockaddr_in server;

    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not create socket\n");
        exit(-1);
    }

    server.sin_addr.s_addr = ip_addr;
    server.sin_family = AF_INET;
    server.sin_port = port;

    //Connect to remote server
    if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0){
        printf("Connect error\n");
        exit(-1);
    }
    return socket_desc;
}

//Parses a line from a file to create a send_message struct
struct send_message parseLine(std::string line){
	struct send_message request;
	char op = line[0];
	if(op=='r')
	{
		request.operation = 0;
	}
	if(op=='w')
	{
		request.operation = 1;
	}
	int index = 2;
	int keyindex = 0;
	int valueindex = 0;
	while(line[index]!=':' and keyindex<16)
	{
		request.key[keyindex] = line[index];
		index++;
		keyindex++;
	}
	while(keyindex<16)
	{
		request.key[keyindex] = '\0';
		keyindex++;
	}

	index++;
	while(line[index]!=':' and valueindex<32)
	{
		request.value[valueindex] = line[index];
		index++;
		valueindex++;
	}
	while(valueindex<32)
	{
		request.value[valueindex] = '\0';
		valueindex++;
	}
	return request;
}


void usage(char *name){
    printf("%s -i <ip_addr> -p <port> -c <num_of_connections> -m <msg_num> -n <req_num> -d <milliseconds> -v -h\n",name);
    printf("Example: %s -i 0.0.0.0 -p 12345 -c 12 -m 100 -d 0 -n 100000\n", name);
}

void help(char * name){
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

// The actual sending messages part of the benchmark
// We generate random keys using rand() and then send send_message structs to the server
// We then recv the response_messages back
// which currently will have values that are just key repeated
uint32_t send_tcp_requests(int s, uint16_t msgs_per_request,
                        uint32_t num_of_msgs_per_connection, uint32_t delay, std::string filename){
    uint32_t msgs_sent = 0;
    while(msgs_sent < num_of_msgs_per_connection)
    {
        struct send_message msgs[msgs_per_request];
        struct response_message resp_msgs[msgs_per_request];
        for(int i = 0; i < msgs_per_request; i++){
            for(int j = 0; j < 16; j++)
                msgs[i].key[j] = rand() % 256;
        }
        send(s, msgs, sizeof(struct send_message) * msgs_per_request, 0);
        recv(s, resp_msgs, msgs_per_request * sizeof(struct response_message), 0);
        if(g_verbose){
            for(int i = 0; i < msgs_per_request; i++){
                print_response_message(&resp_msgs[i]);
            }
        }
        msgs_sent += msgs_per_request;
        if(delay)
            usleep(delay * 1000);
    }
    return msgs_sent;
}

// Sends UDP request to the server
// The actual sending messages part of the benchmark
// We generate random keys using rand() and then send send_message structs to the server
// We then recv the response_messages back
// which currently will have values that are just key repeated
uint32_t send_udp_requests(int s,
			uint16_t msgs_per_request, uint32_t num_of_msgs_per_connection,
			uint32_t delay,	struct sockaddr_in* server_ptr, std::string filename){

    uint32_t msgs_sent = 0;
    uint32_t server_len = sizeof (*server_ptr);
    std::ifstream ofs;
    ofs.open(filename);
    std::string line;
    getline(ofs, line);
    int length = stoi(line);
    int linecount = 0;
    while(msgs_sent < num_of_msgs_per_connection)
    {
        struct send_message msgs[msgs_per_request];
        struct response_message resp_msgs[msgs_per_request];
	for(int i = 0; i < msgs_per_request; i++){
		if(linecount>=length)
		{
			ofs.close();
			ofs.open(filename);
			getline(ofs, line);
			linecount = 0;
		}
		getline(ofs, line);
		msgs[i] = parseLine(line);
		linecount++;
		for(int q = 0; q<16; q++){
			//std::cout << msgs[i].key[q];
		}
		//std::cout << "\n";
	}

        sendto(s,
        		msgs,
        		sizeof(struct send_message) * msgs_per_request,
				0,
				(struct sockaddr *) server_ptr,
				server_len);
	

        //TODO: currently not distinguishing the server from which the response is received
        recvfrom(s,
        		resp_msgs,
				msgs_per_request * sizeof(struct response_message),
				0,
				(struct sockaddr *)server_ptr,
				&server_len);
	//printf("Received\n");

        if(g_verbose){
            for(int i = 0; i < msgs_per_request; i++){
                print_response_message(&resp_msgs[i]);
            }
        }
        msgs_sent += msgs_per_request;
        if(delay)
            usleep(delay * 1000);
    }
    return msgs_sent;
}



// The actual benchmark
// Basically we fork() for every connection, so each socket will have its own process
// Then each child does the actual sending and recieving of messages to the server
// and the result are reported back to the parent process using pipes
// The parent can then print the results out, etc
void run_benchmark(uint32_t ip_addr, uint16_t port, uint16_t num_of_connections,
                    uint16_t msgs_per_request, uint32_t num_of_msgs_per_connection,
                    uint32_t delay, std::string filename){
    bool is_child = false;
    int read_fds[num_of_connections];
    int write_fd;
    int requestindex = -1;
    for(uint16_t i = 0; i < num_of_connections; i++){
        int fd[2];
        pipe(fd);
	requestindex++;
        pid_t pid = fork();
        if(pid == 0){
            close(fd[0]);
            write_fd = fd[1];
            is_child = true;
            break;
        }
        else{
            close(fd[1]);
            read_fds[i] = fd[0];
        }
    }
    if(is_child){
        //int s = init_udp_connection(ip_addr, port);
	int s = init_connection(ip_addr, port);
        struct benchmark_results br;
        clock_gettime(CLOCK_MONOTONIC, &br.tstart);

        //br.messages_sent = send_tcp_requests(s,msgs_per_request, num_of_msgs_per_connection ,delay);
        struct sockaddr_in server;
        server.sin_addr.s_addr = ip_addr;
        server.sin_family = AF_INET;
        server.sin_port = port;
	std::stringstream stream2;
	stream2 << requestindex;
	std::string filetitle = filename + stream2.str() + ".txt";
        br.messages_sent = send_udp_requests(s,msgs_per_request, num_of_msgs_per_connection ,delay, &server, filetitle);
        clock_gettime(CLOCK_MONOTONIC, &br.tend);
        write(write_fd, &br, sizeof(struct benchmark_results));
        close(s);
    }
    else{
        struct benchmark_results br;
        struct timespec earliest_start = {0,0};
        struct timespec latest_end = {0,0};
        uint64_t total_messages_sent = 0;
        for(uint16_t i = 0; i < num_of_connections; i++){
            read(read_fds[i], &br, sizeof(struct benchmark_results));
            if(!earliest_start.tv_sec && !latest_end.tv_sec){
                copy_timespec(&earliest_start, &br.tstart);
                copy_timespec(&latest_end, &br.tend);
            }
            else{
                copy_timespec(&earliest_start, min_timespec(&br.tstart, &earliest_start));
                copy_timespec(&latest_end, max_timespec(&br.tend, &latest_end));
            }
            total_messages_sent += br.messages_sent;
            double diff_t;
            diff_t = ((double)br.tend.tv_sec + 1.0e-9*br.tend.tv_nsec) -
               ((double)br.tstart.tv_sec + 1.0e-9*br.tstart.tv_nsec);
            printf("Sent messages in: %lf seconds\n", diff_t);
        }
        double total_t;
        total_t = ((double)latest_end.tv_sec + 1.0e-9*latest_end.tv_nsec) -
           ((double)earliest_start.tv_sec + 1.0e-9*earliest_start.tv_nsec);

        printf("\n\nTotal Time Elapsped: %lf \n", total_t);
        printf("Total Messages Sent: %lu\n", total_messages_sent);
        printf("Messages per Second: %lf\n",total_messages_sent / total_t);

    }

}

// Sets up all the arguments that were passed to the program
// and then starts the benchmark
int main(int argc, char *argv[]) {
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
    while ((option = getopt(argc, argv,"vhi:p:c:m:d:n:t:")) != -1) {
        switch (option) {
             case 'i' :
                 ip_addr = inet_addr(optarg);
                 req_args &= 0b10;
                 break;
            case 'p' :
                 port = htons(atoi(optarg));
                 req_args &= 0b01;
                 break;
            case 'c' :
                 num_of_connections = atoi(optarg);
                 break;
            case 'm':
                msgs_per_request = atoi(optarg);
                break;
            case 'n':
                num_of_msgs_per_connection = atoi(optarg);
                break;
            case 'd':
                delay = atoi(optarg);
                break;
            case 't':
                filename = std::string(optarg);
                break;
            case 'v':
                g_verbose = true;
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
    run_benchmark(ip_addr, port, num_of_connections, msgs_per_request,
                    num_of_msgs_per_connection, delay, filename);
    return 0;
}