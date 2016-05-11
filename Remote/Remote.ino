/*
	Cupdrone Remote

	Made by Baram
*/

#include "MSP_Cmd.h"
#include "MSP.h"



#define JOY_IN_THRUST   A0
#define JOY_IN_ROLL		A1
#define JOY_IN_PITCH	A2	
#define JOY_IN_TAKE_OFF	2





MSP		Remote;

typedef struct 
{
	int    Count;
	String strName[5];
	String strAddr[5];	
} BLE_LIST;

BLE_LIST BleList;


uint8_t take_off_state = 0;
bool    take_off = false;

uint16_t Roll   = 0;
uint16_t Pitch  = 0;
uint16_t Yaw    = 0;
uint16_t Thrust = 0;



void setup() 
{
	Serial.begin(115200);
	Remote.begin(); 
	pinMode( JOY_IN_TAKE_OFF, INPUT );

	//Remote.bConnected = true;
}





void loop() 
{
	static uint32_t tTime;  
	static uint32_t i = 0;  


	/*
	if( Remote.update() == true )
	{		
		Serial.print( Remote.Get_Roll() ); 		Serial.print( "\t" );
		Serial.print( Remote.Get_Pitch() ); 	Serial.print( "\t" );
		Serial.print( Remote.Get_Yaw() ); 		Serial.print( "\t" );
		Serial.print( Remote.Get_Throtle() ); 	Serial.print( "\t" );
		Serial.println(" ");
	}
	*/

	menu_loop();
	joystick_loop();


	if( Remote.bConnected == true )
	{
		if( (millis()-tTime) > 50 )
		{
			Remote.SendCmd_RC( Roll, Pitch, Yaw, Thrust );
			tTime = millis();
			/*
			Serial.print(i++);
			Serial.print(" ");
			Serial.print(take_off);
			Serial.print(" ");
			Serial.print(Thrust);
			Serial.print(" ");
			Serial.println(" Send Cmd");
			*/
		}
	}
	else
	{
		take_off_state = 0;		
		take_off = false;
	}

}


void joystick_loop()
{
	static uint32_t tTime[4];
	static uint16_t tDelay;
	int joy_roll;
	int joy_pitch;
	int joy_yaw;
	int joy_thrust;


	if( (millis()-tTime[0]) > 10 )
	{
		tTime[0] = millis();


		if( take_off == false )
		{
			// 0~1000
			joy_thrust = constrain( analogRead(JOY_IN_THRUST), 0, 500 );
			joy_thrust = map( joy_thrust, 0, 500, 800, 0 );
			//Serial.println( joy_thrust );
		}
		else
		{
			joy_thrust = constrain( analogRead(JOY_IN_THRUST), 0, 1000 );
			joy_thrust = joy_thrust - 500;
			joy_thrust = map( joy_thrust, -500, 500,  250, -250 );

			if     ( joy_thrust >  100 ) joy_thrust =  1;			
			else if( joy_thrust < -100 ) joy_thrust = -1;
			else
			{
				joy_thrust = 0;
			}

			Thrust = constrain( Thrust+joy_thrust, 0, 800 );
		}


		// Left/Right(-500~500)
		joy_roll = constrain( analogRead(JOY_IN_ROLL), 0, 1000 );
		joy_roll = joy_roll - 490;
		joy_roll = map( joy_roll, -500, 500,  250, -250 );
		//Serial.println( analogRead(A1) );
		//Serial.println( joy_roll );


		// Up/Down(500~-500)
		joy_pitch = constrain( analogRead(JOY_IN_PITCH), 0, 1000 );
		joy_pitch = joy_pitch - 536;
		joy_pitch = map( joy_pitch, -500, 500, 250, -250 );
		//Serial.println( joy_pitch );		

/*
		Serial.print( joy_roll );
		Serial.print( "\t" );
		Serial.print( joy_pitch );
		Serial.print( "\t" );
		Serial.println( joy_thrust );
*/
		if( Remote.bConnected == true )
		{
			Roll   = joy_roll;
			Pitch  = joy_pitch;

			if( take_off == false && take_off_state == 0 ) 
			{
				Thrust = joy_thrust;			
			}
		}
		else
		{
			Thrust = 0;
		}
	}	


	if( (millis()-tTime[1]) > 100 )
	{
		tTime[1] = millis();

		switch( take_off_state )
		{
			case 0:
				if( digitalRead(JOY_IN_TAKE_OFF) == 0 )
				{
					take_off_state = 1;
				}
				break;

			case 1:
				if( digitalRead(JOY_IN_TAKE_OFF) == 1 )
				{
					if( take_off == false )
					{
						take_off_state = 2;
						tDelay   = 0;
						Thrust   = 300;					
					}
					else
					{
						take_off = false;
						take_off_state = 0;
					}
				}
				else
				{
					take_off_state = 0;
				}
				break;

			case 2:
				tDelay++;

				if( tDelay > (2000/100) )
				{
					take_off_state = 0;
					take_off = true;
				}
				break;
		}

	}



}


void menu_loop()
{
	uint8_t ch;

	//-- 명령어 수신
	//
	if( Serial.available() )
	{
		ch = Serial.read();

		switch( ch )
		{
			case '1':
				do_ble_scan();
				break;

			case '2':
				do_ble_connect();
				break;

			case '3':
				Remote.SendCmd_ARM();
				break;

			case '4':
				Remote.SendCmd_DISARM();
				break;

			case '5':
				Serial.println("Disconnect");				
				Remote.bConnected = false;
				break;

			case 'q':
				Thrust = 200;				
				break;

			case 'a':
				Thrust = 0;
				break;

			case 'm':
				menu_show_list();
				break;
		}
	}
}


void menu_show_list()
{
	Serial.println("1. BLE Scan");
	Serial.println("2. BLE Connect");
	Serial.println("m. menu");
}


void do_ble_scan( void )
{
	uint32_t tTime;
	String strLine;

	
	Serial.println(ble_cmd("AT", 100));
	Serial.println(ble_cmd("AT+SHOW1", 100));	
	Serial.println(ble_cmd("AT+NAME?", 100));
	Serial.println(ble_cmd("AT+ROLE1", 100)); // Master 모드로 변경 
	Serial.println(ble_cmd("AT+ROLE?", 100));
	//Serial.println(ble_cmd("AT+IMME1", 100));

	ble_scan( 5000 );

	for( int i=0; i<BleList.Count; i++ )
	{
		Serial.print(i);
		Serial.print(" ");
		Serial.print(BleList.strAddr[i]);
		Serial.print(" ");
		Serial.println(BleList.strName[i]);
	}

}


String ble_cmd( String str, uint32_t TimeOut )
{
	uint32_t tTime;
	String strRet;
	char ch;


	Serial1.print(str);

	strRet = "";

	tTime = millis();
	while( (millis()-tTime) < TimeOut )
	{
		if( Serial1.available() )
		{
			ch = Serial1.read();

			strRet += (char)ch;
			//Serial.print((char)ch);
			tTime = millis();
		}
	} 

	return strRet;
}





int ble_scan( uint32_t TimeOut )
{
	String strRet;
	int Index;
	int IndexEnd;
	int i;

	delay(1000);
	strRet = ble_cmd("AT+DISC?", TimeOut);

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

	return BleList.Count;
}


void do_ble_connect( void )
{
	char ch;


	Serial.println("Input Module Index");
	while(1)
	{
		//-- 명령어 수신
		//
		if( Serial.available() )
		{
			ch = Serial.read();

			Serial.println(ch);


			if( (ch-'0') < BleList.Count )
			{
				Serial.println(ch-'0');

				ble_cmd("AT+NOTI1", 100);
				Serial.println(ble_cmd("AT+CON"+BleList.strAddr[ch-'0'], 2000));

				
				Remote.bConnected = true;
				break;
			}
			else
			{
				Serial.println("No Module");
				break;
			}
			
		}
	}
	Serial.println("Exit");
}





