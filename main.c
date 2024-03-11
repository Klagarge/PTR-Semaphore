/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/
 
#include "RTE_Components.h"
#include  CMSIS_device_header
#include "cmsis_os2.h"
#include "EventRecorder.h"
 
const osThreadAttr_t thread1_attr = {
	.priority = osPriorityNormal	
};

const osThreadAttr_t thread2_attr = {
	.priority = osPriorityNormal
};

volatile uint32_t gCounter = 0; // Global and shared resource

volatile uint32_t locCounter1 = 0; // Internal counter Thread1
volatile uint32_t locCounter2 = 0; // Internal counter Thread2
osSemaphoreId_t semId;

void Thread_1(void *argument) {
	uint32_t temp;
	for(;;){
		osSemaphoreAcquire(semId, osWaitForever);
		temp = gCounter;
		locCounter1++;
		gCounter = temp + 1;
		osSemaphoreRelease(semId);
	}
}

void Thread_2(void *argument) {
	uint32_t temp;
	for(;;) {
		osSemaphoreAcquire(semId, osWaitForever);
		temp = gCounter;
		locCounter2++;
		gCounter = temp + 1;
		osSemaphoreRelease(semId);
	}
}
  
int main (void) {
 
  // System Initialization
  SystemCoreClockUpdate();
  EventRecorderInitialize(EventRecordAll, 1U);
	
	osThreadId_t thread1;
	osThreadId_t thread2;
	
	semId = osSemaphoreNew(1,1,NULL);
 
  osKernelInitialize();                 // Initialize CMSIS-RTOS
	
  thread1 = osThreadNew(Thread_1, NULL, &thread1_attr);
	thread2 = osThreadNew(Thread_2, NULL, &thread2_attr);
  
	osKernelStart();                      // Start thread execution
  for (;;) {}
}
