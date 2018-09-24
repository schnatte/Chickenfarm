/************************************************************/
/*Chickenfarm                                              	*/
/*----------------------------------------------------------*/
/*Author: Daniel Kettnaker                                 	*/
/*														 	*/ 	
/*History:													*/
/*15.01.2017 Initial version "Quick and Dirty" 				*/
/*19.01.2017 Integration or IRQ for Button open/close 		*/
/*20.01.2017 Integration of Display HD4478					*/
/*21.01.2017 Integration of MAX44009						*/
/*                                                         	*/ 
/************************************************************/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <time.h>
#include <lcd.h> 

#include <unistd.h>				//Needed for I2C port
#include <fcntl.h>				//Needed for I2C port
#include <sys/ioctl.h>			//Needed for I2C port
#include <linux/i2c-dev.h>		//Needed for I2C port

#include "max44009.h"   
#include "i2c.h"           

/************************************************************/
/*Set-up GPIO Pins for wiringPi libary                     	*/
/************************************************************/
//Set-up the PINS for the Switches 
#define BUTTON_START_STOP 0 //START-STOP Switch, GPIO17
#define MOTOR_UP 3  //Motor up, GPIO22 
#define MOTOR_DOWN 4 //Motor down, GPIO23
#define DOOR_CLOSED 5 //Door closed, GPIO24
#define DOOR_OPEN 6 //Door open, GPIO25
/*************************************************************/
//Set-up the PINS for the Display use wiringPi numbers
#define LCD_RS  10               //Register select pin grün
#define LCD_E   11               //Enable Pin gelb
#define LCD_D4  12               //Data pin 4
#define LCD_D5  13               //Data pin 5
#define LCD_D6  14               //Data pin 6
#define LCD_D7  15               //Data pin 7

int lcd; //Interrims solution no global variable

/*************************************************************/
/*Definition of Custom Characters for LCD Display            */
/*************************************************************/
char Arrow_up[8] = {
	0b00100,
	0b01110,
	0b11111,
	0b00100,
	0b00100,
	0b00100,
	0b00100,
	0b00100};

char Arrow_down[8] = {
	0b00100,
	0b00100,
	0b00100,
	0b00100,
	0b00100,
	0b11111,
	0b01110,
	0b00100};

char Door_closed[8] = {
	0b00000,
	0b11111,
	0b10001,
	0b10001,
	0b10111,
	0b10101,
	0b10001,
	0b10001};
	
char Door_open[8] = {
	0b00000,
	0b11111,
	0b10001,
	0b10001,
	0b10001,
	0b10001,
	0b10001,
	0b10001};

char drop[8] = {
	0b00100,
	0b00100,
	0b01010,
	0b01010,
	0b10001,
	0b10001,
	0b10001,
	0b01110};

/*************************************************************/
/*IRQ Handler                                   	         */
/*************************************************************/
//Variables for IRQ Handler to control bouncing
unsigned int LIT_BUTTON_START_STOP = 0; //LIT = Last Interrupt time
unsigned int LIT_DOOR_CLOSED = 0; //LIT = Last Interrupt time
unsigned int LIT_DOOR_OPEN = 0; //LIT = Last Interrupt time
/*************************************************************/
void ISR_BUTTON_START_STOP(void) {
   unsigned long interrupt_time = millis();
   //If interrupts come faster than 200ms, assume it's a bounce and ignore
   if(interrupt_time - LIT_BUTTON_START_STOP > 200){
   //Interrupt is accepted, do something!
   //Check Status of the different Switches
   	if(digitalRead(DOOR_OPEN) == 0){//When DOOR is open then.....
   	digitalWrite(MOTOR_DOWN, 1); //Close Door
   	digitalWrite(MOTOR_UP, 0); //TEST
	//Pfeil runter
	lcdPosition(lcd, 19, 3); //Define the position where to write the Date column,row
	lcdPutchar(lcd, 11);
	}
   	if(digitalRead(DOOR_CLOSED) == 0){//When DOOR is closed then....
   	digitalWrite(MOTOR_DOWN, 0); //Open Door
        digitalWrite(MOTOR_UP, 1); //TEST 
   	//Pfeil hoch 
	lcdPosition(lcd, 19, 1); //Define the position where to write the Date column,row
	lcdPutchar(lcd, 10);
	}
	if(digitalRead(DOOR_OPEN) == 1 && digitalRead(DOOR_CLOSED) == 1){//When DOOR is not in the end possition then....
		if(digitalRead(MOTOR_DOWN) == 0 && digitalRead(MOTOR_UP) == 0){//When DOOR is moving STOP
		digitalWrite(MOTOR_DOWN, 1);//STOP MOTOR
        	digitalWrite(MOTOR_UP, 1);
		}
		else{//When DOOR is STOPPED OPEN DOOR
		digitalWrite(MOTOR_DOWN, 0);//STOP MOTOR
		digitalWrite(MOTOR_UP, 0);
		//Pfeil hoch 
		lcdPosition(lcd, 19, 1); //Define the position where to write the Date column,row
		lcdPutchar(lcd, 10);
		}
	}
   }
   LIT_BUTTON_START_STOP = interrupt_time;
}
/*************************************************************/
void ISR_DOOR_CLOSED(void){
	unsigned long interrupt_time = millis();
	//If interrupts come faster than 500ms, assume it's a bounce and ignore
   	if(interrupt_time - LIT_DOOR_CLOSED > 500){
   			//Interrupt is accepted, do something!
			digitalWrite(MOTOR_DOWN, 1);//STOP MOTOR
            digitalWrite(MOTOR_UP, 1);//STOP MOTOR
            //Lösche Pfeil
            lcdPosition(lcd, 19, 3); //Define the position where to write the arrow column,row
			lcdPuts(lcd, " ");
			//Door Closed
        	lcdPosition(lcd, 19, 2); //Define the position where to write the DOOR Symbol
        	lcdPutchar(lcd, 12);
	}
	LIT_DOOR_CLOSED = interrupt_time;
}
/*************************************************************/
void ISR_DOOR_OPEN(void){
        unsigned long interrupt_time = millis();
        //If interrupts come faster than 500ms, assume it's a bounce and ignore
        if(interrupt_time - LIT_DOOR_OPEN > 500){
            //Interrupt is accepted, do something!
            digitalWrite(MOTOR_DOWN, 0);//STOP MOTOR
            digitalWrite(MOTOR_UP, 0);//STOP MOTOR
            //Lösche Pfeil
            lcdPosition(lcd, 19, 1); //Define the position where to write the Date column,row
			lcdPuts(lcd, " ");
			//Door OPEN
        	lcdPosition(lcd, 19, 2); //Define the position where to write the DOOR Symbol
        	lcdPutchar(lcd, 13);
        }
        LIT_DOOR_OPEN = interrupt_time;
}
/*************************************************************/
/*MAIN Handler                                               */
/*************************************************************/
int main(void) {
	/*Variables in Main*/
	//int lcd; actually globaly
	float LUX; //LUX Sensor Value
 	time_t timer; 
    char buffer_date[26]; //Data Buffer for DATE
    char buffer_time[26]; //Data Buffer for TIME
    //char buffer_LUX[5]; //Data Buffer for LUX Value
	struct tm* tm_info;
	    
	/* sets up the wiringPi library */
	if (wiringPiSetup () < 0) {
 	     	fprintf (stderr, "Unable to setup wiringPi: %s\n", strerror (errno));
	      	return 1;
	}
	//activate I2C HW depending on Raspberry PI HW Revision and Open I2C Port 
	//if(HwRev < 2) 	I2C_Open("/dev/i2c-0");	 // Hardware Revision 1.0
	//else		I2C_Open("/dev/i2c-1");  // Hardware Revision 2.0
	I2C_Open("/dev/i2c-1");  // Hardware Revision 2.0
	
 	/* set PIN 22/23 as Output for an LED/Motor */
 	pinMode (MOTOR_UP, OUTPUT);
 	pinMode (MOTOR_DOWN, OUTPUT);
 	
  	/*Initialize the ISR*/
	// set Pin 17/0 generate an interrupt on high-to-low transitions
	if ( wiringPiISR (BUTTON_START_STOP, INT_EDGE_FALLING, &ISR_BUTTON_START_STOP) < 0 ) {
  	    	fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno));
	     	return 1;
	}
 	// set Pin 24/5 generate an interrupt on high-to-low transitions
	// and attach ISR_DOOR_CLOSED() to the interrupt
	if ( wiringPiISR (DOOR_CLOSED, INT_EDGE_FALLING, &ISR_DOOR_CLOSED) < 0 ) {
      		fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno));
      		return 1;
 	}
  	// set Pin 25/6 generate an interrupt on high-to-low transitions
  	// and attach ISR_DOOR_OPEN() to the interrupt
	if ( wiringPiISR (DOOR_OPEN, INT_EDGE_FALLING, &ISR_DOOR_OPEN) < 0 ) {
      		fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno));
      		return 1;
	}
	/*Initialize the LCD*/
  	    // Lines, rows, 4bit mode, RS, E, DB0, DB1, DB2, DB3, DB4, DB5, DB6, DB7)
    lcd = lcdInit (4, 20, 4, LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7, 0, 0, 0, 0);
    lcdClear(lcd); //Clear the Display
    lcdPosition(lcd, 0, 0);
    lcdPuts(lcd,"Chickenfarm");//Put later on the time behind
	/*Load Display with Custom Characters*/
	lcdCharDef(lcd, 10, Arrow_up);
	lcdCharDef(lcd, 11, Arrow_down);
	lcdCharDef(lcd, 12, Door_closed);
	lcdCharDef(lcd, 13, Door_open);
	lcdCharDef(lcd, 14, drop);
	
	//Configure MAX44009	
	//I2C Adress of the device of which we want to speak
	I2C_Setup(I2C_SLAVE, MAX44009_ADRESS);
	// Error handling output depends on failure
	if(I2cError){	
		I2C_PrintError();
		exit(1);
	}
	/* Write default values to the display */
	/*Check Status of DOOR for the first Initialisation of the Display*/
  	if(digitalRead(DOOR_OPEN) == 0){//When DOOR is open then.....
	   	lcdPosition(lcd, 19, 2); //Define the position where to write the DOOR Symbol
       	lcdPutchar(lcd, 13);
  	}	
  	if(digitalRead(DOOR_CLOSED) == 0){//When DOOR is closed then....
  	   	lcdPosition(lcd, 19, 2); //Define the position where to write the DOOR Symbol
       	lcdPutchar(lcd, 12);
  	}
  	lcdPosition(lcd, 0, 1); //Define the position where to write the LUX column,row
	lcdPuts(lcd,"Helligkeit:"); //Write default modus for Lux Value	
	while ( 1 ) {
  		
	    time(&timer);
        tm_info = localtime(&timer); //write Local time to tm_info
		//prepare Buffer to send time and date info to the Display
	    strftime(buffer_date, 26, "Date: %m:%d:%Y", tm_info); //strftime(buffer_date, 26, "Date: %m:%d:%Y", tm_info);
	    strftime(buffer_time, 26, "%H:%M", tm_info); //strftime(buffer_time, 26, "Time: %H:%M:%S", tm_info);

        lcdPosition(lcd, 0, 2); //Define the position where to write the Date column,row
	    lcdPuts(lcd, buffer_date); //Send Date to the Display

	    lcdPosition(lcd, 15, 0); //Define the position where to write the Time column,row
	    lcdPuts(lcd, buffer_time); //Send the time to the Display
  		
  		//Read the Luminosity of MAX44009
		LUX=MAX44009getLUX();
		//LUX = (float)((int)(LUX*100))/100; //LUX Wert reduction to 2 digits after .
		lcdPosition(lcd, 11, 1); //Define the position where to write the LUX column,row
		lcdPrintf(lcd,"%.1fLX ",LUX); //Send LUX Value to the Display
		
  		delay( 1000 ); // wait 1 second
	}
	//Close I2C Port
	I2C_Close();
	return 0;
}

