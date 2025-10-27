/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
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
#include "main.h"
#include "dma.h"
#include "fdcan.h"
#include "i2c.h"
#include "rtc.h"
#include "spi.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd_demo.h"
#include "flash_demo.h"
#include "eeprom_demo.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

uint8_t buf[24];
FDCAN_RxHeaderTypeDef FDCAN1_RxHeader;
FDCAN_RxHeaderTypeDef FDCAN2_RxHeader;
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs);
const uint8_t data[] = {"hello"};


FDCAN_TxHeaderTypeDef FDCAN1_TxHeader;
void CAN1_SetMsg(void)
{
 
    uint8_t msg[8] = {1,2,3,4,5,6,7,8};
    FDCAN1_TxHeader.Identifier=  0x1234;                       //32位ID
    FDCAN1_TxHeader.IdType=FDCAN_STANDARD_ID;                  //标准ID
    FDCAN1_TxHeader.TxFrameType=FDCAN_DATA_FRAME;              //数据帧
    FDCAN1_TxHeader.DataLength=8;                              //数据长度
    FDCAN1_TxHeader.ErrorStateIndicator=FDCAN_ESI_ACTIVE;            
    FDCAN1_TxHeader.BitRateSwitch=FDCAN_BRS_OFF;               //关闭速率切换
    FDCAN1_TxHeader.FDFormat=FDCAN_CLASSIC_CAN;                //传统的CAN模式
    FDCAN1_TxHeader.TxEventFifoControl=FDCAN_NO_TX_EVENTS;     //无发送事件
    FDCAN1_TxHeader.MessageMarker=0;                           
    
    HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1,&FDCAN1_TxHeader,msg);
	printf("send can1 successed\n");

}


FDCAN_TxHeaderTypeDef FDCAN2_TxHeader;
void CAN2_SetMsg(void)
{
 
    uint8_t msg[8] = {1,2,3,4,5,6,7,8};
    FDCAN2_TxHeader.Identifier=  0x1234;                       //32位ID
    FDCAN2_TxHeader.IdType=FDCAN_STANDARD_ID;                  //标准ID
    FDCAN2_TxHeader.TxFrameType=FDCAN_DATA_FRAME;              //数据帧
    FDCAN2_TxHeader.DataLength=8;                              //数据长度
    FDCAN2_TxHeader.ErrorStateIndicator=FDCAN_ESI_ACTIVE;            
    FDCAN2_TxHeader.BitRateSwitch=FDCAN_BRS_OFF;               //关闭速率切换
    FDCAN2_TxHeader.FDFormat=FDCAN_CLASSIC_CAN;                //传统的CAN模式
    FDCAN2_TxHeader.TxEventFifoControl=FDCAN_NO_TX_EVENTS;     //无发送事件
    FDCAN2_TxHeader.MessageMarker=0;                           
    
    HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan2,&FDCAN2_TxHeader,msg);
	printf("send can2 successed\n");

}

void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
	if(hfdcan == &hfdcan1){
		printf("get can1 rx data\n");
		uint8_t i=0;
		uint8_t rxdata[8];
		if((RxFifo0ITs&FDCAN_IT_RX_FIFO0_NEW_MESSAGE)!=RESET)  
		{
			HAL_FDCAN_GetRxMessage(hfdcan,FDCAN_RX_FIFO0,&FDCAN1_RxHeader,rxdata);
			printf("id:%#x ",FDCAN1_RxHeader.Identifier);
			printf("len:%d ",FDCAN1_RxHeader.DataLength>>16);
			for(i=0;i<8;i++)
			printf("rxdata[%d]:%d,",i,rxdata[i]);
			printf("\n");
			HAL_FDCAN_ActivateNotification(hfdcan,FDCAN_IT_RX_FIFO0_NEW_MESSAGE,0);
		}
	}else if(hfdcan == &hfdcan2){
		printf("get can2 rx data\n");
		uint8_t i=0;
		uint8_t rxdata[8];
		if((RxFifo0ITs&FDCAN_IT_RX_FIFO0_NEW_MESSAGE)!=RESET)  
		{
			HAL_FDCAN_GetRxMessage(hfdcan,FDCAN_RX_FIFO0,&FDCAN2_RxHeader,rxdata);
			printf("id:%#x ",FDCAN1_RxHeader.Identifier);
			printf("len:%d ",FDCAN1_RxHeader.DataLength>>16);
			for(i=0;i<8;i++)
			printf("rxdata[%d]:%d",i,rxdata[i]);
			printf("\n");
			HAL_FDCAN_ActivateNotification(hfdcan,FDCAN_IT_RX_FIFO0_NEW_MESSAGE,0);
		}
	}
    
}





/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_RTC_Init();
  MX_USART3_UART_Init();
  MX_I2C1_Init();
  MX_SPI2_Init();
  MX_SPI1_Init();
  MX_FDCAN1_Init();
  MX_FDCAN2_Init();
  /* USER CODE BEGIN 2 */
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	CAN1_SetMsg();
	HAL_Delay(5000);
	CAN2_SetMsg();
	HAL_Delay(5000);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 16;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
