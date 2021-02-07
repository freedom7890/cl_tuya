/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "usart.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "IR.h"
#include "stop.h"
#include "motor.h"
#include "wifi.h"


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

//#define USER_MAIN_DEBUG
#define tuya

#ifdef USER_MAIN_DEBUG

#define log(format, ...) printf( format "\r\n",##__VA_ARGS__)

#else
#define log(format, ...)

#endif
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

uint16_t adcbuf[10]; //电池adc
uint8_t hall_flag=1;    //霍尔低电平标志
uint8_t CONTROL=0x01;  //控制状态
uint16_t time=0,time1,time2,now=0,min=0,max=600;
uint32_t temp = 0;//用于接收红外数据
uint16_t rx_temp[1]; //串口接收

#ifdef tuya
uint8_t PERCENT_CONTROL=0,AUTO_POWER=1,BATTERY_PERCENTAGE;
uint8_t PERCENT_STATE=0;
uint16_t now_control=0xffff;
#endif
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
int fputc(int ch, FILE *f)	  //重写fputc函数 代码可放在usart.c或者主函数里
{
	 uint8_t temp[1]={ch};
    HAL_UART_Transmit(&hlpuart1,temp,1,2);
	return ch;
}
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void adcbat(void)
{
  HAL_GPIO_WritePin(adc_en_GPIO_Port, adc_en_Pin, GPIO_PIN_SET);
  HAL_ADC_Start_DMA(&hadc,(uint32_t *)adcbuf,10);
  HAL_Delay(10);
	HAL_ADC_Stop_DMA(&hadc);
	HAL_GPIO_WritePin(adc_en_GPIO_Port, adc_en_Pin, GPIO_PIN_RESET);
  uint8_t i;
		for(i=0;i<9;i++)
	{
		adcbuf[0]+=adcbuf[i+1];
		
	}
	  adcbuf[0]/=10;
	
#ifdef tuya	
	if(adcbuf[0]<2048)
	{
		adcbuf[0]=2048;
	}
		BATTERY_PERCENTAGE=(adcbuf[0]-2048)*100/560;
	if(BATTERY_PERCENTAGE>100)
	{
		BATTERY_PERCENTAGE=100;
	}
	mcu_dp_value_update(DPID_BATTERY_PERCENTAGE,BATTERY_PERCENTAGE); //上报电量百分比
#endif
	
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
  MX_ADC_Init();
  MX_LPUART1_UART_Init();
  MX_TIM2_Init();
  MX_TIM21_Init();
  /* USER CODE BEGIN 2 */


	IR_Init();
	HAL_TIM_Base_Start_IT(&htim21);
	HAL_TIM_Base_Stop(&htim21);
	adcbat();
	HAL_GPIO_WritePin(led_GPIO_Port, led_Pin, GPIO_PIN_SET);
	
#ifdef tuya	
	wifi_protocol_init();
  HAL_UART_Receive_IT(&hlpuart1,(uint8_t *)rx_temp, 1);//开接收中断
#endif	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
#ifdef tuya
		if(HAL_GPIO_ReadPin(sw2_GPIO_Port, sw2_Pin)==0)               //检测按键按下2000ms进入配网模式
		{
			HAL_GPIO_WritePin(led_GPIO_Port, led_Pin, GPIO_PIN_SET);
			HAL_Delay(2000);
			if(HAL_GPIO_ReadPin(sw2_GPIO_Port, sw2_Pin)==0)
		  {
				HAL_GPIO_WritePin(led_GPIO_Port, led_Pin, GPIO_PIN_RESET);
				mcu_reset_wifi();
			}
			HAL_Delay(100);		
			
		switch(mcu_get_wifi_work_state())                   //查询配网状态
			{   
				case SMART_CONFIG_STATE: //Smart 配置状态 LED 快闪 ，led 闪烁请用户完成 
				  while(mcu_get_wifi_work_state() == SMART_CONFIG_STATE )
					{
						HAL_GPIO_WritePin(led_GPIO_Port, led_Pin, GPIO_PIN_SET);
						HAL_Delay(250);
						HAL_GPIO_WritePin(led_GPIO_Port, led_Pin, GPIO_PIN_RESET);
						HAL_Delay(250);
          }						
					break; 
				case AP_STATE: //AP 配置状态 LED 慢闪，led 闪烁请用户完成 
				  while(mcu_get_wifi_work_state() == AP_STATE )
					{
						HAL_GPIO_WritePin(led_GPIO_Port, led_Pin, GPIO_PIN_SET);
						HAL_Delay(1500);
						HAL_GPIO_WritePin(led_GPIO_Port, led_Pin, GPIO_PIN_RESET);
						HAL_Delay(1500);
					}
					break; 
				case WIFI_NOT_CONNECTED: //WIFI 配置完成，正在连接路由器
					HAL_GPIO_WritePin(led_GPIO_Port, led_Pin, GPIO_PIN_SET);			
					break; 
				case WIFI_CONN_CLOUD: //wifi 已连上云端 LED 常亮
					HAL_GPIO_WritePin(led_GPIO_Port, led_Pin, GPIO_PIN_SET);
					break;				
				default:break; 
			}
		}
		
		PERCENT_STATE=now*100/max;
		mcu_dp_value_update(DPID_PERCENT_STATE,PERCENT_STATE); //上报开启状态百分比 100为全关闭
#endif
			
		HAL_Delay(1000);

		if(HAL_GPIO_ReadPin(motoren_GPIO_Port, motoren_Pin)==0 )   //检测待机进入stop模式
		{
			
			//HAL_UART_MspDeInit(&hlpuart1);
			
			HAL_GPIO_WritePin(led_GPIO_Port, led_Pin, GPIO_PIN_RESET);
	    HAL_TIM_IC_Stop_IT(&IR_TIM, IR_TIM_CHANNE);
    	HAL_TIM_Base_MspDeInit(&htim2);
			HAL_ADC_MspDeInit(&hadc);
    	HAL_ADC_DeInit(&hadc);
    	__HAL_RCC_DMA1_CLK_DISABLE();

			enter_stop_mode();
			
			adcbat();			
			log("vbat=%d\r\n",adcbuf[0]);
		}
		
			if(adcbuf[0]<2000)
		{			
			HAL_GPIO_WritePin(led_GPIO_Port, led_Pin, GPIO_PIN_RESET);
			HAL_Delay(1000);
			HAL_GPIO_WritePin(led_GPIO_Port, led_Pin, GPIO_PIN_SET);
		}

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LPUART1;
  PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)//捕获回调函数
{
	if (TIM2 == htim->Instance)
	{ 
		IR_CaptureCallback();
		if (IR_Scanf(&temp))
		{
			log("%X\r\n", temp);
			switch(temp)
			{
				case 0x00FFE01F:   //左关       //同时控制左右
						 close();						     
						 break;  
				case 0x00FFA857:  //右开
						 open();						    
						 break;  
				case 0x00FF906F:    //停
						 stop();
						 break;  
				
//				case 0x00FF6897:   //左关      //单独控制左边
//						 close();						     
//						 break;  
//				case 0x00FF9867:  //右开
//						 open();						    
//						 break; 
//				case 0x00FFB04F:    //停
//						 stop();
//						 break;
				
//						case 0x00FF22DD:   //左关    //单独控制右边
//                 close();						     
//                 break;  
//						case 0x00FF02FD:  //右开
//                 open();						    
//                 break; 
//						case 0x00FFC23D:    //停
//                 stop();
//                 break;
 
				default:break;
			}				
	  }
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{ 
	if(GPIO_Pin == IR_Pin)
	{ 
		
	}

  if(GPIO_Pin == sw2_Pin)
	{ 
	}	
	  
	if(GPIO_Pin == hall_Pin)
  { 	          
		
    if(hall_flag ==0)
    {
       hall_flag=1;
       stop();
    }
    else
    {
      time++;
      if(time==1)
			{
				if(HAL_GPIO_ReadPin(hall_GPIO_Port, hall_Pin)==0)	
				{
				__HAL_TIM_SetCounter(&htim21,0);
				HAL_TIM_Base_Start(&htim21);
				}
				else
				time=0;
			}
			if(time==2)
			{ if(HAL_GPIO_ReadPin(hall_GPIO_Port, hall_Pin)==1)
				{
					HAL_TIM_Base_Stop(&htim21);
				time1=__HAL_TIM_GetCounter(&htim21);
				log("time1=%d\r\n",time1);
				__HAL_TIM_SetCounter(&htim21,0);
      	HAL_TIM_Base_Start(&htim21);
				}
			}
			if(time==3)
			{
				if(HAL_GPIO_ReadPin(hall_GPIO_Port, hall_Pin)==0)
				{
					HAL_TIM_Base_Stop(&htim21);
					time2=__HAL_TIM_GetCounter(&htim21);
					log("time2=%d\r\n",time2);
			
					if(time1<time2)
					{
						now--;
						if(now == 0xffff)
						{ 
							now=0;
						}
						
						log("now=%d\r\n",now);
						
						if(CONTROL==0x01 && now>=min+1)
						{
							#ifdef tuya
							if(AUTO_POWER ==1)
							{
							#endif
								
							open();	
							
							#ifdef tuya
							}
							#endif
					  }
				  }
					else if(time1>time2)
					{
						now++;
						log("now=%d\r\n",now);
							
						if(CONTROL==0x01 && now<=max-1 )
						{
							#ifdef tuya
							if(AUTO_POWER ==1)
							{
							#endif
								
							close();
								
							#ifdef tuya						
							}
							#endif
						}
					}
					
					#ifdef tuya	
					if(now==now_control)
					{
						stop();
						now_control=0xffff;	
					}
					#endif
				
				time=1;
				__HAL_TIM_SetCounter(&htim21,0);
				HAL_TIM_Base_Start(&htim21);
			  }
			}    
		} 
		if(CONTROL == 0x02)
		       {
		      	if(now >= max)
		       	{
		      		stop();
							
		      	}
	      	}
		if(CONTROL == 0x00)
		       {
		      	if(now <= min)
	       		{
		      		stop();
              				
		      	}	
		       }
	}			
}


void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{  
	if (htim->Instance == TIM21)
	{
		if(CONTROL == 0x02 && max==600 )
		{
			max=now;
			log("max=%d",max);
		}

		stop();
	}
}


#ifdef tuya	
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == LPUART1)
	{
		 uart_receive_input(rx_temp[0]);
		 HAL_UART_Receive_IT(&hlpuart1,(uint8_t *)rx_temp, 1);//开接收中断
		 wifi_uart_service();
	}
}
#endif
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
