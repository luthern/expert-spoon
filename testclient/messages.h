
#ifndef MESSAGES_H
#define MESSAGES_H

#include <stdint.h>

// Simple header that specifies what messages we are going to be sending
// and recieving to the server

// TODO: Should probably put this outside the client and server folders
//  so both can draw from the same file??

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
#endif /* MESSAGES_H */
