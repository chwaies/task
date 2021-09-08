
/*
 *!
 * \file       : key.h
 * \brief      : 
 * \version    : 
 * \date       : 2021/03/30
 * \author     : notrynohigh
 * Last modified by notrynohigh 2021/03/30
 * Copyright (c) 2021 by NOTRYNOHIGH. All Rights Reserved.
 */
   
/*Includes ----------------------------------------------*/
#include "main.h"   
/* \defgroup Key_Exported_TypesDefinitions
 * \{
 */
   
/**
 * \}
 */
   
/* \defgroup Key_Exported_Defines
 * \{
 */
#define TICKDELAY_BUTTON_MODE  			 (1)
#define	ALIENTEK_BUTTON_MODE   			 (2)
#define TIM_BUTTON_MODE 			     (3)

#define KEY_CONFIG_MODE  			 (TIM_BUTTON_MODE)


#define KEY_OFF    1
#define KEY_ON     0  


/**
 * \}
 */
   
/* \defgroup Key_Exported_Macros
 * \{
 */
 
/*  judge key Tick delay mode */ 
#if (KEY_CONFIG_MODE == 1)
typedef GPIO_PinState(*Button_select)(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

/*key status */
typedef enum
{
	BUTTON_ON,
	BUTTON_OFF
}Button_status;

/*not use */
typedef struct
{
	Button_select but_fun;
	GPIO_TypeDef* GPIOx;
	uint16_t GPIO_Pin;
}button_ty;

Button_status Tick_ReadButton(Button_select Pro_fun,GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

#define Key_Status()  								 HAL_GPIO_ReadPin(Key_0_GPIO_Port, Key_0_Pin)
#define EXTEND_DELAY           				 10 																						   /* ms */
#define BUTTON_CALLFUNBASE             HAL_GPIO_ReadPin

#endif




#if (KEY_CONFIG_MODE == 2)


#define KEY0_PRES 					1
#define KEY1_PRES						2
#define KEY2_PRES						3
#define WKUP_PRES  					4

#define KEY0 		HAL_GPIO_ReadPin(Key_0_GPIO_Port, Key_0_Pin)  //PE4
#define KEY1 		HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_3)  					//PE3 
#define KEY2 		HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_2)  					//PE2
#define WK_UP 	HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0)						//PA0

void KEY_Init(void);
uint8_t KEY_Scan(uint8_t mode);

#endif






#if (KEY_CONFIG_MODE == TIM_BUTTON_MODE)

  

#define BUTTON_NUMBER      4 
#define USE_LONG_BUTTON    0 
#define BUTTON_LONG_TIME  2000

#define KEY0	HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_4)
#define KEY1	HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_3)
#define KEY2	HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_2)
#define WK_UP	HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0)

#define KEY0_PRES	1
#define KEY1_PRES	2
#define KEY2_PRES	3
#define WKUP_PRES	4




extern uint8_t KeySta[4];							//按键目前状态
extern uint32_t KeyDownTime[4];					//按键按下时间累积
extern uint8_t KeyCodeMap[4];						//按键编号

void KEY_Init(void);
void KeyScan(void);
void KeyDriver(void);
extern void KeyAction(uint8_t key);


#endif









/**
 * \}
 */
   
/* \defgroup Key_Exported_Variables
 * \{
 */
   
/**
 * \}
 */
   
/* \defgroup Key_Exported_Functions
 * \{
 */
void Key_Readstatus(void);
uint8_t Key_Read_StatusFlag(void);
/**
 * \}
 */
  
/************************ (C) COPYRIGHT NOTRYNOHIGH *****END OF FILE****/

