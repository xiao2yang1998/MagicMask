/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "oled.h"
#include "tick.h"
#include "i2c.h"
#include "timer32_tempHum.h"
#include "sht2x.h"
#include "uart1.h"
#include "led.h"
#include "json.h"
#include "hardware.h"
#include "queue.h"
#include "userButton.h"
#include "../sgp30.h"


#define  I2C_ADDR_W 0x40
uint8_t* WJAP_BUF=NULL;
uint8_t* ILOPSTATUS_BUF;
char str_AT[][100]={
    //	"AT+ILOPSTATUS?\r",
    //	"AT+FWVER?\r",
    //	"AT+ILOPRESET\r",
    //	"AT+WJAPIP?\r",
    //	"AT+ILOPSTOP\r",
    //	"AT+FACTORY\r",

    "AT\r",
    "AT+UARTE=OFF\r",
    "AT+ILOPSET=a11407CgHIo,zHgpYHHnS4aWgdWu,N2uryMo2mNrCHFE0Iq0HhMhvKJgTRyGB,zhuming\r",
    "AT+ILOPDOMAIN=0\r",
    "AT+ILOPSTART\r",
    "AT+ILOPAWSSTART\r",
	"AT+ILOPSTATUS",
	"AT+FWVER?"
};

typedef enum {
    TRIG_TEMP_MEASUREMENT_HM   = 0xE3,
    TRIG_HUMI_MEASUREMENT_HM   = 0xE5,
    TRIG_TEMP_MEASUREMENT_POLL = 0xF3,
    TRIG_HUMI_MEASUREMENT_POLL = 0xF5,
    USER_REG_W                 = 0xE6,
    USER_REG_R                 = 0xE7,
    SOFT_RESET                 = 0xFE
} SHT2xCommand;
const char oled_clear_line[OLED_DISPLAY_MAX_CHAR_PER_ROW]="                ";
static float temperature = 24.00;
static float humidity = 50.00;
static char tempHum_key[2][18] = {"Temperature","RelativeHumidity"};
static float tempHum_value[2];
static char* temp;
static char* Hum;
char sensor_display[2][OLED_DISPLAY_MAX_CHAR_PER_ROW + 1];
char hsb[3][4]={};
char data[50]={};   /*Store the content of the online debugging module*/
static char str_ILOPSENDJSON[200] = "AT+ILOPSENDJSON=property,";
static char tmp_str[4]; /*use to save the length of str which is a jsonString repoting to the Web*/
static int jsonLength=0;
static int count_enter = 0;
static int offset = 0;   
static int offset_1 = 0;
static int offset_2 = 0;
static char* address = NULL;
//static bool flag_ATClear;
//static int queueLength_delete=0;
//char sat[4]={};
//char bright[4]={};

bool judgeReceiveEnd();
uint32_t uart_rec();
void sleep();
//void euscib1IntHandler();
	
int main(void)
{
    MAP_WDT_A_holdTimer();      /* ????? */
    MAP_FPU_enableModule();     

    //set clock
    MAP_CS_setDCOFrequency(12000000);   /* ??DCO???????,??DCO=12M*/ 
    MAP_CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
//		MAP_CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
//		MAP_CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1); //12M
    
//    ms_ticker_init();
    MAP_SysTick_enableModule();
    MAP_SysTick_setPeriod(12000);
//    MAP_SysTick_setPeriod(24000);
//    MAP_SysTick_setPeriod(48000);

    MAP_SysTick_enableInterrupt();
    MAP_Interrupt_enableMaster(); 
		//sensirion_i2c_init();
		
//    MAP_Interrupt_registerInterrupt(INT_EUSCIB1,euscib1IntHandler);
    static void *i2c = NULL; 
	  timer32_Init();         /*Timer32 module initialization, temperature and humidity reading module needs timing*/
    SHT2x_Init();  
    
    userButton_restore();
    
//    SHT20_task_init();
	OLED_Init();
	OLED_Clear(); 
	
	uart1_init();
	led_init();	
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
    err = sgp30_iaq_init();
    if (err == STATUS_OK) {
        printf("sgp30_iaq_init done\n");
    } else {
        printf("sgp30_iaq_init failed!\n");
    }
    /* (B) If a recent baseline is available, set it after sgp30_iaq_init() for
     * faster start-up */
    /* IMPLEMENT: retrieve iaq_baseline from presistent storage;
     * err = sgp30_set_iaq_baseline(iaq_baseline);
     */

    /* Run periodic IAQ measurements at defined intervals */
    while (1) {
        /*
         * IMPLEMENT: get absolute humidity to enable humidity compensation
         * uint32_t ah = get_absolute_humidity(); // absolute humidity in mg/m^3
         * sgp30_set_absolute_humidity(ah);
         */
        err = sgp30_measure_iaq_blocking_read(&tvoc_ppb, &co2_eq_ppm);
        if (err == STATUS_OK) {
            printf("tVOC  Concentration: %dppb\n", tvoc_ppb);
            printf("CO2eq Concentration: %dppm\n", co2_eq_ppm);
        } else {
            printf("error reading IAQ values\n");
        }

        /* Persist the current baseline every hour */
        if (++i % 3600 == 3599) {
            err = sgp30_get_iaq_baseline(&iaq_baseline);
            if (err == STATUS_OK) {
                /* IMPLEMENT: store baseline to presistent storage */
            }
        }

        /* The IAQ measurement must be triggered exactly once per second (SGP30)
         * to get accurate values.
         */
        sleep(1000);  // SGP30
    }
    return 0;
            
}

void sleep(uint32_ti){
		delay_ms(uint32_ti);
}

int indexOf(char* str,char c){
    for(int i= 0; i<strlen(str); i++){
        if(c==str[i]){
            count_enter++;
        }
    }
    return count_enter;
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
                //????,?'\r'????JSON???,JSNO???????
                for(int i = 0; i<2; i++){
                   DeQueue(&q);
                }
               return false;
                
            }else if(jsonLength > 0){
               //????????,?????JSON????????
               printf("====normal===\n");
               memcpy(buffer_JSON,&q.data[q.front],jsonLength);
            }else{
               //????,??JSON???????
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
            if(strstr(buffer_JSON,"protocol")){
                if(strstr((char *)buffer_JSON,"PowerSwitch")){
                    char value;
                    char* address = strstr(buffer_JSON,"PowerSwitch");
                    value=*(address+strlen("PowerSwitch")+3);
                    if(value=='1'){
                        flag_light_on=true;
                        if(!strstr(buffer_JSON,"hsl")){
                            led_open_rgb(255,255,255);
                            printf("=====%s\n","first open,white");
                        }else{
                            char* address = strstr(buffer_JSON,"hsl");
                            char* address_sign_0 = strstr(address,",");
                            int value_length_hue = address_sign_0-(address+strlen("hsl")+4);
                            printf("value_length_sat:====%d\n",value_length_hue);
                            strncpy(hsb[0],address+strlen("hsl")+4,value_length_hue);
                        
                            char* address_sign_1 = strstr(address_sign_0+1,",");
                            int value_length_sat = address_sign_1-address_sign_0-1;
                            printf("value_length_sat:====%d\n",value_length_sat);
                            //						printf("value_length_sat:%d\n",value_length_sat);
                            strncpy(hsb[1],address_sign_0+1,value_length_sat);	
                            char* address_sign_2 = strstr(address_sign_1+1,"]");
                            int value_length_bright = address_sign_2-address_sign_1-1;
                            printf("value_length_bright:====%d\n",value_length_bright);
                            //						printf("value_length_bright:%d\n",value_length_bright);							
                            strncpy(hsb[2],address_sign_1+1,value_length_bright);

                            printf("====%s\n",hsb[0]);
                            printf("====%s\n",hsb[1]);
                            printf("====%s\n",hsb[2]);		
                            
                            if(strstr(buffer_JSON,"sat") || strstr(buffer_JSON,"Brightness")){
                                printf("====%s\n","come into sat or bri");	
                                char* address_sat = strstr(buffer_JSON,"sat");
                                char* address_sign_0 = strstr(address_sat+strlen("sat")+3,",");
                                int value_length_sat = address_sign_0-(address_sat+strlen("sat")+3);
                                printf("value_length_sat:====%d\n",value_length_sat);
                                memset(hsb[1],0,sizeof(hsb[1]));
                                strncpy(hsb[1],address_sat+strlen("sat")+3,value_length_sat);	
                                
                                char* address_bright = strstr(buffer_JSON,"Brightness");
                                char* address_sign_1= strstr(address_bright+strlen("Brightness")+3,",");
                                int value_length_bright = address_sign_1-(address_bright+strlen("Brightness")+3);
                                printf("value_length_bright:====%d\n",value_length_bright);
                                memset(hsb[2],0,sizeof(hsb[2]));
                                strncpy(hsb[2],address_bright+strlen("Brightness")+3,value_length_bright);
                                
                                printf("====%s\n",hsb[0]);
                                printf("====%s\n",hsb[1]);
                                printf("====%s",hsb[2]);	
                            }
                            color_led_open_hsb(atoi(hsb[0]),atoi(hsb[1]),atoi(hsb[2]));
                        }
                    }else{
                        led_close();
                        flag_light_on=false;
                    }
                }else if(strstr(buffer_JSON,"data")){
                    length = 0;
                    char* address = strstr(buffer_JSON,"data");
                    char* address_sign = strstr(address+strlen("data")+5,"\\");
                    int value_length = address_sign-(address+strlen("data")+5);
                    printf("value_length:====%d\n",value_length);
                    strncpy(data,address+strlen("data")+5,value_length);
                    OLED_ShowString(OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_4, (char *)oled_clear_line);
                    OLED_ShowString(OLED_DISPLAY_COLUMN_START, OLED_DISPLAY_ROW_4, data);
                    //concat string which is to send
                    initKeyValueList(keyValueList);
                    initKeyValueList(keyValueList_1);
                    
                    setString(keyValueList_1,"data",data);
            //				char* str1 = toString(keyValueList_1);
            //				printf("====after toString %s\n",str1);

                    length = 0;
                    setJSON(keyValueList,"protocol",keyValueList_1);
                    char* str2 = toString(keyValueList);
                    printf("====after toString %s\n",str2);
                    
            //                    AT+ILOPSENDJSON=property,86\r{"PowerSwitch":0,"PauseSwitch":0,"LocalMassage":2,"GasbagPosition":4,"MassageSpeed":5}
                    sprintf(tmp_str,"%d",strlen(str));
                    strcat((char*)str_ILOPSENDJSON,tmp_str);
                    strcat((char*)str_ILOPSENDJSON,"\r");
                    strcat((char*)str_ILOPSENDJSON,str);
                    printf("====report data %s\n",str_ILOPSENDJSON);
                    Send_Response((char*)str_ILOPSENDJSON,strlen(str_ILOPSENDJSON));
                }         
            }
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
            printf("====????%s\n",q.data);
            //printf("after dequeue print,the element=========%c %c %c\n",q.data[q.front],q.data[q.front+1],q.data[q.front+2]);
            return true;
        }else{
            return false;
        }
    }

}



//uint32_t uart_rec(){
//	uint32_t flag;
//	//??????
////	if(uart_flag){
//		printf("result:%s",USART_RX_BUF_ADDPRE);
//		if(strcmp((char*)USART_RX_BUF,"OK\r\n")){
//			flag = 1;
//		}else if(strcmp((char*)USART_RX_BUF,"+WJAP:%100[^\r]\r\n")){
//			flag = 2;
//			WJAP_BUF = USART_RX_BUF;
//		}else if(strcmp((char*)USART_RX_BUF,"+ILOPSTATUS:%20[^\r]\r\n")){
//			flag = 3;
//			ILOPSTATUS_BUF = USART_RX_BUF;
//		}
//		USART_RX_NUM=0;
//		uart_flag = false;
//		memset(USART_RX_BUF,0,sizeof(USART_RX_BUF));
//		memset(USART_RX_BUF,0,sizeof(USART_RX_BUF_ADDPRE));
////	}
//		return flag;
//}

