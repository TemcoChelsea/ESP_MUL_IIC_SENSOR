//==============================================================================
//    S E N S I R I O N   AG,  Laubisruetistr. 44, CH-8712 Staefa, Switzerland
//==============================================================================
// Project   :  SHT3x Sample Code (V1.0)
// File      :  sht3x.h (V1.0)
// Author    :  RFU
// Date      :  16-Jun-2014
// Controller:  STM32F100RB
// IDE       :  �Vision V4.71.2.0
// Compiler  :  Armcc
// Brief     :  Sensor Layer: Definitions of commands and functions for sensor
//                            access.
//==============================================================================

#ifndef MUL_I2C_SENSOR_H
#define MUL_I2C_SENSOR_H

//-- Includes ------------------------------------------------------------------
#include "esp_err.h"
#include "types.h"
#include "sensirion_common.h"
#include "freertos/FreeRTOS.h"

#define LITTLE_ENDIAN



#define CHANNEL_TEMP	1 // OUT2
#define CHANNEL_HUM		0 // OUT1
#define CHANNEL_CO2		2 // OUT3
#define CHANNEL_PRE		3 //  OUT1

typedef struct
{
	S16_T min;
	S16_T max;
} _RANGE_;

#pragma pack(push, 1)
typedef struct
{
	uint16_t tem_org;
	uint16_t hum_org;
	uint16_t co2_org;
	int16_t tem_offset;
	int16_t hum_offset;
}Str_Mul_I2C;

typedef struct
{
	uint8_t deg_c_or_f;
	int16_t internal_temperature_c;
	int16_t internal_temperature_f;
	int16_t int_temp_offset; /* 0.1°C, applied to the 10k sensor */

	uint16_t pre_alarm_setpoint;
	uint16_t alarm_setpoint;
	uint8_t alarm_state;
	uint8_t pre_alarm_on_time;
	uint8_t pre_alarm_off_time;
	uint8_t alarm_delay_time;

	uint8_t output_mode; // 4_20ma 0-5v 0-10v

	uint8_t i2c_sensor_type[3];
	Str_Mul_I2C I2C_Sensor[3];
	

	uint16 co2_asc; // 强制校正后的co2 read-only
	uint16 co2_frc[3]; // 每路 I2C 的 FRC 参考值，ppm
	uint8_t flag_co2_asc[3]; // 0 disable ASC, 1 enable ASC
	uint8_t flag_asc_or_frc; // 1 ASC, 2 FRC, 0 none. Cleared after the I2C packet is sent
	uint8_t co2_cmd_bus; // bus latched when 3141/3142 is written
	uint8_t co2_channel; // current CO2 I2C bus, 0..2

	uint16 analog_output[3];
	int16_t output_offset[2][3]; /* [0] voltage, [1] current; [ch] HUM/TEMP/CO2 */
	uint8_t alarmEnable;
	uint16_t backlight_keep_seconds;

	uint8_t sensor_select;
	uint8_t co2_internal_exist;
	uint16_t co2_filter;
	int16_t co2_offset;

	uint16_t humidity_filter;
	uint16_t temperature_filter;
	int16_t dew_pt;
	int16_t dew_pt_f;
	uint8_t hum_output_sel; /* 0 dew point output, 1 humidity output */
	int16_t dew_pt_min;     /* C, default 70 */
	int16_t dew_pt_max;     /* C, default -30 */
	int16_t mix_ratio;
	int16_t enthalpy;
	int16_t pws;
	uint16_t light_value;
	uint16_t aq_value;
	uint16_t celbra_air[4];
	uint16_t aq_level[3];
	uint16_t max_aq_lvl;

	_RANGE_ output_range[4]; /* CHANNEL_HUM / TEMP / CO2 / PRE */

	uint16_t pressure_sensor_model;
	uint16_t pressure_unit;
	uint16_t pressure_unit_default;
	uint16_t pressure_filter;
	int16_t pressure_value_org;       /* raw, Pa */
	int16_t pressure_value_show;      /* LCD value after unit conversion */
	int16_t pressure_value_org_offset;
	
	uint8_t screenArea[3];
	uint8_t temperature_sensor_select; /* 0 internal 10k, 1 external SHT */
	uint8_t internal_temp_valid;       /* 1 after the 10k ADC is inside the table */

}STR_Mul_Sensor;
#pragma pack(pop)


typedef enum
{
	E_I2C_NO_SENSOR = 0,
	E_I2C_SHT3X = 1,
	E_I2C_SHT4X,//2
	E_I2C_SCD4X,//3
	E_I2C_SHT3X_SCD4X,// 4
	E_I2C_SHT4X_SCD4X,//5
}E_I2C_SENSOR;

// output mode
typedef enum
{
	E_OUT_FAULT = 0,
	E_0_10V = 1,
	E_0_5V,
	E_4_20MA
}E_OUTPUT_MODE;


#define DEGREE_C 0
#define DEGREE_F 1

/* Pressure sensor model (Modbus 700 / menu 1.Model). */
enum
{
	WF100DPZ = 0,
};

/* Pressure display units. Index matches Prs_Unit[]. Sensor value is Pa. */
enum
{
	inWC = 0,
	KPa1,
	Psi,
	mmHg,
	inHg,
	Kg_cm2,
	atmosphere,
	bar,
	pa
};

extern _RANGE_ default_output_range_table[4];
extern const uint8 Prs_Unit[][6];
int16_t pressure_to_display(int16_t pressure_pa, uint8_t *decimals);

extern STR_Mul_Sensor Mul_i2c_data;
#define HUM_data  	Mul_i2c_data
#define PRESS_data	Mul_i2c_data
#define CO2_data  	Mul_i2c_data

//extern uint8_t scd4x_perform_forced;
//extern uint16_t co2_asc;
//extern uint16_t co2_frc;


uint16_t read_Mul_i2c_sensor_by_block(uint16_t addr, uint8_t *mapped);
void write_Mul_i2c_sensor_by_block(uint16_t addr,uint8_t HeadLen,uint8_t *pData,uint8_t type);
void update_hum_para(void);
void sync_screen_area_to_mode(void);
uint8_t screen_area_is_fixed(void);
void select_active_i2c_sensor(const uint8_t hum_exists[3], const uint8_t sensor_type[3]);
void mul_idle_reading(int16_t *tem, int16_t *hum, int16_t *co2);
uint8_t mul_co2_index(void);
void set_co2_channel(uint8_t bus);
void set_co2_frc_cal(uint16_t value);
void save_co2_cal(uint8_t bus);
void save_int_temp_offset(void);
void save_i2c_sensor_offsets(uint8_t sensor);
int16_t calibrated_humidity(uint8_t sensor, uint16_t raw);
int16_t hum_channel_reading(int16_t humidity_tenth);



#endif
