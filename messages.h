
#ifndef MESSAGES_H
#define MESSAGES_H

#include <stdint.h>

// Simple header that specifies what messages we are going to be sending
// and recieving to the server

// TODO: Should probably put this outside the client and server folders
//  so both can draw from the same file??

enum Operation : uint8_t {
	GET=0,
	PUT=1,
	DELETE=2,
};

enum StatusCode : uint8_t {
	OK=0,
	ERROR=1,
};

struct send_message {
	Operation operation;
	unsigned char key[16];
	unsigned char value[32];
};

struct response_message {
	StatusCode status_code;
	unsigned char key[16];
	unsigned char value[32];
};

#endif /* MESSAGES_H */
