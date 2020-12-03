#pragma once
#include <Arduino.h>
#include <driver/gpio.h>
#include "driver/pcnt.h"

//#ifndef voidfuncptr
typedef void (*voidfuncptr)(void);
//#endif

extern volatile int8_t _Synch_Spindle ;



#define MAX_ESP32_ENCODERS PCNT_UNIT_MAX
#define 	_INT16_MAX 32766
#define  	_INT16_MIN -32766
// pour raccourcir les test
//#define 	_INT16_MAX 256
//#define  	_INT16_MIN -256

#define Overflow_Size_default 65536 

enum EncTypeMod {
Lin,
Rot,
};

enum EncTypeIn {
single,
half,
full
};
//Pull Up Type
enum puType {
UP,
DOWN,
NONE
};

class ESP32Encoder {

public:



private:
	void attach(enum EncTypeMod etm,int aPintNumber, int bPinNumber, enum EncTypeIn eti);//type:EncTypeMod Lin Rot EncTypeIn:eti Alf Single Quad
	boolean attached=false;


	static  pcnt_isr_handle_t user_isr_handle; //user's ISR service handle
	static  pcnt_isr_handle_t user_isr_handle_IT; //user's ISR service handle

    bool direction;
    bool working;

	static bool attachedInterrupt;
	int64_t getCountRaw();
public:
	ESP32Encoder();
	~ESP32Encoder();
	void attachHalfQuad(int eTM,int aPintNumber, int bPinNumber);
	void attachFullQuad(int eTM,int aPintNumber, int bPinNumber);
	void attachSingleEdge(int eTM,int aPintNumber, int bPinNumber);
	int64_t getCount();
	int64_t clearCount();
	int64_t pauseCount();
	int64_t resumeCount();
	
/*********************************/	
// AJOUT COMPATIBILITE 
void attachInterrupt(voidfuncptr handler);

	void    setOverflow(uint16_t);    
	void    resume(void);//start the encoder... 
	void 	refresh(void);

	int64_t pause();  //stop...

	int64_t setEdgeCounting(int);
	void    setPrescaleFactor(uint32_t);//normal for encoder to have the lowest or no prescaler. 
	int64_t setMode(int, int); //set mode, the channel is not used when in this mode. 
    uint8_t getDirection();



/*********************************/	


	boolean isAttached(){return attached;}
	void setCount(int64_t value);
	
	static ESP32Encoder *encoders[MAX_ESP32_ENCODERS];
	gpio_num_t aPinNumber;
	gpio_num_t bPinNumber;
	enum EncTypeMod type_mod ;
	enum EncTypeIn type_in ;
	
	pcnt_unit_t unit;
	bool fullQuad=false;
	int countsMode = 2;
	volatile int64_t count=0;
	pcnt_config_t r_enc_config;
	static enum puType useInternalWeakPullResistors;
};

//Added by Sloeber 
#pragma once

