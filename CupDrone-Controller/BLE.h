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
#ifndef _BLE_H_
#define _BLE_H_

#include <inttypes.h>
#include <Arduino.h> 
#include "HardwareSerial.h"


#define BLE_SERIAL		Serial1



#define BLE_OK				0x00
#define BLE_ERR_TIMEOUT		0x01
#define BLE_ERR_NOACK		0x02
#define BLE_ERR_NO_CH		0x10
#define BLE_ERR_NO_DATA		0x20



#define BLE_SAVE_FLAG		0x5555AAAA



typedef struct 
{
	int      Count;
	String strName[5];
	String strAddr[5];	
	uint8_t JoyMode;		
} BLE_LIST;

typedef struct 
{
	int      Count;
	uint32_t SaveFlag;
	char strName[5][16];
	char strAddr[5][16];
	uint8_t JoyMode;	
} BLE_LIST_EEPROM;



class cBLE
{
public:
	cBLE();

	void begin( uint8_t uart_num );
	void begin( void );

	uint8_t check( int Baud );
	uint8_t setup( void );
	uint8_t scan( void );
	uint8_t send_cmd( String str, String &strRet, uint32_t TimeOut );
	uint8_t send_scan( uint32_t TimeOut );
	uint8_t connect( uint8_t ch );

	uint8_t save_list( void );
	uint8_t load_list( void );


public:
	bool bConnected;

	BLE_LIST 		BleList;
	BLE_LIST_EEPROM BleListEEPROM;

private:

  HardwareSerial *pSerial;
};


#endif
