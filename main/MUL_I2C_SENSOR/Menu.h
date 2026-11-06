
#ifndef __MENU_H__

#define	__MENU_H__
#include "types.h"
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "flash.h"
#include "key.h"

#include "lcd.h"
#include "menu.h"
#include "menuIdle.h"
#include "menuMain.h"
#include "menuCo2_Conf.h"
#include "menuTemp_Conf.h"
#include "menuHum_Conf.h"
#include "menuAbout.h"
#include "menuMisc.h"
#include "menuCo2_Opt.h"
#include "Menu_Pres_Conf.h"
#include "MenuSet.h"


#define MENU_BLOCK_SECONDS_DEFAULT		60
#define BACKLIGHT_KEEP_SECONDS_DEFAULT	255
#define SWTIMER_COUNT_SECOND	 configTICK_RATE_HZ   

struct _MENU_STATE_
{
	uint8 Index;
	void (*KeyCope)(U16_T);								
	void (*InitAction)(void);
	void (*DisplayPeriod)(void);
	uint8 BlockTime;
};

extern struct _MENU_STATE_ CurrentState;

extern uint8 menu_system_start;
extern uint8 in_sub_menu;
extern uint8 value_change;
extern uint8 previous_internal_co2_exist;
extern uint8 pre_db_ctr;

#define MAX_PASSWORD_DIGITALS	4
extern uint8 menu_password;
extern uint8 use_password;
extern uint8 password_index;
extern uint8 password_buffer[4];
extern uint8 user_password[4];

extern uint8 text[50];
extern uint8 int_text[21];
//extern uint16 set_value;

extern uint8 const   internal_text[];
extern uint8 const   external_text[];
extern uint8 const   ppm_text[];
//extern uint8 const   co2_text[];
extern uint8 const   int_space[];
//extern uint8 const   online_text[];
//extern uint8 const   offline_text[];
//extern uint8 warming_text[13];

extern uint8 menu_block_seconds;
extern uint8 backlight_keep_seconds; 
extern QueueHandle_t xMutex,IicMutex;

extern uint8_t lcd_i2c_sensor_index;
extern uint8_t setpoint_step;
extern uint8_t setpoint_max_c;
extern uint8_t setpoint_min_c;
extern uint8_t setpoint_max_f;
extern uint8_t setpoint_min_f;
extern uint8 ghost_ip_addr[4];
extern uint8 ghost_mask_addr[4];
extern uint8 ghost_gate_addr[4];
extern uint16 ghost_listen_port;
extern uint8 ghost_ip_mode;
 enum
{
	MenuIdle = 0,
		MenuMain,
			MenuCo2_Conf,
				MenuCo2Option,
			MenuTemp_Conf,
			MenuHum_Conf,
			MenuMisc,
 			MenuPresConf,
	MenuEnd,
};


void show_system_info(void);
void update_menu_state(uint8 MenuId);
void exit_request_password(void);
void menu_init(void);
void vStartMenuTask(unsigned char uxPriority);
//void vStartScrollingTask(unsigned char uxPriority);
void print(char *p);
void menu_itoa(uint16 num, uint8 *p, uint8 dot);
#ifdef itoa
#undef itoa
#endif
#define itoa(num, p, dot) menu_itoa((num), (p), (dot))

#endif

