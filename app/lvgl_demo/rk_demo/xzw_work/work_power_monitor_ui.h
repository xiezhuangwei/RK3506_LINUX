// work_power_monitor_ui.h
#ifndef _WORK_POWER_MONITOR_UI_H
#define _WORK_POWER_MONITOR_UI_H

#include <lvgl/lvgl.h>

// 电源监控UI初始化
void work_power_monitor_ui_init(void);

// 销毁电源监控UI
void destroy_power_monitor_ui(void);

// 更新监控数据
//void update_monitor_data(float voltage, float igbt1_temp, float igbt2_temp, float water_flow);
//void update_alarm_status(const char* alarm_name, bool is_active);

#endif
