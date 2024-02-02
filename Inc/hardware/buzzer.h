#ifndef BUZZER_H_
#define BUZZER_H_

#include <inttypes.h>

#define MAX_OCTAVE 8;
#define MIN_VOLUME 0;
#define MAX_VOLUME 10;

char* BuzzerSetOn(void* arg);
char* BuzzerSetOff(void* arg);

#endif /* BUZZER_H_ */
