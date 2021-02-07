#include "motor.h"
#include "main.h"
#include "tim.h"

#define tuya

#ifdef tuya	
#include "wifi.h"

uint8_t Last_STATE=0x02;
#endif	

extern uint8_t CONTROL;
extern uint16_t time;
extern uint8_t hall_flag;
extern uint16_t now,min,max;

void open(void)
{ 
	if(CONTROL == 0x02)
	{
		time=0;
		HAL_TIM_Base_Stop(&htim21);
		__HAL_TIM_SetCounter(&htim21,0);
	}
	if(now> min+1)
	{
	  CONTROL=0x00;
		#ifdef tuya	
	  Last_STATE=0x00;
		#endif
		HAL_GPIO_WritePin(motoren_GPIO_Port, motoren_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(motora_GPIO_Port, motora_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(motorb_GPIO_Port, motorb_Pin, GPIO_PIN_RESET);
		#ifdef tuya
	  mcu_dp_enum_update(DPID_CONTROL, CONTROL);
    #endif
	}
}

void close(void)
{ 
	if(CONTROL == 0x00)
	{
		time=0;
		HAL_TIM_Base_Stop(&htim21);
		__HAL_TIM_SetCounter(&htim21,0);
	}
	if(now< max-1)
	{
		CONTROL=0x02;
	  #ifdef tuya	
		Last_STATE=0x02;
	  #endif
		HAL_GPIO_WritePin(motoren_GPIO_Port, motoren_Pin, GPIO_PIN_SET);
		HAL_GPIO_WritePin(motora_GPIO_Port, motora_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(motorb_GPIO_Port, motorb_Pin, GPIO_PIN_SET);
		#ifdef tuya	
	  mcu_dp_enum_update(DPID_CONTROL, CONTROL);
    #endif
	}
}

void stop(void)
{
	time=0;
	HAL_TIM_Base_Stop(&htim21);
	__HAL_TIM_SET_COUNTER(&htim21,0);
	if(HAL_GPIO_ReadPin(hall_GPIO_Port, hall_Pin)==0)
	{
		hall_flag=0;
		if(CONTROL == 0x02)
		{
			
			HAL_GPIO_WritePin(motoren_GPIO_Port, motoren_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(motora_GPIO_Port, motora_Pin, GPIO_PIN_RESET);
	    HAL_GPIO_WritePin(motorb_GPIO_Port, motorb_Pin, GPIO_PIN_SET);
		}
		else //if(Reversible == 1)
		{
			HAL_GPIO_WritePin(motoren_GPIO_Port, motoren_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(motora_GPIO_Port, motora_Pin, GPIO_PIN_SET);
    	HAL_GPIO_WritePin(motorb_GPIO_Port, motorb_Pin, GPIO_PIN_RESET);
		}
	}
	if(HAL_GPIO_ReadPin(hall_GPIO_Port, hall_Pin)==1)	
	{
		CONTROL=0x01;
		HAL_GPIO_WritePin(motora_GPIO_Port, motora_Pin, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(motorb_GPIO_Port, motorb_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(motoren_GPIO_Port, motoren_Pin, GPIO_PIN_RESET);
		#ifdef tuya	
	  mcu_dp_enum_update(DPID_CONTROL, CONTROL);
    #endif
	}	
}
