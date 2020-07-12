/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
//#include "oled.h"
#include "tick.h"
//#include "i2c.h"
//#include "timer32_tempHum.h"
//#include "sht2x.h"
#include "uart1.h"
#include "led.h"
#include "json.h"
//#include "hardware.h"
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
char hsb[3][4]= {};
char data[50]= {};

void sleep(int);
bool judgeReceiveEnd();

struct bme280_dev dev;
int8_t rslt = BME280_OK;
uint8_t settings_sel;
struct bme280_data comp_data;

void mybme280_init()
{
    dev.dev_id = BME280_I2C_ADDR_PRIM;
    dev.intf = BME280_I2C_INTF;
    dev.read = bme280_readI2CwithReg;
    dev.write = bme280_writeI2CwithReg;
    dev.delay_ms = delay_ms;
    bme280_initI2C();
    rslt=bme280_init(&dev);

    dev.settings.osr_h = BME280_OVERSAMPLING_1X;
    dev.settings.osr_p = BME280_OVERSAMPLING_16X;
    dev.settings.osr_t = BME280_OVERSAMPLING_2X;
    dev.settings.filter = BME280_FILTER_COEFF_16;

    settings_sel = BME280_OSR_PRESS_SEL | BME280_OSR_TEMP_SEL | BME280_OSR_HUM_SEL | BME280_FILTER_SEL;

    rslt = bme280_set_sensor_settings(settings_sel, &dev);
}



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

    //userButton_restore();
    uart1_init();

    delay_ms(1000);
    InitQueue(&q);
    printf("bme280 init\n");
    mybme280_init();
    printf("max301000 init\n");
    max30100_setup();
    printf("temperature %f\n",max30100_readTemperature());


    printf("=========log=======\n");

    if(0)
    {
        Send_Response("AT+ILOPRESET\r",strlen("AT+ILOPRESET\r"));
        delay_ms(1000);
        while(!judgeReceiveEnd());
//	//
//Send_Response("AT+ILOPSENDJSON=property,44\r{\"model\":\"ff8080816e85d7a9016e92a9cd800002\"}",strlen("AT+ILOPSENDJSON=property,44\r{\"model\":\"ff8080816e85d7a9016e92a9cd800002\"}"));
//delay_ms(1000);
//while(!judgeReceiveEnd());
//	//


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
        user_button_flag = false;
    }
    else
    {
        Send_Response("AT+ILOPSTATUS?\r",strlen("AT+ILOPSTATUS?\r"));
        Send_Response("AT+ILOPSTATUS?\r",strlen("AT+ILOPSTATUS?\r"));
    }
//			Send_Response("AT+ILOPSENDJSON=property,44\r{\"model\":\"ff8080816e85d7a9016e92a9cd800002\"}",strlen("AT+ILOPSENDJSON=property,44\r{\"model\":\"ff8080816e85d7a9016e92a9cd800002\"}"));
//	    delay_ms(1000);
//	    while(!judgeReceiveEnd());

    /* Run periodic IAQ measurements at defined intervals */
    while (1)
    {
        printf(" go into while\n");


        rslt = bme280_set_sensor_mode(BME280_FORCED_MODE, &dev);
        rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, &dev);
        //printf("Pressure:%u\nTemperature:%u\nHumidity:%u\n",comp_data.pressure,comp_data.temperature,comp_data.humidity);
        printf("%u\n",comp_data.pressure);
        json_value[4] = (float)comp_data.pressure/100;
        json_value[5] = (float)comp_data.temperature/100;

        for(int i=4; i<=5; i++)
        {

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

        for(int j=0; j<100; j++)
        {

            struct pulseoxymeter_t result = max30100_update();
            if( result.pulseDetected == true )
            {
                printf("BEAT\n");

                printf( "BPM: " );
                printf( "%f",result.heartBPM );
                printf( " | " );

                printf( "SaO2: " );
                printf( "%f",result.SaO2 );
                printf( "\n" );

                printf("{P2|BPM|255,40,0|");
                printf("%f",result.heartBPM);
                printf("|SaO2|0,0,255|");
                printf("%f",result.SaO2);
                printf("}\n");

                json_value[2] = result.heartBPM ;
                json_value[3] = result.SaO2 ;

                for(int i=2; i<=3; i++)
                {
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
            }
            else
            {
                //printf("false\n");
            }
            sleep(10);
        }

        //sleep(1000);
    }
    return 0;

}

void sleep(int uint32_ti)
{
    delay_ms(uint32_ti);
}


bool judgeReceiveEnd()
{

}