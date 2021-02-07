#include "stop.h"
#include <stdio.h>
#include <tim.h>
#include <gpio.h>
#include <ir.h>
#include "usart.h"
#include "adc.h"
#include "dma.h"

#define tuya
//#define notuya

void system_config_before_stop(void)
{
	  
#ifdef tuya
  __HAL_UART_ENABLE_IT(&hlpuart1, UART_IT_WUF);
  HAL_UARTEx_EnableStopMode(&hlpuart1); 
  HAL_PWREx_EnableUltraLowPower();
  HAL_PWREx_EnableFastWakeUp();
#endif
	
	 __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_HSI);
	
	
#ifdef notuya		
	HAL_NVIC_DisableIRQ(EXTI0_1_IRQn);
	GPIO_InitTypeDef GPIO_Initure;
	
	/* Enable GPIOs clock */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();

	/* Configure all GPIO port pins in Analog Input mode (floating input trigger OFF) */
	GPIO_Initure.Pin = GPIO_PIN_All;
	GPIO_Initure.Mode = GPIO_MODE_ANALOG;
	GPIO_Initure.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_Initure);
	HAL_GPIO_Init(GPIOB, &GPIO_Initure);
	HAL_GPIO_Init(GPIOC, &GPIO_Initure);

	/* Disable GPIOs clock */
	__HAL_RCC_GPIOA_CLK_DISABLE();
	__HAL_RCC_GPIOB_CLK_DISABLE();
	__HAL_RCC_GPIOC_CLK_DISABLE();
#endif
}

void stop_exit_config(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_GPIOA_CLK_ENABLE();
	
	GPIO_InitStruct.Pin = IR_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(IR_GPIO_Port, &GPIO_InitStruct);
	HAL_NVIC_SetPriority(EXTI4_15_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(EXTI4_15_IRQn);
	
#ifdef notuya
			__HAL_RCC_GPIOB_CLK_ENABLE();
  /*Configure GPIO pin : PtPin */
  GPIO_InitStruct.Pin = hall_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(hall_GPIO_Port, &GPIO_InitStruct);
	/* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_1_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);

  GPIO_InitStruct.Pin = sw2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(sw2_GPIO_Port, &GPIO_InitStruct);
	  HAL_NVIC_SetPriority(EXTI2_3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_3_IRQn);
#endif
}



void enter_stop_mode(void)
{
    //1. 配置stop之前各外设
    system_config_before_stop();
	
    //2.设置唤醒中断
    stop_exit_config();

    //3. 进入stop
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

    //3.退出stop后恢复各外设
//    clock_config_after_stop();

    //4.恢复各外设
    
		
#ifdef tuya 
  HAL_UARTEx_DisableStopMode(&hlpuart1);//唤醒后禁止串口唤醒低功耗模式
#endif
	//HAL_UART_MspInit(&hlpuart1);

  HAL_NVIC_DisableIRQ(EXTI4_15_IRQn);
  HAL_TIM_Base_MspDeInit(&htim2);
  HAL_TIM_Base_MspInit(&htim2);
	IR_Init();
	IR_CaptureCallback();

#ifdef notuya		
	MX_GPIO_Init();
#endif
		
	HAL_GPIO_WritePin(led_GPIO_Port, led_Pin, GPIO_PIN_SET); 
	MX_DMA_Init();	 
	MX_ADC_Init();		
}












































