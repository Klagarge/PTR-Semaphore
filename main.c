/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/
 #include "stm32f7xx_hal.h"
#include "RTE_Components.h"
#include  CMSIS_device_header
#include "cmsis_os2.h"
#include "ext_led.h" 
#include "ext_uart.h"
#include "ext_buttons.h"
#include "ext_keyboard.h"
#include "string.h"
#include <stdio.h>
#include <stdlib.h>

#ifdef RTE_Compiler_EventRecorder
#include "EventRecorder.h"
#endif
char gBuffer[200];							// global critical section buffer

osThreadId_t thread1,thread2,thread3,thread4,thread5;
osSemaphoreId_t rw,w,mutexNbReader;
uint32_t nbReader=0;

 const osThreadAttr_t thread1_attr = {
  .stack_size = 1024,          // Create the thread stack size 
	.priority = osPriorityNormal, //Set initial thread priority to high
	 .name = "READER_1",
};
 const osThreadAttr_t thread2_attr = {
  .stack_size = 1024,          // Create the thread stack size 
	.priority = osPriorityNormal, //Set initial thread priority to high
	 .name = "READER_2",
};
 const osThreadAttr_t thread3_attr = {
  .stack_size = 1024,          // Create the thread stack size 
	.priority = osPriorityNormal, //Set initial thread priority to high
	 .name = "READER_3",
};
 const osThreadAttr_t thread4_attr = {
  .stack_size = 256,          // Create the thread stack size
	.priority = osPriorityNormal, //Set initial thread priority to high
	 .name = "WRITER_1",
};
 const osThreadAttr_t thread5_attr = {
  .stack_size = 256,          // Create the thread stack size
	.priority = osPriorityNormal, //Set initial thread priority to high
	 .name = "WRITER_2",
};

const osSemaphoreAttr_t semaR_attr = {
  .name = "MTX_COUNT",          	// name of the semaphore
};
const osSemaphoreAttr_t semaW_attr = {
  .name = "WRITERS",          		// name of the semaphore
};
const osSemaphoreAttr_t semaRW_attr = {
  .name = "READERS_WRITERS",    	// name of the semaphore
};

//------------------------------------------------------------------------------
// Setup system clock to 216MHz
//------------------------------------------------------------------------------
void SystemClock_Config (void) {
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 432;  
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  /* Activate the OverDrive to reach the 216 MHz Frequency */
  HAL_PWREx_EnableOverDrive();
  
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7);
}

void BeginRead(void) {
	osSemaphoreAcquire(rw, osWaitForever);
	
	osSemaphoreAcquire(mutexNbReader, osWaitForever);
		nbReader += 1;

	if(nbReader == 1){
		osSemaphoreAcquire(w, osWaitForever);
	}	
	
	osSemaphoreRelease(mutexNbReader);
	
	osSemaphoreRelease(rw);
}

void EndRead(void) {
	osSemaphoreAcquire(mutexNbReader, osWaitForever);
		nbReader -= 1;

	if(nbReader == 0){
		osSemaphoreRelease(w);
	}	
	
	osSemaphoreRelease(mutexNbReader);
}

void BeginWrite(void) {
	osSemaphoreAcquire(rw, osWaitForever);
	osSemaphoreAcquire(w, osWaitForever);
}

void EndWrite(void) {
	osSemaphoreRelease(w);
	osSemaphoreRelease(rw);
}

/*----------------------------------------------------------------------------
 * Thread Readers
 *---------------------------------------------------------------------------*/
void Thread_Reader (void *argument) {
	volatile uint32_t dummy;
	uint8_t cs;
	const char msgError[50];		// debug error message
	const char msgOk[50];		// debug error message
	sprintf(msgError,"Error on Reader %d !\r\n",argument);
	sprintf(msgOk,"All ok on Reader %d ...\r\n",argument);
  for (;;) 
	{
		BeginRead();											// entry of a reader
		cs = 0;
		for(uint8_t i=0;i<(sizeof(gBuffer)-1);i++)
		{
			cs += gBuffer[i];
			i++;
			for(dummy=0;dummy<4000;dummy++){}	// spend time
		}
		if(cs != gBuffer[(sizeof(gBuffer)-1)])	// if checksum error
		{
			printf(msgError);										// display error message
		}
		else
		{
			printf(msgOk);
		}
		EndRead();												// exit of a reader
	}
}
/*----------------------------------------------------------------------------
 * Thread Writer 1
 *---------------------------------------------------------------------------*/
void Thread_Writer_1 (void *argument) {
	volatile uint32_t dummy;
	char tempStr[20];
	char cs,tmp8;
  for (;;) 
	{
		osDelay(290);										// wait a moment to write
		BeginWrite();										// entry of a writer
		cs = 0;													// clear checksum
		for(uint8_t i=0;i<(sizeof(gBuffer)-1);i++)
		{
			tmp8 = rand();										// get a random number
			cs += tmp8;												// compute checksum
			gBuffer[i] = tmp8;								// place number in memory
			for(dummy=0;dummy<1000;dummy++){}	// spend time
			i++;
		}
		gBuffer[(sizeof(gBuffer)-1)] = cs;	// put checksum on buffer
		EndWrite();											// exit of a writer
	}
}
 
/*----------------------------------------------------------------------------
 * Thread 3
 *---------------------------------------------------------------------------*/
void Thread_Writer_2 (void *argument) {
	volatile uint32_t dummy;
	char tempStr[20];
	char cs,tmp8;
  for (;;) 
	{
		osDelay(370);										// wait a moment to write
		BeginWrite();										// entry of a writer
		cs = 0;													// clear checksum
		for(uint8_t i=0;i<(sizeof(gBuffer)-1);i++)
		{
			tmp8 = rand();										// get a random number
			cs += tmp8;												// compute checksum
			gBuffer[i] = tmp8;								// place number in memory
			for(dummy=0;dummy<1000;dummy++){}	// spend time
			i++;
		}
		gBuffer[(sizeof(gBuffer)-1)] = cs;	// put checksum on buffer
		EndWrite();											// exit of a writer
	}
}


int main (void) {
 
  // System Initialization
	SystemClock_Config();
  SystemCoreClockUpdate();
#ifdef RTE_Compiler_EventRecorder
  // Initialize and start Event Recorder
//	Ext_UART_Init(9600);
  EventRecorderInitialize(EventRecordAll, 1U);
#endif
 
  osKernelInitialize();                 // Initialize CMSIS-RTOS
  thread1 = osThreadNew(Thread_Reader, (void *)1,  &thread1_attr);  // Create application thread
  thread2 = osThreadNew(Thread_Reader, (void *)2,  &thread2_attr);  // Create application thread
  thread3 = osThreadNew(Thread_Reader, (void *)3,  &thread3_attr);  // Create application thread
	
  thread4 = osThreadNew(Thread_Writer_1, NULL,  &thread4_attr);    	// Create application thread
  thread5 = osThreadNew(Thread_Writer_2, NULL,  &thread5_attr);    	// Create application thread
	mutexNbReader = osSemaphoreNew(1,1,&semaR_attr);					// create semaphore
	w = osSemaphoreNew(1,1,&semaW_attr);											// create semaphore
	rw = osSemaphoreNew(1,1,&semaRW_attr);										// create semaphore
	//----------------------------------------------------------------------------------------------
	// get names are placed for TraceAlyzer visualisation
	//----------------------------------------------------------------------------------------------
	osThreadGetName(thread1);
	osThreadGetName(thread2);
	osThreadGetName(thread3);
	osThreadGetName(thread4);
	osThreadGetName(thread5);
	osSemaphoreGetName(mutexNbReader);
	osSemaphoreGetName(w);
	osSemaphoreGetName(rw);
  osKernelStart();                      // Start thread execution
  for (;;) {}
}
