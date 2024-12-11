#include<p30fxxxx.h>
#include <stdlib.h>
#include "driverGLCD.h"
#include "adc.h"
#include "timer2.h"
//#include <outcompare.h>

//_FOSC(CSW_FSCM_OFF & XT_PLL4);//instruction takt je isti kao i kristal//
_FOSC(CSW_ON_FSCM_OFF & HS3_PLL4);
_FWDT(WDT_OFF);
_FGS(CODE_PROT_OFF);

unsigned int X, Y,x_vrednost, y_vrednost, r;

//const unsigned int ADC_THRESHOLD = 900; 
const unsigned int AD_Xmin =220;
const unsigned int AD_Xmax =3642;
const unsigned int AD_Ymin =520;
const unsigned int AD_Ymax =3450;

unsigned int sirovi0,sirovi1,sirovi2;
unsigned int broj,broj1,broj2,temp0,temp1, pokret; 

//#define DRIVE_A PORTBbits.RB10
//#define DRIVE_B PORTCbits.RC13
#define DRIVE_A PORTCbits.RC13
#define DRIVE_B PORTCbits.RC14


void ConfigureTSPins(void)
{
    TRISBbits.TRISB12=0;
    ADPCFGbits.PCFG12=1;
	ADPCFGbits.PCFG10=0;
	ADPCFGbits.PCFG11=1; // definisemo digitalni
    TRISBbits.TRISB11=1;  //definisemo izlaz
	TRISBbits.TRISB10=1;   
	TRISCbits.TRISC13=0;
    TRISCbits.TRISC14=0;
	
	//LATCbits.LATC14=0;
	//LATCbits.LATC13=0;
}



void __attribute__((__interrupt__)) _U1RXInterrupt(void) 
{
    IFS0bits.U1RXIF = 0;
   // tempRX=U1RXREG;

} 

void WriteUART1(unsigned int data)
{
	  while(!U1STAbits.TRMT);

    if(U1MODEbits.PDSEL == 3)
        U1TXREG = data;
    else
        U1TXREG = data & 0xFF;
}

/***********************************************************************
* Ime funkcije      : WriteUART1dec2string                     		   *
* Opis              : Funkcija salje 4-cifrene brojeve (cifru po cifru)*
* Parameteri        : unsigned int data-podatak koji zelimo poslati    *
* Povratna vrednost : Nema                                             *
************************************************************************/
void WriteUART1dec2string(unsigned int data)
{
	unsigned char temp;

	temp=data/1000;
	WriteUART1(temp+'0');
	data=data-temp*1000;
	temp=data/100;
	WriteUART1(temp+'0');
	data=data-temp*100;
	temp=data/10;
	WriteUART1(temp+'0');
	data=data-temp*10;
	WriteUART1(data+'0');
}



void Delay(unsigned int N)
{
	unsigned int i;
	for(i=0;i<N;i++);
}

void Touch_Panel (void)
{
// vode horizontalni tranzistori
	DRIVE_A = 1;  
	DRIVE_B = 0;
    
     LATCbits.LATC13=1;
     LATCbits.LATC14=0;

	Delay(500); //cekamo jedno vreme da se odradi AD konverzija
				
	// ocitavamo x	
	x_vrednost = temp0;//temp0 je vrednost koji nam daje AD konvertor na BOTTOM pinu		

	// vode vertikalni tranzistori
     LATCbits.LATC13=0;
     LATCbits.LATC14=1;
	DRIVE_A = 0;  
	DRIVE_B = 1;

	Delay(500); //cekamo jedno vreme da se odradi AD konverzija
	
	// ocitavamo y	
	y_vrednost = temp1;// temp1 je vrednost koji nam daje AD konvertor na LEFT pinu	
	
//Ako želimo da nam X i Y koordinate budu kao rezolucija ekrana 128x64 treba skalirati vrednosti x_vrednost i y_vrednost tako da budu u opsegu od 0-128 odnosno 0-64
//skaliranje x-koordinate

    X=(x_vrednost-161)*0.03629;



//X= ((x_vrednost-AD_Xmin)/(AD_Xmax-AD_Xmin))*128;	
//vrednosti AD_Xmin i AD_Xmax su minimalne i maksimalne vrednosti koje daje AD konvertor za touch panel.


//Skaliranje Y-koordinate
	Y= ((y_vrednost-500)*0.020725);

//	Y= ((y_vrednost-AD_Ymin)/(AD_Ymax-AD_Ymin))*64;
}

void __attribute__((__interrupt__)) _T2Interrupt(void)
{
   	TMR2 =0;
    IFS0bits.T2IF = 0;
}

	
void __attribute__((__interrupt__)) _ADCInterrupt(void) 
{
							
	
	sirovi0=ADCBUF0;//0
	sirovi1=ADCBUF1;//1
    sirovi2=ADCBUF2;//2

	temp0=sirovi0;
	temp1=sirovi1;

    IFS0bits.ADIF = 0;
} 

void Write_GLCD(unsigned int data)
{
unsigned char temp;

temp=data/1000;
Glcd_PutChar(temp+'0');
data=data-temp*1000;
temp=data/100;
Glcd_PutChar(temp+'0');
data=data-temp*100;
temp=data/10;
Glcd_PutChar(temp+'0');
data=data-temp*10;
Glcd_PutChar(data+'0');
}

void main(void)
{
    
	ConfigureLCDPins();
	ConfigureTSPins();

	GLCD_LcdInit();

	GLCD_ClrScr();
    
	
	ADCinit();
	ConfigureADCPins();
	ADCON1bits.ADON=1;

	while(1)
        
	{ 
        if (PORTBbits.RB11==1){
            LATBbits.LATB12=0;
        }
        else {
            LATBbits.LATB12=1;
        }
        
	
	Touch_Panel ();
    
	GLCD_Rectangle(3,2,124,40);
	
	
	
		
      
       GoToXY(75,10);	   
		GLCD_Printf ("Smanji");
         GLCD_Rectangle(65,53,124,62);
		
		GLCD_Rectangle(65,37,124,41);
		GoToXY(75,5);	
		GLCD_Printf ("Povecaj");

		
	    GoToXY(0,6);
        GLCD_Printf ("X=");
        GoToXY(9,6);
        Write_GLCD(X);
	        
		GoToXY(0,7);
        GLCD_Printf ("Y=");
        GoToXY(9,7);
        Write_GLCD(Y);
    if ((1<X)&&(X<120)&& (17<Y)&&(Y<60))
        LcdSetDot(X,64-Y);

        if ((59<X)&&(X<111)&& (6<Y)&&(Y<15))
            {  
             
          	  GLCD_ClrScr();
            }
     
	}//while

}//main


			
		
												

