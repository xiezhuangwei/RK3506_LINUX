// work_current_monitor_ui.h
#ifndef WORK_CURRENT_MONITOR_UI_H
#define WORK_CURRENT_MONITOR_UI_H

#include <lvgl/lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// 初始化电流监测界面
void work_current_monitor_ui_init(void);

// 销毁界面
//void destroy_current_monitor_ui(void);

// 更新电流数据
//void update_current_data(float curr1, float curr2, float curr3, float conduction_angle, float time_ms);

// 更新微调参数
//void update_fine_tune_params(float a, float b, float c);

// 更新波形数据
//void update_waveform_data(lv_point_t* points, uint16_t point_count);

#ifdef __cplusplus
}
#endif

#endif // WORK_CURRENT_MONITOR_UI_H
