#include "define.h"
#include "Menu.h"
#include <string.h>
#include "rtc.h"
#include "bacnet.h"
#include "wifi.h"
#include "esp_netif.h"

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
uint8 mode_changed_in_menu = 0;
extern uint8_t flag_change_uart0;
extern uint8_t count_change_uart0;
extern void uart_init(uint8_t uart);
extern void Count_com_config(void);
extern void Recievebuf_Initialize(uint8_t port);
extern bool dlmstp_init(char *ifname);
extern U8_T Send_I_Am_Flag;
extern U8_T Send_Whois_Flag; 

uint8 count_write_ghost_system = 0;
uint8 count_write_ghost_system_wifi = 0;

uint8 flag_write_basic_info = 0;
uint8 write_ghost_system = 0;
uint8 count_write_basic_info = 0;

#define MAX_MISC_ITEMS	21
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
	"12.Use password: ",
	"13.Password: ",
	"14.Enable Alarm: ",
	"15.Status Line:  ",
	"16.WIp mode: ",
	"17.Wp:",
	"18.Mk:",
	"19.Gt:",
	"20.WPort:",
	"21.WNew ip:",
};

static uint8 ghost_wifi_mode;
static uint8 ghost_wifi_ip[4];
static uint8 ghost_wifi_mask[4];
static uint8 ghost_wifi_gate[4];
static uint16 ghost_wifi_port;
static uint8 wifi_ip_changed;
static uint8 wifi_mask_changed;
static uint8 wifi_gate_changed;
static uint8 wifi_port_changed;
static uint8 wifi_mode_changed;
static uint8 wificonfig_in_menu;

#define WIFI_MODE_COL	13
#define WIFI_IP_COL		6
#define WIFI_MSK_COL	6
#define WIFI_GAT_COL	6
#define WIFI_PORT_COL	9
#define WIFI_NEW_COL	11

static void append_dotted(const uint8 *addr)
{
	sprintf((char *)int_text, "%u.%u.%u.%u",
		(uint16)addr[0], (uint16)addr[1], (uint16)addr[2], (uint16)addr[3]);
	strcat((char *)text, (char *)int_text);
}

static void append_plain(const uint8 *addr)
{
	sprintf((char *)int_text, "%u.%u.%u.%u",
		(uint16)addr[0], (uint16)addr[1], (uint16)addr[2], (uint16)addr[3]);
	strcat((char *)text, (char *)int_text);
}

static void live_wifi_addr(uint8 which, uint8 *out)
{
	esp_netif_ip_info_t info;
	esp_netif_t *netif;
	const uint8 *fallback;
	const esp_ip4_addr_t *src;

	if(which == 0)
		fallback = SSID_Info.ip_addr;
	else if(which == 1)
		fallback = SSID_Info.net_mask;
	else
		fallback = SSID_Info.getway;

	netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
	if(netif != NULL && esp_netif_get_ip_info(netif, &info) == ESP_OK)
	{
		if(which == 0)
			src = &info.ip;
		else if(which == 1)
			src = &info.netmask;
		else
			src = &info.gw;
		out[0] = esp_ip4_addr1(src);
		out[1] = esp_ip4_addr2(src);
		out[2] = esp_ip4_addr3(src);
		out[3] = esp_ip4_addr4(src);
		return;
	}
	out[0] = fallback[0];
	out[1] = fallback[1];
	out[2] = fallback[2];
	out[3] = fallback[3];
}

static void live_eth_addr(uint8 which, uint8 *out)
{
	esp_netif_ip_info_t info;
	esp_netif_t *netif;
	const uint8 *fallback;
	const esp_ip4_addr_t *src;

	if(which == 0)
		fallback = Modbus.ip_addr;
	else if(which == 1)
		fallback = Modbus.subnet;
	else
		fallback = Modbus.getway;

	netif = esp_netif_get_handle_from_ifkey("ETH_DEF");
	if(netif != NULL && esp_netif_get_ip_info(netif, &info) == ESP_OK)
	{
		if(which == 0)
			src = &info.ip;
		else if(which == 1)
			src = &info.netmask;
		else
			src = &info.gw;
		out[0] = esp_ip4_addr1(src);
		out[1] = esp_ip4_addr2(src);
		out[2] = esp_ip4_addr3(src);
		out[3] = esp_ip4_addr4(src);
		return;
	}
	out[0] = fallback[0];
	out[1] = fallback[1];
	out[2] = fallback[2];
	out[3] = fallback[3];
}

static void copy4(uint8 *dst, const uint8 *src)
{
	dst[0] = src[0];
	dst[1] = src[1];
	dst[2] = src[2];
	dst[3] = src[3];
}

static uint8 addr_differ(const uint8 *a, const uint8 *b)
{
	return (a[0] != b[0]) || (a[1] != b[1]) || (a[2] != b[2]) || (a[3] != b[3]);
}

static uint8 plain_col(uint8 col0, const uint8 *addr, uint8 index)
{
	uint8 col;
	uint8 i;

	col = col0;
	for(i = 0; i < index; i++)
	{
		if(addr[i] >= 100)
			col += 3;
		else if(addr[i] >= 10)
			col += 2;
		else
			col += 1;
		col++;
	}
	return col;
}

static void show_wifi_edit(const uint8 *edit, uint8 index, uint16 value, uint8 col0, uint8 whole_inv)
{
	uint8 addr[4];
	uint8 col;

	copy4(addr, edit);
	if(index < 4)
		addr[index] = (uint8)value;
	strcpy((char *)text, (char *)item_name[item_index]);
	append_plain(addr);
	Lcd_Clear_Row(item_index % SET_MENU_ROWS);
	Lcd_Show_String(item_index % SET_MENU_ROWS, 0, whole_inv ? DISP_INV : DISP_NOR, text);
	if(!whole_inv && index < 4)
	{
		sprintf((char *)text, "%u", value);
		col = plain_col(col0, addr, index);
		Lcd_Show_String(item_index % SET_MENU_ROWS, col, DISP_INV, text);
	}
}

static uint8 const ip_mode_text[2][7];
static uint8 const wifi_mode_text[2][7];
static uint8 const new_ip_status_text[4][10];

static void shown_addr(uint8 which, const uint8 *ghost, uint8 changed, uint8 edit_index, uint8 wifi, uint8 *out)
{
	if(changed || (in_sub_menu && item_index == edit_index))
		copy4(out, ghost);
	else if(wifi)
		live_wifi_addr(which, out);
	else
		live_eth_addr(which, out);
}

static void append_shown_addr(uint8 which, const uint8 *ghost, uint8 changed, uint8 edit_index)
{
	uint8 addr[4];

	shown_addr(which, ghost, changed, edit_index, 1, addr);
	append_plain(addr);
}

static void append_eth_shown(uint8 which)
{
	uint8 addr[4];
	const uint8 *ghost;
	uint8 changed;

	if(which == 0)
	{
		ghost = ghost_ip_addr;
		changed = ip_changed_in_menu;
	}
	else if(which == 1)
	{
		ghost = ghost_mask_addr;
		changed = subnet_changed_in_menu;
	}
	else
	{
		ghost = ghost_gate_addr;
		changed = gateway_changed_in_menu;
	}
	shown_addr(which, ghost, changed, (uint8)(which + 1), 0, addr);
	append_dotted(addr);
}

static void append_wifi_field(uint8 index)
{
	uint8 mode;

	switch(index)
	{
		case 15:
			mode = ghost_wifi_mode > 1 ? 0 : ghost_wifi_mode;
			strcat((char *)text, (char *)wifi_mode_text[mode]);
			break;
		case 16:
			append_shown_addr(0, ghost_wifi_ip, wifi_ip_changed, 16);
			break;
		case 17:
			append_shown_addr(1, ghost_wifi_mask, wifi_mask_changed, 17);
			break;
		case 18:
			append_shown_addr(2, ghost_wifi_gate, wifi_gate_changed, 18);
			break;
		case 19:
			itoa(ghost_wifi_port, int_text, 0);
			strcat((char *)text, (char *)int_text);
			break;
		case 20:
			strcat((char *)text, (char *)new_ip_status_text[wificonfig_in_menu]);
			break;
		default:
			break;
	}
}

static void draw_wifi_addr_rows(uint8 mode)
{
	uint8 saved;
	uint8 index;

	saved = ghost_wifi_mode;
	ghost_wifi_mode = mode;
	for(index = 16; index <= 18; index++)
	{
		strcpy((char *)text, (char *)item_name[index]);
		append_wifi_field(index);
		Lcd_Clear_Row(index % SET_MENU_ROWS);
		Lcd_Show_String(index % SET_MENU_ROWS, 0, DISP_NOR, text);
	}
	ghost_wifi_mode = saved;
}

static void wifi_load_ghost(void)
{
	uint8 n;

	ghost_wifi_mode = SSID_Info.IP_Auto_Manual;
	if(ghost_wifi_mode > 1)
		ghost_wifi_mode = 0;
	for(n = 0; n < 4; n++)
	{
		ghost_wifi_ip[n] = SSID_Info.ip_addr[n];
		ghost_wifi_mask[n] = SSID_Info.net_mask[n];
		ghost_wifi_gate[n] = SSID_Info.getway[n];
	}
	ghost_wifi_port = SSID_Info.modbus_port;
}

static void wifi_apply(void)
{
	uint8 n;

	SSID_Info.IP_Auto_Manual = ghost_wifi_mode;
	for(n = 0; n < 4; n++)
	{
		SSID_Info.ip_addr[n] = ghost_wifi_ip[n];
		SSID_Info.net_mask[n] = ghost_wifi_mask[n];
		SSID_Info.getway[n] = ghost_wifi_gate[n];
	}
	SSID_Info.modbus_port = ghost_wifi_port;
	save_block(FLASH_BLOCK1_SSID);
	wifi_ip_changed = 0;
	wifi_mask_changed = 0;
	wifi_gate_changed = 0;
	wifi_port_changed = 0;
	wifi_mode_changed = 0;
	wificonfig_in_menu = IP_NO_CHANGE;
	count_write_ghost_system_wifi = 0;
}

static void wifi_ip_right(uint8 *edit, const uint8 *live, uint8 *changed, uint8 col0)
{
	if(in_sub_menu == FALSE)
	{
		set_value = edit[0];
		show_wifi_edit(edit, 0, set_value, col0, 0);
		in_sub_menu = TRUE;
		sub_menu_index = 0;
	}
	else if(sub_menu_index < 3)
	{
		edit[sub_menu_index] = set_value;
		sub_menu_index++;
		set_value = edit[sub_menu_index];
		show_wifi_edit(edit, sub_menu_index, set_value, col0, 0);
	}
	else
	{
		edit[3] = set_value;
		show_wifi_edit(edit, 4, 0, col0, 1);
		*changed = addr_differ(edit, live);
		in_sub_menu = FALSE;
		sub_menu_index = 0;
	}
}

static void wifi_ip_left(uint8 *edit, uint8 col0)
{
	if(sub_menu_index == 0)
	{
		show_wifi_edit(edit, 4, 0, col0, 1);
		in_sub_menu = FALSE;
	}
	else
	{
		sub_menu_index--;
		set_value = edit[sub_menu_index];
		show_wifi_edit(edit, sub_menu_index, set_value, col0, 0);
	}
}
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
static uint8 const wifi_mode_text[2][7] = 
{
	"Dhcp",
	"Static",
};

static uint8 const new_ip_status_text[4][10] = 
{
	"No-change",
	"Y/N",
	"Confirm",
	"Cannel",
};

uint8 const baudrate_text[12][7] = 
{
	"1200",
	"2400",
	"3600",
	"4800",
	"7200",
	"9600",
	"19200",
	"38400",
	"76800",
	"115200",
	"921600",
	"57600"
	
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

void Misc_init(void)
{
	uint8 i, display_lines;

	//if(ipconfig_in_menu == IP_DISABLE)
	{
		
		ghost_ip_mode = Modbus.tcp_type;

		for(i=0; i<4; i++)
		{
			ghost_ip_addr[i] = Modbus.ip_addr[i];
			ghost_mask_addr[i] = Modbus.subnet[i];
			ghost_gate_addr[i] = Modbus.getway[i]; 					
		} 
		ghost_listen_port = Modbus.tcp_port;
		wifi_load_ghost();
		wifi_ip_changed = 0;
		wifi_mask_changed = 0;
		wifi_gate_changed = 0;
		wifi_port_changed = 0;
		wifi_mode_changed = 0;
		wificonfig_in_menu = IP_NO_CHANGE;

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
//				sprintf((char *)text, "%s%s", item_name[i + item_index / 5 * 5], ip_mode_text[ghost_ip_mode]);
				strcat((char *)text, (char *)ip_mode_text[ghost_ip_mode]);	
				break;
			case 1:
				append_eth_shown(0);
				break;
			case 2:
				append_eth_shown(1);
				break;
			case 3:
				append_eth_shown(2);
				break;
			case 4:
//				sprintf((char *)text, "%s%u", item_name[i + item_index / 5 * 5], ghost_listen_port);
				itoa(ghost_listen_port, int_text, 0);
				strcat((char *)text, (char *)int_text);
				break;
			case 5:	
//				sprintf((char *)text, "%s%s", item_name[i + item_index / 5 * 5], new_ip_status_text[ipconfig_in_menu]);
				strcat((char *)text, (char *)new_ip_status_text[ipconfig_in_menu]);	
				break;
			case 6: // Modbus ID
//				sprintf((char *)text, "%s%u", item_name[i + item_index / 5 * 5], (uint16)Modbus.address);
				itoa(Modbus.address, int_text, 0);
				strcat((char *)text, (char *)int_text);
				break;
			case 7:
//				sprintf((char *)text, "%s%s", item_name[i + item_index / 5 * 5], baudrate_text[Modbus.baudrate[0]]);
				strcat((char *)text, (char *)baudrate_text[Modbus.baudrate[0]]);
				strcat((char *)text, (char *)PARITY_TEXT[0/*uart1_parity*/]);
				break;
			case 8:
				if(Modbus.com_config[0] == MODBUS_SLAVE || Modbus.com_config[0] == MODBUS_MASTER) 
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
//				sprintf((char *)text, "%s%s", item_name[i + item_index / 5 * 5], YesOrNo[use_password]);
				strcat((char *)text, (char *)YesOrNo[use_password]);
				break;
			case 12:
				strncat((char *)text, (char *)user_password, 4);
				break;			
			case 13:
				strcat((char *)text, (char *)YesOrNo[alarmEnable]);
				break;
			case 14:
				strcat((char *)text, (char *)YesOrNo[enableScroll]);
				break;
			case 15:
			case 16:
			case 17:
			case 18:
			case 19:
			case 20:
				append_wifi_field(i + item_index / SET_MENU_ROWS * SET_MENU_ROWS);
				break;
			default:
				break;
		}
		
		if(i == (item_index % SET_MENU_ROWS))
			Lcd_Show_String(i, 0, DISP_INV, text);
		else
			Lcd_Show_String(i, 0, DISP_NOR, text);
	}

	in_sub_menu = FALSE;
}

static uint8 ip_paint_valid[6];
static uint8 ip_paint_cache[6][4];

static void paint_ip_row(uint8 slot, uint8 index, const uint8 *addr)
{
	uint8 page;

	page = (uint8)(item_index / SET_MENU_ROWS * SET_MENU_ROWS);
	if(index < page || index >= page + SET_MENU_ROWS)
		return;
	if(ip_paint_valid[slot] && addr[0] == ip_paint_cache[slot][0] && addr[1] == ip_paint_cache[slot][1]
		&& addr[2] == ip_paint_cache[slot][2] && addr[3] == ip_paint_cache[slot][3])
		return;
	ip_paint_cache[slot][0] = addr[0];
	ip_paint_cache[slot][1] = addr[1];
	ip_paint_cache[slot][2] = addr[2];
	ip_paint_cache[slot][3] = addr[3];
	ip_paint_valid[slot] = 1;
	strcpy((char *)text, (char *)item_name[index]);
	if(index >= 16)
		append_plain(addr);
	else
		append_dotted(addr);
	Lcd_Clear_Row(index % SET_MENU_ROWS);
	Lcd_Show_String(index % SET_MENU_ROWS, 0, (index == item_index) ? DISP_INV : DISP_NOR, text);
}

static void refresh_shown_ip_rows(void)
{
	uint8 addr[4];

	shown_addr(0, ghost_ip_addr, ip_changed_in_menu, 1, 0, addr);
	paint_ip_row(0, 1, addr);
	shown_addr(1, ghost_mask_addr, subnet_changed_in_menu, 2, 0, addr);
	paint_ip_row(1, 2, addr);
	shown_addr(2, ghost_gate_addr, gateway_changed_in_menu, 3, 0, addr);
	paint_ip_row(2, 3, addr);
	shown_addr(0, ghost_wifi_ip, wifi_ip_changed, 16, 1, addr);
	paint_ip_row(3, 16, addr);
	shown_addr(1, ghost_wifi_mask, wifi_mask_changed, 17, 1, addr);
	paint_ip_row(4, 17, addr);
	shown_addr(2, ghost_wifi_gate, wifi_gate_changed, 18, 1, addr);
	paint_ip_row(5, 18, addr);
}

void Misc_display(void)
{
	ipconfig_in_menu = ip_changed_in_menu | subnet_changed_in_menu | gateway_changed_in_menu | port_changed_in_menu | mode_changed_in_menu;
	wificonfig_in_menu = wifi_ip_changed | wifi_mask_changed | wifi_gate_changed | wifi_port_changed | wifi_mode_changed;
	if(in_sub_menu == FALSE)
	{
		if(pre_item_index != item_index)
		{
			strcpy((char *)text, (char *)item_name[pre_item_index]);
			switch(pre_item_index)
			{
				case 0:
	//				sprintf((char *)text, "%s%s", item_name[i + item_index / 5 * 5], ip_mode_text[ghost_ip_mode]);
					strcat((char *)text, (char *)ip_mode_text[ghost_ip_mode]);	
					break;
				case 1:
					append_eth_shown(0);
					break;
				case 2:
					append_eth_shown(1);
					break;
				case 3:
					append_eth_shown(2);
					break;
				case 4:
	//				sprintf((char *)text, "%s%u", item_name[i + item_index / 5 * 5], ghost_listen_port);
					itoa(ghost_listen_port, int_text, 0);
					strcat((char *)text, (char *)int_text);
					break;
				case 5:	
	//				sprintf((char *)text, "%s%s", item_name[i + item_index / 5 * 5], new_ip_status_text[ipconfig_in_menu]);
					strcat((char *)text, (char *)new_ip_status_text[ipconfig_in_menu]);	
					break;
				case 6: // Modbus ID
	//				sprintf((char *)text, "%s%u", item_name[i + item_index / 5 * 5], (uint16)Modbus.address);
					itoa(Modbus.address, int_text, 0);
					strcat((char *)text, (char *)int_text);
					break;
				case 7:
	//				sprintf((char *)text, "%s%s", item_name[i + item_index / 5 * 5], baudrate_text[Modbus.baudrate[0]]);
					strcat((char *)text, (char *)baudrate_text[Modbus.baudrate[0]]);
				    strcat((char *)text, (char *)PARITY_TEXT[0/*uart1_parity*/]);
					break;
				
				case 8:
					if(Modbus.com_config[0] == MODBUS_SLAVE || Modbus.com_config[0] == MODBUS_MASTER) 
						strcat((char *)text, (char *)protocol_text[1]);
					else
						strcat((char *)text, (char *)protocol_text[0]);
					break; 
				case 9: // Date
	//				sprintf((char *)text, "%s%04d-%02d-%02d", item_name[i + item_index / 5 * 5], (uint16)rtc_date.year, (uint16)rtc_date.month, (uint16)rtc_date.day);
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
	//				sprintf((char *)text, "%s%02d:%02d", item_name[i + item_index / 5 * 5], (uint16)rtc_date.hour, (uint16)rtc_date.minute);
					 
//					itoa(rtc_date.hour, int_text, 0);
//					strcat((char *)text, (char *)int_text);
//					strcat((char *)text, (char *)":");
//					itoa(rtc_date.minute, int_text, 0);
					sprintf((char *)int_text, "%02d:%02d",(uint16)rtc_date.hour, (uint16)rtc_date.minute);
					strcat((char *)text, (char *)int_text);
					break;
//				case 10:	// Ring on time
//	//				sprintf((char *)text, "%s%u", item_name[i + item_index / 5 * 5], (uint16)pre_alarm_on_time);
//					itoa(pre_alarm_on_time, int_text, 0);
//					strcat((char *)text, (char *)int_text);
//					break;
//				case 11: // Ring off time
//	//				sprintf((char *)text, "%s%u", item_name[i + item_index / 5 * 5], (uint16)pre_alarm_off_time);
//					itoa(pre_alarm_off_time, int_text, 0);
//					strcat((char *)text, (char *)int_text);
//					break;
				case 11:
	//				sprintf((char *)text, "%s%s", item_name[i + item_index / 5 * 5], YesOrNo[use_password]);
					strcat((char *)text, (char *)YesOrNo[use_password]);
					break;
				case 12:
	//				sprintf((char *)text, "%s%c%c%c%c", item_name[i + item_index / 5 * 5], user_password[0], user_password[1], user_password[2], user_password[3]);
			 
					strncat((char *)text, (char *)user_password, 4);
					break;
				case 13:
					strcat((char *)text, (char *)YesOrNo[alarmEnable]);
					break;
				case 14:
					strcat((char *)text, (char *)YesOrNo[enableScroll]);
					break;
				case 15:
				case 16:
				case 17:
				case 18:
				case 19:
				case 20:
					append_wifi_field(pre_item_index);
					break;
				default:
					break;
			}
			Lcd_Clear_Row(pre_item_index % SET_MENU_ROWS);
			Lcd_Show_String(pre_item_index % SET_MENU_ROWS, 0, DISP_NOR, text);
			strcpy((char *)text, (char *)item_name[item_index]);
			switch(item_index)
			{
				case 0:
	//				sprintf((char *)text, "%s%s", item_name[i + item_index / 5 * 5], ip_mode_text[ghost_ip_mode]);
					strcat((char *)text, (char *)ip_mode_text[ghost_ip_mode]);	
					break;
				case 1:
					append_eth_shown(0);
					break;
				case 2:
					append_eth_shown(1);
					break;
				case 3:
					append_eth_shown(2);
					break;
				case 4:
	//				sprintf((char *)text, "%s%u", item_name[i + item_index / 5 * 5], ghost_listen_port);
					itoa(ghost_listen_port, int_text, 0);
					strcat((char *)text, (char *)int_text);
					break;
				case 5:	
	//				sprintf((char *)text, "%s%s", item_name[i + item_index / 5 * 5], new_ip_status_text[ipconfig_in_menu]);
					strcat((char *)text, (char *)new_ip_status_text[ipconfig_in_menu]);	
					break;
				case 6: // Modbus ID
	//				sprintf((char *)text, "%s%u", item_name[i + item_index / 5 * 5], (uint16)Modbus.address);
					itoa(Modbus.address, int_text, 0);
					strcat((char *)text, (char *)int_text);
					break;
				case 7:
	//				sprintf((char *)text, "%s%s", item_name[i + item_index / 5 * 5], baudrate_text[Modbus.baudrate[0]]);
					strcat((char *)text, (char *)baudrate_text[Modbus.baudrate[0]]);
				    strcat((char *)text, (char *)PARITY_TEXT[0/*uart1_parity*/]);
					break;
				case 8:
					if(Modbus.com_config[0] == MODBUS_SLAVE || Modbus.com_config[0] == MODBUS_MASTER) 
						strcat((char *)text, (char *)protocol_text[1]);
					else
						strcat((char *)text, (char *)protocol_text[0]);
					break;
				case 9: // Date
	//				sprintf((char *)text, "%s%04d-%02d-%02d", item_name[i + item_index / 5 * 5], (uint16)rtc_date.year, (uint16)rtc_date.month, (uint16)rtc_date.day);
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
	//				sprintf((char *)text, "%s%02d:%02d", item_name[i + item_index / 5 * 5], (uint16)rtc_date.hour, (uint16)rtc_date.minute);
					 
//					itoa(rtc_date.hour, int_text, 0);
//					strcat((char *)text, (char *)int_text);
//					strcat((char *)text, (char *)":");
//					itoa(rtc_date.minute, int_text, 0);
					sprintf((char *)int_text, "%02d:%02d",(uint16)rtc_date.hour, (uint16)rtc_date.minute);
					strcat((char *)text, (char *)int_text);
					break;
//				case 10:	// Ring on time
//	//				sprintf((char *)text, "%s%u", item_name[i + item_index / 5 * 5], (uint16)pre_alarm_on_time);
//					itoa(pre_alarm_on_time, int_text, 0);
//					strcat((char *)text, (char *)int_text);
//					break;
//				case 11: // Ring off time
//	//				sprintf((char *)text, "%s%u", item_name[i + item_index / 5 * 5], (uint16)pre_alarm_off_time);
//					itoa(pre_alarm_off_time, int_text, 0);
//					strcat((char *)text, (char *)int_text);
//					break;
				case 11:
	//				sprintf((char *)text, "%s%s", item_name[i + item_index / 5 * 5], YesOrNo[use_password]);
					strcat((char *)text, (char *)YesOrNo[use_password]);
					break;
				case 12:
	//				sprintf((char *)text, "%s%c%c%c%c", item_name[i + item_index / 5 * 5], user_password[0], user_password[1], user_password[2], user_password[3]);
				 
					strncat((char *)text, (char *)user_password, 4);
					break;
				
				case 13:
					strcat((char *)text, (char *)YesOrNo[alarmEnable]);
					break;
				case 14:
					strcat((char *)text, (char *)YesOrNo[enableScroll]);
					break;
				case 15:
				case 16:
				case 17:
				case 18:
				case 19:
				case 20:
					append_wifi_field(item_index);
					break;
				default:
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
		refresh_shown_ip_rows();
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
					show_wifi_edit(ghost_ip_addr, sub_menu_index, set_value, 5, 0);
					break;
				case 2: // MASK
					show_wifi_edit(ghost_mask_addr, sub_menu_index, set_value, 6, 0);
					break;
				case 3:	// GATE
					show_wifi_edit(ghost_gate_addr, sub_menu_index, set_value, 6, 0);
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
				case 15:
					sprintf((char *)text, "%s", wifi_mode_text[set_value]);
					Lcd_Show_String(item_index % SET_MENU_ROWS, WIFI_MODE_COL, DISP_NOR, (uint8 *)"      ");
					Lcd_Show_String(item_index % SET_MENU_ROWS, WIFI_MODE_COL, DISP_INV, text);
					draw_wifi_addr_rows(set_value);
					break;
				case 16:
					show_wifi_edit(ghost_wifi_ip, sub_menu_index, set_value, WIFI_IP_COL, 0);
					break;
				case 17:
					show_wifi_edit(ghost_wifi_mask, sub_menu_index, set_value, WIFI_MSK_COL, 0);
					break;
				case 18:
					show_wifi_edit(ghost_wifi_gate, sub_menu_index, set_value, WIFI_GAT_COL, 0);
					break;
				case 19:
					sprintf((char *)text, "%u", set_value);
					Lcd_Show_String(item_index % SET_MENU_ROWS, WIFI_PORT_COL, DISP_NOR, (uint8 *)"     ");
					Lcd_Show_String(item_index % SET_MENU_ROWS, WIFI_PORT_COL, DISP_INV, text);
					break;
				case 20:
					sprintf((char *)text, "%s", new_ip_status_text[set_value]);
					Lcd_Show_String(item_index % SET_MENU_ROWS, WIFI_NEW_COL, DISP_NOR, (uint8 *)"         ");
					Lcd_Show_String(item_index % SET_MENU_ROWS, WIFI_NEW_COL, DISP_INV, text);
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
				if(!((item_index >= 1 && item_index <= 3) || (item_index >= 16 && item_index <= 18)))
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
							if(Modbus.com_config[0] == MODBUS_SLAVE || Modbus.com_config[0] == MODBUS_MASTER) 
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
						case 15:
							sprintf((char *)text, "%s%s", item_name[item_index], wifi_mode_text[ghost_wifi_mode > 1 ? 0 : ghost_wifi_mode]);
							break;
						case 19:
							sprintf((char *)text, "%s%u", item_name[item_index], ghost_wifi_port);
							break;
						case 20:
							sprintf((char *)text, "%s%s", item_name[item_index], new_ip_status_text[wificonfig_in_menu]);
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
							if(sub_menu_index == 0)
							{
								show_wifi_edit(ghost_ip_addr, 4, 0, 5, 1);
								in_sub_menu = FALSE;
							}
							else
							{
								sub_menu_index--;
								set_value = ghost_ip_addr[sub_menu_index];
								show_wifi_edit(ghost_ip_addr, sub_menu_index, set_value, 5, 0);
							}
							break;
						case 2:
							if(sub_menu_index == 0)
							{
								show_wifi_edit(ghost_mask_addr, 4, 0, 6, 1);
								in_sub_menu = FALSE;
							}
							else
							{
								sub_menu_index--;
								set_value = ghost_mask_addr[sub_menu_index];
								show_wifi_edit(ghost_mask_addr, sub_menu_index, set_value, 6, 0);
							}
							break;
						case 3:
							if(sub_menu_index == 0)
							{
								show_wifi_edit(ghost_gate_addr, 4, 0, 6, 1);
								in_sub_menu = FALSE;
							}
							else
							{
								sub_menu_index--;
								set_value = ghost_gate_addr[sub_menu_index];
								show_wifi_edit(ghost_gate_addr, sub_menu_index, set_value, 6, 0);
							}
							break;
						case 16:
							wifi_ip_left(ghost_wifi_ip, WIFI_IP_COL);
							break;
						case 17:
							wifi_ip_left(ghost_wifi_mask, WIFI_MSK_COL);
							break;
						case 18:
							wifi_ip_left(ghost_wifi_gate, WIFI_GAT_COL);
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
						ghost_ip_mode = set_value;
						mode_changed_in_menu = (ghost_ip_mode != Modbus.tcp_type);

						sprintf((char *)text, "%s%s", item_name[item_index], ip_mode_text[ghost_ip_mode]);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_INV, text);

						in_sub_menu = FALSE;
					}
					break;
				case 1: // ip
					if(in_sub_menu == FALSE)
					{
						if(!ip_changed_in_menu)
							live_eth_addr(0, ghost_ip_addr);
						ip_paint_valid[0] = 0;
						set_value = ghost_ip_addr[0];
						show_wifi_edit(ghost_ip_addr, 0, set_value, 5, 0);
						in_sub_menu = TRUE;
						sub_menu_index = 0;
					}
					else if(sub_menu_index < 3)
					{
						ghost_ip_addr[sub_menu_index] = set_value;
						sub_menu_index++;
						set_value = ghost_ip_addr[sub_menu_index];
						show_wifi_edit(ghost_ip_addr, sub_menu_index, set_value, 5, 0);
					}
					else
					{
						ghost_ip_addr[3] = set_value;
						show_wifi_edit(ghost_ip_addr, 4, 0, 5, 1);
						if((Modbus.ip_addr[0] != ghost_ip_addr[0]) || (Modbus.ip_addr[1] != ghost_ip_addr[1]) || (Modbus.ip_addr[2] != ghost_ip_addr[2]) || (Modbus.ip_addr[3] != ghost_ip_addr[3]))
							ip_changed_in_menu = 1;
						else
							ip_changed_in_menu = 0;
						in_sub_menu = FALSE;
						sub_menu_index = 0;
					}
					break;
			   case 2: // subnet mask address
					if(in_sub_menu == FALSE)
					{
						if(!subnet_changed_in_menu)
							live_eth_addr(1, ghost_mask_addr);
						ip_paint_valid[1] = 0;
						set_value = ghost_mask_addr[0];
						show_wifi_edit(ghost_mask_addr, 0, set_value, 6, 0);
						in_sub_menu = TRUE;
						sub_menu_index = 0;
					}
					else if(sub_menu_index < 3)
					{
						ghost_mask_addr[sub_menu_index] = set_value;
						sub_menu_index++;
						set_value = ghost_mask_addr[sub_menu_index];
						show_wifi_edit(ghost_mask_addr, sub_menu_index, set_value, 6, 0);
					}
					else
					{
						ghost_mask_addr[3] = set_value;
						show_wifi_edit(ghost_mask_addr, 4, 0, 6, 1);
						if((Modbus.subnet[0] != ghost_mask_addr[0]) || (Modbus.subnet[1] != ghost_mask_addr[1]) || (Modbus.subnet[2] != ghost_mask_addr[2]) || (Modbus.subnet[3] != ghost_mask_addr[3]))
							subnet_changed_in_menu = 1;
						else
							subnet_changed_in_menu = 0;
						in_sub_menu = FALSE;
						sub_menu_index = 0;
					}
					break;
				case 3: // gateway address
					if(in_sub_menu == FALSE)
					{
						if(!gateway_changed_in_menu)
							live_eth_addr(2, ghost_gate_addr);
						ip_paint_valid[2] = 0;
						set_value = ghost_gate_addr[0];
						show_wifi_edit(ghost_gate_addr, 0, set_value, 6, 0);
						in_sub_menu = TRUE;
						sub_menu_index = 0;
					}
					else if(sub_menu_index < 3)
					{
						ghost_gate_addr[sub_menu_index] = set_value;
						sub_menu_index++;
						set_value = ghost_gate_addr[sub_menu_index];
						show_wifi_edit(ghost_gate_addr, sub_menu_index, set_value, 6, 0);
					}
					else
					{
						ghost_gate_addr[3] = set_value;
						show_wifi_edit(ghost_gate_addr, 4, 0, 6, 1);
						if((Modbus.getway[0] != ghost_gate_addr[0]) || (Modbus.getway[1] != ghost_gate_addr[1]) || (Modbus.getway[2] != ghost_gate_addr[2]) || (Modbus.getway[3] != ghost_gate_addr[3]))
							gateway_changed_in_menu = 1;
						else
							gateway_changed_in_menu = 0;
						in_sub_menu = FALSE;
						sub_menu_index = 0;
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
						if(ghost_listen_port != Modbus.tcp_port)
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
						mode_changed_in_menu = 0;
						if(ipconfig_in_menu == IP_DISABLE)
						{
							Misc_init();							
						}
						else if(ipconfig_in_menu == IP_ENABLE)
						{
							uint8 n;
							for(n = 0; n < 4; n++)
							{
								Modbus.ip_addr[n] = ghost_ip_addr[n];
								Modbus.subnet[n] = ghost_mask_addr[n];
								Modbus.getway[n] = ghost_gate_addr[n];
							}
							Modbus.tcp_port = ghost_listen_port;
							Modbus.tcp_type = ghost_ip_mode;
							save_uint16_to_flash(FLASH_TCP_PORT, Modbus.tcp_port);
							save_uint8_to_flash(FLASH_TCP_TYPE, Modbus.tcp_type);
							Save_Ethernet_Info();
							in_sub_menu = FALSE;
							ipconfig_in_menu = IP_NO_CHANGE;
							esp_retboot();
						}
					}
					break;
				case 15:
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s", item_name[item_index]);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_NOR, text);
						set_value = ghost_wifi_mode;
						in_sub_menu = TRUE;
					}
					else
					{
						ghost_wifi_mode = set_value;
						if(SSID_Info.IP_Auto_Manual > 1)
							wifi_mode_changed = (ghost_wifi_mode != 0);
						else
							wifi_mode_changed = (ghost_wifi_mode != SSID_Info.IP_Auto_Manual);
						sprintf((char *)text, "%s%s", item_name[item_index], wifi_mode_text[ghost_wifi_mode > 1 ? 0 : ghost_wifi_mode]);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_INV, text);
						in_sub_menu = FALSE;
						draw_wifi_addr_rows(ghost_wifi_mode);
					}
					break;
				case 16:
					if(in_sub_menu == FALSE && !wifi_ip_changed)
						live_wifi_addr(0, ghost_wifi_ip);
					ip_paint_valid[3] = 0;
					wifi_ip_right(ghost_wifi_ip, SSID_Info.ip_addr, &wifi_ip_changed, WIFI_IP_COL);
					break;
				case 17:
					if(in_sub_menu == FALSE && !wifi_mask_changed)
						live_wifi_addr(1, ghost_wifi_mask);
					ip_paint_valid[4] = 0;
					wifi_ip_right(ghost_wifi_mask, SSID_Info.net_mask, &wifi_mask_changed, WIFI_MSK_COL);
					break;
				case 18:
					if(in_sub_menu == FALSE && !wifi_gate_changed)
						live_wifi_addr(2, ghost_wifi_gate);
					ip_paint_valid[5] = 0;
					wifi_ip_right(ghost_wifi_gate, SSID_Info.getway, &wifi_gate_changed, WIFI_GAT_COL);
					break;
				case 19:
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s", item_name[item_index]);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_NOR, text);
						set_value = ghost_wifi_port;
						in_sub_menu = TRUE;
					}
					else
					{
						sprintf((char *)text, "%s%u", item_name[item_index], set_value);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_INV, text);
						ghost_wifi_port = set_value;
						wifi_port_changed = (ghost_wifi_port != SSID_Info.modbus_port);
						in_sub_menu = FALSE;
					}
					break;
				case 20:
					if(in_sub_menu == FALSE)
					{
						if(wificonfig_in_menu != IP_NO_CHANGE)
						{
							sprintf((char *)text, "%s", item_name[item_index]);
							Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_NOR, text);
							set_value = wificonfig_in_menu;
							in_sub_menu = TRUE;
						}
					}
					else
					{
						sprintf((char *)text, "%s%s", item_name[item_index], new_ip_status_text[set_value]);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_INV, text);
						wificonfig_in_menu = set_value;
						wifi_ip_changed = 0;
						wifi_mask_changed = 0;
						wifi_gate_changed = 0;
						wifi_port_changed = 0;
						wifi_mode_changed = 0;
						if(wificonfig_in_menu == IP_DISABLE)
						{
							wifi_load_ghost();
							wificonfig_in_menu = IP_NO_CHANGE;
							Misc_init();
						}
						else if(wificonfig_in_menu == IP_ENABLE)
						{
							wifi_apply();
							in_sub_menu = FALSE;
							wificonfig_in_menu = IP_NO_CHANGE;
							esp_retboot();
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
								flag_write_basic_info = 1;
								count_write_basic_info = 0;
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
						if(set_value > 11)
							set_value = 9;
						Modbus.baudrate[0] = set_value;
						save_uint8_to_flash(FLASH_BAUD_RATE, Modbus.baudrate[0]);
						flag_change_uart0 = 1;
						count_change_uart0 = 0;
						sprintf((char *)text, "%s%s", item_name[item_index], baudrate_text[Modbus.baudrate[0]]);
						strcat((char *)text, (char *)PARITY_TEXT[0/*uart1_parity*/]);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_INV, text);
						in_sub_menu = FALSE;
					}
					break;
				case 8:
					if(in_sub_menu == FALSE)
					{
						sprintf((char *)text, "%s", item_name[item_index]);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 0, DISP_NOR, text);
						if(Modbus.com_config[0] == MODBUS_SLAVE || Modbus.com_config[0] == MODBUS_MASTER)
							set_value = 1;
						else
							set_value = 0;
						sprintf((char *)text, "%s", protocol_text[set_value]);
						Lcd_Show_String(item_index % SET_MENU_ROWS, 13, DISP_INV, text);
						in_sub_menu = TRUE;
					}
					else
					{
						if(set_value)
							Modbus.com_config[0] = MODBUS_SLAVE;
						else
							Modbus.com_config[0] = BACNET_SLAVE;
						save_uint8_to_flash(FLASH_UART_CONFIG, Modbus.com_config[0]);
						com_config_back[0] = Modbus.com_config[0];
						uart_init(0);
						Count_com_config();
						if(Modbus.com_config[0] == BACNET_SLAVE || Modbus.com_config[0] == BACNET_MASTER)
						{
							Recievebuf_Initialize(0);
							dlmstp_init(NULL);
							Send_I_Am_Flag = 1;
							Send_Whois_Flag = 1;
						}
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
							PCF_SetDateTime(&rtc_date);							
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
							PCF_SetDateTime(&rtc_date);
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
							PCF_SetDateTime(&rtc_date);
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
							PCF_SetDateTime(&rtc_date);
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
							PCF_SetDateTime(&rtc_date);							
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
					case 15:
						if(set_value == 1)
							set_value = 0;
						else
							set_value = 1;
						break;
					case 1:	// ip address
					case 2:	// subnet mask address
					case 3:	// gateway address
					case 16:
					case 17:
					case 18:
						if((key_value & KEY_FUNCTION_MASK) == KEY_SPEED_1)
							set_value += SPEED_1;
						else
							set_value = (((set_value + SPEED_10) < 255) ? (set_value + SPEED_10) : 1);

						set_value %= 255;
						break;
					case 4:	// tcp port
					case 19:
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
					case 20:
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
						if(set_value >= 11)
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
					case 15:
						if(set_value == 1)
							set_value = 0;
						else
							set_value = 1;
						break;
					case 1:	// ip address
					case 2:	// subnet mask address
					case 3:	// gateway address
					case 16:
					case 17:
					case 18:
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
					case 19:
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
					case 20:
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
							set_value = 11;
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




