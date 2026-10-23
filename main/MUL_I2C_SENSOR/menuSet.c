#include "bacnet.h"
#include "define.h"
#include "Menu.h"
#include "mstpdef.h"
#include "rtc.h"
#include "user_data.h"
#include "modbus.h"
#include "flash.h"

static uint8 item_index = 0;
static uint8 pre_item_index = 0;

static uint16 set_value;

static uint8 sub_menu_index = 0;

bool alarmEnable = false;

#define IP_NO_CHANGE	0
#define IP_CHANGED		1
#define IP_ENABLE		2
#define IP_DISABLE		3
extern uint8 update_flag;
uint8 ipconfig_in_menu = IP_NO_CHANGE;
uint8 ip_changed_in_menu = 0;
uint8 subnet_changed_in_menu = 0;
uint8 gateway_changed_in_menu = 0;
uint8 port_changed_in_menu = 0; 
#define MAX_MISC_ITEMS	15
#define SET_MENU_ROWS	7
static uint8 const item_name[MAX_MISC_ITEMS][19] = 
{
	"1.Ip mode: ",
	"2.Ip:",
	"3.Msk:",
	"4.Gat:",
	"5.Port:",
	"6.New ip: ",
	"7.Device ID: ",
	"8.Baudrate: ",
  "9.Protocol:  ",
	"10.Date:",
	"11.Time: ",
//	"11.Alarm on sec: ",
//	"12.Alarm off sec: ",
	"12.Use password: ",
	"13.Password: ",
	"14.Enable Alarm: ",
	"15.Status Line:  "
};
static uint8 const protocol_text[2][7] = 
{
	"Mstp",
	"Modbus", 
}; 
static uint8 const ip_mode_text[2][7] = 
{
	"Static",
	"Dhcp",
};

static uint8 const new_ip_status_text[4][10] = 
{
	"No-change",
	"Y/N",
	"Confirm",
	"Cannel",
};

  uint8 const baudrate_text[6][7] = 
{
	"9600",
	"19200",
	"38400",
	"57600",
	"115200",
	"76800"
	
};

static uint8 const YesOrNo[2][4] = 
{
	"No",
	"Yes",
};

static uint8 const PARITY_TEXT[3][3]=
{
	"-N",
	"-O",
	"-E",
};

uint8_t ghost_ip_mode;
uint16_t ghost_listen_port;
uint8_t ghost_ip_addr[4];
uint8_t ghost_mask_addr[4];
uint8_t ghost_gate_addr[4];

void Misc_init(void)
{
	uint8 i, display_lines;

	if(ipconfig_in_menu == IP_DISABLE)
	{
		
		ghost_ip_mode = Modbus.tcp_type;

		for(i=0; i<4; i++)
		{
			ghost_ip_addr[i] = Modbus.ip_addr[i];
			ghost_mask_addr[i] = Modbus.subnet[i];
			ghost_gate_addr[i] = Modbus.getway[i]; 					
		} 
		ghost_listen_port = Modbus.tcp_port;

		ipconfig_in_menu = IP_NO_CHANGE;
	}


	Lcd_Full_Screen(0);

	if(((MAX_MISC_ITEMS % SET_MENU_ROWS) == 0) || (item_index < (MAX_MISC_ITEMS / SET_MENU_ROWS * SET_MENU_ROWS)))
		display_lines = SET_MENU_ROWS;
	else
		display_lines = MAX_MISC_ITEMS % SET_MENU_ROWS;

	for(i = 0; i < display_lines; i++)
	{
		strcpy((char *)text, (char *)item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS]);
		switch(i + item_index / SET_MENU_ROWS * SET_MENU_ROWS)
		{
			case 0:
//				sprintf((char *)text, "%s%s", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], ip_mode_text[ghost_ip_mode]);
				strcat((char *)text, (char *)ip_mode_text[ghost_ip_mode]);	
				break;
			case 1:
//				sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], (uint16)ghost_ip_addr[0], (uint16)ghost_ip_addr[1], (uint16)ghost_ip_addr[2], (uint16)ghost_ip_addr[3]);
				itoa(ghost_ip_addr[0], int_text, 0);
				strcat((char *)text, (char *)int_text);
				strcat((char *)text,  ".");
				itoa(ghost_ip_addr[1], int_text, 0);
				strcat((char *)text, (char *)int_text);
				strcat((char *)text, (char *)".");
				itoa(ghost_ip_addr[2], int_text, 0);
				strcat((char *)text, (char *)int_text);
				strcat((char *)text, (char *)".");
				itoa(ghost_ip_addr[3], int_text, 0);
				strcat((char *)text, (char *)int_text);
				break;
			case 2:
//				sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], (uint16)ghost_mask_addr[0], (uint16)ghost_mask_addr[1], (uint16)ghost_mask_addr[2], (uint16)ghost_mask_addr[3]);
				itoa(ghost_mask_addr[0], int_text, 0);
				strcat((char *)text, (char *)int_text);
				strcat((char *)text, (char *)".");
				itoa(ghost_mask_addr[1], int_text, 0);
				strcat((char *)text, (char *)int_text);
				strcat((char *)text, (char *)".");
				itoa(ghost_mask_addr[2], int_text, 0);
				strcat((char *)text, (char *)int_text);
				strcat((char *)text, (char *)".");
				itoa(ghost_mask_addr[3], int_text, 0);
				strcat((char *)text, (char *)int_text);
				break;
			case 3:
//				sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], (uint16)ghost_gate_addr[0], (uint16)ghost_gate_addr[1], (uint16)ghost_gate_addr[2], (uint16)ghost_gate_addr[3]);
				itoa(ghost_gate_addr[0], int_text, 0);
				strcat((char *)text, (char *)int_text);
				strcat((char *)text, (char *)".");
				itoa(ghost_gate_addr[1], int_text, 0);
				strcat((char *)text, (char *)int_text);
				strcat((char *)text, (char *)".");
				itoa(ghost_gate_addr[2], int_text, 0);
				strcat((char *)text, (char *)int_text);
				strcat((char *)text, (char *)".");
				itoa(ghost_gate_addr[3], int_text, 0);
				strcat((char *)text, (char *)int_text);
				break;
			case 4:
//				sprintf((char *)text, "%s%u", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], ghost_listen_port);
				itoa(ghost_listen_port, int_text, 0);
				strcat((char *)text, (char *)int_text);
				break;
			case 5:	
//				sprintf((char *)text, "%s%s", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], new_ip_status_text[ipconfig_in_menu]);
				strcat((char *)text, (char *)new_ip_status_text[ipconfig_in_menu]);	
				break;
			case 6: // Modbus ID
//				sprintf((char *)text, "%s%u", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], (uint16)Modbus.address);
				itoa(Modbus.address, int_text, 0);
				strcat((char *)text, (char *)int_text);
				break;
			case 7:
//				sprintf((char *)text, "%s%s", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], baudrate_text[Modbus.baud]);
				strcat((char *)text, (char *)baudrate_text[Modbus.baudrate[0]]);
				strcat((char *)text, (char *)PARITY_TEXT[0/*uart1_parity*/]);
				break;
			case 8:
				if(Modbus.com_config[0] == MODBUS_SLAVE) 
					strcat((char *)text, (char *)protocol_text[1]);
				else
					strcat((char *)text, (char *)protocol_text[0]);
				break; 
			case 9: // Date
				sprintf((char *)int_text, "%04d-%02d-%02d",(uint16)rtc_date.year, (uint16)rtc_date.month, (uint16)rtc_date.day);
				strcat((char *)text, (char *)int_text);
				break;
			case 10: // Time
				sprintf((char *)int_text, "%02d:%02d",(uint16)rtc_date.hour, (uint16)rtc_date.minute); 
				strcat((char *)text, (char *)int_text);
				break;
			case 11:
//				sprintf((char *)text, "%s%s", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], YesOrNo[use_password]);
				strcat((char *)text, (char *)YesOrNo[use_password]);
				break;
			case 12:
//				sprintf((char *)text, "%s%c%c%c%c", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], user_password[0], user_password[1], user_password[2], user_password[3]);
//				strcpy((char *)text, (char *)item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS]);
				strncat((char *)text, (char *)user_password, 4);
				break;
			
			case 13:
				strcat((char *)text, (char *)YesOrNo[alarmEnable]);
				break;
			case 14:
				strcat((char *)text, (char *)YesOrNo[enableScroll]);
				break;
		}
		
		if(i == (item_index % SET_MENU_ROWS))
			Lcd_Show_String(i, 0, DISP_INV, text);
		else
			Lcd_Show_String(i, 0, DISP_NOR, text);
	}

	in_sub_menu = FALSE;
}

void Misc_display(void)
{
	ipconfig_in_menu = ip_changed_in_menu | subnet_changed_in_menu | gateway_changed_in_menu | port_changed_in_menu;
	if(in_sub_menu == FALSE)
	{
		if(pre_item_index != item_index)
		{
			strcpy((char *)text, (char *)item_name[pre_item_index]);
			switch(pre_item_index)
			{
				case 0:
	//				sprintf((char *)text, "%s%s", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], ip_mode_text[ghost_ip_mode]);
					strcat((char *)text, (char *)ip_mode_text[ghost_ip_mode]);	
					break;
				case 1:
	//				sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], (uint16)ghost_ip_addr[0], (uint16)ghost_ip_addr[1], (uint16)ghost_ip_addr[2], (uint16)ghost_ip_addr[3]);
					itoa(ghost_ip_addr[0], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(ghost_ip_addr[1], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(ghost_ip_addr[2], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(ghost_ip_addr[3], int_text, 0);
					strcat((char *)text, (char *)int_text);
					break;
				case 2:
	//				sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], (uint16)ghost_mask_addr[0], (uint16)ghost_mask_addr[1], (uint16)ghost_mask_addr[2], (uint16)ghost_mask_addr[3]);
					itoa(ghost_mask_addr[0], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(ghost_mask_addr[1], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(ghost_mask_addr[2], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(ghost_mask_addr[3], int_text, 0);
					strcat((char *)text, (char *)int_text);
					break;
				case 3:
	//				sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], (uint16)ghost_gate_addr[0], (uint16)ghost_gate_addr[1], (uint16)ghost_gate_addr[2], (uint16)ghost_gate_addr[3]);
					itoa(ghost_gate_addr[0], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(ghost_gate_addr[1], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(ghost_gate_addr[2], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(ghost_gate_addr[3], int_text, 0);
					strcat((char *)text, (char *)int_text);
					break;
				case 4:
	//				sprintf((char *)text, "%s%u", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], ghost_listen_port);
					itoa(ghost_listen_port, int_text, 0);
					strcat((char *)text, (char *)int_text);
					break;
				case 5:	
	//				sprintf((char *)text, "%s%s", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], new_ip_status_text[ipconfig_in_menu]);
					strcat((char *)text, (char *)new_ip_status_text[ipconfig_in_menu]);	
					break;
				case 6: // Modbus ID
	//				sprintf((char *)text, "%s%u", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], (uint16)Modbus.address);
					itoa(Modbus.address, int_text, 0);
					strcat((char *)text, (char *)int_text);
					break;
				case 7:
	//				sprintf((char *)text, "%s%s", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], baudrate_text[Modbus.baud]);
					strcat((char *)text, (char *)baudrate_text[Modbus.baudrate[0]]);
				    strcat((char *)text, (char *)PARITY_TEXT[0/*uart1_parity*/]);
					break;
				
				case 8:
					if(Modbus.com_config[0] == MODBUS_SLAVE) 
						strcat((char *)text, (char *)protocol_text[1]);
					else
						strcat((char *)text, (char *)protocol_text[0]);
					break; 
				case 9: // Date
	//				sprintf((char *)text, "%s%04d-%02d-%02d", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], (uint16)rtc_date.year, (uint16)rtc_date.month, (uint16)rtc_date.day);
//					itoa(rtc_date.year, int_text, 0);
//					strcat((char *)text, (char *)int_text);
//					strcat((char *)text, (char *)"-");
//					itoa(rtc_date.month, int_text, 0);
//					strcat((char *)text, (char *)int_text);
//					strcat((char *)text, (char *)"-");
//					itoa(rtc_date.day, int_text, 0);
					sprintf((char *)int_text, "%04d-%02d-%02d",(uint16)rtc_date.year, (uint16)rtc_date.month, (uint16)rtc_date.day);
					strcat((char *)text, (char *)int_text);
					break;
				case 10: // Time
	//				sprintf((char *)text, "%s%02d:%02d", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], (uint16)rtc_date.hour, (uint16)rtc_date.minute);
					 
//					itoa(rtc_date.hour, int_text, 0);
//					strcat((char *)text, (char *)int_text);
//					strcat((char *)text, (char *)":");
//					itoa(rtc_date.minute, int_text, 0);
					sprintf((char *)int_text, "%02d:%02d",(uint16)rtc_date.hour, (uint16)rtc_date.minute);
					strcat((char *)text, (char *)int_text);
					break;
//				case 10:	// Ring on time
//	//				sprintf((char *)text, "%s%u", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], (uint16)pre_alarm_on_time);
//					itoa(pre_alarm_on_time, int_text, 0);
//					strcat((char *)text, (char *)int_text);
//					break;
//				case 11: // Ring off time
//	//				sprintf((char *)text, "%s%u", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], (uint16)pre_alarm_off_time);
//					itoa(pre_alarm_off_time, int_text, 0);
//					strcat((char *)text, (char *)int_text);
//					break;
				case 11:
	//				sprintf((char *)text, "%s%s", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], YesOrNo[use_password]);
					strcat((char *)text, (char *)YesOrNo[use_password]);
					break;
				case 12:
	//				sprintf((char *)text, "%s%c%c%c%c", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], user_password[0], user_password[1], user_password[2], user_password[3]);
			 
					strncat((char *)text, (char *)user_password, 4);
					break;
				case 13:
					strcat((char *)text, (char *)YesOrNo[alarmEnable]);
					break;
				case 14:
					strcat((char *)text, (char *)YesOrNo[enableScroll]);
					break;
			}
			Lcd_Clear_Row(pre_item_index % SET_MENU_ROWS);
			Lcd_Show_String(pre_item_index % SET_MENU_ROWS, 0, DISP_NOR, text);
			strcpy((char *)text, (char *)item_name[item_index]);
			switch(item_index)
			{
				case 0:
	//				sprintf((char *)text, "%s%s", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], ip_mode_text[ghost_ip_mode]);
					strcat((char *)text, (char *)ip_mode_text[ghost_ip_mode]);	
					break;
				case 1:
	//				sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], (uint16)ghost_ip_addr[0], (uint16)ghost_ip_addr[1], (uint16)ghost_ip_addr[2], (uint16)ghost_ip_addr[3]);
					itoa(ghost_ip_addr[0], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(ghost_ip_addr[1], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(ghost_ip_addr[2], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(ghost_ip_addr[3], int_text, 0);
					strcat((char *)text, (char *)int_text);
					break;
				case 2:
	//				sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], (uint16)ghost_mask_addr[0], (uint16)ghost_mask_addr[1], (uint16)ghost_mask_addr[2], (uint16)ghost_mask_addr[3]);
					itoa(ghost_mask_addr[0], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(ghost_mask_addr[1], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(ghost_mask_addr[2], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(ghost_mask_addr[3], int_text, 0);
					strcat((char *)text, (char *)int_text);
					break;
				case 3:
	//				sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], (uint16)ghost_gate_addr[0], (uint16)ghost_gate_addr[1], (uint16)ghost_gate_addr[2], (uint16)ghost_gate_addr[3]);
					itoa(ghost_gate_addr[0], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(ghost_gate_addr[1], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(ghost_gate_addr[2], int_text, 0);
					strcat((char *)text, (char *)int_text);
					strcat((char *)text, (char *)".");
					itoa(ghost_gate_addr[3], int_text, 0);
					strcat((char *)text, (char *)int_text);
					break;
				case 4:
	//				sprintf((char *)text, "%s%u", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], ghost_listen_port);
					itoa(ghost_listen_port, int_text, 0);
					strcat((char *)text, (char *)int_text);
					break;
				case 5:	
	//				sprintf((char *)text, "%s%s", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], new_ip_status_text[ipconfig_in_menu]);
					strcat((char *)text, (char *)new_ip_status_text[ipconfig_in_menu]);	
					break;
				case 6: // Modbus ID
	//				sprintf((char *)text, "%s%u", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], (uint16)Modbus.address);
					itoa(Modbus.address, int_text, 0);
					strcat((char *)text, (char *)int_text);
					break;
				case 7:
	//				sprintf((char *)text, "%s%s", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], baudrate_text[Modbus.baud]);
					strcat((char *)text, (char *)baudrate_text[Modbus.baudrate[0]]);
				    strcat((char *)text, (char *)PARITY_TEXT[0/*uart1_parity*/]);
					break;
				case 8:
					if(Modbus.com_config[0] == MODBUS_SLAVE) 
						strcat((char *)text, (char *)protocol_text[1]);
					else
						strcat((char *)text, (char *)protocol_text[0]);
					break;
				case 9: // Date
	//				sprintf((char *)text, "%s%04d-%02d-%02d", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], (uint16)rtc_date.year, (uint16)rtc_date.month, (uint16)rtc_date.day);
//					itoa(rtc_date.year, int_text, 0);
//					strcat((char *)text, (char *)int_text);
//					strcat((char *)text, (char *)"-");
//					itoa(rtc_date.month, int_text, 0);
//					strcat((char *)text, (char *)int_text);
//					strcat((char *)text, (char *)"-");
//					itoa(rtc_date.day, int_text, 0);
					sprintf((char *)int_text, "%04d-%02d-%02d",(uint16)rtc_date.year, (uint16)rtc_date.month, (uint16)rtc_date.day);
					strcat((char *)text, (char *)int_text);
					break;
				case 10: // Time
	//				sprintf((char *)text, "%s%02d:%02d", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], (uint16)rtc_date.hour, (uint16)rtc_date.minute);
					 
//					itoa(rtc_date.hour, int_text, 0);
//					strcat((char *)text, (char *)int_text);
//					strcat((char *)text, (char *)":");
//					itoa(rtc_date.minute, int_text, 0);
					sprintf((char *)int_text, "%02d:%02d",(uint16)rtc_date.hour, (uint16)rtc_date.minute);
					strcat((char *)text, (char *)int_text);
					break;
//				case 10:	// Ring on time
//	//				sprintf((char *)text, "%s%u", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], (uint16)pre_alarm_on_time);
//					itoa(pre_alarm_on_time, int_text, 0);
//					strcat((char *)text, (char *)int_text);
//					break;
//				case 11: // Ring off time
//	//				sprintf((char *)text, "%s%u", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], (uint16)pre_alarm_off_time);
//					itoa(pre_alarm_off_time, int_text, 0);
//					strcat((char *)text, (char *)int_text);
//					break;
				case 11:
	//				sprintf((char *)text, "%s%s", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], YesOrNo[use_password]);
					strcat((char *)text, (char *)YesOrNo[use_password]);
					break;
				case 12:
	//				sprintf((char *)text, "%s%c%c%c%c", item_name[i + item_index / SET_MENU_ROWS * SET_MENU_ROWS], user_password[0], user_password[1], user_password[2], user_password[3]);
				 
					strncat((char *)text, (char *)user_password, 4);
					break;
				
				case 13:
					strcat((char *)text, (char *)YesOrNo[alarmEnable]);
					break;
				case 14:
					strcat((char *)text, (char *)YesOrNo[enableScroll]);
					break;
			}
			Lcd_Clear_Row(item_index % SET_MENU_ROWS);
 			Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_INV, text);

			pre_item_index = item_index;
		}
		else
		{
//			if((item_index >= 5) && (item_index < 10))// in the time menu page
//			{
////				sprintf((char *)text, "%s%s", item_name[5], new_ip_status_text[ipconfig_in_menu]);
//				strcpy((char *)text, (char *)item_name[5]); 
//				strcat((char *)text, (char *)new_ip_status_text[ipconfig_in_menu]);
//				if(item_index == 5)
//					Lcd_Show_String(5 % SET_MENU_ROWS, 0, DISP_INV, text);
//				else
//					Lcd_Show_String(5 % SET_MENU_ROWS, 0, DISP_NOR, text);

//		//		sprintf((char *)text, "%s%02d:%02d", item_name[9], (uint16)rtc_date.hour, (uint16)rtc_date.minute);
//				strcpy((char *)text, (char *)item_name[item_index]);
//				itoa(rtc_date.hour, int_text, 0);
//				strcat((char *)text, (char *)int_text);
//				itoa(rtc_date.minute, int_text, 0);
//				strcat((char *)text, (char *)int_text);
//				if(item_index == 9)
//					Lcd_Show_String(9 % SET_MENU_ROWS, 0, DISP_INV, text);
//				else
//					Lcd_Show_String(9 % SET_MENU_ROWS, 0, DISP_NOR, text);
//			}
		}
	}
	else
	{
		if(value_change == TRUE)
		{
			switch(item_index)
			{
				case 0:
 					sprintf((char *)text, "%s", ip_mode_text[set_value]); 
					Lcd_Show_String(item_index % SET_MENU_ROWS, 11, DISP_NOR,(uint8 *)"      ");
					Lcd_Show_String(item_index % SET_MENU_ROWS, 11, DISP_INV, text);
					break;
				case 1: // IP
					sprintf((char *)text, "%03d", set_value);
					if(sub_menu_index == 0)
						Lcd_Show_String(item_index % SET_MENU_ROWS, 5, DISP_INV, text);
					else if(sub_menu_index == 1)
						Lcd_Show_String(item_index % SET_MENU_ROWS, 9, DISP_INV, text);
					else if(sub_menu_index == 2)
						Lcd_Show_String(item_index % SET_MENU_ROWS, 13, DISP_INV, text);
					else
						Lcd_Show_String(item_index % SET_MENU_ROWS, 17, DISP_INV, text);
					break;
				case 2: // MASK
				case 3:	// GATE
					sprintf((char *)text, "%03d", set_value);
					if(sub_menu_index == 0)
						Lcd_Show_String(item_index % SET_MENU_ROWS, 6, DISP_INV, text);
					else if(sub_menu_index == 1)
						Lcd_Show_String(item_index % SET_MENU_ROWS, 10, DISP_INV, text);
					else if(sub_menu_index == 2)
						Lcd_Show_String(item_index % SET_MENU_ROWS, 14, DISP_INV, text);
					else
						Lcd_Show_String(item_index % SET_MENU_ROWS, 18, DISP_INV, text);
					break;
				case 4:	// PORT
					sprintf((char *)text, "%u", set_value);
					Lcd_Show_String(item_index % SET_MENU_ROWS, 7, DISP_NOR, (uint8 *)"     ");
					Lcd_Show_String(item_index % SET_MENU_ROWS, 7, DISP_INV, text);
					break;
				case 5:
					sprintf((char *)text, "%s", new_ip_status_text[set_value]);
					Lcd_Show_String(item_index % SET_MENU_ROWS, 10, DISP_NOR, (uint8 *)"         ");
					Lcd_Show_String(item_index % SET_MENU_ROWS, 10, DISP_INV, text);
					break;
				case 6:	// ID
					sprintf((char *)text, "%u", set_value);
					Lcd_Show_String(item_index % SET_MENU_ROWS, 13, DISP_NOR, (uint8 *)"   ");
					Lcd_Show_String(item_index % SET_MENU_ROWS, 13, DISP_INV, text);
					break;
				case 7:	// BAUDRATE
					Lcd_Show_String(item_index % SET_MENU_ROWS, 12, DISP_NOR, (uint8 *)"        ");
					Lcd_Show_String(item_index % SET_MENU_ROWS, 12, DISP_INV, (uint8 *)baudrate_text[set_value]);
					break;
				case 8:
					Lcd_Show_String(item_index % SET_MENU_ROWS, 13, DISP_NOR, (uint8 *)"      ");
					Lcd_Show_String(item_index % SET_MENU_ROWS, 13, DISP_INV, (uint8 *)protocol_text[set_value]); 
					break;
				case 9:	// DATE
					
					if(sub_menu_index == 0)
					{
						sprintf((char *)text, "%04d", set_value);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 8, DISP_INV, text);
					}
					else if(sub_menu_index == 1)
					{
						sprintf((char *)text, "%02d", set_value);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 13, DISP_INV, text);
					}
					else if(sub_menu_index == 2)
					{
						sprintf((char *)text, "%02d", set_value);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 16, DISP_INV, text);
					}
					break;
				case 10:	// TIME
					sprintf((char *)text, "%02d", set_value);
					if(sub_menu_index == 0)
						Lcd_Show_String(item_index % SET_MENU_ROWS, 9, DISP_INV, text);
					else if(sub_menu_index == 1)
						Lcd_Show_String(item_index % SET_MENU_ROWS, 12, DISP_INV, text);
					break;
//				case 10:	// RING ON TIME
//					sprintf((char *)text, "%u", set_value);
//					Lcd_Show_String(item_index % SET_MENU_ROWS, 17, DISP_NOR, (uint8 *)"   ");
//					Lcd_Show_String(item_index % SET_MENU_ROWS, 17, DISP_INV, text);
//					break;
//				case 11:	// RING OFF TIME
//					sprintf((char *)text, "%u", set_value);
//					Lcd_Show_String(item_index % SET_MENU_ROWS, 18, DISP_NOR, (uint8 *)"   ");
//					Lcd_Show_String(item_index % SET_MENU_ROWS, 18, DISP_INV, text);
//					break;
				case 11: // PASSWORD ENABLE
					Lcd_Show_String(item_index % SET_MENU_ROWS, 17, DISP_NOR, (uint8 *)"   ");
					Lcd_Show_String(item_index % SET_MENU_ROWS, 17, DISP_INV, (uint8 *)YesOrNo[set_value]);
					break;
				case 12: // PASSWORD TEXT
					cursor.on_byte = (uint8)set_value;
					break;
				
				case 13:
				case 14:
					//strcat((char *)text, (char *)YesOrNo[alarmEnable]);
					Lcd_Show_String(item_index % SET_MENU_ROWS, 17, DISP_NOR, (uint8 *)"   ");
					Lcd_Show_String(item_index % SET_MENU_ROWS, 17, DISP_INV, (uint8 *)YesOrNo[set_value]);
					break;
			}

			value_change = FALSE;
		}
	}
}

void Misc_keycope(uint16 key_value)
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
				ipconfig_in_menu = IP_DISABLE;
				update_menu_state(MenuMain);
			}
			else
			{
				if(((item_index < 1) || (item_index >= 4)))// && (item_index != 13))
				{
					switch(item_index)
					{
						case 0:
							sprintf((char *)text, "%s%s", item_name[item_index], ip_mode_text[ghost_ip_mode]);
							break;
//						case 1:
//							sprintf(text, "%s%03d.%03d.%03d.%03d", item_name[item_index], (uint16)ip_address[0], (uint16)ip_address[1], (uint16)ip_address[2], (uint16)ip_address[3]);
//							break;
//						case 2:
//							sprintf(text, "%s%03d.%03d.%03d.%03d", item_name[item_index], (uint16)subnet_mask_address[0], (uint16)subnet_mask_address[1], (uint16)subnet_mask_address[2], (uint16)subnet_mask_address[3]);
//							break;
//						case 3:
//							sprintf(text, "%s%03d.%03d.%03d.%03d", item_name[item_index], (uint16)gateway_address[0], (uint16)gateway_address[1], (uint16)gateway_address[2], (uint16)gateway_address[3]);
//							break;
						case 4:
//							listen_port_at_tcp_server_mode_ghost[0] = listen_port_at_tcp_server_mode[0];
//							listen_port_at_tcp_server_mode_ghost[1] = listen_port_at_tcp_server_mode[1];
							sprintf((char *)text, "%s%u", item_name[item_index], ghost_listen_port);
							break;
						case 5:
							sprintf((char *)text, "%s%s", item_name[item_index], new_ip_status_text[ipconfig_in_menu]);
							break;
						case 6:
							sprintf((char *)text, "%s%u", item_name[item_index], (uint16)Modbus.address);
							break;
						case 7:
							sprintf((char *)text, "%s%s", item_name[item_index], baudrate_text[Modbus.baudrate[0]]);
							break;
						case 8:
							//sprintf((char *)text, "%s", item_name[item_index]);
							if(Modbus.com_config[0] == MODBUS_SLAVE) 
								sprintf((char *)text, "%s%s", item_name[item_index], protocol_text[1]);
							else 
								sprintf((char *)text, "%s%s", item_name[item_index], protocol_text[0]);
							break;
						case 9:
							sprintf((char *)text, "%s%04d-%02d-%02d", item_name[item_index], (uint16)rtc_date.year, (uint16)rtc_date.month, (uint16)rtc_date.day);
							break;
						case 10:
							sprintf((char *)text, "%s%02d:%02d", item_name[item_index], (uint16)rtc_date.hour, (uint16)rtc_date.minute);
							break;
//						case 10:
//							sprintf((char *)text, "%s%u", item_name[item_index], (uint16)pre_alarm_on_time);
//							break;
//						case 11:
//							sprintf((char *)text, "%s%u", item_name[item_index], (uint16)pre_alarm_off_time);
//							break;
						case 11:
							sprintf((char *)text, "%s%s", item_name[item_index], YesOrNo[use_password]);
							break;
						
						case 13:
							sprintf((char *)text, "%s%s", item_name[item_index], YesOrNo[alarmEnable]);
							break;
						case 14:
							sprintf((char *)text, "%s%s", item_name[item_index], YesOrNo[enableScroll]);
							break;
					}
					Lcd_Clear_Row(item_index % SET_MENU_ROWS);
					Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_INV, text);
					in_sub_menu = FALSE;
				}
				else // in the password setting menu
				{
					switch(item_index)
					{
						case 1:
							switch(sub_menu_index)
							{
								case 0:
									sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[item_index], (uint16)ghost_ip_addr[0], (uint16)ghost_ip_addr[1], (uint16)ghost_ip_addr[2], (uint16)ghost_ip_addr[3]);
									Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_INV, text);
									in_sub_menu = FALSE;
									break;
								case 1:
//									ip_address_ghost[1] = set_value;	
//									sprintf(text, "%03d", set_value);
//									Lcd_Show_String(item_index % SET_MENU_ROWS, 9, DISP_NOR, text);
									sprintf((char *)text, "%03d", (uint16)ghost_ip_addr[1]);
									Lcd_Show_String(item_index % SET_MENU_ROWS, 9, DISP_NOR, text);
		
									set_value = ghost_ip_addr[0];
									sprintf((char *)text, "%03d", set_value);
									Lcd_Show_String(item_index % SET_MENU_ROWS, 5, DISP_INV, text);
		
									sub_menu_index = 0;
									break;
								case 2:
//									ip_address_ghost[2] = set_value;	
//									sprintf(text, "%03d", set_value);
//									Lcd_Show_String(item_index % SET_MENU_ROWS, 13, DISP_NOR, text);
									sprintf((char *)text, "%03d", (uint16)ghost_ip_addr[2]);
									Lcd_Show_String(item_index % SET_MENU_ROWS, 13, DISP_NOR, text);
		
									set_value = ghost_ip_addr[1];
									sprintf((char *)text, "%03d", set_value);
									Lcd_Show_String(item_index % SET_MENU_ROWS, 9, DISP_INV, text);
		
									sub_menu_index = 1;
									break;
								case 3:
//									ip_address_ghost[3] = set_value;	
//									sprintf(text, "%03d", set_value);
//									Lcd_Show_String(item_index % SET_MENU_ROWS, 17, DISP_NOR, text);
									sprintf((char *)text, "%03d", (uint16)ghost_ip_addr[3]);
									Lcd_Show_String(item_index % SET_MENU_ROWS, 17, DISP_NOR, text);
		
									set_value = ghost_ip_addr[2];
									sprintf((char *)text, "%03d", set_value);
									Lcd_Show_String(item_index % SET_MENU_ROWS, 13, DISP_INV, text);
		
									sub_menu_index = 2;
									break;
							}
							break;
						case 2:
							switch(sub_menu_index)
							{
								case 0:
									sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[item_index], (uint16)ghost_mask_addr[0], (uint16)ghost_mask_addr[1], (uint16)ghost_mask_addr[2], (uint16)ghost_mask_addr[3]);
									Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_INV, text);
									in_sub_menu = FALSE;
									break;
								case 1:
//									subnet_mask_address_ghost[1] = set_value;	
//									sprintf(text, "%03d", set_value);
//									Lcd_Show_String(item_index % SET_MENU_ROWS, 10, DISP_NOR, text);
									sprintf((char *)text, "%03d", (uint16)ghost_mask_addr[1]);
									Lcd_Show_String(item_index % SET_MENU_ROWS, 10, DISP_NOR, text);
		
									set_value = ghost_mask_addr[0];
									sprintf((char *)text, "%03d", set_value);
									Lcd_Show_String(item_index % SET_MENU_ROWS, 6, DISP_INV, text);
		
									sub_menu_index = 0;
									break;
								case 2:
//									subnet_mask_address_ghost[2] = set_value;	
//									sprintf(text, "%03d", set_value);
//									Lcd_Show_String(item_index % SET_MENU_ROWS, 14, DISP_NOR, text);
									sprintf((char *)text, "%03d", (uint16)ghost_mask_addr[2]);
									Lcd_Show_String(item_index % SET_MENU_ROWS, 14, DISP_NOR, text);
		
									set_value = ghost_mask_addr[1];
									sprintf((char *)text, "%03d", set_value);
									Lcd_Show_String(item_index % SET_MENU_ROWS, 10, DISP_INV, text);
		
									sub_menu_index = 1;
									break;
								case 3:
//									subnet_mask_address_ghost[3] = set_value;	
//									sprintf(text, "%03d", set_value);
//									Lcd_Show_String(item_index % SET_MENU_ROWS, 18, DISP_NOR, text);
									sprintf((char *)text, "%03d", (uint16)ghost_mask_addr[3]);
									Lcd_Show_String(item_index % SET_MENU_ROWS, 18, DISP_NOR, text);
		
									set_value = ghost_mask_addr[2];
									sprintf((char *)text, "%03d", set_value);
									Lcd_Show_String(item_index % SET_MENU_ROWS, 14, DISP_INV, text);
		
									sub_menu_index = 2;
									break;
							}
							break;
						case 3:
							switch(sub_menu_index)
							{
								case 0:
									sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[item_index], (uint16)ghost_gate_addr[0], (uint16)ghost_gate_addr[1], (uint16)ghost_gate_addr[2], (uint16)ghost_gate_addr[3]);
									Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_INV, text);
									in_sub_menu = FALSE;
									break;
								case 1:
//									gateway_address_ghost[1] = set_value;	
//									sprintf(text, "%03d", set_value);
//									Lcd_Show_String(item_index % SET_MENU_ROWS, 10, DISP_NOR, text);
									sprintf((char *)text, "%03d", (uint16)ghost_gate_addr[1]);
									Lcd_Show_String(item_index % SET_MENU_ROWS, 10, DISP_NOR, text);
		
									set_value = ghost_gate_addr[0];
									sprintf((char *)text, "%03d", set_value);
									Lcd_Show_String(item_index % SET_MENU_ROWS, 6, DISP_INV, text);
		
									sub_menu_index = 0;
									break;
								case 2:
//									gateway_address_ghost[2] = set_value;	
//									sprintf(text, "%03d", set_value);
//									Lcd_Show_String(item_index % SET_MENU_ROWS, 14, DISP_NOR, text);
									sprintf((char *)text, "%03d", (uint16)ghost_gate_addr[2]);
									Lcd_Show_String(item_index % SET_MENU_ROWS, 14, DISP_NOR, text);
		
									set_value = ghost_gate_addr[1];
									sprintf((char *)text, "%03d", set_value);
									Lcd_Show_String(item_index % SET_MENU_ROWS, 10, DISP_INV, text);
		
									sub_menu_index = 1;
									break;
								case 3:
//									gateway_address_ghost[3] = set_value;	
//									sprintf(text, "%03d", set_value);
//									Lcd_Show_String(item_index % SET_MENU_ROWS, 18, DISP_NOR, text);
									sprintf((char *)text, "%03d", (uint16)ghost_gate_addr[3]);
									Lcd_Show_String(item_index % SET_MENU_ROWS, 18, DISP_NOR, text);
		
									set_value = ghost_gate_addr[2];
									sprintf((char *)text, "%03d", set_value);
									Lcd_Show_String(item_index % SET_MENU_ROWS, 14, DISP_INV, text);
		
									sub_menu_index = 2;
									break;
							}
							break;
						case 12:
							if(password_index == 0)
							{
								cursor_off();
//								sprintf((char *)text, "%s%c%c%c%c", item_name[item_index], user_password[0], user_password[1], user_password[2], user_password[3]);
								strcpy((char *)text, (char *)item_name[item_index]);
								strncat((char *)text, (char *)user_password, 4);
								Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_INV, text);
								in_sub_menu = FALSE;
							}
							else
							{
								password_buffer[password_index] = set_value;
								Lcd_Write_Char(cursor.row, cursor.line--, set_value, DISP_INV);
		
								password_index--;
								set_value = password_buffer[password_index];
								cursor.on_byte = set_value;
							}
							break;
					}
				}
			}
			break;
		case KEY_RIGHT_MASK:
			// enter the sub menu of miscellaneous settings
			switch(item_index)
			{
				case 0:
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s", item_name[item_index]);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_NOR, text);

						set_value = ghost_ip_mode;
						in_sub_menu = TRUE;
					}
					else
					{
						if(ghost_ip_mode != set_value)
						{
							ghost_ip_mode = set_value;
							write_ghost_system = 1; 
						}

						sprintf((char *)text, "%s%s", item_name[item_index], ip_mode_text[ghost_ip_mode]);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_INV, text);

						in_sub_menu = FALSE;
					}
					break;
				case 1: // ip
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[item_index], (uint16)ghost_ip_addr[0], (uint16)ghost_ip_addr[1], (uint16)ghost_ip_addr[2], (uint16)ghost_ip_addr[3]);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_NOR, text);

						set_value = ghost_ip_addr[0];
						sprintf((char *)text, "%03d", set_value);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 5, DISP_INV, text);

						in_sub_menu = TRUE;
						sub_menu_index = 0;
					}
					else
					{
						if(sub_menu_index == 0) // first byte done
						{
							ghost_ip_addr[0] = set_value;	
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % SET_MENU_ROWS, 5, DISP_NOR, text);

							set_value = ghost_ip_addr[1];
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % SET_MENU_ROWS, 9, DISP_INV, text);

							sub_menu_index = 1;
						}
						else if(sub_menu_index == 1) // second byte done
						{
							ghost_ip_addr[1] = set_value;	
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % SET_MENU_ROWS, 9, DISP_NOR, text);

							set_value = ghost_ip_addr[2];
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % SET_MENU_ROWS, 13, DISP_INV, text);

							sub_menu_index = 2;
						}
						else if(sub_menu_index == 2) // third byte done
						{
							ghost_ip_addr[2] = set_value;	
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % SET_MENU_ROWS, 13, DISP_NOR, text);

							set_value = ghost_ip_addr[3];
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % SET_MENU_ROWS, 17, DISP_INV, text);

							sub_menu_index = 3;
						}
						else if(sub_menu_index == 3)
						{
							ghost_ip_addr[3] = set_value;
								
							sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[item_index], (uint16)ghost_ip_addr[0], (uint16)ghost_ip_addr[1], (uint16)ghost_ip_addr[2], (uint16)ghost_ip_addr[3]);
							Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_INV, text);

							if((Modbus.ip_addr[0] != ghost_ip_addr[0]) || (Modbus.ip_addr[1] != ghost_ip_addr[1]) || (Modbus.ip_addr[2] != ghost_ip_addr[2]) || (Modbus.ip_addr[3] != ghost_ip_addr[3]))
								ip_changed_in_menu = 1;
							else
								ip_changed_in_menu = 0;
							
							in_sub_menu = FALSE;
							sub_menu_index = 0;
						}
					}
					break;
			   case 2: // subnet mask address
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[item_index], (uint16)ghost_mask_addr[0], (uint16)ghost_mask_addr[1], (uint16)ghost_mask_addr[2], (uint16)ghost_mask_addr[3]);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_NOR, text);

						set_value = ghost_mask_addr[0];
						sprintf((char *)text, "%03d", set_value);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 6, DISP_INV, text);

						in_sub_menu = TRUE;
						sub_menu_index = 0;
					}
					else
					{
						if(sub_menu_index == 0) // first byte done
						{
							ghost_mask_addr[0] = set_value;	
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % SET_MENU_ROWS, 6, DISP_NOR, text);

							set_value = ghost_mask_addr[1];
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % SET_MENU_ROWS, 10, DISP_INV, text);

							sub_menu_index = 1;
						}
						else if(sub_menu_index == 1) // second byte done
						{
							ghost_mask_addr[1] = set_value;	
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % SET_MENU_ROWS, 10, DISP_NOR, text);

							set_value = ghost_mask_addr[2];
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % SET_MENU_ROWS, 14, DISP_INV, text);

							sub_menu_index = 2;
						}
						else if(sub_menu_index == 2) // third byte done
						{
							ghost_mask_addr[2] = set_value;	
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % SET_MENU_ROWS, 14, DISP_NOR, text);

							set_value = ghost_mask_addr[3];
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % SET_MENU_ROWS, 18, DISP_INV, text);

							sub_menu_index = 3;
						}
						else if(sub_menu_index == 3)
						{
							ghost_mask_addr[3] = set_value;	
							
							sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[item_index], (uint16)ghost_mask_addr[0], (uint16)ghost_mask_addr[1], (uint16)ghost_mask_addr[2], (uint16)ghost_mask_addr[3]);
							Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_INV, text);

							if((Modbus.subnet[0] != ghost_mask_addr[0]) || (Modbus.subnet[1] != ghost_mask_addr[1]) || (Modbus.subnet[2] != ghost_mask_addr[2]) || (Modbus.subnet[3] != ghost_mask_addr[3]))
								subnet_changed_in_menu = 1;
							else
								subnet_changed_in_menu = 0;

							in_sub_menu = FALSE;
							sub_menu_index = 0;
						}
					}
					break;
				case 3: // gateway address
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[item_index], (uint16)ghost_gate_addr[0], (uint16)ghost_gate_addr[1], (uint16)ghost_gate_addr[2], (uint16)ghost_gate_addr[3]);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_NOR, text);

						set_value = ghost_gate_addr[0];
						sprintf((char *)text, "%03d", set_value);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 6, DISP_INV, text);

						in_sub_menu = TRUE;
						sub_menu_index = 0;
					}
					else
					{
						if(sub_menu_index == 0) // first byte done
						{
							ghost_gate_addr[0] = set_value;	
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % SET_MENU_ROWS, 6, DISP_NOR, text);

							set_value = ghost_gate_addr[1];
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % SET_MENU_ROWS, 10, DISP_INV, text);

							sub_menu_index = 1;
						}
						else if(sub_menu_index == 1) // second byte done
						{
							ghost_gate_addr[1] = set_value;	
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % SET_MENU_ROWS, 10, DISP_NOR, text);

							set_value = ghost_gate_addr[2];
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % SET_MENU_ROWS, 14, DISP_INV, text);

							sub_menu_index = 2;
						}
						else if(sub_menu_index == 2) // third byte done
						{
							ghost_gate_addr[2] = set_value;	
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % SET_MENU_ROWS, 14, DISP_NOR, text);

							set_value = ghost_gate_addr[3];
							sprintf((char *)text, "%03d", set_value);
							Lcd_Show_String(item_index % SET_MENU_ROWS, 18, DISP_INV, text);

							sub_menu_index = 3;
						}
						else if(sub_menu_index == 3)
						{
							ghost_gate_addr[3] = set_value;	
							
							sprintf((char *)text, "%s%03d.%03d.%03d.%03d", item_name[item_index], (uint16)ghost_gate_addr[0], (uint16)ghost_gate_addr[1], (uint16)ghost_gate_addr[2], (uint16)ghost_gate_addr[3]);
							Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_INV, text);

							if((Modbus.getway[0] != ghost_gate_addr[0]) || (Modbus.getway[1] != ghost_gate_addr[1]) || (Modbus.getway[2] != ghost_gate_addr[2]) || (Modbus.getway[3] != ghost_gate_addr[3]))
								gateway_changed_in_menu = 1;
							else
								gateway_changed_in_menu = 0;

							in_sub_menu = FALSE;
							sub_menu_index = 0;
						}
					}
					break;
				case 4:	// tcp port
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s", item_name[item_index]);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_NOR, text);
						set_value = ghost_listen_port; 
						in_sub_menu = TRUE;
					}
					else
					{
						sprintf((char *)text, "%s%u", item_name[item_index], set_value);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_INV, text);
 
						ghost_listen_port =  set_value;	
						if(ghost_listen_port != Modbus.listen_port)
							port_changed_in_menu = 1;
						else
							port_changed_in_menu = 0;

						in_sub_menu = FALSE;
					}
					break;
				case 5:
					if(in_sub_menu == FALSE)
					{
						if(ipconfig_in_menu != IP_NO_CHANGE)
						{
							sprintf((char *)text, "%s", item_name[item_index]);
							Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_NOR, text);
	
							set_value = ipconfig_in_menu;
							in_sub_menu = TRUE;
						}
					}
					else
					{
						sprintf((char *)text, "%s%s", item_name[item_index], new_ip_status_text[set_value]);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_INV, text);

						ipconfig_in_menu = set_value;
						ip_changed_in_menu =0;
						subnet_changed_in_menu =0;
						gateway_changed_in_menu =0;
						port_changed_in_menu = 0;
						if(ipconfig_in_menu == IP_DISABLE)
						{
							

							Misc_init();
							
						}
						else if(ipconfig_in_menu == IP_ENABLE)
						{
							write_ghost_system = 1;
							in_sub_menu = FALSE;
							ipconfig_in_menu = IP_NO_CHANGE;
						}
					}
					break;
				case 6:	// modbus id
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s", item_name[item_index]);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_NOR, text);
						set_value = Modbus.address;
						in_sub_menu = TRUE;
					}
					else
					{
						if(Modbus.address != set_value)	
						{
							if((set_value != 0) && (set_value != 255))
							{
								Modbus.address = set_value;
								save_uint8_to_flash(FLASH_MODBUS_ID, Modbus.address);

								Station_NUM = Modbus.address;
								dlmstp_init(NULL);
							}
						}
						sprintf((char *)text, "%s%u", item_name[item_index], set_value);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_INV, text);
						in_sub_menu = FALSE;
					}
					break;
				case 7:	// baudrate
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s", item_name[item_index]);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_NOR, text);

						set_value = Modbus.baudrate[0];
						sprintf((char *)text, "%s", baudrate_text[set_value]);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 12, DISP_INV, text);
						in_sub_menu = TRUE;
					}
					else
					{
						Modbus.baudrate[0] = set_value;
						sprintf((char *)text, "%s%s", item_name[item_index], baudrate_text[Modbus.baudrate[0]]);
						strcat((char *)text, (char *)PARITY_TEXT[0/*uart1_parity*/]);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_INV, text);
						in_sub_menu = FALSE;

						save_uint8_to_flash(FLASH_BAUD_RATE, Modbus.baudrate[0]);
						modbus_init();
					}
					break;
				case 8:
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s", item_name[item_index]);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_NOR, text);
						if(Modbus.com_config[0] == BAC_MSTP)
							set_value = 0;
						else
							set_value = 1;
						sprintf((char *)text, "%s", protocol_text[set_value]);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 13, DISP_INV, text);
						in_sub_menu = TRUE;
					}
					else
					{
						if(set_value)
						{							
							Modbus.com_config[0] = MODBUS_SLAVE;
							update_flag = 1;
						}
						else
						{
							Modbus.com_config[0] = BAC_MSTP;
							update_flag = 2;
						}
//						start_data_save_timer();
//						flash_write_int(FLASH_USE_PASSWORD, use_password);
						save_uint8_to_flash(FLASH_UART_CONFIG, Modbus.com_config[0]);
						sprintf((char *)text, "%s%s", item_name[item_index], protocol_text[set_value]);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_INV, text);
						in_sub_menu = FALSE;
					}
					break; 
				case 9: // date
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s%04d-%02d-%02d", item_name[item_index], (uint16)rtc_date.year, (uint16)rtc_date.month, (uint16)rtc_date.day);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_NOR, text);

						set_value = rtc_date.year;
						sprintf((char *)text, "%04d", set_value);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 8, DISP_INV, text);
						in_sub_menu = TRUE;
						sub_menu_index = 0;
					}
					else
					{
						if(sub_menu_index == 0) // year done
						{
							rtc_date.year = set_value;
							Time_Adjust();							
							//rtc_set_time(RTC_YEAR, TOBCD(Modbus.Time.Clk.year));

							sprintf((char *)text, "%s%02d-%02d-%02d", item_name[item_index], (uint16)rtc_date.year, (uint16)rtc_date.month, (uint16)rtc_date.day);
							Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_NOR, text);

							set_value = rtc_date.month;
							sprintf((char *)text, "%02d", set_value);
							Lcd_Show_String(item_index % SET_MENU_ROWS, 13, DISP_INV, text);
							sub_menu_index = 1;
						}
						else if(sub_menu_index == 1) // month done
						{
							rtc_date.month = set_value;	
							Time_Adjust();
							//rtc_set_time(RTC_MONTH, TOBCD(Modbus.Time.Clk.mon));

							sprintf((char *)text, "%s%02d-%02d-%02d", item_name[item_index], (uint16)rtc_date.year, (uint16)rtc_date.month, (uint16)rtc_date.day);
							Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_NOR, text);

							set_value = rtc_date.day;
							sprintf((char *)text, "%02d", set_value);
							Lcd_Show_String(item_index % SET_MENU_ROWS, 16, DISP_INV, text);
							sub_menu_index = 2;
						}
						else if(sub_menu_index == 2) // day done
						{
							rtc_date.day = set_value;	
							Time_Adjust();
							//rtc_set_time(RTC_DATE, TOBCD(Modbus.Time.Clk.day));

							sprintf((char *)text, "%s%02d-%02d-%02d", item_name[item_index], (uint16)rtc_date.year, (uint16)rtc_date.month, (uint16)rtc_date.day);
							Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_INV, text);

							in_sub_menu = FALSE;
							sub_menu_index = 0;
						}
					}
					break;
				case 10: // time
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s%02d:%02d", item_name[item_index], (uint16)rtc_date.hour, (uint16)rtc_date.minute);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_NOR, text);

						set_value = rtc_date.hour;
						sprintf((char *)text, "%02d", set_value);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 9, DISP_INV, text);
						in_sub_menu = TRUE;
						sub_menu_index = 0;
					}
					else
					{
						if(sub_menu_index == 0) // hour done
						{
							rtc_date.hour = set_value;	
							Time_Adjust();
							//rtc_set_time(RTC_HOUR, TOBCD(Modbus.Time.Clk.hour));

							sprintf((char *)text, "%s%02d:%02d", item_name[item_index], (uint16)rtc_date.hour, (uint16)rtc_date.minute);
							Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_NOR, text);

							set_value = rtc_date.minute;
							sprintf((char *)text, "%02d", set_value);
							Lcd_Show_String(item_index % SET_MENU_ROWS, 12, DISP_INV, text);
							sub_menu_index = 1;
						}
						else if(sub_menu_index == 1) // minute done
						{
							rtc_date.minute = set_value;
							Time_Adjust();							
							//rtc_set_time(RTC_MINUTE, TOBCD(Modbus.Time.Clk.min));

							sprintf((char *)text, "%s%02d:%02d", item_name[item_index], (uint16)rtc_date.hour, (uint16)rtc_date.minute);
							Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_INV, text);

							in_sub_menu = FALSE;
							sub_menu_index = 0;
						}
					}
					break;
//				case 10: // ring on time
//					if(in_sub_menu == FALSE)
//					{
//						sprintf((char *)text, "%s", item_name[item_index]);
//						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_NOR, text);

//						set_value = pre_alarm_on_time;
//						sprintf((char *)text, "%u", set_value);
//						Lcd_Show_String(item_index % SET_MENU_ROWS, 17, DISP_INV, text);
//						in_sub_menu = TRUE;
//					}
//					else
//					{
//						pre_alarm_on_time = set_value;	
////						start_data_save_timer();
////						flash_write_int(FLASH_PRE_ALARM_SETTING_ON_TIME, pre_alarm_on_time);
//						write_eeprom(EEP_PRE_ALARM_SETTING_ON_TIME,pre_alarm_on_time);  
//						sprintf((char *)text, "%s%u", item_name[item_index], set_value);
//						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_INV, text);
//						in_sub_menu = FALSE;
//					}
//					break;
//				case 11: // ring off time
//					if(in_sub_menu == FALSE)
//					{
//						sprintf((char *)text, "%s", item_name[item_index]);
//						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_NOR, text);

//						set_value = pre_alarm_off_time;
//						sprintf((char *)text, "%u", set_value);
//						Lcd_Show_String(item_index % SET_MENU_ROWS, 18, DISP_INV, text);
//						in_sub_menu = TRUE;
//					}
//					else
//					{
//						pre_alarm_off_time = set_value;	
////						start_data_save_timer();
////						flash_write_int(FLASH_PRE_ALARM_SETTING_OFF_TIME, pre_alarm_off_time);
//						write_eeprom(EEP_PRE_ALARM_SETTING_OFF_TIME,pre_alarm_off_time);  
//						sprintf((char *)text, "%s%u", item_name[item_index], set_value);
//						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_INV, text);
//						in_sub_menu = FALSE;
//					}
//					break;
				case 11:
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s", item_name[item_index]);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_NOR, text);

						set_value = use_password;
						sprintf((char *)text, "%s", YesOrNo[set_value]);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 17, DISP_INV, text);
						in_sub_menu = TRUE;
					}
					else
					{
						use_password = set_value;	
//						start_data_save_timer();
//						flash_write_int(FLASH_USE_PASSWORD, use_password);
						save_uint8_to_flash(FLASH_USE_PASSWORD, use_password);
						sprintf((char *)text, "%s%s", item_name[item_index], YesOrNo[use_password]);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_INV, text);
						in_sub_menu = FALSE;
					}
					break;
				case 12:
					if(in_sub_menu == FALSE)
					{
 						sprintf((char *)text, "%s", item_name[item_index]);
//						strcpy((char *)text, (char *)item_name[item_index]);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_NOR, text);
						
//						strncat((char *)text, (char *)user_password, 4);
//						text[4] = '\0';
 						sprintf((char *)text, "%c%c%c%c", user_password[0], user_password[1], user_password[2], user_password[3]);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 13, DISP_INV, text);

						password_buffer[0] = user_password[0];
						password_buffer[1] = user_password[1];
						password_buffer[2] = user_password[2];
						password_buffer[3] = user_password[3];
						password_index = 0;
						set_value = password_buffer[password_index];
						cursor_on(item_index % SET_MENU_ROWS, 13, (uint8)set_value, ' ');

						in_sub_menu = TRUE;
					}
					else
					{
						if(password_index < MAX_PASSWORD_DIGITALS)
						{
							password_buffer[password_index++] = set_value;
							Lcd_Write_Char(cursor.row, cursor.line++, set_value, DISP_INV);
						}

						if(password_index == MAX_PASSWORD_DIGITALS)
						{
							cursor_off();
							user_password[0] = password_buffer[0];
							user_password[1] = password_buffer[1];
							user_password[2] = password_buffer[2];
							user_password[3] = password_buffer[3];
//							start_data_save_timer();
//							flash_write_int(FLASH_USER_PASSWORD0, user_password[0]);
//							flash_write_int(FLASH_USER_PASSWORD1, user_password[1]);
//							flash_write_int(FLASH_USER_PASSWORD2, user_password[2]);
//							flash_write_int(FLASH_USER_PASSWORD3, user_password[3]);
							save_uint8_to_flash(FLASH_USER_PASSWORD0, user_password[0]);
							save_uint8_to_flash(FLASH_USER_PASSWORD1, user_password[1]);
							save_uint8_to_flash(FLASH_USER_PASSWORD2, user_password[2]);
							save_uint8_to_flash(FLASH_USER_PASSWORD3, user_password[3]);	
 							//sprintf((char *)text, "%s%c%c%c%c", item_name[item_index], user_password[0], user_password[1], user_password[2], user_password[3]);
 							strcpy((char *)text, (char *)item_name[item_index]);
							strncat((char *)text, (char *)user_password, 4);
							Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_INV, text);
//							Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_INV, (uint8 *)"test");//text);

							in_sub_menu = FALSE;
						}
						else
						{
							set_value = password_buffer[password_index];
							cursor.on_byte = set_value;
						}
					}
					break;
				
				case 13:
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s", item_name[item_index]);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_NOR, text);

						set_value = alarmEnable;
						sprintf((char *)text, "%s", YesOrNo[set_value]);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 17, DISP_INV, text);
						in_sub_menu = TRUE;
					}
					else
					{
						alarmEnable = set_value;	
//						start_data_save_timer();
//						flash_write_int(FLASH_USE_PASSWORD, use_password);
						save_uint8_to_flash(FLASH_ENABLE_ALARM, alarmEnable);
						sprintf((char *)text, "%s%s", item_name[item_index], YesOrNo[alarmEnable]);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_INV, text);
						in_sub_menu = FALSE;
					}
					break;
				case 14:
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s", item_name[item_index]);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_NOR, text);

						set_value = enableScroll;
						sprintf((char *)text, "%s", YesOrNo[set_value]);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 17, DISP_INV, text);
						in_sub_menu = TRUE;
					}
					else
					{
						enableScroll = set_value;	
//						start_data_save_timer();
//						flash_write_int(FLASH_USE_PASSWORD, use_password);
						save_uint8_to_flash(FLASH_ENABLE_SCROLL, enableScroll);
						sprintf((char *)text, "%s%s", item_name[item_index], YesOrNo[enableScroll]);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_INV, text);
						in_sub_menu = FALSE;
					}
					break;
			}
			break;
		case KEY_UP_MASK:
			if(in_sub_menu == FALSE)
			{
				if((item_index %  SET_MENU_ROWS) == 0)	
				{
					if(item_index)
						item_index--;
					else
						item_index =  MAX_MISC_ITEMS - 1;

					Misc_init();
					pre_item_index = item_index;
				}
				else
				{
					if(item_index)
						item_index--;
					else
						item_index = MAX_MISC_ITEMS - 1;	
				}
//				if(item_index)
//					item_index--;
//				else
//					item_index = MAX_MISC_ITEMS - 1;
//				item_index %= MAX_MISC_ITEMS;

//				if((item_index % SET_MENU_ROWS) == 0)
//				{
//					Misc_init();
//					pre_item_index = item_index;
//				}
			}
			else
			{
				switch(item_index)
				{
					case 0:
						if(set_value == 1)
							set_value = 0;
						else
							set_value = 1;
						break;
					case 1:	// ip address
					case 2:	// subnet mask address
					case 3:	// gateway address
						if((key_value & KEY_FUNCTION_MASK) == KEY_SPEED_1)
							set_value += SPEED_1;
						else
							set_value = (((set_value + SPEED_10) < 255) ? (set_value + SPEED_10) : 1);

						set_value %= 255;
						break;
					case 4:	// tcp port
						switch(key_value & KEY_FUNCTION_MASK)
						{
							case KEY_SPEED_100:
								set_value += SPEED_100;
								break;
							case KEY_SPEED_50:
								set_value += SPEED_50;
								break;
							case KEY_SPEED_10:
								set_value += SPEED_10;
								break;
							case KEY_SPEED_1:
							default:
								set_value += SPEED_1;
								break;
						}
						break;
					case 5:
						if(set_value != IP_ENABLE)
							set_value = IP_ENABLE;
						else
							set_value = IP_DISABLE;
						break;
					case 6: // modbus id setting, check in the database
						if((key_value & KEY_FUNCTION_MASK) == KEY_SPEED_1)
						{
							if(set_value < 254)
								set_value += SPEED_1;
							else 
								set_value = 1;
						}							
						else
							set_value = (((set_value + SPEED_10) < 255) ? (set_value + SPEED_10) : 1);

						set_value %= 255;
#if OLD_CO2
						set_value = check_master_id_in_database(set_value, 1);
#endif
						break;
					case 7:	// bardrate
						if(set_value >= 5)
							set_value = 0;
						else
							set_value++;
						break;
						
					case 8:
						if(set_value == 1)
							set_value = 0;
						else
							set_value = 1;
						break;
					case 9:
						set_value++;
						if(sub_menu_index == 0)
						{
							if(set_value > 2099) set_value = 2099;
							 //set_value %= 100;
						}
						else if(sub_menu_index == 1)
						{
							if(set_value == 13)
								set_value = 1;
						}
						else if(sub_menu_index == 2)
						{
							switch(rtc_date.month)
							{
								case 1:
								case 3:
								case 5:
								case 7:
								case 8:
								case 10:
								case 12:
									if(set_value > 31)
										set_value = 1;
									break;
								case 4:
								case 6:
								case 9:
								case 11:
									if(set_value > 30)
										set_value = 1;
									break;
								case 2:
									if(rtc_date.year % 4)
									{
										if(set_value > 29)
											set_value = 1;
									}
									else
									{
										if(set_value > 28)
											set_value = 1;
									}
									break;
							}
						}
						break;
					case 10:
						set_value++;
						if(sub_menu_index == 0)
							set_value %= 24;
						else if(sub_menu_index == 1)
							set_value %= 60;
						break;
//					case 10:
//					case 11:
//						if((key_value & KEY_FUNCTION_MASK) == KEY_SPEED_1)
//							set_value += SPEED_1;
//						else
//							set_value = (((set_value + SPEED_10) < 255) ? (set_value + SPEED_10) : 0);
//						set_value %= 256;
//						
//						if(set_value == 0)
//							set_value = 1;

//						break;
					case 11:
						if(set_value == 1)
							set_value = 0;
						else
							set_value = 1;
						break;
					case 12:
						if((set_value >= '9') || (set_value < '0'))
							set_value = '0';
						else
							set_value++;
						break;
					case 13:
					case 14:
						if(set_value == 1)
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
				item_index %= MAX_MISC_ITEMS;

				if((item_index % SET_MENU_ROWS) == 0)
				{
					Misc_init();
					pre_item_index = item_index;
				}
			}
			else
			{
				switch(item_index)
				{
					case 0:
						if(set_value == 1)
							set_value = 0;
						else
							set_value = 1;
						break;
					case 1:	// ip address
					case 2:	// subnet mask address
					case 3:	// gateway address
						if((key_value & KEY_FUNCTION_MASK) == KEY_SPEED_1)
						{
							if(set_value)
								set_value -= SPEED_1;
							else
								set_value = 255;
						}
						else
							set_value = ((set_value > SPEED_10) ? (set_value - SPEED_10) : 255);
						break;
					case 4:	// tcp port
						switch(key_value & KEY_FUNCTION_MASK)
						{
							case KEY_SPEED_100:
								set_value -= SPEED_100;
								break;
							case KEY_SPEED_50:
								set_value -= SPEED_50;
								break;
							case KEY_SPEED_10:
								set_value -= SPEED_10;
								break;
							case KEY_SPEED_1:
							default:
								set_value -= SPEED_1;
								break;
						}
						break;
					case 5:
						if(set_value != IP_DISABLE)
							set_value = IP_DISABLE;
						else
							set_value = IP_ENABLE;
						break;
					case 6: // modbus id setting, check in the database
						if((key_value & KEY_FUNCTION_MASK) == KEY_SPEED_1)
						{
							if(set_value > 1)
								set_value -= SPEED_1;
							else
								set_value = 254;
						}
						else
							set_value = ((set_value > SPEED_10) ? (set_value - SPEED_10) : 254);
#if OLD_CO2
						set_value = check_master_id_in_database(set_value, 0);
#endif
						break;
					case 7:	// bardrate
						if(set_value == 0)
							set_value = 5;
						else
							set_value --;
						break;
						
					case 8:
						if(set_value == 1)
							set_value = 0;
						else
							set_value = 1;
						break; 
					case 9:
						if(sub_menu_index == 0)
						{
							if(set_value > 2000)
								set_value--;
							else
								set_value = 2099;
						}
						else if(sub_menu_index == 1)
						{
							if(set_value > 1)
								set_value--;
							else
								set_value = 12;
						}
						else if(sub_menu_index == 2)
						{
							switch(rtc_date.month)
							{
								case 1:
								case 3:
								case 5:
								case 7:
								case 8:
								case 10:
								case 12:
									if(set_value > 1)
										set_value--;
									else
										set_value = 31;
									break;
								case 4:
								case 6:
								case 9:
								case 11:
									if(set_value > 1)
										set_value--;
									else
										set_value = 30;
									break;
								case 2:
									if(rtc_date.year % 4)
									{
										if(set_value > 1)
											set_value--;
										else
											set_value = 29;
									}
									else
									{
										if(set_value > 1)
											set_value--;
										else
											set_value = 28;
									}
									break;
							}
						}
						break;
					case 10:
						if(sub_menu_index == 0)
						{
							if(set_value > 0)
								set_value--;
							else
								set_value = 23;
						}
						else if(sub_menu_index == 1)
						{
							if(set_value > 0)
								set_value--;
							else
								set_value = 59;
						}
						break;
//					case 10:
//					case 11:
//						if((key_value & KEY_FUNCTION_MASK) == KEY_SPEED_1)
//						{
//							if(set_value)
//								set_value -= SPEED_1;
//							else
//								set_value = 255;
//						}
//						else
//							set_value = ((set_value > SPEED_10) ? (set_value - SPEED_10) : 255);
//						
//						if(set_value == 0)
//							set_value = 255;

//						break;
					case 11:
						if(set_value == 1)
							set_value = 0;
						else
							set_value = 1;
						break;
					case 12:
						if((set_value <= '0') || (set_value > '9'))
							set_value = '9';
						else
							set_value--;
						break;
					case 13:
					case 14:
						if(set_value == 1)
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




