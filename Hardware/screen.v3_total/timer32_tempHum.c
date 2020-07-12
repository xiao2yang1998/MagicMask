#include "timer32_tempHum.h"

bool flag_SHT20 = false;

void timer32_Init(void){
            
    /* 配置Timer32的第一个计数器为32位模式，周期模式，定时器频率=MCLK/定时器分频系数 此处第一个计数器频率=MCLK=1M */
    MAP_Timer32_initModule(TIMER32_0_BASE, TIMER32_PRESCALER_1, TIMER32_32BIT,TIMER32_PERIODIC_MODE);
    
    /* 使能Timer32第一个计数器中断开关*/
    MAP_Interrupt_enableInterrupt(INT_T32_INT1);    
    MAP_Timer32_enableInterrupt(TIMER32_0_BASE);
    
//    MAP_Interrupt_enableMaster();/*使能中断总开关*/
    MAP_Timer32_setCount(TIMER32_0_BASE,60000000); /*设置Timer32第一个计数器计数周期，此处 60M/12MHz=5s*/  
    
    MAP_Timer32_startTimer(TIMER32_0_BASE, false);/*Timer32第一个计数器以周期性模式开始计数*/
}

/* Timer32第一个计数器中断函数*/
void T32_INT1_IRQHandler(void)
{
    MAP_Timer32_clearInterruptFlag(TIMER32_0_BASE); /*清除Timer32第一个计数器中断标志*/  
    flag_SHT20 = true;
}
