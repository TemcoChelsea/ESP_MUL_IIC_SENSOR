#include "define.h"
#include "Menu.h"
#include <string.h>

#define INTERNAL_TEMPERATURE_SENSOR 0


#define ext_sensor() ((HUM_data.sensor_select > 2) ? 2 : HUM_data.sensor_select)
#define ext_temp_c() ((int16)(HUM_data.I2C_Sensor[ext_sensor()].tem_org + HUM_data.I2C_Sensor[ext_sensor()].tem_offset))
#define ext_temp_f() ((int16)(ext_temp_c() * 9 / 5 + 320))


static uint8 item_index = 0;
static uint8 pre_item_index = 0;

static int16 set_value;

/* Open 10k (not fitted) clamps to -40.0C. menu_itoa() is unsigned, so -400 prints as 6513.6. */
static uint8 int_10k_absent(void)
{
	return (int16)(HUM_data.internal_temperature_c - HUM_data.int_temp_offset) <= -400;
}

static void format_temp_tenths(int16 value, uint8 *p)
{
	if(value < 0)
	{
		*p++ = '-';
		value = (int16)(-value);
	}
	menu_itoa((uint16)value, p, 1);
}

static void append_int_reading(uint8 *dst)
{
	if(int_10k_absent())
		strcat((char *)dst, "----");
	else if(HUM_data.deg_c_or_f == DEGREE_C)
	{
		format_temp_tenths(HUM_data.internal_temperature_c, int_text);
		strcat((char *)dst, (char *)int_text);
	}
	else
	{
		format_temp_tenths(HUM_data.internal_temperature_f, int_text);
		strcat((char *)dst, (char *)int_text);
	}

	if(HUM_data.deg_c_or_f == DEGREE_C)
		strcat((char *)dst, "~");
	else
		strcat((char *)dst, "|");
}

static void copy_int_reading(uint8 *dst)
{
	if(int_10k_absent())
		strcpy((char *)dst, "----");
	else if(HUM_data.deg_c_or_f == DEGREE_C)
		format_temp_tenths(HUM_data.internal_temperature_c, dst);
	else
		format_temp_tenths(HUM_data.internal_temperature_f, dst);

	if(HUM_data.deg_c_or_f == DEGREE_C)
		strcat((char *)dst, "~ ");
	else
		strcat((char *)dst, "| ");
}

#define MAX_TEMP_ITEMS	4
static uint8 const item_name[MAX_TEMP_ITEMS][17] = 
{
	"1.Int: ",
	"2.Ext: ",
	"3.Unit: ",
	"4.Sensor disp.: ",
};
 

void Temp_Conf_init(void)
{
	uint8 i;
	Lcd_Full_Screen(0);
	 
	for(i = 0; i < MAX_TEMP_ITEMS; i++)
	{ 
		strcpy((char *)text, (char *)item_name[i]);
		switch(i)
		{
			case 0: // internal temperature
				append_int_reading(text);
				break;
			case 1:	// external temperature
				if(HUM_data.deg_c_or_f == DEGREE_C)
				{	
//					sprintf((char *)text, "%s%u.%u|", item_name[i], ext_temp_c() / 10, ext_temp_c() % 10);
					itoa( ext_temp_c(), int_text, 1);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)"~");
				}
				else
				{	
//					sprintf((char *)text, "%s%u.%u|", item_name[i], ext_temp_f() / 10, ext_temp_f() % 10);
					itoa( ext_temp_f(), int_text, 1);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)"|");
				} 
				break;
			case 2:	// Unit select
				if(HUM_data.deg_c_or_f == DEGREE_C)
				{	
//					sprintf((char *)text, "%s~", item_name[i]);
					strcat((char *)text, (char *)"~");
				}
				else
				{	
//					sprintf((char *)text, "%s|", item_name[i]);
					strcat((char *)text, (char *)"|");
				}
				break;
			case 3:	// Select to display on normal screen
				if(HUM_data.temperature_sensor_select == INTERNAL_TEMPERATURE_SENSOR)
				{	
//					sprintf((char *)text, "%sInt.", item_name[i]);
					strcat((char *)text, (char *)"Int.");
				}
				else
				{	
//					sprintf((char *)text, "%sExt.", item_name[i]);
					strcat((char *)text, (char *)"Ext.");
				}
				break;
		}
		 
		if(i == item_index)
			Lcd_Show_String(i, 0, DISP_INV, text);
		else
			Lcd_Show_String(i, 0, DISP_NOR, text);
	}
	in_sub_menu = FALSE;
}

void Temp_Conf_display(void)
{    
	if(in_sub_menu == FALSE)
	{
		if(pre_item_index != item_index)
		{
			strcpy((char *)text, (char *)item_name[pre_item_index]);
			switch(pre_item_index)
			{
				case 0:
					append_int_reading(text);
					break;
				case 1:
					if(HUM_data.deg_c_or_f == DEGREE_C)
					{	
//						sprintf((char *)text, "%s%u.%u~", item_name[pre_item_index], ext_temp_c() / 10, ext_temp_c() % 10);
						itoa(ext_temp_c(), int_text, 1);
						strcat((char *)text, (char *)int_text);
						strcat((char *)text, (char *)"~");
					}
					else
					{	
//						sprintf((char *)text, "%s%u.%u|", item_name[pre_item_index], ext_temp_f() / 10, ext_temp_f() % 10);
						itoa(ext_temp_f(), int_text, 1);
						strcat((char *)text, (char *)int_text);
						strcat((char *)text, (char *)"|");
					}
					break;
				case 2:
					if(HUM_data.deg_c_or_f == DEGREE_C)
					{	
//						sprintf((char *)text, "%s~", item_name[pre_item_index]);
					 
						strcat((char *)text, (char *)"~");
					}
					else
					{	
//						sprintf((char *)text, "%s|", item_name[pre_item_index]);
						strcat((char *)text, (char *)"|");
					}
					break;
				case 3:
					if(HUM_data.temperature_sensor_select == INTERNAL_TEMPERATURE_SENSOR)
					{	
//						sprintf((char *)text, "%sInt.", item_name[pre_item_index]);
						strcat((char *)text, (char *)"Int.");
					}
					else
					{	
//						sprintf((char *)text, "%sExt.", item_name[pre_item_index]);
						strcat((char *)text, (char *)"Ext.");
					}
					break;
			}
			Lcd_Clear_Row(pre_item_index);
			Lcd_Show_String(pre_item_index, 0, DISP_NOR, text);
			
			strcpy((char *)text, (char *)item_name[item_index]);
			switch(item_index)
			{
				case 0:
					append_int_reading(text);
					break;
				case 1:
					if(HUM_data.deg_c_or_f == DEGREE_C)
					{	
//						sprintf((char *)text, "%s%u.%u~", item_name[pre_item_index], ext_temp_c() / 10, ext_temp_c() % 10);
						itoa(ext_temp_c(), int_text, 1);
						strcat((char *)text, (char *)int_text);
						strcat((char *)text, (char *)"~");
					}
					else
					{	
//						sprintf((char *)text, "%s%u.%u|", item_name[pre_item_index], ext_temp_f() / 10, ext_temp_f() % 10);
						itoa(ext_temp_f(), int_text, 1);
						strcat((char *)text, (char *)int_text);
						strcat((char *)text, (char *)"|");
					}
					break;
				case 2:
					if(HUM_data.deg_c_or_f == DEGREE_C)
					{	
//						sprintf((char *)text, "%s~", item_name[pre_item_index]);
					 
						strcat((char *)text, (char *)"~");
					}
					else
					{	
//						sprintf((char *)text, "%s|", item_name[pre_item_index]);
						strcat((char *)text, (char *)"|");
					}
					break;
				case 3:
					if(HUM_data.temperature_sensor_select == INTERNAL_TEMPERATURE_SENSOR)
					{	
//						sprintf((char *)text, "%sInt.", item_name[pre_item_index]);
						strcat((char *)text, (char *)"Int.");
					}
					else
					{	
//						sprintf((char *)text, "%sExt.", item_name[pre_item_index]);
						strcat((char *)text, (char *)"Ext.");
					}
					break;
			}
			Lcd_Clear_Row(item_index);
			Lcd_Show_String(item_index, 0, DISP_INV, text);

			pre_item_index = item_index;
		}
		else
		{
			 
//			// internal temperature
//			if(HUM_data.deg_c_or_f == DEGREE_C)
//			{	
////				sprintf((char *)text, "%u.%u~", HUM_data.internal_temperature_c /10, HUM_data.internal_temperature_c % 10);
//				itoa(HUM_data.internal_temperature_c, int_text, 1);
//				strcpy((char *)text, (char *)int_text);
//				strcat((char *)text, (char *)"~ ");
//			}	
//			else
//			{	
////				sprintf((char *)text, "%u.%u|", HUM_data.internal_temperature_f /10, HUM_data.internal_temperature_f % 10);
//				itoa(HUM_data.internal_temperature_f, int_text, 1);
//				strcpy((char *)text, (char *)int_text);
//				strcat((char *)text, (char *)"| "); 
//			}
//			Lcd_Show_String(0, 7, DISP_NOR, (uint8 *)"      ");
//			
//			if(item_index == 0)
//				Lcd_Show_String(0, 7, DISP_INV, text);
//			else
//				Lcd_Show_String(0, 7, DISP_NOR, text);


//			// external temperature
//			if(HUM_data.deg_c_or_f == DEGREE_C)
//			{	
////				sprintf((char *)text, "%u.%u~", ext_temp_c() /10, ext_temp_c() % 10);
//				itoa(ext_temp_c() , int_text, 1);
//				strcpy((char *)text, (char *)int_text);
//				strcat((char *)text, (char *)"~");
//			}	
//			else
//			{	
////				sprintf((char *)text, "%u.%u|", ext_temp_f() /10, ext_temp_f() % 10);
//				itoa(ext_temp_f() , int_text, 1);
//				strcpy((char *)text, (char *)int_text);
//				strcat((char *)text, (char *)"|");
//			}
//			Lcd_Show_String(1, 7, DISP_NOR, (uint8 *)"        "); 
//			if(item_index == 1)
//				Lcd_Show_String(1, 7, DISP_INV, text);
//			else
//				Lcd_Show_String(1, 7, DISP_NOR, text);
//			
//			strcpy((char *)text, (char *)item_name[2]); 
//			if(HUM_data.deg_c_or_f == DEGREE_C)
//			{	 
//				strcat((char *)text, (char *)"~");
//			}
//			else
//			{	
//				strcat((char *)text, (char *)"|");
//			}
//			Lcd_Clear_Row(2);
//			if(item_index == 2)
//				Lcd_Show_String(2, 0, DISP_INV, text);		 
//			else
//				Lcd_Show_String(2, 0, DISP_NOR, text);	

//			strcpy((char *)text, (char *)item_name[3]);
//			if(HUM_data.temperature_sensor_select == INTERNAL_TEMPERATURE_SENSOR)
//			{	
////						sprintf((char *)text, "%sInt.", item_name[pre_item_index]);
//				strcat((char *)text, (char *)"Int.");
//			}
//			else
//			{	
////						sprintf((char *)text, "%sExt.", item_name[pre_item_index]);
//				strcat((char *)text, (char *)"Ext.");
//			}  
//			Lcd_Clear_Row(3);
//			if(item_index == 3)
//				Lcd_Show_String(3, 0, DISP_INV, text);
//			else
//				Lcd_Show_String(3, 0, DISP_NOR, text);
//			
			
		}
	} 
	else
	{
		if(value_change == TRUE)
		{
			switch(item_index)
			{
				case 0:
				case 1:
					if(HUM_data.deg_c_or_f == DEGREE_C)
					{	
						format_temp_tenths(set_value, text);
						strcat((char *)text, (char *)"~");
					}
					else
					{	
						format_temp_tenths(set_value, text);
						strcat((char *)text, (char *)"|");
					}
					Lcd_Show_String(item_index, 7, DISP_NOR, (uint8 *)"      ");
					Lcd_Show_String(item_index, 7, DISP_INV, text);
					break;
				case 2:
					if(set_value == DEGREE_C)
						Lcd_Write_Char(item_index, 8, '~', DISP_INV);
					else
						Lcd_Write_Char(item_index, 8, '|', DISP_INV);
					break;
				case 3:
					if(set_value == INTERNAL_TEMPERATURE_SENSOR)
						Lcd_Show_String(item_index, 16, DISP_INV, (uint8 *)"Int.");
					else
						Lcd_Show_String(item_index, 16, DISP_INV, (uint8 *)"Ext.");
					break;
			}
			
			value_change = FALSE;	
		}
	} 
}

void Temp_Conf_keycope(uint16 key_value)
{
	start_back_light(backlight_keep_seconds);
	switch(key_value & KEY_SPEED_MASK)
	{
		case KEY_NON:
			// do nothing
			break;
		case KEY_LEFT_MASK:
			if(in_sub_menu == FALSE)
			{
				update_menu_state(MenuMain);
			}
			else
			{
				strcpy((char *)text, (char *)item_name[item_index]);
				switch(item_index)
				{
					case 0:
						append_int_reading(text);
						break;
					case 1:
						if(HUM_data.deg_c_or_f == DEGREE_C)
						{	
	//						sprintf((char *)text, "%s%u.%u~", item_name[pre_item_index], ext_temp_c() / 10, ext_temp_c() % 10);
							itoa(ext_temp_c() , int_text, 1);
							strcat((char *)text, (char *)int_text);
							strcat((char *)text, (char *)"~");
						}
						else
						{	
	//						sprintf((char *)text, "%s%u.%u|", item_name[pre_item_index], ext_temp_f() / 10, ext_temp_f() % 10);
							itoa(ext_temp_f() , int_text, 1);
							strcat((char *)text, (char *)int_text);
							strcat((char *)text, (char *)"|");
						}
						break;
					case 2:
						if(HUM_data.deg_c_or_f == DEGREE_C)
						{	
	//						sprintf((char *)text, "%s~", item_name[pre_item_index]);
						 
							strcat((char *)text, (char *)"~");
						}
						else
						{	
	//						sprintf((char *)text, "%s|", item_name[pre_item_index]);
							strcat((char *)text, (char *)"|");
						}
						break;
					case 3:
						if(HUM_data.temperature_sensor_select == INTERNAL_TEMPERATURE_SENSOR)
						{	
	//						sprintf((char *)text, "%sInt.", item_name[pre_item_index]);
							strcat((char *)text, (char *)"Int.");
						}
						else
						{	
	//						sprintf((char *)text, "%sExt.", item_name[pre_item_index]);
							strcat((char *)text, (char *)"Ext.");
						}
						break;
				}
				Lcd_Show_String(item_index, 0, DISP_INV, text);

				in_sub_menu = FALSE;
			}
			break;
		case KEY_RIGHT_MASK:
			if(in_sub_menu == FALSE)
			{
				Lcd_Show_String(item_index, 0, DISP_NOR, (uint8 *)item_name[item_index]);
				switch(item_index)
				{
					case 0:
						if(int_10k_absent())
						{
							strcpy((char *)text, (char *)item_name[item_index]);
							append_int_reading(text);
							Lcd_Clear_Row(item_index);
							Lcd_Show_String(item_index, 0, DISP_INV, text);
							break;
						}
						if(HUM_data.deg_c_or_f == DEGREE_C)
						{
							set_value = HUM_data.internal_temperature_c;
							format_temp_tenths(set_value, text);
							strcat((char *)text, (char *)"~");
						}
						else
						{
							set_value = HUM_data.internal_temperature_f;
							format_temp_tenths(set_value, text);
							strcat((char *)text, (char *)"|");
						}
						Lcd_Show_String(item_index, 7, DISP_INV, text);
						break;
					case 1:
						if(HUM_data.deg_c_or_f == DEGREE_C)
						{
							set_value = ext_temp_c();
//							sprintf((char *)text, "%u.%u~", set_value / 10, set_value % 10);
							itoa(set_value, int_text, 1);
							strcpy((char *)text, (char *)int_text);
							strcat((char *)text, (char *)"~");
						}
						else
						{
							set_value = ext_temp_f();
//							sprintf((char *)text, "%u.%u|", set_value / 10, set_value % 10);
							itoa(set_value, int_text, 1);
							strcpy((char *)text, (char *)int_text);
							strcat((char *)text, (char *)"|");
						}
						Lcd_Show_String(item_index, 7, DISP_INV, text);
						break;
					case 2:
						set_value = HUM_data.deg_c_or_f;
						if(set_value == DEGREE_C)
							Lcd_Write_Char(item_index, 8, '~', DISP_INV);
						else
							Lcd_Write_Char(item_index, 8, '|', DISP_INV);
						break;
					case 3:
						set_value = HUM_data.temperature_sensor_select;
						if(set_value == INTERNAL_TEMPERATURE_SENSOR)
							Lcd_Show_String(item_index, 16, DISP_INV, (uint8 *)"Int.");
						else
							Lcd_Show_String(item_index, 16, DISP_INV, (uint8 *)"Ext.");
						break;
				}
			
				if(!(item_index == 0 && int_10k_absent()))
					in_sub_menu = TRUE;
			}
			else
			{
				
				switch(item_index)
				{
					case 0:
						strcpy((char *)text, (char *)item_name[item_index]);
						if(HUM_data.deg_c_or_f == DEGREE_C)
						{
							format_temp_tenths(set_value, int_text);
							strcat((char *)text, (char *)int_text);
							strcat((char *)text, (char *)"~");
							
							HUM_data.int_temp_offset += set_value - HUM_data.internal_temperature_c;
//							start_data_save_timer();

							HUM_data.internal_temperature_c = set_value;
						}
						else
						{
							format_temp_tenths(set_value, int_text);
							strcat((char *)text, (char *)int_text);
							strcat((char *)text, (char *)"|");
							HUM_data.int_temp_offset += ((int16)set_value - HUM_data.internal_temperature_f) * 5 / 9;
//							start_data_save_timer();

							HUM_data.internal_temperature_f = set_value;
						}
						save_int_temp_offset();
						break;
					case 1:
						strcpy((char *)text, (char *)item_name[item_index]);
						if(HUM_data.deg_c_or_f == DEGREE_C)
						{
		//					sprintf((char *)text, "%s%u.%u~", item_name[item_index], set_value / 10, set_value % 10);
							itoa(set_value, int_text, 1);
							strcat((char *)text, (char *)int_text);
							strcat((char *)text, (char *)"~");
							HUM_data.I2C_Sensor[ext_sensor()].tem_offset = set_value - HUM_data.I2C_Sensor[ext_sensor()].tem_org;

							 
						}
						else
						{
//							sprintf((char *)text, "%s%u.%u|", item_name[item_index], set_value / 10, set_value % 10);
							itoa(set_value, int_text, 1);
							strcat((char *)text, (char *)int_text);
							strcat((char *)text, (char *)"|");
							HUM_data.I2C_Sensor[ext_sensor()].tem_offset = ((int16)set_value - 320) * 5 / 9 - HUM_data.I2C_Sensor[ext_sensor()].tem_org;
 
						}
						save_i2c_sensor_offsets(ext_sensor());
						update_hum_para();
						break;
					case 2:
						HUM_data.deg_c_or_f = set_value;
					
						copy_int_reading(text);
						Lcd_Show_String(0, 7, DISP_NOR, (uint8 *)"      ");
						
						if(item_index == 0)
							Lcd_Show_String(0, 7, DISP_INV, text);
						else
							Lcd_Show_String(0, 7, DISP_NOR, text);


						// external temperature
						if(HUM_data.deg_c_or_f == DEGREE_C)
						{	
			//				sprintf((char *)text, "%u.%u~", ext_temp_c() /10, ext_temp_c() % 10);
							itoa(ext_temp_c() , int_text, 1);
							strcpy((char *)text, (char *)int_text);
							strcat((char *)text, (char *)"~");
						}	
						else
						{	
			//				sprintf((char *)text, "%u.%u|", ext_temp_f() /10, ext_temp_f() % 10);
							itoa(ext_temp_f() , int_text, 1);
							strcpy((char *)text, (char *)int_text);
							strcat((char *)text, (char *)"|");
						}
						Lcd_Show_String(1, 7, DISP_NOR, (uint8 *)"        "); 
						if(item_index == 1)
							Lcd_Show_String(1, 7, DISP_INV, text);
						else
							Lcd_Show_String(1, 7, DISP_NOR, text);
						
						
						
						strcpy((char *)text, (char *)item_name[item_index]);
						
						if(HUM_data.deg_c_or_f == DEGREE_C)
					//		sprintf((char *)text, "%s~", item_name[item_index]);
							strcat((char *)text, (char *)"~");
						else
					//		sprintf((char *)text, "%s|", item_name[item_index]);
							strcat((char *)text, (char *)"|");
						save_uint8_to_flash(FLASH_DEG_C_OR_F, (uint8)HUM_data.deg_c_or_f);
						
			
						break;
					case 3:
						strcpy((char *)text, (char *)item_name[item_index]);
						HUM_data.temperature_sensor_select = set_value;
						if(HUM_data.temperature_sensor_select == INTERNAL_TEMPERATURE_SENSOR)
			//				sprintf((char *)text, "%sInt.", item_name[item_index]);
							strcat((char *)text, (char *)"Int.");
						else
			//				sprintf((char *)text, "%sExt.", item_name[item_index]);
							strcat((char *)text, (char *)"Ext.");
						save_uint8_to_flash(FLASH_SENSOR_SELECT, HUM_data.temperature_sensor_select);
						break;
						
				}
				Lcd_Show_String(item_index, 0, DISP_INV, text);

				in_sub_menu = FALSE;
			}
			break;
		case KEY_UP_MASK:
			if(in_sub_menu == FALSE)
			{
				if(item_index)
					item_index--;
				else
					item_index = MAX_TEMP_ITEMS - 1;
			}
			else
			{
				switch(item_index)
				{
					case 0:
					case 1:
						if(set_value < 1500)
							set_value++;
						else
							set_value = 0;						
						break;
					case 2:
					case 3:
						if(set_value)
							set_value = 0;
						else
							set_value = 1;
						break;
				}

				value_change = TRUE;
			}
			break;
		case KEY_DOWN_MASK:
			if(in_sub_menu == FALSE)
			{
				item_index++;
				item_index %= MAX_TEMP_ITEMS;
			}
			else
			{
				switch(item_index)
				{
					case 0:
					case 1:
						if(set_value)
							set_value--;
						else
							set_value = 1500;						
						break;
					case 2:
					case 3:
						if(set_value)
							set_value = 0;
						else
							set_value = 1;
						break;
				}
				value_change = TRUE;
			}
			break;
	}
}



