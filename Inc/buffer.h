/*
 * buffer.h
 *
 *  Created on: 30 янв. 2024 г.
 *      Author: Mary
 */

#ifndef BUFFER_H
#define BUFFER_H

#include <stdint.h>

#define BUFFER_SIZE 128
struct Buffer {
    uint16_t ptrIn;
    uint16_t ptrOut;
    uint16_t size;
    uint8_t data [BUFFER_SIZE];
};
typedef struct Buffer Buffer;

uint8_t bufferPush(Buffer *buffer, uint8_t value);
uint16_t bufferPushValues(Buffer *buffer, const uint8_t *values, uint16_t size);
uint8_t bufferPop(Buffer *buffer);
uint8_t bufferPeek(const Buffer *buffer);
uint8_t bufferPeekLast(const Buffer *buffer);
void bufferClear(Buffer *buffer);
uint16_t bufferGetSize(const Buffer *buffer);
uint16_t bufferPopValues(Buffer *buffer, uint8_t *buf, uint16_t bufSize);
uint8_t bufferHasValues(const Buffer *buffer);
uint16_t bufferGetFreeSize(const Buffer *buffer);
uint16_t bufferPeekValues(const Buffer * buffer, uint8_t **values);

#endif /* BUFFER_H */
