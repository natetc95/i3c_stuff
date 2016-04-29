// ******************************************************************************************* //
//
// File:         lab3p1.c
// Date:         3-28-2014
// Authors:      Alex Levine
//
// Description: This file initializes the PWM, and remaps pins to allow the robot to go
//      forwards and backwards. It also shuts off the output compare modules 2, 4, and 5 to
//      go into the idle state. The PWM uses timer 3 with a prescalar of 1 and a PR value of 1023.
// ******************************************************************************************* //

#ifndef PWM_H
#define	PWM_H

#ifdef	__cplusplus
extern "C" {
#endif

void initPWM();
void changeForwards();
void turnAround();
void OC_off();
void OC_on();
void accelerator(int L, int R);


#ifdef	__cplusplus
}
#endif

#endif	/* PWM_H */
