#include <math.h>
#include <string.h>
#include <stdio.h>
#include "hardware/buzzer.h"
#include "tim.h"

uint8_t isPlaying = 0;

uint8_t octave = 4;
char* octaveNames[9] = {"sub-contra octave", "contra octave", "great octave", "small octave",
		"one-line octave", "two-line octave", "three-line octave", "four-line octave", "five-line octave"};

double notes[7] = {16.352, 18.354, 20.602, 21.827, 24.5, 27.5, 30.868};
char* noteNames[7] = {"A", "B", "C", "D", "E", "F", "G"};

double getNoteFrequency(uint8_t noteNumber) {
	if (noteNumber < 0 || noteNumber > 6) return 0;
	return rint(notes[noteNumber] * pow(2, octave));
}

void setVolume(uint8_t volume) {
	if (0 <= volume && volume <= 10) {
		TIM1->CCR1 = volume;
	}
}

void setFrequency(uint16_t freq) {
	TIM1->PSC = HAL_RCC_GetPCLK2Freq() / (20 * freq) - 1;
}

char* BuzzerSetOn(void* arg) {
	setFrequency((uint16_t) getNoteFrequency(6));
	setVolume(7);
	return "done";
}

char* BuzzerSetOff(void* arg) {
	setVolume(0);
	return "done";
}
