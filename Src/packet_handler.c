#include "packet_handler.h"
#include "hardware/led.h"
#include "hardware/button.h"
#include "hardware/buzzer.h"

char* packetHandle(void* data, uint16_t* len, Command* command, uint8_t* commandNumber) {
    if (*len == sizeof(CommandType)) {
    	CommandType recved_type = *(CommandType*) data;
    	for (uint8_t i = 0; i < *commandNumber; i++) {
    		Command cm = command[i];
    		if (cm.type == recved_type) {
    			return cm.handler(cm.data);
    		}
    	}
    	return "Command not found";
    }
    return "Invalid packet";
}

void commandCreate(Command* command, CommandType type, char* (*handler)(void* data), void* data) {
	command->type = type;
	command->handler = handler;
	command->data = data;
}
