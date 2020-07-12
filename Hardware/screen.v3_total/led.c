#include "led.h"

bool flag_light_on=false;
//float bright = 100.0;
//float saturation = 100.0;
//float hue = 50.0;

Timer_A_PWMConfig pwm_red_Config =
{
    TIMER_A_CLOCKSOURCE_SMCLK,	 //选用SMCLK作为时钟源，12M
    TIMER_A_CLOCKSOURCE_DIVIDER_32,	 //12M/32 = 375000
    1875,				 //1875/375000=5ms
    TIMER_A_CAPTURECOMPARE_REGISTER_4,   //选用CCR4通道
    TIMER_A_OUTPUTMODE_RESET_SET,         //采用复位/置位
    1875                                        //占空比
};

Timer_A_PWMConfig pwm_green_Config =
{
    TIMER_A_CLOCKSOURCE_SMCLK,
    TIMER_A_CLOCKSOURCE_DIVIDER_32,
    1875,
    TIMER_A_CAPTURECOMPARE_REGISTER_3,
    TIMER_A_OUTPUTMODE_RESET_SET,
    1875
};

Timer_A_PWMConfig pwm_blue_Config =
{
    TIMER_A_CLOCKSOURCE_SMCLK,
    TIMER_A_CLOCKSOURCE_DIVIDER_32,
    1875,
    TIMER_A_CAPTURECOMPARE_REGISTER_1,
    TIMER_A_OUTPUTMODE_RESET_SET,
    1875
};

//const uint8_t port_mapping[] =
//{
//		PM_NONE,PM_NONE,PM_NONE,PM_NONE,PM_TA0CCR1A,PM_NONE,PM_NONE,PM_NONE
//};

void led_init(void)
{
	MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN4 + GPIO_PIN6 + GPIO_PIN7,GPIO_PRIMARY_MODULE_FUNCTION);
	//configure port mapping
	MAP_Timer_A_generatePWM(TIMER_A0_BASE, &pwm_red_Config);
	MAP_Timer_A_generatePWM(TIMER_A0_BASE, &pwm_blue_Config);
	MAP_Timer_A_generatePWM(TIMER_A0_BASE, &pwm_green_Config);
	
}

void led_open_rgb(uint8_t red, uint8_t green, uint8_t blue)
{
	pwm_red_Config.dutyCycle = 1875 * red / 255;
	pwm_blue_Config.dutyCycle = 1875 * blue / 255;
	pwm_green_Config.dutyCycle = 1875 * green / 255;

	MAP_Timer_A_generatePWM(TIMER_A0_BASE, &pwm_red_Config);
	MAP_Timer_A_generatePWM(TIMER_A0_BASE, &pwm_blue_Config);
	MAP_Timer_A_generatePWM(TIMER_A0_BASE, &pwm_green_Config);
	
}

void led_close(void)
{
	pwm_red_Config.dutyCycle = 0;
	pwm_blue_Config.dutyCycle = 0;
	pwm_green_Config.dutyCycle = 0;

	MAP_Timer_A_generatePWM(TIMER_A0_BASE, &pwm_red_Config);
	MAP_Timer_A_generatePWM(TIMER_A0_BASE, &pwm_blue_Config);
	MAP_Timer_A_generatePWM(TIMER_A0_BASE, &pwm_green_Config);
	
}









