
//MiniThread_ESP32 _DEV_MF
//version fichiers multiples



/*********************************************************************
Project Name    :   MiniThread
Hard revision   :   V1.0
Soft revision   :   1.1.0
Description     :   
Chip            :   STM32F103CBT6
freq uc         :   72Mhz (use 8Mhz external oscillator with PLL ) 
Chip            :   ESP32
freq uc         :   240Mhz
Compiler        :   Arduino IDE 1.8.13
Author          :   G.Pailleret, 2020 
Remark          :  
Revision        : JFJF

*********************************************************************/
 //#define JF_Snake

#include "src/Language/Language.h"
#ifdef JF_Snake
#include "src/SnakeGame/Snake.h"
#endif
#include "src/Msg/Msg.h"

#define TEXT_MAIN_MENU_TITLE "MiniThread 1.1.0"
#define TEXT_AUTHOR_SOFT "Pailpoe JF"
#define TEXT_VERSION_SOFT "1.0.0 Dev"

//ESP32 VERSION DEBUG TIMER motorcontrol1
//ESP32 VERSION FastAccelStepper OK 2 Moteurs 25/11/2020
//29/11/2020
// IO def ( for quad decoder, define in class !)
#include "src/GEM/GEM_u8g2.h"

#include <EEPROM.h>
#define EEPROM_SIZE 64


#include "Hardware_Pin.h"

typedef struct
{
  boolean Inverted_X;  
  boolean Inverted_Y;
  boolean Inverted_C;
  boolean Diameter_Mode_Y;
  int  Reso_X;
  int  Reso_Y;
  int  Reso_C;
  boolean Inverted_M1;
  int  Reso_M1;
  int  thread_M1;
  float Accel_M1;
  int Speed_M1;
  byte Lang;
  boolean UseUSBFunctions;
} tsConfigDro;
const tsConfigDro csConfigDefault = {false,false,false,true,512,512,1200,false,1600,200,60000.0,12000,LANG_FR,true};



#include "Hardware.h"  //utilise tsConfigDro


#include "src/StepperMotor/StepperMotor.h"
#include "src/Various/Splash.h"
#include "src/Various/Various.h"


//Keyboard
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
char hexaKeys[ROWS][COLS] = {
  {'Z'             ,'Z'           ,GEM_KEY_UP   ,'Z'            },
  {GEM_KEY_CANCEL  ,GEM_KEY_LEFT  ,GEM_KEY_OK   ,GEM_KEY_RIGHT  },
  {'Z'             ,'Z'           ,GEM_KEY_DOWN ,'Z'            },
  {'Z'             ,'Z'           ,'Z'          ,'Z'            }
};
byte rowPins[ROWS] = {PIN_SW_LIN_1, PIN_SW_LIN_2, PIN_SW_LIN_3, PIN_SW_LIN_4};
byte colPins[COLS] = {PIN_SW_COL_1, PIN_SW_COL_2, PIN_SW_COL_3, PIN_SW_COL_4};

//Struct and Enum def  ******************************************

//Threading machine state
typedef enum
{
  MS_THREAD_IDLE = 0, //Idle
  MS_THREAD_WAIT_THE_START = 1, //Wait the button to start
  MS_THREAD_WAIT_THE_SPLINDLE_ZERO = 2, // Wait spindle zero
  MS_THREAD_IN_THREAD = 3, // In threat
  MS_THREAD_END_THREAD = 4, // Wait the button to return
  MS_THREAD_IN_RETURN = 5 // In return
} teMS_ThreadingMode;

//Parameter for the thread
typedef struct
{
  int32_t Numerator;
  int32_t Denominator;
  int32_t Offset;  
} tsThreadCalc;

//Screen choose
#define  SCREEN_DRO 0
#define  SCREEN_MOT1 1
#define  SCREEN_DEBUG 2
#define  SCREEN_END_LIST 1

//Motor mode
#define MOTOR_MODE_NO_MODE  0
#define MOTOR_MODE_MANUAL   1
#define MOTOR_MODE_AUTO     2
#define MOTOR_MODE_TH_EXT_N 3
#define MOTOR_MODE_TH_EXT_I 4
#define MOTOR_MODE_TH_INT_N 5
#define MOTOR_MODE_TH_INT_I 6

// Variables global ******************************************
//deplacé avant "Hardware.h"
/*
const tsConfigDro csConfigDefault = {false,false,false,true,512,512,1200,false,1600,200,60000.0,12000,LANG_FR,true};
*/
tsConfigDro  sGeneralConf;
boolean     bSettingsNeedToBeSaved = false;
float       TestFloat = 999.2;
byte        bToolChoose = 0; //Tool selection
boolean     bRelativeModeActived = false; //Relative or absolute mode
float       fAxeXPos = 0; // X position
float       fAxeYPos = 0; // Y position
float       fAxeCSpeed = 0; // C speed
float       mem_fAxeXPos,mem_fAxeYPos,mem_fAxeCSpeed;

byte        bMotorMode = 0;
boolean     bUseMotor = false;
float       fMotorStopMin = 0.0;
float       fMotorStopMax = 200.0;
boolean     bUseMotorEndLimit = true;
float       fMotorCurrentPos = 0;
int         iMotorSpeed = 1000;
int         iMotorThread = 100;
float       fMotor1ThreadOffset = 0.0;
boolean     bMotor1ThreadUseY = false;
float       fMotor1ThreadDiameter = 0.0;
float       fMotor1ThreadAngle = 0.0;
float       fM1ActualSpeed; // Motor Actual Speed
float       fM1MaxThreadSpeed; // Motor Max spindle speed for thread
byte        eScreenChoose = SCREEN_DRO;
    
teMS_ThreadingMode  eMS_Thread = MS_THREAD_IDLE;
tsThreadCalc sThreadCalc; 
volatile uint16_t _M1_StepInterval =  500;   //Step interval in µs
volatile uint16_t _M2_StepInterval =  500;   //Step interval in µs
int16_t _T_Count_us[4];
volatile int8_t _Synch_Spindle = 0 ;

// Forward declarations Funtions  ******************************************
void CalcMotorParameterForThread(); 
void CalcMotorParameterOffsetForThread();
void CalcMotorMaxSpeedForThread();
void UsbSerial_Pos(); 
void Display_UpdateRealTimeData(); 
void ActionMotorSpeedUp();
void ActionMotorSpeedDown();
void Display_StartScreen(); 
void ActionDro(); 
void ActionDebug();
#ifdef JF_Snake
void ActionLaunchSnakeGame();
void SnakeContextEnter();
void SnakeContextLoop();
void SnakeContextExit(); 
#endif
void applyTool(); 
void NeedToSave();
void ActionUpdateMenuTitle();

void ActionShortcutsResetX();
void ActionShortcutsResetY();
void ActionShortcutsResetM1();
void ActionShortcutsSetCurrentToMax();
void ActionShortcutsSetCurrentToMin();
void ActionShortcutsM1inManual();
void ActionShortcutsM1inAuto();

void ActionChangeDirX();
void ActionChangeDirY();
void ActionChangeDirZ();
void ActionChangeDirM1();
void ActionChangeResoX();
void ActionChangeResoY();
void ActionChangeResoZ();
void ActionChangeResoM1();
void ActionChangeDiamY();
void ActionChangeThreadM1();
void ActionChangeAccelM1();
void ActionChangeSpeedM1();
void ActionChangeLang();
void ActionChangeUseUSB();
void ActionRestoreSettingsInFlash(); 
void ActionSaveSettingsInFlash(); 
void ActionChangeRelaticeMode();
void ActionResetX(); 
void ActionResetY(); 
void ActionAxeXPos(); 
void ActionAxeYPos(); 
void SetReadOnlyMotorFunctions(boolean state); // true = Read only
void ActionUseMotor(); 
void applyMotorMode(); 
void ActionMotorStopMin(); 
void ActionMotorStopMax(); 
void ActionUseMotorEndLimit(); 
void ActionMotorCurrentPos(); 
void ActionMotorMotorSpeed(); 
void ActionSetCurrentToMax(); 
void ActionSetCurrentToMin(); 
void ActionResetCurrentPos(); 
void ActionMotorChangeThread(); 
void ActionChangeMotor1Offset(); 
void ActionIncMotor1Offset();
void ActionDecMotor1Offset(); 
void ActionMotor1ThreadUseY();
void ActionChangeMotor1ThreadDiameter(); 
void ActionChangeMotor1ThreadAngle(); 
boolean M1_AreYouOkToStartTheThread();
boolean M1_AreYouOkToReturnAfterThread();
void ActionScreenMode(); 
void ActionChangeScreen();
void IT_Timer1_Overflow(); 
void IT_Timer2_Overflow(); 
void IT_Timer3_Overflow(); 
void Update_Overlfow_Timer4();



#include "display.h"




GEM_u8g2 menu(u8g2,GEM_POINTER_ROW,5,10,10,75); // menu
#ifdef JF_Snake
Snake MySnake(u8g2);
#endif
Msg MyMsg(u8g2);
Keypad customKeypad ( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); //Keypad
//StepperMotor Motor1(800,false,PIN_MOT1_STEP,PIN_MOT1_DIR,PIN_MOT1_EN,Update_Overlfow_Timer4);// Motor 1





// ***************************************************************************************
// ***************************************************************************************
// *** setup, loop, ...  *****************************************************************
void setup() 
{
  //External reset for boot of the screen without problem
  pinMode(PIN_RES_SCR, OUTPUT);  //channel A
  digitalWrite(PIN_RES_SCR,0);
  delay(500);
  digitalWrite(PIN_RES_SCR,1);
  delay(500);

  SetUp_Hardware();
  
  //Restore config  
  Restore_Config();

  u8g2.initDisplay();
  u8g2.setPowerSave(0);
  u8g2.clear();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();   

/*
MyMsg.DisplayMsg(GetTxt(Id_Msg_Warning_SpeedTooHigh),Msg::Warning,7000);
MyMsg.DisplayMsg(GetTxt(Id_Msg_Warning_WrongDirection),Msg::Warning,7000);
MyMsg.DisplayMsg(GetTxt(Id_Msg_Warning_NoEndLimit),Msg::Warning,7000);
MyMsg.DisplayMsg(GetTxt(Id_Msg_Warning_NoAtMinPos),Msg::Warning,7000);
MyMsg.DisplayMsg(GetTxt(Id_Msg_Warning_YINFDIA),Msg::Warning,7000);
MyMsg.DisplayMsg(GetTxt(Id_Msg_Warning_YSUPDIA),Msg::Warning,7000);
*/
  //Display start screen
  Display_StartScreen(); 

  //Debug port...
  //USB Serial
  Serial.begin(115200); // Ignored by Maple. But needed by boards using hardware serial via a USB to Serial adaptor  


  customKeypad.begin( makeKeymap(hexaKeys)); 

 
  // Menu init, setup and draw
  menu.init();
  setupMenu();
  ActionDro(); //Start with dro screen
}
void setupMenu() {
  // Add menu items to menu page
  menuPageMain.addMenuItem(menuItemButtonDro);
  //Add Sub menu shortcuts
  menuPageMain.addMenuItem(menuItemShortcuts);
  menuPageShortcuts.addMenuItem(menuItemButtonShortcutsResetX);
  menuPageShortcuts.addMenuItem(menuItemButtonShortcutsResetY);
  menuPageShortcuts.addMenuItem(menuItemButtonShortcutsResetM1);
  menuPageShortcuts.addMenuItem(menuItemButtonShortcutsSetPosToMax);
  menuPageShortcuts.addMenuItem(menuItemButtonShortcutsSetPosToMin);
  menuPageShortcuts.addMenuItem(menuItemButtonShortcutsM1inManual);
  menuPageShortcuts.addMenuItem(menuItemButtonShortcutsM1inAuto);
  menuPageShortcuts.setParentMenuPage(menuPageMain);
  //Add Sub menu Axe
  menuPageMain.addMenuItem(menuItemAxe);
  menuPageAxe.addMenuItem(menuItemTool);
  menuPageAxe.addMenuItem(menuItemRelativeMode);
  menuPageAxe.addMenuItem(menuItemButtonResetX);
  menuPageAxe.addMenuItem(menuItemButtonResetY);
  menuPageAxe.addMenuItem(menuItemAxeXPos);
  menuItemAxeXPos.setPrecision(3);
  menuPageAxe.addMenuItem(menuItemAxeYPos);
  menuItemAxeYPos.setPrecision(3);
  menuPageAxe.setParentMenuPage(menuPageMain);
  //Create sub menu Thread parameter for menu Motor
  menuPageThreadParameters.setParentMenuPage(menuPageMotor);
  menuPageThreadParameters.addMenuItem(menuItemMotorThread);
  menuPageThreadParameters.addMenuItem(menuItemMotor1ThreadInfo);
  menuItemMotor1ThreadInfo.setPrecision(1);  
  menuPageThreadParameters.addMenuItem(menuItemMotor1ThreadOffset);
  menuItemMotor1ThreadOffset.setPrecision(2);
  menuPageThreadParameters.addMenuItem(menuItemMotor1ThreadUseY);
  menuPageThreadParameters.addMenuItem(menuItemMotor1ThreadDiameter);
  menuItemMotor1ThreadDiameter.setPrecision(2);
  menuPageThreadParameters.addMenuItem(menuItemMotor1ThreadAngle);
  menuItemMotor1ThreadAngle.setPrecision(2);
  menuPageThreadParameters.addMenuItem(menuItemMotorIncOffset);
  menuPageThreadParameters.addMenuItem(menuItemMotorDecOffset);
  //Create sub menu Profil parameter for menu Motor  
  menuPageProfilParameters.setParentMenuPage(menuPageMotor);

 
  //Add Sub menu Motor
  menuPageMain.addMenuItem(menuItemMotor);
  menuPageMotor.addMenuItem(menuItemUseMotor);
  menuPageMotor.addMenuItem(menuItemUseMotorEndLimit);
  menuPageMotor.addMenuItem(menuItemMotorMode);
  menuPageMotor.addMenuItem(menuItemMotorSpeed); 
  menuPageMotor.addMenuItem(menuItemMotorCurrentPos);
  menuItemMotorCurrentPos.setPrecision(2); 
  menuPageMotor.addMenuItem(menuItemMotorStopMin);
  menuItemMotorStopMin.setPrecision(2);
  menuPageMotor.addMenuItem(menuItemMotorStopMax);
  menuItemMotorStopMax.setPrecision(2);
  menuPageMotor.addMenuItem(menuItemThreadParameters); //Sub menu thread parameter
  menuPageMotor.addMenuItem(menuItemProfil); //Sub menu profil parameter
  menuPageMotor.addMenuItem(menuItemButtonSetPosToMax);
  menuPageMotor.addMenuItem(menuItemButtonSetPosToMin);
  menuPageMotor.addMenuItem(menuItemButtonResetCurrentPos);
  menuPageMotor.setParentMenuPage(menuPageMain);
  //Add Sub menu Settings
  menuPageMain.addMenuItem(menuItemMainSettings); 
  menuPageSettings.addMenuItem(menuItemDirX);
  menuPageSettings.addMenuItem(menuItemResoX);
  menuPageSettings.addMenuItem(menuItemDirY);
  menuPageSettings.addMenuItem(menuItemResoY);
  menuPageSettings.addMenuItem(menuItemDirZ);
  menuPageSettings.addMenuItem(menuItemResoZ);
  menuPageSettings.addMenuItem(menuItemDiamY);
  menuPageSettings.addMenuItem(menuItemDirM1);
  menuPageSettings.addMenuItem(menuItemResoM1);
  menuPageSettings.addMenuItem(menuItemThreadM1);
  menuPageSettings.addMenuItem(menuItemAccelM1);
  menuPageSettings.addMenuItem(menuItemSpeedM1); 
  menuPageSettings.addMenuItem(menuItemLang);
  menuPageSettings.addMenuItem(menuItemUseUSB);
  menuPageSettings.addMenuItem(menuItemButtonRestoreSettings);
  menuPageSettings.addMenuItem(menuItemButtonSaveSettings);
  menuPageSettings.setParentMenuPage(menuPageMain);
  //Add item screen mode
  menuPageMain.addMenuItem(menuItemScreenMode);
 #ifdef JF_Snake
  //Add snake game button
  menuPageMain.addMenuItem(menuItemButtonSnakeGame);
 #endif 
  //Add Sub menu Debug
  menuPageMain.addMenuItem(menuItemDebug);
  menuPageDebug.addMenuItem(menuItemTestFloat);
  menuPageDebug.addMenuItem(menuItemButtonDebug); 
  menuPageDebug.setParentMenuPage(menuPageMain);
  // Add menu page to menu and set it as current
  menu.setMenuPageCurrent(menuPageMain);
  //Set read only because it's OFF
  SetReadOnlyMotorFunctions(true);
}


uint8_t in_ptr = 0;
char in_buffer[256];
bool stopped = true;
bool verbose = true;
uint32_t last_time = 0;
int selected = -1;


void loop() {
  
  // This loop turn when i'm in the menu !
  if (menu.readyForKey()) 
  {
    Display_UpdateRealTimeData(); 
    menu.registerKeyPress(customKeypad.getKey());
  }

}// endloop
// ***************************************************************************************
// ***************************************************************************************
// *** DRO main context ******************************************************************
void ActionDro() {
  menu.context.loop = DroContextLoop;
  menu.context.enter = DroContextEnter;
  menu.context.exit = DroContextExit;
  menu.context.allowExit = false; // Setting to false will require manual exit from the loop
  menu.context.enter();
}
void DroContextEnter() {
  // Clear sreen
  u8g2.clear();
}
void DroContextLoop() 
{
  byte key = customKeypad.getKey();
  if (key == GEM_KEY_CANCEL) 
  { 
    // Exit Dro screen if GEM_KEY_CANCEL key was pressed
    menu.context.exit();
  } else 
  {
    if( bMotorMode == MOTOR_MODE_MANUAL || bMotorMode == MOTOR_MODE_AUTO )
    {
      if(key == GEM_KEY_UP)
      {
        ActionMotorSpeedUp();
        ActionMotorMotorSpeed();
      }
      if(key == GEM_KEY_DOWN)
      {
        ActionMotorSpeedDown(); 
        ActionMotorMotorSpeed();  
      }
    } 
    // Ok key for change the screen only when the motor is not in speed mode
    if(key == GEM_KEY_OK && (Motor1.ReturnTheMode()!=StepperMotor::SpeedModeUp))
    {
      if(key == GEM_KEY_OK && (Motor1.ReturnTheMode()!=StepperMotor::SpeedModeDown))
      {
        ActionChangeScreen();
      }  
    }
    //**** Manual mode Key *****
    if( bMotorMode == MOTOR_MODE_MANUAL)
    {
      if( customKeypad.isPressed(GEM_KEY_LEFT) || customKeypad.isPressed(GEM_KEY_RIGHT))
      {
        eScreenChoose = SCREEN_MOT1;
        //eScreenChoose = SCREEN_DEBUG;
        if( customKeypad.isPressed(GEM_KEY_LEFT))
        {
          Motor1.ChangeTheMode(StepperMotor::SpeedModeUp);
          if(customKeypad.isPressed(GEM_KEY_UP))
          {
            //Move the end limit Max
            fMotorStopMax += 0.05;
            ActionMotorStopMax();     
          }
          if(customKeypad.isPressed(GEM_KEY_DOWN))
          {
            //Move the end limit Max
            fMotorStopMax -= 0.05;
            ActionMotorStopMax();     
          }    
        }
        if( customKeypad.isPressed(GEM_KEY_RIGHT))
        {
          Motor1.ChangeTheMode(StepperMotor::SpeedModeDown);
          if(customKeypad.isPressed(GEM_KEY_UP))
          {
            //Move the end limit Max
            fMotorStopMin += 0.05;
            ActionMotorStopMin();     
          }
          if(customKeypad.isPressed(GEM_KEY_DOWN))
          {
            //Move the end limit Max
            fMotorStopMin -= 0.05;
            ActionMotorStopMin();     
          }  
        }
      }
      else Motor1.ChangeTheMode(StepperMotor::NoMode);
      //Fast Speed with OK pressed
      if(Motor1.ReturnTheMode()!=StepperMotor::SpeedModeUp || Motor1.ReturnTheMode()!=StepperMotor::SpeedModeDown)
      {
        if(customKeypad.isPressed(GEM_KEY_OK))Motor1.ChangeMaxSpeed(sGeneralConf.Speed_M1);
        else Motor1.ChangeMaxSpeed(iMotorSpeed);   
      } 
    }
    //**** Auto mode Key *****
    if (bMotorMode == MOTOR_MODE_AUTO)
    {
      if (key == GEM_KEY_LEFT ) 
      {
        eScreenChoose = SCREEN_MOT1;
        if( Motor1.ReturnTheMode() == StepperMotor::NoMode ) Motor1.ChangeTheMode(StepperMotor::SpeedModeUp);
        else Motor1.ChangeTheMode(StepperMotor::NoMode);      
      }  
      if (key == GEM_KEY_RIGHT ) 
      { 
        eScreenChoose = SCREEN_MOT1;
        if( Motor1.ReturnTheMode() == StepperMotor::NoMode ) Motor1.ChangeTheMode(StepperMotor::SpeedModeDown);
        else Motor1.ChangeTheMode(StepperMotor::NoMode);  
      }
      //Fast Speed with OK pressed
      if(Motor1.ReturnTheMode()!=StepperMotor::SpeedModeUp || Motor1.ReturnTheMode()!=StepperMotor::SpeedModeDown)
      {
        if(customKeypad.isPressed(GEM_KEY_OK))Motor1.ChangeMaxSpeed(sGeneralConf.Speed_M1);
        else Motor1.ChangeMaxSpeed(iMotorSpeed);   
      }    
     } 
     if ( bMotorMode == MOTOR_MODE_TH_EXT_N ||
          bMotorMode == MOTOR_MODE_TH_EXT_I ||
          bMotorMode == MOTOR_MODE_TH_INT_N ||
          bMotorMode == MOTOR_MODE_TH_INT_I )
    {
      switch(eMS_Thread)
      {
        case MS_THREAD_IDLE:
        break;
        case MS_THREAD_WAIT_THE_START:
          if (key == GEM_KEY_LEFT )
          {
            if(M1_AreYouOkToStartTheThread() == true)
            {
              //Calcul the motor parameter for Thread before start
              CalcMotorParameterForThread();
              eMS_Thread = MS_THREAD_WAIT_THE_SPLINDLE_ZERO;
              _Synch_Spindle = 1;                
            }
          }   
        break;
        case MS_THREAD_WAIT_THE_SPLINDLE_ZERO:
          //No action here
        break;
        case MS_THREAD_IN_THREAD:
          if ( Motor1.AreYouAtMaxPos() )
          {
            eMS_Thread = MS_THREAD_END_THREAD;
            Motor1.ChangeTheMode(StepperMotor::NoMode);    
          }   
        break;
        case MS_THREAD_END_THREAD:
          if (key == GEM_KEY_RIGHT )
          {
            if(M1_AreYouOkToReturnAfterThread() == true)
            {
              eMS_Thread = MS_THREAD_IN_RETURN;
              Motor1.ChangeTheMode(StepperMotor::SpeedModeDown);              
            }
          }
        break;
        case MS_THREAD_IN_RETURN:
          if ( Motor1.AreYouAtMinPos() )
          {
            eMS_Thread = MS_THREAD_WAIT_THE_START;
            Motor1.ChangeTheMode(StepperMotor::PositionMode);    
          }          
        break;   
      }
    }     
    DisplayDrawInformations();
    UsbSerial_Pos();   
  }
}
void DroContextExit() 
{
  menu.reInit();
  menu.drawMenu();
  menu.clearContext();
}
// ***************************************************************************************
// ***************************************************************************************
// *** Debug context *********************************************************************
void ActionDebug()
{
  menu.context.loop = DebugContextLoop;
  menu.context.enter = DebugContextEnter;
  menu.context.exit = DebugContextExit;
  menu.context.allowExit = false; // Setting to false will require manual exit from the loop
  menu.context.enter();    
}
void DebugContextEnter() 
{
  // Clear sreen
  u8g2.clear();
}
void DebugContextLoop() 
{
  byte key = customKeypad.getKey();
  u8g2.firstPage();
  do {
      u8g2.setColorIndex(1);
      u8g2.setFont(u8g2_font_profont10_mr); // choose a suitable font
      char buffer[16];
      sprintf(buffer,"Numerator:%ld",sThreadCalc.Numerator);
      u8g2.drawStr(0,0,buffer);
      sprintf(buffer,"Denominator:%ld",sThreadCalc.Denominator);
      u8g2.drawStr(0,10,buffer);
      sprintf(buffer,"Offset:%ld",sThreadCalc.Offset);
      u8g2.drawStr(0,20,buffer);  
      sprintf(buffer,"speed:%f",fM1MaxThreadSpeed);
      u8g2.drawStr(0,30,buffer);  

           
  } while (u8g2.nextPage());
  if (key == GEM_KEY_CANCEL) 
  { 
    menu.context.exit();
  }
}
void DebugContextExit() 
{
  menu.reInit();
  menu.drawMenu();
  menu.clearContext();
}

#include "Display_Fonc.h"


// ***************************************************************************************
// ***************************************************************************************
// *** Action functions from menu ********************************************************

void NeedToSave()
{
  bSettingsNeedToBeSaved = true;
}
void ActionChangeDirX()
{
  NeedToSave();  
}
void ActionChangeDirY()
{
  NeedToSave();
}
void ActionChangeDirZ()
{
  NeedToSave();
}
void ActionChangeDirM1()
{
  NeedToSave();
}
void ActionChangeResoX()
{
  NeedToSave();
  if(sGeneralConf.Reso_X < 1)sGeneralConf.Reso_X = 1;
  if(sGeneralConf.Reso_X > 10000)sGeneralConf.Reso_X = 10000;     
}
void ActionChangeResoY()
{
  NeedToSave();
  if(sGeneralConf.Reso_Y < 1)sGeneralConf.Reso_Y = 1;
  if(sGeneralConf.Reso_Y > 10000)sGeneralConf.Reso_Y = 10000;  
}
void ActionChangeResoZ()
{
  NeedToSave();
  if(sGeneralConf.Reso_C < 1)sGeneralConf.Reso_C = 1;
  if(sGeneralConf.Reso_C > 10000)sGeneralConf.Reso_C = 10000;   
}
void ActionChangeResoM1()
{
  NeedToSave();
  if(sGeneralConf.Reso_M1 < 1)sGeneralConf.Reso_M1 = 1;
  if(sGeneralConf.Reso_M1 > 10000)sGeneralConf.Reso_M1 = 10000;  
}
void ActionChangeDiamY()
{
  NeedToSave();
}
void ActionChangeThreadM1()
{
  NeedToSave();
  if(sGeneralConf.thread_M1 < 1)sGeneralConf.thread_M1 = 1;
  if(sGeneralConf.thread_M1 > 10000)sGeneralConf.thread_M1 = 10000;
}
void ActionChangeAccelM1()
{
  NeedToSave();
  if(sGeneralConf.Accel_M1 < 1.0)sGeneralConf.Accel_M1 = 1.0;
  if(sGeneralConf.Accel_M1 > 500000.0)sGeneralConf.Accel_M1 = 500000.0;
}
void ActionChangeSpeedM1()
{
  NeedToSave();
  if(sGeneralConf.Speed_M1 < 1)sGeneralConf.Speed_M1 = 1;
  if(sGeneralConf.Speed_M1 > 30000)sGeneralConf.Speed_M1 = 30000;
}
void ActionChangeUseUSB()
{
  
  
}

void ActionShortcutsResetX()
{
  Quad_X.SetZeroActiveMode();
  ActionDro();   
}
void ActionShortcutsResetY()
{
  Quad_Y.SetZeroActiveMode();
  ActionDro();  
}
void ActionShortcutsResetM1()
{
  ActionResetCurrentPos();
  ActionDro(); 
}
void ActionShortcutsSetCurrentToMax()
{
  ActionSetCurrentToMax();
  ActionDro();  
}
void ActionShortcutsSetCurrentToMin()
{
  ActionSetCurrentToMin();
  ActionDro();  
}
void ActionShortcutsM1inManual()
{
  bMotorMode = MOTOR_MODE_MANUAL; 
  applyMotorMode();  
  ActionDro();  
}
void ActionShortcutsM1inAuto()
{
  bMotorMode = MOTOR_MODE_AUTO; 
  applyMotorMode();  
  ActionDro();     
}

void ActionChangeRelaticeMode()
{  
  if( bRelativeModeActived == true )
  {
    Quad_X.SetRelative();  
    Quad_Y.SetRelative();
    //Quad_C.SetRelative();
  }
  else
  {
    Quad_X.SetAbsolut();  
    Quad_Y.SetAbsolut();
    //Quad_C.SetAbsolut();
  }      
}
void applyTool()
{    
}
void SetReadOnlyMotorFunctions(boolean state)
{
  menuItemMotorMode.setReadonly(state);  
  menuItemMotorStopMin.setReadonly(state);
  menuItemMotorStopMax.setReadonly(state);
  menuItemUseMotorEndLimit.setReadonly(state);
  menuItemMotorCurrentPos.setReadonly(state);
  menuItemMotorSpeed.setReadonly(state);
  menuItemButtonSetPosToMax.setReadonly(state);
  menuItemButtonSetPosToMin.setReadonly(state);
  menuItemButtonResetCurrentPos.setReadonly(state);
  menuItemThreadParameters.setReadonly(state);
  menuItemProfil.setReadonly(state);
}
void ActionUseMotor()
{
  if( bUseMotor == true ) 
  {
    Motor1.ChangeTheMode(StepperMotor::NoMode);
    bMotorMode = MOTOR_MODE_NO_MODE; 
    ActionMotorStopMin();
    ActionMotorStopMax(); 
    ActionMotorCurrentPos();
    ActionMotorMotorSpeed();
    Motor1.UseEndLimit(bUseMotorEndLimit);
    Motor1.MotorChangePowerState(true);
    eMS_Thread = MS_THREAD_IDLE;
    eScreenChoose = SCREEN_MOT1;
    SetReadOnlyMotorFunctions(false); 
  }
  else
  {
    Motor1.ChangeTheMode(StepperMotor::NoMode);
    bMotorMode = MOTOR_MODE_NO_MODE; 
    Motor1.MotorChangePowerState(false);
    eMS_Thread = MS_THREAD_IDLE;
    eScreenChoose = SCREEN_DRO; 
    SetReadOnlyMotorFunctions(true);   
  }     
}
void ActionUseMotorEndLimit()
{
  Motor1.UseEndLimit(bUseMotorEndLimit);  
}

void CalcMotorParameterForThread()
{
  long lGCD;
  //Calc Numerator and Denominator with simplification
  sThreadCalc.Numerator = sGeneralConf.Reso_M1 * iMotorThread;  
  sThreadCalc.Denominator = sGeneralConf.thread_M1 * sGeneralConf.Reso_C ; 
  lGCD = GCD_Function(sThreadCalc.Numerator,sThreadCalc.Denominator);
  sThreadCalc.Numerator = sThreadCalc.Numerator / lGCD;
  sThreadCalc.Denominator = sThreadCalc.Denominator / lGCD;   
  //If reverse direction
  if( bMotorMode == MOTOR_MODE_TH_EXT_I || bMotorMode == MOTOR_MODE_TH_INT_I)
  {
    sThreadCalc.Numerator = -sThreadCalc.Numerator;
  }
  CalcMotorParameterOffsetForThread();
}
void CalcMotorParameterOffsetForThread()
{
  float OffsetFixe = 0.0; //Constant offset
  float OffsetVariable = 0.0; //variable offset
  float fTemp = 0.0; // pour calcul du demi pas
  //Calc of the fixe offset
  OffsetFixe = (float)((360.0 - fMotor1ThreadOffset)*iMotorThread*sGeneralConf.Reso_M1) /(float)(360*sGeneralConf.thread_M1); 
  //Calcul of the variable offset ( depend of the diameter and Y position).
  if(bMotor1ThreadUseY == true)
  {
    if(bMotorMode == MOTOR_MODE_TH_EXT_N || bMotorMode == MOTOR_MODE_TH_EXT_I)
    {
      if(fAxeYPos <= fMotor1ThreadDiameter)
      {
        OffsetVariable = (float)((fMotor1ThreadDiameter - fAxeYPos)/2.0 * tan(fMotor1ThreadAngle * 0.01745)); //Pi/180 = 0.01745
        OffsetVariable = OffsetVariable *(float)(sGeneralConf.Reso_M1*100.0/sGeneralConf.thread_M1) ; 
      }else
      {
        OffsetVariable = 0.0;  
      }          
    }
    if(bMotorMode == MOTOR_MODE_TH_INT_N || bMotorMode == MOTOR_MODE_TH_INT_I)
    {
      if(fAxeYPos >= fMotor1ThreadDiameter)
      {
        OffsetVariable = (float)((fAxeYPos - fMotor1ThreadDiameter)/2.0 * tan(fMotor1ThreadAngle * 0.01745)); //Pi/180 = 0.01745
        OffsetVariable = OffsetVariable *(float)(sGeneralConf.Reso_M1*100.0/sGeneralConf.thread_M1) ; 
      }else
      {
        OffsetVariable = 0.0;  
      }          
    }
    //L'offset variable ne peut pas dépasser le demi pas demandé 
    fTemp = (float)(sGeneralConf.Reso_M1*iMotorThread /(sGeneralConf.thread_M1 * 2.0)) ; 
    if( OffsetVariable > fTemp ) OffsetVariable = fTemp;  
    
  }
  //Global offset
  sThreadCalc.Offset = Motor1.GetStopPositionMinStep() - (long)OffsetFixe + (long)OffsetVariable ;
}
void CalcMotorMaxSpeedForThread()
{
  fM1MaxThreadSpeed = (float)(sGeneralConf.Speed_M1*60.0*sGeneralConf.thread_M1/(sGeneralConf.Reso_M1*iMotorThread));  
}
void applyMotorMode()
{
  switch (bMotorMode)
  {
    case MOTOR_MODE_TH_EXT_N :
    case MOTOR_MODE_TH_EXT_I:
    case MOTOR_MODE_TH_INT_N:
    case MOTOR_MODE_TH_INT_I:
      eMS_Thread = MS_THREAD_WAIT_THE_START;
      //Use Max speed in the setting
      Motor1.ChangeMaxSpeed(sGeneralConf.Speed_M1); 
      //Motor in position mode
      Motor1.ChangeTheMode(StepperMotor::PositionMode);    
    break; 
    case MOTOR_MODE_NO_MODE :
    case MOTOR_MODE_MANUAL :
    case MOTOR_MODE_AUTO :
    default:
      eMS_Thread = MS_THREAD_IDLE;
      ActionMotorMotorSpeed();
      Motor1.ChangeTheMode(StepperMotor::NoMode);  
    break;
  }  
}
void ActionMotorStopMin()
{
  Motor1.ChangeStopPositionMinReal(fMotorStopMin);  
}
void ActionMotorStopMax()
{
  Motor1.ChangeStopPositionMaxReal(fMotorStopMax);  
}
void ActionMotorCurrentPos()
{
  Motor1.ChangeCurrentPositionReal(fMotorCurrentPos);    
}
void ActionMotorSpeedUp()
{ //20%
  if(iMotorSpeed == iMotorSpeed + iMotorSpeed / 5) iMotorSpeed++;
  else iMotorSpeed = iMotorSpeed + iMotorSpeed / 5 ;   
}
void ActionMotorSpeedDown()
{ //-20%
  if(iMotorSpeed == iMotorSpeed - iMotorSpeed / 5) iMotorSpeed--;
  else iMotorSpeed = iMotorSpeed - iMotorSpeed / 5 ;  
}
void ActionMotorMotorSpeed()
{
  if(iMotorSpeed < 1)iMotorSpeed=1;
  if(iMotorSpeed > sGeneralConf.Speed_M1)iMotorSpeed = sGeneralConf.Speed_M1;
  Motor1.ChangeMaxSpeed(iMotorSpeed);    
}
void ActionMotorChangeThread()
{
  if(iMotorThread<=0)iMotorThread = 100;
  CalcMotorMaxSpeedForThread();  
}
void ActionSetCurrentToMax()
{
  Display_UpdateRealTimeData();
  fMotorStopMax = fMotorCurrentPos;
  ActionMotorStopMax();    
}
void ActionSetCurrentToMin()
{
  Display_UpdateRealTimeData();
  fMotorStopMin = fMotorCurrentPos;
  ActionMotorStopMin();   
}
void ActionResetCurrentPos()
{
  fMotorCurrentPos = 0;
  ActionMotorCurrentPos();  
}
void ActionResetX()
{
  Quad_X.SetZeroActiveMode(); 
}
void ActionResetY()
{
  Quad_Y.SetZeroActiveMode();  
}
void ActionAxeXPos()
{
  Quad_X.SetValue(fAxeXPos);    
}
void ActionAxeYPos()
{
  Quad_Y.SetValue(fAxeYPos);   
}
void ActionScreenMode()
{
  
}
void ActionChangeScreen()
{
  if(eScreenChoose>=SCREEN_END_LIST)eScreenChoose = SCREEN_DRO;
  else eScreenChoose++; 
}
void ActionChangeMotor1Offset()
{
  if(fMotor1ThreadOffset < 0)fMotor1ThreadOffset = 0.0;
  if(fMotor1ThreadOffset > 360)fMotor1ThreadOffset = 360.0;
  ///Possibility to change the offset durring the threading and spindle off  
  if(eMS_Thread == MS_THREAD_IN_THREAD && fAxeCSpeed == 0)
  {
    CalcMotorParameterOffsetForThread();    
  } 
}
void ActionIncMotor1Offset()
{
  fMotor1ThreadOffset = fMotor1ThreadOffset+2;
  ActionChangeMotor1Offset();
}
void ActionDecMotor1Offset()
{
  fMotor1ThreadOffset = fMotor1ThreadOffset-2;
  ActionChangeMotor1Offset();
} 
void ActionMotor1ThreadUseY()
{
}
void ActionChangeMotor1ThreadDiameter()
{
  if(fMotor1ThreadDiameter<0) fMotor1ThreadDiameter = -fMotor1ThreadDiameter;
}
void ActionChangeMotor1ThreadAngle()
{
  if(fMotor1ThreadAngle < 0)fMotor1ThreadAngle = 0.0;
  if(fMotor1ThreadAngle > 45)fMotor1ThreadAngle = 45; 
}
boolean M1_AreYouOkToStartTheThread()
{
  boolean result = true;
  //Check position of Y
  if(bMotor1ThreadUseY == true)
  {
    if( bMotorMode == MOTOR_MODE_TH_EXT_N || bMotorMode == MOTOR_MODE_TH_EXT_I)
    {
//      if( fAxeYPos > fMotor1ThreadDiameter)
//      {
//        result = false;
//        Display_Notice_Informations("Move Y : Y > Dia");     
//      }    
    }
    if( bMotorMode == MOTOR_MODE_TH_INT_N || MOTOR_MODE_TH_INT_I)
    {
//      if( fAxeYPos < fMotor1ThreadDiameter)
//      {
//        result = false;
//        Display_Notice_Informations("Move Y : Y < Dia");     
//      }    
    }    
  } 
  CalcMotorMaxSpeedForThread(); //Check the speed
  //Check the max speed
  if( fAxeCSpeed >= fM1MaxThreadSpeed)
  {
    result = false;
    MyMsg.DisplayMsg(GetTxt(Id_Msg_Warning_SpeedTooHigh),Msg::Warning,2000);    
  }
  //Check the direction
  if( bMotorMode == MOTOR_MODE_TH_EXT_N || bMotorMode == MOTOR_MODE_TH_INT_N)
  {
    if( fAxeCSpeed < 0 )
    {
      result = false;
      MyMsg.DisplayMsg(GetTxt(Id_Msg_Warning_WrongDirection),Msg::Warning,2000);    
    }      
  }
  if( bMotorMode == MOTOR_MODE_TH_EXT_I || bMotorMode == MOTOR_MODE_TH_INT_I)
  {
    if( fAxeCSpeed > 0 )
    {
      result = false;
      MyMsg.DisplayMsg(GetTxt(Id_Msg_Warning_WrongDirection),Msg::Warning,2000);    
    }       
  }   
  //Check if endlimit is on
  if( !bUseMotorEndLimit)
  {
    result = false;
    MyMsg.DisplayMsg(GetTxt(Id_Msg_Warning_NoEndLimit),Msg::Warning,2000);   
  } 
  //Check if the motor is at min pos
  if( !Motor1.AreYouAtMinPos())
  {
    result = false;
    MyMsg.DisplayMsg(GetTxt(Id_Msg_Warning_NoAtMinPos),Msg::Warning,2000);     
  }   
  return result;
}
boolean M1_AreYouOkToReturnAfterThread()
{
  boolean result = true;
  if(bMotor1ThreadUseY == true)
  {
    if( bMotorMode == MOTOR_MODE_TH_EXT_N || bMotorMode == MOTOR_MODE_TH_EXT_I)
    {
      if( fAxeYPos < fMotor1ThreadDiameter)
      {
        result = false;
        MyMsg.DisplayMsg(GetTxt(Id_Msg_Warning_YINFDIA),Msg::Warning,2000);
      }    
    }
    if( bMotorMode == MOTOR_MODE_TH_INT_N || bMotorMode == MOTOR_MODE_TH_INT_I)
    {
      if( fAxeYPos > fMotor1ThreadDiameter)
      {
        result = false;
        MyMsg.DisplayMsg(GetTxt(Id_Msg_Warning_YSUPDIA),Msg::Warning,2000);
      }   
    }      
  }
  return result;    
}
void ActionChangeLang()
{  
  ChangeLang(sGeneralConf.Lang); 
  ActionUpdateMenuTitle();  
}
void ActionUpdateMenuTitle()
{
  menuPageSettings.setTitle(GetTxt(Id_Msg_TEXT_MENU_SETTINGS));
  menuItemMainSettings.setTitle(GetTxt(Id_Msg_TEXT_MENU_SETTINGS));
  menuItemButtonDro.setTitle(GetTxt(Id_Msg_TEXT_MENU_RETURN_SCREEN));
  menuItemAxe.setTitle(GetTxt(Id_Msg_TEXT_MENU_AXE_FUNCTIONS));
  menuItemMotor.setTitle(GetTxt(Id_Msg_TEXT_MENU_MOTOR_FUNCTIONS));  
  menuItemScreenMode.setTitle(GetTxt(Id_Msg_TEXT_MENU_ECRAN));
  menuItemButtonRestoreSettings.setTitle(GetTxt(Id_Msg_TEXT_MENU_RESTORE_SETTINGS) );
  menuItemButtonSaveSettings.setTitle(GetTxt(Id_Msg_TEXT_MENU_SAVE_SETTINGS));
  menuItemShortcuts.setTitle(GetTxt(Id_Msg_TEXT_MENU_FAST_FUNCTIONS));
  menuPageShortcuts.setTitle(GetTxt(Id_Msg_TEXT_MENU_FAST_FUNCTIONS));
  menuItemButtonShortcutsM1inManual.setTitle(GetTxt(Id_Msg_TEXT_MENU_FAST_M1MANU));
  menuItemButtonShortcutsM1inAuto.setTitle(GetTxt(Id_Msg_TEXT_MENU_FAST_M1AUTO));
  menuPageAxe.setTitle(GetTxt(Id_Msg_TEXT_MENU_AXE_FUNCTIONS));
  menuPageMotor.setTitle(GetTxt(Id_Msg_TEXT_MENU_MOTOR_FUNCTIONS));
  menuPageDebug.setTitle(GetTxt(Id_Msg_TEXT_MENU_DEBUG));
  menuItemDebug.setTitle(GetTxt(Id_Msg_TEXT_MENU_DEBUG));
  menuPageThreadParameters.setTitle(GetTxt(Id_Msg_TEXT_MENU_THREAD_PARAMETERS));
  menuItemThreadParameters.setTitle(GetTxt(Id_Msg_TEXT_MENU_THREAD_PARAMETERS));
  menuItemUseMotorEndLimit.setTitle(GetTxt(Id_Msg_TEXT_MENU_MOTORDLIMIT));
  menuItemUseMotor.setTitle(GetTxt(Id_Msg_TEXT_MENU_MOTORABLED));
  menuItemMotorSpeed.setTitle(GetTxt(Id_Msg_TEXT_MENU_MOTOR_SPEED)); 
  menuItemMotorThread.setTitle(GetTxt(Id_Msg_TEXT_MENU_THREAD_THREAD));
  menuItemMotor1ThreadOffset.setTitle(GetTxt(Id_Msg_TEXT_MENU_THREAD_OFFSET));
  menuItemMotor1ThreadUseY.setTitle(GetTxt(Id_Msg_TEXT_MENU_THREAD_USEY));
  menuItemMotor1ThreadDiameter.setTitle(GetTxt(Id_Msg_TEXT_MENU_THREAD_DIAMETER));
  menuItemMotor1ThreadAngle.setTitle(GetTxt(Id_Msg_TEXT_MENU_THREAD_ANGLE));
  menuItemMotorIncOffset.setTitle(GetTxt(Id_Msg_TEXT_MENU_THREAD_INC));
  menuItemMotorDecOffset.setTitle(GetTxt(Id_Msg_TEXT_MENU_THREAD_DEC));
  menuItemTool.setTitle(GetTxt(Id_Msg_TEXT_MENU_AXE_TOOL));
  menuItemRelativeMode.setTitle(GetTxt(Id_Msg_TEXT_MENU_AXE_REL));
  menuItemDirX.setTitle(GetTxt(Id_Msg_TEXT_MENU_SETTINGS_XDIR));
  menuItemDirY.setTitle(GetTxt(Id_Msg_TEXT_MENU_SETTINGS_YDIR));
  menuItemDirZ.setTitle(GetTxt(Id_Msg_TEXT_MENU_SETTINGS_CDIR));
  menuItemDiamY.setTitle(GetTxt(Id_Msg_TEXT_MENU_SETTINGS_YDIAM));
  menuItemResoX.setTitle(GetTxt(Id_Msg_TEXT_MENU_SETTINGS_XSTEP));
  menuItemResoY.setTitle(GetTxt(Id_Msg_TEXT_MENU_SETTINGS_YSTEP));
  menuItemResoZ.setTitle(GetTxt(Id_Msg_TEXT_MENU_SETTINGS_CSTEP));
  menuItemDirM1.setTitle(GetTxt(Id_Msg_TEXT_MENU_SETTINGS_M1DIR));
  menuItemResoM1.setTitle(GetTxt(Id_Msg_TEXT_MENU_SETTINGS_M1STEP));
  menuItemThreadM1.setTitle(GetTxt(Id_Msg_TEXT_MENU_SETTINGS_M1TH));
  menuItemAccelM1.setTitle(GetTxt(Id_Msg_TEXT_MENU_SETTINGS_M1ACC));
  menuItemSpeedM1.setTitle(GetTxt(Id_Msg_TEXT_MENU_SETTINGS_M1SPE));
  menuItemLang.setTitle(GetTxt(Id_Msg_TEXT_MENU_SETTINGS_M1LAN));
  menuItemUseUSB.setTitle(GetTxt(Id_Msg_TEXT_MENU_SETTINGS_USB));
  menuPageProfilParameters.setTitle(GetTxt(Id_Msg_TEXT_MENU_PROFIL)); 
  menuItemProfil.setTitle(GetTxt(Id_Msg_TEXT_MENU_PROFIL));
//JF
}

  
// ***************************************************************************************
// ***************************************************************************************
// *** Save / Restore config *************************************************************


void Restore_Config()
{
  //Read Config in Memory
  ReadConfigInFlash(&sGeneralConf);
  //Dispatch the config
  Dispatch_Config(&sGeneralConf);
}
void Dispatch_Config(tsConfigDro *pConf)
{
  Quad_X.SetSens( pConf->Inverted_X );  
  Quad_Y.SetSens( pConf->Inverted_Y );
  Quad_C.SetSens( pConf->Inverted_C );
  Quad_Y.SetDiameterMode(pConf->Diameter_Mode_Y);
  Quad_X.SetResolution(pConf->Reso_X);
  Quad_Y.SetResolution(pConf->Reso_Y);
  Quad_C.SetResolution(pConf->Reso_C);
  Motor1.ChangeParameter((uint16_t)((int32_t)(  pConf->Reso_M1*100/pConf->thread_M1)) , pConf->Inverted_M1);
  Motor1.ChangeAcceleration(pConf->Accel_M1);
  CalcMotorMaxSpeedForThread(); 
  //JF
  ActionUpdateMenuTitle();  
}
void ActionSaveSettingsInFlash()
{
  //Store config in memort
  SaveConfigInFlash(&sGeneralConf);
  //Dispatch config to function
  Dispatch_Config(&sGeneralConf); 
  //PrintInformationOnScreen("Save in flash");
  //delay(100);  
  bSettingsNeedToBeSaved = false;
  MyMsg.DisplayMsg(GetTxt(Id_Msg_Save),Msg::Info,1000);
  menu.drawMenu(); //Refresh screen after  
}
void ActionRestoreSettingsInFlash()
{
  //Save default config in flash
  SaveConfigInFlash((tsConfigDro*)&csConfigDefault);
  Restore_Config();
  bSettingsNeedToBeSaved = false;
  MyMsg.DisplayMsg(GetTxt(Id_Msg_Restore),Msg::Info,1000);
  menu.drawMenu(); //Refresh screen after 
}

// ***************************************************************************************

// END
