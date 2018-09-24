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
 
/** @file
 * @brief MAX44009 I2C
 */
 
#ifndef MAX44009_H
#define MAX44009_H

//=== Includes =====================================================================================	


//=== Preprocessing directives (#define) ===========================================================
 
#define INT_STATUS 0x00
#define INT_ENABLE 0x01
 
#define CONFIG 0x02
 
#define LUX_HIGH_B 0x03
#define LUX_LOW_B  0x04
 
#define UP_THRESH_HIGH_B 0x05
#define LOW_THRESH_HIGH_B 0x06
 
#define THRESH_TIMER 0x07

//The MAX44009 have two I2C Adresses, one must be choosen
#define MAX44009_ADRESS 0x4A
//#define MAX44009_ADRESS 0x4B

//=== Preprocessing directives (#define) ===========================================================

//=== Type definitions (typedef) ===================================================================

//=== Global constants =============================================================================

//=== Global variables =============================================================================

//=== Local constants  =============================================================================

//=== Local variables ==============================================================================

//=== Local function prototypes ====================================================================
 
	/** Get LUX reading from ADC **/
	float MAX44009getLUX();

///** MAX44009 class  */
//class MAX44009 {
//public:
    /** init MAX44009 class
     * @param *i2c pointer to I2C serial interface
     * @param addr sensor I2C address
     */
  //  MAX44009 (I2C* i2c, char addr); 
 
    /** Set configuration register for the device
     * @param config desired configuration register bits
     * BIT 7 - CONT: 1 = continuous mode, 0 = single measurement
     * BIT 6 - MANUAL: 1 = CDR, TIM[2:0] set by user, 0 = CDR, TIM[2:0] set by internal autorange
     * BIT [5:4] - Not Used
     * BIT 3 - CDR: 1 = Current divided by 8. (High-brightness), 0 = Current not divided.
     * BIT [2:0] - TIM: Integration Time. See datasheet.
     */
   // void setConfig (char config);
    
    /** Get device INT_STATUS register
     * BIT 0 : 0 = No interrupt event occurred, 1 = Ambient light intensity is outside the threshold range. 
     */  
 //   char getIntStatus();
 
    /** Set device INT_ENABLE register
     * @param Enable  BIT 0 : 0 = INT pin and INTS bit not effected if an interrupt event occurred, 1 = INT pin pulled low and INTS bit is set if interrupt occurred. /
     */  
  //  void setIntEnable(bool Enable);
 

    /** Set upper threshold
    * @param threshold set upper threshold value. Further info, see datasheet
    */
 //  void setUpperThres(char threshold);
    
    /** Set lower threshold
    * @param threshold set lower threshold value. Further info, see datasheet
    */
   //oid setLowerThres(char threshold);
    
    /** Set Threshold time
    * @param time set time to trigger interrupt if value is below or above threshold value. Further info, see datasheet
    */
  //  void setThresTimer(char time);
    
//    float operator= (float d);    
    
//protected:
 
 
//private:
  //  char _addr;
   // I2C *_i2c;
    
//};
    
#endif 
