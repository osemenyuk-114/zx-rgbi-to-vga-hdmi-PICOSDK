#pragma once

#define PIN_LED (25u)
#define INPUT 0
#define OUTPUT 1
#define LOW 0
#define HIGH 1

void pinMode(unsigned int, bool);
void digitalWrite(unsigned int, bool);
int digitalRead(unsigned int);
