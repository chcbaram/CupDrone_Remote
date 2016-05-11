//----------------------------------------------------------------------------
//    프로그램명 	: 
//
//    만든이     	: Cho Han Cheol 
//
//    날  짜     : 
//    
//    최종 수정  	: 
//
//    MPU_Type	: 
//
//    파일명     	: JOY.cpp
//----------------------------------------------------------------------------




#include <Arduino.h> 

#include "JOY.h"



#define OFFSET_GAP		10



/*---------------------------------------------------------------------------
     TITLE   : JOY
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
JOY::JOY()
{
	dir_roll  = 1;
	dir_pitch = 1;
}


/*---------------------------------------------------------------------------
     TITLE   : begin
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
void JOY::begin( int roll, int pitch, int button )
{
	int i;
	int sum;

	pin_roll 	= roll;
	pin_pitch	= pitch;
	pin_button	= button;

	pinMode( pin_button, INPUT );	

	sum = 0;
	offset_roll = 0;
	for( i=0; i<10; i++ )
	{
		sum += get_roll();
	}
	offset_roll = sum / 10;

	sum = 0;
	offset_pitch = 0;
	for( i=0; i<10; i++ )
	{
		sum += get_pitch();
	}
	offset_pitch = sum / 10;	
}


/*---------------------------------------------------------------------------
     TITLE   : get_roll
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
int JOY::get_roll()
{
	int ret;


	ret = constrain( analogRead(pin_roll), 0, 1000 );
	ret = ret - offset_roll;

	if( abs(ret) < OFFSET_GAP ) ret = 0;

	return ret*dir_roll;
}


/*---------------------------------------------------------------------------
     TITLE   : get_pitch
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
int JOY::get_pitch()
{
	int ret;


	ret = constrain( analogRead(pin_pitch), 0, 1000 );
	ret = ret - offset_pitch;

	if( abs(ret) < OFFSET_GAP ) ret = 0;

	return ret*dir_pitch;
}


/*---------------------------------------------------------------------------
     TITLE   : set_dir
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
void JOY::set_dir( int roll, int pitch )
{
	if( roll >= 0 )  dir_roll =  1;
	else             dir_roll = -1;

	if( pitch >= 0 ) dir_pitch =  1;
	else             dir_pitch = -1;
}


/*---------------------------------------------------------------------------
     TITLE   : get_button
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
bool JOY::get_button()
{
	bool ret;


	ret = digitalRead(pin_button);

	return ret;
}