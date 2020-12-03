
//***************Pin Harware STM32***************************
#ifndef ESP32  //ESP32

// IO def ( for quad decoder, define in class !)
#define PIN_RES_SCR    PB9
#define PIN_MOT1_STEP  PA10
#define PIN_MOT1_DIR   PB15
#define PIN_MOT1_EN    PA15
#define PIN_MOT2_STEP  PB13
#define PIN_MOT2_DIR   PB12
#define PIN_MOT2_EN    PB14



#define PIN_SW_LIN_1   PA5
#define PIN_SW_LIN_2   PA4
#define PIN_SW_LIN_3   PA3
#define PIN_SW_LIN_4   PA2
#define PIN_SW_COL_1   PB0
#define PIN_SW_COL_2   PB1
#define PIN_SW_COL_3   PB10
#define PIN_SW_COL_4   PB11

#define Pin_X_A
#define Pin_X_B
#define Pin_Y_A
#define Pin_Y_B
#define Pin_C_A
#define Pin_C_B


//***************Pin Harware STM32***************************
#else  //ESP32
//***************Pin Harware ESP32***************************

#define Pin_X_A 33
#define Pin_X_B 34
#define Pin_Y_A 35 //c
#define Pin_Y_B 39 //c
#define Pin_C_A 26
#define Pin_C_B 36

#define PIN_RES_SCR    PB9

#define PIN_MOT1_STEP  4
#define PIN_MOT1_DIR   32
#define PIN_MOT1_EN    25
#define PIN_MOT2_STEP  17
#define PIN_MOT2_DIR   16
#define PIN_MOT2_EN    27



#define PA10 4 //10
#define PB15 32 //11
#define PA15 25 //12
#define PB13 27 //13
#define PB12 16 //14
#define PB14 17 //15

#define PIN_SW_LIN_1   2
#define PIN_SW_LIN_2   2
#define PIN_SW_LIN_3   2
#define PIN_SW_LIN_4   2
#define PIN_SW_COL_1   2
#define PIN_SW_COL_2   2
#define PIN_SW_COL_3   2
#define PIN_SW_COL_4   2


#define PB9 5 //9 ( 5 pin libre)

//***************Pin Harware***************************
#endif //ESP32
