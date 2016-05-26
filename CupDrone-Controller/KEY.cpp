//----------------------------------------------------------------------------
//    프로그램명 	: KEY
//
//    만든이     	: Cho Han Cheol 
//
//    날  짜     : 
//    
//    최종 수정  	: 
//
//    MPU_Type	: 
//
//    파일명     	: KEY.cpp
//----------------------------------------------------------------------------




#include <Arduino.h> 

#include "KEY.h"







/*---------------------------------------------------------------------------
     TITLE   : cKEY
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
cKEY::cKEY()
{
	uint8_t i;


	use_button = false;
	use_joypad = false;

	for( i=0; i<5; i++ )
	{
		pin_state[i] = 0;	
		pin_press[i] = 0;	
	}	

	pJoyPad = NULL;
}


/*---------------------------------------------------------------------------
     TITLE   : begin
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
void cKEY::begin( int up, int down, int left, int right, int enter )
{
	pinMode(up,    INPUT);          	// set pin to input
	pinMode(down,  INPUT);           	// set pin to input
	pinMode(left,  INPUT);         		// set pin to input
	pinMode(right, INPUT);         		// set pin to input
	pinMode(enter, INPUT);         		// set pin to input

	digitalWrite(up,    HIGH);       	// turn on pullup resistors
	digitalWrite(down,  HIGH);       	// turn on pullup resistors
	digitalWrite(left,  HIGH);       	// turn on pullup resistors
	digitalWrite(right, HIGH);     		// turn on pullup resistors
	digitalWrite(enter, HIGH);     		// turn on pullup resistors

	pin_num[0] = up;
	pin_num[1] = down;
	pin_num[2] = left;
	pin_num[3] = right;
	pin_num[4] = enter;


	use_button = true;
}


/*---------------------------------------------------------------------------
     TITLE   : begin
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
void cKEY::begin( cJOY *p_joypad )
{
	pJoyPad = p_joypad;
	use_joypad = true;
}


/*---------------------------------------------------------------------------
     TITLE   : set_delay
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
void cKEY::set_delay( int trigger, int repeat )
{
	trigger_time = trigger;
	repeat_time  = repeat;
}


/*---------------------------------------------------------------------------
     TITLE   : update
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
void cKEY::update( void )
{
	uint8_t i;

	for( i=0; i<5; i++ )
	{
		switch( pin_state[i] )
		{
			case 0:				
				pin_press[i] = 0;

				if( get_button(i) == LOW )
				{
					tTime[i] = millis();
					pin_state[i] = 1;
				}
				break;

			case 1:
				if( get_button(i) == HIGH ) 
				{
					pin_state[i] = 0;
				}
				if( millis()-tTime[i] > trigger_time )
				{
					pin_press[i] = 1;
					pin_state[i] = 2;	
				}
				break;

			case 2:				
				if( get_button(i) == HIGH ) 
				{
					pin_state[i] = 0;
				}
				if( pin_press[i] == 0 )
				{
					tTime[i] = millis();
					pin_state[i] = 3;
				}			
				break;

			case 3:
				if( millis()-tTime[i] > trigger_time )
				{
					pin_state[i] = 0;	
				}				
				break;
		}		
	}
}


/*---------------------------------------------------------------------------
     TITLE   : refresh
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
bool cKEY::refresh( uint8_t ch )
{
	bool ret; 

	ret = (bool)pin_press[ch];

	if( ret == 1 )
	{
		pin_press[ch] = 0;		
	}

	return ret;
}


int cKEY::get_button( uint8_t ch )
{
	int ret = HIGH;
	int joy_value;


	if( use_button == true )
	{
		ret = digitalRead(pin_num[ch]);
	}


	if( use_joypad == true && pJoyPad != NULL )
	{
		joy_value = pJoyPad->get_pitch() * pJoyPad->dir_pitch;

		if( ch == 0 && joy_value < -100) ret = LOW;
		if( ch == 1 && joy_value >  100) ret = LOW;


		joy_value = pJoyPad->get_roll() * pJoyPad->dir_roll;

		if( ch == 2 && joy_value >  100) ret = LOW;
		if( ch == 3 && joy_value < -100) ret = LOW;

	}

	return ret;
}