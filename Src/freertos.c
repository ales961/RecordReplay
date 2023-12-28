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
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MAX_MESSAGES 32
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
typedef struct {
    char* data;
    u16_t length;
    uint32_t timestamp;
} TcpMessage;

TcpMessage messageBuffer[MAX_MESSAGES];
uint32_t nextWriteIndex = 0;
uint32_t nextReadIndex = 0;

uint8_t replayFlag = 0;
uint32_t replayStartTimestamp;
/* USER CODE END Variables */
osThreadId tcpEchoHandle;
osThreadId loopbackConnHandle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
void storeMessage(void* data, u16_t len);
TcpMessage retrieveMessage();
/* USER CODE END FunctionPrototypes */

void startTcpEcho(void const * argument);
void startLoopbackConn(void const * argument);

extern void MX_LWIP_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

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

                 if (strncmp((char*)data, "q", len) == 0 && replayFlag == 0) {
                	 netconn_close(newconn);
           	      	 netconn_delete(newconn);
                	 replayFlag = 1;
                	 replayStartTimestamp = osKernelSysTick();
           	         break;
                 }
                 if (replayFlag == 0) {
                     storeMessage(data, len);
                     err = netconn_write(newconn, data, len, NETCONN_COPY);
                 } else
	            	 HAL_UART_Transmit(&huart6, "Packet received\n", strlen("Packet received\n"), 100);
                 //HAL_UART_Transmit(&huart6, (char*) data, len, 100);
	        } while (netbuf_next(buf) >= 0);
	        netbuf_delete(buf);
	      }
	      netconn_close(newconn);
	      netconn_delete(newconn);
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
	  struct netconn *conn;
	  err_t err = ERROR;
  /* Infinite loop */
	  for(;;)
	  {
			if(replayFlag == 1) {
				osDelay(100);
			    conn = netconn_new(NETCONN_TCP);

			     ip_addr_t dest_addr;
			     IP4_ADDR(&dest_addr, 127, 0, 0, 1);
			     while (err != ERR_OK)
			    	 err = netconn_connect(conn, &dest_addr, 80);

			    // Check for available messages in the buffer
			    while (nextReadIndex != nextWriteIndex) {
			        // Retrieve the next message from the buffer
			    	TcpMessage message = retrieveMessage();

			        // Calculate the delay until the scheduled replay time
			        uint32_t currentTime = osKernelSysTick();
			        uint32_t delay = message.timestamp + replayStartTimestamp - currentTime;

			        if (delay > 0) {
			        	osDelay(delay);
			        }
			        err_t write_err = netconn_write(conn, (void*) message.data, message.length, NETCONN_COPY);
			    }
			}
			osDelay(100);
	  }
  /* USER CODE END startLoopbackConn */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void storeMessage(void* data, u16_t len) {
    // Copy the message to the buffer
	for (uint16_t i = 0; i < len; i++) {
		messageBuffer[nextWriteIndex].data[i] = ((char*)data)[i];
	}
    //memcpy(messageBuffer[nextWriteIndex].data, data, len); //???
    messageBuffer[nextWriteIndex].length = len;
    messageBuffer[nextWriteIndex].timestamp = osKernelSysTick();
    // Update the write index in a circular manner
    nextWriteIndex = (nextWriteIndex + 1) % MAX_MESSAGES;
}

TcpMessage retrieveMessage() {
    TcpMessage message = messageBuffer[nextReadIndex];
    nextReadIndex = (nextReadIndex + 1) % MAX_MESSAGES;
    return message;
}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
