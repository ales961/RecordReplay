#include "hardware/button.h"

static uint8_t buttonHardwareGetState(Button* button);

void buttonInit(Button* button, GPIO_TypeDef* port, uint16_t pin) {
	button->state = BUTTON_UNCERTAIN;
	button->onClickHandler = NULL;
	button->port = port;
	button->pin = pin;
}

void buttonSetOnClick(Button* button, buttonActionListener actionListener) {
	button->onClickHandler = actionListener;
}

static volatile uint32_t clickStart = 0;
uint8_t handled = 0;
void buttonUpdateState(Button* button) {
    uint32_t time = HAL_GetTick();
    uint8_t isPressed = buttonHardwareGetState(button);
    if (isPressed) {
    	if (clickStart > 0 && (time - clickStart) >= SHORT_CLICK) {
    		clickStart = 0;
    		button->state = BUTTON_PRESSED;
    		if (button->onClickHandler != NULL && !handled) {
    			(*(button->onClickHandler))();
    			handled = 1;
    		}
    	} else {
            clickStart = time;
        }
    } else {
    	clickStart = 0;
    	handled = 0;
    	button->state = BUTTON_RELEASED;
    }
}

char* buttonGetState(void* arg) {
	Button* button = (Button*) arg;
    switch (button->state) {
    case BUTTON_PRESSED:
    	return "Pressed";
    	break;
    case BUTTON_RELEASED:
    	return "Released";
    	break;
    default:
    	return "Unknown";
    }
}

static uint8_t buttonHardwareGetState(Button* button) {
	return HAL_GPIO_ReadPin(button->port, button->pin) == GPIO_PIN_RESET;
}


