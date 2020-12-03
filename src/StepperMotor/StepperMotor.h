#ifndef StepperMotor_h
#define StepperMotor_h   
#include <Arduino.h>

/**
 *  \file StepperMotor.h
 *  \brief Stepper motor class
 *  \author G.pailleret
 *  \version 1.0
 *  \date 22/10/2019
 */

#ifdef ESP32
#include "ESP32TimerInterrupt.h"
#include "FastAccelStepper.h"

#ifndef voidFuncPtr
typedef void (*voidFuncPtr)(void);
#endif
#ifndef voidFuncPtr
typedef void (*F_StepperFuncPtr)(void);
#endif


extern volatile uint16_t _M1_StepInterval ;   //Step interval in µs
#endif
 
class StepperMotor
{
public:
	StepperMotor(uint16_t  Resolution , boolean Sens, char STEP_Pin, char DIR_Pin, char EN_Pin,voidFuncPtr ChangeStepInterval);
	//Change the motor parameter
  void ChangeParameter(uint16_t  Resolution , boolean Sens);
  
	void 		TimeToPrepareToMove (); //Be call every 100µs 
	void 		TimeToMove (); 					//Be call every 10µs after TimeToPrepareToMove() 
	uint16_t NewInterval ();    //New interval in µs
  

	enum teMotorMode 
	{   
      NoMode,
      SpeedModeUp,
      SpeedModeDown,
      PositionMode
  };

void SetFastAccelStepper (FastAccelStepper *F_);
int F_status;
  //Change the actual mode
  void    ChangeTheMode(teMotorMode eMode);
  //Return the actual mode
  teMotorMode ReturnTheMode();
  
   //Change the On/off state  ( false = motor disabled no power)
  void    MotorChangePowerState ( boolean State);  

  //Change the target position in "PositionMode"
  void 	JFChangeTargetPositionStep (int32_t Target_Position);
  void  JFChangeTargetPositionReal (float Target_Position);

  //Change the Current motor position
  void 	JFChangeCurrentPositionStep (int32_t Position);
  void  ChangeCurrentPositionReal (float Position);

  
  
  //Electronic end limit min and max functions 
  void    UseEndLimit ( boolean State );
	void 		ChangeStopPositionMaxStep (int32_t Stop_Position);
  void    ChangeStopPositionMaxReal (float Stop_Position);  
	void 		ChangeStopPositionMinStep (int32_t Stop_Position);  
  void    ChangeStopPositionMinReal (float Stop_Position);
	boolean AreYouAtMaxPos();
	boolean AreYouAtMinPos(); 
  int32_t    GetStopPositionMinStep();
  int32_t    GetStopPositionMaxStep();

  //Change the max speed in step/s
	void 		     ChangeMaxSpeed (float MaxSpeed);
  //Get the max speed in step/s
  float GetMaxSpeed ();
  
  //Change the Acceleration in step/s²
	void 		     ChangeAcceleration (float Acceleration);
  //Get the max speed in step/s
  float GetAcceleration ();  
  
  

  //Get the absolute position of the motor
	int32_t 		 GetPositionStep();
  float    GetPositionReal(); // mm,turn
  
  int32_t _n;                      //The step counter for speed calculations
//  float _c0;                    //Intial step size in µs
//  float _cmin;                  //Min step size in microseconds based on maxSpeed
 int32_t _c0;                    //Intial step size in µs
  int32_t _cmin;                  //Min step size in microseconds based on maxSpeed
private:
	enum eMS_Motor 
	{   
      State_Rotation_Positive,
      State_Rotation_Negative, 
      State_No_Rotation 
  };
    voidFuncPtr _ChangeStepInterval; 
	FastAccelStepper * F_Stepper;
	
	boolean   _UseEndLimit;       //If true, use end limit
    teMotorMode _eActualMode;     //Motor mode
	int32_t 			_AbsoluteCounter; 	//Absolute position in step
	int32_t 			_TargetPosition; 		//Target position in step
    int32_t      _ErrorPos;          //Error absolute - target
	int32_t 			_StopPositionMax; 	//End limit Max position in step
	int32_t 			_StopPositionMin;		//End limit Min position in step
	boolean 	_Sens; 							//Sens de rotation
  char      _PinDIR;            //IO for Dir
  char      _PinSTEP;           //IO for Step
  char      _PinEN;             //IO for Enable
	eMS_Motor eState; 						//Motor machine state
	uint16_t _Resolution; 		//Resolution/tr or mm 
  
 /*
 float _Speed;                 //Actual speed
  float _MaxSpeed;    			    //Max speed in step/s 
  float _Acceleration;          //Acceleration in step/s²
*/
  int32_t _Speed;                 //Actual speed
  int32_t _MaxSpeed;    			    //Max speed in step/s 
  int32_t _Acceleration;          //Acceleration in step/s²
  uint16_t _StepInterval;   //Step interval in µs
  
//  float _cn;                    //Last step size in µs
  int32_t  _cn; 
};
    
#endif
