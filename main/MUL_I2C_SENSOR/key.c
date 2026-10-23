#include "key.h"
#include "driver/gpio.h"
#include "freertos/task.h"

u8 global_key = KEY_NON;
u16 pre_key = KEY_NON;
u8 lock_menu;

void KEY_Init(void)
{
	gpio_config_t io_conf = {0};

	io_conf.intr_type = GPIO_INTR_DISABLE;
	io_conf.mode = GPIO_MODE_INPUT;
	io_conf.pin_bit_mask = (1ULL << GPIO_SW1) | (1ULL << GPIO_SW2) |
		(1ULL << GPIO_SW3) | (1ULL << GPIO_SW4);
	io_conf.pull_down_en = 0;
	/* External 10K pull-ups on board; keep internal pull-up as backup */
	io_conf.pull_up_en = 1;
	gpio_config(&io_conf);

	if(qKey == NULL)
		qKey = xQueueCreate(8, sizeof(uint16));
}

static uint16 key_gpio_raw(void)
{
	uint16 key_val = KEY_NON;

	/* Active-low: pressed == 0 */
	if(gpio_get_level(GPIO_SW1) == 0)
		key_val |= KEY_1;
	if(gpio_get_level(GPIO_SW2) == 0)
		key_val |= KEY_2;
	if(gpio_get_level(GPIO_SW3) == 0)
		key_val |= KEY_3;
	if(gpio_get_level(GPIO_SW4) == 0)
		key_val |= KEY_4;

	return key_val;
}

u8 KEY_Scan(void)
{
	uint16 key_1st, key_2nd;
	uint16 key_val = KEY_NON;

	key_1st = key_gpio_raw();
	delay_ms(20);
	key_2nd = key_gpio_raw();

	if(key_1st & key_2nd & KEY_1)
	{
		key_val |= KEY_3;
	}

	if(key_1st & key_2nd & KEY_2)
		key_val |= KEY_2;

	if(key_1st & key_2nd & KEY_3)
	{
		key_val |= KEY_1;
	}

	if(key_1st & key_2nd & KEY_4)
		key_val |= KEY_4;

	return (u8)(key_val >> 12);
}

void vKEYTask(void *pvParameters)
{
	u16 key_temp;
	static U8_T long_press_key_start = 0;

	KEY_Init();
	delay_ms(5000);
	task_test.enable[10] = 1;
	for( ;; )
	{
		task_test.count[10]++;

		if((key_temp = KEY_Scan()) != pre_key)
		{
			xQueueSend(qKey, &key_temp, 0);
			pre_key = key_temp;
			long_press_key_start = 0;
			Test[35] = key_temp;
		}
		else
		{
			if(key_temp != KEY_NON)
			{
				if(long_press_key_start >= LONG_PRESS_TIMER_SPEED_100)
					key_temp |= KEY_SPEED_100;
				else if(long_press_key_start >= LONG_PRESS_TIMER_SPEED_50)
					key_temp |= KEY_SPEED_50;
				else if(long_press_key_start >= LONG_PRESS_TIMER_SPEED_10)
					key_temp |= KEY_SPEED_10;
				else if(long_press_key_start >= LONG_PRESS_TIMER_SPEED_1)
					key_temp |= KEY_SPEED_1;

				if(long_press_key_start >= LONG_PRESS_TIMER_SPEED_1)
				{
					/* Menu redraw is slower than this scan. Queue only one
					   repeat so the value stops when the key is released. */
					if(uxQueueMessagesWaiting(qKey) == 0)
					{
						xQueueSend(qKey, &key_temp, 0);
						Test[36]++;
						Test[37] = key_temp;
					}
				}

				if(long_press_key_start < LONG_PRESS_TIMER_SPEED_100)
					long_press_key_start++;
			}
		}
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
}

void vStartKeyTask(unsigned char uxPriority)
{
	xTaskCreate(vKEYTask, "KEYTask", 2048, NULL, uxPriority, NULL);
}
