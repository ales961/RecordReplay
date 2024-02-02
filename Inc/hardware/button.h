#ifndef HARDWARE_BUTTON_H
#define HARDWARE_BUTTON_H

#include <stdint.h>

#include "stm32f4xx_hal.h"

#define SHORT_CLICK 50

typedef void (*buttonActionListener)();
typedef enum { BUTTON_PRESSED, BUTTON_RELEASED, BUTTON_UNCERTAIN } ButtonState;
typedef struct Button {
    ButtonState state;
    buttonActionListener onClickHandler;
    GPIO_TypeDef* port;
    uint16_t pin;
} Button;

void buttonSetOnClick(Button* button, buttonActionListener actionListener);
void buttonInit(Button* button, GPIO_TypeDef* port, uint16_t pin);
void buttonUpdateState(Button* button);
char* buttonGetState(void* arg);

#endif //HARDWARE_BUTTON_H
