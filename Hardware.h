#ifndef Hardware_h
#define Hardware_h


#ifndef ESP32  //ESP32
//STM32 *******************************
#include "src/QuadDecoder/QuadDecoder.h"
#include "src/Keypad/Keypad.h"
#include "src/StepperMotor/StepperMotor.h"

//Class instance ******************************************
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0,/* reset=*/ U8X8_PIN_NONE); //Screen -->external reset (boot problem)
QuadDecoder Quad_Y(3,QuadDecoder::LinearEncoder,512,false,false,IT_Timer3_Overflow); //Quad Y with timer 3
QuadDecoder Quad_Z(2,QuadDecoder::RotaryEncoder,1200,true,false,IT_Timer2_Overflow); //Quad Z with timer 2
QuadDecoder Quad_X(1,QuadDecoder::LinearEncoder,512,false,false,IT_Timer1_Overflow); //Quad X with timer 1
HardwareTimer MotorControl(4);  //for motor control with timer 4
StepperMotor Motor1(800,false,PIN_MOT1_STEP,PIN_MOT1_DIR,PIN_MOT1_EN,Update_Overlfow_Timer4);// Motor 1

//Interrupt handler functions ******************************************
void IT_Timer1_Overflow(){Quad_X.IT_OverflowHardwareTimer();}
void IT_Timer3_Overflow(){Quad_Y.IT_OverflowHardwareTimer();}
void IT_Timer2_Overflow()
{
  Quad_Z.IT_OverflowHardwareTimer();
  if(eMS_Thread == MS_THREAD_WAIT_THE_SPLINDLE_ZERO)
  {
    Quad_Z.ResetAllTheCounter();
    eMS_Thread = MS_THREAD_IN_THREAD;   
  }  
}
//Timer 4 overflow for Step motor
void handler_Timer4_overflow()
{ 
  if(eMS_Thread == MS_THREAD_IN_THREAD)
  {
    Motor1.ChangeTargetPositionStep ((Quad_Z.GetValueLong()*sThreadCalc.Numerator ) / sThreadCalc.Denominator + sThreadCalc.Offset );
  }  
  Motor1.TimeToPrepareToMove();  
}
//Timer 4 change overflow value in µs
void Update_Overlfow_Timer4()
{
  MotorControl.setOverflow(Motor1.NewInterval());  
}

//Timer 4 channel 3 compare interrupt (10µs after overflow)
void handler_Timer4_compare3()
{
Motor1.TimeToMove();
}


//STM32 *******************************
#else  //ESP32

//ESP32 *******************************

#define JF_ENGINE 
#define JF_ENGINE2
#define JF_ENGINE1


#ifndef voidFuncPtr
typedef void (*voidFuncPtr)(void);
#endif

#ifndef EEPROM_OK
#define EEPROM_OK 0
#endif


#include "src/ESP32_Encoder/QuadDecoder.h"
//#include "src/ESP32_Timer/ESP32Timer.h" // doublon avec LIB ESP32Timer
#include "src/Keypad_TM1638/Keypad.h"
#include "ESP32TimerInterrupt.h"
#include "src/StepperMotor/StepperMotor.h"




#include "FastAccelStepper.h"

struct stepper_config_s {
  uint8_t step;
  uint8_t enable_low_active;
  uint8_t enable_high_active;
  uint8_t direction;
  bool direction_high_count_up;
  bool auto_enable;
  uint32_t on_delay_us;
  uint16_t off_delay_ms;
};

//const uint8_t led_pin = 13;  // turn off with PIN_UNDEFINED
const uint8_t led_pin = PIN_UNDEFINED;  // turn off with PIN_UNDEFINED


const struct stepper_config_s stepper_config[MAX_STEPPER] = {
    {
      step : 4,
      enable_low_active : 25,
      enable_high_active : PIN_UNDEFINED,
      direction : 32,
      direction_high_count_up : true,
      auto_enable : true,
      on_delay_us : 5000,
      off_delay_ms : 10
    },  
    {
      step : 17,
      enable_low_active : 27,
      enable_high_active : PIN_UNDEFINED,
      direction : 16,
      direction_high_count_up : true,
      auto_enable : true,
      on_delay_us : 5000,
      off_delay_ms : 10
    },
  //  {step : PIN_UNDEFINED},  // unused stepper slot
    {step : PIN_UNDEFINED},  // unused stepper slot
    {step : PIN_UNDEFINED},  // unused stepper slot
    {step : PIN_UNDEFINED},  // unused stepper slot
    {step : PIN_UNDEFINED},  // unused stepper slot
};

#ifndef Fstepper
FastAccelStepperEngine engine = FastAccelStepperEngine();
 FastAccelStepper *Fstepper[MAX_STEPPER];
#endif


void info( FastAccelStepper *s) {
  Serial.print(s->isrSpeedControlEnabled() ? "AUTO" : "MANU");
  Serial.print(" Curr=");
  Serial.print(s->getCurrentPosition());
  Serial.print(" QueueEnd=");
  Serial.print(s->getPositionAfterCommandsCompleted());
  Serial.print(" Target=");
  Serial.print(s->targetPos());
  if (s->isRunning()) {
    Serial.print(" RUN ");
  } else {
    Serial.print(" STOP ");
  }
  Serial.print(" state=");
  switch (s->rampState() & RAMP_MOVE_MASK) {
    case RAMP_MOVE_UP:
      Serial.print("+");
      break;
    case RAMP_MOVE_DOWN:
      Serial.print("-");
      break;
    case 0:
      Serial.print("=");
      break;
    default:
      Serial.print("ERR");
      break;
  }
  switch (s->rampState() & RAMP_STATE_MASK) {
    case RAMP_STATE_IDLE:
      Serial.print("IDLE ");
      break;
    case RAMP_STATE_ACCELERATE:
      Serial.print("ACC  ");
      break;
    case RAMP_STATE_DECELERATE_TO_STOP:
      Serial.print("DEC ");
      break;
    case RAMP_STATE_DECELERATE:
      Serial.print("RED  ");  // Reduce
      break;
    case RAMP_STATE_COAST:
      Serial.print("COAST ");
      break;
    default:
      Serial.print(s->rampState());
  }
#if (TEST_MEASURE_ISR_SINGLE_FILL == 1)
  Serial.print(" max/us=");
  Serial.print(s->max_micros);
#endif
#if (TEST_CREATE_QUEUE_CHECKSUM == 1)
  Serial.print(" checksum=");
  Serial.print(s->checksum());
#endif
  Serial.print(" ");
}


void output_info() {
  for (uint8_t i = 0; i < MAX_STEPPER; i++) {
    if (Fstepper[i]) {
      Serial.print("M");
      Serial.print(i + 1);
      Serial.print(": ");
      info(Fstepper[i]);
    }
  }
  Serial.println();
}



//Class instance ******************************************
//ecran OLED JF pour tests
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, SCL, SDA, U8X8_PIN_NONE);   // All Boards without Reset of the Display
int top1,top2,top3,top4;
//QuadDecoder Quad_Y(3,QuadDecoder::f_LinearEncoder,512,false,false,&top3); //Timer 3 Encoder 4
QuadDecoder Quad_Y(3,QuadDecoder::h_LinearEncoder,512,false,false,&top3); //Timer 3 Counter /Encoder 2
QuadDecoder Quad_C(2,QuadDecoder::f_RotaryEncoder,512,true,false,&top2); //Timer 2  Encoder 4
//QuadDecoder Quad_X(1,QuadDecoder::f_LinearEncoder,512,false,false,&top1); //Timer 1 Encoder 4
QuadDecoder Quad_X(1,QuadDecoder::h_LinearEncoder,512,false,false,&top1); //Timer 1 Counter /Encoder 2




///**********Compatibilite avant modif motor stepper
//Timer 4 change overflow value in µs
void Update_Overlfow_Timer4()
{
//  MotorControl.setOverflow(Motor1.NewInterval());  
}

StepperMotor Motor1(800,false,PIN_MOT1_STEP,PIN_MOT1_DIR,PIN_MOT1_EN,Update_Overlfow_Timer4);// Motor 1
 //info(Fstepper[i]);
//StepperMotor Motor1(800,false,PIN_MOT1_STEP,PIN_MOT1_DIR,PIN_MOT1_EN,Fstepper[i]);// Motor 1


//ESP32 *******************************
#endif  //ESP32 

void SetUp_Hardware();

void SetUp_Hardware(){
#ifndef ESP32  //ESP32
//STM32 *******************************
  afio_cfg_debug_ports(AFIO_DEBUG_SW_ONLY); //Only SWD

//Timer 4 for motor control
  // Interval in microsecs
   // Interval in microsecs
  MotorControl.pause(); //stop...
  MotorControl.setCompare(TIMER_CH3, 20); //10µs 
  MotorControl.setChannel3Mode(TIMER_OUTPUT_COMPARE);
  MotorControl.setPrescaleFactor(72); // 72Mhz, 1 = 1µs
  MotorControl.setOverflow(100); // default value 100µs overflow
  //MotorControl.setPeriod(100); //Period 100µs --> 10Khz
  MotorControl.attachCompare3Interrupt(handler_Timer4_compare3); //interrupt conmpare 3
  MotorControl.attachInterrupt(0, handler_Timer4_overflow); //Overflow interrupt  
  MotorControl.resume();

//STM32 *******************************
#else  //ESP32
//ESP32 *******************************

if (!EEPROM.begin(sizeof(tsConfigDro)))
  {
    Serial.println("failed to initialise EEPROM");//JF
  // delay(1000000);//JF
  }


Serial.println(" engine.init");
  engine.init();
  if (led_pin != PIN_UNDEFINED) {
    engine.setDebugLed(led_pin);
  }

 Serial.println(" FastAccelStepper init");

#ifdef JF_ENGINE1
 //import TEST*******************
  for (uint8_t i = 0; i < MAX_STEPPER; i++) {
    FastAccelStepper *s = NULL;
    const struct stepper_config_s *config = &stepper_config[i];
    if (config->step != PIN_UNDEFINED) {
      s = engine.stepperConnectToPin(config->step);
      if (s) {
        s->setDirectionPin(config->direction, config->direction_high_count_up);
        s->setEnablePin(config->enable_low_active, true);
        s->setEnablePin(config->enable_high_active, false);
        s->setAutoEnable(config->auto_enable);
        s->setDelayToEnable(config->on_delay_us);
        s->setDelayToDisable(config->off_delay_ms);
      }
    }
    Fstepper[i] = s;
  }
#endif

#ifdef JF_ENGINE1
 Serial.println(" Fstepper_M1 init");
 
 Serial.println(" setAcceleration 200000");
 Fstepper[0]->setAcceleration(200000);     // A
 Serial.println(" setSpeed 10");
 Fstepper[0]->setSpeed(10);          // V
 Serial.println(" setAutoEnable");
 Fstepper[0]->setAutoEnable(true);      //O
#endif
 
#ifdef JF_ENGINE2
 Serial.println(" Fstepper_M2 init");
 
 Serial.println(" setAcceleration 200000");
 Fstepper[1]->setAcceleration(200000);     // A
 Serial.println(" setSpeed 10");
 Fstepper[1]->setSpeed(10);          // V
 Serial.println(" setAutoEnable");
 Fstepper[1]->setAutoEnable(true);      //O
#endif
 
 #ifdef JF_ENGINE 
   //import from TEST*******************
 
 #endif JF_ENGINE 

 Motor1.SetFastAccelStepper(Fstepper[0]);
//ESP32 *******************************
#endif  //ESP32   
}

// ***************************************************************************************
// ***************************************************************************************
// *** Save / Restore config *************************************************************

void SaveConfigInFlash(tsConfigDro *pConf)
{
  uint16_t uiCount;
  char *pt;
  
#ifndef ESP32
 EEPROM.format();
#endif 

  pt = (char*)pConf; 
  for(uiCount=0;uiCount<sizeof(tsConfigDro);uiCount++)
  {
#ifndef ESP32
    EEPROM.write(uiCount,*pt);
#else
     EEPROM.writeChar(uiCount,*pt);
#endif
    pt++; 
  } 
#ifdef ESP32  
 EEPROM.commit(); 
#endif
}

void ReadConfigInFlash(tsConfigDro *pConf)
{
  uint16_t uiCount;
  uint16_t uiState;
  uint16_t value;
  char *pt;
  uiState = EEPROM_OK;
  pt = (char*)pConf; 
  for(uiCount=0;uiCount<sizeof(tsConfigDro);uiCount++)
  {
#ifndef ESP32
    uiState |= EEPROM.read(uiCount,&value);
    *pt = (char) value;
#else
    *pt = EEPROM.readChar(uiCount);
#endif
    pt++;  
  if(uiState != EEPROM_OK)
  {
    //Problem, restore default  
    *pConf = csConfigDefault;
  } 
 }
}




#endif  //#ifndef Hardware_h
