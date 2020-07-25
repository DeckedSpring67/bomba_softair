#include "ssd1306_i2c.h"
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <wiringPi.h>
#include <limits.h>
#include <softTone.h>
#define BUZZER 0

#define DO 523
#define REb 554
#define RE 587
#define MIb 622
#define MI 659
#define FA 698 
#define SOLb 739
#define SOL 783
#define LAb 830
#define LA 880
#define SIb 932
#define SI 987
#define DO8 1046


int nge[] = {DO,MIb,FA,MIb,RE,FA,FA,SIb,LAb,SOL,FA,SOL,SOL,SIb,
	DO8,FA,MIb,SIb,SIb,SOL,SIb,SIb,DO8};

int nge_beat[] = {2,2,2,1,1,1,1,1,1,1,1,2,2,2,2,1,1,1,1,1,1,1,2};

void handleInterrupt(){
	printf("Premuto pulsante\n");
	fflush(stdout);
}

void standbyBuzzer(){
	pinMode(BUZZER,OUTPUT);
	delay(10);
	digitalWrite(BUZZER,HIGH);
}

void setupTimer(){
}

int activeBuzzer(){
	if(softToneCreate(BUZZER) == -1){
		printf("Soft Tone Failed \n");
		return 1;
	}
	return 0;
}

int main() {
	//setup pins and other stuff
	wiringPiSetup();
	//Interrupt for button
	wiringPiISR(27,INT_EDGE_RISING,&handleInterrupt);
	printf("Bomba Partita\n");
	standbyBuzzer();
	activeBuzzer();
	int i = 0;
	for(i = 0; i < sizeof(nge)/4; i++){
		softToneWrite(BUZZER,nge[i]);
		delay(nge_beat[i] * 250);
		softToneWrite(BUZZER,0);
		delay(25);
	}
	standbyBuzzer();
	delay(1000);
	activeBuzzer();
	for(i = 0; i < sizeof(nge)/4; i++){
		softToneWrite(BUZZER,nge[i]);
		delay(nge_beat[i] * 250);
		softToneWrite(BUZZER,0);
		delay(25);
	}
	standbyBuzzer();
	ssd1306_begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);
	ssd1306_clearDisplay();
	ssd1306_setTextSize(1);
	ssd1306_drawString("Bomba inizializzata");
	ssd1306_display();
	ssd1306_clearDisplay();
	return 0;
}

