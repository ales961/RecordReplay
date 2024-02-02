#ifndef PACKET_HANDLER_H
#define PACKET_HANDLER_H

#include <stdint.h>

typedef enum {
	LED_RED_GET_STATE 	 = 0xA001,
    LED_RED_ON 			 = 0xA002,
	LED_RED_OFF 		 = 0xA003,
	LED_YELLOW_GET_STATE = 0xA011,
    LED_YELLOW_ON 		 = 0xA012,
	LED_YELLOW_OFF 		 = 0xA013,
	LED_GREEN_GET_STATE  = 0xA021,
    LED_GREEN_ON 		 = 0xA022,
	LED_GREEN_OFF 		 = 0xA023,
    BUTTON_GET_STATE 	 = 0xB001,
	BUZZER_ON 		 	 = 0xC002,
	BUZZER_OFF 		 	 = 0xC003
} CommandType;

typedef struct Command {
	CommandType type;
	char* (*handler)(void* data);
	void* data;
} Command;

char* packetHandle(void* data, uint16_t* len, Command command[12], uint8_t* commandNumber);
void commandCreate(Command* command, CommandType type, char* (*handler)(void* data), void* data);


#endif
