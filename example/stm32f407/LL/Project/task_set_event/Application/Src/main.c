#include "main.h"
#include "app_usart.h"
#include "printf.h"
#include "app_tim.h"
#include "task.h"
#include "task_app.h"

void SystemClock_Config(void);


int main(void)
{
	/* Enable APB2 peripherals clock. */
	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
	
	/* Check if APB2 peripheral clock is enabled or not */
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);
	
	/*  Set Priority Grouping */
	NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
	
	/* Configure the system clock */
	SystemClock_Config();
	
	/* Peripheral initialization */
	
	/* config usart1 init */
	config_uart1();
	/* config tim7 init */
	config_tim7();
	
	/* task init */
	task_init();
	
	/* task mana init */
	user_task_init();
	
	/* TIM7 start working... */
	tim7_start();
	
	task_run();
	
}




/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  LL_FLASH_SetLatency(LL_FLASH_LATENCY_5);

  if(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_5)
  {
     ERROR_TEXT();
  }
  LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
  LL_RCC_HSE_Enable();

   /* Wait till HSE is ready */
  while(LL_RCC_HSE_IsReady() != 1)
  {
    
  }
  LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSE, LL_RCC_PLLM_DIV_8, 336, LL_RCC_PLLP_DIV_2);
  LL_RCC_PLL_Enable();

   /* Wait till PLL is ready */
  while(LL_RCC_PLL_IsReady() != 1)
  {
    
  }
  LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
  LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_4);
  LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_2);
  LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

   /* Wait till System clock is ready */
  while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL)
  {
  
  }
  LL_Init1msTick(168000000);
  LL_SetSystemCoreClock(168000000);
}


