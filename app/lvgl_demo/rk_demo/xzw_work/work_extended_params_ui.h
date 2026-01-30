// work_extended_params_ui.h
#ifndef WORK_EXTENDED_PARAMS_UI_H
#define WORK_EXTENDED_PARAMS_UI_H

#include <stdbool.h>

// 界面初始化
void work_extended_params_ui_init(void);
void destroy_extended_params_ui(void);

// 参数更新接口
void update_current_ref(float value);
void update_current_over(float percent);
void update_current_under(float percent);
void update_alarm_enable(bool enable);
void update_prog_output(int index, int value);
void update_prog_enable(int index, bool enable);
void update_weld_current(int index, float value);

#endif // WORK_EXTENDED_PARAMS_UI_H
