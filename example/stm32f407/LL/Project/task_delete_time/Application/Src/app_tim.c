#include "app_tim.h"
#include "main.h"

/* TIM6 init function */
void config_tim7(void)
{
  LL_TIM_InitTypeDef TIM_InitStruct = {0};

  /* Peripheral clock enable */
  LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM7);

  /* TIM7 interrupt Init */
  NVIC_SetPriority(TIM7_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),0, 0));
  NVIC_EnableIRQ(TIM7_IRQn);

  TIM_InitStruct.Prescaler = TIMPRESCALER;
  TIM_InitStruct.CounterMode = LL_TIM_COUNTERMODE_UP;
  TIM_InitStruct.Autoreload = TIMAUTORELORD;
  LL_TIM_Init(TIM7, &TIM_InitStruct);
  LL_TIM_DisableARRPreload(TIM7);
  LL_TIM_SetTriggerOutput(TIM7, LL_TIM_TRGO_RESET);
  LL_TIM_DisableMasterSlaveMode(TIM7);
}



void tim7_start(void)
{
	LL_TIM_EnableIT_UPDATE(TIM7);
	/* Enable counter */
	LL_TIM_EnableCounter(TIM7);
	/* Force update generation */
	LL_TIM_GenerateEvent_UPDATE(TIM7);
}




/* clear IT*/
void tim7_clear_it(void)
{
	if(LL_TIM_IsActiveFlag_UPDATE(TIM7) == 1)
	{
		/* Clear the update interrupt flag*/
		LL_TIM_ClearFlag_UPDATE(TIM7);
	}
}

