#include "define.h"
#include "Menu.h"

#define MAX_MAIN_ITEMS	6
static uint8 const item_name[MAX_MAIN_ITEMS][16] = 
{
	"1.CO2",
	"2.Temperature",
	"3.Humidity",
	"4.Miscellaneous",
	"5.Pressure",
	"6.PM25"
};

static uint8 const item[] = {MenuCo2Option, MenuTemp_Conf, MenuHum_Conf, MenuMisc, MenuPresConf}; 

static uint8 item_index = 0;
static uint8 pre_item_index = 0;
static uint8 visible_items[MAX_MAIN_ITEMS];
static uint8 visible_count;

static void build_main_items(void)
{
	uint8 i;

	if(Modbus.mul_i2c_mode == MUL_I2C_MODE_HUM)
	{
		visible_items[0] = 1;
		visible_items[1] = 2;
		visible_items[2] = 3;
		visible_count = 3;
	}
	else if(Modbus.mul_i2c_mode == MUL_I2C_MODE_CO2)
	{
		visible_items[0] = 0;
		visible_items[1] = 1;
		visible_items[2] = 2;
		visible_items[3] = 3;
		visible_count = 4;
	}
	else if(Modbus.mul_i2c_mode == MUL_I2C_MODE_PRESSURE)
	{
		visible_items[0] = 4;
		visible_items[1] = 3;
		visible_count = 2;
	}
	else
	{
		for(i = 0; i < MAX_MAIN_ITEMS; i++)
			visible_items[i] = i;
		visible_count = MAX_MAIN_ITEMS;
	}
	if(item_index >= visible_count)
		item_index = 0;
	if(pre_item_index >= visible_count)
		pre_item_index = item_index;
}

static uint8 *main_item_label(uint8 row)
{
	uint8 i;
	const uint8 *src;

	src = item_name[visible_items[row]];
	for(i = 0; src[i] != 0 && i < sizeof(text) - 1; i++)
		text[i] = src[i];
	text[i] = 0;
	if(Modbus.mul_i2c_mode == MUL_I2C_MODE_HUM || Modbus.mul_i2c_mode == MUL_I2C_MODE_PRESSURE)
		text[0] = (uint8)('1' + row);
	return text;
}

void Co2_Main_init(void)
{
	uint8 i;

	build_main_items();
	if(use_password == FALSE)
		menu_password = TRUE;

	Lcd_Full_Screen(0);
	for(i = 0; i < visible_count; i++)
	{
		if((i == item_index) && (menu_password == TRUE))
			Lcd_Show_String(i, 0, DISP_INV, main_item_label(i));
		else
			Lcd_Show_String(i, 0, DISP_NOR, main_item_label(i));
	}

	if(menu_password == FALSE)
	{
		Lcd_Show_String(1, 5, DISP_INV, (uint8 *)"  PASSWORD:  ");
		Lcd_Show_String(2, 5, DISP_INV, (uint8 *)"    ****     ");
		password_buffer[0] = '*';
		password_buffer[1] = '*';
		password_buffer[2] = '*';
		password_buffer[3] = '*';
		password_index = 0;
		cursor_on(2, 9, '*', ' ');
	}
}

void Co2_Main_display(void)
{
	if(menu_password == FALSE)
	{
		if(value_change == TRUE)
		{
			cursor.on_byte = password_buffer[password_index];
			value_change = FALSE;
		}
	}
	else
	{
		if(pre_item_index != item_index)
		{
			Lcd_Show_String(pre_item_index, 0, DISP_NOR, main_item_label(pre_item_index));
			Lcd_Show_String(item_index, 0, DISP_INV, main_item_label(item_index));
			pre_item_index = item_index;
		}
	}
}

void Co2_Main_keycope(uint16 key_value)
{
	start_back_light(backlight_keep_seconds);
	if(menu_password == FALSE)
	{
		switch(key_value & KEY_SPEED_MASK)
		{
			case KEY_NON:
				// do nothing
				break;
			case KEY_UP_MASK:
				if((password_buffer[password_index] >= '9') || (password_buffer[password_index] < '0') || (password_buffer[password_index] == '*'))
					password_buffer[password_index] = '0';
				else
					password_buffer[password_index]++;

				value_change = TRUE;
				break;
			case KEY_DOWN_MASK:
				if((password_buffer[password_index] <= '0') || (password_buffer[password_index] > '9') || (password_buffer[password_index] == '*'))
					password_buffer[password_index] = '9';
				else
					password_buffer[password_index]--;

				value_change = TRUE;
				break;
			case KEY_LEFT_MASK:
				if(password_index == 0)
				{
					cursor_off();
					update_menu_state(MenuIdle);
				}
				else
				{
					Lcd_Write_Char(cursor.row, cursor.line--, password_buffer[password_index--], DISP_INV);
					cursor.on_byte = password_buffer[password_index];
				}
				break;
			case KEY_RIGHT_MASK:
				if(password_index < (MAX_PASSWORD_DIGITALS - 1))
				{
					Lcd_Write_Char(cursor.row, cursor.line++, password_buffer[password_index++], DISP_INV);
					cursor.on_byte = password_buffer[password_index];
				}
				else
				{
					if((password_buffer[0] == user_password[0]) && (password_buffer[1] == user_password[1]) 
					&& (password_buffer[2] == user_password[2]) && (password_buffer[3] == user_password[3]))
					{
						cursor_off();
						menu_password = TRUE;
					}
					Co2_Main_init();
				}
				break;
		}
	}
	else
	{
		switch(key_value & KEY_SPEED_MASK)
		{
			case KEY_NON:
				// do nothing
				break;
			case KEY_UP_MASK:
				if(item_index)
					item_index--;
				else
					item_index = visible_count - 1;
				break;
			case KEY_DOWN_MASK:
				item_index++;
				item_index %= visible_count;
				break;
			case KEY_LEFT_MASK:
				update_menu_state(MenuIdle);
				break;
			case KEY_RIGHT_MASK:
			{
				uint8 src;

				src = visible_items[item_index];
				if(src >= (sizeof(item) / sizeof(item[0])))
					break;
				if(Modbus.mul_i2c_mode == MUL_I2C_MODE_PM25)
				{
					if(item[src] == MenuMisc)
						update_menu_state(item[src]);
				}
				else
					update_menu_state(item[src]);
				break;
			}
		}
	}
}



