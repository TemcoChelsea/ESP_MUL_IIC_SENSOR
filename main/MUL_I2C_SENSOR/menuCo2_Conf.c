#include "define.h"
#include "Menu.h"

static uint8 item_index = 0;
static uint8 pre_item_index = 0;

uint8 const   online_text[] = " ONLINE";
uint8 const   offline_text[] = " OFFLINE";

static uint8 co2_bus_online(void)
{
	uint8_t t;

	t = Mul_i2c_data.i2c_sensor_type[mul_co2_index()];
	return (t == E_I2C_SCD4X)
		|| (t == E_I2C_SHT3X_SCD4X)
		|| (t == E_I2C_SHT4X_SCD4X);
}

static void display_zone_text(uint8 index)
{
	(void)index;
	if(co2_bus_online())
		sprintf((char *)text, "%s%s", internal_text, online_text);
	else
		sprintf((char *)text, "%s%s", internal_text, offline_text);

	Lcd_Clear_Row(0);
	Lcd_Show_String(0, 0, DISP_INV, text);
}

void Co2_Conf_init(void)
{
	Lcd_Full_Screen(0);
	item_index = 0;
	pre_item_index = 0;
	display_zone_text(0);
	previous_internal_co2_exist = co2_bus_online();
	pre_db_ctr = 1;
}

void Co2_Conf_display(void)
{
	uint8 online;

	online = co2_bus_online();
	if(previous_internal_co2_exist != online)
	{
		previous_internal_co2_exist = online;
		display_zone_text(0);
	}
}

void Co2_Conf_keycope(uint16 key_value)
{
	start_back_light(backlight_keep_seconds);
	switch(key_value & KEY_SPEED_MASK)
	{
		case KEY_NON:
			break;
		case KEY_LEFT_MASK:
			update_menu_state(MenuMain);
			break;
		case KEY_RIGHT_MASK:
			update_menu_state(MenuCo2Option);
			break;
		case KEY_UP_MASK:
		case KEY_DOWN_MASK:
			break;
	}
}
