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
    bme280_set_sensor_mode(BME280_FORCED_MODE, &dev);
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
    //set Internet
    if(0)
    {
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
        user_button_flag = false;
    }
    else
    {
        Send_Response("AT+ILOPSTATUS?\r",strlen("AT+ILOPSTATUS?\r"));
        Send_Response("AT+ILOPSTATUS?\r",strlen("AT+ILOPSTATUS?\r"));
    }
    //sgp30 init
    printf("sgp30 init\n");
    uint16_t sgp30_i = 0;
    int16_t err;
    uint16_t tvoc_ppb, co2_eq_ppm;
    uint32_t iaq_baseline;
    uint16_t ethanol_raw_signal, h2_raw_signal;

    const char *driver_version = sgp30_get_driver_version();
    if (driver_version) {
        printf("SGP30 driver version %s\n", driver_version);
    } else {
        printf("fatal: Getting driver version failed\n");
        return -1;
    }

    /* Busy loop for initialization. The main loop does not work without
    * a sensor. */
    while (sgp30_probe() != STATUS_OK) {
        printf("SGP sensor probing failed\n");
    }
    printf("SGP sensor probing successful\n");

    uint16_t feature_set_version;
    uint8_t product_type;
    err = sgp30_get_feature_set_version(&feature_set_version, &product_type);
    if (err == STATUS_OK) {
        printf("Feature set version: %u\n", feature_set_version);
        printf("Product type: %u\n", product_type);
    } else {
        printf("sgp30_get_feature_set_version failed!\n");
    }
    uint64_t serial_id;
    err = sgp30_get_serial_id(&serial_id);
    if (err == STATUS_OK) {
        printf("SerialID: %llu \n", serial_id);
    } else {
        printf("sgp30_get_serial_id failed!\n");
    }

    /* Read gas raw signals */
    err = sgp30_measure_raw_blocking_read(&ethanol_raw_signal, &h2_raw_signal);
    if (err == STATUS_OK) {
        /* Print ethanol raw signal and h2 raw signal */
        printf("Ethanol raw signal: %u\n", ethanol_raw_signal);
        printf("H2 raw signal: %u\n", h2_raw_signal);
    } else {
        printf("error reading raw signals\n");
    }

    /* Consider the two cases (A) and (B):
        * (A) If no baseline is available or the most recent baseline is more than
        *     one week old, it must discarded. A new baseline is found with
        *     sgp30_iaq_init() */
    err = sgp30_iaq_init();
    if (err == STATUS_OK) {
        printf("sgp30_iaq_init done\n");
    } else {
        printf("sgp30_iaq_init failed!\n");
    } 
    //bme280 init  
	printf("bme280 ini\n");
	mybme280_init();
    //max30100 init
    printf("max301000 init\n");
    max30100_setup();
    delay_ms(1000); 

    printf("=========log=======\n");
    while (1) {
        err = sgp30_measure_iaq_blocking_read(&tvoc_ppb, &co2_eq_ppm);
        if (err == STATUS_OK) {
            printf("tVOC  Concentration: %dppb\n", tvoc_ppb);
            printf("CO2eq Concentration: %dppm\n", co2_eq_ppm);
        } else {
            printf("error reading IAQ values\n");
        }

        /* Persist the current baseline every hour */
        if (++sgp30_i % 3600 == 3599) {
            err = sgp30_get_iaq_baseline(&iaq_baseline);
            if (err == STATUS_OK) {
                /* IMPLEMENT: store baseline to presistent storage */
            }
        }
        json_value[0] = (float)co2_eq_ppm;
        if(tvoc_ppb == 0)
            json_value[1] = 1.0;
        else
            json_value[1] = (float)tvoc_ppb;
        for(int i=0; i<=1; i++)
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
        for(int bme=0;i<=10;i++){
            //bme280
            struct respiration_t res_result = bme280_update();
            if(res_result.pulseDetected == true){
                json_value[4] = res_result.pulseDetected;
            }
            json_value[5] = max30100_readTemperature();
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
            for(int max30=0; max30<10; max30++)
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
        }
		 
    }
    return 0;         
}

void sleep(int uint32_ti){
		delay_ms(uint32_ti);
}


bool judgeReceiveEnd()
{
}
