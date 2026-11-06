#include "define.h"
#include "Menu.h"
#include <stdio.h>
#include <string.h>

static uint8 item_index = 0;
static uint8 pre_item_index = 0;
static uint16 set_value = 0;

#define MAX_PRESS_ITEMS	3
static uint8 const item_name[MAX_PRESS_ITEMS][17] =
{
	"1.Model: ",
	"2.Unit: ",
	"3.ZERO: ",
};
static uint8 const item_name_Zero[][6] =
{
	"  N  ",
	"  Y  ",
};

static void append_model(char *dst)
{
	if(Mul_i2c_data.pressure_sensor_model == WF100DPZ)
		strcat(dst, "WF100DPZ");
	else
	{
		char num[8];

		sprintf(num, "%u", (unsigned)Mul_i2c_data.pressure_sensor_model);
		strcat(dst, num);
	}
}

static void fill_item(uint8 row)
{
	strcpy((char *)text, (char *)item_name[row]);
	switch(row)
	{
		case 0:
			append_model((char *)text);
			break;
		case 1:
			if(Mul_i2c_data.pressure_unit <= pa)
				strcat((char *)text, (char *)Prs_Unit[Mul_i2c_data.pressure_unit]);
			break;
		case 2:
			strcat((char *)text, "Y/N ?");
			break;
	}
}

void Pres_init(void)
{
	uint8 i;

	Lcd_Full_Screen(0);
	for(i = 0; i < MAX_PRESS_ITEMS; i++)
	{
		fill_item(i);
		if(i == item_index)
			Lcd_Show_String(i, 0, DISP_INV, text);
		else
			Lcd_Show_String(i, 0, DISP_NOR, text);
	}
	in_sub_menu = FALSE;
}

void Pres_display(void)
{
	if(in_sub_menu == FALSE)
	{
		if(pre_item_index != item_index)
		{
			Lcd_Clear_Row(item_index);
			Lcd_Clear_Row(pre_item_index);

			fill_item(pre_item_index);
			Lcd_Show_String(pre_item_index, 0, DISP_NOR, text);
			fill_item(item_index);
			Lcd_Show_String(item_index, 0, DISP_INV, text);
			pre_item_index = item_index;
		}
	}
	else if(value_change == TRUE)
	{
		switch(item_index)
		{
			case 1:
				strcpy((char *)text, (char *)Prs_Unit[set_value]);
				Lcd_Show_String(item_index, strlen((char *)item_name[item_index]), DISP_INV, text);
				break;
			case 2:
				strcpy((char *)text, (char *)item_name_Zero[set_value]);
				Lcd_Show_String(item_index, strlen((char *)item_name[item_index]), DISP_INV, text);
				break;
		}
		value_change = FALSE;
	}
}

void Pres_keycope(uint16 key_value)
{
	key_value = key_value & KEY_SPEED_MASK;
	start_back_light(backlight_keep_seconds);
	switch(key_value)
	{
		case KEY_NON:
			break;
		case KEY_LEFT_MASK:
			if(in_sub_menu == FALSE)
				update_menu_state(MenuMain);
			else
			{
				fill_item(item_index);
				Lcd_Show_String(item_index, 0, DISP_INV, text);
				in_sub_menu = FALSE;
			}
			break;
		case KEY_RIGHT_MASK:
			if(in_sub_menu == FALSE)
			{
				if(item_index == 0)
					break;
				strcpy((char *)text, (char *)item_name[item_index]);
				Lcd_Show_String(item_index, 0, DISP_NOR, text);
				if(item_index == 1)
				{
					set_value = Mul_i2c_data.pressure_unit;
					if(set_value > pa)
						set_value = pa;
					strcpy((char *)text, (char *)Prs_Unit[set_value]);
					Lcd_Show_String(item_index, strlen((char *)item_name[item_index]), DISP_INV, text);
				}
				else
				{
					set_value = 0;
					strcpy((char *)text, (char *)item_name_Zero[set_value]);
					Lcd_Show_String(item_index, strlen((char *)item_name[item_index]), DISP_INV, text);
				}
				in_sub_menu = TRUE;
			}
			else
			{
				strcpy((char *)text, (char *)item_name[item_index]);
				if(item_index == 1)
				{
					Mul_i2c_data.pressure_unit = set_value;
					save_uint16_to_flash(FLASH_PRESSURE_UNIT, set_value);
					pressure_to_display(Mul_i2c_data.pressure_value_org, NULL);
					strcat((char *)text, (char *)Prs_Unit[Mul_i2c_data.pressure_unit]);
				}
				else
				{
					strcat((char *)text, " Y/N ");
					if(set_value)
					{
						int32_t off;

						off = (int32_t)Mul_i2c_data.pressure_value_org_offset
							- (int32_t)Mul_i2c_data.pressure_value_org;
						if(off > 32767)
							off = 32767;
						else if(off < -32768)
							off = -32768;
						Mul_i2c_data.pressure_value_org_offset = (int16_t)off;
						Mul_i2c_data.pressure_value_org = 0;
						save_int16_to_flash(FLASH_PRESSURE_OFFSET, Mul_i2c_data.pressure_value_org_offset);
						pressure_to_display(Mul_i2c_data.pressure_value_org, NULL);
					}
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
					item_index = MAX_PRESS_ITEMS - 1;
			}
			else if(item_index == 1)
			{
				set_value++;
				if(set_value > pa)
					set_value = 0;
				value_change = TRUE;
			}
			else if(item_index == 2)
			{
				set_value = set_value ? 0 : 1;
				value_change = TRUE;
			}
			break;
		case KEY_DOWN_MASK:
			if(in_sub_menu == FALSE)
			{
				item_index++;
				item_index %= MAX_PRESS_ITEMS;
			}
			else if(item_index == 1)
			{
				if(set_value)
					set_value--;
				else
					set_value = pa;
				value_change = TRUE;
			}
			else if(item_index == 2)
			{
				set_value = set_value ? 0 : 1;
				value_change = TRUE;
			}
			break;
	}
}
