#include <stdio.h>
#include <stdint.h>
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <time.h>

struct send_message {
    uint8_t operation;
    unsigned char key[16];
    unsigned char value[32];
};

struct response_message {
    uint8_t status_code;
    unsigned char key[16];
    unsigned char value[32];
};

void usage(char *name){
    printf("%s -i <ip_addr> -p <port> -c <num_of_connections> -m <msg_num> -n <req_num> -d <milliseconds> -h\n",name);
}

void help(char * name){
    usage(name);
    printf("-i: The IP address of the server you are connecting to\n");
    printf("-p: The port of the server you are connecting to\n");
    printf("-c(default=1): The number of socket connections you want to send messages from\n");
    printf("-m(default=1): The number of messages that are sent per request\n");
    printf("-d(default=0): Time to delay in milliseconds after each request\n");
    printf("-n(NOT IMPLEMENTED): The number of requests you want to send per socket in total\n");
    printf("-h: Displays this message\n");


}

void print_response_message(struct response_message * resp){
    char key[16 * 2 + 1];
    char value[32 * 2 + 1];

    char *key_ptr = key;
    for(int i = 0; i < 16; i++){
        key_ptr += sprintf(key_ptr, "%02X", resp->key[i]);
    }

    char *val_ptr = value;
    for(int i = 0; i < 32; i++){
        val_ptr += sprintf(val_ptr, "%02X", resp->value[i]);
    }
    printf("status_code: 0x%X\nkey: %s\nvalue: %s\n", resp->status_code, resp->key, resp->value);
}

void print_send_message(struct send_message * send){
    char key[16 * 2 + 1];
    char *key_ptr = key;

    for(int i = 0; i < 16; i++){
        key_ptr += sprintf(key_ptr, "%02X", send->key[i]);
    }
    printf("operation: 0x%X\nkey: %s\n", send->operation, key);

}


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

void benchmark_loop(int s, uint16_t msgs_per_request, uint32_t delay){
    while(true)
    {
        struct send_message msgs[msgs_per_request];
        struct response_message resp_msgs[msgs_per_request];
	char *alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJLKMNOPQRSTUVWXYZ";
        for(int i = 0; i < msgs_per_request; i++){
		msgs[i].operation = rand() % 2;
            for(int j = 0; j < 16; j++)
                msgs[i].key[j] = alphabet[rand() % 52];
            for(int j = 0; j < 32; j++)
                msgs[i].value[j] = alphabet[rand() % 52];
        }
        send(s, msgs, sizeof(struct send_message) * msgs_per_request, 0);
        recv(s, resp_msgs, msgs_per_request * sizeof(struct response_message), 0);
      
	for(int i = 0; i < msgs_per_request; i++){
            print_response_message(&resp_msgs[i]);
        }
        usleep(delay * 1000);
    }
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    int option;
    uint16_t port;
    uint32_t ip_addr;
    uint16_t num_of_connections = 1;
    uint16_t msgs_per_request = 1;
    uint8_t req_args = 0b11;
    uint32_t delay = 0;
    while ((option = getopt(argc, argv,"hi:p:c:m:d:")) != -1) {
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
            case 'd':
                delay = atoi(optarg);
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
    bool is_child = false;
    for(uint16_t i = 0; i < num_of_connections; i++){
        pid_t pid = fork();
        if(pid == 0){
            is_child = true;
            break;
        }
    }
    if(is_child){
        int s = init_connection(ip_addr, port);
        benchmark_loop(s,msgs_per_request, delay);
    }
    else{
        while(true)
            sleep(1000);
    }
    return 0;
}
