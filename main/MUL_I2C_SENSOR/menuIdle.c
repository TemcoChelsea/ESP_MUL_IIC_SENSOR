#include "define.h"
#include "Menu.h"

#define	NODES_POLL_PERIOD	30

uint8 co2_check_status = 0;//0:ini value  1 to 5: read co2 1-5 times  6:read co2 fail 7: read co2 OK   
static uint8 display_around_time_ctr = NODES_POLL_PERIOD;
static uint8 disp_index = 0;
static uint8 warming_state = TRUE;
static uint8 force_refresh = TRUE;
extern uint8 rx_icon;
extern uint8 tx_icon;

uint8_t lcd_i2c_sensor_index = 0;
uint8_t flag_setpoint;
int8_t index_setpoint;
//uint8_t max_setpoint;
uint8_t icon_setpoint[3];
uint8_t count_enter_idle;
uint8_t Last_Screen_Area;

uint8_t setpoint_step;
uint8_t setpoint_max_c;
uint8_t setpoint_min_c;
uint8_t setpoint_max_f;
uint8_t setpoint_min_f;

uint8_t i2c_sht_num = 0;
uint8_t i2c_co2_num = 0;

extern uint16_t SetPoint[3];

void start_setpoint_count(void);

char get_curret_setp(uint8_t index)
{
	uint8_t i;
	if(index < 0)	return -1;
	if(index > 2) return -1;
	for (i = 0; i < 3;i++)
	{
		if(icon_setpoint[i] != 0 && (i == index))
			return i;
			
	} 
	return -1;
}

void Idle_init(void)
{
	uint8 db_ctr;
	db_ctr = 1;

//	intial_setpoint();
//	start_setpoint_count();
	
	if(!screen_area_is_fixed())
	{
		if(Modbus.mul_i2c_mode == MUL_I2C_MODE_HUM || Modbus.mul_i2c_mode == MUL_I2C_MODE_PRESSURE)  // RTS2
		{
			if(Mul_i2c_data.screenArea[1] == SCREEN_AREA_TEMSETP)
				Mul_i2c_data.screenArea[1] = SCREEN_AREA_TEMP;
			Last_Screen_Area = Mul_i2c_data.screenArea[1];
		}
		else
		{
			if(Mul_i2c_data.screenArea[0] == SCREEN_AREA_TEMSETP)
				Mul_i2c_data.screenArea[0] = SCREEN_AREA_TEMP;
			Last_Screen_Area = Mul_i2c_data.screenArea[0];
		}
	}
	
//	Mul_i2c_data.screenArea[2] = SCREEN_AREA_NONE;

	/* LCD_Init already cleared the screen. The first Idle_init runs after
	   Lcd_task may have painted zeros; do not clear between those frames.
	   Later entries (back from a menu) still clear. */
	{
		static uint8 idle_inited;
		if(idle_inited)
			Lcd_Full_Screen(0);
		idle_inited = 1;
	}
	lcd_i2c_sensor_index = 0;
	exit_request_password();
}

 
void get_data_format(u8 loc,float num,char *s)
{
	u8 i,s_len,s_start,buf_start;
	
	if(loc == 0)
		sprintf(s,"%9.0f",num);
	else if(loc == 1)
		sprintf(s,"%9.1f",num);
	else if(loc == 2)
		sprintf(s,"%9.2f",num);
	else if(loc == 3)
		sprintf(s,"%9.3f",num);
	else if(loc == 4)
		sprintf(s,"%9.4f",num);
	else if(loc == 5)
		sprintf(s,"%9.5f",num);
	else if(loc == 6)
		sprintf(s,"%9.6f",num);
	else if(loc == 7)
		sprintf(s,"%9.0f",num);
	else
		sprintf(s,"%f",num);
	
	for(i=0;i<9;i++)
	{
		if(s[i]!=0x20) break;
	}
	s_len = 9 - i;   					//???????
	s_start = i;     					//???????????
	buf_start = i - i / 2; 				//??????????????????
	
	for(i=0;i<s_len;i++) 				//????????
	{
		s[buf_start + i] = s[s_start + i];
	}
	for(i=buf_start + s_len;i<9;i++ ) 	//??" "
	{
		s[i] = 0x20;
	} 
}
 

void Idle_display(void)
{
//	static uint16 previous_co2 = EXCEPTION_PPM;
	u8 temp = 0; 
	float ftemp;
	
	if(isColorScreen == false)
	{
	}
	check_setpoint_count();
}

void intial_setpoint(void)
{
	flag_setpoint = 0;
	index_setpoint = -1;
	count_enter_idle = 0;
//	max_setpoint = check_idle_setp();
	
}
void start_setpoint_count(void)
{
	count_enter_idle = 10;
}

void check_setpoint_count(void)
{
	if(flag_setpoint > 0)
	{
		if(count_enter_idle > 0)
			count_enter_idle--;
		else
		{
			intial_setpoint();	
			if(!screen_area_is_fixed())
			{
			/*if((Modbus.mini_type == PROJECT_HUM || Modbus.mini_type == PROJECT_PRESSURE) && (sub_product == 1)) // RTS2
				Mul_i2c_data.screenArea[1] = Last_Screen_Area;
			else*/
				Mul_i2c_data.screenArea[0] = Last_Screen_Area;
			}
		}
	}
}

void Idle_keycope(uint16 key_value)
{
	start_back_light(backlight_keep_seconds);	
	switch(key_value /*& KEY_SPEED_MASK*/)
	{
		case KEY_NON:
			// do nothing
			break;
		case KEY_UP_MASK:
			// PM2.5 dont have temp setp by press UP/DOWN button
			// 
			if(lock_menu == 2) break;
			if(lock_menu == 1) break;
			if(Modbus.mul_i2c_mode == MUL_I2C_MODE_PM25) 		break;
			if(Modbus.mul_i2c_mode == MUL_I2C_MODE_PRESSURE)		break;
			if((Modbus.mul_i2c_mode == MUL_I2C_MODE_CO2) && (Mul_i2c_data.screenArea[0] == SCREEN_AREA_NONE))
				break;
			if(flag_setpoint == 0)
			{
				if(i2c_sht_num + i2c_co2_num > 1)
				{
					if(lcd_i2c_sensor_index < i2c_sht_num + i2c_co2_num - 1)	
						lcd_i2c_sensor_index++;
					else
						lcd_i2c_sensor_index = 0;
				}
				else if(!screen_area_is_fixed())
				{
					if(Modbus.mul_i2c_mode == MUL_I2C_MODE_HUM || Modbus.mul_i2c_mode == MUL_I2C_MODE_PRESSURE)  // RTS2
						Mul_i2c_data.screenArea[1] = SCREEN_AREA_TEMSETP;
					else
						Mul_i2c_data.screenArea[0] = SCREEN_AREA_TEMSETP;
					//if(max_setpoint > 0)
					{
						start_setpoint_count();
						flag_setpoint = 1;
	//					if(index_setpoint < 2)
	//						index_setpoint++;
	//					else
							index_setpoint = 0;
					}
				}
			}
			else
			{
				char i;
				start_setpoint_count();
				/*i = get_curret_setp(index_setpoint);
				if(i >= 0)
				{
					if(icon_setpoint[i] == SCREEN_AREA_TEMSETP)
					{
						PID[1].EEP_SetPoint += 10;
						save_uint16_to_flash(FLASH_PID2_SETPOINT,PID[1].EEP_SetPoint);
					}
					if(icon_setpoint[i] == SCREEN_AREA_HUMSETP)
					{
						PID[0].EEP_SetPoint += 10;
						save_uint16_to_flash(FLASH_PID1_SETPOINT,PID[0].EEP_SetPoint);
					}
					if(icon_setpoint[i] == SCREEN_AREA_CO2SETP)
					{
						PID[2].EEP_SetPoint += 10;
						save_uint16_to_flash(FLASH_PID3_SETPOINT,PID[2].EEP_SetPoint);
					}
				}*/
				if(HUM_data.deg_c_or_f == DEGREE_C)
				{
					if(SetPoint[1] < setpoint_max_c * 10)
						SetPoint[1] += setpoint_step;
//					else
//						PID[1].EEP_SetPoint = setpoint_min_c * 10;
				}
				else
				{
					if(SetPoint[1] < setpoint_max_f * 10)
						SetPoint[1] += setpoint_step;
//					else
//						PID[1].EEP_SetPoint = setpoint_min_f * 10;
				}
				save_uint16_to_flash(FLASH_PID2_SETPOINT,SetPoint[1]);
			}
			// do nothing
			break;
		case KEY_DOWN_MASK:
			if(lock_menu == 2) break;
			if(lock_menu == 1) break;
			if(Modbus.mul_i2c_mode == MUL_I2C_MODE_PM25) 						break;
			if(Modbus.mul_i2c_mode == MUL_I2C_MODE_PRESSURE)		break;
			if((Modbus.mul_i2c_mode == MUL_I2C_MODE_CO2) && (Mul_i2c_data.screenArea[0] == SCREEN_AREA_NONE))
					break;
			if(flag_setpoint == 0)
			{
				if(i2c_sht_num + i2c_co2_num > 1)
				{
					if(lcd_i2c_sensor_index > 0)	
						lcd_i2c_sensor_index--;
					else
						lcd_i2c_sensor_index = i2c_sht_num + i2c_co2_num - 1;
				}
				else if(!screen_area_is_fixed())
				{
					if((Modbus.mul_i2c_mode == MUL_I2C_MODE_HUM || Modbus.mul_i2c_mode == MUL_I2C_MODE_PRESSURE) /*&& (sub_product == 1)*/) // RTS2
						Mul_i2c_data.screenArea[1] = SCREEN_AREA_TEMSETP;
					else
						Mul_i2c_data.screenArea[0] = SCREEN_AREA_TEMSETP;
					//if(max_setpoint > 0)
					{
						start_setpoint_count();
						flag_setpoint = 1;
	//					if(index_setpoint < 2)
	//						index_setpoint++;
	//					else
							index_setpoint = i2c_sht_num + i2c_co2_num;
					}
				}
			}
			else
			{
				char i;
				start_setpoint_count();
				/*i = get_curret_setp(index_setpoint);
				if(i >= 0)
				{
					if(icon_setpoint[i] == SCREEN_AREA_TEMSETP)
					{
						PID[1].EEP_SetPoint -= 10;
						save_uint16_to_flash(FLASH_PID2_SETPOINT,PID[1].EEP_SetPoint);
					}
					if(icon_setpoint[i] == SCREEN_AREA_HUMSETP)
					{
						PID[0].EEP_SetPoint -= 10;
						save_uint16_to_flash(FLASH_PID1_SETPOINT,PID[0].EEP_SetPoint);
					}
					if(icon_setpoint[i] == SCREEN_AREA_CO2SETP)
					{
						PID[2].EEP_SetPoint -= 10;
						save_uint16_to_flash(FLASH_PID3_SETPOINT,PID[2].EEP_SetPoint);
					}
				}*/
				if(HUM_data.deg_c_or_f == DEGREE_C)
				{
					if(SetPoint[1] > setpoint_min_c * 10)
						SetPoint[1] -= setpoint_step;
//					else
//						PID[1].EEP_SetPoint = setpoint_max_c * 10;
				}
				else
				{
					if(SetPoint[1] > setpoint_min_f * 10)
						SetPoint[1] -= setpoint_step;
//					else
//						PID[1].EEP_SetPoint = setpoint_max_f * 10;
				}
				
				save_uint16_to_flash(FLASH_PID2_SETPOINT,SetPoint[1]);
			}
			break;
		case KEY_LEFT_MASK:
//			if(max_setpoint > 0)
//			{
//				start_setpoint_count();
//				flag_setpoint = 1;
//				if(index_setpoint < 2)
//					index_setpoint++;
//				else
//					index_setpoint = 0;
//			}
			break;
		case KEY_RIGHT_MASK:
			if(lock_menu == 2) break;
			if(lock_menu == 1) break;
		// go into main menu
			flag_setpoint = 0;
			update_menu_state(MenuMain);  // lock_menu == 0
			
			break;
		case KEY_HOLD_LR_MASK:
			if(lock_menu == 2) break;
			if(lock_menu == 0) break;		
		// go into main menu
			flag_setpoint = 0;
			update_menu_state(MenuMain);  // lock_menu == 1
			break;
		default:
			break;
	}
}



