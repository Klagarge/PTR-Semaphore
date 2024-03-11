/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/
#include "RTE_Components.h"
#include  CMSIS_device_header
#include "cmsis_os2.h"
#include "ext_led.h" 
 
#ifdef RTE_Compiler_EventRecorder
#include "EventRecorder.h"
#endif
uint32_t gTableA[200];
uint32_t gTableB[200];

osThreadId_t thread1,thread2,thread3,thread4;
osSemaphoreId_t mutexLed, mutexData;

 const osThreadAttr_t thread1_attr = {
  .stack_size = 1024,          // Create the thread stack size of 1024 bytes
	.priority = osPriorityNormal, //Set initial thread priority to high
	 .name = "FIRST"
};
 const osThreadAttr_t thread2_attr = {
  .stack_size = 1024,          // Create the thread stack size of 1024 bytes
	.priority = osPriorityNormal, //Set initial thread priority to high
	 .name = "SECOND"
};
 const osThreadAttr_t thread3_attr = {
  .stack_size = 1024,          // Create the thread stack size of 1024 bytes
	.priority = osPriorityAboveNormal, //Set initial thread priority to high
	 .name = "DUMMY"
};

 const osSemaphoreAttr_t mutexLed_attr = {
  .name = "Mtx_LED",
};

 const osSemaphoreAttr_t mutexData_attr = {
  .name = "Mtx_DATA",
};


/*----------------------------------------------------------------------------
 * Thread 1
 *---------------------------------------------------------------------------*/
void Thread_1 (void *argument) {
  uint32_t i;
	
  for (;;) 
	{
		osSemaphoreAcquire(mutexLed,osWaitForever);
		osSemaphoreAcquire(mutexData,osWaitForever);
		Ext_LEDs(1);
		for(i=0;i<200;i++)
		{
			for(uint32_t j=0;j<1000;j++)
			{
				gTableA[i] = gTableB[i];
			}
		}
		Ext_LEDs(0);
		osSemaphoreRelease(mutexData);
		osSemaphoreRelease(mutexLed);
	}
}
/*----------------------------------------------------------------------------
 * Thread 2
 *---------------------------------------------------------------------------*/
void Thread_2 (void *argument) {
  uint32_t i;
  for (;;) 
	{
		osSemaphoreAcquire(mutexData,osWaitForever);
		osSemaphoreAcquire(mutexLed,osWaitForever);
		Ext_LEDs(2);
		for(i=0;i<200;i++)
		{
			for(uint32_t j=0;j<1000;j++)
			{
				gTableA[i] = gTableB[i];
			}
		}
		Ext_LEDs(0);
		osSemaphoreRelease(mutexData);
		osSemaphoreRelease(mutexLed);
	}
}
 
/*----------------------------------------------------------------------------
 * Thread 3
 *---------------------------------------------------------------------------*/
void Thread_3 (void *argument) {
  for (;;) 
	{
		osDelay(200);
	}
}


int main (void) {
 
  // System Initialization
  SystemCoreClockUpdate();
#ifdef RTE_Compiler_EventRecorder
  // Initialize and start Event Recorder
	Ext_LED_Init();
  EventRecorderInitialize(EventRecordAll, 1U);
#endif
 
  osKernelInitialize();                 // Initialize CMSIS-RTOS
	mutexLed = osSemaphoreNew(1,1,&mutexLed_attr);
	mutexData = osSemaphoreNew(1,1,&mutexData_attr);
	
  thread1 = osThreadNew(Thread_1, NULL,  &thread1_attr);    // Create application thread
  thread2 = osThreadNew(Thread_2, NULL,  &thread2_attr);    // Create application thread
  thread3 = osThreadNew(Thread_3, NULL,  &thread3_attr);    // Create application thread
//---------------------------------------------------------------------------------------
	osSemaphoreGetName(mutexData);
	osSemaphoreGetName(mutexLed);
  osKernelStart();                      // Start thread execution
  for (;;) {}
}
