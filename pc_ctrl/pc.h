#ifndef __PC_H__
#define __PC_H__

#define SWITCH_PIN 4        // 开关控制引脚 D2
#define RESTART_PIN 5       // 重启控制引脚 D1
#define POWER_STATUS_PIN 13 // 电源状态检测引脚 D7

uint8_t pc_get_sta();
void pc_gpio_init();
void pc_power_ctrl(uint8_t sta);
void pc_restart_ctrl(void);
void pc_sta_scan();

#endif
