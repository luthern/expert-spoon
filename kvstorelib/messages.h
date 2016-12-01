
#ifndef MESSAGES_H
#define MESSAGES_H

#include <stdint.h>

// Simple header that specifies what messages we are going to be sending
// and recieving to the server

// TODO: Should probably put this outside the client and server folders
//  so both can draw from the same file??

typedef enum {
	GET,
	PUT,
	DELETE,
	UPDATE,
} Operation;

typedef enum {
	OK,
	ERROR,
	NOT_FOUND,
	KEY_IN_USE,
} StatusCode;

struct send_message {
	char operation;
	char keepalive;	
	char key[16];
	char value[32];
};

struct response_message {
	char status_code;
	char keepalive;
	char key[16];
	char value[32];
};

#endif /* MESSAGES_H */
