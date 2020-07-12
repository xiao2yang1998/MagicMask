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


char str_AT[][100]={

    "AT\r",
    "AT+UARTE=OFF\r",
    "AT+ILOPSET=a11407CgHIo,zHgpYHHnS4aWgdWu,4n5TLRxgmkmYXtnx1pmqMt68LJrTh6X7,YingJiaHao\r",
    "AT+ILOPDOMAIN=0\r",
    "AT+ILOPSTART\r",
    "AT+ILOPAWSSTART\r",
		"AT+ILOPSTATUS",
wo		"AT+FWVER?"
};
static char str_ILOPSENDJSON[200] = "AT+ILOPSENDJSON=property,";
static char tmp_str[4]; /*use to save the length of str which is a jsonString repoting to the Web*/

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
    /////////////////////

		 printf("start\n");
    
     printf("=========log=======\n");
		 while(!judgeReceiveEnd());
    
		while(1){

			// method 1
			char * model_id = "{\"model\":\"4028c6526f0e53b80170edd30db10003\"}";
			sprintf(tmp_str,"%d",strlen(model_id));   //tmp_str is use to save the length of str which is a jsonString repoting to the Web*/
			printf("tmp_str========%s\n",tmp_str);
			strcat(str_ILOPSENDJSON,tmp_str);
			strcat(str_ILOPSENDJSON,"\r");
			strcat(str_ILOPSENDJSON,model_id);
			printf("str_ILOPSENDJSON=====%s\n",str_ILOPSENDJSON);
			Send_Response((char*)str_ILOPSENDJSON,strlen(str_ILOPSENDJSON));
			sprintf(str_ILOPSENDJSON,"%s","AT+ILOPSENDJSON=property,");
			//AT+ILOPSENDJSON=property,17\r{"PauseSwitch":1}
//			Send_Response("AT+ILOPSENDJSON=property,17\r{\"PauseSwitch\":1}",strlen("AT+ILOPSENDJSON=property,17\r{\"PauseSwitch\":1}"));
//			delay_ms(1000);
		
			// method 2
//			Send_Response("AT+ILOPSENDJSON=property,44\r{\"model\":\"4028c6526f0e53b80170edd30db10003\"}",strlen("AT+ILOPSENDJSON=property,44\r{\"model\":\"4028c6526f0e53b80170edd30db10003\"}"));
//			delay_ms(1000);

			delay_ms(2000);
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
