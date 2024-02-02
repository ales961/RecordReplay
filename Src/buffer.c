#include <stdint.h>
#include <stdlib.h>

#include "buffer.h"

#include <cmsis_gcc.h>

static uint32_t pmask;

static void disableInt();
static void enableInt();


uint8_t bufferPush(Buffer * const buffer, const uint8_t value) {
    if (buffer->size < BUFFER_SIZE) {
        disableInt();

        buffer->data[buffer->ptrIn] = value;
        buffer->ptrIn = (buffer->ptrIn + 1) % BUFFER_SIZE;
        buffer->size++;

        enableInt();
        return 1;
    } else
        return 0;
}

uint16_t bufferPushValues(Buffer * const buffer, const uint8_t * const values, const uint16_t size) {
    uint16_t pushed;
    for (pushed = 0; pushed < size; pushed++) {
        if (!bufferPush(buffer, values[pushed]))
            return pushed;
    }
    return pushed;
}

uint8_t bufferPop(Buffer * const buffer) {
    if (buffer->size == 0)
        return 0;

    disableInt();

    buffer->size--;
    uint8_t value = buffer->data[buffer->ptrOut];
    buffer->ptrOut = (buffer->ptrOut + 1) % BUFFER_SIZE;

    enableInt();

    return value;
}

uint8_t bufferPeek(const Buffer * const buffer) {
    return buffer->data[buffer->ptrOut];
}

uint8_t bufferPeekLast(const Buffer * const buffer) {
    return buffer->data[buffer->ptrIn-1];
}

void bufferClear(Buffer * const buffer) {
    disableInt();
    buffer->size = 0;
    buffer->ptrIn = 0;
    buffer->ptrOut = 0;
    enableInt();
}

uint8_t bufferHasValues(const Buffer * const buffer) {
    return buffer->size > 0;
}

uint16_t bufferPopValues(Buffer * const buffer, uint8_t * const buf, const uint16_t bufSize) {
    if (!bufferHasValues(buffer))
        return 0;
    uint16_t popSize = bufferGetSize(buffer);
    if (popSize > bufSize)
        popSize = bufSize;
    for (uint16_t i = 0; i < popSize; i++)
        buf[i] = bufferPop(buffer);
    return popSize;
}

uint16_t bufferGetSize(const Buffer * const buffer) {
    return buffer->size;
}

uint16_t bufferGetFreeSize(const Buffer * const buffer) {
    return BUFFER_SIZE - buffer->size;
}

uint16_t bufferPeekValues(const Buffer * const buffer, uint8_t **values) {
    *values = buffer->data + buffer->ptrOut;
    return buffer->size;
}

static void disableInt() {
	pmask = __get_PRIMASK();
    __disable_irq();
}

static void enableInt() {
    __set_PRIMASK(pmask);
}
