/*
	Cupdrone Remote

	Made by Baram ( chcbaram@paran.com )
*/
#include <EEPROM.h>
#include "MSP_Cmd.h"
#include "MSP.h"
#include "BLE.h"
#include "JOY.h"
#include "U8glib.h"


#define JOY_IN_THRUST   	A0
#define JOY_IN_ROLL			A1
#define JOY_IN_PITCH		A2	


// Button
#define KEY_UP     			2
#define KEY_DOWN   			3
#define KEY_LEFT   			4
#define KEY_RIGHT  			5
#define KEY_ENTER  			9


#define STATE_MAIN_MENU		0
#define STATE_RUN_MENU		1
#define STATE_BLE_LIST		2	
#define STATE_CONNECT		3
#define STATE_REMOTE		4


#define MENU_BLESETUP		0
#define MENU_SCAN			1
#define MENU_CONNECT		2
#define MENU_REMOTE			3
#define MENU_SAVE			4


#define TAKE_OFF_TIME		2000	// ms
#define TAKE_OFF_TRUST		300



uint16_t Roll   = 0;
uint16_t Pitch  = 0;
uint16_t Yaw    = 0;
uint16_t Thrust = 0;

int joy_roll 	= 0;
int joy_pitch 	= 0;
int joy_yaw 	= 0;
int joy_thrust 	= 0;


//U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);	// I2C / TWI 
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0 |  U8G_I2C_OPT_NO_ACK | U8G_I2C_OPT_FAST);

BLE     Ble;
MSP		Msp;
JOY     JoyL;
JOY     JoyR;

uint8_t main_state = 0;


uint8_t  take_off_state = 0;
bool     take_off = false;
uint16_t take_off_time    = TAKE_OFF_TIME;
uint16_t take_off_thrust  = TAKE_OFF_TRUST;


#define MENU_ITEMS 5
char *menu_strings[MENU_ITEMS] = { "BLE Step", "Scan", "Connect", "Remote", "Save" };

uint8_t menu_current = 0;
uint8_t menu_redraw_required = 0;

uint32_t tTime_Msp;  







void setup() 
{
	int ret;

	//-- 사용되는 버튼 설정
	//
	pinMode(KEY_UP, INPUT);           	// set pin to input
	pinMode(KEY_DOWN, INPUT);           // set pin to input
	pinMode(KEY_LEFT, INPUT);         	// set pin to input
	pinMode(KEY_RIGHT, INPUT);         	// set pin to input
	pinMode(KEY_ENTER, INPUT);         	// set pin to input

	digitalWrite(KEY_UP, HIGH);       	// turn on pullup resistors
	digitalWrite(KEY_DOWN, HIGH);       // turn on pullup resistors
	digitalWrite(KEY_LEFT, HIGH);       // turn on pullup resistors
	digitalWrite(KEY_RIGHT, HIGH);     	// turn on pullup resistors
	digitalWrite(KEY_ENTER, HIGH);     	// turn on pullup resistors

	//-- LCD 라이브러리 설정
	//
	u8g.setRot180();
	u8g.setFont(u8g_font_6x13);
	u8g.setFontPosTop();
	u8g.setColorIndex(1);

	Msp.begin(); 
	Ble.begin();

	JoyL.begin(A2, A3, 5);
	JoyR.begin(A0, A1, 5);
	JoyL.set_dir( -1, -1 );
	JoyR.set_dir( -1, -1 );


	menu_redraw_required = 1;


	lcd_draw_str( 0, 0, "Remote For CupDrone" );
	delay(1500);

	if( Ble.load_list() == BLE_OK )
	{
		lcd_draw_str( 0, 0, "Loading...OK" );
	}
	else
	{
		lcd_draw_str( 0, 0, "Loading...No Data" );
	}

	delay(1000);
}





void loop() 
{
	static uint32_t tTime;  
	static uint32_t i = 0;  


	tTime = millis();
	loop_main();
	tTime_Msp = millis()-tTime;
	
	loop_joystick();
	loop_msp();
}





bool key_get_button( int pin )
{
	bool ret = false;


	if( digitalRead(pin) == LOW )
	{
		delay(70);
		if( digitalRead(pin) == LOW )
		{
			while( digitalRead(pin) == LOW );
			ret = true;
		}					
	}

	return ret;
}





void loop_main()
{
	uint8_t err_code;
	char Str[24];
	static uint32_t tTime;  


	switch( main_state )
	{
		case STATE_MAIN_MENU:
			lcd_draw_menu();

			if( key_get_button(KEY_RIGHT) == true )
			{
				menu_current++;	
				menu_current %= MENU_ITEMS;
				menu_redraw_required = 1;
			}
			if( key_get_button(KEY_LEFT) == true )
			{
				menu_current--;	
				menu_current %= MENU_ITEMS;
				menu_redraw_required = 1;
			}
			if( key_get_button(KEY_ENTER) == true )
			{
				main_state = STATE_RUN_MENU;
			}

			take_off_state = 0;
			take_off       = false;
			break;

		case STATE_RUN_MENU:
			main_state = run_menu( menu_current );
			menu_redraw_required = 1;
			break;

		case STATE_BLE_LIST:
			lcd_draw_ble_list();

			if( key_get_button(KEY_RIGHT) == true )
			{
				menu_current++;	
				menu_current %= Ble.BleList.Count;
				menu_redraw_required = 1;
			}
			if( key_get_button(KEY_LEFT) == true )
			{
				menu_current--;	
				menu_current %= Ble.BleList.Count;
				menu_redraw_required = 1;
			}
			if( key_get_button(KEY_ENTER) == true )
			{
				main_state = STATE_CONNECT;
			}		
			if( key_get_button(KEY_UP) == true )
			{
				menu_current = MENU_CONNECT;
				main_state = STATE_MAIN_MENU;
				menu_redraw_required = 1;
			}				
			break;

		case STATE_CONNECT:
			lcd_draw_str( 0, 0, "Connecting..." ); 
			delay(10);
			err_code = Ble.connect(menu_current);
			if( err_code == BLE_OK && Ble.bConnected == true )
			{
				Ble.BleList.strName[menu_current].toCharArray(Str, 24);
				lcd_draw_str( 0, 0, Str); 
				Msp.bConnected = true;
			}
			else
			{
				lcd_draw_str( 0, 0, "Connect fail" );
			}

			delay(1000);
			menu_redraw_required = 1;
			main_state = STATE_MAIN_MENU;
			break;

		case STATE_REMOTE:
			if( (millis()-tTime) > 500 )
			{
				menu_redraw_required = 1;
				tTime = millis();
			}
			lcd_draw_remote();
			
			if( key_get_button(KEY_ENTER) == true )
			{
				main_state = STATE_MAIN_MENU;
				menu_redraw_required = 1;
			}		

			if( key_get_button(KEY_DOWN) == true )
			{
				if( Msp.bConnected == true )
				{
					Msp.SendCmd_ARM();
				}
			}	

			if( key_get_button(KEY_UP) == true )
			{
				if( Msp.bConnected == true )
				{
					Msp.SendCmd_DISARM();
				}
			}	

			loop_take_off();
			break;

		case 0xFF:
			break;
	}
}



void loop_take_off( )
{
	static uint32_t tTime;


		switch( take_off_state )
		{
			case 0:
				if( key_get_button(KEY_RIGHT) == true )
				{
					take_off_state = 1;
				}		

				if( key_get_button(KEY_LEFT) == true )
				{
					if( take_off == true )
					{
						take_off_thrust = Thrust;
					}
				}		

				break;

			case 1:
				if( take_off == false )
				{
					take_off_state = 2;
					tTime    = millis();
					Thrust   = take_off_thrust;					
				}
				else
				{
					take_off = false;
					take_off_state = 0;
				}
				break;

			case 2:
				if( (millis()-tTime) >= take_off_time )
				{
					take_off_state = 0;
					take_off = true;
				}
				break;
		}
}





uint8_t run_menu( int menu_sel )
{
	char Str[24];
	char StrConv[24];
	String strRet;
	uint8_t err_code;
	uint8_t i;
	uint8_t ret_state = STATE_MAIN_MENU;


	switch( menu_sel )
	{
		case MENU_BLESETUP:
			lcd_draw_str( 0, 0, "BLE Setup..." );
			sprintf(Str, "ret : %d", Ble.setup() );
			err_code = Ble.send_cmd("AT", strRet, 100);
			if( err_code == BLE_OK ) lcd_draw_str( 0, 0, "OK" );
			else                     lcd_draw_str( 0, 0, "FAIL" ); 

			//strRet.toCharArray(Str, 20);
			//lcd_draw_str( 0, 1, Str );	

			//delay(1000);
			//sprintf(Str, "ret : %d", err_code);
			//lcd_draw_str( 0, 2, Str );	

			delay(1000);
			Msp.bConnected = false;

			ret_state = STATE_MAIN_MENU;		
			break;

		case MENU_SCAN:
			lcd_draw_str( 0, 0, "Scan..." );
			if( Ble.scan() == BLE_OK &&  Ble.BleList.Count > 0 )
			{
				sprintf(Str, "Found.. %d", Ble.BleList.Count);
				lcd_draw_str( 0, 0, Str);
			}
			else
			{
				lcd_draw_str( 0, 0, "No Drone" );
			}

			delay(1000);
			ret_state = STATE_MAIN_MENU;
			break;

		case MENU_CONNECT:
			if( Ble.BleList.Count > 0 )
			{
				menu_current = 0;
				ret_state = STATE_BLE_LIST;
			}
			else
			{
				lcd_draw_str( 0, 0, "No Drone" );
				delay(1000);
				ret_state = STATE_MAIN_MENU;
			}
			break;

		case MENU_REMOTE:
			//lcd_draw_str( 0, 0, "Remote..." );
			//delay(1000);		
			ret_state = STATE_REMOTE;
			break;

		case MENU_SAVE:
			lcd_draw_str( 0, 0, "Saving..." ); 

			if( Ble.BleList.Count == 0 )
			{
				lcd_draw_str( 0, 0, "Saving..No Data" );
			}
			else
			{	
				Ble.save_list();
				lcd_draw_str( 0, 0, "Saving..OK" );
			}

			delay(1000);		
			break;
	}

	return ret_state;
}





void loop_msp() 
{
	static uint32_t tTime;  
	static uint32_t i = 0;  


	if( Msp.bConnected == true )
	{
		if( (millis()-tTime) > 50 )
		{
			Msp.SendCmd_RC( Roll, Pitch, Yaw, Thrust );
			tTime = millis();
		}
	}
	else
	{
		take_off_state = 0;		
		take_off = false;		
	}
}





void loop_joystick()
{
	static uint32_t tTime;



		if( take_off == false )
		{
			// 0~1000
			joy_thrust = constrain( JoyR.get_pitch(), 0, 500 );
			joy_thrust = map( joy_thrust, 0, 500, 0, 800 );
		}
		else
		{
			if( (millis()-tTime) > 10 )
			{
				tTime = millis();

				#if 0
				joy_thrust = map( JoyR.get_pitch(), -500, 500,  -250, 250 );

				if     ( joy_thrust >  50 ) joy_thrust =  1;			
				else if( joy_thrust < -50 ) joy_thrust = -1;
				else
				{
					joy_thrust = 0;
				}
				Thrust = Thrust + joy_thrust;
				Thrust = constrain( Thrust, 0, 800 );				
				#else
				joy_thrust = map( JoyR.get_pitch(), -500, 500,  -500, 500 );

				joy_thrust = take_off_thrust + joy_thrust;
				Thrust = constrain( joy_thrust, 0, 800 );								
				#endif

			}
		}



		// Left/Right(-500~500)
		joy_roll = map( JoyL.get_roll(), -500, 500,  -100, 100 );

		// Up/Down(500~-500)
		joy_pitch = map( JoyL.get_pitch(), -500, 500, -100, 100 );




		if( Msp.bConnected == true )
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





void lcd_draw_menu( void )
{
	uint8_t i, h;
	u8g_uint_t w, d;
	static uint32_t tTime;

	if( menu_redraw_required == 0 ) return;

	u8g.firstPage();  
  	do 
  	{
		//u8g.setFont(u8g_font_6x13);
		u8g.setFontRefHeightText();
		u8g.setFontPosTop();

		h = u8g.getFontAscent()-u8g.getFontDescent();
		w = u8g.getWidth();
		for( i = 0; i < MENU_ITEMS; i++ ) 
		{
			d = (w-u8g.getStrWidth(menu_strings[i]))/2;
			u8g.setDefaultForegroundColor();
			if ( i == menu_current ) 
			{
				u8g.drawBox(0, i*h+1, w, h);
      			u8g.setDefaultBackgroundColor();
			}
			u8g.drawStr(d, i*h, menu_strings[i]);
		}

  	} while( u8g.nextPage() );

  	u8g.setDefaultForegroundColor();

  	menu_redraw_required = 0;
}





void lcd_draw_ble_list( void )
{
	uint8_t i, h;
	u8g_uint_t w, d;
	static uint32_t tTime;
	String strRet;
	char StrList[32];

	if( menu_redraw_required == 0 ) return;

	u8g.firstPage();  
  	do 
  	{
		u8g.setFontRefHeightText();
		u8g.setFontPosTop();

		h = u8g.getFontAscent()-u8g.getFontDescent();
		w = u8g.getWidth();
		for( i = 0; i < Ble.BleList.Count; i++ ) 
		{
			strRet = Ble.BleList.strName[i] + ":" + Ble.BleList.strAddr[i];
			strRet.toCharArray(StrList, 32);

			d = (w-u8g.getStrWidth(StrList))/2;
			u8g.setDefaultForegroundColor();
			if ( i == menu_current ) 
			{
				u8g.drawBox(0, i*h+1, w, h);
      			u8g.setDefaultBackgroundColor();
			}
			u8g.drawStr(0, i*h, StrList);
		}

  	} while( u8g.nextPage() );

	u8g.setDefaultForegroundColor();

  	menu_redraw_required = 0;
}


void lcd_draw_str( int x, int y, char *pStr )
{
	uint8_t i, h;
	u8g_uint_t w, d;
	

	h = u8g.getFontAscent()-u8g.getFontDescent();
	w = u8g.getWidth();

	u8g.firstPage();  
  	do 
  	{
		u8g.drawStr( w*x, y*h+1, pStr);
  	} while( u8g.nextPage() );
}



void lcd_draw_remote( void )
{
	uint8_t i, h;
	u8g_uint_t w, d;
	static uint32_t tTime;
	String strRet;
	char Str[32];


	if( menu_redraw_required == 0 ) return;

	h = u8g.getFontAscent()-u8g.getFontDescent();
	w = u8g.getWidth();

	u8g.firstPage();  
  	do 
  	{
		sprintf(Str, "L : %d, %d", JoyL.get_roll(), JoyL.get_pitch());
		u8g.drawStr( 0*w, 0*h+1, Str);

		//sprintf(Str, "LP : %d", JoyL.get_pitch());
		//u8g.drawStr( 0*w, 1*h+1, Str);

		sprintf(Str, "R : %d, %d", JoyR.get_roll(), JoyR.get_pitch());
		u8g.drawStr( 0*w, 1*h+1, Str);

		//sprintf(Str, "RP : %d", JoyR.get_pitch());
		//u8g.drawStr( 0*w, 3*h+1, Str);
		sprintf(Str, "RP : %d, %d", joy_roll, joy_pitch);
		u8g.drawStr( 0*w, 2*h+1, Str);
		sprintf(Str, "TR : %d", Thrust);
		u8g.drawStr( 0*w, 3*h+1, Str);


		sprintf(Str, "B:%d, C:%d, T: %d", Ble.bConnected, Msp.bConnected, take_off );
		u8g.drawStr( 0*w, 4*h+1, Str);

	

  	} while( u8g.nextPage() );

  	menu_redraw_required = 0;
}

