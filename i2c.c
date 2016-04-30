/* 
 * File:   i2c.c
 * Author: gvanhoy
 *
 * Created on October 15, 2015, 8:18 AM
 */

#include <xc.h>
#include "lcd.h"

#define MPU6050_ADDRESS 0b01101000
#define dt 0.4

#define I2C_WRITE 0
#define I2C_READ 1

void initI2C2(){
    I2C2CONbits.ON = 1;
//    I2C2BRG = 100*0x030;
    I2C2BRG = 4096/2 - 1;
    TRISDbits.TRISD2 = 0;
}

void i2cWrite(char i2cAddress, char data){
// Start condition
    I2C2CONbits.SEN = 1;                    // Start condition
    while(I2C2CONbits.SEN);
    
    // Send I2C Address and wait for ACK
    IFS1bits.I2C2MIF = 0;
    I2C2TRN = i2cAddress << 1 | I2C_WRITE;   // Send data
    if(I2C2STATbits.IWCOL == 1){
        I2C2STATbits.IWCOL = 0;
    }
    while(IFS1bits.I2C2MIF == 0);           // Wait until its transmitted
    if(!I2C2STATbits.ACKSTAT){
        LATDbits.LATD2 = 1;   
    }
    else{
        LATDbits.LATD2 = 0;
//        return;
    }    

    // Send data register address and wait for acknowledge
    IFS1bits.I2C2MIF = 0;
    I2C2TRN = data;               // Send Register
    if(I2C2STATbits.IWCOL == 1){
        I2C2STATbits.IWCOL = 0;
    }
    while(IFS1bits.I2C2MIF == 0);           // Wait until its transmitted
    if(!I2C2STATbits.ACKSTAT){
        LATDbits.LATD2 = 1;   
    }
    else{
        LATDbits.LATD2 = 0;
//        return;
    }
    
    // Start-restart condition
    I2C2CONbits.PEN = 1;
    while(I2C2CONbits.PEN);           // Wait until its transmitted
}

char i2cRead(char i2cAddress, char regAddress){
    char data;
    
    // Start condition
    I2C2CONbits.SEN = 1;                    // Start condition
    while(I2C2CONbits.SEN);
    
    // Send I2C Address and wait for ACK
    IFS1bits.I2C2MIF = 0;
    I2C2TRN = i2cAddress << 1 | I2C_WRITE;   // Send data
    if(I2C2STATbits.IWCOL == 1){
        I2C2STATbits.IWCOL = 0;
    }
    while(IFS1bits.I2C2MIF == 0);           // Wait until its transmitted
    if(!I2C2STATbits.ACKSTAT){
        LATDbits.LATD2 = 0;   
    }
    else{
        LATDbits.LATD2 = 1;
//        return;
    }
    

    // Send data register address and wait for acknowledge
    IFS1bits.I2C2MIF = 0;
    I2C2TRN = regAddress;               // Send Register
    if(I2C2STATbits.IWCOL == 1){
        I2C2STATbits.IWCOL = 0;
    }
    while(IFS1bits.I2C2MIF == 0);           // Wait until its transmitted
    if(!I2C2STATbits.ACKSTAT){
        LATDbits.LATD2 = 1;   
    }
    else{
        LATDbits.LATD2 = 0;
//        return;
    }
    
    // Start-restart condition
    I2C2CONbits.PEN = 1;
    while(I2C2CONbits.PEN);           // Wait until its transmitted
    
    delayUs(10);
    
    I2C2CONbits.SEN = 1;
    while(I2C2CONbits.SEN);           // Wait until its transmitted
    
    // Send I2C and READ
    I2C2TRN = i2cAddress << 1 | I2C_READ;   // Send data
    if(I2C2STATbits.IWCOL == 1){
        I2C2STATbits.IWCOL = 0;
    }
    while(I2C2STATbits.TRSTAT);           // Wait until its transmitted
    
//    if(I2C2STATbits.ACKSTAT);
    
    // Receive Data
    I2C2CONbits.RCEN = 1;
    while(I2C2CONbits.RCEN);           // Wait until its transmitted
    data = I2C2RCV;
    
    // Not Acknowledge
    I2C2CONbits.ACKDT = 1;
    I2C2CONbits.ACKEN = 1;
    while(I2C2CONbits.ACKEN);           // Wait until its transmitted
    
    // Stop condition
    I2C2CONbits.PEN = 1;                    // Create a stop condition
    while(I2C2CONbits.PEN);
    
    return data;
}

void Test_MPU6050(void) {
    char data = 0x00;
    char dat[2];
    clearLCD();
    data  = i2cRead(0b01101000, 0b01110101);
    if(data == 0x68)
	{
		printStringLCD("MPU6050 - DTSUCC"); // Data Transfer Successful
	}
	else
	{
		printStringLCD("MPU6050 - DTFAIL"); // Data Transfer Failure
	}
    delayMs(2000);
}

void Setup_MPU6050(void) {
    int i;
    i2cWrite(25,0x01);  //Sets sample rate to 1000/1+1 = 500Hz
    i2cWrite(26,0x03);  //Disable FSync, 48Hz DLPF
    i2cWrite(27,0x08);  //Disable gyro self tests, scale of 500 degrees/s
    i2cWrite(28,0x08);  //Disable accel self tests, scale of +-4g, no DHPF
    i2cWrite(29,0x00);  //Freefall threshold of <|0mg|
    i2cWrite(30,0x00);  //Freefall duration limit of 0
    i2cWrite(31,0x00);  //Motion threshold of >0mg
    i2cWrite(32,0x00);  //Motion duration of >0s
    i2cWrite(33,0x00);  //Zero motion threshold
    i2cWrite(34,0x00);  //Zero motion duration threshold
    i2cWrite(35,0x00);  //Disable sensor output to FIFO buffer
    
    //Setup AUX I2C slaves and set master control to single master
    for(i = 36; i < 54; i++) {
        i2cWrite(i,0x00);
    }
    
    i2cWrite(55,0x00);  //Setup INT pin and AUX I2C pass through
    i2cWrite(56,0x00);  //Enable data ready interrupt
    
    //Slave out, dont care
    for(i = 99; i < 103; i++) {
        i2cWrite(i,0x00);
    }
    
    i2cWrite(103,0x00); //More slave config
    i2cWrite(104,0x00); //Reset sensor signal paths
    i2cWrite(105,0x00); //Motion detection control
    i2cWrite(106,0x00); //Disables FIFO, AUX I2C, FIFO and I2C reset bits to 0
    i2cWrite(107,0x02); //Sets clock source to gyro reference w/ PLL
    i2cWrite(108,0x00); //Controls frequency of wakeups in accel low power mode plus the sensor standby modes
    
    writeCMD(0x01);
    
    printStringLCD("MPU6050 - DICOMP"); // Device Initialization Complete
    delayMs(2000);
    
}

void Get_Gyro_Rates()
{
    float X,Y,Z;
    short GYRO_XOUT, GYRO_YOUT, GYRO_ZOUT;
    char GYRO_XOUT_H, GYRO_XOUT_L, GYRO_YOUT_H, GYRO_YOUT_L, GYRO_ZOUT_H, GYRO_ZOUT_L;
    char buf[16];
    const char* str;
    
    GYRO_XOUT_H = i2cRead(MPU6050_ADDRESS, 0x43);
    GYRO_XOUT_L = i2cRead(MPU6050_ADDRESS, 0x44);
    GYRO_XOUT_H = i2cRead(MPU6050_ADDRESS, 0x45);
    GYRO_XOUT_L = i2cRead(MPU6050_ADDRESS, 0x46);
    GYRO_XOUT_H = i2cRead(MPU6050_ADDRESS, 0x47);
    GYRO_XOUT_L = i2cRead(MPU6050_ADDRESS, 0x48);
 
	GYRO_XOUT = ((GYRO_XOUT_H<<8)|GYRO_XOUT_L);
	GYRO_YOUT = ((GYRO_YOUT_H<<8)|GYRO_YOUT_L);
	GYRO_ZOUT = ((GYRO_ZOUT_H<<8)|GYRO_ZOUT_L);
 
 
	X = (float)GYRO_XOUT/131;
	Y = (float)GYRO_YOUT/131;
	Z = (float)GYRO_ZOUT/131;
    writeCMD(0x01);
    delayUs(500);
    sprintf(buf, "%.1f %.1f %.1f", X, Y, Z);
    str = buf;
    printStringLCD(str);
}

void Get_ACC_Rates()
{
    short X,Y,Z;
    short GYRO_XOUT, GYRO_YOUT, GYRO_ZOUT;
    char GYRO_XOUT_H, GYRO_XOUT_L, GYRO_YOUT_H, GYRO_YOUT_L, GYRO_ZOUT_H, GYRO_ZOUT_L;
    char buf[16];
    const char* str;
    
    GYRO_XOUT_H = i2cRead(MPU6050_ADDRESS, 0x3B);
    GYRO_XOUT_L = i2cRead(MPU6050_ADDRESS, 0x3C);
    GYRO_XOUT_H = i2cRead(MPU6050_ADDRESS, 0x3D);
    GYRO_XOUT_L = i2cRead(MPU6050_ADDRESS, 0x3E);
    GYRO_XOUT_H = i2cRead(MPU6050_ADDRESS, 0x3F);
    GYRO_XOUT_L = i2cRead(MPU6050_ADDRESS, 0x40);
 
	X = ((GYRO_XOUT_H<<8)|GYRO_XOUT_L);
	Y = ((GYRO_YOUT_H<<8)|GYRO_YOUT_L);
	Z = ((GYRO_ZOUT_H<<8)|GYRO_ZOUT_L);

    writeCMD(0x01);
    delayUs(500);
    sprintf(buf, "%2d %2d %2d", X, Y, Z);
    str = buf;
    printStringLCD(str);
}