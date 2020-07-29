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

int nge_beat[] = {2,2,2,1,1,1,1,1,1,1,1,2,2,2,2,1,1,1,1,1,1,2,4};

int button_state = 0; // 0 not pressed, 1 (<300ms)short press, 2 (<3000ms)long press , 3 (<6000ms) press, 4 (>6000ms) press
unsigned long last_press = 0;
unsigned long hold_press = 0;
int game_duration = 10; //in mins

void writeOled(char* string){
	ssd1306_clearDisplay();
	ssd1306_setTextSize(1);
	ssd1306_drawString(string);
	ssd1306_display();
}

void standbyBuzzer(){
	pinMode(BUZZER,OUTPUT);
	delay(10);
	digitalWrite(BUZZER,HIGH);
}




int activeBuzzer(){
	if(softToneCreate(BUZZER) == -1){
		printf("Soft Tone Failed \n");
		return 1;
	}
	return 0;
}

void beep(int note){
	activeBuzzer();
	softToneWrite(BUZZER,note);
	delay(150);
	standbyBuzzer();
}

void playEvangelion(){
	activeBuzzer();
	int i = 0;
	for(i = 0; i < sizeof(nge)/4; i++){
		softToneWrite(BUZZER,nge[i]);
		delay(nge_beat[i] * 250);
		softToneWrite(BUZZER,0);
		delay(25);
	}
	standbyBuzzer();
}



static void handleInterrupt(){
	if(!hold_press)
		hold_press = millis();
	unsigned long duration;
	static unsigned long button_pressed_timestamp;
	int level = digitalRead(2);
	if (level == HIGH) { 
		button_pressed_timestamp = millis();
	}
	else{
		duration = millis() - button_pressed_timestamp;
		button_pressed_timestamp = millis();
		if(duration < 40){
			return; //debounce
		}
		else if( duration < 300){ //Short Press
			button_state = 1;
			last_press = millis();
			beep(DO);
			hold_press = 0;
		}
		else if( duration < 3000){ //Long Press
			button_state = 2;
			last_press = millis();
			beep(RE);
			hold_press = 0;
		}
		else if( duration < 6000){ //Very Long Press
			button_state = 3;
			last_press = millis();
			beep(MI);
			hold_press = 0;
		}
		else if( duration >= 6000){
			button_state = 4;
			last_press = millis();
			beep(MI);
			hold_press = 0;
		}
	}
}

void stub(){
	//DOES NOTHING
}

void startTimer(){
	unsigned long time_expl = millis() + (game_duration * 60 * 1000); //Time explosion
	char output[100];
	long time_left;
	unsigned long last_beep = millis();
	hold_press = 0; //Reset button state
	button_state = 0;
	int exploded = 0;
	while(button_state < 4 && !exploded){
		time_left = (long)(time_expl - millis());
		int mins = (int)(time_left / 1000 / 60);
		int secs = (int)(time_left / 1000 - (mins*60));
		int mill = (int)(time_left - (mins*60*1000 + secs*1000));
		if(hold_press != 0 ){
			if(6 - (int)((millis() - hold_press)/1000) < 0)
				break;
			else{
				sprintf(output,"Bomba Attiva!\n - %2d:%2d:%3d\nPremere per disatt %d",mins,secs,mill,6 - (int)((millis() - hold_press)/1000));
				writeOled(output);
			}
			if(last_beep >= 100){
				beep(3500);
				last_beep = millis();
			}
		}else{
			if(time_left >= 60000){
				if(millis() - last_beep >=3000){
					beep(2000);
					last_beep = millis();
				}
			}
			else if(time_left < 60000 && time_left >= 10000){
				if(millis() - last_beep >= 1000){
					beep(2000);
					last_beep = millis();
				}
			}
			else{
				if(millis() - last_beep >= 500){
				beep(2000);
				last_beep = millis();
				}
			}
			sprintf(output,"Bomba Attiva!\n - %2d:%2d:%2d\nPremere per disatt 6",mins,secs,mill);
			writeOled(output);
		}
		if(time_left <= 0){
			exploded = 1;
			break;
		}
	}
	wiringPiISR(2,INT_EDGE_BOTH,&stub);
	hold_press = 0;
	button_state = 0;
	if(exploded){
		writeOled("~POW!~\nLa bomba e' esplosa\n");
		softToneWrite(BUZZER,3000);
		delay(5000);
		standbyBuzzer();
		wiringPiISR(2,INT_EDGE_BOTH,&handleInterrupt);
		while(button_state < 3){
			if(hold_press != 0 ){
				if(3 - (int)((millis() - hold_press)/1000) < 0)
				 	writeOled("Rilasciare il pulsante");
				else{
					sprintf(output,"Bomba Esplosa\nPremere il pulsante \nper almeno 3 secondi\nper resettare      %d",3 - (int)((millis() - hold_press)/1000));
					writeOled(output);
				}
			}else{
				writeOled("Bomba Esplosa\nPremere il pulsante \nper almeno 3 secondi\nper resettare      -");
			}
		}
	}else{
		writeOled("La bomba e' stata\n disinnescata ");
		playEvangelion();
		standbyBuzzer();
		wiringPiISR(2,INT_EDGE_BOTH,&handleInterrupt);
		while(button_state < 3){
			if(hold_press != 0 ){
				if(3 - (int)((millis() - hold_press)/1000) < 0)
				 	writeOled("Rilasciare il pulsante");
				else{
					sprintf(output,"Bomba Disinnescata\nPremere il pulsante \nper almeno 3 secondi\nper resettare      %d",3 - (int)((millis() - hold_press)/1000));
					writeOled(output);
				}
			}else{
				writeOled("Bomba Disinnescata\nPremere il pulsante \nper almeno 3 secondi\nper resettare      -");
			}
		}

	}
	standbyBuzzer();
}



void menu(){
	last_press = millis();	
	unsigned long temp_time = last_press;
	char* gametime = "10m\t20m\t30m\t60m\n + \t   \t   \t  ";
	while((millis() - last_press) < 5000){ //Timeout 5 seconds
		if(last_press != temp_time){
			switch(game_duration){
				case 10: game_duration = 20;break;
				case 20: game_duration = 30;break;
				case 30: game_duration = 60;break;
				default: game_duration = 10;break;
			}
			temp_time = last_press;
		}
		switch(game_duration){
			case 10: gametime = "10m\t20m\t30m\t60m\n + \t   \t   \t  ";break;
			case 20: gametime = "10m\t20m\t30m\t60m\n   \t + \t   \t  ";break;
			case 30: gametime = "10m\t20m\t30m\t60m\n   \t   \t + \t  ";break;
			case 60: gametime = "10m\t20m\t30m\t60m\n   \t   \t   \t +";break;
			default: gametime = "10m\t20m\t30m\t60m\n + \t   \t   \t  ";break;
		}
		char output[100];
		sprintf(output,"Tempo di gioco: \n%s\nScegliere entro: %lds",gametime,5-(millis() - last_press)/1000);
		writeOled(output);
	}
	char selection[100];
	sprintf(selection,"\n%d Minuti Selezionati\n",game_duration);
	writeOled(selection);
	beep(2000);
	delay(2000);
}

int main() {
	//setup pins and other stuff
	wiringPiSetup();
	ssd1306_begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);
	//Interrupt for button
	wiringPiISR(2,INT_EDGE_BOTH,&handleInterrupt);
	standbyBuzzer();
	char output[100];
	while(button_state < 3){
		if(hold_press != 0 ){
			if(3 - (int)((millis() - hold_press)/1000) < 0)
			 	writeOled("Rilasciare il pulsante");
			else{
				sprintf(output,"~~Bomba Softair~~\nPremere il pulsante \nper almeno 3 secondi\nper iniziare      %d",3 - (int)((millis() - hold_press)/1000));
				writeOled(output);
			}
		}else{
			writeOled("~~Bomba Softair~~\nPremere il pulsante \nper almeno 3 secondi\nper iniziare      -");
		}
		
	}
	menu();
	startTimer();
	return 0;
}

