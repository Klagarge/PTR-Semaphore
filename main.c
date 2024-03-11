/*----------------------------------------------------------------------------
 * CMSIS-RTOS 'main' function template
 *---------------------------------------------------------------------------*/
#include "RTE_Components.h"
#include  CMSIS_device_header
#include "cmsis_os2.h"
#include "ext_led.h" 
#include "ext_uart.h"
 
#ifdef RTE_Compiler_EventRecorder
#include "EventRecorder.h"
#endif

osThreadId_t thread1,thread2;
osSemaphoreId_t mutexData;

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
	uint8_t msg[] = "Hello World. This is a message from Task 1.\r\n";
	for (;;) {
		osSemaphoreAcquire(mutexData, osWaitForever);
		HAL_StatusTypeDef state = HAL_UART_Transmit(&ext_uart, msg, sizeof(msg), 50);
		osSemaphoreRelease(mutexData);
	}
}
/*----------------------------------------------------------------------------
 * Thread 2
 *---------------------------------------------------------------------------*/
void Thread_2 (void *argument) {
	uint8_t msg[] = "Hi there, special word from Task 2.\r\n";
	for (;;) {
		osSemaphoreAcquire(mutexData, osWaitForever);
		HAL_StatusTypeDef state = HAL_UART_Transmit(&ext_uart, msg, sizeof(msg), 50);
		osSemaphoreRelease(mutexData);
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
	Ext_UART_Init(115200);
	mutexData = osSemaphoreNew(1,1,&mutexData_attr);
	
  thread1 = osThreadNew(Thread_1, NULL,  &thread1_attr);    // Create application thread
  thread2 = osThreadNew(Thread_2, NULL,  &thread2_attr);    // Create application thread
//---------------------------------------------------------------------------------------
	osSemaphoreGetName(mutexData);
  osKernelStart();                      // Start thread execution
  for (;;) {}
}
