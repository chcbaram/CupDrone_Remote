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
//    파일명     	: MSP_Cmd.ino
//----------------------------------------------------------------------------




#include <Arduino.h> 

#include "MSP.h"











/*---------------------------------------------------------------------------
     TITLE   : MSP_Cmd
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
MSP::MSP()
{
	Cmd 	   = 0x00;
	ArmMode    = false;
	CmdRoll    = 0;
	CmdPitch   = 0;
	CmdYaw     = 0;;
	CmdThrotle = 0;

    bConnected = false;
}





/*---------------------------------------------------------------------------
     TITLE   : begin
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
void MSP::begin( uint8_t uart_num )
{
	MspCmd.begin( uart_num );	
}





/*---------------------------------------------------------------------------
     TITLE   : begin
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
void MSP::begin( void )
{
    MspCmd.begin();   
}




/*---------------------------------------------------------------------------
     TITLE   : update
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
bool MSP::update( void )
{
    bool Ready_Cmd = false;


    Ready_Cmd = MspCmd.update();


    //-- 명령어 처리 
    //
    if( Ready_Cmd == true )
    {
        pCmd  = MspCmd.Get_CmdPtr();
        pResp = MspCmd.Get_RespPtr();
        
        switch( pCmd->Cmd )
        {
            case MSP_SET_RAW_RC_TINY:
    			CmdRoll     = 1000 + pCmd->Data[0] * 4 - 1500;
    			CmdPitch    = 1000 + pCmd->Data[1] * 4 - 1500;
    			CmdYaw      = 1000 + pCmd->Data[2] * 4 - 1500;
    			CmdThrotle  = 1000 + pCmd->Data[3] * 4 - 1000;
                break;

            case MSP_ARM:
                ArmMode = true;
                break;

            case MSP_DISARM:
                ArmMode = false;
                break;

            default:
                pResp->Cmd       = pCmd->Cmd;
                pResp->ErrorCode = 0x00;
                pResp->Length    = 0;
                MspCmd.SendResp( pResp );
                break;
        }
        
        Cmd = pCmd->Cmd;
    }

    if( ArmMode == false )
    {
    	CmdRoll     = 0;
    	CmdPitch    = 0;
    	CmdYaw      = 0;
    	CmdThrotle  = 0;
    }

	return Ready_Cmd;
}





/*---------------------------------------------------------------------------
     TITLE   : SendCmd_ARM
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
bool MSP::SendCmd_ARM( void )
{
    /*
    uint8_t Cmd;
    uint8_t Length;
    uint8_t CheckSum;
    uint8_t Data[MSP_CMD_MAX_LENGTH];
    */

    SendCmd.Cmd    = MSP_ARM;
    SendCmd.Length = 0;
    MspCmd.SendCmd( &SendCmd );

    return true;
}





/*---------------------------------------------------------------------------
     TITLE   : SendCmd_DISARM
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
bool MSP::SendCmd_DISARM( void )
{
    SendCmd.Cmd    = MSP_DISARM;
    SendCmd.Length = 0;
    MspCmd.SendCmd( &SendCmd );

    return true;
}





/*---------------------------------------------------------------------------
     TITLE   : SendCmd_RC
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
bool MSP::SendCmd_RC( int16_t R, int16_t P, int16_t Y, int16_t T )
{
    SendCmd.Cmd    = MSP_SET_RAW_RC_TINY;
    SendCmd.Length = 5;

    R = constrain( R, -500, 500 );
    P = constrain( P, -500, 500 );
    Y = constrain( Y, -500, 500 );
    T = constrain( T,    0, 1000);

    SendCmd.Data[0] = (R+500)/4;
    SendCmd.Data[1] = (P+500)/4;
    SendCmd.Data[2] = (Y+500)/4;
    SendCmd.Data[3] = (T+  0)/4;
    
    SendCmd.Data[4] = 0;

    MspCmd.SendCmd( &SendCmd );


    return true;
}





/*---------------------------------------------------------------------------
     TITLE   : SendCmd_TRIM_UP
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
bool MSP::SendCmd_TRIM_UP( void )
{
    SendCmd.Cmd    = MSP_TRIM_UP;
    SendCmd.Length = 0;
    MspCmd.SendCmd( &SendCmd );

    return true;
}





/*---------------------------------------------------------------------------
     TITLE   : SendCmd_TRIM_DOWN
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
bool MSP::SendCmd_TRIM_DOWN( void )
{
    SendCmd.Cmd    = MSP_TRIM_DOWN;
    SendCmd.Length = 0;
    MspCmd.SendCmd( &SendCmd );

    return true;
}




/*---------------------------------------------------------------------------
     TITLE   : SendCmd_TRIM_LEFT
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
bool MSP::SendCmd_TRIM_LEFT( void )
{
    SendCmd.Cmd    = MSP_TRIM_LEFT;
    SendCmd.Length = 0;
    MspCmd.SendCmd( &SendCmd );

    return true;
}




/*---------------------------------------------------------------------------
     TITLE   : SendCmd_TRIM_RIGHT
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
bool MSP::SendCmd_TRIM_RIGHT( void )
{
    SendCmd.Cmd    = MSP_TRIM_RIGHT;
    SendCmd.Length = 0;
    MspCmd.SendCmd( &SendCmd );

    return true;
}



