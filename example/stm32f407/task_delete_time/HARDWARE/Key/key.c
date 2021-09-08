
/*
 *!
 * \file       :key.c
 * \brief      : 
 * \version    : 
 * \date       : 2021/03/30
 * \author     : notrynohigh
 *Last modified by notrynohigh 2021/03/30
 *Copyright (c) 2021 by NOTRYNOHIGH. All Rights Reserved.
 */
   
/*Includes ----------------------------------------------*/

#include "Key.h"
#include "task_mem.h"
#include <string.h>
extern hpSeBind tkhpHandler;

#if (KEY_CONFIG_MODE == TICKDELAY_BUTTON_MODE)

extern uint32_t Old_tick;

Button_status Tick_ReadButton(Button_select Pro_fun,GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	static uint8_t status = 0 ;
	static uint32_t New_tick; 

	New_tick  = HAL_GetTick();
    
	if(New_tick > (Old_tick+EXTEND_DELAY))
	{
		Old_tick = New_tick;
		if(Pro_fun(GPIOx, GPIO_Pin) == KEY_ON)
		{
			if(status == 0)
			{
				status = 1;
			}
			else if(status == 1)
			{
				status = 2 ;
				//printf("hi key!\n");
				return BUTTON_ON;

			}
		}
		else 
		{
			if(status == 2)
			{
				status = 0 ;//clear flags
				//printf("hi keykey!\n");
			}
		}
	}

	return BUTTON_OFF;

}	

#endif





/* ALIENTEK example key mode */
#if (KEY_CONFIG_MODE == ALIENTEK_BUTTON_MODE)


void KEY_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_GPIOA_CLK_ENABLE();           //开启GPIOA时钟
    __HAL_RCC_GPIOE_CLK_ENABLE();           //开启GPIOE时钟

    
    GPIO_Initure.Pin=GPIO_PIN_0;            //PA0
    GPIO_Initure.Mode=GPIO_MODE_INPUT;      //输入
    GPIO_Initure.Pull=GPIO_PULLDOWN;        //下拉
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);
    
		GPIO_Initure.Pin=GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4; //PE2,3,4
    GPIO_Initure.Mode=GPIO_MODE_INPUT;      //输入
    GPIO_Initure.Pull=GPIO_PULLUP;          //上拉
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
    HAL_GPIO_Init(GPIOE,&GPIO_Initure);
    
}


uint8_t KEY_Scan(uint8_t mode)
{
	static uint8_t key_up=1;//按键按松开标志
	if(mode)key_up=1;  //支持连按		  
	if(key_up&&(KEY0==0||KEY1==0||KEY2==0||WK_UP==1))
	{
		HAL_Delay(20);//去抖动 
		key_up=0;
		if(KEY0==0)return 1;
		else if(KEY1==0)return 2;
		else if(KEY2==0)return 3;
		else if(WK_UP==1)return 4;
	}else if(KEY0==1&&KEY1==1&&KEY2==1&&WK_UP==0)key_up=1; 	    
 	return 0;// 无按键按下
}

#endif

#if (KEY_CONFIG_MODE == TIM_BUTTON_MODE)



uint8_t KeySta[BUTTON_NUMBER]={1,1,1,1};										 	//按键目前状态

uint32_t KeyDownTime[BUTTON_NUMBER]={0,0,0,0};								//按键按下时间累积

uint8_t KeyCodeMap[BUTTON_NUMBER]={1,2,3,4};							  	//按键编号

void KEY_Init(void)
{
    GPIO_InitTypeDef GPIO_Initure;
    
    __HAL_RCC_GPIOA_CLK_ENABLE();           //开启GPIOA时钟
    __HAL_RCC_GPIOE_CLK_ENABLE();           //开启GPIOE时钟

    
    GPIO_Initure.Pin=GPIO_PIN_0;            //PA0
    GPIO_Initure.Mode=GPIO_MODE_INPUT;      //输入
    GPIO_Initure.Pull=GPIO_PULLDOWN;        //下拉
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);
    
	GPIO_Initure.Pin=GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4; //PE2,3,4
    GPIO_Initure.Mode=GPIO_MODE_INPUT;      //输入
    GPIO_Initure.Pull=GPIO_PULLUP;          //上拉
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //高速
    HAL_GPIO_Init(GPIOE,&GPIO_Initure);
	
}

//按键扫描函数 函数1ms运行一次 在某个定时器中断中
void KeyScan(void)
{
	uint8_t i;
	static uint8_t keybuf[4]={			//按键扫描缓冲区
	0xff,0xff,0xff,0xff};
	
	keybuf[0]=(keybuf[0] << 1)|KEY0;				//是0为正常按下按键
	keybuf[1]=(keybuf[1] << 1)|KEY1;
	keybuf[2]=(keybuf[2] << 1)|KEY2;
	keybuf[3]=(keybuf[3] << 1)|(!WK_UP);		//是1为正常按下按键  WK_UP一般情况为0 按下为1
	
	for(i=0;i<BUTTON_NUMBER;i++)						//按下8ms才稳定
	{
		if((keybuf[i] & 0xff)==0x00)					//按键稳定按下
		{
			KeySta[i]=0;
			KeyDownTime[i] += 4;								//4ms加4相当于1ms加1
		}
		else if((keybuf[i] & 0xff)==0xff)			//按键稳定弹起
		{
			KeySta[i]=1;
		}
		else
		{
			KeyDownTime[i]=0;
		}
	}
}

//按键驱动函数，在主函数的while循环中
void KeyDriver(void)
{
	uint8_t i;
	static uint8_t backup[BUTTON_NUMBER]={1,1,1,1};					//ACR_BYTE0_ADDRESS没有按下时默认是1
#if	(USE_LONG_BUTTON == 1)	
	static unsigned long TimeThr[BUTTON_NUMBER]={1000,2000,3000,4000};		//1秒触发
#endif	
	for(i=0;i<BUTTON_NUMBER;i++)
	{
		//click 
		if(backup[i] != KeySta[i])
		{
			if(backup[i] == 0)
			{
				KeyAction(KeyCodeMap[i]);							//按下后弹起一瞬间执行动作
			}
			backup[i] = KeySta[i];
		}
		if(KeyDownTime[i]>0)
		{
			
#if	(USE_LONG_BUTTON == 1)	
			if(KeyDownTime[i] >= TimeThr[i] && KeyDownTime[i] <= TimeThr[i] + 20)//长按1秒达到触发  tacitly approve: if(KeyDownTime[i] >= TimeThr[i])
			{
				printf("%d\n",KeyDownTime[i]);
				//KeyDownTime[i]=0;
				KeyAction(KeyCodeMap[i]);
				//TimeThr[i]= 0;									  	//确保下一次满足上面if是在200ms以后
			}
//			else
//			{
//				KeyDownTime[i]=0;
//			}
#else
			
#endif			
		}
		else
		{
			//TimeThr[i]=8000;
			
		}
	}
}

//customary
#if 0

//按键驱动函数，在主函数的while循环中
void KeyDriver(void)
{
	unsigned char i;
	static unsigned char backup[4]={1,1,1,1};					//ACR_BYTE0_ADDRESS没有按下时默认是1
	static unsigned long TimeThr[4]={1000,1000,1000,1000};		//1秒触发
	
	for(i=0;i<4;i++)
	{
		if(backup[i] != KeySta[i])
		{
			if(backup[i] == 0)
			{
				KeyAction(KeyCodeMap[i]);						//按下后弹起一瞬间执行动作
			}
			backup[i] = KeySta[i];
		}
		if(KeyDownTime[i]>0)
		{
			if(KeyDownTime[i] >= TimeThr[i])					//长按1秒达到触发
			{
				KeyAction(KeyCodeMap[i]);
				TimeThr[i] += 200;								//确保下一次满足上面if是在200ms以后
			}
		}
		else
		{
			TimeThr[i]=1000;
		}
	}
}
#endif


#include "task.h"

//extern taskType task1;
extern void fun (void);
void KeyAction(uint8_t key)
{
	switch(key)
	{
		case KEY0_PRES:
		{
			fun();
		}
			break;
		case KEY1_PRES:
		{
			printf("1\n");
		}
			break;
		case KEY2_PRES:
		{
			printf("2\n");
		}
			break;
		case WKUP_PRES:
		{
			printf("3\n");
		}
			break;
		default:
			break;
	}
}

extern TIM_HandleTypeDef htim7;
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim == &htim7)
	{
		KeyScan();
	}
}



#endif
/**
 * \}
 */
  
/************************ (C) COPYRIGHT NOTRYNOHIGH *****END OF FILE****/

