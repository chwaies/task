#ifndef APP_TIM_H
#define APP_TIM_H



#define TIMPRESCALER      (8400-1)
#define TIMAUTORELORD     (10000-1)


void config_tim7(void);
void tim7_start(void);
void tim7_clear_it(void);

#endif

