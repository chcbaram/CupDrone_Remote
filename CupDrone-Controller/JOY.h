//----------------------------------------------------------------------------
//    프로그램명 	: BLE
//
//    만든이     	: Cho Han Cheol 
//
//    날  짜     : 
//    
//    최종 수정  	: 
//
//    MPU_Type	: 
//
//    파일명     	: BLE.h
//----------------------------------------------------------------------------
#ifndef _JOY_H_
#define _JOY_H_

#include <inttypes.h>
#include <Arduino.h> 




class cJOY
{
public:
	cJOY();

	void begin( int Roll, int Pitch, int button );

	int get_roll();
	int get_pitch();
	bool get_button();

	void set_dir( int roll, int pitch );
public:
	int pin_roll;
	int pin_pitch;
	int pin_button;

	int offset_roll;
	int offset_pitch;
	int dir_roll;
	int dir_pitch;
};


#endif