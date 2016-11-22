#include <stdio.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

// Prints a response_message struct in a human readable manner
// useful for debugging
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
    printf("status_code: 0x%X\nkey: %s\nvalue: %s\n", resp->status_code, key, value);
}

// Prints a send_message struct in a human readable manner
// useful for debugging
void print_send_message(struct send_message * send){
    char key[16 * 2 + 1];
    char *key_ptr = key;

    for(int i = 0; i < 16; i++){
        key_ptr += sprintf(key_ptr, "%02X", send->key[i]);
    }
    printf("operation: 0x%X\nkey: %s\n", send->operation, key);

}
// Gets the minumum of two timespec structs
struct timespec* min_timespec(struct timespec *t1, struct timespec *t2){
    if (t1->tv_sec == t2->tv_sec)
        return t1->tv_nsec < t2->tv_nsec ? t1 : t2;
    else
        return t1->tv_sec < t2->tv_sec ? t1 : t2;
}

// Gets the maximum of two timespec structs
struct timespec* max_timespec(struct timespec *t1, struct timespec *t2){
    if (t1->tv_sec == t2->tv_sec)
        return t1->tv_nsec > t2->tv_nsec ? t1 : t2;
    else
        return t1->tv_sec > t2->tv_sec ? t1 : t2;
}

// Basically a simple memcpy for timespecs
void copy_timespec(struct timespec *dest, struct timespec *src){
    dest->tv_nsec = src->tv_nsec;
    dest->tv_sec = src->tv_sec;
}

// Does all the tcp socket initialization that must be done
// and returns the fd
int init_tcp_connection(uint32_t ip_addr, uint16_t port){
    int socket_desc;
    struct sockaddr_in server;

    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Failed to create socket\n");
        exit(-1);
    }

    server.sin_addr.s_addr = ip_addr;
    server.sin_family = AF_INET;
    server.sin_port = port;

    //Connect to remote server
    if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0){
        printf("Failed to connect to server\n");
        exit(-1);
    }
    return socket_desc;
}


// Does all the tcp socket initialization that must be done
// and returns the fd
int init_udp_connection(uint32_t ip_addr, uint16_t port){
    int socket_desc;

    //Create UDP socket
    socket_desc = socket(AF_INET , SOCK_DGRAM , 0);
    if (socket_desc == -1)
    {
        printf("Failed to create socket\n");
        exit(-1);
    }
    return socket_desc;
}
