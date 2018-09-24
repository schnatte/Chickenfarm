#include <wiringPi.h>          
#include <lcd.h>               
#include <stdio.h>
#include <time.h>
 
//USE WIRINGPI PIN NUMBERS
#define LCD_RS  10               //Register select pin grün
#define LCD_E   11               //Enable Pin gelb
#define LCD_D4  12               //Data pin 4
#define LCD_D5  13               //Data pin 5
#define LCD_D6  14               //Data pin 6
#define LCD_D7  15               //Data pin 7
 
int main()
{
    	int lcd;               
    	wiringPiSetup();        
    	lcd = lcdInit (4, 20, 4, LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7, 0, 0, 0, 0);
    	lcdClear(lcd);
    	lcdPosition(lcd, 0, 0);               
    	//lcdPuts(lcd,"Chickenfarm V1.0");//Put later on the time behind   
   	while(1){
        	time_t timer;
        	char buffer_date[26];
        	char buffer_time[26];
	        struct tm* tm_info;

	        time(&timer);
        	tm_info = localtime(&timer);

	        strftime(buffer_date, 26, "Date: %m:%d:%Y", tm_info);
	        strftime(buffer_time, 26, "Time: %H:%M:%S", tm_info);

        	lcdPosition(lcd, 0, 1);
	        lcdPuts(lcd, buffer_date);

	        lcdPosition(lcd, 0, 2);
	        lcdPuts(lcd, buffer_time);

	}
}

