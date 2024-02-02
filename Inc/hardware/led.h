#ifndef HARDWARE_LED_H
#define HARDWARE_LED_H

#include <stdint.h>

#include "stm32f4xx_hal.h"



typedef enum { LED_TURN_ON, LED_TURN_OFF, LED_UNCERTAIN } LedState;
typedef struct Led{
    LedState state;
    GPIO_TypeDef* port;
    uint16_t pin;
} Led;

void ledInit(Led* led, GPIO_TypeDef* port, uint16_t pin);
char* ledGetState(void* arg);
char* ledSetOn(void* arg);
char* ledSetOff(void* arg);


#endif //HARDWARE_LED_H
