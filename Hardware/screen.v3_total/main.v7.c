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

    printf("start\n");


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
	printf("start\n");
    

     printf("=========log=======\n");
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

void sleep(int uint32_ti){
		delay_ms(uint32_ti);
}


bool judgeReceiveEnd()
{

}
