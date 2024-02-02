/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/api.h"
#include "string.h"
#include "usart.h"
#include "packet_handler.h"
#include "hardware/led.h"
#include "hardware/button.h"
#include "hardware/buzzer.h"
#include "hardware/uart.h"
#include "rr.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define REPLAY 0
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
uint8_t commandNumber = 12;
Command commands[12];

Led ledRed;
Led ledYellow;
Led ledGreen;
Button button;
/* USER CODE END Variables */
osThreadId tcpEchoHandle;
osThreadId loopbackConnHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void startTcpEcho(void const * argument);
void startLoopbackConn(void const * argument);

extern void MX_LWIP_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
  ledInit(&ledRed, LED_RED_GPIO_Port, LED_RED_Pin);
  ledInit(&ledYellow, LED_YELLOW_GPIO_Port, LED_YELLOW_Pin);
  ledInit(&ledGreen, LED_GREEN_GPIO_Port, LED_GREEN_Pin);

  buttonInit(&button, BUTTON_GPIO_Port, BUTTON_Pin);

  commandCreate(&commands[0], LED_RED_GET_STATE, ledGetState, &ledRed);
  commandCreate(&commands[1], LED_RED_ON, ledSetOn, &ledRed);
  commandCreate(&commands[2], LED_RED_OFF, ledSetOff, &ledRed);
  commandCreate(&commands[3], LED_YELLOW_GET_STATE, ledGetState, &ledYellow);
  commandCreate(&commands[4], LED_YELLOW_ON, ledSetOn, &ledYellow);
  commandCreate(&commands[5], LED_YELLOW_OFF, ledSetOff, &ledYellow);
  commandCreate(&commands[6], LED_GREEN_GET_STATE, ledGetState, &ledGreen);
  commandCreate(&commands[7], LED_GREEN_ON, ledSetOn, &ledGreen);
  commandCreate(&commands[8], LED_GREEN_OFF, ledSetOff, &ledGreen);
  commandCreate(&commands[9], BUTTON_GET_STATE, buttonGetState, &button);
  commandCreate(&commands[10], BUZZER_ON, BuzzerSetOn, NULL);
  commandCreate(&commands[11], BUZZER_OFF, BuzzerSetOff, NULL);

  uartInit();
  uartEnableInterruption();
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of tcpEcho */
  osThreadDef(tcpEcho, startTcpEcho, osPriorityNormal, 0, 128);
  tcpEchoHandle = osThreadCreate(osThread(tcpEcho), NULL);

  /* definition and creation of loopbackConn */
  osThreadDef(loopbackConn, startLoopbackConn, osPriorityNormal, 0, 128);
  loopbackConnHandle = osThreadCreate(osThread(loopbackConn), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_startTcpEcho */
/**
  * @brief  Function implementing the tcpEcho thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_startTcpEcho */
void startTcpEcho(void const * argument)
{

  /* init code for LWIP */
  MX_LWIP_Init();

  /* USER CODE BEGIN startTcpEcho */
  struct netconn *conn, *newconn;
  err_t err;
  LWIP_UNUSED_ARG(argument);

  conn = netconn_new(NETCONN_TCP);

  ip4_addr_t ip_addr;
  IP4_ADDR(&ip_addr, 0, 0, 0, 0);
  netconn_bind(conn, &ip_addr, 80);

  netconn_listen(conn);
  for(;;)
  {
	    err = netconn_accept(conn, &newconn);
	    if (err == ERR_OK) {
	      struct netbuf *buf;
	      void *data;
	      u16_t len;

	      while ((err = netconn_recv(newconn, &buf)) == ERR_OK) {
	        do {
	             netbuf_data(buf, &data, &len);
	             char* response = packetHandle(data, &len, commands, &commandNumber);
	             if (!REPLAY) {
	            	 storeMessage(data, len);
	            	 netconn_write(newconn, response, strlen(response), NETCONN_COPY);
	             }
	        } while (netbuf_next(buf) >= 0);
	        netbuf_delete(buf);
	        osDelay(100);
	      }
	      netconn_close(newconn);
	      netconn_delete(newconn);
	      osDelay(100);
	    }
  }
  /* USER CODE END startTcpEcho */
}

/* USER CODE BEGIN Header_startLoopbackConn */
/**
* @brief Function implementing the loopbackConn thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_startLoopbackConn */
void startLoopbackConn(void const * argument)
{
  /* USER CODE BEGIN startLoopbackConn */
	if (REPLAY) {
	  struct netconn *conn;
	  err_t err = ERROR;
	  loadMessages();
  /* Infinite loop */
	  for(;;) {
			osDelay(3000);
			conn = netconn_new(NETCONN_TCP);

			ip_addr_t dest_addr;
			IP4_ADDR(&dest_addr, 127, 0, 0, 1);
			while (err != ERR_OK) {
			    err = netconn_connect(conn, &dest_addr, 80);
			    osDelay(50);
			}

			while (!isMessageBufferEmpty()) {
			    TcpMessage message = retrieveMessage();

			    uint32_t currentTime = osKernelSysTick();
			    uint32_t delay = message.timestamp - currentTime;

			    if (delay > 0) {
			        osDelay(delay);
			    }
			    err_t write_err = netconn_write(conn, (void*) message.data, message.length, NETCONN_COPY);
			}
			osDelay(100);
	  }
	} else {
		for (;;) {
			buttonUpdateState(&button);
			uartPolling();
			if (uartHasNext()) {
				uartEcho();
			}
			commandCheck();
			osDelay(100);
		}
	}
  /* USER CODE END startLoopbackConn */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
