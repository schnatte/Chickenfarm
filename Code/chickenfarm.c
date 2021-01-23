/***********************************************************/
/*Chickenfarm                                              */
/*---------------------------------------------------------*/
/*Author: Daniel Kettnaker                                 */
/*                                                         */
/***********************************************************/
//scp /Users/danielkettnaker/Projekte/ChickenFarm/actual/* pi@192.168.0.141:/chickenfarm/
/***********************************************************/


/***********************************************************/
/*INCLUDES                                                 */
/***********************************************************/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <wiringPi.h>	//Needed for Port control
#include <time.h>		//Needed for Time Calculations
#include <lcd.h>		//Needed for LCD Display

#include <unistd.h>			//Needed for I2C port
#include <fcntl.h>			//Needed for I2C port
#include <sys/ioctl.h>		//Needed for I2C port
#include <linux/i2c-dev.h>	//Needed for I2C port
#include <wiringPiI2C.h>	//Needed for I2C port
#include <pcf8574.h>		//Needed for I2C Port Expander PCF8574

#include <sys/types.h>	//Needed to get IP Adress
#include <sys/socket.h> //Needed to get IP Adress
#include <netinet/in.h>	//Needed to get IP Adress
#include <net/if.h>		//Needed to get IP Adress
#include <arpa/inet.h>	//Needed to get IP Adress

#include <curl/curl.h>	//Needed to upload to WEB

#include "max44009.h"
#include "i2c.h"
#include "bme280.h"

/***********************************************************/
/*Function Declarations                                    */
/***********************************************************/
//char GETDATE();
void GETIP(char *outputBuffer);
void SETUP();
float minimum(float a, float min);
float maximum(float a, float max);
float mittelwert(float a, float rms);
void FILLSTRING(char *outputBuffer);
void FILLSTRINGTOPOSITION(char *outputBuffer, int position);
void SendTelegramText(char *text);
void TimeCalculation(int seconds, char *output);
int ReadFileContentInt(char *filename);
void SaveFileContentChar(char *filename, char *content);
void SaveFileContentInt(char *filename, int content);
void SaveFileContentOneFloat(char *filename, char *unit, float content);
void SaveFileContentTwoFloat(char *filename, char *unit1, char *unit2, float content1, float content2);
void uploadtoEMONCMS(char *json);

/***********************************************************/
/*Set-up GPIO Pins for wiringPi libary                     */
/***********************************************************/
//Set-up the PINS for the Switches
#define BUTTON_START_STOP 0 //START-STOP Switch, GPIO17
#define DOOR_CLOSED 5 //Door closed, GPIO24
#define DOOR_OPEN 6 //Door open, GPIO25
#define ENCODER_A 1 //Rotary Encoder PIN
#define ENCODER_B 2 //Rotary Encoder PIN
#define POWER_ON_OFF 7//Detection if 12V is alive
//#define EGG_COUNTER //Photoelectric barrier for egg counter
/************************************************************/
//Set-up the PINS for the Display use wiringPi numbers
#define LCD_RS  10 //Register select pin grün
#define LCD_E   11 //Enable Pin gelb
#define LCD_D4  12 //Data pin 4
#define LCD_D5  13 //Data pin 5
#define LCD_D6  14 //Data pin 6
#define LCD_D7  3  //Data pin 7
/************************************************************/
//Other Definitions
#define SHORT_LONG 1500 //define time difference short long button preasure in ms
#define BOUNCING 300 //define bouncing time in ms

#define MAXMENUE 7//Define max items for Main Menue first number is a 0

//Time definitions for Display Backlight
#define BACKLIGHT_MAX_TIME 60 //define Backlight duration after last interaction in seconds

/***********************************************************/
/*Define Relais PinBase Adress                             */
/***********************************************************/
#define K1 100 //Relais 1 Motor
#define K2 101 //Relais 2 Motor
#define K3 102 //Relais 3 Display Backlight
#define K4 103 //Relais 4
#define K5 104 //Relais 5
#define K6 105 //Relais 6
#define K7 106 //Relais 7 Relais Umschalter 12V Netzteil 12V Boost
#define K8 107 //Relais 8 Relais Booster ON für Motor

/***********************************************************/
/*EmonCMS Account Information                              */
/***********************************************************/
volatile int giIOTActiveStatus = 0; //Set EmonCMS active to send information
volatile int giIOTActiveStatus_old = 1;
volatile int giUpdateSequence = 180;//emon update sequence in Seconds
volatile int giUpdateSequence_old = 100;

#define EmonCMS_HOST "192.168.0.119"
#define EmonCMS_NODE "ChickenFarm"
#define EmonCMS_APIKey "60ab9904683c455cd2230ac5a7aa0f60"

/***********************************************************/
/*Telegram Account                                         */
/***********************************************************/
volatile int giTelegramActiveStatus = 0; //Set Telegram active to send messages
volatile int giTelegramActiveStatus_old = 1;
volatile int giTelegramFlag = 0; //Set Flag to 1 to send message otherwise reset to avoid continiously sending
#define CHATID 475180895 //Chat ID Empfänger
#define TOKEN "465754748:AAFg51b8EJSuCIeH3Tejiph76FzbMs1mcDA"

/***********************************************************/
/* Global Variables                                        */
/***********************************************************/
volatile unsigned short int uiEncoderFlag = 0; //Status Variable
volatile int iEncoderDirectionFlag = 0; //Status Variable
volatile int iENCODER = 0; //STATUS for Main Menue
//volatile int iLongButtonPreasure = 0; //Status Variable
volatile int iValueChangeActive = 0; //Status Variable
volatile int iSUBMENUE = 1; //Status Variable
volatile int giPOWER_ON_OFF = 0; //Status Variable 0 = Disable  1 = Active Alarm
volatile int giEGG = 0; //Status Variable
volatile int giButtonPushFlag = 0; //Button pushed Flag;  1 = Button pushed active

volatile int RMS = 300; //RMS Value over x measurement values
volatile int x = 1;// help variable for RMS calculation

time_t gNow; //Variable for actual time
time_t gBacklightTime; //Variable to count Backlight time
time_t gPowerOFFTime; //Variable to count Power Off Time
time_t gPowerONTime; //Variable to count Power ON Time
/************************************************************/
/*IRQ Handler                                               */
/************************************************************/
//Variables for IRQ Handler to control bouncing
volatile unsigned int LIT_BUTTON_START_STOP = 0; //LIT = Last Interrupt time
volatile unsigned int LIT_PUSH_TIME = 0; //LIT = Last Interrupt time
volatile unsigned int LIT_ENCODER = 0; //LIT = Last Interrupt time
volatile unsigned int LIT_POWER_CONTROL = 0; //LIT = Last Interrupt time
volatile unsigned int LIT_EGG_COUNTER = 0; //LIT = Last Interrupt time
/************************************************************/
void ISR_BUTTON_START_STOP(void){
	unsigned long Interrupt_Time_ISR_BUTTON_START_STOP = millis();//Bouncing check
	//If interrupts come faster than xxxms, assume it's a bounce and ignore
	if((Interrupt_Time_ISR_BUTTON_START_STOP - LIT_BUTTON_START_STOP) > BOUNCING){
		if(digitalRead(BUTTON_START_STOP) == 0){
		//Interrupt is accepted, do something!
		digitalWrite(K3, 0);//Switch on Blacklight Relais
		gBacklightTime = gNow;//Reset Backlight
		giTelegramFlag = 1; //Send Flag to push message send
		giButtonPushFlag = 1; //Set Flag to initiate move of DOOR
		}
	}
	LIT_BUTTON_START_STOP = Interrupt_Time_ISR_BUTTON_START_STOP;
}
/************************************************************/
void ISR_ENCODER(void){
	/* Check IRQ 10 - Encoder; Interrupt PIN1, IO PIN 2 */
	/******************************************************/
	/*Description of the Encoderfunction:                 */
	/* Position    1 _2  3 __    __    __                 */
	/* A          __|  |__|  |__|  |__|                   */
	/*             __    __    __    __                   */
	/* B          |  |__|  |__|  |__|  |_                 */
	/*                    _     _                         */
	/* Edge detection:  _| =0    |_=1                     */
	/*                                                    */
	/* Pos1 A=0 / B=1; Edge 0 => A-Level                  */
	/*                                                    */
	/* Pos2 A=1 / B=0; Edge 1 => A-Level                  */
	/*                                                    */
	/* Pos3 A=0 / B=1; Edge 0 => A-Level                  */
	/******************************************************/
	unsigned long interrupt_time = millis();
	//If interrupts come faster than 200ms, assume it's a bounce and ignore
	if(interrupt_time - LIT_ENCODER > BOUNCING){
		//Interrupt is accepted, do something!
		digitalWrite(K3, 0);//Switch on Blacklight Relais
		gBacklightTime = gNow;//Reset Backlight
		//short int A_old, A_actual;
		short int B_old, B_actual;
		int flag = 0;

		/*Encoder Flag*/
		uiEncoderFlag = 1;

		B_old = digitalRead(ENCODER_B); //Read PIN 2
		while(!digitalRead(ENCODER_A)){
			B_actual = digitalRead(ENCODER_B);
			flag = 1;
		}
		if(flag == 1){
			flag = 0;
			if((B_old == 0)&&(B_actual == 1)){//Encoder count up
				iEncoderDirectionFlag++;
				if(iENCODER == MAXMENUE){//Limit Max value for Main Menue
					iENCODER = 0;
				}
				else{
					iENCODER++;
				}
			}
			if((B_old == 1)&&(B_actual == 0)){//Encoder count down
				iEncoderDirectionFlag--;
				if(iENCODER == 0){//Limit Max value for Main Menue
					iENCODER = MAXMENUE;
				}
				else{
					iENCODER--;
				}
			}
		}
	}
}
/************************************************************/
void ISR_POWER_CONTROL(void){
	unsigned long interrupt_time = millis();
	//If interrupts come faster than 200ms, assume it's a bounce and ignore
	if(interrupt_time - LIT_POWER_CONTROL > BOUNCING){
			//Interrupt is accepted, do something!
		if(giPOWER_ON_OFF == 1){
			//Alarm was already set check if Alarm is off
			//Check Status of PIN 3V3 = Power OK / 0V = Power NOK
			if(digitalRead(POWER_ON_OFF) == 1){
					giPOWER_ON_OFF = 0;//Disable Alarm
					printf("Starting of gPowerONTime...\n");
	   			time(&gPowerONTime);
				}
		}
		if(giPOWER_ON_OFF == 0){
			//Alarm was off check if Alarm is set
			//Check Status of PIN 3V3 = Power OK / 0V = Power NOK
			if(digitalRead(POWER_ON_OFF) == 0){
				giPOWER_ON_OFF = 1;//Show alarm
				printf("Starting of gPowerOFFTime...\n");
   			time(&gPowerOFFTime);
			}
		}
	}
	LIT_POWER_CONTROL = interrupt_time;
}
/************************************************************/
void ISR_EGG_COUNTER(void){
	unsigned long interrupt_time = millis();
	//If interrupts come faster than 200ms, assume it's a bounce and ignore
	if(interrupt_time - LIT_POWER_CONTROL > 1000){//1000ms bouncing
			//Interrupt is accepted, do something!
			giEGG = 1; //Set flag for Main loop
	}
	LIT_EGG_COUNTER = interrupt_time;
}
/************************************************************/
/*MAIN Handler                                              */
/************************************************************/
int main() {
	/*Variables in Main*/
	/*MAX44009*/
	float fLUX = 0;
	float fLUX_old = 0;
	float fLUX_MIN = 10000;
	float fLUX_MAX = 0;
	float fLUX_RMS = 0;  //Lux Sensor Value RMS LUX_RMS_CALC = 0;
	int n = 1;
	//float a[RMS]; //Gleittender mittelwert über x Messungen
	char cLUX_MIN[20];
	char cLUX_MAX[20];
	char cLUX_RMS[20];
	int iTRESHOLD_LUX = 10; //default threshold value
	int iTRESHOLD_LUX_old = 8;
	int iHYSTERESIS_LUX = 6; //default hysteresis value
	int iHYSTERESIS_LUX_old = 6;
	char cTRESHOLD_LUX[20];
	char cHYSTERESIS_LUX[20];
	/*BME280 GLOBAL*/
	int fd;
	int32_t t_fine;
	bme280_calib_data cal;
	bme280_raw_data raw;
	float fALT = 0;
	char cALT[20];
	float fPRESS = 0;
	float fPRESS_MIN = 10000;
	float fPRESS_MAX = 0;
	char cPRESS[20];
	char cPRESS_MIN[20];
	char cPRESS_MAX[20];
	/*BME280 INTERN*/
	float fTEMP_INTERN = 0;
	float fTEMP_MIN_INTERN = 100;
	float fTEMP_MAX_INTERN = 0;
	float fHUM_INTERN = 0;
	float fHUM_MIN_INTERN = 100;
	float fHUM_MAX_INTERN = 0;
	float fPRESS_INTERN = 0;
	float fPRESS_MIN_INTERN = 10000;
	float fPRESS_MAX_INTERN = 0;
	float fALT_INTERN = 0;
	char cTEMP_INTERN[15];
	char cTEMP_MIN_INTERN[15];
	char cTEMP_MAX_INTERN[15];
	char cHUM_INTERN[15];
	char cHUM_MIN_INTERN[15];
	char cHUM_MAX_INTERN[15];
	int iTRESHOLD_TEMP_INTERN = 1; //default threshold value
	int iHYSTERESIS_TEMP_INTERN = 5; //default hysteresis value
	int iTRESHOLD_TEMP_INTERN_old = 1;
	int iHYSTERESIS_TEMP_INTERN_old = 5;
	int iTEMP_ALARM = 0;//Set Alarm off
	char cTRESHOLD_TEMP_INTERN[20];
	char cHYSTERESIS_TEMP_INTERN[20];

	/*BME280 EXTERN*/
	float fTEMP_EXTERN = 0;
	float fTEMP_MIN_EXTERN = 100;
	float fTEMP_MAX_EXTERN = 0;
	float fHUM_EXTERN = 0;
	float fHUM_MIN_EXTERN = 100;
	float fHUM_MAX_EXTERN = 0;
	float fPRESS_EXTERN = 0;
	float fPRESS_MIN_EXTERN = 10000;
	float fPRESS_MAX_EXTERN = 0;
	float fALT_EXTERN = 0;
	char cTEMP_EXTERN[20];
	char cTEMP_MIN_EXTERN[20];
	char cTEMP_MAX_EXTERN[20];
	char cHUM_EXTERN[20];
	char cHUM_MIN_EXTERN[20];
	char cHUM_MAX_EXTERN[20];

	/*DATE & TIME*/
	// time_t timer; //Timer for Clock Time
	char buffer_date[26]; //Data Buffer for DATE
	char buffer_time[26]; //Data Buffer for TIME
	struct tm* tm_info;
	time_t timeold; //Timer for save every minute
	time(&timeold); //initialize time reference
	time_t TempAlarmTimeStart; //Timer to count low Temp Time
	time_t TempAlarmTimeEnd; //Timer to count low Temp Time
	time_t TempAlarmTimeDifference; //Timer to count low Temp Time difference
	time(&TempAlarmTimeStart); //initialize time reference
	time(&TempAlarmTimeEnd); //initialize time reference
	time(&TempAlarmTimeDifference); //initialize time reference
	char cTempAlarmTime[80]; //Char for Saving value
	double diff_t = 0; //Variable to store delta Time

	int RUNNING_MINUTES = 0;
	int RUNNING_HOURS = 0;
	int RUNNING_DAYS = 0;

	#define TOGGLETIME 3//define duration in seconds
	time_t toggletimeold;//Timer for Toggletime
	time(&toggletimeold); //initialize time reference
	int iToggle = 0;
	/*DOOR */
	time_t tDoorMove; //Timer for Toggletime
	time(&tDoorMove); //initialize time reference
	time_t tDoorMoveDifference; //Timer to count difference of DoorMove
	time(&tDoorMoveDifference); //initialize time reference
	int  iDOORMOVETIME = 13; //define max Door move time before send error in s
	int iDOORMOVETIME_old = 13;
	char cDOORMOVETIME[20];
	char STATE = 'C'; //State machine for automatic movements
	int iDoor_Modus = 1; //0 = Manual Modus, 1 = Automatic Modus
	int iDoor_Modus_old = 0;
	bool DOORAlarm = FALSE;

	/*EGG Counter*/
	int iEGG_COUNTER = 0;
	char cEGG_COUNTER[20];

	/*USV AKKU*/
	#define AKKUGROESSE 26//in Ah
	#define VERBRAUCH 2//5V Verbrauch in A
	#define LIMIT_ABSCHALTUNG 10//lower then x% shut down
	#define AKKULADESTROM 3//Charging Current in A
	int iAkkulaufzeit;
	int iAkkuladezeit;
	float fAKKU_STATUS;
	fAKKU_STATUS = 100;//alternative bis zur Implementierung
	char cAKKU_STATUS[20];
	char cAKKUGROESSE[20];
	char cVERBRAUCH[20];
	char cLIMIT_ABSCHALTUNG[20];
	char cAKKULADESTROM[20];

	/*Boolean for Telegram*/
	bool LuxDark = TRUE;
	bool LuxBright = TRUE;
	bool InternalTempLow = TRUE;
	bool InternalTempOK = TRUE;
	bool PowerAlarm = TRUE;

	/*Data Buffer for EmonCMS*/
	char json[256];

	/*DISPLAY BUFFER*/
	int lcd;
	char buffer_FIRST_LINE[21]; //Data Buffer for first Line
	char buffer_SECOND_LINE[21]; //Data Buffer for second Line
	char buffer_THIRD_LINE[21]; //Data Buffer for third Line
	char buffer_FOURTH_LINE[21]; //Data Buffer for fourth Line

	char IP_ADRESS[20]; //Data Buffer for the IP Adress

	SETUP(); //Set Port and activations

	GETIP(IP_ADRESS); //write IP Adress in char IP_ADRESS

	// Lines, rows, 4bit mode, RS, E, DB0, DB1, DB2, DB3, DB4, DB5, DB6, DB7)
	lcd = lcdInit (4, 20, 4, LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7, 0, 0, 0, 0);

	/******************************************************************/
	/*STARTUP SCREEN                                                  */
	/******************************************************************/
	digitalWrite(K3, 0);//Switch on Blacklight Relais
	lcdHome(lcd);//goto first curser position
	lcdPuts(lcd,"********************");//write 1st line
	lcdPuts(lcd,"*  Chickenfarm by  *");//write 2nd line
	lcdPuts(lcd,"* Daniel Kettnaker *");//write 3rd line
	lcdPuts(lcd,"********************");//write 4th line
	delay(4000); //show for 4seconds
	digitalWrite(K3, 1);//Switch off Blacklight Relais

	/******************************************************************/
	/* WHILE LOOP in Main                                             */
	/******************************************************************/
	while (1){
   time_t rawtime;
   time(&rawtime);
	 tm_info = localtime( &rawtime );
	 time(&gNow);//init now time

		//prepare Buffer to send time and date info to the Display
		strftime(buffer_date, 26, "Datum: %d:%m:%Y", tm_info); //strftime(buffer_date, 26, "Date: %m:%d:%Y", tm_info);
		strftime(buffer_time, 26, "%H:%M", tm_info); //strftime(buffer_time, 26, "Time: %H:%M:%S", tm_info);

		/******************************************************************/
		/* Read values from File otherwise take default                   */
		/* - Treshold (in C degrees)                                      */
		/* - Hysteresis (in C degrees)                                    */
		/* - Door Modus (1 = active; 0 = inactive)                        */
		/* - DOORMOVETIME (in ms)                                         */
		/* - Telegram Status (1 = active; 0 = inactive)                   */
		/* - UpdateSequence (time in seconds)                             */
		/* -                                                              */
		/******************************************************************/
		iTRESHOLD_LUX = ReadFileContentInt("/var/www/html/chickenfarm/config/LUXthreshold.txt");
		iHYSTERESIS_LUX = ReadFileContentInt("/var/www/html/chickenfarm/config/LUXhysteresis.txt");
		iTRESHOLD_TEMP_INTERN = ReadFileContentInt("/var/www/html/chickenfarm/config/TEMPthreshold.txt");
		iHYSTERESIS_TEMP_INTERN = ReadFileContentInt("/var/www/html/chickenfarm/config/TEMPhysteresis.txt");
		iDoor_Modus = ReadFileContentInt("/var/www/html/chickenfarm/config/Door_Modus.txt");
		iDOORMOVETIME = ReadFileContentInt("/var/www/html/chickenfarm/config/DOORMOVETIME.txt");
		giTelegramActiveStatus = ReadFileContentInt("/var/www/html/chickenfarm/config/TelegramStatus.txt");
		giIOTActiveStatus = ReadFileContentInt("/var/www/html/chickenfarm/config/IOTStatus.txt");
		giUpdateSequence = ReadFileContentInt("/var/www/html/chickenfarm/config/UpdateSequence.txt");
		/******************************************************************/
		/*Check if the file content has changes and send a Telegram*/
		if(giTelegramActiveStatus){
			char cText[20];
			/*iTRESHOLD_LUX*/
			if (iTRESHOLD_LUX != iTRESHOLD_LUX_old){
				sprintf(cText, "%d Lux", iTRESHOLD_LUX);
				SendTelegramText("iTRESHOLD_LUX geändert. Neuer Wert:");
				SendTelegramText(cText);
				iTRESHOLD_LUX_old = iTRESHOLD_LUX;
			}
			/*iHYSTERESIS_LUX*/
			if (iHYSTERESIS_LUX != iHYSTERESIS_LUX_old){
				sprintf(cText, "%d Lux", iHYSTERESIS_LUX);
				SendTelegramText("iHYSTERESIS_LUX geändert. Neuer Wert:");
				SendTelegramText(cText);
				iHYSTERESIS_LUX_old = iHYSTERESIS_LUX;
			}
			/*iTRESHOLD_TEMP_INTERN*/
			if (iTRESHOLD_TEMP_INTERN != iTRESHOLD_TEMP_INTERN_old){
				sprintf(cText, "%d °C", iTRESHOLD_TEMP_INTERN);
				SendTelegramText("iTRESHOLD_TEMP_INTERN geändert. Neuer Wert:");
				SendTelegramText(cText);
				iTRESHOLD_TEMP_INTERN_old = iTRESHOLD_TEMP_INTERN;
			}
			/*iHYSTERESIS_TEMP_INTERN*/
			if (iHYSTERESIS_TEMP_INTERN != iHYSTERESIS_TEMP_INTERN_old){
				sprintf(cText, "%d °C", iHYSTERESIS_TEMP_INTERN);
				SendTelegramText("iHYSTERESIS_TEMP_INTERN geändert. Neuer Wert:");
				SendTelegramText(cText);
				iHYSTERESIS_TEMP_INTERN_old = iHYSTERESIS_TEMP_INTERN;
			}
			/*iDoor_Modus*/
			if (iDoor_Modus != iDoor_Modus_old){
				sprintf(cText, "%d", iDoor_Modus);
				SendTelegramText("iDoor_Modus geändert. Neuer Wert:");
				SendTelegramText(cText);
				iDoor_Modus_old = iDoor_Modus;
			}
			/*iDOORMOVETIME*/
			if(iDOORMOVETIME != iDOORMOVETIME_old){
				sprintf(cText, "%d s", iDOORMOVETIME);
				SendTelegramText("iDOORMOVETIME geändert. Neuer Wert:");
				SendTelegramText(cText);
				iDOORMOVETIME_old = iDOORMOVETIME;
			}
			/*giTelegramActiveStatus*/
			if (giTelegramActiveStatus != giTelegramActiveStatus_old){
				sprintf(cText, "%d", giTelegramActiveStatus);
				SendTelegramText("giTelegramActiveStatus geändert. Neuer Wert:");
				SendTelegramText(cText);
				giTelegramActiveStatus_old = giTelegramActiveStatus;
			}
			/*giIOTActiveStatus*/
			if (giIOTActiveStatus != giIOTActiveStatus_old){
				sprintf(cText, "%d", giIOTActiveStatus);
				SendTelegramText("giIOTActiveStatus geändert. Neuer Wert:");
				SendTelegramText(cText);
				giIOTActiveStatus_old = giIOTActiveStatus;
			}
			/*UpdateSequence*/
			if (giUpdateSequence != giUpdateSequence_old){
				sprintf(cText, "%d", giUpdateSequence);
				SendTelegramText("UpdateSequence geändert. Neuer Wert:");
				SendTelegramText(cText);
				giUpdateSequence_old = giUpdateSequence;
			}
		}
		/******************************************************************/
		/*Read Lux Value from Sensor and provide information to different strings*/
		fLUX = MAX44009getLUX();//Read the Luminosity of MAX44009
		fLUX_MIN = minimum(fLUX_MIN, fLUX);
		fLUX_MAX = maximum(fLUX_MAX, fLUX);

		if(n<512){
			fLUX_RMS = ((n-1)*fLUX_old+fLUX)/n;
			fLUX_old = fLUX_RMS;
			n++;
		}
		else{
			fLUX_RMS = ((n-1)*fLUX_old+fLUX)/n;
			fLUX_old = fLUX_RMS;
		}
		//fLUX_RMS = mittelwert(fLUX_RMS, fLUX);//Average over 200 measurements values
		/*Convert float into char for LCD*/
		sprintf(cLUX_MIN, "Min: %0.1fLX", fLUX_MIN);
		sprintf(cLUX_MAX, "Max: %0.1fLX", fLUX_MAX);
		sprintf(cLUX_RMS, "Typ: %0.1fLX", fLUX_RMS);
		/******************************************************************/
		/*Decide if the Motor needs to move based on LUX value*/
		if(iDoor_Modus == 1){//Automatic Modus active
			if(fLUX_RMS >= (iTRESHOLD_LUX + iHYSTERESIS_LUX)){//Move up door
				if(LuxBright){//Check only once in the while otherwise endless Telegram will be send.
					LuxBright = FALSE;//reset Flag
					LuxDark = TRUE; //set Flag
					giTelegramFlag = 1; //set Flag
					if(digitalRead(DOOR_OPEN) == 0){//When DOOR is open then do nothing all fine
						STATE='C';//check only Door Status
					}
					if(digitalRead(DOOR_CLOSED) == 0){//When DOOR is closed then... open DOOR
						STATE='B';//STATE definition Door move up!
						if(giTelegramActiveStatus){
							SendTelegramText("Tuer geht auf! (Automatic)");
							SendTelegramText(cLUX_RMS);
						}
						else{
							printf("Tuer geht auf! (Automatic)\n");
						}
					}
					if(digitalRead(DOOR_CLOSED) == 1 && digitalRead(DOOR_OPEN) == 1){//When DOOR is inbetween then
						STATE='B';//STATE definition Door move up!
						if(giTelegramActiveStatus){
							SendTelegramText("Tuer geht auf! Zustand war nicht klar. (Automatic)");
							SendTelegramText(cLUX_RMS);
						}
						else{
							printf("Tuer geht auf! Zustand war nicht klar. (Automatic)\n");
						}
					}
				}
			}
			if(fLUX_RMS <= iTRESHOLD_LUX){//Move down
				if(LuxDark){//Check only once in the while otherwise endless Telegram will be send.
					LuxDark = FALSE;//reset Flag
					LuxBright = TRUE; //set Flag
					giTelegramFlag = 1; //set Flag
					if(digitalRead(DOOR_OPEN) == 0){//When DOOR is open then...close DOOR
						STATE='A';//STATE definition close DOOR
						if(giTelegramActiveStatus){
							SendTelegramText("Tuer geht zu! (Automatic)");
							SendTelegramText(cLUX_RMS);
						}
						else{
							printf("Tuer geht zu! (Automatic)\n");
						}
					}
					if(digitalRead(DOOR_CLOSED) == 0){//When DOOR is closed then do nothing all fine
						STATE='C';//check only Door Status
					}
					if(digitalRead(DOOR_CLOSED) == 1 && digitalRead(DOOR_OPEN) == 1){//When DOOR is inbetween then
						STATE='A';//STATE definition close DOOR
						if(giTelegramActiveStatus){
							SendTelegramText("Tuer geht zu! Zustand war nicht klar. (Automatic)");
							SendTelegramText(cLUX_RMS);
						}
						else{
							printf("Tuer geht zu! Zustand war nicht klar. (Automatic)\n");
						}
					}
				}
			}
		}
		/******************************************************************/
		//Check Power OFF and Switch ON Booster and switch to 12V Booster
		if(giPOWER_ON_OFF == 1){//Alarm active
			digitalWrite(K7, 0);//Switch to 12V Booster
			digitalWrite(K8, 0);//Switch Booster ON
		}
		/*State for DOOR*/
		switch(STATE){
			/******************************************************************/
			/*Statemachine for DOOR Status                                    */
			/*case 'A'://STATE definition close DOOR                          */
			/*case 'B'://STATE definition open DOOR                           */
			/*case 'C'://check only Door Status                               */
			/******************************************************************/
			/*Endschalter sind unzuverlässig und Programm hängt im while.
				Fehlermeldung kann zur Blockade der Tür führen. Ein Linearmotor
				ist eine Zuverlässige Zwangsführung mit eingebauten Endschaltern
				auf welche man aber keinen Zugriff hat.

				Neue Idee:
				Der Motor wird gestartet und eine Zeit läuft, das ist die normal benötigte
				Wegezeit. Nach Ablauf der Zeit wird das Relais gestoppt.
				Nach der Zeit werden die Endschalter abgefragt und kommuniziert (Kontrolle)
				Der Manual Modus wird nie aktiviert.

				tDoorMove  = gNow; //start time measurement
				//Move Door START MOTOR (close)
				digitalWrite(K1, 1);
				digitalWrite(K2, 0);
				time(&gNow);//init now time
				tDoorMoveDifference = difftime(gNow, tDoorMove);//Check delta time in Seconds
				if(tDoorMoveDifference >= iDOORMOVETIME){
					//STOP MOTOR
					digitalWrite(K1, 1);
					digitalWrite(K2, 1);

					if(digitalRead(DOOR_CLOSED) == 1){//Check end contact
						if(giTelegramFlag == 1){
							if(giTelegramActiveStatus){
								SendTelegramText("Tuer zu! (Automatic)");
							}
							else{
								printf("Tuer zu (Automatic)\n");
							}
						giTelegramFlag = 0; //Reset Flag
						}
					}
					else{//end contact failure
						if(giTelegramFlag == 1){
							if(giTelegramActiveStatus){
								SendTelegramText("Tuer zu! (Automatic) ALARM: Endschalter nicht erkannt");
							}
							else{
								printf("Tuer zu (Automatic) ALARM: Endschalter nicht erkannt\n");
							}
						giTelegramFlag = 0; //Reset Flag
						}
					}
			}
			*/
			case 'A'://STATE definition close DOOR
				tDoorMove  = gNow; //start time measurement
				//Move Door START MOTOR (close)
				digitalWrite(K1, 1);
				digitalWrite(K2, 0);
				time(&gNow);//init now time
				tDoorMoveDifference = difftime(gNow, tDoorMove);//Check delta time in Seconds
				if(tDoorMoveDifference >= iDOORMOVETIME){
					//STOP MOTOR
					digitalWrite(K1, 1);
					digitalWrite(K2, 1);

					strcpy(buffer_FIRST_LINE,"Tuer zu!");
					//Save Door staus in file
					SaveFileContentChar("/var/www/html/tmp/door.txt", "Tuer zu");

					STATE='C';//check only Door Status

					if (giIOTActiveStatus == 1){
						//send information to EmonCMS
						strcpy(json, "{Tuer:0}");
						uploadtoEMONCMS(json);
						//Clear arrays to avoid overflow
						memset(json, 0, sizeof(json));
					}

					if(digitalRead(DOOR_CLOSED) == 1){//Check end contact
						if(giTelegramFlag == 1){
							if(giTelegramActiveStatus){
								SendTelegramText("Tuer zu! (Automatic)");
							}
							else{
								printf("Tuer zu (Automatic)\n");
							}
						giTelegramFlag = 0; //Reset Flag
						}
					}
					else{//end contact failure
						if(giTelegramFlag == 1){
							if(giTelegramActiveStatus){
								SendTelegramText("Tuer zu! (Automatic) ALARM: Endschalter nicht erkannt. Keine weitere Aktion eingeleitet.");
							}
							else{
								printf("Tuer zu (Automatic) ALARM: Endschalter nicht erkannt. Keine weitere Aktion eingeleitet.\n");
							}
						giTelegramFlag = 0; //Reset Flag
						}
					}
			}
				break;
				/*OLD Code
				tDoorMove  = gNow; //start time measurement
				while(digitalRead(DOOR_CLOSED) == 1){//Close Door till close contact is active
					//Move Door start Motor (close)
					digitalWrite(K1, 1);
					digitalWrite(K2, 0);
					time(&gNow);//init now time
					tDoorMoveDifference = difftime(gNow, tDoorMove);//Check delta time in Seconds
					//Check if the DOOR is open or closed in the given time
					if(tDoorMoveDifference >= iDOORMOVETIME){
						DOORAlarm = TRUE;//Set Flag
						//Door is not open or closed in time send ALARM
						if (giTelegramActiveStatus == 1) {
							SendTelegramText("ALARM: Tür Wegzeit überschritten Tür wird gestoppt. Manual Modus wird aktiviert.");
						}
						else{
							printf("ALARM: Tür wegzeit überschritten\n");
						}
						break;//jump out of while loop
					}
				}
				//STOP MOTOR
				digitalWrite(K1, 1);
				digitalWrite(K2, 1);
				if(DOORAlarm){
					strcpy(buffer_FIRST_LINE,"ALARM: Tuer!");
					//Save Door staus in file
					SaveFileContentChar("/var/www/html/tmp/door.txt", "Alarm: Tuer!");

					iDoor_Modus = 0;//Activate Manual Door_Modus
					SaveFileContentInt("/var/www/html/chickenfarm/config/Door_Modus.txt", 0);
					DOORAlarm = FALSE;//Reset Flag
				}
				else{
					digitalWrite(K4, 0);//switch light on
					strcpy(buffer_FIRST_LINE,"Tuer zu!");
					if(giTelegramFlag == 1){
						if(giTelegramActiveStatus){
							SendTelegramText("Tuer zu! (Automatic)");
						}
						else{
							printf("Tuer zu (Automatic)\n");
						}
						giTelegramFlag = 0; //Reset Flag
						if (giIOTActiveStatus == 1){
							//send information to EmonCMS
							strcpy(json, "{Tuer:0}");
							uploadtoEMONCMS(json);
							//Clear arrays to avoid overflow
							memset(json, 0, sizeof(json));
						}
						//Save Door staus in file
						SaveFileContentChar("/var/www/html/tmp/door.txt", "Tuer zu");
					}
				}
				STATE='C';//check only Door Status
				OLD CODE END*/
			case 'B'://STATE definition open DOOR
				tDoorMove  = gNow; //start time measurement
				//Move Door START MOTOR (open)
				digitalWrite(K1, 0);
				digitalWrite(K2, 1);
				time(&gNow);//init now time
				tDoorMoveDifference = difftime(gNow, tDoorMove);//Check delta time in Seconds
				if(tDoorMoveDifference >= iDOORMOVETIME){
					//STOP MOTOR
					digitalWrite(K1, 1);
					digitalWrite(K2, 1);

					strcpy(buffer_FIRST_LINE,"Tuer auf!");
					//Save Door staus in file
					SaveFileContentChar("/var/www/html/tmp/door.txt", "Tuer auf");

					STATE='C';//check only Door Status

					if (giIOTActiveStatus == 1){
						//send information to EmonCMS
						strcpy(json, "{Tuer:1}");
						uploadtoEMONCMS(json);
						//Clear arrays to avoid overflow
						memset(json, 0, sizeof(json));
					}

					if(digitalRead(DOOR_OPEN) == 1){//Check end contact
						if(giTelegramFlag == 1){
							if(giTelegramActiveStatus){
								SendTelegramText("Tuer auf! (Automatic)");
							}
							else{
								printf("Tuer auf (Automatic)\n");
							}
						giTelegramFlag = 0; //Reset Flag
						}
					}
					else{//end contact failure
						if(giTelegramFlag == 1){
							if(giTelegramActiveStatus){
								SendTelegramText("Tuer auf! (Automatic) ALARM: Endschalter nicht erkannt. Keine weitere Aktion eingeleitet.");
							}
							else{
								printf("Tuer auf (Automatic) ALARM: Endschalter nicht erkannt. Keine weitere Aktion eingeleitet.\n");
							}
						giTelegramFlag = 0; //Reset Flag
						}
					}
			}
				break;
				/*OLD Code
				tDoorMove  = gNow; //start time measurement
				while(digitalRead(DOOR_OPEN) == 1){//Open Door till open contact is active
					//Move Door start Motor (open)
					digitalWrite(K1, 0);
					digitalWrite(K2, 1);
					time(&gNow);//init now time
					tDoorMoveDifference = difftime(gNow, tDoorMove);//Check delta time in Seconds
					//Check if the DOOR is open or closed in the given time
					if(tDoorMoveDifference >= iDOORMOVETIME){
						DOORAlarm = TRUE;//Set Flag
						//Door is not open or closed in time send ALARM
						if (giTelegramActiveStatus == 1) {
							SendTelegramText("ALARM: Tür Wegzeit überschritten. Tür wird gestoppt. Manual Modus wird aktiviert.");
						}
						else{
							printf("ALARM: Tür wegzeit überschritten\n");
						}
						break;//jump out of while loop
					}
				}
				//STOP MOTOR
				digitalWrite(K1, 1);
				digitalWrite(K2, 1);
				if(DOORAlarm){
					strcpy(buffer_FIRST_LINE,"ALARM: Tuer!");
					//Save Door staus in file
					SaveFileContentChar("/var/www/html/tmp/door.txt", "Alarm: Tuer!");

					iDoor_Modus = 0;//Activate Manual Door_Modus
					SaveFileContentInt("/var/www/html/chickenfarm/config/Door_Modus.txt", 0);
					DOORAlarm = FALSE;//Reset Flag
				}
				else{
					digitalWrite(K4, 1);//switch light off
					strcpy(buffer_FIRST_LINE,"Tuer auf!");
					if(giTelegramFlag == 1){
						if(giTelegramActiveStatus){
							SendTelegramText("Tuer auf! (Automatic)");
						}
						else{
							printf("Tuer auf! (Automatic)\n");
						}
						giTelegramFlag = 0; //Reset Flag
						if(giIOTActiveStatus == 1) {
							//send information to EmonCMS
							strcpy(json, "{Tuer:1}");
							uploadtoEMONCMS(json);
							//Clear arrays to avoid overflow
							memset(json, 0, sizeof(json));
						}
						//Save Door staus in file
						SaveFileContentChar("/var/www/html/tmp/door.txt", "Tuer auf");
					}
				}OLD CODE END*/
			case 'C'://check only Door Status
				if(digitalRead(DOOR_OPEN) == 0){//When DOOR is open then
					strcpy(buffer_FIRST_LINE,"Tuer auf!");
				}
				if(digitalRead(DOOR_CLOSED) == 0){//When DOOR is closed then
					strcpy(buffer_FIRST_LINE,"Tuer zu!");
				}
				if(digitalRead(DOOR_CLOSED) == 1 && digitalRead(DOOR_OPEN) == 1){//When DOOR is inbetween then
					strcpy(buffer_FIRST_LINE,"Tuer wedernoch!");
				}
				break;
			default:
				break;
		}
		//Check Power OFF and Switch OFF Booster and switch to 12V normal
		if(giPOWER_ON_OFF == 1){//Alarm active
		 	digitalWrite(K7, 1);//Switch to 12V normal
		 	digitalWrite(K8, 1);//Switch Booster OFF
		}
		/******************************************************************/
		if (giButtonPushFlag == 1){//If Button was pushed move Door.....
			int DOOR = 0;
			if(digitalRead(DOOR_OPEN) == 0){//When DOOR is open then.....Close Door
				DOOR = '1';//STATE definition close DOOR
				if(giTelegramActiveStatus){
					SendTelegramText("Tuer geht zu! (Manuell)");
				}
				else{
					printf("Tuer geht zu! (Manuell)\n");
				}
			}
			if(digitalRead(DOOR_CLOSED) == 0){//When DOOR is closed then...Open Door
				DOOR = '2';//STATE definition Door move up!
				if(giTelegramActiveStatus){
					SendTelegramText("Tuer geht auf! (Manuell)");
				}
				else{
					printf("Tuer geht auf! (Manuell)\n");
				}
			}
			if(digitalRead(DOOR_OPEN) == 1 && digitalRead(DOOR_CLOSED) == 1){//When DOOR is not in the end possition then....Open Door
				DOOR = '2';//STATE definition Door move up!
				if(giTelegramActiveStatus){
					SendTelegramText("Tuer geht auf! (Manuell)");
				}
				else{
					printf("Tuer geht auf! (Manuell)\n");
				}
			}
			/*************************************************************************/
			/*case '1'://STATE definition close DOOR                                 */
			/*case '2'://STATE definition open DOOR                                  */
			/*case '3'://check only Door Status                                      */
			/*************************************************************************/
			switch(DOOR){
				case '1'://STATE definition close DOOR
					tDoorMove  = gNow; //start time measurement
					while(digitalRead(DOOR_CLOSED) == 1){//Close Door till close contact is active
						digitalWrite(K1, 1);
						digitalWrite(K2, 0);
						time(&gNow);//init now time
						tDoorMoveDifference = difftime(gNow, tDoorMove);//Check delta time in Seconds
						//Check if the DOOR is open or closed in the given time
						if(tDoorMoveDifference >= iDOORMOVETIME){
							DOORAlarm = TRUE;//Set Flag
							//Door is not open or closed in time send ALARM
							if (giTelegramActiveStatus == 1) {
								SendTelegramText("ALARM: Tür Wegzeit überschritten Tür wird gestoppt. Manual Modus wird aktiviert.");
							}
							else{
								printf("ALARM: Tür wegzeit überschritten\n");
							}
							break;//jump out of while loop
						}
					}
					//STOP MOTOR
					digitalWrite(K1, 1);
					digitalWrite(K2, 1);
					if(DOORAlarm){
						strcpy(buffer_FIRST_LINE,"ALARM: Tuer!");
						//Save Door staus in file
						SaveFileContentChar("/var/www/html/tmp/door.txt", "Alarm: Tuer!");

						iDoor_Modus = 0;//Activate Manual Door_Modus
						SaveFileContentInt("/var/www/html/chickenfarm/config/Door_Modus.txt", 0);
						DOORAlarm = FALSE;//Reset Flag
					}
					else{
						digitalWrite(K4, 0);//switch light on
						strcpy(buffer_FIRST_LINE,"Tuer zu!");
						if(giTelegramFlag == 1){
							if(giTelegramActiveStatus){
									SendTelegramText("Tuer zu! (Manuell)");
								}
							else{
									printf("Tuer zu (Manuell)\n");
								}
							giTelegramFlag = 0; //Reset Flag
							if (giIOTActiveStatus == 1){
									//send information to EmonCMS
									strcpy(json, "{Tuer:0}");
									uploadtoEMONCMS(json);
									//Clear arrays to avoid overflow
									memset(json, 0, sizeof(json));
								}
							//Save Door staus in file
							SaveFileContentChar("/var/www/html/tmp/door.txt", "Tuer zu");
						}
					}
					DOOR = '3';//check only Door Status and stop Motor
					break;
				case '2'://STATE definition open DOOR
					tDoorMove  = gNow; //start time measurement
					while(digitalRead(DOOR_OPEN) == 1){//Open Door till close contact is active
						digitalWrite(K1, 0);
						digitalWrite(K2, 1);
						time(&gNow);//init now time
						tDoorMoveDifference = difftime(gNow, tDoorMove);//Check delta time in Seconds
						//Check if the DOOR is open or closed in the given time
						if(tDoorMoveDifference >= iDOORMOVETIME){
							DOORAlarm = TRUE;//Set Flag
							//Door is not open or closed in time send ALARM
							if (giTelegramActiveStatus == 1) {
								SendTelegramText("ALARM: Tür Wegzeit überschritten. Tür wird gestoppt. Manual Modus wird aktiviert.");
							}
							else{
								printf("ALARM: Tür wegzeit überschritten\n");
							}
							break;//jump out of while loop
						}
					}
					//STOP MOTOR
					digitalWrite(K1, 1);
					digitalWrite(K2, 1);
					if(DOORAlarm){
						strcpy(buffer_FIRST_LINE,"ALARM: Tuer!");
						//Save Door staus in file
						SaveFileContentChar("/var/www/html/tmp/door.txt", "Alarm: Tuer!");

						iDoor_Modus = 0;//Activate Manual Door_Modus
						SaveFileContentInt("/var/www/html/chickenfarm/config/Door_Modus.txt", 0);
						DOORAlarm = FALSE;//Reset Flag
					}
					else{
						digitalWrite(K4, 1);//switch light off
						strcpy(buffer_FIRST_LINE,"Tuer auf!");
						if(giTelegramFlag == 1){
							if(giTelegramActiveStatus){
								SendTelegramText("Tuer auf! (Manuell)");
							}
							else{
								printf("Tuer auf (Manuell)\n");
							}
							giTelegramFlag = 0; //Reset Flag
							if(giIOTActiveStatus == 1) {
								//send information to EmonCMS
								strcpy(json, "{Tuer:1}");
								uploadtoEMONCMS(json);
								//Clear arrays to avoid overflow
								memset(json, 0, sizeof(json));
							}
							//Save Door staus in file
							SaveFileContentChar("/var/www/html/tmp/door.txt", "Tuer auf");
						}
					}
					DOOR = '3';//check only Door Status and stop Motor
					break;
				case '3'://check only Door Status
					if(digitalRead(DOOR_OPEN) == 0){//When DOOR is open then
						strcpy(buffer_FIRST_LINE,"Tuer auf!");
					}
					if(digitalRead(DOOR_CLOSED) == 0){//When DOOR is closed then
						strcpy(buffer_FIRST_LINE,"Tuer zu!");
					}
					if(digitalRead(DOOR_CLOSED) == 1 && digitalRead(DOOR_OPEN) == 1){//When DOOR is inbetween then
						strcpy(buffer_FIRST_LINE,"Tuer wedernoch!");
					}
					break;
				default:
					break;
			}
			//Check Power OFF and Switch OFF Booster and switch to 12V normal
			if(giPOWER_ON_OFF == 1){
			 	digitalWrite(K7, 1);//Switch to 12V normal
			 	digitalWrite(K8, 1);//Switch Booster OFF
			}
			giButtonPushFlag = 0; //Reset Flag
		}
		/******************************************************************/

		/******************************************************************/
		/*SET-UP I2C Port for BME280 INTERN*/
		fd = wiringPiI2CSetup(BME280_ADDRESS_INTERN);
		if(fd < 0) {
			printf("Device not found\n");
			return -1;
		}
		readCalibrationData(fd, &cal);
		wiringPiI2CWriteReg8(fd, 0xf2, 0x01);// humidity oversampling x 1
		wiringPiI2CWriteReg8(fd, 0xf4, 0x25);// pressure and temperature oversampling x 1, mode normal
		getRawData(fd, &raw);
		/******************************************************************/
		/*Read Temp Value from Sensor and provide information to different strings*/
		t_fine = getTemperatureCalibration(&cal, raw.temperature);
		fTEMP_INTERN = compensateTemperature(t_fine);//C
		fTEMP_MIN_INTERN = minimum(fTEMP_MIN_INTERN, fTEMP_INTERN);
		fTEMP_MAX_INTERN = maximum(fTEMP_MAX_INTERN, fTEMP_INTERN);
		/*Convert float into char*/
		sprintf(cTEMP_MIN_INTERN, "Min: %0.1f C", fTEMP_MIN_INTERN);
		sprintf(cTEMP_MAX_INTERN, "Max: %0.1f C", fTEMP_MAX_INTERN);
		sprintf(cTEMP_INTERN, "Act: %0.1f C", fTEMP_INTERN);
		/******************************************************************/
		/*Read Humidity Value from Sensor and provide information to different strings*/
		fHUM_INTERN = compensateHumidity(raw.humidity, &cal, t_fine);// %
		fHUM_MIN_INTERN = minimum(fHUM_MIN_INTERN, fHUM_INTERN);
		fHUM_MAX_INTERN = maximum(fHUM_MAX_INTERN, fHUM_INTERN);
		/*Convert float into char*/
		sprintf(cHUM_MIN_INTERN, "Min: %0.1f %%", fHUM_MIN_INTERN);
		sprintf(cHUM_MAX_INTERN, "Max: %0.1f %%", fHUM_MAX_INTERN);
		sprintf(cHUM_INTERN, "Act: %0.1f %%", fHUM_INTERN);
		/******************************************************************/
		/*Read Preasure Value from Sensor and provide information to different strings*/
		fPRESS_INTERN = compensatePressure(raw.pressure, &cal, t_fine) / 100;// hPa
		fPRESS_MIN_INTERN = minimum(fPRESS_MIN_INTERN, fPRESS_INTERN);
		fPRESS_MAX_INTERN = maximum(fPRESS_MAX_INTERN, fPRESS_INTERN);
		/******************************************************************/
		/*Read Altitude Value from Sensor and provide information to different strings*/
		fALT_INTERN = getAltitude(fPRESS_INTERN);// meters
		close (fd) ;
		/******************************************************************/
		/*SET-UP I2C Port for BME280 EXTERN*/
		fd = wiringPiI2CSetup(BME280_ADDRESS_EXTERN);
		if(fd < 0) {
			printf("Device not found\n");
			return -1;
		}
		readCalibrationData(fd, &cal);
		wiringPiI2CWriteReg8(fd, 0xf2, 0x01);// humidity oversampling x 1
		wiringPiI2CWriteReg8(fd, 0xf4, 0x25);// pressure and temperature oversampling x 1, mode normal
		getRawData(fd, &raw);
		/******************************************************************/
		/*Read Temp Value from Sensor and provide information to different strings*/
		t_fine = getTemperatureCalibration(&cal, raw.temperature);
		fTEMP_EXTERN = compensateTemperature(t_fine);//C
		fTEMP_MIN_EXTERN = minimum(fTEMP_MIN_EXTERN, fTEMP_EXTERN);
		fTEMP_MAX_EXTERN = maximum(fTEMP_MAX_EXTERN, fTEMP_EXTERN);
		/*Convert float into char*/
		sprintf(cTEMP_MIN_EXTERN, "Min: %0.1f C", fTEMP_MIN_EXTERN);
		sprintf(cTEMP_MAX_EXTERN, "Max: %0.1f C", fTEMP_MAX_EXTERN);
		sprintf(cTEMP_EXTERN, "Act: %0.1f C", fTEMP_EXTERN);
		/******************************************************************/
		/*Read Humidity Value from Sensor and provide information to different strings*/
		fHUM_EXTERN = compensateHumidity(raw.humidity, &cal, t_fine);// %
		fHUM_MIN_EXTERN = minimum(fHUM_MIN_EXTERN, fHUM_EXTERN);
		fHUM_MAX_EXTERN = maximum(fHUM_MAX_EXTERN, fHUM_EXTERN);
		/*Convert float into char*/
		sprintf(cHUM_MIN_EXTERN, "Min: %0.1f %%", fHUM_MIN_EXTERN);
		sprintf(cHUM_MAX_EXTERN, "Max: %0.1f %%", fHUM_MAX_EXTERN);
		sprintf(cHUM_EXTERN, "Act: %0.1f %%", fHUM_EXTERN);
		/***}***************************************************************/
		/*Read Preasure Value from Sensor and provide information to different strings*/
		fPRESS_EXTERN = compensatePressure(raw.pressure, &cal, t_fine) / 100;// hPa
		fPRESS_MIN_EXTERN = minimum(fPRESS_MIN_EXTERN, fPRESS_EXTERN);
		fPRESS_MAX_EXTERN = maximum(fPRESS_MAX_EXTERN, fPRESS_EXTERN);
		/******************************************************************/
		/*Read Altitude Value from Sensor and provide information to different strings*/
		fALT_EXTERN = getAltitude(fPRESS_EXTERN);// meters
		close (fd) ;
		/******************************************************************/
		/*Altitude RMS Intern + Extern*/
		fALT = (fALT_INTERN + fALT_EXTERN)/2;
		sprintf(cALT, "Hoehe: %0.1f m", fALT);
		/******************************************************************/
		/*Preasure RMS Intern + Extern*/
		fPRESS_MIN = (fPRESS_MIN_EXTERN + fPRESS_MIN_INTERN)/2;
		fPRESS_MAX = (fPRESS_MAX_EXTERN + fPRESS_MAX_INTERN)/2;
		fPRESS = (fPRESS_EXTERN + fPRESS_INTERN)/2;
		//Convert float into char
		sprintf(cPRESS_MIN, "Min: %0.1f hPa", fPRESS_MIN);
		sprintf(cPRESS_MAX, "Max: %0.1f hPa", fPRESS_MAX);
		sprintf(cPRESS, "Act: %0.1f hPa", fPRESS);
		/******************************************************************/
		/*Decide if Intern Temperature exeed limit*/
		if(fTEMP_INTERN <= iTRESHOLD_TEMP_INTERN){//Show ALARM
			iTEMP_ALARM = 1; //Alarm is active
			if(InternalTempLow){
				if (giTelegramActiveStatus == 1) {
					SendTelegramText("ALARM: Internal TEMPERATURE LOW!");
					SendTelegramText(cTEMP_INTERN);
				}
				else{
					printf("ALARM: Internal Temperature Low\n");
				}
				InternalTempLow = FALSE; //Reset Flag
				InternalTempOK = TRUE; //set Flag
				/*Count time duration of alarm*/
				TempAlarmTimeStart  = gNow;
			}
		}
		if(fTEMP_INTERN >= (iTRESHOLD_TEMP_INTERN + iHYSTERESIS_TEMP_INTERN)){//disable ALARM
			iTEMP_ALARM = 0; //Alarm is deactivated
			if(InternalTempOK){
				/*Calculate running time*/
				TempAlarmTimeDifference = difftime(TempAlarmTimeStart, TempAlarmTimeEnd);
				TimeCalculation(TempAlarmTimeDifference,cTempAlarmTime);
				TempAlarmTimeStart = 0; //Reset Value
				if (giTelegramActiveStatus == 1) {
					SendTelegramText("Internal TEMPERATURE OK since:");
					SendTelegramText(cTempAlarmTime);
					SendTelegramText(cTEMP_INTERN);
				}
				else{
					printf("Internal Temperature OK again\n");
				}
				InternalTempLow = TRUE; //Reset Flag
				InternalTempOK = FALSE; //set Flag
			}
		}
		/******************************************************************/
		/*AKKU Laufzeit Berechnen*/
		/*Check Status of USV*/
		if(giPOWER_ON_OFF == 1){ //ALARM
			//Check Status of AKKU.
			iAkkulaufzeit = AKKUGROESSE / VERBRAUCH;
			diff_t = difftime(gNow, gPowerOFFTime);//calculate delta time in seconds
			//printf("USV diff_t: %f\n", diff_t);
			fAKKU_STATUS = 100-((100/iAkkulaufzeit)*(diff_t/3600));

			//Decide to Power down Raspberry
			if(fAKKU_STATUS <= LIMIT_ABSCHALTUNG){
				//Power Down Raspberry
				//Show Message on Display of shutdown
				digitalWrite(K3, 0);//Switch on Blacklight Relais
				lcdHome(lcd);//goto first curser position
				lcdPuts(lcd,"********************");//write 1st line
				lcdPuts(lcd,"*  System will be  *");//write 2nd line
				lcdPuts(lcd,"*Shutdown Low Power*");//write 3rd line
				lcdPuts(lcd,"********************");//write 4th line
				if (giTelegramActiveStatus == 1) {
					SendTelegramText("Power Low System shut down!");
				}
				else{
					printf("Power Low System shut down\n");
				}
				if(giIOTActiveStatus == 1){
					//send information to EmonCMS
					//build json string
					strcpy(json, "{Betriebsmodi:0}");
					uploadtoEMONCMS(json);
					//Clear arrays to avoid overflow
					memset(json, 0, sizeof(json));
				}
				delay(4000); //show for 4seconds
				//system("sudo shutdown -h now"); //Shutdown System
				digitalWrite(K3, 1);//Switch off Blacklight Relais
			}
			if(PowerAlarm){
				if (giTelegramActiveStatus == 1) {
					SendTelegramText("Power Alarm USV active!");
				}
				else{
					printf("Power Alarm USV active\n");
				}
				PowerAlarm = FALSE;//Reset Flag
			}
		}
		if(giPOWER_ON_OFF == 0){//No ALARM
			//Check Status of AKKU
			if(fAKKU_STATUS >= 100){
				//AKKU is charged
				fAKKU_STATUS = 100;
				//printf("AKKU 100%%\n");
			}
			else{
				//Calculate charge status
				iAkkuladezeit = AKKUGROESSE / AKKULADESTROM * 1.3; //Values are in Ah
				diff_t = difftime(gNow, gPowerONTime);//calculate delta time
				//printf("Charge diff_t: %f\n", diff_t);
				fAKKU_STATUS = (fAKKU_STATUS + ((100/iAkkuladezeit)*(diff_t/3600)));
			}
			PowerAlarm = TRUE; //Set Flag
		}
		//AKKUINFO
		//printf("fAKKU_STATUS:  %0.2f\n",fAKKU_STATUS);
		sprintf(cAKKU_STATUS, "AKKU: %0.2f %%", fAKKU_STATUS);
		sprintf(cAKKUGROESSE, "Akkugroesse: %d Ah", AKKUGROESSE);
		sprintf(cVERBRAUCH, "Verbrauch: %d A", VERBRAUCH);
		sprintf(cLIMIT_ABSCHALTUNG, "ShutDownLimit: %d %%", LIMIT_ABSCHALTUNG);
		sprintf(cAKKULADESTROM, "Ladestrom: %d A", AKKULADESTROM);

		/******************************************************************/
		/*Egg counter*/
		if(giEGG == 1){
			iEGG_COUNTER ++; //Count EGG EGG_COUNTER
			sprintf(cEGG_COUNTER, "%d", iEGG_COUNTER);
			if(giTelegramActiveStatus == 1){
				SendTelegramText("Es gibt ein Ei!");//send Telegram Message
			}
			else{
				printf("Es gibt ein Ei\n");
			}
			if(giIOTActiveStatus == 1){
				//send information to EmonCMS
				//build json string
				strcpy(json, "{Anzahl_Eier:");
				strcat(json, cEGG_COUNTER);
				strcat(json, "}");
				uploadtoEMONCMS(json);
				//Clear arrays to avoid overflow
				memset(json, 0, sizeof(json));
			}
			giEGG = 0;//Reset igEGG
		}
		/******************************************************************/
		/*Convert integer into char for Values*/
		sprintf(cTRESHOLD_LUX, "Lux GW: %d LX", iTRESHOLD_LUX);
		sprintf(cHYSTERESIS_LUX, "Lux H: %d LX", iHYSTERESIS_LUX);
		sprintf(cTRESHOLD_TEMP_INTERN, "Temp GW: %d C", iTRESHOLD_TEMP_INTERN);
		sprintf(cHYSTERESIS_TEMP_INTERN, "Temp H: %d C", iHYSTERESIS_TEMP_INTERN);
		sprintf(cDOORMOVETIME, "Time Türfahrt: %d s", iDOORMOVETIME);
		/******************************************************************/
		/*Check Encoder Status and show Menue*/
		if(gNow - toggletimeold >= TOGGLETIME){
			toggletimeold = gNow;
			iToggle = 1 - iToggle;
		}
		/******************************************************************/
		/*Menue*/
		switch(iENCODER){
			/*************************************************************************/
			/*case '0'://MAIN MENUE                                                  */
			/*case '1'://2nd MENUE Helligkeit                                        */
			/*case '2'://3rd MENUE Temperatur Innen / Temperatur Aussen (toggletime) */
			/*case '3'://4rd MENUE Luftfeuchte Innen Luftfeuchte Aussen (toggletime) */
			/*case '4'://5rd MENUE Luftfeuchte Innen Luftfeuchte Aussen (toggletime) */
			/*case '5'://6th MENUE Akku Info                                         */
			/*case '6'://7th MENUE SET-UP                                            */
			/*case '7'://8th MENUE INFO                                              */
			/*************************************************************************/
			case 0: //MAIN MENUE
				/*Check Position of string, time shall start on position 15*/
				FILLSTRINGTOPOSITION(buffer_FIRST_LINE,15);
				strcat(buffer_FIRST_LINE,buffer_time);
				switch(iToggle){
					case 0:
						if(iTEMP_ALARM == 1){
							strcpy(buffer_SECOND_LINE,"Intern TEMP ALARM");
						}
						else{
							strcpy(buffer_SECOND_LINE,"No TEMP Alarm");
						}
						break;
					case 1:
						strcpy(buffer_SECOND_LINE,cEGG_COUNTER);
						break;
					default:
						strcpy(buffer_SECOND_LINE,"");
						break;
				}
				switch(iToggle){
					case 0:
						if(giPOWER_ON_OFF == 1){
							strcpy(buffer_THIRD_LINE,"NOTSTROM AKTIV!");
						}
						else{
							strcpy(buffer_THIRD_LINE,"STROM OK");
						}
						break;
					case 1:
							strcat(buffer_THIRD_LINE,cAKKU_STATUS);
							//strcat(buffer_THIRD_LINE,cALT);
						break;
					default:
						strcpy(buffer_THIRD_LINE,"");
						break;
				}
				switch(iToggle){
					case 0:
							if(iDoor_Modus == 0){
								strcpy(buffer_FOURTH_LINE,"Manual Modus");
							}
							else{
								strcpy(buffer_FOURTH_LINE,"Automatic Modus");
							}
						break;
					case 1:
					if(giTelegramActiveStatus == 0){
						strcpy(buffer_FOURTH_LINE,"Telegram deaktiviert");
					}
					else{
						strcpy(buffer_FOURTH_LINE,"Telegram aktiviert");
					}
						break;
					default:
						strcpy(buffer_FOURTH_LINE,"");
						break;
				}
				break;
			case 1: //2nd MENUE Helligkeit
				strcpy(buffer_FIRST_LINE,"*****Helligkeit*****");
				strcat(buffer_SECOND_LINE,cLUX_RMS);
				strcat(buffer_THIRD_LINE,cLUX_MIN);
				strcat(buffer_FOURTH_LINE,cLUX_MAX);
				break;
			case 2: //3rd MENUE Temperatur Innen / Temperatur Aussen
				switch(iToggle){
					case 0:
						strcpy(buffer_FIRST_LINE,"**Temperatur Innen**");
						strcat(buffer_SECOND_LINE,cTEMP_INTERN);
						strcat(buffer_THIRD_LINE,cTEMP_MIN_INTERN);
						strcat(buffer_FOURTH_LINE,cTEMP_MAX_INTERN);
						break;
					case 1:
						strcpy(buffer_FIRST_LINE,"**Temperatur Aussen*");
						strcat(buffer_SECOND_LINE,cTEMP_EXTERN);
						strcat(buffer_THIRD_LINE,cTEMP_MIN_EXTERN);
						strcat(buffer_FOURTH_LINE,cTEMP_MAX_EXTERN);
						break;
				}
				break;
			case 3: //4rd MENUE Luftfeuchte Innen Luftfeuchte Aussen
				switch(iToggle){
					case 0:
						strcpy(buffer_FIRST_LINE,"*Luftfeuchte Innen**");
						strcat(buffer_SECOND_LINE,cHUM_INTERN);
						strcat(buffer_THIRD_LINE,cHUM_MIN_INTERN);
						strcat(buffer_FOURTH_LINE,cHUM_MAX_INTERN);
						break;
					case 1:
						strcpy(buffer_FIRST_LINE,"*Luftfeuchte Aussen*");
						strcat(buffer_SECOND_LINE,cHUM_EXTERN);
						strcat(buffer_THIRD_LINE,cHUM_MIN_EXTERN);
						strcat(buffer_FOURTH_LINE,cHUM_MAX_EXTERN);
						break;
				}
				break;
			case 4: //5rd MENUE Luftfeuchte Innen Luftfeuchte Aussen
				strcpy(buffer_FIRST_LINE,"*****Luftdruck******");
				strcat(buffer_SECOND_LINE,cPRESS);
				strcat(buffer_THIRD_LINE,cPRESS_MIN);
				strcat(buffer_FOURTH_LINE,cPRESS_MAX);
				break;
			case 5: //6th MENUE Akku Info
				switch(iToggle){
					case 0:
						strcpy(buffer_FIRST_LINE,"*****AKKU INFO******");
						strcat(buffer_SECOND_LINE,cAKKUGROESSE);
						strcat(buffer_THIRD_LINE,cVERBRAUCH);
						strcat(buffer_FOURTH_LINE,cAKKULADESTROM);
						break;
					case 1:
						strcpy(buffer_FIRST_LINE,"*****AKKU INFO******");
						strcat(buffer_SECOND_LINE,cVERBRAUCH);
						strcat(buffer_THIRD_LINE,cAKKULADESTROM);
						strcat(buffer_FOURTH_LINE,cLIMIT_ABSCHALTUNG);
						break;
				}
				break;
			case 6: //7th MENUE SET-UP
				switch(iToggle){
					case 0:
						strcpy(buffer_FIRST_LINE,"******Set-up********");
						strcat(buffer_SECOND_LINE,cTRESHOLD_LUX);
						strcat(buffer_THIRD_LINE,cHYSTERESIS_LUX);
						strcat(buffer_FOURTH_LINE,cDOORMOVETIME);
						break;
					case 1:
						strcpy(buffer_FIRST_LINE,"******Set-up********");
						strcat(buffer_SECOND_LINE,cTRESHOLD_TEMP_INTERN);
						strcat(buffer_THIRD_LINE,cHYSTERESIS_TEMP_INTERN);
						strcpy(buffer_FOURTH_LINE,"");
						break;
				}
				break;
			case 7: //8th MENUE INFO
				strcpy(buffer_FIRST_LINE,"********INFO********");
				strcpy(buffer_SECOND_LINE,IP_ADRESS);
				strcpy(buffer_THIRD_LINE,buffer_date);
				strcpy(buffer_FOURTH_LINE,"Version: V1.0");
				break;
			default:
				break;
		}
		/******************************************************************/
		/*Send provided Information to the Display*/
		/*Check if String is full of characters to ensure correct line shift*/
		FILLSTRING(buffer_FIRST_LINE);
		FILLSTRING(buffer_SECOND_LINE);
		FILLSTRING(buffer_THIRD_LINE);
		FILLSTRING(buffer_FOURTH_LINE);

		/*Write to Display*/
		lcdHome(lcd);//goto first curser position
		lcdPuts(lcd, buffer_FIRST_LINE);//write 1st Line
		lcdPuts(lcd, buffer_SECOND_LINE);//write 2nd line
		lcdPuts(lcd, buffer_THIRD_LINE);//write 3rd line
		lcdPuts(lcd, buffer_FOURTH_LINE);//write 4th line

		delay(500);
		/*Clear arrays to avoid overflow*/
		memset(buffer_FIRST_LINE, 0, sizeof(buffer_FIRST_LINE));//write 1st Line
		memset(buffer_SECOND_LINE, 0, sizeof(buffer_SECOND_LINE));//write 2nd line
		memset(buffer_THIRD_LINE, 0, sizeof(buffer_THIRD_LINE));//write 3rd line
		memset(buffer_FOURTH_LINE, 0, sizeof(buffer_FOURTH_LINE));//write 4th line
		/******************************************************************************************/
		/* Check Timestamp and save every x second the information to the different storage files */
		/******************************************************************************************/
		if(gNow - timeold >= giUpdateSequence){
			timeold = gNow;
			FILE *datei;
			/*Running time claculation*/
			RUNNING_MINUTES++;

			if(RUNNING_MINUTES == 60){//Minutes/Hours overflow
				RUNNING_MINUTES = 0;
				RUNNING_HOURS++;
			}
			if(RUNNING_HOURS >= 24){//Hours/Days overflow
				RUNNING_HOURS = 0;
				RUNNING_DAYS++;
			}
			/*Save Temperature in file*/
			SaveFileContentTwoFloat("/var/www/html/tmp/temperature.csv", "in", "out", fTEMP_INTERN, fTEMP_EXTERN);
			/*Save Humidity in file*/
			SaveFileContentTwoFloat("/var/www/html/tmp/humidity.csv", "in", "out", fHUM_INTERN, fHUM_EXTERN);
			/*Save Pressure in file*/
			SaveFileContentOneFloat("/var/www/html/tmp/pressure.csv","hPa",fPRESS);
			/*Save LUX in file*/
			SaveFileContentOneFloat("/var/www/html/tmp/lux.csv","LUX",fLUX_RMS);
			/*Save Timestamp in file*/
			SaveFileContentChar("/var/www/html/tmp/time.txt", asctime(tm_info));

			/*Save runtime in file*/
			datei=fopen("/var/www/html/tmp/runtime.txt","w"); //open file
			if(datei == NULL) {//File could not open
				printf("No file: runtime.txt\n");
				fprintf(datei, "%d Tage %d Stunden %d Minuten\n",RUNNING_DAYS, RUNNING_HOURS, RUNNING_MINUTES);
			}
			else{//open file
				fprintf(datei, "%d Tage %d Stunden %d Minuten\n",RUNNING_DAYS, RUNNING_HOURS, RUNNING_MINUTES);
			}
			fclose(datei); //close file
			/*Comand line Information*/
			printf("##########################################################\n");
			printf("# INFO from:                                             #\n");
			printf("# Current local time and date: %s #", asctime(tm_info));
			printf("##########################################################\n");
			printf("Betriebsstunden:\n%d Tage %d Stunden %d Minuten\n",RUNNING_DAYS, RUNNING_HOURS, RUNNING_MINUTES);
			printf("**********************************************************\n");
			TempAlarmTimeDifference = difftime(gNow, TempAlarmTimeStart);
			TimeCalculation(TempAlarmTimeDifference,cTempAlarmTime);
      printf("Temperature Intern Information:\nTemperature low since: %s", asctime(gmtime(&TempAlarmTimeStart)));
			printf("Temperatur low running time: %s\n", cTempAlarmTime);
			printf("**********************************************************\n");
			printf("Power Status:\ngiPOWER_ON_OFF Status %d\n", giPOWER_ON_OFF);
			printf("Akkustatus: %s\n",cAKKU_STATUS );//Akkustatus
			printf("diff_t: %f\n", diff_t);//Time since charging or USV active
			printf("**********************************************************\n");
			printf("Door Modus Flag: %d 0=Manual; 1=Automatic\n", iDoor_Modus);
			printf("**********************************************************\n");
			printf("LUXthreshold: %d\n",iTRESHOLD_LUX );//LUXthreshold
			printf("LUXhysteresis: %d\n",iHYSTERESIS_LUX );//LUXhysteresis
			printf("**********************************************************\n");
			printf("LuxBright: %d\n",LuxBright );//LuxBright
			printf("LuxDark: %d\n",LuxDark );//LuxDark
			printf("**********************************************************\n");
			/*******************************************************************/
			/*Safe Information to the EmonCMS Channel                          */
			/*******************************************************************/
			//Convert fload into char for Values
			sprintf(cTEMP_EXTERN,"%0.1f",fTEMP_EXTERN);//Temperatur aussen
			sprintf(cTEMP_INTERN,"%0.1f",fTEMP_INTERN );//Temperatur innen
			sprintf(cHUM_EXTERN,"%0.1f",fHUM_EXTERN);//Feuchtigkeit aussen
			sprintf(cHUM_INTERN,"%0.1f",fHUM_INTERN );//Feuchtigkeit innen
			sprintf(cPRESS,"%0.1f",fPRESS);//Druck
			sprintf(cLUX_RMS,"%0.1f",fLUX_RMS );//Helligkeit
			sprintf(cAKKU_STATUS,"%0.2f",fAKKU_STATUS );//Akkustatus

			printf("Daten die an EmonCMS gesendet werden:\n");
			printf("Temp extern: %s\n",cTEMP_EXTERN);//Temperatur aussen
			printf("Temp intern: %s\n",cTEMP_INTERN );//Temperatur innen
			printf("Hum extern: %s\n",cHUM_EXTERN);//Feuchtigkeit aussen
			printf("Hum intern: %s\n",cHUM_INTERN );//Feuchtigkeit innen
			printf("Press: %s\n",cPRESS);//Druck
			printf("Lux: %s\n",cLUX_RMS );//Helligkeit
			printf("Akkustatus: %s\n",cAKKU_STATUS );//Akkustatus
			if(giIOTActiveStatus == 1){
				//build json string
				strcpy(json, "{Temperatur_extern:");
				strcat(json, cTEMP_EXTERN);
				strcat(json, ",Temperatur_intern:");
				strcat(json, cTEMP_INTERN);
				strcat(json, ",Feuchte_extern:");
				strcat(json, cHUM_EXTERN);
				strcat(json, ",Feuchte_intern:");
				strcat(json, cHUM_INTERN);
				strcat(json, ",Luftdruck:");
				strcat(json, cPRESS);
				strcat(json, ",Helligkeit:");
				strcat(json, cLUX_RMS);
				strcat(json, ",Akkustatus:");
				strcat(json, cAKKU_STATUS);
				strcat(json, "}");

				uploadtoEMONCMS(json);
				//Clear arrays to avoid overflow
				memset(json, 0, sizeof(json));
			}
		}
		/*********************************************************/
		/* Check Timestamp and switch Display Backlight off      */
		/*********************************************************/
		if(gNow - gBacklightTime >= BACKLIGHT_MAX_TIME){
			digitalWrite(K3, 1);//Switch off Blacklight Relais
		}
	}
}//MAIN END
/************************************************************/
/* GETIP						                          							*/
/* Gets the IP from the device and write it to the LCD for  */
/* input: outputBuffer                                      */
/* output: -                                                */
/************************************************************/
void GETIP(char *outputBuffer){
	int n;
  struct ifreq ifr;
  char iface[] = "eth0"; //Change this to the network of your choice (eth0, wlan0, etc.)

  n = socket(AF_INET, SOCK_DGRAM, 0);
  ifr.ifr_addr.sa_family = AF_INET;
  strncpy(ifr.ifr_name , iface , IFNAMSIZ - 1);
  ioctl(n, SIOCGIFADDR, &ifr);
  close(n);
  //strcpy(outputBuffer,":");
  strcpy(outputBuffer,iface);
  strcat(outputBuffer,": ");
  strcat(outputBuffer,inet_ntoa(( (struct sockaddr_in *)&ifr.ifr_addr )->sin_addr));
}
/************************************************************/
/* SETUP         											*/
/* Set-up and initialization of:                            */
/* - Encode PIN Mode                                        */
/* - ISR for Entcoder and Button                            */
/* - PCF8574                                                */
/* - Check Power Status and send to EmonCMS                 */
/************************************************************/
void SETUP(){
	char json[256];
	/* sets up the wiringPi library */
	if(wiringPiSetup () < 0) {
		fprintf (stderr, "Unable to setup wiringPi: %s\n", strerror (errno));
		return;
	}
	/*Setup of PCF8574 (pinBase, Adress of the device*/
	pcf8574Setup(100, 0x20);

	/*Set all pins of the PCF8574 to output */
	int i;
	for (i = 0; i < 8; ++i) {
		pinMode(100 + i, OUTPUT);
	}
	/*Set default Relais*/
	digitalWrite(K1, 1); //Motor to GND
	digitalWrite(K2, 1); //Motor to GND
	digitalWrite(K3, 1); //LCD Display Backlight
	digitalWrite(K4, 1); //Beleuchtung
	digitalWrite(K5, 1);
	digitalWrite(K6, 1);
	digitalWrite(K7, 1); //12V Normal/12V Boost
	digitalWrite(K8, 1); //Booster ON/OFF

	//activate I2C HW depending on Raspberry PI HW Revision and Open I2C Port
	//if(HwRev < 2) 	I2C_Open("/dev/i2c-0");	 // Hardware Revision 1.0
	//else		I2C_Open("/dev/i2c-1");  // Hardware Revision 2.0
	//I2C_Open("/dev/i2c-1");  // Hardware Revision 2.0

	/* set Pin Modes */
	pinMode(ENCODER_A, INPUT);
	pinMode(ENCODER_B, INPUT);//set PIN 13/2 as Input for Rotary Encoder
	pinMode(BUTTON_START_STOP, INPUT);
	pinMode(DOOR_CLOSED, INPUT);
	pinMode(DOOR_OPEN, INPUT);
	/* set pull Up/Down */
	pullUpDnControl(BUTTON_START_STOP, PUD_UP);
	pullUpDnControl(DOOR_CLOSED, PUD_UP);
	pullUpDnControl(DOOR_OPEN, PUD_UP);
	/*Initialize the ISR*/
	// set Pin 11/0 generate an interrupt on both transitions
	// and attach ISR_BUTTON_START_STOP() to the interrupt
	if(wiringPiISR (BUTTON_START_STOP, INT_EDGE_FALLING, &ISR_BUTTON_START_STOP) < 0 ) {
		fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno));
		printf("Unable to setup ISR: BUTTON_START_STOP\n");
		return;
	}
	// set Pin 18/5 generate an interrupt on high-to-low transitions
	// and attach ISR_DOOR_CLOSED() to the interrupt
	/*if(wiringPiISR (DOOR_CLOSED, INT_EDGE_FALLING, &ISR_DOOR_CLOSED) < 0 ) {
		fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno));
		printf("Unable to setup ISR: DOOR_CLOSED\n");
		return;
	}*/
	// set Pin 22/6 generate an interrupt on high-to-low transitions
	// and attach ISR_DOOR_OPEN() to the interrupt
	/*if(wiringPiISR (DOOR_OPEN, INT_EDGE_FALLING, &ISR_DOOR_OPEN) < 0 ) {
		fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno));
		printf("Unable to setup ISR: DOOR_OPEN\n");
		return;
	}*/
	// set Pin 12/1 generate an interrupt on high-to-low transitions
	// and attach ISR_ENCODER() to the interrupt
	if(wiringPiISR (ENCODER_A, INT_EDGE_BOTH, &ISR_ENCODER) < 0 ) {
		fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno));
		printf("Unable to setup ISR: ENCODER_A\n");
		return;
	}
	//set Pin 07/07 generate an interrupt on both transitions
	// and attach ISR_POWER_CONTROL() to the interrupt
	if(wiringPiISR (POWER_ON_OFF, INT_EDGE_BOTH, &ISR_POWER_CONTROL) < 0 ) {
		fprintf (stderr, "Unable to setup ISR: POWER_ON_OFF %s\n", strerror (errno));
		printf("Tuer auf!\n");
		return;
	}/*
	//set Pin XX/XX generate an interrupt on both transitions
	// and attach ISR_EGG_COUNTER() to the interrupt
	if(wiringPiISR (EGG_COUNTER, INT_EDGE_BOTH, &ISR_EGG_COUNTER) < 0 ) {
		fprintf (stderr, "Unable to setup ISR: %s\n", strerror (errno));
		return;
	}*/
	//Check Status of Power during start-up
	//Check Status of PIN 3V3 = Power OK / 0V = Power NOK

	if(digitalRead(POWER_ON_OFF) == 1){
		giPOWER_ON_OFF = 0;//Disable Alarm
		time(&gPowerONTime); //start counting Power ON time
		//build json string
		strcpy(json, "{Betriebsmodi:1}");//ON
	}
	if(digitalRead(POWER_ON_OFF) == 0){
		giPOWER_ON_OFF = 1;//Show alarm
		time(&gPowerOFFTime); //start counting Power OFF time
		//build json string
		strcpy(json, "{Betriebsmodi:2}");//AKKU
	}
	//Print information on command line
	printf("giPOWER_ON_OFF Status %d: 0=Power OK 1=Power NOK\n", giPOWER_ON_OFF);
	/*Check Status of DOOR*/
	printf("Check Status of DOOR before STARTUP\n");
	if(digitalRead(DOOR_OPEN) == 0){//When DOOR is open then
		printf("DOOR Open\n");
	}
	if(digitalRead(DOOR_CLOSED) == 0){//When DOOR is closed then
		printf("DOOR Closed\n");
	}
	if(digitalRead(DOOR_CLOSED) == 1 && digitalRead(DOOR_OPEN) == 1){//When DOOR is inbetween then
		printf("DOOR inbetween\n");
	}
	if(giIOTActiveStatus == 1){
		//send information to EmonCMS
		uploadtoEMONCMS(json);
		//Clear arrays to avoid overflow
		memset(json, 0, sizeof(json));
	}
	printf("Setup end\n");
	//send Telegram if start-up occurs
	SendTelegramText("Start-up. Set-up finished.");

	return;
}
/************************************************************/
/* minimum       						                      					*/
/* calculate the minimum of two values                      */
/*                                                          */
/************************************************************/
float minimum(float a, float min){ //Funktion zur Bestimmung der kleinsten Zahl
	if (a < min){                  //wenn a kleiner als das Minimum ist
		min = a;					//wird das Minimum = a gesetz
	}
	return(min);                  //das Minimum wird zurück gegeben.
}
/************************************************************/
/* maximum                            											*/
/* calculate the maximum of two values                      */
/*                                                          */
/************************************************************/
float maximum(float a, float max){ //Funktion zur Bestimmung der grössten Zahl
	if (a > max){               //wenn a grösser als das Maximum ist
		max = a;
	}                   //wird das Maximum = a gesetzt
	return(max);               //das Maximum wird zurückgegeben
}
/************************************************************/
/* mittelwert     											                    */
/* calculate the rms of two values                          */
/* input: value for RMS calculation                         */
/* output: RMS value                                        */
/************************************************************/
float mittelwert(float a, float rms){ //Funktion zur Bestimmung des Mittelwertes
	//Average over 200 measurements values
	//x++;//Count Measurement steps for RMS Calculation
	a=a+a+rms+rms;//Calculate Value
	//if (x == RMS){
	//	rms=a/RMS;
		//LUX_RMS_CALC=0;//Reset RMS Value
	//	x=0;//Reset x Value
	//}
	//else{
		rms=a/4; //interim value till 200 measurements are reached
	//}
	return(rms);
	/*Gleittender Mittelwert

	float a[RMS]; //Gleittender mittelwert über x Messungen

	int anzahl sizeof a / sizeof (float);

	float oldValue = a[0];
	for(int i = 0; i < anzahl-1; i++){
		float aux = a[i];
		a[i] = (oldValue+a[i]+a[i+1])/3.0;
		oldValue = aux;
	}
	a[anzahl-1] = (2.0*a[anzahl-1] + oldValue)/3.0;*/
}
/************************************************************/
/* FILLSTRING 										                      		*/
/* Fill string with " " to ensure correct line shift        */
/* input: output buffer                                     */
/* return:                                                  */
/************************************************************/
void FILLSTRING(char *outputBuffer){
	int i,x;
	x = 20 - strlen(outputBuffer);
	for(i=0; i<x;i++){
		strcat(outputBuffer," ");
	}
}
/************************************************************/
/* FILLSTRINGTOPOSITION                                     */
/* Fill string with " " to defined position                 */
/* input: Output buffer + position                          */
/* return: -                                                */
/************************************************************/
void FILLSTRINGTOPOSITION(char *outputBuffer, int position){
	int i,x;
	if (strlen(outputBuffer) < position){
		x = position - strlen(outputBuffer);
		for (i=0 ; i<x ;i++){
			strcat(outputBuffer," ");
		}
	}
}
/************************************************************/
/* Send Telegram Message                                    */
/* input: Text of sending Message                           */
/* return: -                                                */
/************************************************************/
void SendTelegramText(char *text){
	CURL *curl;
	CURLcode res;
	char   url[256];

	printf("SendTelegramText: %s\n", text);
	// build the URL string
	snprintf(url, sizeof(url)-1, "https://api.telegram.org/bot%s/sendMessage?chat_id=%d&text=%s", TOKEN, CHATID, text);

	// guarantee null termination of string
	url[sizeof(url)-1] = 0;

	curl = curl_easy_init();
	if(curl) {
		curl_easy_setopt(curl, CURLOPT_URL, url);
		printf(url);

		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl);
		/* Check for errors */
		if(res != CURLE_OK){
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
			curl_easy_strerror(res));
		}
	/* always cleanup */
	curl_easy_cleanup(curl);
	}
	return;
}
/************************************************************/
/* Calculate time: week, day, hour, seconds                 */
/* input:  secondes                                         */
/* return: is time in char                                  */
/************************************************************/
void TimeCalculation(int seconds, char *output){
	int week=0, day=0, hours=0, minutes=0;
	//char ctime[50];
	//60 sek = 1 min; 3600 sek = 1 h; 86400 sek = 1Tag; 604800sek = 1 Woche
	if(seconds >= 604800){
		week=seconds/604800;
		seconds=seconds%604800;
	}
	if(seconds >= 86400){
		day=seconds/86400;
		seconds=seconds%86400;
	}
	if(seconds >= 3600){
		hours=seconds/3600;
		seconds=seconds%3600;
	}
	if(seconds >= 60){
		minutes=seconds/60;
		seconds=seconds%60;
	}
	/*Convert integer into char for Values*/
	sprintf(output, "%dweeks, %ddays, %dhours, %dminutes, %dseconds", week,day,hours,minutes,seconds);
	return;
}
/************************************************************/
/* Read out file and return integer value                   */
/* input: Filename + path                                   */
/* return: result in integer                                */
/************************************************************/
int ReadFileContentInt(char *filename){
	FILE *datei;
	int result = -1;
	datei=fopen(filename,"r"); //open file
	if(datei == NULL) {
		printf("File not found: %s.\n", filename);
		return -1;
	}
	fscanf(datei, "%d\n", &result); //read value in file and write to variable
	fclose(datei); //close file
	return result;
}
/************************************************************/
/* save char to file                                        */
/* input: Filename + path                                   */
/* return: -                                                */
/************************************************************/
void SaveFileContentChar(char *filename, char *content) {
	FILE *datei;
	datei=fopen(filename,"w"); //open file
	if(datei == NULL) {//File could not open
		printf("File not found: %s\n", filename);
		fprintf(datei, "%s\n", content);
	}
	else{//open file
		fprintf(datei, "%s\n", content);
	}
	fclose(datei); //close file
	return;
}
/************************************************************/
/* save integer to file                                     */
/* input: Filename + path                                   */
/* return: -                                                */
/************************************************************/
void SaveFileContentInt(char *filename, int content) {
	FILE *datei;
	datei=fopen(filename,"w"); //open file
	if(datei == NULL) {//File could not open
		printf("File not found: %s\n", filename);
		fprintf(datei, "%d\n", content);
	}
	else{//open file
		fprintf(datei, "%d\n", content);
	}
	fclose(datei); //close file
	return;
}
/************************************************************/
/* save 1 float to file                                     */
/* input: Filename + path + unit + float                    */
/* return: -                                                */
/************************************************************/
void SaveFileContentOneFloat(char *filename, char *unit, float content) {
	FILE *datei;
	datei=fopen(filename,"w"); //open file
	if(datei == NULL) {//File could not open
		printf("File not found: %s\n", filename);
		fprintf(datei, "label,value\n %s,%0.1f\n", unit, content);
	}
	else{//open file
		fprintf(datei, "label,value\n %s,%0.1f\n", unit, content);
	}
	fclose(datei); //close file
	return;
}
/************************************************************/
/* save 2 float to file                                     */
/* input: Filename + path + unit1 + float1 + unit2 + float2 */
/* return: -                                                */
/************************************************************/
void SaveFileContentTwoFloat(char *filename, char *unit1, char *unit2, float content1, float content2) {
	FILE *datei;
	datei=fopen(filename,"w"); //open file
	if(datei == NULL) {//File could not open
		printf("File not found: %s\n", filename);
		fprintf(datei, "label,value\n %s,%0.1f\n %s,%0.1f", unit1, content1, unit2, content2);
	}
	else{//open file
		fprintf(datei, "label,value\n %s,%0.1f\n %s,%0.1f", unit1, content1, unit2, content2);
	}
	fclose(datei); //close file
	return;
}
/************************************************************/
/* UpdateEMONCMS                                            */
/* input: json string                                       */
/* return: -                                                */
/************************************************************/
void uploadtoEMONCMS(char *json){
	CURL *curl;
	CURLcode res;
	char   url[256];

	// build URL string
	strcpy(url, "http://");
	strcat(url, EmonCMS_HOST);
	strcat(url, "/input/post?node=");
	strcat(url, EmonCMS_NODE);
	strcat(url, "&json=");
	strcat(url, json);
	strcat(url, "&apikey=");
	strcat(url, EmonCMS_APIKey);

	//snprintf(url, sizeof(url)-1, "https://%s/input/post?node=%s&json=%s&apikey=%s", EmonCMS_HOST, EmonCMS_NODE, json,EmonCMS_APIKey);
	// guarantee null termination of string
	url[sizeof(url)-1] = 0;

	printf("Update EmonCMS send URL:");
	printf("%s\n",url);
	/* get a curl handle */
	curl = curl_easy_init();
	if(curl) {

    /* the DEBUGFUNCTION has no effect until we enable VERBOSE */
    //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    /* example.com is redirected, so we tell libcurl to follow redirection */
    //curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

		//curl_easy_setopt(curl, CURLOPT_URL, "https://emoncms.org/input/post?node=ChickenFarm&json={power1:100,power2:200,power3:300}&apikey=4ee6b1214cc19d87f46910ba5e938295");
		curl_easy_setopt(curl, CURLOPT_URL, url);

		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl);
		printf("\n");
		/* Check for errors */
		if(res != CURLE_OK){
			fprintf(stderr, "curl_easy_perform() failed: %s\n",
			curl_easy_strerror(res));
		}
	/* always cleanup */
	curl_easy_cleanup(curl);
	}
	curl_global_cleanup();
	printf("**********************************************************\n");
	return;
}
