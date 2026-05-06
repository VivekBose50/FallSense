/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_freertos.c
  * Description        : FreeRTOS applicative file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_freertos.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "logger.h"
#include "imu.h"
#include "fall_detection.h"
#include "event_handler.h"
#include "filter.h"
#include "fsr.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
typedef StaticTask_t osStaticThreadDef_t;
typedef StaticQueue_t osStaticMessageQDef_t;
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */

/* Definitions for fsrTask */
osThreadId_t fsrTaskHandle;
uint32_t FsrBufferTask[ 512 ];
osStaticThreadDef_t FsrControlBlockTask;
const osThreadAttr_t fsrTask_attributes = {
  .name = "fsrTask",
  .stack_mem = &FsrBufferTask[0],
  .stack_size = sizeof(FsrBufferTask),
  .cb_mem = &FsrControlBlockTask,
  .cb_size = sizeof(FsrControlBlockTask),
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for imuTask */
osThreadId_t imuTaskHandle;
uint32_t ImuBufferTask[ 1024 ];
osStaticThreadDef_t ImuControlBlockTask;
const osThreadAttr_t imuTask_attributes = {
  .name = "imuTask",
  .stack_mem = &ImuBufferTask[0],
  .stack_size = sizeof(ImuBufferTask),
  .cb_mem = &ImuControlBlockTask,
  .cb_size = sizeof(ImuControlBlockTask),
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for detectFallTask */
osThreadId_t detectFallTaskHandle;
uint32_t DetectFallBufferTask[ 1024 ];
osStaticThreadDef_t DetectFallControlBlockTask;
const osThreadAttr_t detectFallTask_attributes = {
  .name = "detectFallTask",
  .stack_mem = &DetectFallBufferTask[0],
  .stack_size = sizeof(DetectFallBufferTask),
  .cb_mem = &DetectFallControlBlockTask,
  .cb_size = sizeof(DetectFallControlBlockTask),
  .priority = (osPriority_t) osPriorityHigh,
};
/* Definitions for eventTask */
osThreadId_t eventTaskHandle;
uint32_t EventBufferTask[ 1024 ];
osStaticThreadDef_t EventControlBlockTask;
const osThreadAttr_t eventTask_attributes = {
  .name = "eventTask",
  .stack_mem = &EventBufferTask[0],
  .stack_size = sizeof(EventBufferTask),
  .cb_mem = &EventControlBlockTask,
  .cb_size = sizeof(EventControlBlockTask),
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for imuQueue */
osMessageQueueId_t imuQueueHandle;
uint8_t ImuQueueBuffer[ 16 * sizeof( imu_data_t ) ];
osStaticMessageQDef_t ImuQueueControlBlock;
const osMessageQueueAttr_t imuQueue_attributes = {
  .name = "imuQueue",
  .cb_mem = &ImuQueueControlBlock,
  .cb_size = sizeof(ImuQueueControlBlock),
  .mq_mem = &ImuQueueBuffer,
  .mq_size = sizeof(ImuQueueBuffer)
};
/* Definitions for eventsQueue */
osMessageQueueId_t eventsQueueHandle;
uint8_t EventsQueueBuffer[ 5 * sizeof( fall_event_t ) ];
osStaticMessageQDef_t EventsQueueControlBlock;
const osMessageQueueAttr_t eventsQueue_attributes = {
  .name = "eventsQueue",
  .cb_mem = &EventsQueueControlBlock,
  .cb_size = sizeof(EventsQueueControlBlock),
  .mq_mem = &EventsQueueBuffer,
  .mq_size = sizeof(EventsQueueBuffer)
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

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
  /* creation of imuQueue */
  imuQueueHandle = osMessageQueueNew (16, sizeof(imu_data_t), &imuQueue_attributes);
  /* creation of eventsQueue */
  eventsQueueHandle = osMessageQueueNew (5, sizeof(fall_event_t), &eventsQueue_attributes);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */
  /* creation of fsrTask */
  fsrTaskHandle = osThreadNew(StartFsrTask, NULL, &fsrTask_attributes);

  /* creation of imuTask */
  imuTaskHandle = osThreadNew(StartImuTask, NULL, &imuTask_attributes);

  /* creation of detectFallTask */
  detectFallTaskHandle = osThreadNew(StartdetectFallTask, NULL, &detectFallTask_attributes);

  /* creation of eventTask */
  eventTaskHandle = osThreadNew(StartEventTask, NULL, &eventTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}
/* USER CODE BEGIN Header_StartFsrTask */
/**
* @brief Function implementing the fsrTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartFsrTask */
void StartFsrTask(void *argument)
{
  /* USER CODE BEGIN fsrTask */
  bool fsr_prev = false;
  /* Infinite loop */
  for(;;)
  {
    bool fsr_now = false;
    fsr_now = fsr_detect_instability();
    if (fsr_now && !fsr_prev) {
      osThreadFlagsSet(imuTaskHandle, 0x01);
    }
		fsr_prev = fsr_now;
    osDelay(50);
  }
  /* USER CODE END fsrTask */
}

/* USER CODE BEGIN Header_StartImuTask */
/**
* @brief Function implementing the imuTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartImuTask */
void StartImuTask(void *argument)
{
  /* USER CODE BEGIN imuTask */

  imu_data_t imu;

  /* Infinite loop */
  for(;;)
  {
    osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);
    reset_fall_detection();  // incase free fall is delayed after IMU is turned on - if not reset the timeout will be early and impact will be missed

    uint32_t imu_start_time = HAL_GetTick();
    
    while((HAL_GetTick() - imu_start_time) < IMU_ACTIVE_WINDOW_MS){
      imu_process(&imu);
      if(osMessageQueuePut(imuQueueHandle, &imu, 0, 0) != osOK){
        log_error("queue full imu data last");
      }
      osDelay(IMU_SAMPLE_PERIOD_MS);
    }


  }
  /* USER CODE END imuTask */
}

/* USER CODE BEGIN Header_StartdetectFallTask */
/**
* @brief Function implementing the detectFallTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartdetectFallTask */
void StartdetectFallTask(void *argument)
{
  /* USER CODE BEGIN detectFallTask */
  imu_data_t imu;
  /* Infinite loop */
  for(;;)
  {
    osMessageQueueGet(imuQueueHandle, &imu, NULL, osWaitForever);
    fall_event_t event = detect_fall(&imu);
    if(event != EVENT_NONE){
      if(osMessageQueuePut(eventsQueueHandle, &event, 0, 0) != osOK){
        log_error("queue is full, Event data lost");
      }
    }
  }
  /* USER CODE END detectFallTask */
}

/* USER CODE BEGIN Header_StartEventTask */
/**
* @brief Function implementing the eventTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartEventTask */
void StartEventTask(void *argument)
{
  /* USER CODE BEGIN eventTask */
  fall_event_t event;
  /* Infinite loop */
  for(;;)
  {
    osMessageQueueGet(eventsQueueHandle, &event, NULL, osWaitForever);
    handle_event(event);
  }
  /* USER CODE END eventTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

