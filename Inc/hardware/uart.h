#ifndef UART_H
#define UART_H

#include "inttypes.h"

uint16_t uartTransmit(const uint8_t * const msg, const uint16_t msgSize);
uint8_t uartTransmitChar(const uint8_t msg);
uint16_t uartReceive(uint8_t * const buffer, const uint16_t size);
uint8_t uartHasNext();
void uartInit();
void uartPolling();
void uartEnableInterruption();
void uartDisableInterruption();
uint8_t uartIsInterruptionEnabled();
void commandCheck();
void uartEcho();

#endif /* UART_H */
