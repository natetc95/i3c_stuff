#include <xc.h>
#include <sys/attribs.h>
#include <peripheral/i2c.h>
#include <lcd.h>

#define gyro_xsensitivity 131 //66.5 Dead on at last check
#define gyro_ysensitivity 131 //72.7 Dead on at last check
#define gyro_zsensitivity 131
#define dt 0.01 //TIMER 5: TCKPS = 0 / PR = 100 / Set an interupt / 10ms per update

void initI2C() {
    I2CEnable(I2C2, FALSE);
    I2CConfigure(I2C2, I2C_EN);
    I2C2BRG = 4096/2 - 1;
    TRISDbits.TRISD0 = 0;
    I2CEnable(I2C2, TRUE);
}

void i2cWrite (unsigned char address, unsigned char data) {
    while(!I2CBusIsIdle(I2C2));                 // Wait until I2C bus is idle
    I2CStart(I2C2);                             // Send I2C Start condition on SDA
    while(!I2CTransmitterIsReady(I2C2));        // Wait until transmitter is ready
    I2CSendByte(I2C2, address << 1);            // If it is, send address w/ write byte
    while(!I2CTransmissionHasCompleted(I2C2));  // Wait for transmission to complete
    if (I2CByteWasAcknowledged(I2C1)) {         // Check if slave acknowledged the transmission
        LATDbits.LATD0 = 0;
    }
    else {                                      // If Slave sends no-ack (i.e. transmission was received successfully)
        LATDbits.LATD0 = 1;
        while(!I2CBusIsIdle(I2C2));                 // Wait until I2C bus is idle
        while(!I2CTransmitterIsReady(I2C2));        // Wait until transmitter is ready
        I2CSendByte(I2C2, data);                    // If it is, send data
        while(!I2CTransmissionHasCompleted(I2C2));  // Wait for transmission to complete
        if (I2CByteWasAcknowledged(I2C1)) {         // Check if slave acknowledged the transmission
            LATDbits.LATD0 = 0;
        }
        else {
            LATDbits.LATD0 = 1;
        }
    }
    I2CStop();
}

unsigned char i2cRead (unsigned char address) {
    unsigned char data = 0x00;
    while(!I2CBusIsIdle(I2C2));                 // Wait until I2C bus is idle
    I2CStart(I2C2);                             // Send I2C Start condition on SDA
    while(!I2CTransmitterIsReady(I2C2));        // Wait until transmitter is ready
    I2CSendByte(I2C2, address << 1 | 1);        // If it is, send address w/ read byte
    while(!I2CTransmissionHasCompleted(I2C2));  // Wait for transmission to complete
    if (I2CByteWasAcknowledged(I2C1)) {         // Check if slave acknowledged the transmission
        LATDbits.LATD0 = 0;
    }
    else {
        LATDbits.LATD0 = 1;
        I2CStop();
        delayUs(10);
        I2CStart();
        while(!I2CBusIsIdle(I2C2));             // Wait until I2C bus is idle
        I2CReceiverEnable(I2C2,TRUE);           // Tell the device to get ready to receive
        if (I2CReceivedDataIsAvailable(I2C1)) {
            I2CAcknowledgeByte(I2C2, FALSE);
            data = I2CGetByte(I2C2);
        }
    }
    I2CStop();
    return data;
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
    
    clearLCD();
    printStringLCD("MPU6050 - DICOMP"); // Device Initialization Complete
    delayMs(2000);
    
}

void Test_MPU6050(void) {
    unsigned char data = 0x00;
    clearLCD();*
    data = i2cRead(117);
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

double GyroZUpdate(double current) {
    double new;
    UINT8 H, L;
    short c;
    H = i2cRead(71);
    L = i2cRead(72);
    c = H << 8 | L;
    new = c/gyro_zsensitivity;
    new = current + new;
    return new;
}