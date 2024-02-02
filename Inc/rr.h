#ifndef RR_H
#define RR_H

#include "stdint.h"

#define MAX_MESSAGES 128
#define MAX_MESSAGE_SIZE 32

typedef struct {
    char data[MAX_MESSAGE_SIZE];
    uint16_t length;
    uint32_t timestamp;
} TcpMessage;

void storeMessage(void* data, uint16_t len);
TcpMessage retrieveMessage();
void saveMessages();
void loadMessages();
uint8_t isMessageBufferEmpty();
char* commandExecute(char* str);

#endif /* RR_H */
