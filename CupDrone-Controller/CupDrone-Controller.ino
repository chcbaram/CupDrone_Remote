/*
	Cupdrone Remote

	Made by Baram ( chcbaram@paran.com )

        www.steammaker.co.kr 
*/

#include <EEPROM.h>
#include "MSP_Cmd.h"
#include "MSP.h"
#include "BLE.h"
#include "JOY.h"
#include "KEY.h"
#include "U8glib.h"



// 왼좌우 :  A2   / 왼위아래 :   A3 
// 오좌우 :  A0   / 오위아래 :   A1
#define JOY_IN_THRUST           A0
#define JOY_IN_ROLL             A1
#define JOY_IN_PITCH            A2	
#define JOY_IN_YAW              A3


// Button
#define KEY_UP     			    4
#define KEY_DOWN   			    5
#define KEY_LEFT   			    6
#define KEY_RIGHT  		 	    8
#define KEY_ENTER  			    7


#define MENU_0_IDLE		    	0
#define MENU_0_RUN				1
#define MENU_1_IDLE				2        
#define MENU_1_RUN				3

#define STATE_MENU_BLE          0	
#define STATE_MENU_MODE         1	
#define STATE_MENU_REMOTE       2
#define STATE_MENU_TRIM         3
#define STATE_MENU_SAVE         4


#define RUN_BLE_SETUP           0
#define RUN_BLE_SCAN            1
#define RUN_BLE_CONNECT         2





#define TAKE_OFF_TIME		  2000	// ms
#define TAKE_OFF_TRUST		  500  


uint16_t Roll   = 0;
uint16_t Pitch  = 0;
uint16_t Yaw    = 0;
uint16_t Thrust = 0;


int joy_roll 	= 0;
int joy_pitch 	= 0;
int joy_yaw 	= 0;
int joy_thrust 	= 0;



U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0 |  U8G_I2C_OPT_NO_ACK | U8G_I2C_OPT_FAST);

MSP      Msp;
cBLE     Ble;
cJOY     JoyL;
cJOY     JoyR;
cKEY	 Key;

cJOY     *pJoy[2];


uint8_t joy_mode    = 0;
uint8_t remote_mode = 0;

uint8_t  take_off_state = 0;
bool     take_off = false;
uint16_t take_off_time    = TAKE_OFF_TIME;
int16_t  take_off_thrust  = TAKE_OFF_TRUST;


#define MENU_MAX     2

#define MENU_0_ITEMS 5
char *menu_0_strings[MENU_0_ITEMS] = { "BLE", 
                                       "MODE", 
                                       "REMOTE", 
                                       "TRIM", 
                                       "SAVE"};

#define MENU_1_ITEMS 3
char *menu_1_strings[MENU_1_ITEMS] = { "BLE_SETUP", 
                                       "BLE_SCAN", 
                                       "BLE_CONNECT" };


uint8_t menu_items[MENU_MAX]   = { MENU_0_ITEMS, MENU_1_ITEMS };
uint8_t menu_redraw_required = 0;






void setup() 
{
	int ret;

	//-- 사용되는 버튼 설정
	//
	Key.begin( KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_ENTER );
	Key.set_delay(70, 100);
	Key.begin( &JoyR );

	//-- LCD 라이브러리 설정
	//
	//u8g.setRot180();
	u8g.setFont(u8g_font_6x13);
	u8g.setFontPosTop();
	u8g.setColorIndex(1);

	Msp.begin(); 
	Ble.begin();

	JoyL.begin(A2, A3, 5);    
	JoyR.begin(A0, A1, 5);   
	JoyL.set_dir( -1, -1 );
	JoyR.set_dir( -1, -1 );

	pJoy[0] = &JoyL;
	pJoy[1] = &JoyR;


	lcd_redraw();


	lcd_draw_str( 0, 0, "Remote For CupDrone" );
	delay(500);

	if( Ble.load_list() == BLE_OK )
	{
		joy_mode = Ble.BleList.JoyMode;
		lcd_draw_str( 0, 0, "Loading...OK" );
	}
	else
	{
		lcd_draw_str( 0, 0, "Loading...No Data" );
	}

	delay(500);

	run_ble_setup();
}





void loop() 
{
	Key.update();

	loop_main();	
	loop_joystick();
	loop_msp();
}


void loop_main()
{
	static uint8_t state = MENU_0_IDLE;
	static uint8_t menu_0_index = 0;
	static uint8_t menu_1_index = 0;


	Key.use_joypad = false;

	switch( state )
	{
		case MENU_0_IDLE:
			Key.use_joypad = true;

			menu_0_index = loop_menu(0);

			if( Key.get_enter() || Key.get_right() )
			{
				state = MENU_0_RUN; 
			}
			take_off_state = 0;
			take_off       = false;
			break;

		case MENU_0_RUN:
			lcd_redraw();
			state = run_menu_0( MENU_0_RUN, menu_0_index );
			break;

		case MENU_1_IDLE:
			Key.use_joypad = true;

			menu_1_index = loop_menu_ble_list();		

			if( Key.get_enter() || Key.get_right() )
			{
				run_ble_connect( menu_1_index );
				lcd_redraw();
				state = MENU_0_RUN;
			}
			if( Key.get_left() )
			{
				lcd_redraw();
				state = MENU_0_RUN;				
			}		
			break;

		default:
			state = MENU_0_IDLE;
			break;
	}
}


uint8_t run_menu_0( uint8_t cur_state, int menu_sel )
{
	uint8_t ret_state = cur_state;
	uint8_t menu_index;
	uint8_t err_code;
	char Str[24];
	String strRet;
	static uint32_t tTime;

	remote_mode = 0;

	switch( menu_sel )
	{
		case STATE_MENU_BLE:
			Key.use_joypad = true;

			menu_index = loop_menu(1);
			lcd_draw_menu(1, menu_index);

			if( Key.get_enter() || Key.get_right() )
			{
				if( menu_index == RUN_BLE_SETUP )   run_ble_setup();
				if( menu_index == RUN_BLE_SCAN )    run_ble_scan();
				if( menu_index == RUN_BLE_CONNECT )
				{
					lcd_redraw();
					ret_state = MENU_1_IDLE;
				}
				lcd_redraw();
			}
			if( Key.get_left() )
			{
				lcd_redraw();
				ret_state = MENU_0_IDLE;
			}			
			break;

		case STATE_MENU_MODE:
			Key.use_joypad = true;

			if( joy_mode == 0 ) lcd_draw_str( 0, 0, "MODE LEFT" );
			else                lcd_draw_str( 0, 0, "MODE RIGHT"); 

			if( Key.get_enter() || Key.get_right() )
			{
				change_joymode();
				lcd_redraw();
			}
			if( Key.get_left() )
			{
				lcd_redraw();
				ret_state = MENU_0_IDLE;
			}				
			break;

		case STATE_MENU_REMOTE:
			Key.use_joypad = false;
			remote_mode = 1;

			if( (millis()-tTime) > 200 )
			{
				lcd_redraw();
				tTime = millis();
			}
			lcd_draw_remote();
			
			if( Key.get_enter() )
			{
				lcd_redraw();
				ret_state = MENU_0_IDLE;
			}		

			if(  Key.get_down() && Msp.bConnected == true )
			{
				Msp.SendCmd_ARM();
             	lcd_draw_str( 0, 0, "Send ARM     " );
              	delay(1000);		
			}	
			if( Key.get_up() && Msp.bConnected == true )
			{
				Msp.SendCmd_DISARM();
             	lcd_draw_str( 0, 0, "Send DISARM     " );
              	delay(1000);				
			}	
			if( Key.get_left() )
			{
				take_off_thrust += joy_yaw;
				if( take_off_thrust > 1000 ) take_off_thrust = 1000;
				if( take_off_thrust < 100  ) take_off_thrust = 100;
			}	

			loop_take_off();
			break;

		case STATE_MENU_TRIM:
			if( Key.get_up() && Msp.bConnected == true )
			{
				Msp.SendCmd_TRIM_UP();
            	lcd_draw_str( 0, 0, "Up" );
              	delay(1000);			   
			}
			else
      		if( Key.get_down() && Msp.bConnected == true )
			{
				Msp.SendCmd_TRIM_DOWN();
             	lcd_draw_str( 0, 0, "Down" );
              	delay(1000);
			}
			else
      		if( Key.get_left() && Msp.bConnected == true )
			{
				Msp.SendCmd_TRIM_LEFT();
              	lcd_draw_str( 0, 0, "Left" );
              	delay(1000);
			}	
			else
      		if( Key.get_right() && Msp.bConnected == true )
			{
				Msp.SendCmd_TRIM_RIGHT();
             	lcd_draw_str( 0, 0, "Right" );
              	delay(1000);
			}
			else
      		if( Key.get_enter() )
			{
				lcd_redraw();
				ret_state = MENU_0_IDLE;
			}	
			else
			{
				lcd_redraw();
				lcd_draw_str( 0, 0, "Press Key" );				
			}	
			break;

		case STATE_MENU_SAVE:
			run_save();
			ret_state = MENU_0_IDLE;
			break;

		default:
			ret_state = MENU_0_IDLE;
			break;
	}

	return ret_state;
}


void run_ble_setup( void )
{
	uint8_t err_code;
	char Str[24];
	String strRet;


	lcd_draw_str( 0, 0, "BLE Setup..." );
	sprintf(Str, "ret : %d", Ble.setup() );
	err_code = Ble.send_cmd("AT", strRet, 100);
	if( err_code == BLE_OK ) lcd_draw_str( 0, 0, "OK" );
	else                     lcd_draw_str( 0, 0, "FAIL" ); 
	delay(1000);
	Msp.bConnected = false;
}


void run_ble_scan( void )
{
	char Str[24];


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
}


void run_ble_connect( uint8_t menu_index )
{
	uint8_t err_code;
	char Str[24];

	lcd_draw_str( 0, 0, "Connecting..." ); 
	delay(10);
	err_code = Ble.connect(menu_index);
	if( err_code == BLE_OK && Ble.bConnected == true )
	{
		Ble.BleList.strName[menu_index].toCharArray(Str, 24);
		lcd_draw_str( 0, 0, Str); 
		Msp.bConnected = true;
	}
	else
	{
		lcd_draw_str( 0, 0, "Connect fail" );
	}

	delay(1000);
	lcd_redraw();;
}


void run_save( void )
{
	lcd_draw_str( 0, 0, "Saving..." ); 

	if( Ble.BleList.Count == 0 )
	{
		lcd_draw_str( 0, 0, "Saving..No Data" );
	}
	else
	{
		Ble.BleList.JoyMode = joy_mode;	
		Ble.save_list();
		lcd_draw_str( 0, 0, "Saving..OK" );
	}

	delay(1000);
}


void change_joymode( void )
{
	joy_mode ^= 1;	
	joy_mode &= 1;

	if( joy_mode == 0 )
	{
		pJoy[0] = &JoyL;
		pJoy[1] = &JoyR;
	}
	else
	{
		pJoy[0] = &JoyR;
		pJoy[1] = &JoyL;		
	}
}


int8_t loop_menu( uint8_t ch )
{
	static int8_t menu_cur[MENU_MAX] = { 0, };
	int8_t ret = -1;


	lcd_draw_menu(ch, menu_cur[ch]);

	if( Key.get_down() )
	{
		menu_cur[ch]++;	
		menu_cur[ch] %= menu_items[ch];
		lcd_redraw();
	}
	if( Key.get_up() )
	{
		if( menu_cur[ch] > 0 )
			menu_cur[ch]--;
		else
			menu_cur[ch] = menu_items[ch]-1;	
		menu_cur[ch] %= menu_items[ch];
		lcd_redraw();
	}

	return menu_cur[ch];
}


int8_t loop_menu_ble_list( void )
{
	static int8_t menu_current = 0;
	int8_t ret = -1;

	lcd_draw_ble_list(menu_current);

	if( Key.get_down() )
	{
		menu_current++;	
		menu_current %= Ble.BleList.Count;
		lcd_redraw();
	}
	if( Key.get_up() )
	{
		menu_current--;	
		menu_current %= Ble.BleList.Count;
		lcd_redraw();
	}

	return menu_current;
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
			joy_thrust = constrain( pJoy[1]->get_pitch(), 0, 500 );
			joy_thrust = map( joy_thrust, 0, 500, 0, 1000 );

      		// Left turn/Right turn (-500~500)
      		joy_yaw = map( pJoy[1]->get_roll(), -500, 500, -100, 100);      
		}
		else
		{
			if( (millis()-tTime) > 10 )
			{
				tTime = millis();

				#if 0
				joy_thrust = map( JoyR.get_pitch(), -500, 500,  -250, 250 );
        		joy_yaw = map( JoyL.get_roll(), -500, 500, -350, 350);  
        
				if     ( joy_thrust >  50 ) joy_thrust =  1;			
				else if( joy_thrust < -50 ) joy_thrust = -1;
				else
				{
					joy_thrust = 0;
				}
				Thrust = Thrust + joy_thrust;
				Thrust = constrain( Thrust, 0, 800 );				
				#else
				joy_thrust = map( pJoy[1]->get_pitch(), -500, 500,  -500, 500 );
        		joy_yaw = map( pJoy[1]->get_roll(), -500, 500, -100, 100);  
        
				joy_thrust = take_off_thrust + joy_thrust;
				Thrust = constrain( joy_thrust, 0, 800 );								
				#endif

			}
		}

     	// Left/Right(-500~500)
		joy_roll = map( pJoy[0]->get_roll(), -500, 500,  -100, 100 );

		// Up/Down(500~-500)
		joy_pitch = map( pJoy[0]->get_pitch(), -500, 500, -100, 100 );


		if( Msp.bConnected == true && remote_mode == 1 )
		{
			Roll   = joy_roll;
			Pitch  = joy_pitch;
     		Yaw    = joy_yaw;
            
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





void loop_take_off( )
{
	static uint32_t tTime;


		switch( take_off_state )
		{
			case 0:
				if( Key.get_right() )
				{
					take_off_state = 1;
				}		

				if( Key.get_left() )
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





void lcd_draw_menu( uint8_t ch, uint8_t sel_index )
{
	uint8_t i, h;
	u8g_uint_t w, d;
	static uint32_t tTime;
	char *pStr;


	if( menu_redraw_required == 0 ) return;

	u8g.firstPage();  
  	do 
  	{
		//u8g.setFont(u8g_font_6x13);
		u8g.setFontRefHeightText();
		u8g.setFontPosTop();

		h = u8g.getFontAscent()-u8g.getFontDescent();
		w = u8g.getWidth();
		for( i = 0; i < menu_items[ch]; i++ ) 
		{
			switch( ch )
			{
				case 0:
					pStr = menu_0_strings[i];					
					break;
				case 1:
					pStr = menu_1_strings[i];					
					break;					
			}


			d = (w-u8g.getStrWidth(pStr))/2;
			u8g.setDefaultForegroundColor();
			if ( i == sel_index ) 
			{
				u8g.drawBox(0, i*h+1, w, h);
      			u8g.setDefaultBackgroundColor();
			}
			u8g.drawStr(d, i*h, pStr);
		}

  	} while( u8g.nextPage() );

  	u8g.setDefaultForegroundColor();

  	menu_redraw_required = 0;
}


void lcd_redraw(void)
{
	menu_redraw_required = 1;
}


void lcd_draw_ble_list( uint8_t sel_index )
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
			if ( i == sel_index ) 
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
		sprintf(Str, "Thrust : %d", take_off_thrust );
		u8g.drawStr( 0*w, 0*h+1, Str);


		sprintf(Str, "RP : %d, %d", joy_roll, joy_pitch);
		u8g.drawStr( 0*w, 2*h+1, Str);
		sprintf(Str, "TY : %d, %d", Thrust, joy_yaw);
		u8g.drawStr( 0*w, 3*h+1, Str);


		sprintf(Str, "B:%d, C:%d, T: %d", Ble.bConnected, Msp.bConnected, take_off );
		u8g.drawStr( 0*w, 4*h+1, Str);

	

  	} while( u8g.nextPage() );

  	menu_redraw_required = 0;
}

