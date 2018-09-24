/*
 * MAX44009 Ambient Light Sensor with ADC library
 *
 *
 * Copyright (c) 2013 Davy Van Belle, MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
 * and associated documentation files (the "Software"), to deal in the Software without restriction, 
 * including without limitation the rights to use, copy, modify, merge, publish, distribute, 
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or 
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING 
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
 
 
#include "math.h"
#include "max44009.h"
#include "i2c.h"
  
/* Get LUX reading from ADC */
float MAX44009getLUX(){
	char buff[2];
	char cmd[2];
	char expo,mant;
	float lux;
		 
	cmd[0] = LUX_HIGH_B;
	cmd[1] = LUX_LOW_B;
    
	//I2C Adress of the device of which we want to speak
	//I2C_Setup(I2C_SLAVE, MAX44009_ADRESS);
    
	//Read from the register 
	I2C_Write1(cmd[0]);
	I2C_Read(&buff[0],1);
	I2C_Write1(cmd[1]);
	I2C_Read(&buff[1],1);
	
	//LUX Calculation, formular from datasheet
	expo = (buff[0] & 0xF0) >> 4;
	mant = ((buff[0] & 0x0F) << 4) | (buff[1] & 0x0F) ;
	lux = pow((double)2,(double)expo) * mant * 0.045;
    return lux;
}
    
   
