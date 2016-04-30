

#include <xc.h>
#include <sys/attribs.h>
#include <peripheral/i2c.h>
#include "config.h"
#include "interrupt.h"
#include "timer.h"
#include "i2c.h"
#include "accel.h"
#include "lcd.h"

#define SYS_CLK 10000000
#define MPU_ADDR 0b01101000


int main(void){
    SYSTEMConfigPerformance(SYS_CLK);
    
    enableInterrupts();
    initI2C2();
    initLCD();
    TRISDbits.TRISD0 = 0;
    
    char data = 0x00;
    int i;
    LATDbits.LATD0 = 0;
    
    writeCMD(0x01);
    Test_MPU6050();
    for(i = 0; i < 5000; i++) { delayUs(1000); }
    writeCMD(0x01);
    delayUs(1000);
    printStringLCD("Setting up...");
    for(i = 0; i < 5000; i++) { delayUs(1000); }
    
    i2cWrite(0x6B, 0x00);
    Setup_MPU6050();
    
    writeCMD(0x01);
    Test_MPU6050();
    for(i = 0; i < 5000; i++) { delayUs(1000); }
    
    while(1){
        LATDbits.LATD2 = 0;
        data  = i2cRead(MPU_ADDR, 0b01110101);
        Get_ACC_Rates();
        LATDbits.LATD0 = 1;
        for(i = 0; i < 200; i++) { delayUs(1000); }
        LATDbits.LATD0 = !(data == 0x68);
        for(i = 0; i < 200; i++) { delayUs(1000); }

    }
    
    return 0;
}