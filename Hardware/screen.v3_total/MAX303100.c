/*
MIT License

Copyright (c) 2017 Raivis Strogonovs (https://morf.lv)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.


*/

#include "MAX303100.h"

static bool debug = true;

static uint8_t redLEDCurrent;
static float lastREDLedCurrentCheck;

static uint8_t currentPulseDetectorState;
static float currentBPM;
static float valuesBPM[PULSE_BPM_SAMPLE_SIZE];
static float valuesBPMSum;
static uint8_t valuesBPMCount;
static uint8_t bpmIndex;
static uint32_t lastBeatThreshold;

static struct fifo_t prevFifo;

static struct dcFilter_t dcFilterIR;
static struct dcFilter_t dcFilterRed;
static struct butterworthFilter_t lpbFilterIR;
static struct meanDiffFilter_t meanDiffIR;

static float irACValueSqSum;
static float redACValueSqSum;
static uint16_t samplesRecorded;
static uint16_t pulsesDetected;
static float currentSaO2Value;

LEDCurrent IrLedCurrent;




static bool max30100_detectPulse(float sensor_value)
{ 
	//printf("sensor_value:%f\n",sensor_value);
  static float prev_sensor_value = 0;
  static uint8_t values_went_down = 0;
  static uint32_t currentBeat = 0;
  static uint32_t lastBeat = 0;

  if(sensor_value > PULSE_MAX_THRESHOLD)
  {
    currentPulseDetectorState = PULSE_IDLE;
    prev_sensor_value = 0;
    lastBeat = 0;
    currentBeat = 0;
    values_went_down = 0;
    lastBeatThreshold = 0;
    return false;
  }
  //printf("-----currentPulseDetectorState:%d\n",currentPulseDetectorState);
  switch(currentPulseDetectorState)
  {
    case PULSE_IDLE:
      //printf("PULSE_IDLE\n");
      if(sensor_value >= PULSE_MIN_THRESHOLD) {
        currentPulseDetectorState = PULSE_TRACE_UP;
        values_went_down = 0;
      }
      break;

    case PULSE_TRACE_UP:
      //printf("PULSE_TRACE_UP\n");
      if(sensor_value > prev_sensor_value)
      {
        currentBeat = ms_ticker_read();

        lastBeatThreshold = sensor_value;
      }
      else
      {

        if(debug == true) 
        {
          printf("Peak reached: ");
          printf("%f",sensor_value);
          printf(" ");
          printf("%f\n",prev_sensor_value);
        }

        uint32_t beatDuration = currentBeat - lastBeat;
        lastBeat = currentBeat;
        //printf("!!!!!!!!!!   currentBeat %d lastBeat:%d\n ",currentBeat,lastBeat);

        float rawBPM = 0;
        if(beatDuration > 0)
          rawBPM = 60000.0 / (float)beatDuration;
        if(debug == true) 
          printf("%f\n",rawBPM);

        //This method sometimes glitches, it's better to go through whole moving average everytime
        //IT's a neat idea to optimize the amount of work for moving avg. but while placing, removing finger it can screw up
        //valuesBPMSum -= valuesBPM[bpmIndex];
        //valuesBPM[bpmIndex] = rawBPM;
        //valuesBPMSum += valuesBPM[bpmIndex];

        valuesBPM[bpmIndex] = rawBPM;
        valuesBPMSum = 0;
        for(int i=0; i<PULSE_BPM_SAMPLE_SIZE; i++)
        {
          valuesBPMSum += valuesBPM[i];
        }

        if(debug == true) 
        {
          printf("CurrentMoving Avg: ");
          for(int i=0; i<PULSE_BPM_SAMPLE_SIZE; i++)
          {
            printf("%f",valuesBPM[i]);
            printf(" ");
          }
  
          printf(" \n");
        }

        bpmIndex++;
        bpmIndex = bpmIndex % PULSE_BPM_SAMPLE_SIZE;

        if(valuesBPMCount < PULSE_BPM_SAMPLE_SIZE)
          valuesBPMCount++;

        currentBPM = valuesBPMSum / valuesBPMCount;
        if(debug == true) 
        {
          printf("AVg. BPM: ");
          printf("%f\n",currentBPM);
        }


        currentPulseDetectorState = PULSE_TRACE_DOWN;

        return true;
      }
      break;

    case PULSE_TRACE_DOWN:
      if(sensor_value < prev_sensor_value)
      {
        values_went_down++;
      }


      if(sensor_value < PULSE_MIN_THRESHOLD)
      {
        currentPulseDetectorState = PULSE_IDLE;
      }
      break;
  }

  prev_sensor_value = sensor_value;
  return false;
}

static void max30100_balanceIntesities( float redLedDC, float IRLedDC )
{   
  //printf("currentBeat %d lastREDLedCurrentCheck:%f\n ",ms_ticker_read(),lastREDLedCurrentCheck);
  if( (ms_ticker_read()) - lastREDLedCurrentCheck >= RED_LED_CURRENT_ADJUSTMENT_MS) 
  {
    //Serial.println( redLedDC - IRLedDC );
    if( IRLedDC - redLedDC > MAGIC_ACCEPTABLE_INTENSITY_DIFF && redLEDCurrent < MAX30100_LED_CURRENT_50MA) 
    {
      redLEDCurrent++;
      max30100_setLEDCurrents( redLEDCurrent, IrLedCurrent );
      if(debug == true) 
        printf("RED LED Current +\n");
    } 
    else if(redLedDC - IRLedDC > MAGIC_ACCEPTABLE_INTENSITY_DIFF && redLEDCurrent > 0) 
    {
      redLEDCurrent--;
      max30100_setLEDCurrents( redLEDCurrent, IrLedCurrent );
      if(debug == true) 
        printf("RED LED Current -\n");
    }
    lastREDLedCurrentCheck = ms_ticker_read();
  }
}


// Writes val to address register on device
static void max30100_writeRegister(uint8_t address, uint8_t val)
{
    writeI2CwithReg(MAX30100_DEVICE,address,&val,1);
}

static uint8_t max30100_readRegister(uint8_t address)
{
  uint8_t Data;
  readI2CwithReg(MAX30100_DEVICE,address,&Data,1);
  return Data;
}

// Reads num uint8_ts starting from address register on device in to _buff array
static void max30100_readFrom(uint8_t address, int num, uint8_t _buff[])
{
    readBurstI2C(MAX30100_DEVICE,address,_buff,num);
}

void static max30100_resetFifo(void){
	max30100_writeRegister(MAX30100_FIFO_WRITE,0);
	max30100_writeRegister(MAX30100_FIFO_OVERFLOW_COUNTER,0);
	max30100_writeRegister(MAX30100_FIFO_READ,0);
	
	
	uint8_t write = max30100_readRegister(MAX30100_FIFO_WRITE);
	uint8_t read = max30100_readRegister(MAX30100_FIFO_READ);
	//printf("?????????? after reset write %x read %x num %x\n",write,read,write-read);
}




void max30100_init(
    Mode p_mode, 
    SamplingRate p_samplingRate, 
    LEDPulseWidth p_pulseWidth, 
    LEDCurrent p_IrLedCurrent,
    bool p_highResMode,
    bool p_debug){
        debug = p_debug;
        currentPulseDetectorState = PULSE_IDLE;
        max30100_setMode(p_mode);

        //Check table 8 in datasheet on page 19. You can't just throw in sample rate and pulse width randomly. 100hz + 1600us is max for that resolution
        max30100_setSamplingRate( p_samplingRate );
        max30100_setLEDPulseWidth( p_pulseWidth );
        redLEDCurrent = (uint8_t)STARTING_RED_LED_CURRENT;
        lastREDLedCurrentCheck = 0;
        IrLedCurrent = p_IrLedCurrent;
        max30100_setHighresModeEnabled(p_highResMode);
        max30100_setLEDCurrents( redLEDCurrent, IrLedCurrent );
        dcFilterIR.w = 0;
        dcFilterIR.result = 0;

        dcFilterRed.w = 0;
        dcFilterRed.result = 0;


        lpbFilterIR.v[0] = 0;
        lpbFilterIR.v[1] = 0;
        lpbFilterIR.result = 0;

        meanDiffIR.index = 0;
        meanDiffIR.sum = 0;
        meanDiffIR.count = 0;


        valuesBPM[0] = 0;
        valuesBPMSum = 0;
        valuesBPMCount = 0;
        bpmIndex = 0;
        

        irACValueSqSum = 0;
        redACValueSqSum = 0;
        samplesRecorded = 0;
        pulsesDetected = 0;
        currentSaO2Value = 0;

        lastBeatThreshold = 0;
}


void max30100_setup(void){
    /* Initialize I2C */
    initI2C();
    max30100_init(DEFAULT_OPERATING_MODE, DEFAULT_SAMPLING_RATE, DEFAULT_LED_PULSE_WIDTH, DEFAULT_IR_LED_CURRENT, true, true );
	
		max30100_resetFifo();
		//printf("ID %x\n",max30100_readRegister(MAX30100_PART_ID));

}


void print_pulseoxymeter_t(struct pulseoxymeter_t p){
  printf("pulseDetected %d\n",p.pulseDetected);
  printf("heartBPM %f\n",p.heartBPM);

  printf("irCardiogram %f\n",p.irCardiogram);

  printf("irDcValue %f\n",p.irDcValue);
  printf("redDcValue %f\n",p.redDcValue);

  printf("SaO2 %f\n",p.SaO2);

  printf("lastBeatThreshold %d\n",p.lastBeatThreshold);

  printf("dcFilteredIR %f\n", p.dcFilteredIR);
  printf("dcFilteredRed %f\n", p.dcFilteredRed); 

}
struct pulseoxymeter_t max30100_update(void)
{
	//printf("MAX30100_INT_STATUS %x\n",max30100_readRegister(MAX30100_INT_STATUS));
  //printf("MAX30100_INT_ENABLE %x\n",max30100_readRegister(MAX30100_INT_ENABLE));
  
  struct pulseoxymeter_t result = {
    /*bool pulseDetected*/ false,
    /*float heartBPM*/ 0.0,
    /*float irCardiogram*/ 0.0,
    /*float irDcValue*/ 0.0,
    /*float redDcValue*/ 0.0,
    /*float SaO2*/ currentSaO2Value,
    /*uint32_t lastBeatThreshold*/ 0,
    /*float dcFilteredIR*/ 0.0,
    /*float dcFilteredRed*/ 0.0
  };

  
  struct fifo_t  rawData = max30100_readFIFO();  
  //printf("MAX30100_INT_STATUS %x\n",max30100_readRegister(MAX30100_INT_STATUS));

  dcFilterIR = max30100_dcRemoval( (float)rawData.rawIR, dcFilterIR.w, ALPHA );
  
  dcFilterRed = max30100_dcRemoval( (float)rawData.rawRed, dcFilterRed.w, ALPHA );

  float meanDiffResIR = max30100_meanDiff( dcFilterIR.result, &meanDiffIR);
  max30100_lowPassButterworthFilter( meanDiffResIR/*-dcFilterIR.result*/, &lpbFilterIR );

  irACValueSqSum += dcFilterIR.result * dcFilterIR.result;
  redACValueSqSum += dcFilterRed.result * dcFilterRed.result;
  samplesRecorded++;

  if( max30100_detectPulse( lpbFilterIR.result ) && samplesRecorded > 0 )
  {
		
    result.pulseDetected=true;
    pulsesDetected++;

    float ratioRMS = log( sqrt(redACValueSqSum/samplesRecorded) ) / log( sqrt(irACValueSqSum/samplesRecorded) );

    if( debug == true )
    {
      printf("RMS Ratio: ");
      printf("%f\n",ratioRMS);
    }

    //This is my adjusted standard model, so it shows 0.89 as 94% saturation. It is probably far from correct, requires proper empircal calibration
    currentSaO2Value = 110.0 - 18.0 * ratioRMS;
    result.SaO2 = currentSaO2Value;
    
    if( pulsesDetected % RESET_SPO2_EVERY_N_PULSES == 0)
    {
      irACValueSqSum = 0;
      redACValueSqSum = 0;
      samplesRecorded = 0;
    }
  }

  max30100_balanceIntesities( dcFilterRed.w, dcFilterIR.w );


  result.heartBPM = currentBPM;
  result.irCardiogram = lpbFilterIR.result;
  result.irDcValue = dcFilterIR.w;
  result.redDcValue = dcFilterRed.w;
  result.lastBeatThreshold = lastBeatThreshold;
  result.dcFilteredIR = dcFilterIR.result;
  result.dcFilteredRed = dcFilterRed.result;  
  
  //print_pulseoxymeter_t(result);
  
  return result;
}



void max30100_setMode(Mode mode)
{ 
  // printf("--------start set mode\n");
  uint8_t currentModeReg = max30100_readRegister( MAX30100_MODE_CONF );
  max30100_writeRegister( MAX30100_MODE_CONF, (currentModeReg & 0xF8) | mode );
}

void max30100_setHighresModeEnabled(bool enabled)
{  
  //  printf("--------start set high res mode enables\n");
    uint8_t previous = max30100_readRegister(MAX30100_SPO2_CONF);
    if (enabled) {
        max30100_writeRegister(MAX30100_SPO2_CONF, previous | MAX30100_SPO2_HI_RES_EN);
    } else {
        max30100_writeRegister(MAX30100_SPO2_CONF, previous & ~MAX30100_SPO2_HI_RES_EN);
    }
}

void max30100_setSamplingRate(SamplingRate rate)
{
  //  printf("--------start set sampling rate\n");
  uint8_t currentSpO2Reg = max30100_readRegister( MAX30100_SPO2_CONF );
  max30100_writeRegister( MAX30100_SPO2_CONF, ( currentSpO2Reg & 0xE3 ) | (rate<<2) );
}

void max30100_setLEDPulseWidth(LEDPulseWidth pw)
{ 
  // printf("--------start set pulse width\n");
  uint8_t currentSpO2Reg = max30100_readRegister( MAX30100_SPO2_CONF );
  max30100_writeRegister( MAX30100_SPO2_CONF, ( currentSpO2Reg & 0xFC ) | pw );
}

void max30100_setLEDCurrents( uint8_t redLedCurrent, uint8_t IRLedCurrent )
{
  // printf("--------start set led current\n");
  max30100_writeRegister( MAX30100_LED_CONF, (redLedCurrent << 4) | IRLedCurrent );
}

float max30100_readTemperature(void)
{
  uint8_t currentModeReg = max30100_readRegister( MAX30100_MODE_CONF );
  max30100_writeRegister( MAX30100_MODE_CONF, currentModeReg | MAX30100_MODE_TEMP_EN );

  delay_ms(100); //This can be changed to a while loop, there is an interrupt flag for when temperature has been read.

  int8_t temp = (int8_t)max30100_readRegister( MAX30100_TEMP_INT );
  float tempFraction = (float)max30100_readRegister( MAX30100_TEMP_FRACTION ) * 0.0625;

  return (float)temp + tempFraction;
}

struct fifo_t max30100_readFIFO(void)
{
 
	//printf("MAX30100_FIFO_READ %x\n",max30100_readRegister(MAX30100_FIFO_READ));
//  printf("0 MAX30100_FIFO_DATA %x\n",max30100_readRegister(MAX30100_FIFO_DATA));
//	printf("MAX30100_FIFO_READ %x\n",max30100_readRegister(MAX30100_FIFO_READ));
//  printf("1 MAX30100_FIFO_DATA %x\n",max30100_readRegister(MAX30100_FIFO_DATA));
//	printf("MAX30100_FIFO_READ %x\n",max30100_readRegister(MAX30100_FIFO_READ));
//  printf("2 MAX30100_FIFO_DATA %x\n",max30100_readRegister(MAX30100_FIFO_DATA));
//	printf("MAX30100_FIFO_READ %x\n",max30100_readRegister(MAX30100_FIFO_READ));
//  printf("3 MAX30100_FIFO_DATA %x\n",max30100_readRegister(MAX30100_FIFO_DATA));
//	printf("MAX30100_FIFO_READ %x\n",max30100_readRegister(MAX30100_FIFO_READ));
	struct fifo_t result;
  uint8_t buffer[4] = {1,1,1,1,};
	uint8_t write = max30100_readRegister(MAX30100_FIFO_WRITE);
	uint8_t read = max30100_readRegister(MAX30100_FIFO_READ);

		max30100_readFrom( MAX30100_FIFO_DATA, 4, buffer );
		result.rawIR = (buffer[0] << 8) | buffer[1];
		result.rawRed = (buffer[2] << 8) | buffer[3];
		
		uint8_t writet = max30100_readRegister(MAX30100_FIFO_WRITE);
		uint8_t readt = max30100_readRegister(MAX30100_FIFO_READ);

	return result;
}

struct dcFilter_t max30100_dcRemoval(float x, float prev_w, float alpha)
{
  struct dcFilter_t filtered;
  filtered.w = x + alpha * prev_w;
  filtered.result = filtered.w - prev_w;

  return filtered;
}

void max30100_lowPassButterworthFilter( float x, struct butterworthFilter_t * filterResult )
{  
  filterResult->v[0] = filterResult->v[1];

  //Fs = 100Hz and Fc = 10Hz
  filterResult->v[1] = (2.452372752527856026e-1 * x) + (0.50952544949442879485 * filterResult->v[0]);

  //Fs = 100Hz and Fc = 4Hz
  //filterResult->v[1] = (1.367287359973195227e-1 * x) + (0.72654252800536101020 * filterResult->v[0]); //Very precise butterworth filter 

  filterResult->result = filterResult->v[0] + filterResult->v[1];
}

float max30100_meanDiff(float M, struct meanDiffFilter_t* filterValues)
{
  float avg = 0;

  filterValues->sum -= filterValues->values[filterValues->index];
  filterValues->values[filterValues->index] = M;
  filterValues->sum += filterValues->values[filterValues->index];

  filterValues->index++;
  filterValues->index = filterValues->index % MEAN_FILTER_SIZE;

  if(filterValues->count < MEAN_FILTER_SIZE)
    filterValues->count++;

  avg = filterValues->sum / filterValues->count;
  return avg - M;
}

void max30100_printRegisters(void)
{
  printf("MAX30100_INT_STATUS %x\n",max30100_readRegister(MAX30100_INT_STATUS));
  printf("MAX30100_INT_ENABLE %x\n",max30100_readRegister(MAX30100_INT_ENABLE));
  printf("MAX30100_FIFO_WRITE %x\n",max30100_readRegister(MAX30100_FIFO_WRITE));
  printf("MAX30100_FIFO_OVERFLOW_COUNTER %x\n",max30100_readRegister(MAX30100_FIFO_OVERFLOW_COUNTER));
  printf("MAX30100_FIFO_READ %x\n",max30100_readRegister(MAX30100_FIFO_READ));
  printf("MAX30100_FIFO_DATA %x\n",max30100_readRegister(MAX30100_FIFO_DATA));

  printf("%x\n",max30100_readRegister(MAX30100_MODE_CONF));
  printf("%x\n",max30100_readRegister(MAX30100_SPO2_CONF));
  printf("%x\n",max30100_readRegister(MAX30100_LED_CONF));
  printf("%x\n",max30100_readRegister(MAX30100_TEMP_INT));
  printf("%x\n",max30100_readRegister(MAX30100_TEMP_FRACTION));
  printf("%x\n",max30100_readRegister(MAX30100_REV_ID));
  printf("%x\n",max30100_readRegister(MAX30100_PART_ID));
}

