#include <xc.h>
#include <sys/attribs.h>
#include "lcd.h"
#include "timer.h"
#include "config.h"
#include "interrupt.h"
#include "switch.h"
#include "keypad.h"
#include "adc.h"
#include "pwm.h"
#include <stdlib.h>
#include "i2c.h"

#define T_SCL TRISDbits.TRISD0
#define T_SDA TRISDbits.TRISD1

#define SCL LATDbits.LATD0
#define SDA LATDbits.LATD1

int main( void ) {
    
    initTimer2();
    initLCD();
    initADC_1();
    initPWM();
    
    initI2C();
    test_MPU6050();
    
}
