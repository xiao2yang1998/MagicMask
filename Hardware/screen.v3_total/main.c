/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "tick.h"
#include "uart1.h"
#include "led.h"
#include "json.h"
#include "queue.h"
#include "userButton.h"
#include "sgp30.h"
#include "bme280.h"
#include "MAX303100.h"

//0"CO2",1"ALC",2"BPM",3"SaO2",4"PRE",5"TEMP"
static char json_key[6][10] = {"CO2","ALC","BPM","SaO2","PRE","TEMP"};
static float json_value[6];

char str_AT[][100]=
{
    //	"AT+ILOPSTATUS?\r",
    //	"AT+FWVER?\r",
    //	"AT+ILOPRESET\r",
    //	"AT+WJAPIP?\r",
    //	"AT+ILOPSTOP\r",
    //	"AT+FACTORY\r",

    "AT\r",
    "AT+UARTE=OFF\r",
    "AT+ILOPSET=a11407CgHIo,zHgpYHHnS4aWgdWu,4n5TLRxgmkmYXtnx1pmqMt68LJrTh6X7,YingJiaHao\r",
    "AT+ILOPDOMAIN=0\r",
    "AT+ILOPSTART\r",
    "AT+ILOPAWSSTART\r",
    "AT+ILOPSTATUS",
    "AT+FWVER?"
};
static char str_ILOPSENDJSON[200] = "AT+ILOPSENDJSON=property,";
static char tmp_str[4]; /*use to save the length of str which is a jsonString repoting to the Web*/
static char co2_str[4];
static char total_str[50];

static int jsonLength=0;
static int count_enter = 0;
static int offset = 0;
static int offset_1 = 0;
static int offset_2 = 0;
static char* address = NULL;
char data[50]= {};

void sleep(int);
bool judgeReceiveEnd();
	
int main(void)
{
    MAP_WDT_A_holdTimer();      /* 关闭看门狗 */
    MAP_FPU_enableModule();     

    //set clock
    MAP_CS_setDCOFrequency(12000000);   /* 设置DCO频率为指定频率，此处DCO=12M*/ 
    MAP_CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    MAP_SysTick_enableModule();
    MAP_SysTick_setPeriod(12000);

    MAP_SysTick_enableInterrupt();
    MAP_Interrupt_enableMaster(); 

	uart1_init();

    delay_ms(1000);
    InitQueue(&q);
		printf("=========log=======\n");
		while(!judgeReceiveEnd());
    //set Internet
    bool set_Internet = false;
    if(set_Internet){
        Send_Response("AT+ILOPRESET\r",strlen("AT+ILOPRESET\r"));
        delay_ms(1000);
        while(!judgeReceiveEnd());  
        //Send_Response("AT+ILOPSENDJSON=property,44\r{\"model\":\"ff8080816e85d7a9016e92a9cd800002\"}",strlen("AT+ILOPSENDJSON=property,44\r{\"model\":\"ff8080816e85d7a9016e92a9cd800002\"}"));
        //delay_ms(1000);
        //while(!judgeReceiveEnd());
        //ff8080816e85d7a9016e92a9cd800002
        Send_Response("AT+FACTORY\r",strlen("AT+FACTORY\r"));
        delay_ms(1000);
        while(!judgeReceiveEnd());
        Send_Response(str_AT[0],strlen(str_AT[0]));
        for(int i = 1; i<sizeof(str_AT)/sizeof(str_AT[0]); i++)
        {
            delay_ms(1000);
            while(!judgeReceiveEnd());
            Send_Response(str_AT[i],strlen(str_AT[i]));
        }
        //Ensure that the last one also receives a response and prints
        delay_ms(1000);
        while(!judgeReceiveEnd());
    }else
    {
			while(!judgeReceiveEnd());
        Send_Response("AT+ILOPSTATUS?\r",strlen("AT+ILOPSTATUS?\r"));
        delay_ms(1000);
        while(!judgeReceiveEnd());
    }
    
        
    //send response
    while(1){
        //0"CO2",1"ALC"
        json_value[0] = (float)7.7;
        json_value[1] = (float)77.7;
        for(int i=0; i<=1; i++)
        {
					  while(!judgeReceiveEnd());
            length = 0;
            initKeyValueList(keyValueList);
            initKeyValueList(keyValueList_1);
            memset(str,0,sizeof(str));
            setDouble(keyValueList_1,json_key[i],json_value[i]);
            length = 0;
            setJSON(keyValueList,"protocol",keyValueList_1);
            char* str2 = toString(keyValueList);
            sprintf(tmp_str,"%d",strlen(str));
            strcat((char*)str_ILOPSENDJSON,tmp_str);
            strcat((char*)str_ILOPSENDJSON,"\r");
            strcat((char*)str_ILOPSENDJSON,str);
            printf("====report data %s\n",str_ILOPSENDJSON);
            Send_Response((char*)str_ILOPSENDJSON,strlen(str_ILOPSENDJSON));
            sprintf(str_ILOPSENDJSON,"%s","AT+ILOPSENDJSON=property,");
            memset(tmp_str,0,sizeof(tmp_str));
            memset(str,0,sizeof(str));
					
        }
        sleep(2000);
    }  

    return 0;         
}

void sleep(int uint32_ti){
		delay_ms(uint32_ti);
}


bool judgeReceiveEnd()
{
}
