#include "mbed.h"
#include "LCD.h"
lcd mylcd;

//Sobald an PC_8 eine steigende Flank kommt wird der Counter zurückgesetzt
//Sobald an PC_8 eine fallende Flanke kommt, endet die Zeitmessung
//Es wird die Zeit = Counterstand zwischen zwei fallenden Flanken als laufzeit aufgenommen
//Ein Triggerimpuls wird erst nach dem Ende einer Messung gesendet. Dafür sorgt der Marker "fertig" in der while(fertig==0) und ISR

DigitalOut led(PC_0);		//LED an PC_0 als Ausgang
DigitalOut trigger(PA_10);	//Var. Trigger als Ausgang
InterruptIn echo(PC_8);		//Interrupteingang für den Start uns für das Ende der Messung
volatile int laufzeit,fertig=0;	//Variable laufzeit für die Ausgabe, Merkervariable fertig zum Abwarten bis eine Messung zu Ende ist


void initTimer(){		
    RCC->APB1ENR|=0b10000;	//TIM6 Init
    TIM6->PSC=31;		//Eine Mikrosekunde Countertakt
    TIM6->ARR=0xFFFF;		//Endwert 65535
    TIM6->CNT=0xFFFF;		//Counter Startwert 65535
    TIM6->SR=0;			//Überlaufflag auf 0
    TIM6->CR1=1;		//Timer starten
}
void startMessung(){		//ISR zur Messungsstart
    TIM6->CNT=0;		//TIM6-Counter zurück mit 0 starten
}
void endMessung(){		//ISR zum Messungsende
    laufzeit=TIM6->CNT;		//Var. laufzeit übernimmt den erreichten Counterstand in Mikrosekunden
    fertig=1;			//Merker auf 1
}

int main(){
    	initTimer();		//Initialisierung des TIM6

	echo.mode(PullUp);	//Interrupteingang-Startwert mit 0
    	echo.rise(&startMessung);	//Auslöser steigende Flanke für Start-Messung
    	echo.fall(&endMessung);		//Auslöser fallende Flanke für Ende-Messung
    	mylcd.clear();

    	while (true) {  
        	mylcd.cursorpos(0);			//Cursor an Pos. 0
       		mylcd.printf("%dcm   ",laufzeit/58);	//Ausgabe
        	
		fertig=0;	//Merker wieder auf 0
        	trigger=1;	//Eine 1 auf PA_10 ausgeben
        	
		TIM6->CNT=0;	//TIM6-Counter zurück mit 0 starten
        	
		while(TIM6->CNT<10);	//10 Counterschritte warten = 10 Mikrosekunden
        	trigger=0;		//Die 1 auf PA_10 wieder wegnehmen => 0		
        	while(fertig==0);	//Warten bis die fallende Flanke kommt
    }
}