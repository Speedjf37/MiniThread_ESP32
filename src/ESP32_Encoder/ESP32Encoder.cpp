/*
 * ESP32Encoder.cpp
 *
 *  Created on: Oct 15, 2018
 *      Author: hephaestus
 */
#define PCNT_UNIT_FIRST 3 // RESERVE 0,1,2 FastAccelStepper


//#include <ESP32Encoder.h>
#include "ESP32Encoder.h"
#include "QuadDecoder.h"
//static ESP32Encoder *gpio2enc[48];
//
//
enum puType ESP32Encoder::useInternalWeakPullResistors=DOWN;
ESP32Encoder *ESP32Encoder::encoders[MAX_ESP32_ENCODERS] = { NULL, NULL, NULL,
NULL,
NULL, NULL, NULL, NULL };

bool ESP32Encoder::attachedInterrupt=false;
pcnt_isr_handle_t ESP32Encoder::user_isr_handle = NULL;
pcnt_isr_handle_t ESP32Encoder::user_isr_handle_IT = NULL;

ESP32Encoder::ESP32Encoder() {
	attached = false;
	aPinNumber = (gpio_num_t) 0;
	bPinNumber = (gpio_num_t) 0;
	working = false;
	direction = false;
	type_mod = Lin;
	type_in = full;
	unit = (pcnt_unit_t) -1;
}

ESP32Encoder::~ESP32Encoder() {
	// TODO Auto-generated destructor stub
}

/* Decode what PCNT's unit originated an interrupt
 * and pass this information together with the event type
 * the main program using a queue.
 */
static void IRAM_ATTR pcnt_example_intr_handler(void *arg) {
	ESP32Encoder * ptr;
	uint32_t intr_status = PCNT.int_st.val;
	int i;

	for (i = PCNT_UNIT_FIRST ; i < PCNT_UNIT_MAX; i++) {
//	for (i = 0; i < PCNT_UNIT_MAX; i++) {
		if (intr_status & (BIT(i))) {
			ptr = ESP32Encoder::encoders[i];
			/* Save the PCNT event type that caused an interrupt
			 to pass it to the main program */
			int64_t status=0;
			if(PCNT.status_unit[i].h_lim_lat){
				status=ptr->r_enc_config.counter_h_lim;
			// Serial.print("I:");
			// Serial.print(i);
			// Serial.println(" + ");
			}
			if(PCNT.status_unit[i].l_lim_lat){
				status=ptr->r_enc_config.counter_l_lim;
			//Serial.print("I:");
			//Serial.print(i);
			//Serial.println(" - ");
			}
			//pcnt_counter_clear(ptr->unit);
			PCNT.int_clr.val = BIT(i); // clear the interrupt
			ptr->count = status + ptr->count;
		
		}// fin if evt for i
	
	}// fin for i
}

static void IRAM_ATTR IT_Overflow_handler(void *arg){
		ESP32Encoder * ptr;
	uint32_t intr_status = PCNT.int_st.val;
	int i;

	for (i = PCNT_UNIT_FIRST ; i < PCNT_UNIT_MAX; i++) {
//	for (i = 0; i < PCNT_UNIT_MAX; i++) {
		if (intr_status & (BIT(i))) {
			ptr = ESP32Encoder::encoders[i];
			// Save the PCNT event type that caused an interrupt
			// to pass it to the main program 

			int64_t status=0;
			if(PCNT.status_unit[i].h_lim_lat){
				status=ptr->r_enc_config.counter_h_lim;
			 //Serial.print("I:");
			 //Serial.print(i);
			 //Serial.println(" ++ ");
			}
			if(PCNT.status_unit[i].l_lim_lat){
				status=ptr->r_enc_config.counter_l_lim;
			//Serial.print("I:");
			//Serial.print(i);
			//Serial.println(" -- ");
			
			}
			  
			if ((status==ptr->r_enc_config.counter_l_lim)||(status==ptr->r_enc_config.counter_h_lim))
			 {
			 /*
			 Serial.print("I:");
			 Serial.print(i);
			 if (status==ptr->r_enc_config.counter_l_lim)
			  Serial.print(" - ");
			 if (status==ptr->r_enc_config.counter_h_lim)
			  Serial.print(" + ");
			*/
			 
			 
			 if (ptr->type_mod == Lin)
			  {// Lin
			  //Serial.println(" Lin");
			  ptr->count = status + ptr->count;// ptr->count totalisateur count + overflow 
			  }
			else			
			  {// Rot
			  //Serial.println(" Rot");
			 
 			  ptr->count = 0;// Allways 0
			  
			  if(_Synch_Spindle == 0)
				{
				//Quad_C.ResetAllTheCounter();
				_Synch_Spindle = 1;   
				}  
			   
			  }// else LIN
			} // end overflow
			else
			{
			//Serial.println(" ???");
			}
		 PCNT.int_clr.val = BIT(i); // clear the interrupt
		}// end if (intr_status & (BIT(i)))
	
	}// end for i



	
}

//type:EncTypeMod Lin Rot EncTypeIn:eti Alf Single Quad
void ESP32Encoder::attach(enum EncTypeMod etm,int a, int b, enum EncTypeIn eti) {
	if (attached) {
		Serial.println("All ready attached, FAIL!");
		return;
	}

	int index = PCNT_UNIT_FIRST;
//	int index = 0;
	for (; index < MAX_ESP32_ENCODERS; index++) {
		if (ESP32Encoder::encoders[index] == NULL) {
			encoders[index] = this;
			break;
		}
	}
	if (index == MAX_ESP32_ENCODERS) {
		Serial.println("Too many encoders, FAIL!");
		return;
	}

	// Set data now that pin attach checks are done
	fullQuad = eti != single;
	unit = (pcnt_unit_t) index;
	this->aPinNumber = (gpio_num_t) a;
	this->bPinNumber = (gpio_num_t) b;
	this->type_mod = (EncTypeMod) etm ;// 0 Linear / 1 Rotate
	this->type_in = (EncTypeIn) eti ;// 0 single, 1 half, 2full
	
	//Set up the IO state of hte pin
	gpio_pad_select_gpio(aPinNumber);
	gpio_pad_select_gpio(bPinNumber);
	gpio_set_direction(aPinNumber, GPIO_MODE_INPUT);
	gpio_set_direction(bPinNumber, GPIO_MODE_INPUT);
	if(useInternalWeakPullResistors==DOWN){
		gpio_pulldown_en(aPinNumber);
		gpio_pulldown_en(bPinNumber);
	}
	if(useInternalWeakPullResistors==UP){
		gpio_pullup_en(aPinNumber);
		gpio_pullup_en(bPinNumber);
	}
	// Set up encoder PCNT configuration
	r_enc_config.pulse_gpio_num = aPinNumber; //Rotary Encoder Chan A
	r_enc_config.ctrl_gpio_num = bPinNumber;    //Rotary Encoder Chan B

	r_enc_config.unit = unit;
	r_enc_config.channel = PCNT_CHANNEL_0;

/*
	r_enc_config.pos_mode = fullQuad ? PCNT_COUNT_DEC : PCNT_COUNT_DIS; //Count Only On Rising-Edges
	r_enc_config.neg_mode = PCNT_COUNT_INC;   // Discard Falling-Edge
	
*/	
	if (eti == full)
{
	r_enc_config.pos_mode = PCNT_COUNT_DEC; //Count Only On Rising-Edges
	r_enc_config.neg_mode = PCNT_COUNT_INC;   // Discard Falling-Edge
}

if (eti == half)
{
	r_enc_config.pos_mode = PCNT_COUNT_INC; //Count Only On Rising-Edges
	r_enc_config.neg_mode = PCNT_COUNT_DIS;   // Discard Falling-Edge
}

if (eti == single)
{
	r_enc_config.pos_mode = PCNT_COUNT_DIS ;
	r_enc_config.neg_mode = PCNT_COUNT_INC;   // Discard Falling-Edge
}
	
	
	

	r_enc_config.lctrl_mode = PCNT_MODE_KEEP;    // Rising A on HIGH B = CW Step
	r_enc_config.hctrl_mode = PCNT_MODE_REVERSE; // Rising A on LOW B = CCW Step

	r_enc_config.counter_h_lim = _INT16_MAX;
	r_enc_config.counter_l_lim = _INT16_MIN ;

	pcnt_unit_config(&r_enc_config);//set up first channel 

	if (eti == full) {
		// set up second channel for full quad
		r_enc_config.pulse_gpio_num = bPinNumber; //make prior control into signal
		r_enc_config.ctrl_gpio_num = aPinNumber;    //and prior signal into control

		r_enc_config.unit = unit;
		r_enc_config.channel = PCNT_CHANNEL_1; // channel 1

		r_enc_config.pos_mode = PCNT_COUNT_DEC; //Count Only On Rising-Edges
		r_enc_config.neg_mode = PCNT_COUNT_INC;   // Discard Falling-Edge

		r_enc_config.lctrl_mode = PCNT_MODE_REVERSE;    // prior high mode is now low
		r_enc_config.hctrl_mode = PCNT_MODE_KEEP; // prior low mode is now high

		r_enc_config.counter_h_lim = _INT16_MAX;
		r_enc_config.counter_l_lim = _INT16_MIN ;

		pcnt_unit_config(&r_enc_config);//set up second channel for full quad

	} else { // make sure channel 1 is not set when not full quad
		r_enc_config.pulse_gpio_num = bPinNumber; //make prior control into signal
		r_enc_config.ctrl_gpio_num = aPinNumber;    //and prior signal into control

		r_enc_config.unit = unit;
		r_enc_config.channel = PCNT_CHANNEL_1; // channel 1

		r_enc_config.pos_mode = PCNT_COUNT_DIS; //disabling channel 1
		r_enc_config.neg_mode = PCNT_COUNT_DIS;   // disabling channel 1

		r_enc_config.lctrl_mode = PCNT_MODE_DISABLE;    // disabling channel 1
		r_enc_config.hctrl_mode = PCNT_MODE_DISABLE; // disabling channel 1

		r_enc_config.counter_h_lim = _INT16_MAX;
		r_enc_config.counter_l_lim = _INT16_MIN ;

		pcnt_unit_config(&r_enc_config);//set up second channel for not full quad
	}


	// Filter out bounces and noise
	pcnt_set_filter_value(unit, 250);  // Filter Runt Pulses
	pcnt_filter_enable(unit);


	/* Enable events on  maximum and minimum limit values */
	pcnt_event_enable(unit, PCNT_EVT_H_LIM);
	pcnt_event_enable(unit, PCNT_EVT_L_LIM);

	pcnt_counter_pause(unit); // Initial PCNT init
	pcnt_counter_clear(unit);
	/* Register ISR handler and enable interrupts for PCNT unit */
	if(attachedInterrupt==false){
		attachedInterrupt=true;
//		esp_err_t er = pcnt_isr_register(pcnt_example_intr_handler,(void *) NULL, (int)0,
		esp_err_t er = pcnt_isr_register(IT_Overflow_handler,(void *) NULL, (int)0,
				(pcnt_isr_handle_t *)&ESP32Encoder::user_isr_handle_IT);
		if (er != ESP_OK){
			Serial.println("Encoder wrap interupt failed");
		}
	}
	pcnt_intr_enable(unit);
	pcnt_counter_resume(unit);

}

void ESP32Encoder::attachHalfQuad(int eTM,int aPintNumber, int bPinNumber) {
	attach((EncTypeMod) eTM,aPintNumber, bPinNumber, half);

}
void ESP32Encoder::attachSingleEdge(int eTM,int aPintNumber, int bPinNumber) {
	attach((EncTypeMod) eTM,aPintNumber, bPinNumber, single);
}
void ESP32Encoder::attachFullQuad(int eTM,int aPintNumber, int bPinNumber) {
	attach((EncTypeMod) eTM,aPintNumber, bPinNumber,full);
}

void ESP32Encoder::attachInterrupt( voidfuncptr handler) {

	// Register ISR handler and enable interrupts for PCNT unit 
	if(attachedInterrupt==false){
		attachedInterrupt=true;
//		esp_err_t er = pcnt_isr_register(pcnt_example_intr_handler,(void *) NULL, (int)0,
		esp_err_t er = pcnt_isr_register(IT_Overflow_handler,(void *) NULL, (int)0,
				(pcnt_isr_handle_t *)&ESP32Encoder::user_isr_handle_IT);
		if (er != ESP_OK){
			Serial.println("Encoder wrap interupt failed");
		}
	}
}



void ESP32Encoder::setOverflow(uint16_t param){
	char bufferChar[30];

	ESP32Encoder::pause();
	
	if (type_mod == Rot)
	{
	// change overflow to resolution encoder
	
    //sprintf(bufferChar, "Overflow U:%d %d", unit,param);
	//Serial.println(bufferChar);
	
	// Set up encoder PCNT configuration this level driver -> pcnt_unit_config(&r_enc_config)for change PCNT
// set up first channel for full quad
	
	r_enc_config.pulse_gpio_num = aPinNumber; //Rotary Encoder Chan A
	r_enc_config.ctrl_gpio_num = bPinNumber;    //Rotary Encoder Chan B
	r_enc_config.channel = PCNT_CHANNEL_0;
	r_enc_config.lctrl_mode = PCNT_MODE_KEEP;    // Rising A on HIGH B = CW Step
	r_enc_config.hctrl_mode = PCNT_MODE_REVERSE; // Rising A on LOW B = CCW Step
	r_enc_config.counter_h_lim = param;
	r_enc_config.counter_l_lim = -param ;

	pcnt_unit_config(&r_enc_config);//set up first channel 

// set up second channel for full quad
	r_enc_config.pulse_gpio_num = bPinNumber; //make prior control into signal
	r_enc_config.ctrl_gpio_num = aPinNumber;    //and prior signal into control
	r_enc_config.channel = PCNT_CHANNEL_1; // channel 1
//	r_enc_config.pos_mode = PCNT_COUNT_DEC; //Count Only On Rising-Edges
//	r_enc_config.neg_mode = PCNT_COUNT_INC;   // Discard Falling-Edge
	r_enc_config.lctrl_mode = PCNT_MODE_REVERSE;    // prior high mode is now low
	r_enc_config.hctrl_mode = PCNT_MODE_KEEP; // prior low mode is now high
	r_enc_config.counter_h_lim = param;
	r_enc_config.counter_l_lim = -param ;

	pcnt_unit_config(&r_enc_config);//set up second channel for full quad

	// Filter out bounces and noise
	pcnt_set_filter_value(unit, 250);  // Filter Runt Pulses
	pcnt_filter_enable(unit);


	// Enable events on  maximum and minimum limit values 
	pcnt_event_enable(unit, PCNT_EVT_H_LIM);
	pcnt_event_enable(unit, PCNT_EVT_L_LIM);

	pcnt_counter_pause(unit); // Initial PCNT init
	pcnt_counter_clear(unit);

	pcnt_intr_enable(unit);

	ESP32Encoder::clearCount();
//	ESP32Encoder::resume();
	}

}; 

void ESP32Encoder::refresh(void)
{
int test=0;	
}

int64_t ESP32Encoder::setEdgeCounting(int param){//QuadDecoder
int test = param;	
return test;	
};

void ESP32Encoder::setPrescaleFactor(uint32_t param){//QuadDecoder
uint32_t test = param;	

}; //normal for encoder to have the lowest or no prescaler. 
int64_t ESP32Encoder::setMode(int param1,int param2){//QuadDecoder
int test = param1;	
return test;	
}; //set mode, the channel is not used when in this mode.

uint8_t ESP32Encoder::getDirection(){//QuadDecoder
uint8_t test = true;
return test;	
};



void ESP32Encoder::setCount(int64_t value) {
	count = value - getCountRaw();
}
int64_t ESP32Encoder::getCountRaw() {
	int16_t c;
	pcnt_get_counter_value(unit, &c);
	//Serial.println("ESP32encoder getCountRaw");
	//Serial.println(unit,DEC);
	//Serial.println(c,DEC);
	
	return c;
}


int64_t ESP32Encoder::getCount() {
	return getCountRaw() + count;
}

int64_t ESP32Encoder::clearCount() {
	count = 0;
	//Serial.print("ESP32encoder clearCount ");
    //Serial.println(unit ,DEC);

	return pcnt_counter_clear(unit);
}


int64_t ESP32Encoder::pause() {
//Serial.print("ESP32encoder pause ");
//Serial.println(unit ,DEC);
	if (working = true)
	 {
	 pcnt_counter_pause(unit);
	 working = false ;
	 //Serial.println("ESP32encoder  working = false");
	 }
return (working);
}



int64_t ESP32Encoder::pauseCount() {
//Serial.print("ESP32encoder pauseCount ");
//Serial.println(unit ,DEC);
	if (working = true)
	 {
	 pcnt_counter_pause(unit);
	 working = false ;
	 //Serial.println("ESP32encoder  working = false");
	 }
return (working);
}

int64_t ESP32Encoder::resumeCount() {
//Serial.print("ESP32encoder resumeCount ");
//Serial.println(unit ,DEC);
//	if (working = false )
//	 {
	 pcnt_counter_resume(unit);
	 working = true;
	 //Serial.println("ESP32encoder  working = true");
//	 }
return (working);
}

void ESP32Encoder::resume(void){//QuadDecoder start the encoder... 
//Serial.print("ESP32encoder resume ");
//Serial.println(unit ,DEC);
//	if (working = false )
//	 {
	 pcnt_counter_resume(unit);
	 working = true;
	 //Serial.println("ESP32encoder  working = true");
//	 }
};

	