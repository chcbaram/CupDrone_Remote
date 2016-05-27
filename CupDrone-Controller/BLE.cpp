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
//    파일명     	: BLE.ino
//----------------------------------------------------------------------------




#include <Arduino.h> 
#include <EEPROM.h>
#include "BLE.h"






/*---------------------------------------------------------------------------
     TITLE   : BLE
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
cBLE::cBLE()
{
	bConnected = false;
}




/*---------------------------------------------------------------------------
     TITLE   : begin
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
void cBLE::begin(uint8_t uart_num )
{
	//BLE_SERIAL.begin(115200);	

  //-- 통신 포트 초기화  
  //
  switch( uart_num )
  {
    case 1:
      pSerial = (HardwareSerial *)&Serial;
      Serial.begin(115200); 
      break;

    case 2:
      pSerial = (HardwareSerial *)&Serial1;
      Serial1.begin(115200);  
      break;

    default:
      pSerial = (HardwareSerial *)&Serial;
      Serial.begin(115200);     
      break;
  }    
}


/*---------------------------------------------------------------------------
     TITLE   : begin
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
void cBLE::begin( void )
{
	//BLE_SERIAL.begin(Baud);
  pSerial = (HardwareSerial *)&Serial1;
  Serial1.begin(115200); 		
}


/*---------------------------------------------------------------------------
     TITLE   : setup
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
uint8_t cBLE::setup( void )
{
	uint8_t err_code;
	String strRet;


	BLE_SERIAL.begin(115200);
	delay(10);
	err_code = send_cmd("AT", strRet, 100);

	if( err_code != BLE_OK )
	{
		BLE_SERIAL.begin(9600);
		delay(10);
		err_code = send_cmd("AT", strRet, 100);

		if( err_code == BLE_OK )
		{
			err_code = send_cmd("AT+BAUD4", strRet, 100);
			BLE_SERIAL.begin(115200);
		}
	}

	return err_code;
}


/*---------------------------------------------------------------------------
     TITLE   : scan
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
uint8_t cBLE::scan( void )
{
	uint8_t err_code = BLE_OK;
	uint32_t tTime;
	String strLine;
	String strRet;
	
	err_code |= send_cmd("AT", strRet, 100);
	err_code |= send_cmd("AT+SHOW1", strRet, 100);	
	err_code |= send_cmd("AT+NAME?", strRet, 100);
	err_code |= send_cmd("AT+ROLE1", strRet, 100); // Master 모드로 변경 
	err_code |= send_cmd("AT+ROLE?", strRet, 100);
	err_code |= send_cmd("AT+IMME1", strRet, 100);

	if( err_code != BLE_OK ) return err_code;

	
	return send_scan( 5000 );
}


/*---------------------------------------------------------------------------
     TITLE   : send_cmd
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
uint8_t cBLE::send_cmd( String str, String &strRet, uint32_t TimeOut )
{
	uint32_t tTime;
	char ch;
	uint8_t err_code = BLE_OK;


	BLE_SERIAL.print(str);

	strRet = "";

	tTime = millis();
	while(1)
	{
		if( (millis()-tTime) >= TimeOut )
		{
			break;
		}

		if( BLE_SERIAL.available() )
		{
			ch = BLE_SERIAL.read();

			strRet += (char)ch;
			tTime = millis();
		}
	} 

	if( strRet.indexOf("OK", 0) < 0 )
	{
		err_code = BLE_ERR_NOACK;
	}
	
	return err_code;
}


/*---------------------------------------------------------------------------
     TITLE   : send_scan
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
uint8_t cBLE::send_scan( uint32_t TimeOut )
{
	uint8_t err_code = BLE_OK;
	String strRet;
	int Index;
	int IndexEnd;
	int i;

	delay(1000);
	err_code = send_cmd("AT+DISC?", strRet, TimeOut);
	if( err_code != BLE_OK ) return err_code;


	BleList.Count = 0;
	Index = 0;

	for( i=0; i<5; i++ )
	{
		Index = strRet.indexOf("DIS0", Index);		
		if( Index < 0 )
		{			
			break;
		} 
		BleList.Count = i+1;

		// MAC 주소 읽기 
		BleList.strAddr[i] = strRet.substring(Index+5, Index+5+12);

		// Name 읽기 
		Index    = strRet.indexOf("NAME:", Index+1);		
		IndexEnd = strRet.indexOf("\n", Index+1);		
		BleList.strName[i] = strRet.substring(Index+5, IndexEnd);

		//Serial.println(BleList.strAddr[i]);
		//Serial.println(BleList.strName[i]);

		/*
		Index = strRet.indexOf("DIS0");
		Serial.println(strRet);
		Serial.println( Index );

		String str_1st = strRet.substring(Index+5, Index+5+12);
		Serial.println(str_1st);

		Index = strRet.indexOf("DIS0", Index+1);
		Serial.println( Index );

		String str_2nd = strRet.substring(Index+5, Index+5+12);
		Serial.println(str_2nd);

		Index = strRet.indexOf("DIS0", Index+1);
		Serial.println( Index );
		*/
	}

	return err_code;
}


/*---------------------------------------------------------------------------
     TITLE   : connect
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
uint8_t cBLE::connect( uint8_t ch )
{
	uint8_t err_code = BLE_OK;
	String strRet;


	if( ch < BleList.Count )
	{
		send_cmd("AT+NOTI1", strRet, 100);
		err_code = send_cmd("AT+CON"+BleList.strAddr[ch], strRet, 2000);

		if( err_code == BLE_OK )
		{				
			bConnected = true;			
		}
		else
		{
			bConnected = false;
		}
	}
	else
	{
		err_code = BLE_ERR_NO_CH;
	}

	return err_code;
}


/*---------------------------------------------------------------------------
     TITLE   : save_list
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
uint8_t cBLE::save_list( void )
{
	int i;
	uint8_t *p;

	BleListEEPROM.Count    = BleList.Count;
	BleListEEPROM.SaveFlag = BLE_SAVE_FLAG;
	BleListEEPROM.JoyMode  = BleList.JoyMode;
	
	//EEPROM.put(0, BleList);

	for( i=0; i<BleList.Count; i++ )
	{
		BleList.strName[i].toCharArray(BleListEEPROM.strName[i], 16);
		BleList.strAddr[i].toCharArray(BleListEEPROM.strAddr[i], 16);
	}

	p = (uint8_t *)&BleListEEPROM;
	for( i=0; i<sizeof(BleListEEPROM); i++ )
	{
		EEPROM.write(i, p[i]);
	}

	return BLE_OK;
}


/*---------------------------------------------------------------------------
     TITLE   : load_list
     WORK    : 
     ARG     : void
     RET     : void
---------------------------------------------------------------------------*/
uint8_t cBLE::load_list( void )
{
	uint8_t err_code = BLE_OK;
	int i;
	uint8_t *p;

	//EEPROM.get(0, BleList);

	p = (uint8_t *)&BleListEEPROM;
	for( i=0; i<sizeof(BleListEEPROM); i++ )
	{
		p[i] = EEPROM.read(i);
	}

	if( BleListEEPROM.SaveFlag != BLE_SAVE_FLAG )
	{
		err_code = BLE_ERR_NO_DATA;
	}
	else if( BleListEEPROM.Count == 0 || BleListEEPROM.Count > 5 )
	{
		err_code = BLE_ERR_NO_DATA;
	}
	else
	{
		BleList.Count   = BleListEEPROM.Count;
		BleList.JoyMode = BleListEEPROM.JoyMode; 
		for( i=0; i<BleList.Count; i++ )
		{
			BleList.strName[i] = String(BleListEEPROM.strName[i]);
			BleList.strAddr[i] = String(BleListEEPROM.strAddr[i]);			
		}
	}

	return err_code;
}



