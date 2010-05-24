#ifndef BME_JOY_H
#define BME_JOY_H

// Joystick functions header file

int joy_detect(unsigned id);
unsigned joy_getstatus(unsigned id, int threshold);

#endif
