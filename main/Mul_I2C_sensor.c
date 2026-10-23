#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "types.h"
#include "define.h"
#include "Mul_I2C_sensor.h"
#include "modbus.h"
#include "flash.h"
#include "nvs.h"
#include "rtc.h"
#include "WireGuard_App.h"

extern void update_timers(void);

extern uint16_t read_wifi_data_by_block(uint16_t addr);
extern void write_wifi_data_by_block(uint16_t StartAdd, uint8_t HeadLen, uint8_t *pData, uint8_t type);
extern uint8 backlight_keep_seconds;
extern void start_back_light(uint8 timer);


extern STR_Task_Test task_test;

Str_points_ptr put_io_buf(Point_type_equate type, uint8 point);

const uint8 Var_label[13][9] = {

	"Baudrate",   //0
	"StnNumer",   //1
	"Protocol", //2
	"Instance",//3
	"Unit", //10
	"Trgger_S",
	"Timer_S",
	"Trgger_L",
	"Timer_L",
	"Trgger_O",
	"Timer_O",
	"Trgger_C",
	"Timer_C",

};
const uint8 Var_Description[13][21] = {

	"baudrate select",   	//0
	"station number",   	//1
	"modbus/bacnet switch",    //2
	"instance number",		//3
	"temprature unit",//10
	"Trigger of Sound",
	"Timer of Sound",
	"Trigger of light",
	"Timer of light",
	"Trigger of OCC",
	"Timer of OCC",
	"Trigger of CO2",
	"Timer of CO2",
};

const uint8 Prs_Unit[][6] = 
{
	"inWC ",
	"KPa  ",
	"Psi  ",
	"mmHg ",
	"inHg ",
	"Kgcm ",
	"atmos",
	"bar  ",
	"Pa   ",
};

typedef struct
{
	int16 pr_range[2];
} STR_Prs;

static STR_Prs Pressure;



static float get_mmhg_ratio(unsigned char unit)  //rate = unit/mmhg*10^7
{
 
	float Ftemp = 0;  //10^7
	
	switch (unit)
	{													   
	   case inWC:         //W.C				0.01		 0	
				Ftemp = 5353000;
		 break;
	   case KPa1:          //KPa 			0.0001		 0	
				Ftemp = 1332800;
		 break;  
	   case Psi:          //Psi				0.00001		0.1
				Ftemp = 193400;				
		 break; 
	   case mmHg:         //mmHg			0.01		0
				Ftemp = 10000000;			
		 break;
	   case inHg:         //inches of Hg	0.00001		0.01
				Ftemp = 393700;				
		 break;
	   case Kg_cm2:       //Kg/cm2			0.000001	0.001
				Ftemp = 13595;
		 break;  
	   case atmosphere:   //atmosphere		0.000001	0.001
				Ftemp = 13158;
		 break;
	   case bar:          //bar				0.000001	0.001
				Ftemp = 13328;				
	   break; 
	   case pa:          //bar				0.000001	0.001
				Ftemp = 1332800000;				
	   break; 
		 
	}
    return Ftemp;
}



// *************************************************************************************
// 	input: new unit   default unit
//	output: get the  ratio between the two unit 
// ************************************************************************************* 

static float get_units_ratio(unsigned char new_unit,unsigned char default_unit)
{  
	  float Ftemp; 
	  Ftemp = get_mmhg_ratio(new_unit)/get_mmhg_ratio(default_unit);
	  return Ftemp;
} 


 
// *************************************************************************************
// 	input: pressure value, data base and data index unit
//	output: none  
// ************************************************************************************* 
static const u8 decimal_num[3][9] = {
	{2,4,5,2,5,6,6,6,0},		//default unit = inWC
	{0,0,1,0,2,3,3,3,0},		//default unit = Psi
	{2,3,3,2,3,3,3,3,0}		//default unit = Pa, LCD digits: inWC 2, kPa 3, Psi 3, mmHg 2, inHg 3, Kg/atm/bar 3, Pa 0
};

 
static void datatype_convert(float input, S32_T *data_base, u16 *data_index,u8 data_unit,u8 data_default_unit)
{
	u8 i; 
	u8 DECIMAL_NUM = 0; 
	if(data_default_unit == inWC)
		DECIMAL_NUM = decimal_num[0][data_unit];
	else if(data_default_unit == Psi) 
		DECIMAL_NUM = decimal_num[1][data_unit];
	else if(data_default_unit == pa)  // new sensor
		DECIMAL_NUM = decimal_num[2][data_unit];
	else
		DECIMAL_NUM = 0;
	 
	for(i = 0;i<DECIMAL_NUM;i++)   
		input *= 10;
	 
	
	if(data_default_unit == inWC)		//when the default unit is inwc, it has two decimals
		*data_base =  input/100;
	else if(data_default_unit == Psi)   //when the default unit is psi, it has one decimals
		*data_base =  input/10;
	
	
	if(*data_base == 0) *data_index = 0;
	else
		*data_index = 0 - DECIMAL_NUM; 
	
}

u8 get_default_unit(u8 pro_model)
{
	u8 temp;
/*	if(pro_model == DLVR_L01D)
	{
		temp = inWC;
		Pressure.pr_range[0] = -100;
		Pressure.pr_range[1] = 100;
	}
	else if(pro_model ==MPXV7002DP)
	{ 
		temp = inWC;
		Pressure.pr_range[0] = -800;
		Pressure.pr_range[1] = 800;
	}
	else if(pro_model ==MPXV7007DP)
	{
		temp = inWC;
		Pressure.pr_range[0] = -2700;
		Pressure.pr_range[1] = 2700;
	}
	else if(pro_model ==MS4515)
	{
		temp = inWC;
		Pressure.pr_range[0] = -1000;
		Pressure.pr_range[1] = 1000;
	}
	else if(pro_model ==PRS_26PCDFA)
	{
		temp = Psi;
		Pressure.pr_range[0] = -1000;
		Pressure.pr_range[1] = 1000;
	}
	else if(pro_model ==PRS_26PCGFA)
	{
		temp = Psi;
		Pressure.pr_range[0] = -2500;
		Pressure.pr_range[1] = 2500;
	}
	else if(pro_model == AMP612X)
	{
		temp = pa;
		Pressure.pr_range[0] = -500;
		Pressure.pr_range[1] = 500;		
	}
	else*/
	{
		temp = pa;
		Pressure.pr_range[0] = -5000;
		Pressure.pr_range[1] = 5000;
	}
	return temp;
}

int16_t pressure_to_display(int16_t pressure_pa, uint8_t *decimals)
{
	uint8_t unit;
	uint8_t dec;
	uint8_t i;
	float scaled;
	int32_t shown;

	unit = (uint8_t)Mul_i2c_data.pressure_unit;
	if(unit > pa)
		unit = pa;
	dec = decimal_num[2][unit];
	{
		int32_t raw = (int32_t)pressure_pa + (int32_t)Mul_i2c_data.pressure_value_org_offset;
		if(raw > 32767)
			raw = 32767;
		else if(raw < -32768)
			raw = -32768;
		pressure_pa = (int16_t)raw;
	}
	scaled = (float)pressure_pa;
	if(unit != pa)
		scaled *= get_units_ratio(unit, pa);
	for(i = 0; i < dec; i++)
		scaled *= 10.0f;
	if(scaled >= 0)
		shown = (int32_t)(scaled + 0.5f);
	else
		shown = (int32_t)(scaled - 0.5f);
	if(shown > 32767)
		shown = 32767;
	else if(shown < -32768)
		shown = -32768;
	if(decimals != NULL)
		*decimals = dec;
	Mul_i2c_data.pressure_value_show = (int16_t)shown;
	return (int16_t)shown;
}

// uint8 isBlankScreen;
// uint8_t display_config[5];
// default output range table
_RANGE_ default_output_range_table[4] =
{
	{-400, 600},	/* CHANNEL_TEMP, 0.1C: -40..60 */
	{0, 1000},		/* CHANNEL_HUM, 0.1% */
	{0, 2000},		/* CHANNEL_CO2, ppm */
	{0, 5000},		/* CHANNEL_PRE, Pa */
}; 


STR_Mul_Sensor Mul_i2c_data = {
	.sensor_select = 2,
	.co2_channel = 2,
	.hum_output_sel = 1,
	.dew_pt_min = 70,
	.dew_pt_max = -30,
	.pressure_sensor_model = WF100DPZ,
	.pressure_unit = pa,
	.pressure_unit_default = pa,
	.screenArea = {0xff, 0xff, 0xff},
	.output_range = {
		{-400, 600},
		{0, 1000},
		{0, 2000},
		{0, 5000},
	},
};

static uint8_t mul_sensor_index(void)
{
	uint8_t sensor;

	sensor = Mul_i2c_data.sensor_select;
	if(sensor > 2)
		sensor = 2;
	return sensor;
}

uint8_t mul_co2_index(void)
{
	if(Mul_i2c_data.co2_channel > 2)
		Mul_i2c_data.co2_channel = 2;
	return Mul_i2c_data.co2_channel;
}

void save_co2_cal(uint8_t bus)
{
	static const char *asc_key[3] = {FLASH_CO2_ASC0, FLASH_CO2_ASC1, FLASH_CO2_ASC2};
	static const char *frc_key[3] = {FLASH_CO2_FRC0, FLASH_CO2_FRC1, FLASH_CO2_FRC2};
	nvs_handle_t handle;
	esp_err_t err;

	if(bus > 2)
		return;
	err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle);
	if(err != ESP_OK)
		return;
	nvs_set_u8(handle, asc_key[bus], Mul_i2c_data.flag_co2_asc[bus]);
	nvs_set_u16(handle, frc_key[bus], Mul_i2c_data.co2_frc[bus]);
	nvs_commit(handle);
	nvs_close(handle);
}

void set_co2_channel(uint8_t bus)
{
	if(bus > 2)
		bus = 2;
	Mul_i2c_data.co2_channel = bus;
	save_uint8_to_flash(FLASH_CO2_BUS, Mul_i2c_data.co2_channel);
}

void set_co2_frc_cal(uint16_t value)
{
	uint8_t bus;

	bus = mul_co2_index();
	Mul_i2c_data.co2_frc[bus] = value;
	Mul_i2c_data.co2_cmd_bus = bus;
	Mul_i2c_data.flag_asc_or_frc = 2;
	Mul_i2c_data.I2C_Sensor[bus].co2_org = value;
	save_co2_cal(bus);
}

static int8_t first_sensor_bus(const uint8_t *flag)
{
	uint8_t i;

	for(i = 0; i < 3; i++)
	{
		if(flag[i] == 3)
			return (int8_t)i;
	}
	return -1;
}

/* Keep the current bus when it is still present. Otherwise take the first present bus. */
static int8_t keep_or_first_bus(uint8_t current, const uint8_t *flag)
{
	if(current < 3 && flag[current] == 3)
		return (int8_t)current;
	return first_sensor_bus(flag);
}

void save_int_temp_offset(void)
{
	nvs_handle_t handle;
	esp_err_t err;

	err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle);
	if(err != ESP_OK)
		return;
	err = nvs_set_i16(handle, FLASH_INT_TEMP_OFFSET, Mul_i2c_data.int_temp_offset);
	if(err == ESP_OK)
		nvs_commit(handle);
	nvs_close(handle);
}

void save_i2c_sensor_offsets(uint8_t sensor)
{
	static const char *tem_key[3] = {FLASH_TEM_OFFSET0, FLASH_TEM_OFFSET1, FLASH_TEM_OFFSET2};
	static const char *hum_key[3] = {FLASH_HUM_OFFSET0, FLASH_HUM_OFFSET1, FLASH_HUM_OFFSET2};
	nvs_handle_t handle;
	esp_err_t err;

	if(sensor > 2)
		return;
	err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle);
	if(err != ESP_OK)
		return;
	nvs_set_i16(handle, tem_key[sensor], Mul_i2c_data.I2C_Sensor[sensor].tem_offset);
	nvs_set_i16(handle, hum_key[sensor], Mul_i2c_data.I2C_Sensor[sensor].hum_offset);
	nvs_commit(handle);
	nvs_close(handle);
}

int16_t calibrated_humidity(uint8_t sensor, uint16_t raw)
{
	int16_t humidity;

	if(sensor > 2)
		sensor = 2;
	humidity = (int16_t)((int16_t)raw + Mul_i2c_data.I2C_Sensor[sensor].hum_offset);
	if(humidity > 1000)
		humidity = 1000;
	else if(humidity < 0)
		humidity = 0;
	return humidity;
}

/* Humidity channel is 0..1000 (0.1%). Dew point maps dew_pt_min -> 0 and dew_pt_max -> 1000. */
int16_t hum_channel_reading(int16_t humidity_tenth)
{
	int32_t lo;
	int32_t hi;
	int32_t span;
	int32_t scaled;

	if(Mul_i2c_data.hum_output_sel != 0)
		return humidity_tenth;

	lo = (int32_t)Mul_i2c_data.dew_pt_min * 10;
	hi = (int32_t)Mul_i2c_data.dew_pt_max * 10;
	span = hi - lo;
	if(span == 0)
		return 0;
	scaled = ((int32_t)Mul_i2c_data.dew_pt - lo) * 1000 / span;
	if(scaled < 0)
		scaled = 0;
	else if(scaled > 1000)
		scaled = 1000;
	return (int16_t)scaled;
}

static void set_int_temp_c(int16_t value)
{
	Mul_i2c_data.int_temp_offset += (int16_t)(value - Mul_i2c_data.internal_temperature_c);
	Mul_i2c_data.internal_temperature_c = value;
	Mul_i2c_data.internal_temperature_f = (int16_t)(320 + value * 9 / 5);
	save_int_temp_offset();
}

static void set_int_temp_f(int16_t value)
{
	int16_t celsius;

	celsius = (int16_t)((value - 320) * 5 / 9);
	Mul_i2c_data.int_temp_offset += (int16_t)(celsius - Mul_i2c_data.internal_temperature_c);
	Mul_i2c_data.internal_temperature_c = celsius;
	Mul_i2c_data.internal_temperature_f = value;
	save_int_temp_offset();
}

static int16_t ext_temp_c(uint8_t sensor)
{
	return (int16_t)((int16_t)Mul_i2c_data.I2C_Sensor[sensor].tem_org
		+ Mul_i2c_data.I2C_Sensor[sensor].tem_offset);
}

static int16_t ext_temp_f(uint8_t sensor)
{
	return (int16_t)(ext_temp_c(sensor) * 9 / 5 + 320);
}

static uint16_t ext_hum(uint8_t sensor)
{
	int16_t humidity;

	humidity = (int16_t)Mul_i2c_data.I2C_Sensor[sensor].hum_org
		+ Mul_i2c_data.I2C_Sensor[sensor].hum_offset;
	if(humidity > 1000)
		humidity = 1000;
	else if(humidity < 0)
		humidity = 0;
	return (uint16_t)humidity;
}

static void set_ext_hum(uint8_t sensor, int16_t value)
{
	if(value > 1000)
		value = 1000;
	else if(value < 0)
		value = 0;
	Mul_i2c_data.I2C_Sensor[sensor].hum_offset =
		(int16_t)(value - (int16_t)Mul_i2c_data.I2C_Sensor[sensor].hum_org);
	save_i2c_sensor_offsets(sensor);
}

static void set_ext_temp_c(uint8_t sensor, int16_t value)
{
	Mul_i2c_data.I2C_Sensor[sensor].tem_offset =
		(int16_t)(value - (int16_t)Mul_i2c_data.I2C_Sensor[sensor].tem_org);
	save_i2c_sensor_offsets(sensor);
}

static void set_ext_temp_f(uint8_t sensor, int16_t value)
{
	set_ext_temp_c(sensor, (int16_t)((value - 320) * 5 / 9));
}

void Get_AVS(void)
{
	
	
}

void CO2_check_calibration(void)
{
	
}

uint8_t screen_area_is_fixed(void)
{
	return Modbus.mul_i2c_mode == MUL_I2C_MODE_HUM
		|| Modbus.mul_i2c_mode == MUL_I2C_MODE_CO2
		|| Modbus.mul_i2c_mode == MUL_I2C_MODE_PRESSURE;
}

void sync_screen_area_to_mode(void)
{
	if(Modbus.mul_i2c_mode == MUL_I2C_MODE_HUM)
	{
		Mul_i2c_data.screenArea[0] = 0;
		Mul_i2c_data.screenArea[1] = 1;
		Mul_i2c_data.screenArea[2] = 3;
	}
	else if(Modbus.mul_i2c_mode == MUL_I2C_MODE_CO2)
	{
		Mul_i2c_data.screenArea[0] = 0;
		Mul_i2c_data.screenArea[1] = 1;
		Mul_i2c_data.screenArea[2] = 2;
	}
	else if(Modbus.mul_i2c_mode == MUL_I2C_MODE_PRESSURE)
	{
		Mul_i2c_data.screenArea[0] = 5;
		Mul_i2c_data.screenArea[1] = 3;
		Mul_i2c_data.screenArea[2] = 3;
	}
	else
		load_screen_area_from_flash();
}

void select_active_i2c_sensor(const uint8_t hum_exists[3], const uint8_t sensor_type[3])
{
	int8_t hum;
	int8_t co2;

	co2 = keep_or_first_bus(Mul_i2c_data.co2_channel, sensor_type);
	if(co2 >= 0)
		Mul_i2c_data.co2_channel = (uint8_t)co2;

	if(Modbus.mul_i2c_mode == MUL_I2C_MODE_HUM)
	{
		hum = keep_or_first_bus(Mul_i2c_data.sensor_select, hum_exists);
		if(hum >= 0)
			Mul_i2c_data.sensor_select = (uint8_t)hum;
	}
	else if(Modbus.mul_i2c_mode == MUL_I2C_MODE_CO2)
	{
		hum = keep_or_first_bus(Mul_i2c_data.sensor_select, hum_exists);
		if(hum >= 0)
			Mul_i2c_data.sensor_select = (uint8_t)hum;
		else if(co2 >= 0)
			Mul_i2c_data.sensor_select = (uint8_t)co2;
	}
}

void mul_idle_reading(int16_t *tem, int16_t *hum, int16_t *co2)
{
	uint8_t sensor;

	if(Modbus.mul_i2c_mode == MUL_I2C_MODE_CUSTOM2)
	{
		Str_points_ptr ptr;
		ptr = put_io_buf(VAR, 0);
		if(ptr.pvar->range == degC || ptr.pvar->range == degF || ptr.pvar->range == RH)
			*tem = ptr.pvar->value / 100;
		else 
			*tem = ptr.pvar->value / 1000;
		ptr = put_io_buf(VAR, 1);
		if(ptr.pvar->range == degC || ptr.pvar->range == degF || ptr.pvar->range == RH)
			*hum = ptr.pvar->value / 100;
		else 
			*hum = ptr.pvar->value / 1000;
		ptr = put_io_buf(VAR, 2);
		if(ptr.pvar->range == degC || ptr.pvar->range == degF || ptr.pvar->range == RH)
			*co2 = ptr.pvar->value / 100;
		else 
			*co2 = ptr.pvar->value / 1000;
	}
	else
	{
		sensor = mul_sensor_index();
		if(tem)
		{
			if(Mul_i2c_data.temperature_sensor_select == 0)
			{
				if(Mul_i2c_data.deg_c_or_f == DEGREE_F)
					*tem = Mul_i2c_data.internal_temperature_f;
				else
					*tem = Mul_i2c_data.internal_temperature_c;
			}
			else if(Mul_i2c_data.deg_c_or_f == DEGREE_F)
				*tem = ext_temp_f(sensor);
			else
				*tem = ext_temp_c(sensor);
		}
		if(hum)
			*hum = (int16_t)ext_hum(sensor);
		if(co2)
			*co2 = (int16_t)Mul_i2c_data.I2C_Sensor[mul_co2_index()].co2_org;
	}
}

uint16_t read_Mul_i2c_sensor_by_block(uint16_t addr, uint8_t *mapped)
{
	uint8_t sensor;

	sensor = mul_sensor_index();
	if(mapped)
		*mapped = 1;

	/* 484 is unused for now. 491-499 stay reserved. */
	if(addr == MODBUS_HUM_SENSOR_HEATING || (addr >= 491 && addr <= 499))
		return 0;

	if(addr == MODBUS_MUL_I2C_MODE)
	{
	  return Modbus.mul_i2c_mode;
	}
	else if(addr == MODBUS_HUM_TEMPERATURE_FAHRENHEIT)
	{
	  return (uint16_t)ext_temp_f(sensor);
	}
	else if(addr == MODBUS_HUM_TEMPERATURE_CELSIUS)
	{
	  return (uint16_t)ext_temp_c(sensor);
	}
	else if(addr == MODBUS_HUM_INT_TEMPERATURE_FAHRENHEIT)
	{
	  return (uint16_t)Mul_i2c_data.internal_temperature_f;
	}
	else if(addr == MODBUS_HUM_INT_TEMPERATURE_CELSIUS)
	{
	  return (uint16_t)Mul_i2c_data.internal_temperature_c;
	}
	else if(addr == MODBUS_HUM_INT_TEMP_OFFSET)
	{
	  return (uint16_t)Mul_i2c_data.int_temp_offset;
	}
	else if(addr == MODBUS_TEMPERATURE_SENSOR_SELECT)
	{
	  return Mul_i2c_data.temperature_sensor_select;
	}
	else if(addr == MODBUS_HUM_TEMPERATURE_DEGREE_C_OR_F)
	{
	  return Mul_i2c_data.deg_c_or_f;
	}
	else if(addr == MODBUS_HUM_OUTPUT_RANGE_MIN_TEM)
	{
	  return Mul_i2c_data.output_range[CHANNEL_TEMP].min;
	}
	else if(addr == MODBUS_HUM_OUTPUT_RANGE_MAX_TEM)
	{
	  return Mul_i2c_data.output_range[CHANNEL_TEMP].max;
	}
	else if(addr == MODBUS_HUM_OUTPUT_RANGE_MIN_HUMIDITY)
	{
	  return Mul_i2c_data.output_range[CHANNEL_HUM].min;
	}
	else if(addr == MODBUS_HUM_OUTPUT_RANGE_MAX_HUMIDITY)
	{
	  return Mul_i2c_data.output_range[CHANNEL_HUM].max;
	}
	else if(addr == MODBUS_HUM_TEMP_OFFSET)
	{
	  return Mul_i2c_data.I2C_Sensor[sensor].tem_offset;
	}
	else if(addr == MODBUS_HUM_HUMIDITY)
	{
	  return ext_hum(sensor);
	}
	else if(addr == MODBUS_HUM_HUMIDITY_OFFSET)
	{
	  return Mul_i2c_data.I2C_Sensor[sensor].hum_offset;
	}
	else if(addr == MODBUS_HUM_EXT_TEMPERATURE_FILTER)
	{
	  return Mul_i2c_data.temperature_filter;
	}
	else if(addr == MODBUS_HUM_HUMIDITY_FILTER)
	{
	  return Mul_i2c_data.humidity_filter;
	}
	else if(addr == MODBUS_HUM_DEW_PT)
	{
	  return Mul_i2c_data.dew_pt;
	}
	else if(addr == MODBUS_HUM_DEW_PT_F)
	{
	  return Mul_i2c_data.dew_pt_f;
	}
	else if(addr == MODBUS_HUM_OUTPUT_SEL)
	{
	  return Mul_i2c_data.hum_output_sel;
	}
	else if(addr == MODBUS_HUM_DEW_PT_MIN)
	{
	  return (uint16_t)Mul_i2c_data.dew_pt_min;
	}
	else if(addr == MODBUS_HUM_DEW_PT_MAX)
	{
	  return (uint16_t)Mul_i2c_data.dew_pt_max;
	}
	else if(addr == MODBUS_HUM_PWS)
	{
	  return Mul_i2c_data.pws;
	}
	else if(addr == MODBUS_HUM_MIX_RATIO)
	{
	  return Mul_i2c_data.mix_ratio;
	}
	else if(addr == MODBUS_HUM_ENTHALPY)
	{
	  return Mul_i2c_data.enthalpy;
	}
	else if(addr == MODBUS_HUM_LIGHT_VALUE)
	{
	  return Mul_i2c_data.light_value;
	}
	else if(addr == MODBUS_HUM_AQ_VALUE)
	{
	  return Mul_i2c_data.aq_value;
	}
	else if(addr == MODBUS_HUM_CELBRA_AIR1)
	{
	  return Mul_i2c_data.celbra_air[0];
	}
	else if(addr == MODBUS_HUM_CELBRA_AIR2)
	{
	  return Mul_i2c_data.celbra_air[1];
	}
	else if(addr == MODBUS_HUM_CELBRA_AIR3)
	{
	  return Mul_i2c_data.celbra_air[2];
	}
	else if(addr == MODBUS_HUM_CELBRA_AIR4)
	{
	  return Mul_i2c_data.celbra_air[3];
	}
	else if(addr == MODBUS_HUM_AQ_LEVEL0)
	{
	  return Mul_i2c_data.aq_level[0];
	}
	else if(addr == MODBUS_HUM_AQ_LEVEL1)
	{
	  return Mul_i2c_data.aq_level[1];
	}
	else if(addr == MODBUS_HUM_AQ_LEVEL2)
	{
	  return Mul_i2c_data.aq_level[2];
	}
	else if(addr == MODBUS_HUM_MAX_AQ_LVL)
	{
	  return Mul_i2c_data.max_aq_lvl;
	}
	else if(addr == MODBUS_HUM_SCREEN_AREA_1)
	{
	  return Mul_i2c_data.screenArea[0];
	}
	else if(addr == MODBUS_HUM_SCREEN_AREA_2)
	{
	  return Mul_i2c_data.screenArea[1];
	}
	else if(addr == MODBUS_HUM_SCREEN_AREA_3)
	{
	  return Mul_i2c_data.screenArea[2];
	}
	else if(addr == MODBUS_CO2_FRC_GET)
	{
	  return Mul_i2c_data.I2C_Sensor[mul_co2_index()].co2_org;
	}
	else if(addr == MODBUS_PRESSURE_SENSOR_MODEL)
	{
	  return Mul_i2c_data.pressure_sensor_model;
	}
	else if(addr == MODBUS_PRESSURE_UNIT)
	{
	  return Mul_i2c_data.pressure_unit;
	}
	else if(addr == MODBUS_PRESSURE_UNIT_DEFAULT)
	{
	  return Mul_i2c_data.pressure_unit_default;
	}
	else if(addr == MODBUS_PRESSURE_OUTPUT_RANGE_MIN)
	{
	  return Mul_i2c_data.output_range[CHANNEL_PRE].min;
	}
	else if(addr == MODBUS_PRESSURE_OUTPUT_RANGE_MAX)
	{
	  return Mul_i2c_data.output_range[CHANNEL_PRE].max;
	}
	else if(addr == MODBUS_PRESSURE_FILTER)
	{
	  return Mul_i2c_data.pressure_filter;
	}
	else if(addr == MODBUS_PRESSURE_RAW)
	{
	  return (uint16_t)Mul_i2c_data.pressure_value_org;
	}
	else if(addr == MODBUS_PRESSURE_VALUE_SHOW)
	{
	  return (uint16_t)pressure_to_display(Mul_i2c_data.pressure_value_org, NULL);
	}
	else if(addr == MODBUS_PRESSURE_VALUE_ORG_OFFSET)
	{
	  return (uint16_t)Mul_i2c_data.pressure_value_org_offset;
	}
	else if(addr == MODBUS_CO2_TEMPERATURE_DEGREE_C_OR_F)
	{
	  return Mul_i2c_data.deg_c_or_f;
	}
	else if(addr == MODBUS_CO2_TEMPERATURE_CELSIUS)
	{
	  return (uint16_t)ext_temp_c(sensor);
	}
	else if(addr == MODBUS_CO2_TEMPERATURE_FAHRENHEIT)
	{
	  return (uint16_t)ext_temp_f(sensor);
	}
	else if(addr == MODBUS_CO2_HUMIDITY)
	{
	  return ext_hum(sensor);
	}
	else if(addr == MODBUS_CO2_INTERNAL_EXIST)
	{
	  return Mul_i2c_data.co2_internal_exist;
	}
	else if(addr == MODBUS_CO2_VALUE)
	{
	  return Mul_i2c_data.I2C_Sensor[mul_co2_index()].co2_org;
	}
	else if(addr == MODBUS_CO2_OFFSET)
	{
	  return Mul_i2c_data.co2_offset;
	}
	else if(addr == MODBUS_CO2_PREALARM_SETPOINT)
	{
	  return Mul_i2c_data.pre_alarm_setpoint;
	}
	else if(addr == MODBUS_CO2_ALARM_SETPOINT)
	{
	  return Mul_i2c_data.alarm_setpoint;
	}
	else if(addr == MODBUS_CO2_FILTER)
	{
	  return Mul_i2c_data.co2_filter;
	}
	else if(addr == MODBUS_RTC_YEAR)
	{
	  return rtc_date.year;
	}
	else if(addr == MODBUS_RTC_MONTH)
	{
	  return rtc_date.month;
	}
	else if(addr == MODBUS_RTC_DAY)
	{
	  return rtc_date.day;
	}
	else if(addr == MODBUS_RTC_WEEK)
	{
	  return rtc_date.weekday;
	}
	else if(addr == MODBUS_RTC_HOUR)
	{
	  return rtc_date.hour;
	}
	else if(addr == MODBUS_RTC_MINUTE)
	{
	  return rtc_date.minute;
	}
	else if(addr == MODBUS_RTC_SECOND)
	{
	  return rtc_date.second;
	}
	else if(addr == MODBUS_CO2_OUTPUT_RANGE_MIN_TEM)
	{
	  return Mul_i2c_data.output_range[CHANNEL_TEMP].min;
	}
	else if(addr == MODBUS_CO2_OUTPUT_RANGE_MAX_TEM)
	{
	  return Mul_i2c_data.output_range[CHANNEL_TEMP].max;
	}
	else if(addr == MODBUS_CO2_OUTPUT_RANGE_MIN_HUM)
	{
	  return Mul_i2c_data.output_range[CHANNEL_HUM].min;
	}
	else if(addr == MODBUS_CO2_OUTPUT_RANGE_MAX_HUM)
	{
	  return Mul_i2c_data.output_range[CHANNEL_HUM].max;
	}
	else if(addr == MODBUS_CO2_OUTPUT_RANGE_MIN_CO2)
	{
	  return Mul_i2c_data.output_range[CHANNEL_CO2].min;
	}
	else if(addr == MODBUS_CO2_OUTPUT_RANGE_MAX_CO2)
	{
	  return Mul_i2c_data.output_range[CHANNEL_CO2].max;
	}
	else if(addr == MODBUS_CO2_BACKLIGHT_KEEP_SECONDS)
	{
	  return backlight_keep_seconds;
	}
	else if(addr == MODBUS_CO2_HUMIDITY_FILTER)
	{
	  return Mul_i2c_data.humidity_filter;
	}
	else if(addr == MODBUS_CO2_TEMPERATURE_FILTER)
	{
	  return Mul_i2c_data.temperature_filter;
	}
	else if(addr == MODBUS_CO2_DEW_PT)
	{
	  return Mul_i2c_data.dew_pt;
	}
	else if(addr == MODBUS_CO2_DEW_PT_F)
	{
	  return Mul_i2c_data.dew_pt_f;
	}
	else if(addr == MODBUS_CO2_PWS)
	{
	  return Mul_i2c_data.pws;
	}
	else if(addr == MODBUS_CO2_MIX_RATIO)
	{
	  return Mul_i2c_data.mix_ratio;
	}
	else if(addr == MODBUS_CO2_ENTHALPY)
	{
	  return Mul_i2c_data.enthalpy;
	}
	else if(addr == MODBUS_CO2_TEMP_OFFSET)
	{
	  return Mul_i2c_data.I2C_Sensor[sensor].tem_offset;
	}
	else if(addr == MODBUS_CO2_HUM_OFFSET)
	{
	  return Mul_i2c_data.I2C_Sensor[sensor].hum_offset;
	}
	else if(addr == MODBUS_CO2_OUTPUT_HUM)
	{
	  return Mul_i2c_data.analog_output[CHANNEL_HUM];
	}
	else if(addr == MODBUS_CO2_OUTPUT_TEMP)
	{
	  return Mul_i2c_data.analog_output[CHANNEL_TEMP];
	}
	else if(addr == MODBUS_CO2_OUTPUT_CO2)
	{
	  return Mul_i2c_data.analog_output[CHANNEL_CO2];
	}
	else if(addr == MODBUS_CO2_OUTPUT_HUM_VOL_OFFSET)
	{
	  return Mul_i2c_data.output_offset[0][CHANNEL_HUM];
	}
	else if(addr == MODBUS_CO2_OUTPUT_TEMP_VOL_OFFSET)
	{
	  return Mul_i2c_data.output_offset[0][CHANNEL_TEMP];
	}
	else if(addr == MODBUS_CO2_OUTPUT_CO2_VOL_OFFSET)
	{
	  return Mul_i2c_data.output_offset[0][CHANNEL_CO2];
	}
	else if(addr == MODBUS_CO2_OUTPUT_HUM_CUR_OFFSET)
	{
	  return Mul_i2c_data.output_offset[1][CHANNEL_HUM];
	}
	else if(addr == MODBUS_CO2_OUTPUT_TEMP_CUR_OFFSET)
	{
	  return Mul_i2c_data.output_offset[1][CHANNEL_TEMP];
	}
	else if(addr == MODBUS_CO2_OUTPUT_CO2_CUR_OFFSET)
	{
	  return Mul_i2c_data.output_offset[1][CHANNEL_CO2];
	}
	else if(addr == MODBUS_CO2_FORCED_RE_CALIBRATION)
	{
	  return Mul_i2c_data.co2_frc[mul_co2_index()];
	}
	else if(addr == MODBUS_CO2_CAL)
	{
	  return Mul_i2c_data.flag_co2_asc[mul_co2_index()];
	}
	else if(addr == MODBUS_CO2_CHANNEL)
	{
	  return mul_co2_index();
	}
	else if(addr >= MODBUS_WIFI_START && addr < MODBUS_WIREGUARD_ENABLE)
	{
	  return read_wifi_data_by_block(addr);
	}
	else if(addr >= MODBUS_WIREGUARD_ENABLE && addr <= MODBUS_WIREGUARD_END)
	{
	  return wireguard_read_by_block(addr);
	}
	if(mapped)
		*mapped = 0;
	return 0;
}


void write_Mul_i2c_sensor_by_block(uint16_t addr,uint8_t HeadLen,uint8_t *pData,uint8_t type)
{
	uint8_t sensor;
	uint16_t value;

	(void)type;
	sensor = mul_sensor_index();
	value = (uint16_t)pData[HeadLen + 5] + ((uint16_t)pData[HeadLen + 4] << 8);

	if(addr == MODBUS_MUL_I2C_MODE)
	{
	  if(value <= MUL_I2C_MODE_PRESSURE || value == MUL_I2C_MODE_CUSTOM2)
	  {
	    Modbus.mul_i2c_mode = (uint8_t)value;
	    save_uint8_to_flash(FLASH_MUL_I2C_MODE, Modbus.mul_i2c_mode);
	    sync_screen_area_to_mode();
	  }
	  return;
	}

	if(addr == MODBUS_HUM_TEMPERATURE_FAHRENHEIT)
	{
	  set_ext_temp_f(sensor, (int16_t)value);
	  update_hum_para();
	}
	else if(addr == MODBUS_HUM_TEMPERATURE_CELSIUS)
	{
	  set_ext_temp_c(sensor, (int16_t)value);
	  update_hum_para();
	}
	else if(addr == MODBUS_HUM_INT_TEMPERATURE_FAHRENHEIT)
	{
	  set_int_temp_f((int16_t)value);
	}
	else if(addr == MODBUS_HUM_INT_TEMPERATURE_CELSIUS)
	{
	  set_int_temp_c((int16_t)value);
	}
	else if(addr == MODBUS_TEMPERATURE_SENSOR_SELECT)
	{
	  Mul_i2c_data.temperature_sensor_select = (uint8_t)value;
	  save_uint8_to_flash(FLASH_SENSOR_SELECT, Mul_i2c_data.temperature_sensor_select);
	}
	else if(addr == MODBUS_HUM_INT_TEMP_OFFSET)
	{
	  int16_t delta = (int16_t)((int16_t)value - Mul_i2c_data.int_temp_offset);

	  Mul_i2c_data.int_temp_offset = (int16_t)value;
	  Mul_i2c_data.internal_temperature_c += delta;
	  Mul_i2c_data.internal_temperature_f = (int16_t)(320 + Mul_i2c_data.internal_temperature_c * 9 / 5);
	  save_int_temp_offset();
	}
	else if(addr == MODBUS_HUM_TEMPERATURE_DEGREE_C_OR_F)
	{
	  Mul_i2c_data.deg_c_or_f = (uint8_t)value;
	  save_uint8_to_flash(FLASH_DEG_C_OR_F, Mul_i2c_data.deg_c_or_f);
	}
	else if(addr == MODBUS_HUM_OUTPUT_RANGE_MIN_TEM)
	{
	  Mul_i2c_data.output_range[CHANNEL_TEMP].min = (int16_t)value;
	  save_output_range();
	}
	else if(addr == MODBUS_HUM_OUTPUT_RANGE_MAX_TEM)
	{
	  Mul_i2c_data.output_range[CHANNEL_TEMP].max = (int16_t)value;
	  save_output_range();
	}
	else if(addr == MODBUS_HUM_OUTPUT_RANGE_MIN_HUMIDITY)
	{
	  Mul_i2c_data.output_range[CHANNEL_HUM].min = (int16_t)value;
	  save_output_range();
	}
	else if(addr == MODBUS_HUM_OUTPUT_RANGE_MAX_HUMIDITY)
	{
	  Mul_i2c_data.output_range[CHANNEL_HUM].max = (int16_t)value;
	  save_output_range();
	}
	else if(addr == MODBUS_HUM_TEMP_OFFSET)
	{
	  Mul_i2c_data.I2C_Sensor[sensor].tem_offset = (int16_t)value;
	  save_i2c_sensor_offsets(sensor);
	  update_hum_para();
	}
	else if(addr == MODBUS_HUM_HUMIDITY)
	{
	  set_ext_hum(sensor, (int16_t)value);
	  update_hum_para();
	}
	else if(addr == MODBUS_HUM_HUMIDITY_OFFSET)
	{
	  Mul_i2c_data.I2C_Sensor[sensor].hum_offset = (int16_t)value;
	  save_i2c_sensor_offsets(sensor);
	  update_hum_para();
	}
	else if(addr == MODBUS_HUM_EXT_TEMPERATURE_FILTER)
	{
	  Mul_i2c_data.temperature_filter = value;
	}
	else if(addr == MODBUS_HUM_HUMIDITY_FILTER)
	{
	  Mul_i2c_data.humidity_filter = value;
	}
	else if(addr == MODBUS_HUM_DEW_PT)
	{
	  Mul_i2c_data.dew_pt = (int16_t)value;
	}
	else if(addr == MODBUS_HUM_DEW_PT_F)
	{
	  Mul_i2c_data.dew_pt_f = (int16_t)value;
	}
	else if(addr == MODBUS_HUM_OUTPUT_SEL)
	{
	  if(value <= 1)
	  {
	    Mul_i2c_data.hum_output_sel = (uint8_t)value;
	    save_uint8_to_flash(FLASH_HUM_OUT_SEL, Mul_i2c_data.hum_output_sel);
	  }
	}
	else if(addr == MODBUS_HUM_DEW_PT_MIN)
	{
	  Mul_i2c_data.dew_pt_min = (int16_t)value;
	  save_int16_to_flash(FLASH_DEW_PT_MIN, Mul_i2c_data.dew_pt_min);
	}
	else if(addr == MODBUS_HUM_DEW_PT_MAX)
	{
	  Mul_i2c_data.dew_pt_max = (int16_t)value;
	  save_int16_to_flash(FLASH_DEW_PT_MAX, Mul_i2c_data.dew_pt_max);
	}
	else if(addr == MODBUS_HUM_PWS)
	{
	  Mul_i2c_data.pws = (int16_t)value;
	}
	else if(addr == MODBUS_HUM_MIX_RATIO)
	{
	  Mul_i2c_data.mix_ratio = (int16_t)value;
	}
	else if(addr == MODBUS_HUM_ENTHALPY)
	{
	  Mul_i2c_data.enthalpy = (int16_t)value;
	}
	else if(addr == MODBUS_HUM_LIGHT_VALUE)
	{
	  Mul_i2c_data.light_value = value;
	}
	else if(addr == MODBUS_HUM_AQ_VALUE)
	{
	  Mul_i2c_data.aq_value = value;
	}
	else if(addr == MODBUS_HUM_CELBRA_AIR1)
	{
	  Mul_i2c_data.celbra_air[0] = value;
	}
	else if(addr == MODBUS_HUM_CELBRA_AIR2)
	{
	  Mul_i2c_data.celbra_air[1] = value;
	}
	else if(addr == MODBUS_HUM_CELBRA_AIR3)
	{
	  Mul_i2c_data.celbra_air[2] = value;
	}
	else if(addr == MODBUS_HUM_CELBRA_AIR4)
	{
	  Mul_i2c_data.celbra_air[3] = value;
	}
	else if(addr == MODBUS_HUM_AQ_LEVEL0)
	{
	  Mul_i2c_data.aq_level[0] = value;
	}
	else if(addr == MODBUS_HUM_AQ_LEVEL1)
	{
	  Mul_i2c_data.aq_level[1] = value;
	}
	else if(addr == MODBUS_HUM_AQ_LEVEL2)
	{
	  Mul_i2c_data.aq_level[2] = value;
	}
	else if(addr == MODBUS_HUM_MAX_AQ_LVL)
	{
	  Mul_i2c_data.max_aq_lvl = value;
	}
	else if(addr == MODBUS_HUM_SCREEN_AREA_1)
	{
	  if(screen_area_is_fixed())
	    return;
	  Mul_i2c_data.screenArea[0] = (uint8_t)value;
	  save_uint8_to_flash(FLASH_SCREEN_AREA1, Mul_i2c_data.screenArea[0]);
	}
	else if(addr == MODBUS_HUM_SCREEN_AREA_2)
	{
	  if(screen_area_is_fixed())
	    return;
	  Mul_i2c_data.screenArea[1] = (uint8_t)value;
	  save_uint8_to_flash(FLASH_SCREEN_AREA2, Mul_i2c_data.screenArea[1]);
	}
	else if(addr == MODBUS_HUM_SCREEN_AREA_3)
	{
	  if(screen_area_is_fixed())
	    return;
	  Mul_i2c_data.screenArea[2] = (uint8_t)value;
	  save_uint8_to_flash(FLASH_SCREEN_AREA3, Mul_i2c_data.screenArea[2]);
	}
	else if(addr == MODBUS_PRESSURE_SENSOR_MODEL)
	{
	  Mul_i2c_data.pressure_sensor_model = value;
	}
	else if(addr == MODBUS_PRESSURE_UNIT)
	{
	  if(value <= pa)
	  {
	    Mul_i2c_data.pressure_unit = value;
	    save_uint16_to_flash(FLASH_PRESSURE_UNIT, Mul_i2c_data.pressure_unit);
	    pressure_to_display(Mul_i2c_data.pressure_value_org, NULL);
	  }
	}
	else if(addr == MODBUS_PRESSURE_UNIT_DEFAULT)
	{
	  Mul_i2c_data.pressure_unit_default = value;
	}
	else if(addr == MODBUS_PRESSURE_OUTPUT_RANGE_MIN)
	{
	  Mul_i2c_data.output_range[CHANNEL_PRE].min = (int16_t)value;
	  save_output_range();
	}
	else if(addr == MODBUS_PRESSURE_OUTPUT_RANGE_MAX)
	{
	  Mul_i2c_data.output_range[CHANNEL_PRE].max = (int16_t)value;
	  save_output_range();
	}
	else if(addr == MODBUS_PRESSURE_FILTER)
	{
	  Mul_i2c_data.pressure_filter = value;
	}
	else if(addr == MODBUS_PRESSURE_RAW || addr == MODBUS_PRESSURE_VALUE_SHOW)
	{
	  return;
	}
	else if(addr == MODBUS_PRESSURE_VALUE_ORG_OFFSET)
	{
	  Mul_i2c_data.pressure_value_org_offset = (int16_t)value;
	  pressure_to_display(Mul_i2c_data.pressure_value_org, NULL);
	}
	else if(addr == MODBUS_CO2_TEMPERATURE_DEGREE_C_OR_F)
	{
	  Mul_i2c_data.deg_c_or_f = (uint8_t)value;
	  save_uint8_to_flash(FLASH_DEG_C_OR_F, Mul_i2c_data.deg_c_or_f);
	}
	else if(addr == MODBUS_CO2_TEMPERATURE_CELSIUS)
	{
	  set_ext_temp_c(sensor, (int16_t)value);
	  update_hum_para();
	}
	else if(addr == MODBUS_CO2_TEMPERATURE_FAHRENHEIT)
	{
	  set_ext_temp_f(sensor, (int16_t)value);
	  update_hum_para();
	}
	else if(addr == MODBUS_CO2_HUMIDITY)
	{
	  set_ext_hum(sensor, (int16_t)value);
	  update_hum_para();
	}
	else if(addr == MODBUS_CO2_INTERNAL_EXIST)
	{
	  Mul_i2c_data.co2_internal_exist = (uint8_t)value;
	}
	else if(addr == MODBUS_CO2_VALUE)
	{
	  Mul_i2c_data.I2C_Sensor[mul_co2_index()].co2_org = value;
	}
	else if(addr == MODBUS_CO2_OFFSET)
	{
	  Mul_i2c_data.co2_offset = (int16_t)value;
	}
	else if(addr == MODBUS_CO2_PREALARM_SETPOINT)
	{
	  Mul_i2c_data.pre_alarm_setpoint = value;
	}
	else if(addr == MODBUS_CO2_ALARM_SETPOINT)
	{
	  Mul_i2c_data.alarm_setpoint = value;
	}
	else if(addr == MODBUS_CO2_FILTER)
	{
	  Mul_i2c_data.co2_filter = value;
	}
	else if(addr == MODBUS_RTC_YEAR)
	{
	  rtc_date.year = value;
	  PCF_SetDateTime(&rtc_date);
	  update_timers();
	}
	else if(addr == MODBUS_RTC_MONTH)
	{
	  rtc_date.month = (uint8_t)value;
	  PCF_SetDateTime(&rtc_date);
	  update_timers();
	}
	else if(addr == MODBUS_RTC_DAY)
	{
	  rtc_date.day = (uint8_t)value;
	  PCF_SetDateTime(&rtc_date);
	  update_timers();
	}
	else if(addr == MODBUS_RTC_WEEK)
	{
	  rtc_date.weekday = (uint8_t)value;
	  PCF_SetDateTime(&rtc_date);
	  update_timers();
	}
	else if(addr == MODBUS_RTC_HOUR)
	{
	  rtc_date.hour = (uint8_t)value;
	  PCF_SetDateTime(&rtc_date);
	  update_timers();
	}
	else if(addr == MODBUS_RTC_MINUTE)
	{
	  rtc_date.minute = (uint8_t)value;
	  PCF_SetDateTime(&rtc_date);
	  update_timers();
	}
	else if(addr == MODBUS_RTC_SECOND)
	{
	  rtc_date.second = (uint8_t)value;
	  PCF_SetDateTime(&rtc_date);
	  update_timers();
	}
	else if(addr == MODBUS_CO2_OUTPUT_RANGE_MIN_TEM)
	{
	  Mul_i2c_data.output_range[CHANNEL_TEMP].min = (int16_t)value;
	  save_output_range();
	}
	else if(addr == MODBUS_CO2_OUTPUT_RANGE_MAX_TEM)
	{
	  Mul_i2c_data.output_range[CHANNEL_TEMP].max = (int16_t)value;
	  save_output_range();
	}
	else if(addr == MODBUS_CO2_OUTPUT_RANGE_MIN_HUM)
	{
	  Mul_i2c_data.output_range[CHANNEL_HUM].min = (int16_t)value;
	  save_output_range();
	}
	else if(addr == MODBUS_CO2_OUTPUT_RANGE_MAX_HUM)
	{
	  Mul_i2c_data.output_range[CHANNEL_HUM].max = (int16_t)value;
	  save_output_range();
	}
	else if(addr == MODBUS_CO2_OUTPUT_RANGE_MIN_CO2)
	{
	  Mul_i2c_data.output_range[CHANNEL_CO2].min = (int16_t)value;
	  save_output_range();
	}
	else if(addr == MODBUS_CO2_OUTPUT_RANGE_MAX_CO2)
	{
	  Mul_i2c_data.output_range[CHANNEL_CO2].max = (int16_t)value;
	  save_output_range();
	}
	else if(addr == MODBUS_CO2_BACKLIGHT_KEEP_SECONDS)
	{
	  backlight_keep_seconds = (uint8)value;
	  save_uint8_to_flash(FLASH_BACKLIGHT_KEEP, backlight_keep_seconds);
	  start_back_light(backlight_keep_seconds);
	}
	else if(addr == MODBUS_CO2_HUMIDITY_FILTER)
	{
	  Mul_i2c_data.humidity_filter = value;
	}
	else if(addr == MODBUS_CO2_TEMPERATURE_FILTER)
	{
	  Mul_i2c_data.temperature_filter = value;
	}
	else if(addr == MODBUS_CO2_DEW_PT)
	{
	  Mul_i2c_data.dew_pt = (int16_t)value;
	}
	else if(addr == MODBUS_CO2_DEW_PT_F)
	{
	  Mul_i2c_data.dew_pt_f = (int16_t)value;
	}
	else if(addr == MODBUS_CO2_PWS)
	{
	  Mul_i2c_data.pws = (int16_t)value;
	}
	else if(addr == MODBUS_CO2_MIX_RATIO)
	{
	  Mul_i2c_data.mix_ratio = (int16_t)value;
	}
	else if(addr == MODBUS_CO2_ENTHALPY)
	{
	  Mul_i2c_data.enthalpy = (int16_t)value;
	}
	else if(addr == MODBUS_CO2_TEMP_OFFSET)
	{
	  Mul_i2c_data.I2C_Sensor[sensor].tem_offset = (int16_t)value;
	  save_i2c_sensor_offsets(sensor);
	  update_hum_para();
	}
	else if(addr == MODBUS_CO2_HUM_OFFSET)
	{
	  Mul_i2c_data.I2C_Sensor[sensor].hum_offset = (int16_t)value;
	  save_i2c_sensor_offsets(sensor);
	  update_hum_para();
	}
	else if(addr == MODBUS_CO2_OUTPUT_HUM)
	{
	  Mul_i2c_data.analog_output[CHANNEL_HUM] = value;
	}
	else if(addr == MODBUS_CO2_OUTPUT_TEMP)
	{
	  Mul_i2c_data.analog_output[CHANNEL_TEMP] = value;
	}
	else if(addr == MODBUS_CO2_OUTPUT_CO2)
	{
	  Mul_i2c_data.analog_output[CHANNEL_CO2] = value;
	}
	else if(addr == MODBUS_CO2_OUTPUT_HUM_VOL_OFFSET)
	{
	  Mul_i2c_data.output_offset[0][CHANNEL_HUM] = (int16_t)value;
	}
	else if(addr == MODBUS_CO2_OUTPUT_TEMP_VOL_OFFSET)
	{
	  Mul_i2c_data.output_offset[0][CHANNEL_TEMP] = (int16_t)value;
	}
	else if(addr == MODBUS_CO2_OUTPUT_CO2_VOL_OFFSET)
	{
	  Mul_i2c_data.output_offset[0][CHANNEL_CO2] = (int16_t)value;
	}
	else if(addr == MODBUS_CO2_OUTPUT_HUM_CUR_OFFSET)
	{
	  Mul_i2c_data.output_offset[1][CHANNEL_HUM] = (int16_t)value;
	}
	else if(addr == MODBUS_CO2_OUTPUT_TEMP_CUR_OFFSET)
	{
	  Mul_i2c_data.output_offset[1][CHANNEL_TEMP] = (int16_t)value;
	}
	else if(addr == MODBUS_CO2_OUTPUT_CO2_CUR_OFFSET)
	{
	  Mul_i2c_data.output_offset[1][CHANNEL_CO2] = (int16_t)value;
	}
	else if(addr == MODBUS_CO2_FORCED_RE_CALIBRATION)
	{
	  uint8_t bus;

	  bus = mul_co2_index();
	  Mul_i2c_data.co2_frc[bus] = value;
	  Mul_i2c_data.co2_cmd_bus = bus;
	  Mul_i2c_data.flag_asc_or_frc = 2;
	  save_co2_cal(bus);
	}
	else if(addr == MODBUS_CO2_CAL)
	{
	  uint8_t bus;

	  if(value <= 1)
	  {
	    bus = mul_co2_index();
	    Mul_i2c_data.flag_co2_asc[bus] = (uint8_t)value;
	    Mul_i2c_data.co2_cmd_bus = bus;
	    Mul_i2c_data.flag_asc_or_frc = 1;
	    save_co2_cal(bus);
	  }
	}
	else if(addr == MODBUS_CO2_CHANNEL)
	{
	  if((uint8_t)value <= 2)
	    Mul_i2c_data.co2_channel = (uint8_t)value;
	  save_uint8_to_flash(FLASH_CO2_BUS, Mul_i2c_data.co2_channel);
	}
	else if(addr >= MODBUS_WIFI_START && addr < MODBUS_WIREGUARD_ENABLE)
	{
	  write_wifi_data_by_block(addr, HeadLen, pData, type);
	}
	else if(addr >= MODBUS_WIREGUARD_ENABLE && addr <= MODBUS_WIREGUARD_END)
	{
	  wireguard_write_by_block(addr, HeadLen, pData);
	}
}



#define A		6.116441f
#define m		7.591386f
#define Tn		240.7263f
#define B		621.9907f
#define Patm	1013.0f

void Get_Hum_Para(signed short int Temperature,signed short int Humidity,signed short int *Dew_pt,\
					unsigned short int *Pws,unsigned short int *Mix_Ratio,unsigned short int *Enthalpy)
{
	float   ftemp1,ftemp2,ftemp;
//get dew point 	
	ftemp1 = log((float)Humidity/1000);
	ftemp2 = 17.625*Temperature;
	ftemp2 = ftemp2/(2430.4+Temperature);
	ftemp1 =ftemp1+ftemp2;
	ftemp = ftemp1*243.04/(17.625-ftemp1); 
	*Dew_pt = ftemp * 10;
//get pws	
	ftemp1 = m*ftemp;
	ftemp2 = (float)ftemp+Tn;
	ftemp1 = pow(10,ftemp1/ftemp2);
	ftemp=ftemp1 * A;  
	*Pws = ftemp * 10;
//get Mix Ratio	
	ftemp1 = ftemp * B;
	ftemp = ftemp1/(Patm-ftemp); 
	*Mix_Ratio = ftemp * 10;
//get Enthalpy  
	ftemp1 = 1.01+(0.00189*ftemp); 
	ftemp=ftemp1*Temperature + ftemp * 25;	
	*Enthalpy = ftemp;
}

void update_hum_para(void)
{
	uint8_t sensor;
	int16_t temperature;
	int16_t humidity;
	int16_t dew_pt;
	uint16_t pws;
	uint16_t mix_ratio;
	uint16_t enthalpy;

	sensor = mul_sensor_index();
	temperature = ext_temp_c(sensor);
	humidity = (int16_t)ext_hum(sensor);
	if(humidity <= 0)
		return;

	Get_Hum_Para(temperature, humidity, &dew_pt, &pws, &mix_ratio, &enthalpy);
	Mul_i2c_data.dew_pt = dew_pt;
	Mul_i2c_data.dew_pt_f = (int16_t)(dew_pt * 9 / 5 + 320);
	Mul_i2c_data.pws = (int16_t)pws;
	Mul_i2c_data.mix_ratio = (int16_t)mix_ratio;
	Mul_i2c_data.enthalpy = (int16_t)enthalpy;
}	