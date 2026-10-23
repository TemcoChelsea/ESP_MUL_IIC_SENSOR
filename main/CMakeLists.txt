idf_component_register(		SRCS
							"app_main.c"
							"modbus.c"
							"Mqtt_Handler/Mqtt_Handler.c"
#							"deviceparams.c"
							"i2c_task.c"
							"ota.c"
#							"microphone.c"
							"wifi.c"
							"sgp30_featureset.c"
							"sensirion_common.c"
							"sgp30.c"
							"ethernet_task.c"
							"flash.c"
							"pro_info.c"
#							"scd40.c"
							"store.c"
							"led_pwm.c"
							"inputs.c"
							"outputs.c"
							"io.c"
							"mlx90632.c"
							"mlx90632_extended_meas.c"
							"rtc.c"
							"timegm.c"
							"sht4x.c"
							"sht3x.c"
							"scd4x_i2c.c"
							"scan.c"
							"commsub.c"
							"ade7953.c"
							"MUL_I2C_SENSOR/LCD.c"
							"MUL_I2C_SENSOR/key.c"
#							"pic.c"
							"smtp_client.c"
						#	"airlab.c"
						#	"LightSwitch.c"
						#	"LS_LED.c"
							"INA228.c"
							"mppt_task.c"
#							"spi_master_arm.c"
#							"ble_mesh.c"
						#	"Constcode.c"
							"MUL_I2C_SENSOR/Menu.c"
							"MUL_I2C_SENSOR/menuIdle.c"
							"MUL_I2C_SENSOR/menuMain.c"
#							"MUL_I2C_SENSOR/menuSet.c"
							"MUL_I2C_SENSOR/menuAbout.c"
							"MUL_I2C_SENSOR/menuCo2_Conf.c"
							"MUL_I2C_SENSOR/menuCo2_Opt.c"
							"MUL_I2C_SENSOR/menuHum_Conf.c"
							"MUL_I2C_SENSOR/menuTemp_Conf.c"
							"MUL_I2C_SENSOR/menuMisc.c"
							"MUL_I2C_SENSOR/Menu_Pres_Conf.c"
							"MUL_I2C_SENSOR/menu_link_stubs.c"
						#	"menuDaySet.c"
							"sntp_app.c"
							"multiMeter.c"
							"mm_spi.c"
							"hy3131_sw.c"
							"Light_pwm.c"
					#		"ddns.c"
						#	"lowPower.c"
							"Mul_I2C_sensor.c"
							"lora.c"
							"LcdTheme.c"
#							"DisHomeScreen.c"
							"DisFonts.c"
							"DisSymbol.c"
						#	"LCD_Driver/lcd_drv.c" 	//T11
						#	"LCD_Driver/lcd_task.c" //T11
						#	"LCD_Driver/touch_drv.c"//T11
						#	"LCD_Driver/lv_UserPeram.c"//T11
							"WireGuard_App/WireGuard_App.c"
							"wifi_web_server.c"

						#	EMBED_TXTFILES server_root_cert.pem esp_logo.png
#							EMBED_TXTFILES ${project_dir}/server_certs/smtp2go.crt
			INCLUDE_DIRS "MUL_I2C_SENSOR"
							"."
							"LCD_Driver"
							"TemcoScreen"
							"Mqtt_Handler"
							"WireGuard_App"
							)

# Auto-include all .c files from TemcoScreen subdirectory
file(GLOB TEMCO_SCREEN_SOURCES "TemcoScreen/*.c")
target_sources(${COMPONENT_LIB} PRIVATE ${TEMCO_SCREEN_SOURCES})

#                    		EMBED_TXTFILES ${project_dir}/server_certs/ca_cert.pem)
set(COMPONENT_ADD_INCLUDEDIRS ".")

target_link_libraries(${COMPONENT_TARGET} "-u pro_info")
