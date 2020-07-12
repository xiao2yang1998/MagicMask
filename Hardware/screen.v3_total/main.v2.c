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

char str_AT[][100]={
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
char hsb[3][4]={};
char data[50]={};   /*Store the content of the online debugging module*/
//const char oled_clear_line[OLED_DISPLAY_MAX_CHAR_PER_ROW]="                ";

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
	
    userButton_restore();
		uart1_init();

    delay_ms(1000);
    InitQueue(&q);
    /////////////////////
   uint16_t i = 0;
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
//    err = sgp30_iaq_init();
//    if (err == STATUS_OK) {
//        printf("sgp30_iaq_init done\n");
//    } else {
//        printf("sgp30_iaq_init failed!\n");
//    }
    /* (B) If a recent baseline is available, set it after sgp30_iaq_init() for
     * faster start-up */
    /* IMPLEMENT: retrieve iaq_baseline from presistent storage;
     * err = sgp30_set_iaq_baseline(iaq_baseline);
     */
	printf("start\n");
    mybme280_init();

     printf("=========log=======\n");
    /* Run periodic IAQ measurements at defined intervals */
    while (1) {

        if(user_button_flag){
        Send_Response("AT+ILOPRESET\r",strlen("AT+ILOPRESET\r"));
        delay_ms(1000);
        while(!judgeReceiveEnd());
//					
//	 Send_Response("AT+ILOPSENDJSON=property,44\r{\"model\":\"ff8080816e85d7a9016e92a9cd800002\"}",strlen("AT+ILOPSENDJSON=property,44\r{\"model\":\"ff8080816e85d7a9016e92a9cd800002\"}"));
//    delay_ms(1000);
//    while(!judgeReceiveEnd());
//					
                
        Send_Response("AT+FACTORY\r",strlen("AT+FACTORY\r"));
        delay_ms(1000);
        while(!judgeReceiveEnd());					
        Send_Response(str_AT[0],strlen(str_AT[0]));
        for(int i = 1;i<sizeof(str_AT)/sizeof(str_AT[0]);i++)
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
    if(!isEmpty(&q)){ 
        while(!judgeReceiveEnd()); 
    }
        /*
         * IMPLEMENT: get absolute humidity to enable humidity compensation
         * uint32_t ah = get_absolute_humidity(); // absolute humidity in mg/m^3
         * sgp30_set_absolute_humidity(ah);
         */
//        err = sgp30_measure_iaq_blocking_read(&tvoc_ppb, &co2_eq_ppm);
//        if (err == STATUS_OK) {
//            printf("tVOC  Concentration: %dppb\n", tvoc_ppb);
//            printf("CO2eq Concentration: %dppm\n", co2_eq_ppm);
//        } else {
//            printf("error reading IAQ values\n");
//        }

        /* Persist the current baseline every hour */
//        if (++i % 3600 == 3599) {
//            err = sgp30_get_iaq_baseline(&iaq_baseline);
//            if (err == STATUS_OK) {
//                /* IMPLEMENT: store baseline to presistent storage */
//            }
//        }
				
				
				//bme280
        /* Wait for the measurement to complete and print data @25Hz */
       rslt = bme280_set_sensor_mode(BME280_FORCED_MODE, &dev);
       
       rslt = bme280_get_sensor_data(BME280_ALL, &comp_data, &dev);
       printf("Pressure:%u\nTemperature:%u\nHumidity:%u\n",comp_data.pressure,comp_data.temperature,comp_data.humidity);

        //AT+ILOPSENDJSON=properuty,86\r{"PowerSwitch":0,"PauseSwitch":0,"LocalMassage":2,"GasbagPosition":4,"MassageSpeed":5}
        sprintf(co2_str,"%u",comp_data.pressure);
        sprintf(total_str , "{\\\"co2_eq_ppm\\\":");
        strcat(total_str,"\"");
        strcat(total_str,co2_str);
        strcat(total_str,"\"");
        strcat(total_str,"}");
        sprintf(tmp_str,"%d",strlen(total_str));
        strcat(str_ILOPSENDJSON,tmp_str);
        strcat(str_ILOPSENDJSON,"\r");
        strcat(str_ILOPSENDJSON,total_str);
        sprintf("-------------------%s\n",str_ILOPSENDJSON);
        Send_Response(str_ILOPSENDJSON,strlen(str_ILOPSENDJSON));
        delay_ms(1000);
        while(!judgeReceiveEnd());
        sprintf(str_ILOPSENDJSON,"%s","AT+ILOPSENDJSON=property,");

        /* The IAQ measurement must be triggered exactly once per second (SGP30)
         * to get accurate values.
         */
        sleep(1000);  // SGP30
    }
    return 0;
            
}

void sleep(int uint32_ti){
		delay_ms(uint32_ti);
}


bool judgeReceiveEnd()
{
    if(isEmpty(&q)){ 
        return false;
    }else{
        address = strstr(q.data,"\r");
//        printf("address=====%s\n",address);
        offset = address - &q.data[0];
        offset_1 = (offset+1) % MAXQSIZE;
        offset_2 = (offset+2) % MAXQSIZE;

//        if(!(address != NULL && q.data[offset_1]=='\r' && q.data[offset_2]=='\n')){
//            printf("======not equal\n");
//        }
        
        if((address != NULL && q.data[offset_1]=='\n') || (address != NULL && q.data[offset_1]=='\r' && q.data[offset_2]=='\n')){
            printf("q.front======%d\n",q.front);
            printf("q.rear======%d\n",q.rear);
//            printf("address=====%s\n",address);
            jsonLength = address - &q.data[q.front];
            printf("jsonLength========%d\n",jsonLength);
            if(jsonLength == 0){
                printf("====0=====\n");
                //如果相等，则'\r'之前没有JSON字符串，JSNO字符串内容为空
                for(int i = 0; i<2; i++){
                   DeQueue(&q);
                }
               return false;
                
            }else if(jsonLength > 0){
               //如果前者大于后者，一条完整的JSON信息没有被分隔开
               printf("====normal===\n");
               memcpy(buffer_JSON,&q.data[q.front],jsonLength);
            }else{
               //如果小于，则被JSON字符串被分隔开
//               printf("q.data========%s\n",q.data);
               printf("unnormal,need concat===\n");
               jsonLength = (MAXQSIZE - q.front) + (address - &q.data[0]) ;
               memcpy(buffer_JSON,&q.data[q.front],MAXQSIZE - q.front);
               memcpy(buffer_JSONS_part,q.data,address - &q.data[0]);
               strcat(buffer_JSON,buffer_JSONS_part);
            }
            printf("jsonLength========%d\n",jsonLength);
            printf("buffer_JSON======%s\n",buffer_JSON);
            //Start parsing the first complete JSON message received
            // if(strstr(buffer_JSON,"protocol")){
            //     if(strstr((char *)buffer_JSON,"PowerSwitch")){
            //         char value;
            //         char* address = strstr(buffer_JSON,"PowerSwitch");
            //         value=*(address+strlen("PowerSwitch")+3);
            //         if(value=='1'){
            //             flag_light_on=true;
            //             if(!strstr(buffer_JSON,"hsl")){
            //                 led_open_rgb(255,255,255);
            //                 printf("=====%s\n","first open,white");
            //             }else{
            //                 char* address = strstr(buffer_JSON,"hsl");
            //                 char* address_sign_0 = strstr(address,",");
            //                 int value_length_hue = address_sign_0-(address+strlen("hsl")+4);
            //                 printf("value_length_sat:====%d\n",value_length_hue);
            //                 strncpy(hsb[0],address+strlen("hsl")+4,value_length_hue);
                        
            //                 char* address_sign_1 = strstr(address_sign_0+1,",");
            //                 int value_length_sat = address_sign_1-address_sign_0-1;
            //                 printf("value_length_sat:====%d\n",value_length_sat);
            //                 //						printf("value_length_sat:%d\n",value_length_sat);
            //                 strncpy(hsb[1],address_sign_0+1,value_length_sat);	
            //                 char* address_sign_2 = strstr(address_sign_1+1,"]");
            //                 int value_length_bright = address_sign_2-address_sign_1-1;
            //                 printf("value_length_bright:====%d\n",value_length_bright);
            //                 //						printf("value_length_bright:%d\n",value_length_bright);							
            //                 strncpy(hsb[2],address_sign_1+1,value_length_bright);

            //                 printf("====%s\n",hsb[0]);
            //                 printf("====%s\n",hsb[1]);
            //                 printf("====%s\n",hsb[2]);		
                            
            //                 if(strstr(buffer_JSON,"sat") || strstr(buffer_JSON,"Brightness")){
            //                     printf("====%s\n","come into sat or bri");	
            //                     char* address_sat = strstr(buffer_JSON,"sat");
            //                     char* address_sign_0 = strstr(address_sat+strlen("sat")+3,",");
            //                     int value_length_sat = address_sign_0-(address_sat+strlen("sat")+3);
            //                     printf("value_length_sat:====%d\n",value_length_sat);
            //                     memset(hsb[1],0,sizeof(hsb[1]));
            //                     strncpy(hsb[1],address_sat+strlen("sat")+3,value_length_sat);	
                                
            //                     char* address_bright = strstr(buffer_JSON,"Brightness");
            //                     char* address_sign_1= strstr(address_bright+strlen("Brightness")+3,",");
            //                     int value_length_bright = address_sign_1-(address_bright+strlen("Brightness")+3);
            //                     printf("value_length_bright:====%d\n",value_length_bright);
            //                     memset(hsb[2],0,sizeof(hsb[2]));
            //                     strncpy(hsb[2],address_bright+strlen("Brightness")+3,value_length_bright);
                                
            //                     printf("====%s\n",hsb[0]);
            //                     printf("====%s\n",hsb[1]);
            //                     printf("====%s",hsb[2]);	
            //                 }
            //                 color_led_open_hsb(atoi(hsb[0]),atoi(hsb[1]),atoi(hsb[2]));
            //             }
            //         }else{
            //             led_close();
            //             flag_light_on=false;
            //         }
            //     }else if(strstr(buffer_JSON,"data")){
            //         length = 0;
            //         char* address = strstr(buffer_JSON,"data");
            //         char* address_sign = strstr(address+strlen("data")+5,"\\");
            //         int value_length = address_sign-(address+strlen("data")+5);
            //         printf("value_length:====%d\n",value_length);
            //         strncpy(data,address+strlen("data")+5,value_length);
            //         OLED_ShowString(OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_4, (char *)oled_clear_line);
            //         OLED_ShowString(OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_4, data);
            //         //concat string which is to send
            //         initKeyValueList(keyValueList);
            //         initKeyValueList(keyValueList_1);
                    
            //         setString(keyValueList_1,"data",data);
            // //				char* str1 = toString(keyValueList_1);
            // //				printf("====after toString %s\n",str1);

            //         length = 0;
            //         setJSON(keyValueList,"protocol",keyValueList_1);
            //         char* str2 = toString(keyValueList);
            //         printf("====after toString %s\n",str2);
                    
            // //                    AT+ILOPSENDJSON=property,86\r{"PowerSwitch":0,"PauseSwitch":0,"LocalMassage":2,"GasbagPosition":4,"MassageSpeed":5}
            //         sprintf(tmp_str,"%d",strlen(str));
            //         strcat((char*)str_ILOPSENDJSON,tmp_str);
            //         strcat((char*)str_ILOPSENDJSON,"\r");
            //         strcat((char*)str_ILOPSENDJSON,str);
            //         printf("====report data %s\n",str_ILOPSENDJSON);
            //         Send_Response((char*)str_ILOPSENDJSON,strlen(str_ILOPSENDJSON));
            //     }         
            // }
//            memset(USART_RX_BUF_ADDPRE,0,sizeof(USART_RX_BUF_ADDPRE));
            memset(hsb[0],0,sizeof(hsb[0]));
            memset(hsb[1],0,sizeof(hsb[1]));
            memset(hsb[2],0,sizeof(hsb[2]));
            memset(data,0,sizeof(data));
            sprintf(str_ILOPSENDJSON,"%s","AT+ILOPSENDJSON=property,");
            memset(tmp_str,0,sizeof(tmp_str));
            memset(str,0,sizeof(str));
            memset(buffer_JSONS_part,0,sizeof(buffer_JSONS_part));
//            memset(buffer_JSONS_whole,0,sizeof(buffer_JSONS_whole));
            memset(buffer_JSON,0,sizeof(buffer_JSON));
            
            if(address != NULL && q.data[offset_1]=='\r' && q.data[offset_2]=='\n'){
                jsonLength = jsonLength+3; 
            }else{
                jsonLength = jsonLength+2; 
            }
            for(int i = 0;i < jsonLength; i++){
                DeQueue(&q);
            }
            printf("after dequeue print,the element=========\n");
            printf("====队列内容%s\n",q.data);
            //printf("after dequeue print,the element=========%c %c %c\n",q.data[q.front],q.data[q.front+1],q.data[q.front+2]);
            return true;
        }else{
            return false;
        }
    }

}
