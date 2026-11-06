#ifndef __KEY_H
#define __KEY_H

#include "define.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

/* Schematic: SW1-SW4 with 10K pull-up to VDD33, active-low */
#define GPIO_SW1	5
#define GPIO_SW2	6
#define GPIO_SW3	7
#define GPIO_SW4	15

#define KEY_NON		0X00
#define	KEY_1		0X1000
#define	KEY_2		0X2000
#define	KEY_3		0X4000
#define	KEY_4		0X8000
#define KEY_1_2 	0X3000

#define	KEY_LEFT	(1 << 12)
#define	KEY_RIGHT	(1 << 13)
#define	KEY_UP		(1 << 14)
#define	KEY_DOWN	(1 << 15)

#define	KEY_UP_MASK			(1 << 0)
#define	KEY_DOWN_MASK		(1 << 1)
#define	KEY_LEFT_MASK		(1 << 2)
#define	KEY_RIGHT_MASK		(1 << 3)

#define KEY_SPEED_1			(0x0000)
#define KEY_SPEED_10		(0x0100)
#define KEY_SPEED_50		(0x0200)
#define KEY_SPEED_100		(0x0300)
#define KEY_SPEED_MASK		(0x00ff)
#define KEY_FUNCTION_MASK	(0xff00)
#define KEY_HOLD_LR_MASK	(KEY_LEFT_MASK | KEY_RIGHT_MASK | KEY_SPEED_10)

#define	LONG_PRESS_TIMER_SPEED_100	70
#define	LONG_PRESS_TIMER_SPEED_50	50
#define	LONG_PRESS_TIMER_SPEED_10	30
#define	LONG_PRESS_TIMER_SPEED_1	10

#define SPEED_100		111
#define SPEED_50		55
#define SPEED_10		11
#define SPEED_1			1

extern u8 lock_menu;
extern QueueHandle_t qKey;
extern u8 global_key;
extern u16 pre_key;
void KEY_Init(void);
void vKEYTask(void *pvParameters);
void vStartKeyTask(unsigned char uxPriority);

#endif
