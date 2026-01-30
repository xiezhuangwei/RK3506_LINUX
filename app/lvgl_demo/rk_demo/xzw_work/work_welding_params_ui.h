// work_welding_params_ui.h
#ifndef WORK_WELDING_PARAMS_UI_H
#define WORK_WELDING_PARAMS_UI_H

#include <lvgl/lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// UI初始化
void work_welding_params_ui_init(void);
void destroy_welding_params_ui(void);

// 数据更新函数
void update_param_value(int index, float value);
void update_frequency(float freq);
void update_time(float time_ms);
void update_welding_stage(int stage_index, bool active);

#ifdef __cplusplus
}
#endif

#endif /* WORK_WELDING_PARAMS_UI_H */

