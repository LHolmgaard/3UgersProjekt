/*
 *
 *  Created on: 	Unknown
 *      Author:
 *     Version:		1.1
 */

/********************************************************************************************

* VERSION HISTORY
********************************************************************************************
* 	v1.1 - 01/05/2015
* 		Updated for Zybo ~ DN
*
*	v1.0 - Unknown
*		First version created.
*******************************************************************************************/

#include "xparameters.h"
#include "xgpio.h"
#include "xtmrctr.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "xil_printf.h"
#include <stdio.h>
#include <stdlib.h>
#include "xtime_l.h"
#include "string.h"
//#include "time.h"
#include "ctype.h"
#include <pthread.h>

// Parameter definitions
#define INTC_DEVICE_ID 		XPAR_PS7_SCUGIC_0_DEVICE_ID
#define TMR_DEVICE_ID		XPAR_TMRCTR_0_DEVICE_ID
#define BTNS_DEVICE_ID		XPAR_AXI_GPIO_0_DEVICE_ID
#define LEDS_DEVICE_ID		XPAR_AXI_GPIO_1_DEVICE_ID
#define SWTS_DEVICE_ID		XPAR_AXI_GPIO_2_DEVICE_ID
#define INTC_BTN_INTERRUPT_ID XPAR_FABRIC_AXI_GPIO_0_IP2INTC_IRPT_INTR
#define INTC_TMR_INTERRUPT_ID XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR
// til switches
#define INTC_SWT_INTERRUPT_ID XPAR_FABRIC_AXI_GPIO_2_IP2INTC_IRPT_INTR

#define XRtcPsu_ReadCurrentTime(InstancePtr) XRtcPsu_ReadReg((InstancePtr)->RtcConfig.BaseAddr+XRTC_CUR_TIME_OFFSET)

#define BTN_INT 			XGPIO_IR_CH1_MASK
#define SWT_INT 			XGPIO_IR_CH1_MASK
#define TMR_LOAD			100000000 // 100 millions

XGpio LEDInst, BTNInst, SWTInst;
XScuGic INTCInst;
XTmrCtr TMRInst;
//intially we start at regular timekeeping
static int led_value = 0;
static int btn_value;
//static int tmr_count;
double tmr_count;
unsigned int SWT_value;


//------------ VORES ARBEJDE
// Ur
int Ur_sekund = 0;
int Ur_minut = 0;
int Ur_time = 0;
int ur_aktiv = 1;


// Lommeregner
double result = 0;
double a=0;
double b=0;
int lommeregner_aktiv = 0;


// Virtuel sms
const char *listG[]={"Hej", "Hallo", "Halløj", "Goddag", "Yo", "heyhey"};
const char *listA[]={"har", "kan", "vil", "er"};
const char *listS[] = {"du", "Camilla", "i", "katten"};
const char *listV[] = {"lyst", "tid", "mulighed"};
const char *listB[] = {"til kaffe","til at spille fodbold", "en date"};

// Batteriniveau
unsigned int batteriniveau = 100;

// Stopur
int stopur_sekund = 0;
int stopur_minut = 0;
int stopur_time = 0;
int stopur_aktiv = 0;

// generelt
int state = 0; // denne variabel bruges til at angive hvilken state telefonen er i.
// state tilstande: 0 = startside/default state, 1 = lommeregner, 2 = TBD, 3 = stopur, 4 = Virtuel SMS, 5 = TBD
int ButtonCounter = 0;

// Edwards work
XTime tStart, tEnd, soundDemo_tEnd, soundDemo_tStart;


//----------------------------------------------------
// PROTOTYPE FUNCTIONS
//----------------------------------------------------
void SWT_Intr_Handler(void *baseaddr_p);
void BTN_Intr_Handler(void *baseaddr_p);
int InterruptSystemSetup(XScuGic *XScuGicInstancePtr);
int IntcInitFunction(u16 DeviceId, XTmrCtr *TmrInstancePtr, XGpio *GpioInstancePtr, XGpio *GpioInstancePtr_SWT );

// Below is Edwards work
void XTmrCtr_ClearInterruptFlag(XTmrCtr * InstancePtr, u8 TmrCtrNumber);
void TMR_Intr_Handler(void *InstancePtr, u8 TmrCtrNumber);

// Lommeregner
void lommeregner(void);
void addition(double a, double b);
void subtraction(double a, double b);
void mult(double a, double b);
void division(double a, double b);

//  Virtuel sms
void virtuelSMS(void);

// Batteriniveau
void batteriniveau_led(void);
void batteriBrugt(void);

// Stopur
void stopur(void);




void SWT_Intr_Handler(void *InstancePtr)
{

	XGpio_InterruptDisable(&SWTInst, SWT_INT);
	// Ignore additional switch changes
	if ((XGpio_InterruptGetStatus(&SWTInst) & SWT_INT) != SWT_INT) return;




	(void)XGpio_InterruptClear(&SWTInst, SWT_INT);
	// Enable GPIO interrupts
	XGpio_InterruptEnable(&SWTInst, SWT_INT);
}


void BTN_Intr_Handler(void *InstancePtr)
{


	double soundDemo_duration;
	// Disable GPIO interrupts
	//XGpio_InterruptDisable(&BTNInst, BTN_INT);
	// Ignore additional button presses
	if ((XGpio_InterruptGetStatus(&BTNInst) & BTN_INT) != BTN_INT) return;


	btn_value = XGpio_DiscreteRead(&BTNInst, 1); // read button value

	
	// dette er et fix pga ét tryk på knappen registreres 2 gange.
    ButtonCounter++;

    if (ButtonCounter == 1) xil_printf("Button interrupt kaldt\n");


	//testing
	//if (btn_value == 2 && ButtonCounter == 1)
	//{
	//	led_value = 15;
	//}
	//testing




	// Lommeregner
    if (btn_value == 1 && ButtonCounter == 1)
    {
    	state = 1; // gå til lommeregner tilstand
    	lommeregner();
    }

	//if (btn_value == 1 && ButtonCounter == 1)
	//{
	//	lommeregner_aktiv = 1;  // aktiver lommeregner
	//	lommeregner();
	//}


    // Knap 2 er tilbageknap for alle states
    if (btn_value == 2 && ButtonCounter == 1) state = 0;


	// Stopur
	if (btn_value == 4 && ButtonCounter == 1)
	{
		state = 3;
		stopur();
	}


	// Virtuel SMS
	if (btn_value == 8 && ButtonCounter == 1)
	{
		state = 4;
		virtuelSMS();
	}


	// til debug
	//xil_printf("Buttonvalue = %d\n", btn_value);



	if (ButtonCounter == 2) ButtonCounter = 0;

    (void)XGpio_InterruptClear(&BTNInst, BTN_INT);
    // Enable GPIO interrupts
    XGpio_InterruptEnable(&BTNInst, BTN_INT);
}


void TMR_Intr_Handler(void *InstancePtr, u8 TmrCtrNumber)
{
	double duration; // used to measure the real-time between the shown time
	static int tmr_count;
	XTime_GetTime(&tEnd);
	XTmrCtr* pTMRInst = (XTmrCtr *) InstancePtr;

	if (TmrCtrNumber==0)  //Handle interrupts generated by timer 0
	{
		//duration = ((double)(tEnd-tStart))/COUNTS_PER_SECOND;
		//printf("Tmr_interrupt, tmr_count= %d, duration=%.6f s\n\r", tmr_count, (double)duration);

		tStart=tEnd;

		if (XTmrCtr_IsExpired(pTMRInst,0)) // if true or has any value, execute if
		{
			

			batteriniveau_led();
			// Light up given LEDS
		    XGpio_DiscreteWrite(&LEDInst, 1, led_value);



		    // 'rigtige' tid som altid tælles op
			Ur_sekund++;

			if (Ur_sekund >= 60)
			{
				Ur_minut++;
				Ur_sekund = 0;
			}

			if (Ur_minut >= 60)
			{
				Ur_time++;
				Ur_minut = 0;
			}

			if (Ur_time >= 24)
			{
				Ur_time = 0;
			}


			// stopur tid
			if (state == 3)
			{
				stopur_sekund++;
				if (stopur_sekund >= 60)
					{
					stopur_minut++;
					}
				if (stopur_minut >= 60)
					{
						stopur_minut = 0;
						stopur_time++;
					}
				if (stopur_time >= 24)
					{
						stopur_time = 0;
					}

			}


			// Udskrivning af tid og batteri på skærmen
			if (state == 0)
			{
				xil_printf("%d %d %d\n", Ur_time, Ur_minut, Ur_sekund);
				xil_printf("Batteriniveau = %d\n", batteriniveau);
			}


			// Udskrivning af stopur tid
			if (state == 3) xil_printf("%d %d %d\n", stopur_time, stopur_minut, stopur_sekund);



			if(tmr_count == 3)		// Edwards arbejde || Skal det overhovedet bruges?
			{
				XTmrCtr_Stop(pTMRInst,0);
				tmr_count = 0;


				//sprintf(A, "%d", seconds);
				//xil_printf("Array = %s\n\n", A);

				XGpio_DiscreteWrite(&LEDInst, 1, led_value);
				XTmrCtr_Reset(pTMRInst,0);
				XTmrCtr_Start(pTMRInst,0);


			}
			else tmr_count++;
		}
	}
	else {  //Handle interrupts generated by timer 0 ( we dont use timer 1)

	}

	XTmrCtr_ClearInterruptFlag(pTMRInst, TmrCtrNumber);
}


//******************************************** Testing

//----------------------------------------------------
// MAIN FUNCTION
//----------------------------------------------------
int main (void)
{
	xil_printf("Program startet\n");

  int status;
  //----------------------------------------------------
  // INITIALIZE THE PERIPHERALS & SET DIRECTIONS OF GPIO
  //----------------------------------------------------
  // Initialise LEDs
  status = XGpio_Initialize(&LEDInst, LEDS_DEVICE_ID);
  if(status != XST_SUCCESS) return XST_FAILURE;
  // Initialise Push Buttons
  status = XGpio_Initialize(&BTNInst, BTNS_DEVICE_ID);
  if(status != XST_SUCCESS) return XST_FAILURE;
  //XScuGic_SetPriorityTriggerType(&BTNInst,BTNS_DEVICE_ID,100,3);
  // Set LEDs direction to outputs
  XGpio_SetDataDirection(&LEDInst, 1, 0x00);
  // Set all buttons direction to inputs
  XGpio_SetDataDirection(&BTNInst, 1, 0xFF);


  // VORES ARBEJDE
  // initialise Switches
  status = XGpio_Initialize(&SWTInst, SWTS_DEVICE_ID);
  if(status != XST_SUCCESS) return XST_FAILURE;

  // set switches direction to inputs
  XGpio_SetDataDirection(&SWTInst, 1, 0xFF);



  //----------------------------------------------------
  // SETUP THE TIMER
  //----------------------------------------------------
  status = XTmrCtr_Initialize(&TMRInst, TMR_DEVICE_ID);
  if(status != XST_SUCCESS) return XST_FAILURE;
  XTmrCtr_SetHandler(&TMRInst, TMR_Intr_Handler, &TMRInst);
  XTmrCtr_SetResetValue(&TMRInst, 0, TMR_LOAD);
  XTmrCtr_SetOptions(&TMRInst, 0, XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION | XTC_DOWN_COUNT_OPTION);
 

  // Initialize interrupt controller
  status = IntcInitFunction(INTC_DEVICE_ID, &TMRInst, &BTNInst, &SWTInst);
  if(status != XST_SUCCESS) return XST_FAILURE;

  XTmrCtr_Start(&TMRInst, 0);


  xil_printf("Initialisering færdig\n");
  while(1);

  return 0;
}

//----------------------------------------------------
// INITIAL SETUP FUNCTIONS
//----------------------------------------------------

int InterruptSystemSetup(XScuGic *XScuGicInstancePtr)
{
	// 1. skridt i IRQ aktivering 
	// Enable BUTTON interrupt
	XGpio_InterruptEnable(&BTNInst, BTN_INT);
	XGpio_InterruptGlobalEnable(&BTNInst);



	// Enable SWITCH interrupt
	XGpio_InterruptEnable(&SWTInst, SWT_INT);
	XGpio_InterruptGlobalEnable(&SWTInst);


	// 3 skridt i IRQ aktivering. Bemærk at ARM CPU ikke har interrupts, men derimod exceptions
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			 	 	 	 	 	 (Xil_ExceptionHandler)XScuGic_InterruptHandler,
			 	 	 	 	 	 XScuGicInstancePtr);
	Xil_ExceptionEnable();


	return XST_SUCCESS;

}

int IntcInitFunction(u16 DeviceId, XTmrCtr *TmrInstancePtr, XGpio *GpioInstancePtr, XGpio *GpioInstancePtr_SWT)
{
	XScuGic_Config *IntcConfig;
	int status;

	// Interrupt controller initialisation
	IntcConfig = XScuGic_LookupConfig(DeviceId);
	status = XScuGic_CfgInitialize(&INTCInst, IntcConfig, IntcConfig->CpuBaseAddress);
	if(status != XST_SUCCESS) return XST_FAILURE;

	// Call to interrupt setup
	status = InterruptSystemSetup(&INTCInst);
	if(status != XST_SUCCESS) return XST_FAILURE;
	


	// 2. skridt for IRQ aktivering
	// VORES SWITCH ARBEJDE
	// Connect switch interrupt to handler
	status = XScuGic_Connect(&INTCInst,
					  	  	 INTC_SWT_INTERRUPT_ID,
					  	  	 (Xil_ExceptionHandler)SWT_Intr_Handler,
					  	  	 (void *)GpioInstancePtr_SWT);
	if(status != XST_SUCCESS) return XST_FAILURE;



	// Connect GPIO interrupt to handler
	status = XScuGic_Connect(&INTCInst,
					  	  	 INTC_BTN_INTERRUPT_ID,
					  	  	 (Xil_ExceptionHandler)BTN_Intr_Handler,
					  	  	 (void *)GpioInstancePtr);
	if(status != XST_SUCCESS) return XST_FAILURE;


	// Connect timer interrupt to handler
	status = XScuGic_Connect(&INTCInst,
							 INTC_TMR_INTERRUPT_ID,
							 (Xil_ExceptionHandler)XTmrCtr_InterruptHandler,
							 (void *)TmrInstancePtr);
	if(status != XST_SUCCESS) return XST_FAILURE;

	// Enable GPIO interrupts interrupt
	XGpio_InterruptEnable(GpioInstancePtr, 1);
	XGpio_InterruptGlobalEnable(GpioInstancePtr);

	// for SWITCHES
	XGpio_InterruptEnable(GpioInstancePtr_SWT, 1);
	XGpio_InterruptGlobalEnable(GpioInstancePtr_SWT);


	// Enable GPIO and timer interrupts in the controller
	XScuGic_Enable(&INTCInst, INTC_BTN_INTERRUPT_ID);
	
	XScuGic_Enable(&INTCInst, INTC_TMR_INTERRUPT_ID);
	
	// for SWITCHES
	XScuGic_Enable(&INTCInst, INTC_SWT_INTERRUPT_ID);

	return XST_SUCCESS;
}

void XTmrCtr_ClearInterruptFlag(XTmrCtr * InstancePtr, u8 TmrCtrNumber)
{
	u32 CounterControlReg;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(TmrCtrNumber < XTC_DEVICE_TIMER_COUNT);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Read current contents of the CSR register so it won't be destroyed
	 */
	CounterControlReg = XTmrCtr_ReadReg(InstancePtr->BaseAddress,
					       TmrCtrNumber, XTC_TCSR_OFFSET);
	/*
	 * Reset the interrupt flag
	 */
	XTmrCtr_WriteReg(InstancePtr->BaseAddress, TmrCtrNumber,
			  XTC_TCSR_OFFSET,
			  CounterControlReg | XTC_CSR_INT_OCCURED_MASK);
}


// mangler threads til scanf. Scanf tager IKKE inputs fra tastaturet.
void lommeregner()
{
	ur_aktiv = 0;
// testing
	xil_printf("Lommeregner aktiveret\n");
// testing

	while(state==1)
	{

		char symbol;
		xil_printf("\nIndtast symbol - || + || - || * || / ||");
		scanf(" %c", &symbol);
		//getchar();



		xil_printf("Indtast tal");
		scanf(" %lf", &a);
		//getchar();


		xil_printf("\nindtast andet tal");
		scanf(" %lf", &b);
		//getchar();


		switch(symbol)
		{
			case '+':
				addition(a,b);
				xil_printf("Resultat er: %.2lf %c %.2lf =  %.2lf\n", a, symbol,b,result);
				break;
			case '-':
				subtraction(a,b);
				xil_printf("Resultat er: %.2lf %c %.2lf =  %.2lf\n", a,symbol,b,result);
				break;
			case '*':
				mult(a,b);
				xil_printf("Resultat er: %.2lf %c %.2lf =  %.2lf\n", a,symbol,b,result);
				break;
			case '/':
				division(a,b);
				xil_printf("Resultat er: %.2lf %c %.2lf =  %.2lf\n", a,symbol,b,result);
				break;
			case '%':
				break;
			default:
				xil_printf("Forkert indtastet operator\n");
				break;
		}
	}

	xil_printf("lommeregner afsluttes\n");
	batteriBrugt();
	ur_aktiv = 1;
}


// ser ikke ud til, at disse funktioner udregner et korrekt resultat
void addition(double a, double b){
    result = a + b;
}

void subtraction(double a, double b){
    result = a - b;
}

void mult(double a, double b){
    result = a * b;
}

void division(double a, double b){
    result = a / b;
}



// Virker fint, men problemer med, at aflæse button værdier mens vi er i denne funktion
void virtuelSMS(void)
{
	//time_t t;
	//srand((unsigned) time(&t));


	//alarm lyd indsat
	//xil_printf("\a\n"); // den kan ikke finde ud af det her


	xil_printf("Besked modtaget\n");



	// problem her
	//if(btn_value==8)
	//{
		xil_printf("%s ", listG[rand() % sizeof(listG) / sizeof(listG[0])]);
		xil_printf("%s ", listA[rand() % sizeof(listA) / sizeof(listA[0])]);
		xil_printf("%s ", listS[rand() % sizeof(listS) / sizeof(listS[0])]);
		xil_printf("%s ", listV[rand() % sizeof(listV) / sizeof(listV[0])]);
		xil_printf("%s \n", listB[rand() % sizeof(listB) / sizeof(listB[0])]);

	//}
		batteriBrugt();

}


// Funktion til, at omskrive batteriniveau til visuel repræsentering med LED'erne.
void batteriniveau_led(void)
{
	if (batteriniveau >= 0 && batteriniveau <= 24) led_value = 1;
	if (batteriniveau >= 25 && batteriniveau <= 59) led_value = 3;
	if (batteriniveau >= 50 && batteriniveau <= 74) led_value = 7;
	if (batteriniveau >= 75 && batteriniveau <= 100) led_value = 15;
}


// Funktion der formindsker batteriniveauet
void batteriBrugt(void)
{
	//batteriniveau = batteriniveau - (batteriniveau/100);
	batteriniveau = batteriniveau - 1;
}


void stopur(void)
{
	//xil_printf("Ønsker du at starte stopuret?\n1 for ja 0 for nej");

	if (stopur_aktiv == 0)
	{
		stopur_aktiv = 1; // start stopuret
		ur_aktiv = 0;
	} else{
		stopur_aktiv = 0; // start stopuret
		ur_aktiv = 1;
	}

}
