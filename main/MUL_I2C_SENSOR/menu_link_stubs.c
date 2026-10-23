/* Temporary link stubs for MUL_I2C_SENSOR / project bring-up */

#include <stdint.h>
#include <stdbool.h>
#include "esp_attr.h"
#include "user_data.h"
#include "define.h"

/* Menu modules currently wrapped in #if 0 */
void About_init(void) {}
void About_display(void) {}
void About_keycope(uint16 key_value) { (void)key_value; }

/* ClearScreen / disp_ch / disp_str are provided by MUL_I2C_SENSOR/LCD.c */

/* Globals referenced by MUL_I2C_SENSOR */
bool enableScroll = false;
uint16_t SetPoint[3] = {0, 220, 0};

/* Bacnet is built with NEW_IO=1, so these arrays are not in that library.
   T3000 writes new_inputs/new_outputs/new_vars; flash save copies those when set. */
EXT_RAM_BSS_ATTR Str_in_point inputs[MAX_INS];
EXT_RAM_BSS_ATTR Str_out_point outputs[MAX_OUTS];
EXT_RAM_BSS_ATTR Str_variable_point vars[MAX_VARS + 12];
EXT_RAM_BSS_ATTR Str_Wireguard_point wireguard_point;

/* Normally from T10_menu/lcd.c / Menu.c (not in this build) */
lcdconfig display_lcd;
uint16_t count_lcd_time_off_delay;
void lcd_back_set(uint8_t status) { (void)status; }

uint8_t flag_receive_rmbp;

void save_flash_trendlog_num(void) {}
uint16_t get_max_trend_page(void) { return 0; }

char *get_TemcoAV_description(uint8_t index) { (void)index; return ""; }
char *get_TemcoAV_label(uint8_t index) { (void)index; return ""; }
uint16_t get_TemcoAVS_value(uint8_t index) { (void)index; return 0; }
void save_TemcoAV_value(uint16_t index, uint32_t value) { (void)index; (void)value; }

const char *bactext_object_type_name(unsigned index)
{
	(void)index;
	return "OBJ";
}
