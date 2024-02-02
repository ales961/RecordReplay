#include "hardware/led.h"

#include <malloc.h>

void ledHardwareSet(Led* led);

void ledInit(Led* led, GPIO_TypeDef* port, uint16_t pin) {
    led->state = LED_UNCERTAIN;
    led->port = port;
    led->pin = pin;
}


char* ledGetState(void* arg) {
	Led* led = (Led*) arg;
    switch (led->state) {
    case LED_TURN_ON:
    	return "Led on";
    	break;
    case LED_TURN_OFF:
    	return "Led off";
    	break;
    default:
    	return "Unknown";
    }
}

char* ledSetOn(void* arg) {
	Led* led = (Led*) arg;
    led->state = LED_TURN_ON;
    ledHardwareSet(led);
    return "done";
}

char* ledSetOff(void* arg) {
	Led* led = (Led*) arg;
    led->state = LED_TURN_OFF;
    ledHardwareSet(led);
    return "done";
}

void ledHardwareSet(Led* led) {
    HAL_GPIO_WritePin(led->port, led->pin, led->state == LED_TURN_ON ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
