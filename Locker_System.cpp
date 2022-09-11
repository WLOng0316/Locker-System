//
// 
//Embedded System Project
// Name:Ong Wei Ling (19001359)
// Name:Fatin Nur Afiqah(20001565)
//
#include <stdio.h>																											 
#include "NUC1xx.h"
#include "SYS.h"
#include "UART.h"
#include "GPIO.h"
#include "Scankey.h"
#include "LCD.h"
#include <string.h>
#include "ADC.h"

volatile uint8_t gu8AdcIntFlag;

void AdcIntCallback(uint32_t u32UserData)
{
    gu8AdcIntFlag = 1;	
}
//Declaration for register 2
int i;
char TEXTREG2[4][16];
char TEXTREG2_1[4][16];
uint16_t adc_value[8];
char TEXTOverrideUnlocked[100]="Overide Unlocked!";


// Declaration and intialization of variables or constants used
int cnt,s,select;
int hour,minute,second=0;
char TEXT0[16]="Lockers";
uint8_t  read_buf[4];
uint8_t  write_buf[1];


// Declaration of functions
void HomePage();
void Unlockedpage();
void LockedPage();
void select_Unlock();
void Register_1();
void Register_2();
void Unlock_1();
void WrongPass();
void EINT1Callback(void);

#define  ONESHOT  0   // counting and interrupt when reach TCMPR number, then stop
#define  PERIODIC 1   // counting and interrupt when reach TCMPR number, then counting from 0 again
#define  TOGGLE   2   // keep counting and interrupt when reach TCMPR number, tout toggled (between 0 and 1)
#define  CONTINUOUS 3 // keep counting and interrupt when reach TCMPR number

static uint16_t Timer1Counter=0;



//---------------------------------------------------------------------------------TIMER
void InitTIMER1(void)
{
	/* Step 1. Enable and Select Timer clock source */          
	SYSCLK->CLKSEL1.TMR1_S = 0;	//Select 12Mhz for Timer1 clock source 
    SYSCLK->APBCLK.TMR1_EN =1;	//Enable Timer1 clock source

	/* Step 2. Select Operation mode */	
	TIMER1->TCSR.MODE=PERIODIC;		//Select periodic mode for operation mode

	/* Step 3. Select Time out period = (Period of timer clock input) * (8-bit Prescale + 1) * (24-bit TCMP)*/
	TIMER1->TCSR.PRESCALE=255;	// Set Prescale [0~255]
	TIMER1->TCMPR = 46875;		// Set TCMPR [0~16777215]								
								// (1/12000000)*(255+1)*46875 = 1 sec / 1 Hz

	/* Step 4. Enable interrupt */
	TIMER1->TCSR.IE = 1;
	TIMER1->TISR.TIF = 1;		//Write 1 to clear for safty		
	NVIC_EnableIRQ(TMR1_IRQn);	//Enable Timer1 Interrupt

	/* Step 5. Enable Timer module */
	TIMER1->TCSR.CRST = 1;		//Reset up counter
	TIMER1->TCSR.CEN = 1;		//Enable Timer1

//  	TIMER1->TCSR.TDR_EN=1;		// Enable TDR function
}

void TMR1_IRQHandler(void) // Timer1 interrupt subroutine 
{
	
	// Timer used to show the time locked
	
	char TEXT8[16];
	
	Timer1Counter+=1;
	if(Timer1Counter == 60) {
		minute++;
		if (minute==60) {
			hour++;
			minute=0;
		}
		Timer1Counter=0;
	}
	sprintf(TEXT8,"%2d:%2d:%2d",hour,minute,Timer1Counter);
	print_Line(2, TEXT8);
 	TIMER1->TISR.TIF =1; 	   

}



// Initial GPIO pins (GPA 12,13,14) to Output mode  
void Init_LED()
{
	// initialize GPIO pins
	DrvGPIO_Open(E_GPA, 12, E_IO_OUTPUT); // GPA12 pin set to output mode
	DrvGPIO_Open(E_GPA, 13, E_IO_OUTPUT); // GPA13 pin set to output mode
	DrvGPIO_Open(E_GPA, 14, E_IO_OUTPUT); // GPA14 pin set to output mode
	// set GPIO pins output Hi to disable LEDs
	DrvGPIO_SetBit(E_GPA, 12); // GPA12 pin output Hi to turn off Blue  LED
	DrvGPIO_SetBit(E_GPA, 13); // GPA13 pin output Hi to turn off Green LED
	DrvGPIO_SetBit(E_GPA, 14); // GPA14 pin output Hi to turn off Red   LED
} 

// Initial GPIO pins (GPC 12,13,14,15) to Output mode  
void Init_LED2()
{
	// initialize GPIO pins
	DrvGPIO_Open(E_GPC, 12, E_IO_OUTPUT); // GPC12 pin set to output mode
	DrvGPIO_Open(E_GPC, 13, E_IO_OUTPUT); // GPC13 pin set to output mode
	DrvGPIO_Open(E_GPC, 14, E_IO_OUTPUT); // GPC14 pin set to output mode
	DrvGPIO_Open(E_GPC, 15, E_IO_OUTPUT); // GPC15 pin set to output mode
	// set GPIO pins to output Low
	DrvGPIO_SetBit(E_GPC, 12); // GPC12 pin output Hi to turn off LED
	DrvGPIO_SetBit(E_GPC, 13); // GPC13 pin output Hi to turn off LED
	DrvGPIO_SetBit(E_GPC, 14); // GPC14 pin output Hi to turn off LED
	DrvGPIO_SetBit(E_GPC, 15); // GPC15 pin output Hi to turn off LED
}

void RFID() {
	
	// Read the card scanned by user if the Lockers into the Emergebcy Locked status
	
	char TEXTRFID[16]="Unlocked by";
	char TEXTRFID2[16]="Admin";
	char TEXTtrial[16]="SN.             ";
	
	//print_Line(3,TEXTRFID2); // print Admin
	
  while(1) {
		//DrvGPIO_ClrBit(E_GPC, 15); // output Low to turn on LED
		DrvUART_Read (UART_PORT1, read_buf, 4);
		sprintf(TEXTtrial+3,"%2x%2x%2x%2x",read_buf[0],read_buf[1],read_buf[2],read_buf[3]);
		clear_LCD();
		print_Line(3,TEXTtrial);	

		// if the card number read is same as 2000 or 0000 or 2100 then will be unlocked 

		if((read_buf[0]==2 && read_buf[1]==0 && read_buf[2]==0 && read_buf[3]==0) || (read_buf[0]==0 && read_buf[1]==0 && read_buf[2]==0 && read_buf[3]==0) || (read_buf[0]==2 && read_buf[1]==1 && read_buf[2]==0 && read_buf[3]==0)) {
			DrvGPIO_SetBit(E_GPB,11); // GPB11 = 1 to turn off Buzzer	
			//DrvGPIO_SetBit(E_GPC, 15); // output high to turn off LED
			// Buzzer beep 2 sounds
			DrvGPIO_ClrBit(E_GPB,11); // GPB11 = 0 to turn on Buzzer
			DrvSYS_Delay(100000);	    // Delay 
			DrvGPIO_SetBit(E_GPB,11); // GPB11 = 1 to turn off Buzzer	
			DrvSYS_Delay(100000);	    // Delay 
			DrvGPIO_ClrBit(E_GPB,11); // GPB11 = 0 to turn on Buzzer
			DrvSYS_Delay(100000);	    // Delay 
			DrvGPIO_SetBit(E_GPB,11); // GPB11 = 1 to turn off Buzzer	
			DrvSYS_Delay(100000);	    // Delay 
			clear_LCD();
			print_Line(0,TEXT0); // print title	
			print_Line(1,TEXTRFID); // print Unlocked by
			print_Line(2,TEXTRFID2); // print Admin
			TIMER1->TCSR.CEN = 0;		//Close Timer1
			// set RGBled to Green
			DrvGPIO_SetBit(E_GPA,12); 
			DrvGPIO_ClrBit(E_GPA,13); // GPA13 = Green, 0 : on, 1 : off
			DrvGPIO_SetBit(E_GPA,14); 
			DrvSYS_Delay(100000);	    // Delay 
		}			
		//sprintf(TEXT1+3,"%2x%2x%2x%2x",read_buf[0],read_buf[1],read_buf[2],read_buf[3]);
		//print_Line(1,TEXT1);	
	}


}




void LockedPage(int numArray[4]) {
	
	
	// Intialise counter
	char TEXTUnlock[16] = "Correct!";
	char TEXTUnlock2[16] = "Unlocked!";
	char TEXTUnlockWrong[16] = "Wrong Passcode!"; //later add on!!
	//char TEXTUnlockWrongTrial[16] = "Trial =  ";
	int i;
	int check=0;
	int trial=4;
	
	int number,counter,loop;
	int checkpass[4];
	
	char TEXT3[16]="Time Locked: "; //! ! can be change to timer ! !
	char TEXT4[16]="Unlock: ";
	char TEXT5[16]=""; 
	char TEXT6[16];
	
	cnt=0;
	loop=1;
	//s=0;
	
	// LCD print Passcode set
	clear_LCD();
	print_Line(0,TEXT0); // print title	
	print_Line(1,TEXT3); // print second line
			
	// RGB Blue light light up indicates that the passcode register successfull
	DrvGPIO_ClrBit(E_GPA,12); // GPA12 = Blue,  0 : on, 1 : off
	DrvGPIO_SetBit(E_GPA,13); 
	DrvGPIO_SetBit(E_GPA,14); 
	// Beep 1 sound 
	DrvGPIO_ClrBit(E_GPB,11); // GPB11 = 0 to turn on Buzzer
	DrvSYS_Delay(100000);	    // Delay 
	DrvGPIO_SetBit(E_GPB,11); // GPB11 = 1 to turn off Buzzer	
	DrvSYS_Delay(100000);	    // Delay 
	
	DrvGPIO_ClrBit(E_GPC, 12); // output Low to turn on LED
	DrvGPIO_ClrBit(E_GPC, 13); // output Low to turn on LED
	DrvGPIO_ClrBit(E_GPC, 14); // output Low to turn on LED
	DrvGPIO_ClrBit(E_GPC, 15); // output Low to turn on LED
	
	// Timer 
	InitTIMER1();
	
	select = 0;
	counter = 0;
	
	while(loop) {
		
		// Scan tghe number entered from user if the user wanna to enter the registered 4 digit passcode to unlock 
		
		number = ScanKey();
		DrvSYS_Delay(500000); 
		
		if(number!=0 && counter==0) {
			checkpass[0] = number;
			sprintf(TEXT6,"%d",checkpass[0]); 
			print_Line(3, TEXT6);      
			counter++;
		}
		else if (number!=0 && counter==1) {
			checkpass[1] = number;
			sprintf(TEXT6,"%d%d",checkpass[0], checkpass[1]); 
			print_Line(3, TEXT6);     
			counter++;
		}
		else if (number!=0 && counter==2) {
			checkpass[2] = number;
			sprintf(TEXT6,"%d%d%d",checkpass[0], checkpass[1], checkpass[2]); 
			print_Line(3, TEXT6);      
			counter++;
		}
		else if (number!=0 && counter==3) {
			checkpass[3] = number;
			sprintf(TEXT6,"%d%d%d%d",checkpass[0], checkpass[1], checkpass[2], checkpass[3]); 
			print_Line(3, TEXT6);     
			
			counter++;

			
			
			// Check the passcode entered correct or not
			// -> if enter correctly -> RGB LED to Green -> beep 2 sounds
			// -> if enetred incorrectly -> go to function WrongPasss()
			
			s=0;
			while(s==0) {
				for(i=0;i<4;i++) {
					if(numArray[i]==checkpass[i]){
						check++;
					}
					
				}		
				
				if(check==4) {
					clear_LCD();
					print_Line(0,TEXT0); // print title	
					print_Line(1,TEXTUnlock); // print second line
					print_Line(3,TEXTUnlock2); // print third line
					// Buzzer beep 2 sounds
					DrvGPIO_ClrBit(E_GPB,11); // GPB11 = 0 to turn on Buzzer
					DrvSYS_Delay(100000);	    // Delay 
					DrvGPIO_SetBit(E_GPB,11); // GPB11 = 1 to turn off Buzzer	
					DrvSYS_Delay(100000);	    // Delay 
					DrvGPIO_ClrBit(E_GPB,11); // GPB11 = 0 to turn on Buzzer
					DrvSYS_Delay(100000);	    // Delay 
					DrvGPIO_SetBit(E_GPB,11); // GPB11 = 1 to turn off Buzzer	
					DrvSYS_Delay(100000);	    // Delay 
						// set RGBled to Green
					DrvGPIO_SetBit(E_GPA,12); 
					DrvGPIO_ClrBit(E_GPA,13); // GPA13 = Green, 0 : on, 1 : off
					DrvGPIO_SetBit(E_GPA,14); 
					//DrvSYS_Delay(1000000);
					// Close all the Red LED
					DrvGPIO_SetBit(E_GPC, 12); // output High to turn off LED
					DrvGPIO_SetBit(E_GPC, 13); // output High to turn off LED
					DrvGPIO_SetBit(E_GPC, 14); // output High to turn off LED
					DrvGPIO_SetBit(E_GPC, 15); // output High to turn off LED
					
					//Stop Timer
					TIMER1->TCSR.CEN = 0;		//Enable Timer1
					s=1;
				}
				else if (check!=4) {
					clear_LCD();
					print_Line(0,TEXT0); // print title	
					//print_Line(1,TEXTUnlockWrong); // print wrong passcode
					trial--;
					print_Line(1,TEXT3); // print second line
					WrongPass(trial);		
					s=1;
					counter = 0;
				
				}
			}
			

			
			// Print on LCD
		}
	}
	
	
	

}


void WrongPass(int trial) {
	
	
	// If passcode entered wrongly -> 4 LEDs turn off accordingly 
	
	
		char TEXTWrongPass[16] = "Emergency LOCKED";
	
		if(trial==3) {
			DrvGPIO_ClrBit(E_GPC, 12); // output Low to turn on LED
			DrvGPIO_ClrBit(E_GPC, 13); // output Low to turn on LED
			DrvGPIO_ClrBit(E_GPC, 14); // output Low to turn on LED
			DrvGPIO_SetBit(E_GPC, 15); // output Low to turn on LED
		}
		else if (trial==2){
			DrvGPIO_ClrBit(E_GPC, 12); // output Low to turn on LED
			DrvGPIO_ClrBit(E_GPC, 13); // output Low to turn on LED
			DrvGPIO_SetBit(E_GPC, 14); // output Low to turn on LED
			DrvGPIO_SetBit(E_GPC, 15); // output Low to turn on LED
		}
		else if (trial==1){
			DrvGPIO_ClrBit(E_GPC, 12); // output Low to turn on LED
			DrvGPIO_SetBit(E_GPC, 13); // output Low to turn on LED
			DrvGPIO_SetBit(E_GPC, 14); // output Low to turn on LED
			DrvGPIO_SetBit(E_GPC, 15); // output Low to turn on LED
		} 
		else if (trial==0){
			DrvGPIO_SetBit(E_GPC, 12); // output Low to turn on LED
			DrvGPIO_SetBit(E_GPC, 13); // output Low to turn on LED
			DrvGPIO_SetBit(E_GPC, 14); // output Low to turn on LED
			DrvGPIO_SetBit(E_GPC, 15); // output Low to turn on LED
			// set RGBled to Red
			DrvGPIO_SetBit(E_GPA,12); 
			DrvGPIO_SetBit(E_GPA,13); 
			DrvGPIO_ClrBit(E_GPA,14); // GPA14 = Red,   0 : on, 1 : off
			
			//beep nonstop
			//DrvGPIO_ClrBit(E_GPB,11); // GPB11 = 0 to turn on Buzzer (!!!!!!)
			//DrvSYS_Delay(1000000);
			
			//print Emergency lock on LCD
			clear_LCD();
			print_Line(0,TEXT0);
			print_Line(1,TEXTWrongPass);
			RFID();
		} 
	
}

void Register_1() {
	
// Function 1: Register Passcode (Request User to enter 4 digit passcode, if the 4 digit passcode registered, RGB Blue light will light up indicates passcode register successfull)
		
		int8_t number, counter;
		int numArray[4];
		int number1, number2;
	
		char TEXT1[16]="Register";
		char TEXT2[16]="Passcode";
		
		
		clear_LCD();
	
		print_Line(0,TEXT0); // print title	
		print_Line(1,TEXT1); // print second line
		print_Line(2,TEXT2); // print third line
		counter = 0;	
		number=0;
		
	while(1) {
		
		// scan the number entered from keypad (only 4 digits will be accepted)
		
		number = ScanKey();
		DrvSYS_Delay(500000); 
		
		if(number!=0 && counter==0) {
			numArray[0] = number;
			sprintf(TEXT2,"%d",numArray[0]); 
			print_Line(3, TEXT2);      
			counter++;
		}
		else if (number!=0 && counter==1) {
			numArray[1] = number;
			sprintf(TEXT2,"%d%d",numArray[0], numArray[1]); 
			print_Line(3, TEXT2);     
			counter++;
		}
		else if (number!=0 && counter==2) {
			numArray[2] = number;
			sprintf(TEXT2,"%d%d%d",numArray[0], numArray[1], numArray[2]); 
			print_Line(3, TEXT2);      
			counter++;
		}
		else if (number!=0 && counter==3) {
			numArray[3] = number;
			sprintf(TEXT2,"%d%d%d%d",numArray[0], numArray[1], numArray[2], numArray[3]); 
			print_Line(3, TEXT2);      
			//DrvSYS_Delay(100000);	    // Delay 
			counter++;
			LockedPage(numArray);
		}
		

	
	}

}

void Register_2() {
	// Function 2: For Admin to Override the locker unlock system
	//Admin will have to adjust the value of the 2 variable resistor to its correct range respectively. Then once, its done
  // admin will have to enter keypad no 9 to unlock the locker system	
	int8_t UnlockedPin,PinCounter;
	
	clear_LCD();
	
	
	gu8AdcIntFlag =0;      // reset AdcIntFlag
 	DrvADC_StartConvert(); // start ADC sampling
	UnlockedPin=0;
	PinCounter=0;
	
	while(1) {	
		
		UnlockedPin = ScanKey();

		i = 7;                 // ADC channel = 7 
	
		while(ADC->ADSR.ADF==0) // wait till conversion flag = 1, conversion is done
		ADC->ADSR.ADF=1;		     // write 1 to clear the flag
		adc_value[i] = DrvADC_GetConversionData(i);   // read ADC value from ADC registers
 		sprintf(TEXTREG2[2],"PASSCODE 1 :%4d",adc_value[i]); // convert ADC value into text
		print_Line(1, TEXTREG2[2]);
		
		i = 1;                     // ADC channel = 1 
		
		while(ADC->ADSR.ADF==0) // wait till conversion flag = 1, conversion is done
		ADC->ADSR.ADF=1;		     // write 1 to clear the flag
		adc_value[i] = DrvADC_GetConversionData(i);   // read ADC value from ADC registers
 		sprintf(TEXTREG2_1[2],"PASSCODE 2 :%4d",adc_value[i]); // convert ADC value into text
		print_Line(2, TEXTREG2_1[2]);
		
		if(ADC->ADDR[7].RSLT<2000 && ADC->ADDR[7].RSLT>1000)       //range value for variable resistor 1
			{  
			if(ADC->ADDR[1].RSLT<4000 && ADC->ADDR[1].RSLT>3000)     //range value for variable resistor 2
			{
					if(UnlockedPin==9 && PinCounter==0)                 //once both value at its range, then press kepad no 9 to unlock
						{
							//PinCounter++;
								clear_LCD();
								print_Line(0,TEXTOverrideUnlocked); // print second line
								// Buzzer beep 2 sounds
								DrvGPIO_ClrBit(E_GPB,11); // GPB11 = 0 to turn on Buzzer
								DrvSYS_Delay(100000);	    // Delay 
								DrvGPIO_SetBit(E_GPB,11); // GPB11 = 1 to turn off Buzzer	
								DrvSYS_Delay(100000);	    // Delay 
								DrvGPIO_ClrBit(E_GPB,11); // GPB11 = 0 to turn on Buzzer
								DrvSYS_Delay(100000);	    // Delay 
								DrvGPIO_SetBit(E_GPB,11); // GPB11 = 1 to turn off Buzzer	
								DrvSYS_Delay(100000);	    // Delay 
								// set RGBled to Green
								DrvGPIO_SetBit(E_GPA,12); 
								DrvGPIO_ClrBit(E_GPA,13); // GPA13 = Green, 0 : on, 1 : off
								DrvGPIO_SetBit(E_GPA,14); 
								//DrvSYS_Delay(1000000);
								// Close all the Red LED
								DrvGPIO_SetBit(E_GPC, 12); // output High to turn off LED
								DrvGPIO_SetBit(E_GPC, 13); // output High to turn off LED
								DrvGPIO_SetBit(E_GPC, 14); // output High to turn off LED
								DrvGPIO_SetBit(E_GPC, 15); // output High to turn off LED
						}
					
						else{}
			}
			else{}
			
		}
		else{}
}
		
}



int32_t main (void)
{
	STR_UART_T sParam;
	//int8_t number, counter, select;
	int32_t numArray[4];
	int number1, number2;
	char TEXT0[16]="Lockers";
	char TEXT1[16]="Register";
	char TEXT2[16]="Select: 1 3";
	
	select =0;
	s=0;

	
	UNLOCKREG();
	DrvSYS_Open(50000000);
	LOCKREG();
	
	DrvADC_Open(ADC_SINGLE_END, ADC_CONTINUOUS_OP, 0xFF, INTERNAL_HCLK, 1);
  DrvADC_EnableADCInt(AdcIntCallback, 0);

	
	DrvGPIO_InitFunction(E_FUNC_UART1);	// Set UART pins

	/* UART Setting */
  sParam.u32BaudRate 		= 9600;
  sParam.u8cDataBits 		= DRVUART_DATABITS_8;
  sParam.u8cStopBits 		= DRVUART_STOPBITS_1;
  sParam.u8cParity 		  = DRVUART_PARITY_NONE;
  sParam.u8cRxTriggerLevel= DRVUART_FIFO_1BYTES;
	
	/* Set UART Configuration */
 	if(DrvUART_Open(UART_PORT1,&sParam) != E_SUCCESS);


	Init_LED();
	Init_LED2();
	init_LCD(); 
	clear_LCD();
	OpenKeyPad();
	
	// configure external interrupt pin GPB15
	DrvGPIO_EnableEINT1(E_IO_BOTH_EDGE, E_MODE_EDGE, EINT1Callback); // configure external interrupt
  write_buf[0] = 0x02;
  DrvUART_Write(UART_PORT1, write_buf, 1);

	
	while(1)
	{

		
			print_Line(0,TEXT0); // print title	
			print_Line(1,TEXT1); // print Register
		print_Line(2,TEXT2); // print Select: 1 3
	
	
			while (s==0) {
				select = ScanKey();
				DrvSYS_Delay(500000); 
				if(select==1) {
				// Go to Register: Function 1
					Register_1();
					s=1;
				}
				else if (select==3) {
					// Go to Register: Function 2
					Register_2();
					s=1;
				
				}
		}


		DrvSYS_Delay(500000); 		
	}
}

// External Interrupt Handler (INT button to trigger GPB15)
void EINT1Callback(void) 
{
	main();
}
