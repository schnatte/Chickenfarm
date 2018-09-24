/*
isr4pi.c
D. Thiebaut
based on isr.c from the WiringPi library, authored by Gordon Henderson
https://github.com/WiringPi/WiringPi/blob/master/examples/isr.c

Compile as follows:

    gcc -o isr4pi isr4pi.c -lwiringPi

Run as follows:

    sudo ./isr4pi

 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <time.h>


/***********************************************************/
/*Set-up GPIO Pins for wiringPi libary                     */
/***********************************************************/
// Use GPIO Pin 17, which is Pin 0 for wiringPi library
#define BUTTON_START_STOP 0 //START-STOP Switch
// Use GPIO Pin 22,23 for Motor Control which is Pin 3,4 for wiringPi libary
#define MOTOR_UP 3  //Motor up
#define MOTOR_DOWN 4 //Motor down
// Use GPIO Pin 24,25 for DOOR end switch, which is Pin 5,6 for wiringPi libary
#define DOOR_CLOSED 5 //Door closed
#define DOOR_OPEN 6 //Door open



/************************************************************/
/*IRQ Handler                                               */
/************************************************************/

/************************************************************/
unsigned int LIT_BUTTON_START_STOP = 0; //LIT = Last Interrupt time
unsigned int LIT_DOOR_CLOSED = 0; //LIT = Last Interrupt time
unsigned int LIT_DOOR_OPEN = 0; //LIT = Last Interrupt time
/************************************************************/
// ISR_BUTTON_START_STOP:  called every time an event occurs when Button will be pushed
void ISR_BUTTON_START_STOP(void) {
   unsigned long interrupt_time = millis();
   //If interrupts come faster than 200ms, assume it's a bounce and ignore
   if(interrupt_time - LIT_BUTTON_START_STOP > 200){
   //Interrupt is accepted, do something!
   //Check Status of the different Switches
   	if(digitalRead(DOOR_OPEN) == 0){
   	digitalWrite(MOTOR_DOWN, 1); //Close Door
   	digitalWrite(MOTOR_UP, 0); //TEST
	}
   	if(digitalRead(DOOR_CLOSED) == 0){
   	digitalWrite(MOTOR_DOWN, 0); //Open Door
        digitalWrite(MOTOR_UP, 1); //TEST 
   	}
	if(digitalRead(DOOR_OPEN) == 1 && digitalRead(DOOR_CLOSED) == 1){
		if(digitalRead(MOTOR_DOWN) == 0 && digitalRead(MOTOR_UP) == 0){
		digitalWrite(MOTOR_DOWN, 1);//STOP MOTOR
        	digitalWrite(MOTOR_UP, 1);
		}
		else{
		digitalWrite(MOTOR_DOWN, 0);//STOP MOTOR
		digitalWrite(MOTOR_UP, 0);
		}
	}
   }
   LIT_BUTTON_START_STOP = interrupt_time;
}
/************************************************************/
void ISR_DOOR_CLOSED(void){
	unsigned long interrupt_time = millis();
	//If interrupts come faster than 500ms, assume it's a bounce and ignore
   	if(interrupt_time - LIT_DOOR_CLOSED > 500){
   		//Interrupt is accepted, do something!
		digitalWrite(MOTOR_DOWN, 1);//STOP MOTOR
                digitalWrite(MOTOR_UP, 1);//STOP MOTOR
	}
	LIT_DOOR_CLOSED = interrupt_time;
}
/************************************************************/
void ISR_DOOR_OPEN(void){
        unsigned long interrupt_time = millis();
        //If interrupts come faster than 500ms, assume it's a bounce and ignore
        if(interrupt_time - LIT_DOOR_OPEN > 500){
                //Interrupt is accepted, do something!
                digitalWrite(MOTOR_DOWN, 0);//STOP MOTOR
                digitalWrite(MOTOR_UP, 0);//STOP MOTOR
        }
        LIT_DOOR_OPEN = interrupt_time;
}
/************************************************************/
/*MAIN Handler                                              */
/************************************************************/
int main(void) {
  	// sets up the wiringPi library
  	if (wiringPiSetup () < 0) {
      	fprintf (stderr, "Unable to setup wiringPi: %s\n", strerror (errno));
      	return 1;
  	}
  	// set PIN 22/23 as Output for an LED/Motor
  	pinMode (MOTOR_UP, OUTPUT);
  	pinMode (MOTOR_DOWN, OUTPUT);
  	
  	// set Pin 17/0 generate an interrupt on high-to-low transitions
  	// and attach ISR_BUTTON_START_STOP() to the interrupt
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
  	// display counter value every second.
  	while ( 1 ) {
    		//printf( "%d\n", eventCounter );
    		//eventCounter = 0;
    		//digitalWrite(LED,0); //LED OFF
    		delay( 1000 ); // wait 1 second
  	}
	return 0;
}

