// work_system_params_ui.h
#ifndef WORK_SYSTEM_PARAMS_UI_H
#define WORK_SYSTEM_PARAMS_UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

// 界面初始化
void work_system_params_ui_init(void);
void destroy_system_params_ui(void);

// 参数更新函数
void update_spec_select_mode(const char* mode);
void update_system_filter_temp(float value);
void update_system_filter_time(int value);
void update_system_temp_warn_limit(float value);
void update_system_current_warn_output(int value);
void update_system_water_flow_limit(float value);
void update_system_weld_end_time(int value);
void update_system_constant_angle_state(bool enable);
void update_system_max_secondary_current(float value);
void update_system_current_warn(int value);
void update_system_current_delay(int value);
void update_system_target_achieve(int value);
void update_system_pre_pressure_state(bool enable);
void update_system_tail_current_state(bool enable);
void update_system_warn_enable_state(bool enable);
void update_system_dual_regulator_state(bool enable);
void update_system_auto_reset_state(bool enable);
void update_system_adjust_enable_state(bool enable);
void update_specification_number(int spec_num);

// 更新所有参数
void update_all_system_params(const char* spec_mode, int spec_num,
                              float filter_temp, int filter_time, float temp_warn,
                              int current_warn_out, float water_flow, int weld_end_time,
                              bool const_angle, float max_sec_current, int current_warn,
                              int current_delay, int target_achieve, bool pre_pressure,
                              bool tail_current, bool warn_enable, bool dual_reg,
                              bool auto_reset, bool adj_enable);

#ifdef __cplusplus
}
#endif

#endif // WORK_SYSTEM_PARAMS_UI_H
