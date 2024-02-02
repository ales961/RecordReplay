#include <hardware/uart.h>
#include "string.h"
#include "usart.h"
#include "buffer.h"
#include "rr.h"

uint8_t usartHardwareTransmitBuffer;
uint8_t usartHardwareReceiveBuffer;

static Buffer txBuffer;
static Buffer rxBuffer;
static Buffer commandBuffer;
static volatile uint8_t interruptionsEnabled = 0;
static volatile uint8_t transmitIntEnd = 1;
uint8_t execBuf[64];



uint8_t uartHardwareTransmitPolling(uint8_t data, const uint16_t timeOut) {
    return HAL_UART_Transmit(&huart6, &data, 1, timeOut) == HAL_OK;
}

void uartHardwareTransmitInterrupt(uint8_t buffer) {
	usartHardwareTransmitBuffer = buffer;
	HAL_NVIC_EnableIRQ(USART6_IRQn);
	HAL_UART_Transmit_IT(&huart6, &usartHardwareTransmitBuffer, 1);
}

static void hardwareTransmitIT() {
    if (!transmitIntEnd)
        return;

    if (bufferHasValues(&txBuffer)) {
        transmitIntEnd = 0;
        uint8_t data = bufferPop(&txBuffer);
        uartHardwareTransmitInterrupt(data);
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart == &huart6) {
	    transmitIntEnd = 1;
	    if (interruptionsEnabled)
	        hardwareTransmitIT();
	}
}

uint16_t uartTransmit(const uint8_t * const msg, const uint16_t msgSize) {
    if (msg == NULL)
        return 0;
    const uint16_t transmitted = bufferPushValues(&txBuffer, msg, msgSize);
    if (interruptionsEnabled) {
        hardwareTransmitIT();
    }
    return transmitted;
}

uint8_t uartTransmitChar(const uint8_t msg) {
    if (msg == NULL)
        return 0;
    const uint8_t transmitted = bufferPush(&txBuffer, msg);
    if (interruptionsEnabled) {
        hardwareTransmitIT();
    }
    return transmitted;
}

uint8_t uartHardwareReceivePolling(uint8_t * const data, const uint16_t timeOut) {
    return HAL_UART_Receive(&huart6, data, 1, timeOut) == HAL_OK;
}

static void hardwareReceiveIT() {
	HAL_NVIC_EnableIRQ(USART6_IRQn);
	HAL_UART_Receive_IT(&huart6, &usartHardwareReceiveBuffer, 1);
}

uint16_t uartReceive(uint8_t * const buffer, const uint16_t size) {
    const uint16_t received = bufferPopValues(&rxBuffer, buffer, size);
    if (interruptionsEnabled) {
        hardwareReceiveIT();
    }
    return received;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart == &huart6) {
        if (bufferGetFreeSize(&rxBuffer) > 0) {
            bufferPush(&rxBuffer, usartHardwareReceiveBuffer);
        }
        if (interruptionsEnabled)
            hardwareReceiveIT();
    }
}


uint8_t uartHasNext() {
    return bufferHasValues(&rxBuffer);
}

void uartInit() {
    bufferClear(&rxBuffer);
    bufferClear(&txBuffer);
    bufferClear(&commandBuffer);
}

static void hardwareTransmit() {
    if (bufferHasValues(&txBuffer)) {
        const uint8_t byte = bufferPeek(&txBuffer);
        if (uartHardwareTransmitPolling(byte, 5))
            bufferPop(&txBuffer);
    }
}

static void hardwareReceive() {
    if (bufferGetFreeSize(&rxBuffer) > 0) {
        uint8_t data = 0;
        if (uartHardwareReceivePolling(&data, 5)) {
            bufferPush(&rxBuffer, data);
        }
    }
    if (!bufferHasValues(&rxBuffer))
        bufferClear(&rxBuffer);
}

void uartPolling() {
    if (!interruptionsEnabled) {
        hardwareTransmit();
        hardwareReceive();
    }
}

void uartEnableInterruption() {
    interruptionsEnabled = 1;
    transmitIntEnd = 1;
    hardwareTransmitIT();
    hardwareReceiveIT();
}

void uartHardwareDisableInterrupts() {
    HAL_UART_Abort(&huart6);
    HAL_NVIC_DisableIRQ(USART6_IRQn);
}

void uartDisableInterruption() {
    interruptionsEnabled = 0;
    uartHardwareDisableInterrupts();
}

uint8_t uartIsInterruptionEnabled() {
    return interruptionsEnabled;
}

void commandCheck() {
	uint8_t bp = bufferPeekLast(&commandBuffer);
	uint8_t r = (uint8_t) '\r';
	if (bp == r) {
		uartTransmitChar((uint8_t) '\n');
		uint16_t bufSize = bufferGetSize(&commandBuffer);
		bufferPopValues(&commandBuffer, execBuf, bufSize);
		bufferClear(&commandBuffer);
		char* response = commandExecute((char*) execBuf);
		uartTransmitChar((uint8_t) '\n');
		uartTransmit((uint8_t*) response, strlen(response));
	}
}

void uartEcho() {
	uint8_t msg = bufferPop(&rxBuffer);
	bufferPush(&commandBuffer, msg);
	uartTransmitChar(msg);
}



