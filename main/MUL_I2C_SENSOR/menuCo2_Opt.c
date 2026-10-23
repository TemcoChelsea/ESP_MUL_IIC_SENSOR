#include "define.h"
#include "Menu.h"

static uint8 item_index = 0;
static uint8 pre_item_index = 0;

static uint16 set_value;

#define MAX_CO2_OPT_ITEMS	4
static uint8 const item_name[MAX_CO2_OPT_ITEMS][16] = 
{
	"1.Channel: ",
	"2.Spt poor: ",
	"3.Spt fair: ",
	"4.Calibration: ",
};

static void format_item(uint8 index, uint8 *buf)
{
	switch(index)
	{
		case 0:
			sprintf((char *)buf, "%s%u", item_name[index], (uint16)mul_co2_index());
			break;
		case 1:
			sprintf((char *)buf, "%s%u", item_name[index], Mul_i2c_data.alarm_setpoint);
			break;
		case 2:
			sprintf((char *)buf, "%s%u", item_name[index], Mul_i2c_data.pre_alarm_setpoint);
			break;
		case 3:
			sprintf((char *)buf, "%s%u", item_name[index], Mul_i2c_data.I2C_Sensor[mul_co2_index()].co2_org);
			break;
	}
}

void Co2_Opt_init(void)
{
	uint8 i;

	Lcd_Full_Screen(0);
	for(i = 0; i < MAX_CO2_OPT_ITEMS; i++)
	{
		format_item(i, text);
		if(i == item_index)
			Lcd_Show_String(i, 0, DISP_INV, text);
		else
			Lcd_Show_String(i, 0, DISP_NOR, text);
	}
	in_sub_menu = FALSE;
}

void Co2_Opt_display(void)
{
	if(in_sub_menu == FALSE)
	{
		if(pre_item_index != item_index)
		{
			format_item(pre_item_index, text);
			Lcd_Clear_Row(pre_item_index);
			Lcd_Show_String(pre_item_index, 0, DISP_NOR, text);

			format_item(item_index, text);
			Lcd_Clear_Row(item_index);
			Lcd_Show_String(item_index, 0, DISP_INV, text);

			pre_item_index = item_index;
		}
		else if(item_index == 3)
		{
			sprintf((char *)text, "%u", Mul_i2c_data.I2C_Sensor[mul_co2_index()].co2_org);
			Lcd_Show_String(3, 15, DISP_NOR, (uint8 *)int_space);
			Lcd_Show_String(3, 15, DISP_INV, text);
		}
	}
	else if(value_change == TRUE)
	{
		sprintf((char *)text, "%u", set_value);
		if(item_index == 0)
		{
			Lcd_Show_String(item_index, 11, DISP_NOR, (uint8 *)"  ");
			Lcd_Show_String(item_index, 11, DISP_INV, text);
		}
		else if((item_index == 1) || (item_index == 2))
		{
			Lcd_Show_String(item_index, 12, DISP_NOR, (uint8 *)int_space);
			Lcd_Show_String(item_index, 12, DISP_INV, text);
		}
		else if(item_index == 3)
		{
			Lcd_Show_String(item_index, 15, DISP_NOR, (uint8 *)int_space);
			Lcd_Show_String(item_index, 15, DISP_INV, text);
		}
		value_change = FALSE;
	}
}

void Co2_Opt_keycope(uint16 key_value)
{
	start_back_light(backlight_keep_seconds);
	switch(key_value & KEY_SPEED_MASK)
	{
		case KEY_NON:
			break;
		case KEY_LEFT_MASK:
			if(in_sub_menu == FALSE)
			{
				update_menu_state(MenuMain);
			}
			else
			{
				format_item(item_index, text);
				Lcd_Clear_Row(item_index);
				Lcd_Show_String(item_index, 0, DISP_INV, text);
				in_sub_menu = FALSE;
			}
			break;
		case KEY_RIGHT_MASK:
			switch(item_index)
			{
				case 0:	/* Channel / co2_bus */
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s", item_name[item_index]);
						Lcd_Show_String(item_index, 0, DISP_NOR, text);
						set_value = mul_co2_index();
						sprintf((char *)text, "%u", set_value);
						Lcd_Show_String(item_index, 11, DISP_INV, text);
						in_sub_menu = TRUE;
					}
					else
					{
						set_co2_channel((uint8_t)set_value);
						format_item(item_index, text);
						Lcd_Clear_Row(item_index);
						Lcd_Show_String(item_index, 0, DISP_INV, text);
						in_sub_menu = FALSE;
					}
					break;
				case 1:	/* Spt poor / alarm_setpoint */
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s", item_name[item_index]);
						Lcd_Show_String(item_index, 0, DISP_NOR, text);
						set_value = Mul_i2c_data.alarm_setpoint;
						sprintf((char *)text, "%u", set_value);
						Lcd_Show_String(item_index, 12, DISP_INV, text);
						in_sub_menu = TRUE;
					}
					else
					{
						Mul_i2c_data.alarm_setpoint = set_value;
						save_uint16_to_flash(FLASH_CO2_ALARM_SP, Mul_i2c_data.alarm_setpoint);
						format_item(item_index, text);
						Lcd_Clear_Row(item_index);
						Lcd_Show_String(item_index, 0, DISP_INV, text);
						in_sub_menu = FALSE;
					}
					break;
				case 2:	/* Spt fair / pre_alarm_setpoint */
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s", item_name[item_index]);
						Lcd_Show_String(item_index, 0, DISP_NOR, text);
						set_value = Mul_i2c_data.pre_alarm_setpoint;
						sprintf((char *)text, "%u", set_value);
						Lcd_Show_String(item_index, 12, DISP_INV, text);
						in_sub_menu = TRUE;
					}
					else
					{
						Mul_i2c_data.pre_alarm_setpoint = set_value;
						save_uint16_to_flash(FLASH_CO2_PREALARM_SP, Mul_i2c_data.pre_alarm_setpoint);
						format_item(item_index, text);
						Lcd_Clear_Row(item_index);
						Lcd_Show_String(item_index, 0, DISP_INV, text);
						in_sub_menu = FALSE;
					}
					break;
				case 3:	/* Calibration / FRC */
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s", item_name[item_index]);
						Lcd_Show_String(item_index, 0, DISP_NOR, text);
						set_value = Mul_i2c_data.I2C_Sensor[mul_co2_index()].co2_org;
						sprintf((char *)text, "%u", set_value);
						Lcd_Show_String(item_index, 15, DISP_INV, text);
						in_sub_menu = TRUE;
					}
					else
					{
						set_co2_frc_cal(set_value);
						format_item(item_index, text);
						Lcd_Clear_Row(item_index);
						Lcd_Show_String(item_index, 0, DISP_INV, text);
						in_sub_menu = FALSE;
					}
					break;
			}
			break;
		case KEY_UP_MASK:
			if(in_sub_menu == FALSE)
			{
				if(item_index)
					item_index--;
				else
					item_index = MAX_CO2_OPT_ITEMS - 1;
			}
			else if(item_index == 0)
			{
				if(set_value >= 2)
					set_value = 0;
				else
					set_value++;
				value_change = TRUE;
			}
			else
			{
				switch(key_value & KEY_FUNCTION_MASK)
				{
					case KEY_SPEED_1:
						set_value += SPEED_1;
						break;
					case KEY_SPEED_10:
						set_value += SPEED_10;
						break;
					case KEY_SPEED_50:
						set_value += SPEED_50;
						break;
					case KEY_SPEED_100:
						set_value += SPEED_100;
						break;
				}
				value_change = TRUE;
			}
			break;
		case KEY_DOWN_MASK:
			if(in_sub_menu == FALSE)
			{
				item_index++;
				item_index %= MAX_CO2_OPT_ITEMS;
			}
			else if(item_index == 0)
			{
				if(set_value == 0)
					set_value = 2;
				else
					set_value--;
				value_change = TRUE;
			}
			else
			{
				switch(key_value & KEY_FUNCTION_MASK)
				{
					case KEY_SPEED_1:
						set_value -= SPEED_1;
						break;
					case KEY_SPEED_10:
						set_value -= SPEED_10;
						break;
					case KEY_SPEED_50:
						set_value -= SPEED_50;
						break;
					case KEY_SPEED_100:
						set_value -= SPEED_100;
						break;
				}
				value_change = TRUE;
			}
			break;
	}
}
