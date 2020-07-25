#include "ssd1306_i2c.h"
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <wiringPi.h>
#include <limits.h>

//Gloabal button thingy
u_int8_t buttonPressed = 0;
time_t timeButtonPressed;


void handleInterrupt(){
	printf("Premuto pulsante\n");
	fflush(stdout);
}

void *refresh_display(void *args){
	return NULL;
}

int main() {
	//setup pins and other stuff
	wiringPiSetup();
	//Interrupt for button
	wiringPiISR(27,INT_EDGE_RISING,&handleInterrupt);
	
	printf("Bomba Partita\n");
	ssd1306_begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);
	ssd1306_clearDisplay();
	
	//Create thread ids
	pthread_t refresh_id; 
	pthread_t hightemps_id;
	pthread_t ac_id;
	pthread_t amb_id;
	pthread_t lowtemps_id;

	temps.timeLastSent = 0;

	//Add a slight delay after the first reading to prevent wrong ones
	pthread_create(&ac_id, NULL, getAcTemp, &temps);
	delay(1000);

	//create other variables
	time_t timeNow;
	double timeDiff;
	char text[300];
	time_t timeNowLog;
	timeLastLog = 0;

	while(1){
		if(!buttonPressed){
			pthread_create(&ac_id, NULL, getAcTemp, &temps);
			pthread_create(&amb_id, NULL, getAmbTemp, &temps);
			
			//Detach threads to avoid memory leak
			pthread_join(ac_id,NULL);
			pthread_join(amb_id,NULL);

			pthread_create(&refresh_id, NULL, refresh_display, &temps);
			pthread_detach(refresh_id);

				if(temps.ac > temps.maxAC || temps.amb > temps.maxAMB){	
					pthread_create(&hightemps_id, NULL, handle_HighTemp, &temps);
				}
				else if(temps.ac < (temps.maxAC - 1) && temps.amb < (temps.maxAMB - 1)){
					pthread_create(&lowtemps_id, NULL, handle_LowTemp, &temps);
				}
			pthread_detach(lowtemps_id);
			pthread_detach(hightemps_id);
		}
		else{
			time(&(timeNow));
			timeDiff = difftime(timeNow,timeButtonPressed);
			//When button has been pressed pause the program for 15 minutes
			// and show it on screen
			if(timeDiff < 900){
				sprintf(text,"Sistema in pausa\n%.0f sec. al riavvio\nPremere RESET per \nriavvio manuale\n",900-timeDiff);
				ssd1306_setTextSize(1);
				ssd1306_drawString(text);
				ssd1306_display();
				ssd1306_clearDisplay();
			}
			else{
				buttonPressed = 0;
			}
		}
		char logbody[400];
		time(&(timeNowLog));
	       	sprintf(logbody,"%f;%f;%s",temps.ac,temps.amb,temps.temp_alert);
		logPrintf(logbody,timeNowLog);
		delay(500);
	}
	return 0;
}

