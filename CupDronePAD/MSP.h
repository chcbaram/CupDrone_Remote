
//----------------------------------------------------------------------------
//    프로그램명 	: MSP 
//
//    만든이     	: Cho Han Cheol 
//
//    날  짜     	: 
//    
//    최종 수정  	: 
//
//    MPU_Type		: 
//
//    파일명     	: MSP_Cmd.h
//----------------------------------------------------------------------------
#ifndef _MSP_H_
#define _MSP_H_

#include <inttypes.h>
#include <Arduino.h> 


#include "MSP_Cmd.h"





class MSP
{
public:
	MSP();

	bool bConnected;

	void begin( uint8_t uart_num );
	void begin( void );
	bool update( void );

	bool    Get_ArmMode( void ) { return ArmMode; };
	uint8_t Get_Cmd( void ) 	{ return Cmd; };
	int16_t Get_Roll( void ) 	{ return CmdRoll; };
	int16_t Get_Pitch( void ) 	{ return CmdPitch; };
	int16_t Get_Yaw( void )		{ return CmdYaw; };
	int16_t Get_Throtle( void ) { return CmdThrotle; };


	bool SendCmd_ARM( void );
	bool SendCmd_DISARM( void );
	bool SendCmd_RC( int16_t R, int16_t P, int16_t Y, int16_t T );

private:
	MSP_Cmd MspCmd;

	MSP_CMD_OBJ     SendCmd;
	MSP_CMD_OBJ    *pCmd;
	MSP_RESP_OBJ   *pResp;	


	uint8_t	Cmd;
	bool    ArmMode;
	int16_t CmdRoll;
	int16_t CmdPitch;
	int16_t CmdYaw;
	int16_t CmdThrotle;

};


#endif