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
//    파일명     	: KEY.h
//----------------------------------------------------------------------------
#ifndef _KEY_H_
#define _KEY_H_

#include <inttypes.h>
#include <Arduino.h> 
#include "JOY.h"



class cKEY
{
public:
	cKEY();

	void begin( int up, int down, int left, int right, int enter );
	void begin( cJOY *p_joypad );
	void set_delay( int trigger, int repeat );
	void update( void );

	bool refresh( uint8_t ch );
	int  get_button( uint8_t ch );

	bool get_up()    { return refresh(0); }
	bool get_down()  { return refresh(1); }
	bool get_left()  { return refresh(2); }
	bool get_right() { return refresh(3); }
	bool get_enter() { return refresh(4); }

public:
	uint8_t  pin_state[5];
	uint8_t  pin_press[5];
	int      pin_num[5];
	uint32_t tTime[5];
	int      trigger_time;
	int      repeat_time;

	cJOY *pJoyPad;

	bool use_button;
	bool use_joypad;
};


#endif