#include "rr.h"
#include "hardware/flash.h"
#include "string.h"
#include "cmsis_os.h"
#include "stdio.h"

#define MAX_COMMAND_LENGTH 16

TcpMessage messageBuffer[MAX_MESSAGES];
uint32_t nextWriteIndex = 0;
uint32_t nextReadIndex = 0;

void storeMessage(void* data, uint16_t len) {
    memcpy(messageBuffer[nextWriteIndex].data, (char*) data, len);
    messageBuffer[nextWriteIndex].length = len;
    messageBuffer[nextWriteIndex].timestamp = osKernelSysTick();
    nextWriteIndex = (nextWriteIndex + 1) % MAX_MESSAGES;
}

TcpMessage retrieveMessage() {
    TcpMessage message = messageBuffer[nextReadIndex];
    nextReadIndex = (nextReadIndex + 1) % MAX_MESSAGES;
    return message;
}

void saveMessages() {
	uint32_t countSector = FLASH_SECTOR_10;
	uint32_t dataSector = FLASH_SECTOR_11;
	EraseSector(countSector);
	EraseSector(dataSector);
	uint32_t messageCounter = 0;
	uint32_t startAddress = sectorAddresses[dataSector];
	size_t size = sizeof(TcpMessage);
	while (nextReadIndex != nextWriteIndex) {
	    TcpMessage message = retrieveMessage();
	    uint8_t* bytes = (uint8_t*)(&message);
	    for (size_t i = 0; i < size; i++) {
	        FlashWriteByte(startAddress++, bytes[i]);
	    }
	    messageCounter++;
	}
	FlashWriteWord(sectorAddresses[countSector], messageCounter);
}

void loadMessages() {
	uint32_t countSector = FLASH_SECTOR_10;
	uint32_t dataSector = FLASH_SECTOR_11;
	uint32_t messageCounter = FlashReadWord(sectorAddresses[countSector]);
	uint32_t startAddress = sectorAddresses[dataSector];
	size_t size = sizeof(TcpMessage);
	for (uint32_t i = 0; i < messageCounter; i++) {
	    uint8_t* bytes = (uint8_t*)&messageBuffer[nextWriteIndex];
	    for (size_t j = 0; j < size; j++) {
	        bytes[j] = FlashReadByte(startAddress++);
	    }
	    nextWriteIndex = (nextWriteIndex + 1) % MAX_MESSAGES;
	}
}

uint8_t isMessageBufferEmpty() {
	return nextWriteIndex == nextReadIndex;
}

void trimString(char* input) {
    // Trim leading whitespace
    while (*input == ' ' || *input == '\t' || *input == '\r' || *input == '\n') {
        input++;
    }

    // Trim trailing whitespace
    size_t length = strlen(input);
    while (length > 0 && (input[length - 1] == ' ' || input[length - 1] == '\t' || input[length - 1] == '\r' || input[length - 1] == '\n')) {
        input[--length] = '\0';
    }
}

char* commandExecute(char* str) {
    trimString(str);
    if (strcmp(str, "save") == 0) {
    	saveMessages();
        return "Done";
    } else if (strcmp(str, "load") == 0) {
    	loadMessages();
    	return "Done";
    }
    return "Not found";
}
