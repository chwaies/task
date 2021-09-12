
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
    
    __HAL_RCC_GPIOA_CLK_ENABLE();           //����GPIOAʱ��
    __HAL_RCC_GPIOE_CLK_ENABLE();           //����GPIOEʱ��

    
    GPIO_Initure.Pin=GPIO_PIN_0;            //PA0
    GPIO_Initure.Mode=GPIO_MODE_INPUT;      //����
    GPIO_Initure.Pull=GPIO_PULLDOWN;        //����
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //����
    HAL_GPIO_Init(GPIOA,&GPIO_Initure);
    
		GPIO_Initure.Pin=GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4; //PE2,3,4
    GPIO_Initure.Mode=GPIO_MODE_INPUT;      //����
    GPIO_Initure.Pull=GPIO_PULLUP;          //����
    GPIO_Initure.Speed=GPIO_SPEED_HIGH;     //����
    HAL_GPIO_Init(GPIOE,&GPIO_Initure);
    
}


uint8_t KEY_Scan(uint8_t mode)
{
	static uint8_t key_up=1;//�������ɿ���־
	if(mode)key_up=1;  //֧������		  
	if(key_up&&(KEY0==0||KEY1==0||KEY2==0||WK_UP==1))
	{
		HAL_Delay(20);//ȥ���� 
		key_up=0;
		if(KEY0==0)return 1;
		else if(KEY1==0)return 2;
		else if(KEY2==0)return 3;
		else if(WK_UP==1)return 4;
	}else if(KEY0==1&&KEY1==1&&KEY2==1&&WK_UP==0)key_up=1; 	    
 	return 0;// �ް�������
}

#endif

#if (KEY_CONFIG_MODE == TIM_BUTTON_MODE)



uint8_t KeySta[BUTTON_NUMBER]={1,1,1,1};										 	//����Ŀǰ״̬

uint32_t KeyDownTime[BUTTON_NUMBER]={0,0,0,0};								//��������ʱ���ۻ�

uint8_t KeyCodeMap[BUTTON_NUMBER]={1,2,3,4};							  	//�������

void KEY_Init(void)
{
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOE);
	LL_GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = KEY0_Pin|KEY1_Pin;
	GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
	LL_GPIO_Init(GPIOE, &GPIO_InitStruct);
}

//����ɨ�躯�� ����1ms����һ�� ��ĳ����ʱ���ж���
void KeyScan(void)
{
	uint8_t i;
	static uint8_t keybuf[4]={			//����ɨ�軺����
	0xff,0xff,0xff,0xff};
	
	keybuf[0]=(keybuf[0] << 1)|KEY0;				//��0Ϊ�������°���
	keybuf[1]=(keybuf[1] << 1)|KEY1;
	keybuf[2]=(keybuf[2] << 1)|KEY2;
	keybuf[3]=(keybuf[3] << 1)|(!WK_UP);		//��1Ϊ�������°���  WK_UPһ�����Ϊ0 ����Ϊ1
	
	for(i=0;i<BUTTON_NUMBER;i++)						//����8ms���ȶ�
	{
		if((keybuf[i] & 0xff)==0x00)					//�����ȶ�����
		{
			KeySta[i]=0;
			KeyDownTime[i] += 4;								//4ms��4�൱��1ms��1
		}
		else if((keybuf[i] & 0xff)==0xff)			//�����ȶ�����
		{
			KeySta[i]=1;
		}
		else
		{
			KeyDownTime[i]=0;
		}
	}
}

//������������������������whileѭ����
void KeyDriver(void)
{
	uint8_t i;
	static uint8_t backup[BUTTON_NUMBER]={1,1,1,1};					//ACR_BYTE0_ADDRESSû�а���ʱĬ����1
#if	(USE_LONG_BUTTON == 1)	
	static unsigned long TimeThr[BUTTON_NUMBER]={1000,2000,3000,4000};		//1�봥��
#endif	
	for(i=0;i<BUTTON_NUMBER;i++)
	{
		//click 
		if(backup[i] != KeySta[i])
		{
			if(backup[i] == 0)
			{
				KeyAction(KeyCodeMap[i]);							//���º���һ˲��ִ�ж���
			}
			backup[i] = KeySta[i];
		}
		if(KeyDownTime[i]>0)
		{
			
#if	(USE_LONG_BUTTON == 1)	
			if(KeyDownTime[i] >= TimeThr[i] && KeyDownTime[i] <= TimeThr[i] + 20)//����1��ﵽ����  tacitly approve: if(KeyDownTime[i] >= TimeThr[i])
			{
				printf("%d\n",KeyDownTime[i]);
				//KeyDownTime[i]=0;
				KeyAction(KeyCodeMap[i]);
				//TimeThr[i]= 0;									  	//ȷ����һ����������if����200ms�Ժ�
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

//������������������������whileѭ����
void KeyDriver(void)
{
	unsigned char i;
	static unsigned char backup[4]={1,1,1,1};					//ACR_BYTE0_ADDRESSû�а���ʱĬ����1
	static unsigned long TimeThr[4]={1000,1000,1000,1000};		//1�봥��
	
	for(i=0;i<4;i++)
	{
		if(backup[i] != KeySta[i])
		{
			if(backup[i] == 0)
			{
				KeyAction(KeyCodeMap[i]);						//���º���һ˲��ִ�ж���
			}
			backup[i] = KeySta[i];
		}
		if(KeyDownTime[i]>0)
		{
			if(KeyDownTime[i] >= TimeThr[i])					//����1��ﵽ����
			{
				KeyAction(KeyCodeMap[i]);
				TimeThr[i] += 200;								//ȷ����һ����������if����200ms�Ժ�
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

extern void start_fun(void);
extern void stop_fun(void);
extern int printf_u1(const char* format, ...);
void KeyAction(uint8_t key)
{
	switch(key)
	{
		case KEY0_PRES:
		{
			/* stop timer in task! */
			stop_fun();
		}
			break;
		case KEY1_PRES:
		{
			/* start timer in task! */
			start_fun();
		}
			break;
		case KEY2_PRES:
		{
			printf_u1("2\n");
		}
			break;
		case WKUP_PRES:
		{
			printf_u1("3\n");
		}
			break;
		default:
			break;
	}
}



#endif
/**
 * \}
 */
  
/************************ (C) COPYRIGHT NOTRYNOHIGH *****END OF FILE****/

